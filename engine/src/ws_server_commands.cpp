/**
 * @file ws_server_commands.cpp
 * @brief Resource policy, transactional bridge changes, and command dispatch.
 */

#include "ws_server_internal.h"

#include "ftd/constants.h"
#include "ftd/lagrangian.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef FTD_ENABLE_CUDA
#include <cuda_runtime_api.h>
#endif

namespace ftd::ws_server_detail {

// SOCKET is declared at global scope in ws_protocol.h (platform-aligned)
// so no `using` is needed — just the WS opcode enum values.
using ftd::WS_TEXT;
using ftd::WS_BINARY;
using ftd::WS_CLOSE;
using ftd::WS_PING;
using ftd::WS_PONG;

constexpr std::uint64_t kGiB = 1024ull * 1024ull * 1024ull;

// Conservative construction estimates from the 2026-08-16 native call-stack
// audit. Available memory is sampled while the old bridge remains live, so the
// decision budgets the actual transactional replacement peak.
// Budget the largest profile the server can acknowledge, not merely the
// default scenario.  Enabling both gauge sectors lazily allocates six live +
// six scratch link fields (1056 GPU B/site) and their six canonical host link
// arrays (528 host B/site).  The former 700 GPU B/site estimate could approve a
// large resize and then OOM on the first gauge-enabled tick.
constexpr std::uint64_t kHostBytesPerVoxelEstimate = 2048ull;
constexpr std::uint64_t kGpuBytesPerVoxelEstimate = 2048ull;
constexpr std::uint64_t kHostConstructionOverhead = 512ull * 1024ull * 1024ull;
constexpr std::uint64_t kGpuConstructionOverhead = 512ull * 1024ull * 1024ull;
constexpr std::uint64_t kHostReserve = 2ull * kGiB;
constexpr std::uint64_t kGpuReserve = 2ull * kGiB;
// A raw `run(n)` must never monopolize the native command/publisher thread.
// The browser already emits these chunks serially; enforce the same bound for
// direct protocol clients so telemetry and stop/control messages regain an
// observation boundary between every batch.
constexpr int interactive_run_chunk_limit(int lattice_size) {
    if (lattice_size >= 113) return 1;
    if (lattice_size >= 65) return 2;
    if (lattice_size >= 49) return 4;
    return 8;
}


std::uint64_t host_available_bytes() {
#ifdef _WIN32
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    return GlobalMemoryStatusEx(&status) ? status.ullAvailPhys : 0;
#else
    std::ifstream meminfo("/proc/meminfo");
    std::string key;
    std::uint64_t value_kib = 0;
    std::string unit;
    while (meminfo >> key >> value_kib >> unit) {
        if (key == "MemAvailable:") return value_kib * 1024ull;
    }
    return 0;
#endif
}

std::uint64_t gpu_available_bytes() {
#ifdef FTD_ENABLE_CUDA
    std::size_t free_bytes = 0;
    std::size_t total_bytes = 0;
    if (cudaMemGetInfo(&free_bytes, &total_bytes) == cudaSuccess)
        return static_cast<std::uint64_t>(free_bytes);
#endif
    return 0;
}

ResourceBudget resource_budget(int requested_size) {
    ResourceBudget out;
    out.size = std::clamp(requested_size, kMinLatticeSize, kMaxLatticeSize);
    out.voxels = static_cast<std::uint64_t>(out.size) * out.size * out.size;
    out.host_required = out.voxels * kHostBytesPerVoxelEstimate +
                        kHostConstructionOverhead;
    out.gpu_required = out.voxels * kGpuBytesPerVoxelEstimate +
                       kGpuConstructionOverhead;
    out.host_available = host_available_bytes();
    out.gpu_available = gpu_available_bytes();
    out.host_ok = out.host_available > kHostReserve &&
                  out.host_required <= out.host_available - kHostReserve;
#ifdef FTD_ENABLE_CUDA
    out.gpu_ok = out.gpu_available > kGpuReserve &&
                 out.gpu_required <= out.gpu_available - kGpuReserve;
#endif
    return out;
}

std::string json_budget(const ResourceBudget& b) {
    std::ostringstream ss;
    ss << "{\"type\":\"resize_preflight\",\"ok\":"
       << (b.accepted() ? "true" : "false")
       << ",\"accepted\":" << (b.accepted() ? "true" : "false")
       << ",\"size\":" << b.size
       << ",\"voxels\":" << b.voxels
       << ",\"estimatedHostBytes\":" << b.host_required
       << ",\"availableHostBytes\":" << b.host_available
       << ",\"estimatedGpuBytes\":" << b.gpu_required
       << ",\"availableGpuBytes\":" << b.gpu_available
       << "}";
    return ss.str();
}

std::string budget_error(const ResourceBudget& b) {
    std::ostringstream ss;
    ss << "L=" << b.size << " rejected by native memory preflight (host "
       << std::fixed << std::setprecision(2)
       << static_cast<double>(b.host_required) / kGiB << " GiB required, "
       << static_cast<double>(b.host_available) / kGiB << " GiB available; GPU "
       << static_cast<double>(b.gpu_required) / kGiB << " GiB required, "
       << static_cast<double>(b.gpu_available) / kGiB << " GiB available).";
    return ss.str();
}

std::string json_progress(const char* operation, const char* phase, int size) {
    std::ostringstream ss;
    ss << "{\"type\":\"operation_progress\",\"operation\":\""
       << operation << "\",\"phase\":\"" << phase
       << "\",\"size\":" << size << "}";
    return ss.str();
}


// ============================================================================
//  JSON response builders
// ============================================================================

std::string json_ok(int tick, int lattice_size = 0) {
    std::ostringstream ss;
    ss << "{\"ok\":true,\"tick\":" << tick;
    if (lattice_size > 0) ss << ",\"latticeSize\":" << lattice_size;
    ss << "}";
    return ss.str();
}

std::string json_escape(const std::string& value) {
    // JSON strings may not contain literal control characters. Validation
    // messages conventionally end in '\n', so quote/backslash-only escaping
    // produced malformed replies and left request/ack clients waiting forever.
    static constexpr char HEX[] = "0123456789abcdef";
    std::string escaped;
    escaped.reserve(value.size());
    for (const unsigned char c : value) {
        switch (c) {
        case '"': escaped += "\\\""; break;
        case '\\': escaped += "\\\\"; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
            if (c < 0x20) {
                escaped += "\\u00";
                escaped += HEX[(c >> 4) & 0x0f];
                escaped += HEX[c & 0x0f];
            } else {
                escaped += static_cast<char>(c);
            }
        }
    }
    return escaped;
}

std::string json_error(const std::string& msg,
                       const std::string& operation = {}) {
    std::string response = "{\"error\":\"" + json_escape(msg) + "\"";
    // Fire-and-forget visual and simulation commands have no request id. Echo
    // the command name so the dashboard can release the correct in-flight
    // guard after a CUDA/runtime exception instead of freezing permanently.
    if (!operation.empty()) {
        response += ",\"operation\":\"" + json_escape(operation) + "\"";
    }
    response += '}';
    return response;
}

// A CUDA snapshot event which timed out or threw is a process-level recovery
// boundary. In particular, GpuBuffers teardown may synchronize that same
// event, so accepting reset/setup_scenario and replacing RenderBridge could
// hang the desktop process indefinitely. Keep this as an ordinary error shape
// (so RPC waiters resolve) with explicit, machine-readable restart semantics.
std::string json_native_recovery_required(
    const std::string& operation,
    const ftd::NativeTelemetryScheduler& telemetry) {
    const std::string& recorded_reason = telemetry.suspension_reason();
    const std::string reason = recorded_reason.empty()
        ? "native GPU telemetry observation failed; restart required"
        : recorded_reason;
    std::ostringstream ss;
    ss << "{\"type\":\"native_recovery_required\""
       << ",\"error\":\"" << json_escape(reason) << "\""
       << ",\"operation\":\"" << json_escape(operation) << "\""
       << ",\"restartRequired\":true"
       << ",\"sourceEpoch\":" << telemetry.source_epoch()
       << ",\"telemetrySourceEpoch\":" << telemetry.source_epoch()
       << ",\"telemetryEpoch\":" << telemetry.epoch()
       << ",\"telemetrySnapshotVersion\":" << telemetry.snapshot_version()
       << '}';
    return ss.str();
}

// A source replacement would destroy the current RenderBridge. If a CUDA
// observation fence is still active, teardown may synchronize that fence, so
// make replacement an explicit non-blocking retry boundary instead of letting
// a resize/scenario command stall the transport thread behind GPU work.
std::string json_operation_deferred(
    const std::string& operation,
    const ftd::NativeTelemetryScheduler& telemetry,
    const char* reason = "telemetry_settling") {
    const auto view = telemetry.latest();
    std::ostringstream ss;
    ss << "{\"type\":\"operation_deferred\""
       << ",\"operation\":\"" << json_escape(operation) << "\""
       << ",\"reason\":\"" << reason << "\""
       << ",\"retryAfterMs\":16"
       << ",\"sourceEpoch\":" << view.source_epoch
       << ",\"telemetrySourceEpoch\":" << view.source_epoch
       << ",\"epoch\":" << view.epoch
       << ",\"tick\":" << view.tick
       << ",\"snapshotVersion\":" << view.snapshot_version
       << ",\"pendingMask\":" << view.pending_mask
       << '}';
    return ss.str();
}

std::uint64_t request_id_from(const std::string& json) {
    const double raw = ftd::json_number(json, "_requestId");
    if (!std::isfinite(raw) || raw <= 0.0) return 0;
    return static_cast<std::uint64_t>(raw);
}

std::string attach_request_id(std::string response, std::uint64_t request_id) {
    if (request_id == 0 || response.empty() || response.back() != '}')
        return response;
    response.pop_back();
    response += ",\"_requestId\":" + std::to_string(request_id) + "}";
    return response;
}

bool send_json_response(SOCKET client, std::string response,
                        std::uint64_t request_id) {
    return ftd::ws_send_text(
        client, attach_request_id(std::move(response), request_id));
}

const ftd::ToggleSpec* find_toggle_spec(const std::string& name) {
    return ftd::term_toggles_detail::find_spec(name);
}

bool apply_profile_fields(const std::string& json,
                          ftd::TermToggles& staged,
                          std::string& error) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const std::string key = std::string("toggle_") + spec.name;
        if (ftd::json_has_key(json, key))
            staged.*(spec.field) = ftd::json_bool(json, key);
    }

    if (ftd::json_has_key(json, "fluxBoundaryMode")) {
        const int mode = static_cast<int>(ftd::json_number(json, "fluxBoundaryMode"));
        if (mode < static_cast<int>(ftd::FluxBoundaryMode::Periodic)
            || mode > static_cast<int>(ftd::FluxBoundaryMode::Dispersal)) {
            error = "fluxBoundaryMode must be 0 (periodic), 1 (reflective), or 2 (dispersal)";
            return false;
        }
        staged.flux_boundary = static_cast<ftd::FluxBoundaryMode>(mode);
    }

    if (!staged.validate(&error)) return false;
    return true;
}

bool validate_profile_for_bridge(const ftd::RenderBridge& rb,
                                 const ftd::TermToggles& staged,
                                 std::string& error) {
    const bool gpu = rb.backend_kind() == ftd::Backend::Kind::Gpu;
    const std::uint8_t backend = gpu
        ? ftd::ToggleBackend::GPU : ftd::ToggleBackend::CPU;
    return staged.validate_backend(
        backend,
        gpu && rb.interactive_gpu_mode(),
        &error);
}

std::string json_profile_ack(const ftd::RenderBridge& rb,
                             const ftd::NativeTelemetryScheduler& telemetry,
                             const std::string& scenario = {}) {
    std::ostringstream ss;
    ss << std::setprecision(17);
    ss << "{\"ok\":true,\"tick\":" << rb.current_tick()
       << ",\"latticeSize\":" << rb.lattice().size();
    ss << ",\"sourceEpoch\":" << telemetry.source_epoch()
       << ",\"telemetrySourceEpoch\":" << telemetry.source_epoch()
       << ",\"telemetryEpoch\":" << telemetry.epoch();
    if (!scenario.empty()) ss << ",\"scenario\":\"" << scenario << "\"";
    ss << ",\"fluxBoundaryMode\":"
       << static_cast<int>(rb.toggles.flux_boundary)
       << ",\"toggles\":{";
    bool first = true;
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (!first) ss << ',';
        first = false;
        ss << '\"' << spec.name << "\":"
           << ((rb.toggles.*(spec.field)) ? "true" : "false");
    }
    ss << "},\"params\":{"
       << "\"dt\":" << rb.dt()
       << ",\"kb\":" << ftd::K_B
       << ",\"gn\":" << ftd::G_N
       << ",\"damping\":" << ftd::DAMPING
       << ",\"omega0\":" << rb.toggles.omega0
       << ",\"langevin_T\":" << rb.toggles.langevin_T
       << ",\"langevin_gamma\":" << rb.toggles.langevin_gamma
       << ",\"langevin_seed\":" << rb.toggles.langevin_seed
       << ",\"coulomb_source_scale\":" << rb.toggles.coulomb_source_scale
       << ",\"kinetic_drain\":" << rb.toggles.kinetic_drain
       << "}}";
    return ss.str();
}

std::unique_ptr<ftd::RenderBridge> make_interactive_bridge(int lattice_size) {
    auto bridge = std::make_unique<ftd::RenderBridge>(lattice_size);
    bridge->set_interactive_gpu_mode(true);
    return bridge;
}

bool replace_bridge_transactionally(
    SOCKET client,
    std::unique_ptr<ftd::RenderBridge>& rb,
    ftd::NativeTelemetryScheduler& telemetry,
    int& lattice_size,
    int requested_size,
    const char* operation,
    const std::optional<std::string>& scenario = std::nullopt,
    std::uint64_t request_id = 0,
    const std::string* profile_json = nullptr) {
    // RenderBridge assignment destroys the old CUDA backend. Do not reach
    // candidate construction or that destructive assignment while the native
    // snapshot scheduler owns a live fence: GpuBuffers teardown can
    // synchronize it. If the old fence is already finished, one nonblocking
    // pump retires it; otherwise the client gets an explicit retry boundary.
    if (!telemetry.safe_to_replace()) {
        try {
            telemetry.pump(*rb);
        } catch (const std::exception& ex) {
            if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
            return send_json_response(
                client, json_native_recovery_required(operation, telemetry), request_id);
        } catch (...) {
            const std::string message = "unknown native telemetry publisher failure";
            if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
            return send_json_response(
                client, json_native_recovery_required(operation, telemetry), request_id);
        }
    }
    if (telemetry.suspended()) {
        return send_json_response(
            client, json_native_recovery_required(operation, telemetry), request_id);
    }
    if (!telemetry.safe_to_replace()) {
        return send_json_response(
            client, json_operation_deferred(operation, telemetry), request_id);
    }
    try {
        if (!rb->visual_snapshot_safe_to_replace()) {
            return send_json_response(
                client, json_operation_deferred(
                    operation, telemetry, "visual_settling"), request_id);
        }
    } catch (const std::exception& ex) {
        if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
        return send_json_response(
            client, json_native_recovery_required(operation, telemetry), request_id);
    } catch (...) {
        const std::string message = "unknown native visual source-barrier failure";
        if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
        return send_json_response(
            client, json_native_recovery_required(operation, telemetry), request_id);
    }

    const ResourceBudget budget = resource_budget(requested_size);
    if (!budget.accepted()) {
        const std::string message = budget_error(budget);
        std::cerr << "[ws_server] " << message << "\n";
        return send_json_response(client, json_error(message, operation), request_id);
    }

    if (!ftd::ws_send_text(client, json_progress(operation, "allocating", budget.size)))
        return false;

    try {
        auto candidate = make_interactive_bridge(budget.size);
        if (scenario && !ftd::dispatch_scenario(*candidate, *scenario)) {
            const std::string message = "failed to dispatch scenario: " + *scenario;
            std::cerr << "[ws_server] " << message << "\n";
            return send_json_response(client, json_error(message, operation), request_id);
        }

        if (profile_json && ftd::json_bool(*profile_json, "applyProfile")) {
            ftd::TermToggles staged = candidate->toggles;
            std::string profile_error;
            if (!apply_profile_fields(*profile_json, staged, profile_error)) {
                const std::string message = "invalid scenario profile: " + profile_error;
                std::cerr << "[ws_server] " << message << "\n";
                return send_json_response(client, json_error(message, operation), request_id);
            }
            if (!validate_profile_for_bridge(*candidate, staged, profile_error)) {
                const std::string message = "unsupported scenario profile: " + profile_error;
                std::cerr << "[ws_server] " << message << "\n";
                return send_json_response(client, json_error(message, operation), request_id);
            }
            candidate->toggles = staged;
        } else {
            std::string profile_error;
            if (!candidate->toggles.validate(&profile_error)) {
                const std::string message = "invalid native scenario profile: " + profile_error;
                std::cerr << "[ws_server] " << message << "\n";
                return send_json_response(client, json_error(message, operation), request_id);
            }
            if (!validate_profile_for_bridge(*candidate, candidate->toggles,
                                             profile_error)) {
                const std::string message = "unsupported native scenario profile: "
                                          + profile_error;
                std::cerr << "[ws_server] " << message << "\n";
                return send_json_response(client, json_error(message, operation), request_id);
            }
        }

        // Commit only after construction and scenario initialization both
        // succeed. Until this move, the old bridge remains fully operational.
        rb = std::move(candidate);
        lattice_size = budget.size;
        // A successful replacement is a new authoritative source generation.
        // Retire any old async observation before this cache can be served.
        telemetry.on_source_replaced(*rb);

        std::cout << "[ws_server] " << operation << " committed at L="
                  << lattice_size << "\n";
        if (!ftd::ws_send_text(client, json_progress(operation, "ready", lattice_size)))
            return false;
        return send_json_response(client,
            scenario ? json_profile_ack(*rb, telemetry, *scenario)
                     : json_profile_ack(*rb, telemetry), request_id);
    } catch (const std::exception& ex) {
        std::ostringstream ss;
        ss << operation << " failed at L=" << budget.size << ": " << ex.what();
        std::cerr << "[ws_server] " << ss.str() << "\n";
        return send_json_response(client, json_error(ss.str(), operation), request_id);
    } catch (...) {
        std::ostringstream ss;
        ss << operation << " failed at L=" << budget.size
           << ": unknown native exception";
        std::cerr << "[ws_server] " << ss.str() << "\n";
        return send_json_response(client, json_error(ss.str(), operation), request_id);
    }
}

// ============================================================================
//  Command dispatch
// ============================================================================

// Returns false if the client should be disconnected.
bool handle_command(const std::string& json, SOCKET client,
                    std::unique_ptr<ftd::RenderBridge>& rb,
                    ftd::NativeTelemetryScheduler& telemetry,
                    int& lattice_size)
{
    std::string cmd = ftd::json_string(json, "cmd");
    const std::uint64_t request_id = request_id_from(json);

    if (telemetry.suspended() && cmd != "info") {
        // Do not attempt reset/resize/setup_scenario here. A timed-out CUDA
        // observation may still own an event that RenderBridge destruction
        // synchronizes, so in-process source replacement is unsafe.
        return send_json_response(
            client, json_native_recovery_required(cmd, telemetry), request_id);
    }

    if (cmd == "tick") {
        rb->tick();
        // The server owns the observation boundary. This only enqueues a
        // non-blocking GPU snapshot; tick_complete is never held behind a
        // side-panel reduction or D2H wait.
        telemetry.on_tick_complete(*rb);
        std::ostringstream ss;
        ss << "{\"type\":\"tick_complete\",\"tick\":" << rb->current_tick() << "}";
        return ftd::ws_send_text(client, ss.str());
    }
    else if (cmd == "run") {
        const double raw_n = ftd::json_has_key(json, "n")
            ? ftd::json_number(json, "n") : 1.0;
        if (!std::isfinite(raw_n) || raw_n < 1.0
            || std::floor(raw_n) != raw_n) {
            return send_json_response(client, json_error(
                "run n must be a positive integer", cmd), request_id);
        }
        const int chunk_limit = interactive_run_chunk_limit(rb->lattice().size());
        if (raw_n > static_cast<double>(chunk_limit)) {
            std::ostringstream message;
            message << "run n=" << raw_n
                    << " exceeds interactive chunk limit " << chunk_limit
                    << " at L=" << rb->lattice().size()
                    << "; submit sequential tick/run chunks";
            return send_json_response(client, json_error(message.str(), cmd), request_id);
        }
        const int n = static_cast<int>(raw_n);
        rb->run(n);
        telemetry.on_tick_complete(*rb);
        std::ostringstream ss;
        ss << "{\"type\":\"run_complete\",\"tick\":" << rb->current_tick() << "}";
        return ftd::ws_send_text(client, ss.str());
    }
    else if (cmd == "get_particles") {
        if (telemetry.has_pending_or_due_observation()) {
            return send_json_response(
                client, json_visual_deferred("get_particles", telemetry), request_id);
        }
        auto data = pack_particle_data(*rb);
        return ftd::ws_send_binary(client, data);
    }
    else if (cmd == "get_diagnostics") {
        const auto view = telemetry.latest();
        const auto response = json_cached_legacy_group(
            view, ftd::TELEMETRY_DIAGNOSTICS);
        if (response.empty()) {
            return send_json_response(client, json_error(
                "diagnostics snapshot unavailable; configure telemetry demand and await telemetry_snapshot",
                cmd), request_id);
        }
        return send_json_response(client, response, request_id);
    }
    else if (cmd == "get_telemetry") {
        // Cache-only latest-snapshot read. Request booleans select returned
        // groups; they never change subscription demand or launch reductions.
        return send_json_response(client,
            json_telemetry_cached(telemetry.latest(),
                                  telemetry_selection_mask(json)),
            request_id);
    }
    else if (cmd == "set_telemetry_demand") {
        ftd::NativeTelemetryScheduler::Demand demand;
        std::string demand_error;
        if (!parse_telemetry_demand(json, telemetry, demand, demand_error)) {
            return send_json_response(client, json_error(demand_error, cmd), request_id);
        }
        telemetry.set_demand(demand);
        return send_json_response(client, json_telemetry_demand_ack(telemetry), request_id);
    }
    else if (cmd == "get_energy_audit") {
        const auto view = telemetry.latest();
        const auto response = json_cached_legacy_group(view, ftd::TELEMETRY_AUDIT);
        if (response.empty()) {
            return send_json_response(client, json_error(
                "energy-audit snapshot unavailable; configure telemetry demand and await telemetry_snapshot",
                cmd), request_id);
        }
        return send_json_response(client, response, request_id);
    }
    else if (cmd == "get_gravity_metric") {
        const auto view = telemetry.latest();
        const auto response = json_cached_legacy_group(view, ftd::TELEMETRY_GRAVITY);
        if (response.empty()) {
            return send_json_response(client, json_error(
                "gravity snapshot unavailable; configure telemetry demand and await telemetry_snapshot",
                cmd), request_id);
        }
        return send_json_response(client, response, request_id);
    }
    else if (cmd == "get_lagrangian") {
        const auto view = telemetry.latest();
        const auto response = json_cached_legacy_group(view, ftd::TELEMETRY_LAGRANGIAN);
        if (response.empty()) {
            return send_json_response(client, json_error(
                "lagrangian snapshot unavailable; configure telemetry demand and await telemetry_snapshot",
                cmd), request_id);
        }
        return send_json_response(client, response, request_id);
    }
    else if (cmd == "inspect_voxel") {
        const int x = static_cast<int>(ftd::json_number(json, "x"));
        const int y = static_cast<int>(ftd::json_number(json, "y"));
        const int z = static_cast<int>(ftd::json_number(json, "z"));
        return send_json_response(client, json_voxel(*rb, x, y, z), request_id);
    }
    else if (cmd == "get_force_at") {
        const int x = static_cast<int>(ftd::json_number(json, "x"));
        const int y = static_cast<int>(ftd::json_number(json, "y"));
        const int z = static_cast<int>(ftd::json_number(json, "z"));
        return send_json_response(client, json_force_at(*rb, x, y, z), request_id);
    }
    else if (cmd == "get_flux_slice") {
        if (telemetry.has_pending_or_due_observation()) {
            return send_json_response(
                client, json_visual_deferred("get_flux_slice", telemetry), request_id);
        }
        int axis = static_cast<int>(ftd::json_number(json, "axis"));
        int index = static_cast<int>(ftd::json_number(json, "index"));
        return ftd::ws_send_text(client, json_flux_slice(*rb, axis, index));
    }
    else if (cmd == "get_flux_volume") {
        if (telemetry.has_pending_or_due_observation()) {
            return send_json_response(
                client, json_visual_deferred("get_flux_volume", telemetry), request_id);
        }
        int axis_samples = static_cast<int>(ftd::json_number(json, "axisSamples"));
        if (axis_samples <= 0) axis_samples = 53;
        return ftd::ws_send_binary(client, pack_flux_volume(*rb, axis_samples));
    }
    else if (cmd == "get_field_sample" || cmd == "get_field_slices") {
        if (telemetry.has_pending_or_due_observation()) {
            return send_json_response(
                client, json_visual_deferred(cmd.c_str(), telemetry), request_id);
        }
        const std::string kind_name = ftd::json_string(json, "kind");
        ftd::VisualFieldKind kind{};
        if (!ftd::parse_visual_field_kind(kind_name, kind)) {
            return send_json_response(
                client, json_error("unknown field sample kind: " + kind_name, cmd), request_id);
        }
        int stride = static_cast<int>(ftd::json_number(json, "stride"));
        stride = std::clamp(stride, 1, 64);
        const double raw_token = ftd::json_number(json, "token");
        const std::uint32_t token = raw_token > 0.0
            ? static_cast<std::uint32_t>(raw_token) : 0u;
        // get_field_slices carries the center mid-plane index; the packer then
        // returns only the three orthogonal center planes (~axis× less traffic).
        const int planes_mid = (cmd == "get_field_slices")
            ? std::max(0, static_cast<int>(ftd::json_number(json, "mid")))
            : -1;
        return ftd::ws_send_binary(client, pack_field_sample(*rb, kind, stride, token, planes_mid));
    }
    else if (cmd == "set_toggle") {
        std::string name = ftd::json_string(json, "name");
        bool value = ftd::json_bool(json, "value");
        const auto* spec = find_toggle_spec(name);
        if (!spec) {
            std::cerr << "[TermToggles] Rejected unknown toggle: " << name << "\n";
            return send_json_response(
                client, json_error("unknown toggle: " + name, cmd), request_id);
        }
        ftd::TermToggles staged = rb->toggles;
        staged.*(spec->field) = value;
        std::string valid_error;
        if (!staged.validate(&valid_error)) {
            std::cerr << "[TermToggles] Rejected invalid update " << name
                      << '=' << (value ? "true" : "false") << ": "
                      << valid_error;
            return send_json_response(
                client, json_error("invalid toggle update: " + valid_error, cmd),
                request_id);
        }
        if (!validate_profile_for_bridge(*rb, staged, valid_error)) {
            std::cerr << "[TermToggles] Rejected unsupported update " << name
                      << '=' << (value ? "true" : "false") << ": "
                      << valid_error;
            return send_json_response(
                client, json_error("unsupported toggle update: " + valid_error, cmd),
                request_id);
        }
        rb->toggles = staged;
        telemetry.on_state_mutated(*rb);
        return true;  // Fire-and-forget
    }
    else if (cmd == "set_flux_boundary") {
        const int mode = static_cast<int>(ftd::json_number(json, "mode"));
        if (mode < 0 || mode > 2) {
            std::cerr << "[TermToggles] Rejected flux boundary mode " << mode << "\n";
            return send_json_response(
                client, json_error("invalid flux boundary mode", cmd), request_id);
        }
        ftd::TermToggles staged = rb->toggles;
        staged.flux_boundary = static_cast<ftd::FluxBoundaryMode>(mode);
        std::string valid_error;
        if (!staged.validate(&valid_error)
            || !validate_profile_for_bridge(*rb, staged, valid_error)) {
            std::cerr << "[TermToggles] Rejected flux boundary update: "
                      << valid_error;
            return send_json_response(
                client, json_error("invalid flux boundary update: " + valid_error, cmd),
                request_id);
        }
        rb->toggles = staged;
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "set_param") {
        std::string name = ftd::json_string(json, "name");
        double value = ftd::json_number(json, "value");
        if (!std::isfinite(value)) {
            std::cerr << "[ws_server] Rejected non-finite parameter " << name << "\n";
            return send_json_response(
                client, json_error("non-finite parameter", cmd), request_id);
        }
        bool changed = false;
        if (name == "dt") {
            rb->set_dt(value);
            changed = true;
        }
        else if (name == "omega0" && value >= 0.0) {
            rb->toggles.omega0 = value;
            changed = true;
        }
        else if (name == "langevin_T" && value >= 0.0) {
            rb->toggles.langevin_T = value;
            changed = true;
        }
        else if (name == "langevin_gamma" && value >= 0.0) {
            rb->toggles.langevin_gamma = value;
            changed = true;
        }
        else if (name == "langevin_seed" && value >= 0.0) {
            const auto seed = static_cast<unsigned int>(value);
            rb->toggles.langevin_seed = seed;
            rb->seed_rng(seed);
            changed = true;
        }
        else if (name == "coulomb_source_scale" && value > 0.0) {
            rb->toggles.coulomb_source_scale = value;
            changed = true;
        }
        else if (name == "kinetic_drain" && value >= 0.0 && value < 1.0) {
            rb->toggles.kinetic_drain = value;
            changed = true;
        }
        else if (name == "genesis_threshold" && value > 0.0) {
            rb->genesis_threshold_override = value;
            changed = true;
        }
        else if (name == "manifest_scale" && value > 0.0) {
            rb->manifest_scale_override = value;
            changed = true;
        }
        else {
            std::cerr << "[ws_server] Rejected unknown/out-of-range parameter "
                      << name << '=' << value << "\n";
            return send_json_response(
                client,
                json_error("unknown/out-of-range parameter: " + name, cmd),
                request_id);
        }
        if (changed) telemetry.on_state_mutated(*rb);
        return send_json_response(client, "{\"ok\":true,\"cmd\":\"set_param\"}", request_id);
    }
    else if (cmd == "inject_flux") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->inject_flux(x, y, z, {fx, fy, fz});
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "inject_flux_add") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->inject_flux_add(x, y, z, {fx, fy, fz});
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "inject_wave_vel_add") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double wx = ftd::json_number(json, "wx");
        double wy = ftd::json_number(json, "wy");
        double wz = ftd::json_number(json, "wz");
        rb->inject_wave_vel_add(x, y, z, {wx, wy, wz});
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "inject_particle") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        int8_t state = static_cast<int8_t>(ftd::json_number(json, "state"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->inject_particle(x, y, z, state, {fx, fy, fz});
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "inject_wavepacket") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        int8_t state = static_cast<int8_t>(ftd::json_number(json, "state"));
        rb->inject_wavepacket(x, y, z, state);
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "create_pair") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->create_entangled_pair(x, y, z, {fx, fy, fz});
        telemetry.on_state_mutated(*rb);
        return true;
    }
    else if (cmd == "resize") {
        int new_size = static_cast<int>(ftd::json_number(json, "size"));
        return replace_bridge_transactionally(
            client, rb, telemetry, lattice_size, new_size, "resize", std::nullopt,
            request_id);
    }
    else if (cmd == "resize_scenario") {
        int new_size = static_cast<int>(ftd::json_number(json, "size"));
        std::string name = ftd::json_string(json, "name");
        return replace_bridge_transactionally(
            client, rb, telemetry, lattice_size, new_size, "resize_scenario", name,
            request_id, &json);
    }
    else if (cmd == "preflight_resize") {
        int new_size = static_cast<int>(ftd::json_number(json, "size"));
        return send_json_response(
            client, json_budget(resource_budget(new_size)), request_id);
    }
    else if (cmd == "reset") {
        return replace_bridge_transactionally(
            client, rb, telemetry, lattice_size, lattice_size, "reset", std::nullopt,
            request_id);
    }
    else if (cmd == "setup_scenario") {
        std::string name = ftd::json_string(json, "name");
        return replace_bridge_transactionally(
            client, rb, telemetry, lattice_size, lattice_size, "setup_scenario", name,
            request_id, &json);
    }
    else if (cmd == "apply_profile") {
        ftd::TermToggles staged = rb->toggles;
        std::string profile_error;
        if (!apply_profile_fields(json, staged, profile_error)) {
            return send_json_response(
                client, json_error("invalid scenario profile: " + profile_error, cmd),
                request_id);
        }
        if (!validate_profile_for_bridge(*rb, staged, profile_error)) {
            return send_json_response(
                client, json_error("unsupported scenario profile: " + profile_error, cmd),
                request_id);
        }
        rb->toggles = staged;
        telemetry.on_state_mutated(*rb);
        return send_json_response(
            client, json_profile_ack(*rb, telemetry, ftd::json_string(json, "name")),
            request_id);
    }
    else if (cmd == "info") {
        return send_json_response(client, json_info(*rb, telemetry), request_id);
    }
    else {
        return send_json_response(
            client, json_error("unknown command: " + cmd, cmd), request_id);
    }
}

}  // namespace ftd::ws_server_detail
