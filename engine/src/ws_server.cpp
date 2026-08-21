/**
 * FTD WebSocket Server
 *
 * Standalone executable that bridges the FTD engine to the web dashboard
 * via WebSocket on port 9100.  Uses a minimal embedded WebSocket
 * implementation over raw winsock2 sockets -- no external dependencies.
 *
 * Build: link against ftd_core (and ftd_cuda when available).
 * Usage: ws_server.exe [lattice_size] [port] [--bind <addr>]
 *        Defaults: lattice_size=32, port=9100, bind=127.0.0.1 (loopback)
 *        LAN/remote control requires explicit opt-in: --bind 0.0.0.0
 *        (the protocol has no auth / no Origin check — see the runtime
 *        warning; revision 1.4 hardening, prior default was INADDR_ANY)
 *
 * Framing + handshake live in ws_protocol.{h,cpp}.  SHA-1 lives in ws_sha1.h.
 * This file is the command-dispatch loop and main().
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/ws_protocol.h"   // Pulls in SOCKET type + framing API
#include "ftd/scenarios.h"
#include "ftd/lagrangian.h"
#include "ftd/native_telemetry_scheduler.h"
#include "ftd/visual_field_sample.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cerrno>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef FTD_ENABLE_CUDA
#include <cuda_runtime_api.h>
#endif

namespace {

// SOCKET is declared at global scope in ws_protocol.h (platform-aligned)
// so no `using` is needed — just the WS opcode enum values.
using ftd::WS_TEXT;
using ftd::WS_BINARY;
using ftd::WS_CLOSE;
using ftd::WS_PING;
using ftd::WS_PONG;

constexpr int kMinLatticeSize = 4;
constexpr int kMaxLatticeSize = 256;
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
// Keep transport bounded to the renderer's fixed MAX_PARTICLES allocation.
// Sending more only fed JS diff maps and WebSocket buffers; Three.js discarded
// every particle above this limit.
constexpr std::size_t kMaxVisualParticles = 100000;

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

struct ResourceBudget {
    int size = 0;
    std::uint64_t voxels = 0;
    std::uint64_t host_required = 0;
    std::uint64_t host_available = 0;
    std::uint64_t gpu_required = 0;
    std::uint64_t gpu_available = 0;
    bool host_ok = false;
    bool gpu_ok = true;

    bool accepted() const { return host_ok && gpu_ok; }
};

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
//  Particle data extraction (matches WASM get_particle_data)
// ============================================================================

std::vector<uint8_t> pack_particle_data(ftd::RenderBridge& rb) {
    constexpr uint32_t kParticleFrameMagic = 0x32505446u; // LE bytes: F T P 2
    std::vector<std::int8_t> states;
    rb.copy_visual_states(states);
    const auto& read_rb = std::as_const(rb);
    const int N = read_rb.lattice().size();
    const int total = N * N * N;

    // Count visible voxels (strictly manifested particles state != 0). A dense
    // L=181 state can contain almost six million particles; sending all of them
    // would allocate a ~166 MiB frame and a second WebGL geometry of comparable
    // size. Systematic deterministic sampling keeps the visualization bounded
    // without changing the engine state or diagnostic counts.
    std::size_t manifested_count = 0;
    for (int i = 0; i < total; i++) {
        if (states[static_cast<std::size_t>(i)] != 0)
            ++manifested_count;
    }
    const std::size_t count = (std::min)(manifested_count, kMaxVisualParticles);

    std::vector<int> selected_indices;
    selected_indices.reserve(count);
    std::uint64_t select_accumulator = 0;
    for (int i = 0; i < total; ++i) {
        if (states[static_cast<std::size_t>(i)] == 0) continue;
        if (manifested_count > count) {
            select_accumulator += count;
            if (select_accumulator < manifested_count) continue;
            select_accumulator -= manifested_count;
        }
        selected_indices.push_back(i);
    }

    // Compact GPU gather: five floats per selected site (remainder xyz, spin,
    // color). This preserves sub-voxel motion without downloading the full AoS
    // voxel mirror, which is multi-gigabyte at native large-lattice sizes.
    std::vector<float> attributes;
    rb.copy_visual_particle_attributes(selected_indices, attributes);

    // FTP2 layout: [u32 magic][u32 count][pos 3N][color 3N][size N]
    //              [spin N][colorCharge N], all payload values float32.
    size_t header_bytes = 8;
    size_t pos_bytes    = count * 3 * sizeof(float);
    size_t col_bytes    = count * 3 * sizeof(float);
    size_t size_bytes   = count * sizeof(float);
    size_t spin_bytes   = count * sizeof(float);
    size_t charge_bytes = count * sizeof(float);
    size_t total_bytes  = header_bytes + pos_bytes + col_bytes + size_bytes
                        + spin_bytes + charge_bytes;

    std::vector<uint8_t> buf(total_bytes);
    auto* ptr = buf.data();

    std::memcpy(ptr, &kParticleFrameMagic, sizeof(kParticleFrameMagic));
    ptr += sizeof(kParticleFrameMagic);
    uint32_t cnt = static_cast<uint32_t>(count);
    std::memcpy(ptr, &cnt, 4);
    ptr += 4;

    float* positions = reinterpret_cast<float*>(ptr);
    float* colors    = reinterpret_cast<float*>(ptr + pos_bytes);
    float* sizes     = reinterpret_cast<float*>(ptr + pos_bytes + col_bytes);
    float* spins     = reinterpret_cast<float*>(ptr + pos_bytes + col_bytes + size_bytes);
    float* charges   = reinterpret_cast<float*>(ptr + pos_bytes + col_bytes + size_bytes + spin_bytes);

    for (std::size_t idx = 0; idx < selected_indices.size(); ++idx) {
        const int i = selected_indices[idx];
        const std::int8_t state = states[static_cast<std::size_t>(i)];
        const auto c = read_rb.lattice().coord(i);
        const std::size_t a = idx * 5u;

        positions[idx * 3]     = static_cast<float>(c.x) + 0.5f + attributes[a + 0u];
        positions[idx * 3 + 1] = static_cast<float>(c.y) + 0.5f + attributes[a + 1u];
        positions[idx * 3 + 2] = static_cast<float>(c.z) + 0.5f + attributes[a + 2u];

        // Color by state
        if (state == 1) {
            // Green (positive)
            colors[idx * 3]     = 0.29f;
            colors[idx * 3 + 1] = 0.87f;
            colors[idx * 3 + 2] = 0.50f;
        } else { // state == -1
            // Red (negative)
            colors[idx * 3]     = 0.97f;
            colors[idx * 3 + 1] = 0.44f;
            colors[idx * 3 + 2] = 0.44f;
        }

        // Size matches WASM particle size
        sizes[idx] = 6.0f;
        spins[idx] = attributes[a + 3u];
        charges[idx] = attributes[a + 4u];
    }

    return buf;
}

// Compact sampled visualization frame:
// ["FTV2"][u32 latticeSize][u32 effectiveStride][u32 axisCount]
// [float32 density[axisCount^3]], x-fastest.
//
// The renderer draws at most 53^3 points. Sending the old dense FTV1 N^3
// lattice therefore downloaded and transferred ~23.7 MiB on every L=181
// refresh only to discard 97.5% of it in JavaScript. Reuse the sparse compact
// GPU field sampler and materialize only its bounded regular grid here.
std::vector<uint8_t> pack_flux_volume(ftd::RenderBridge& rb,
                                      int requested_axis_samples) {
    constexpr uint32_t kFluxVolumeMagic = 0x32565446u; // LE bytes: F T V 2
    const auto& read_rb = std::as_const(rb);
    const int n = read_rb.lattice().size();
    const int target_axis = std::clamp(requested_axis_samples, 1, 64);
    const int requested_stride = std::max(1, (n + target_axis - 1) / target_axis);
    ftd::VisualFieldSample sample;
    rb.copy_visual_field_sample(
        ftd::VisualFieldKind::FluxVector, requested_stride, sample);

    const int stride = std::max(1, sample.effective_stride);
    const int axis_count = (n + stride - 1) / stride;
    const std::size_t count = static_cast<std::size_t>(axis_count)
                            * axis_count * axis_count;
    std::vector<uint8_t> buf(16u + count * sizeof(float), 0u);
    const uint32_t header[4] = {
        kFluxVolumeMagic,
        static_cast<uint32_t>(n),
        static_cast<uint32_t>(stride),
        static_cast<uint32_t>(axis_count),
    };
    std::memcpy(buf.data(), header, sizeof(header));
    float* density = reinterpret_cast<float*>(buf.data() + sizeof(header));

    const std::size_t compact_count = sample.count();
    if (sample.components != 3u || sample.positions.size() != compact_count * 3u
        || sample.data.size() != compact_count * 3u) {
        throw std::runtime_error("invalid compact flux-vector sample layout");
    }
    for (std::size_t i = 0; i < compact_count; ++i) {
        const int x = static_cast<int>(sample.positions[i * 3u + 0u]);
        const int y = static_cast<int>(sample.positions[i * 3u + 1u]);
        const int z = static_cast<int>(sample.positions[i * 3u + 2u]);
        const int xi = x / stride;
        const int yi = y / stride;
        const int zi = z / stride;
        if (xi < 0 || yi < 0 || zi < 0
            || xi >= axis_count || yi >= axis_count || zi >= axis_count) {
            continue;
        }
        const float jx = sample.data[i * 3u + 0u];
        const float jy = sample.data[i * 3u + 1u];
        const float jz = sample.data[i * 3u + 2u];
        const std::size_t q = (static_cast<std::size_t>(zi) * axis_count + yi)
                            * axis_count + xi;
        density[q] = std::sqrt(jx * jx + jy * jy + jz * jz);
    }
    return buf;
}

// FTS2 sampled-field frame. The server always sends the effective compacted
// sample (not the dense candidate grid), so large lattices stay bounded in
// both PCIe and WebSocket traffic. Origin + effective stride describe the
// represented regular grid independently of sparse zero-value omission; the
// dashboard needs them to select the correct physical slice plane.
std::vector<uint8_t> pack_field_sample(ftd::RenderBridge& rb,
                                       ftd::VisualFieldKind kind,
                                       int stride,
                                       std::uint32_t token) {
    constexpr std::uint32_t kFieldSampleMagic = 0x32535446u; // F T S 2
    ftd::VisualFieldSample sample;
    rb.copy_visual_field_sample(kind, stride, sample);
    const std::uint32_t kind_code = static_cast<std::uint32_t>(kind);
    const std::uint32_t components = sample.components;
    const std::uint32_t count = static_cast<std::uint32_t>(sample.count());
    const std::size_t header_bytes = 7u * sizeof(std::uint32_t);
    const std::size_t position_bytes = static_cast<std::size_t>(count) * 3u * sizeof(float);
    const std::size_t data_bytes = static_cast<std::size_t>(count) * components * sizeof(float);
    std::vector<std::uint8_t> frame(header_bytes + position_bytes + data_bytes);
    std::uint32_t header[7] = {
        kFieldSampleMagic, token, kind_code, components, count,
        static_cast<std::uint32_t>(std::max(1, sample.effective_stride)),
        static_cast<std::uint32_t>(std::max(0, sample.origin)),
    };
    std::memcpy(frame.data(), header, header_bytes);
    if (position_bytes != 0)
        std::memcpy(frame.data() + header_bytes, sample.positions.data(), position_bytes);
    if (data_bytes != 0)
        std::memcpy(frame.data() + header_bytes + position_bytes,
                    sample.data.data(), data_bytes);
    return frame;
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

const char* telemetry_group_name(std::size_t index) {
    switch (index) {
    case 0: return "diagnostics";
    case 1: return "audit";
    case 2: return "gravity";
    default: return "lagrangian";
    }
}

constexpr std::array<std::uint32_t, 4> kTelemetryGroupBits{{
    ftd::TELEMETRY_DIAGNOSTICS,
    ftd::TELEMETRY_AUDIT,
    ftd::TELEMETRY_GRAVITY,
    ftd::TELEMETRY_LAGRANGIAN,
}};

const ftd::TelemetryGroupMeta& telemetry_group_meta(
    const ftd::TelemetrySnapshot& snapshot, std::size_t index) {
    switch (index) {
    case 0: return snapshot.diagnostics_meta;
    case 1: return snapshot.audit_meta;
    case 2: return snapshot.gravity_meta;
    default: return snapshot.lagrangian_meta;
    }
}

// All serializers below take immutable publisher values.  In particular, no
// function in this section accepts RenderBridge: `get_telemetry` and the
// legacy scalar commands must never turn a panel refresh into a CUDA reduction.
std::string json_diagnostics_value(
    const ftd::Diagnostics& d,
    const ftd::TelemetryGroupMeta& meta) {
    std::ostringstream ss;
    ss << std::setprecision(10);
    ss << "{";
    ss << "\"tick\":"          << d.tick;
    ss << ",\"physicalTime\":" << meta.physical_time;
    ss << ",\"dt\":"           << meta.dt;
    ss << ",\"manifested\":"   << d.manifested_count;
    ss << ",\"positive\":"     << d.positive_count;
    ss << ",\"negative\":"     << d.negative_count;
    ss << ",\"totalFlux\":"    << d.total_flux;
    ss << ",\"totalEnergy\":"  << d.total_energy;
    ss << ",\"maxBandwidth\":" << d.max_bandwidth;
    ss << ",\"maxCausalBudget\":" << d.max_causal_budget;
    ss << ",\"causalProjectionEvents\":" << d.causal_projection_events;
    ss << ",\"avgDrag\":"      << d.avg_drag;
    ss << ",\"entropy\":"      << d.total_entropy;
    ss << ",\"chargeBalance\":" << (d.positive_count - d.negative_count);
    ss << ",\"spinUp\":"       << d.spin_up_count;
    ss << ",\"spinDown\":"     << d.spin_down_count;
    ss << ",\"colorless\":"    << d.color_count[0];
    ss << ",\"colorRed\":"     << d.color_count[1];
    ss << ",\"colorGreen\":"   << d.color_count[2];
    ss << ",\"colorBlue\":"    << d.color_count[3];
    ss << ",\"angMomX\":"      << d.total_angular_momentum.x;
    ss << ",\"angMomY\":"      << d.total_angular_momentum.y;
    ss << ",\"angMomZ\":"      << d.total_angular_momentum.z;
    ss << "}";
    return ss.str();
}

std::string json_energy_audit_value(const ftd::EnergyAudit& ea) {
    std::ostringstream ss;
    ss << std::setprecision(10);
    ss << "{";
    // NAMING (see diagnostics_compute.cpp): fieldEnergy is flux POTENTIAL energy
    // ½Σ|J|² (NOT E-field energy); EFieldEnergy below is byte-identical to
    // waveEnergy by construction (E = -wave_vel), and BFieldEnergy carries the
    // (c²/2) weight. Do not read "fieldEnergy vs BFieldEnergy" as "|E|² vs |B|²".
    ss << "\"fieldEnergy\":"        << ea.field_energy;
    ss << ",\"waveEnergy\":"        << ea.wave_energy;
    ss << ",\"particleKE\":"        << ea.particle_ke;
    ss << ",\"totalEnergy\":"       << ea.total_energy;
    ss << ",\"gaussViolation\":"    << ea.gauss_violation;
    ss << ",\"maxGaussError\":"     << ea.max_gauss_error;
    ss << ",\"selfFieldInjection\":" << ea.self_field_injection;
    ss << ",\"coulombPE\":"         << ea.coulomb_pe;
    ss << ",\"EFieldEnergy\":"      << ea.E_field_energy;
    ss << ",\"BFieldEnergy\":"      << ea.B_field_energy;
    ss << ",\"chargeTotal\":"       << ea.charge_total;
    ss << ",\"manifested\":"        << ea.manifested_count;
    ss << ",\"particleRestEnergy\":" << ea.particle_rest_energy;
    ss << ",\"particleEnergy\":"     << ea.particle_energy;
    ss << ",\"dynamicEnergy\":"      << ea.dynamic_energy;
    ss << ",\"particleMomentumX\":"  << ea.particle_momentum.x;
    ss << ",\"particleMomentumY\":"  << ea.particle_momentum.y;
    ss << ",\"particleMomentumZ\":"  << ea.particle_momentum.z;
    ss << ",\"poyntingX\":"         << ea.total_poynting.x;
    ss << ",\"poyntingY\":"         << ea.total_poynting.y;
    ss << ",\"poyntingZ\":"         << ea.total_poynting.z;
    ss << ",\"ELTotal\":"           << ea.E_L_total;
    ss << ",\"ERTotal\":"           << ea.E_R_total;
    ss << ",\"waveLTotal\":"        << ea.wv_L_total;
    ss << ",\"waveRTotal\":"        << ea.wv_R_total;
    ss << ",\"chiralityTotal\":"    << ea.chirality_total;
    ss << ",\"strongEnergy\":"      << ea.strong_energy;
    ss << ",\"weakEnergy\":"        << ea.weak_energy;
    ss << ",\"cellVolume\":"        << ea.cell_volume;
    ss << ",\"fieldEnergyDensitySum\":" << ea.field_energy_density_sum;
    ss << ",\"waveEnergyDensitySum\":" << ea.wave_energy_density_sum;
    ss << ",\"strongPotentialEnergy\":" << ea.strong_potential_energy;
    ss << ",\"strongGravitationalMass\":" << ea.strong_gravitational_mass;
    ss << ",\"strongProjectionResidual\":" << ea.strong_projection_residual;
    ss << ",\"strongProjectionLambda\":" << ea.strong_projection_lambda;
    ss << ",\"strongProjectionEvents\":" << ea.strong_projection_events;
    ss << ",\"strongProjectionFailures\":" << ea.strong_projection_failures;
    ss << ",\"strongTopologyFailures\":" << ea.strong_topology_failures;
    ss << "}";
    return ss.str();
}

std::string json_gravity_metric_value(const ftd::GravityMetricAgg& a) {
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"active\":" << (a.active ? "true" : "false")
       << ",\"requested\":" << (a.requested ? "true" : "false")
       << ",\"latencyMax\":" << a.latency_max
       << ",\"latencyMean\":" << a.latency_mean
       << ",\"fMin\":" << a.f_min
       << ",\"gammaMax\":" << a.gamma_max
       << ",\"dilationMaxPct\":" << a.dilation_max_pct
       << ",\"voxelCount\":" << a.voxel_count << "}";
    return ss.str();
}

std::string json_lagrangian_value(const ftd::TelemetryLagrangian& lag) {
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"fieldKinetic\":" << lag.field_kinetic_sum
       << ",\"fieldGradient\":" << lag.field_gradient_sum
       << ",\"bornInfeld\":" << lag.born_infeld_sum
       << ",\"coupling\":" << lag.coupling_sum
       << ",\"velocity\":" << lag.velocity_coupling_sum
       << ",\"gauss\":" << lag.gauss_sum
       << ",\"dissipation\":" << lag.dissipation_sum
       << ",\"total\":" << lag.total_lagrangian
       << ",\"hamiltonian\":" << lag.total_hamiltonian
       << ",\"totalAction\":" << lag.total_action
       << ",\"gaussViolation\":" << lag.gauss_violation
       << ",\"maxGaussError\":" << lag.max_gauss_error
       << ",\"totalFluxMag\":" << lag.total_flux_mag
       << ",\"totalWaveEnergy\":" << lag.total_wave_energy
       << ",\"manifested\":" << lag.manifested_count
       << ",\"locked\":" << lag.locked_count
       << ",\"cellVolume\":" << lag.cell_volume << "}";
    return ss.str();
}

void append_telemetry_groups(
    std::ostringstream& ss,
    const ftd::TelemetrySnapshot& snapshot,
    std::uint32_t mask) {
    bool first = true;
    const auto append = [&](std::size_t index, const std::string& value) {
        if (!first) ss << ',';
        first = false;
        ss << '"' << telemetry_group_name(index) << "\":" << value;
    };
    if (mask & ftd::TELEMETRY_DIAGNOSTICS)
        append(0, json_diagnostics_value(snapshot.diagnostics,
                                         snapshot.diagnostics_meta));
    if (mask & ftd::TELEMETRY_AUDIT)
        append(1, json_energy_audit_value(snapshot.audit));
    if (mask & ftd::TELEMETRY_GRAVITY)
        append(2, json_gravity_metric_value(snapshot.gravity));
    if (mask & ftd::TELEMETRY_LAGRANGIAN)
        append(3, json_lagrangian_value(snapshot.lagrangian));
}

void append_telemetry_group_meta(
    std::ostringstream& ss,
    const ftd::TelemetrySnapshot& snapshot,
    const std::array<std::uint64_t, 4>& group_snapshot_versions,
    std::uint32_t mask,
    std::uint64_t current_epoch) {
    bool first = true;
    for (std::size_t index = 0; index < kTelemetryGroupBits.size(); ++index) {
        const std::uint32_t bit = kTelemetryGroupBits[index];
        if ((mask & bit) == 0) continue;
        const auto& meta = telemetry_group_meta(snapshot, index);
        if (!first) ss << ',';
        first = false;
        ss << '"' << telemetry_group_name(index) << "\":{"
           << "\"epoch\":" << meta.epoch
           // `epoch` is the cross-backend mutation/version contract. The
           // GPU additionally exposes a device state counter; CPU's internal
           // count is merely a snapshot sequence, so keep it diagnostic-only.
           << ",\"stateVersion\":" << meta.epoch
           << ",\"backendStateVersion\":" << meta.state_version
           << ",\"tick\":" << meta.tick
           << ",\"snapshotVersion\":" << group_snapshot_versions[index]
           << ",\"stale\":" << (meta.epoch == current_epoch ? "false" : "true")
           << ",\"physicalTime\":" << std::setprecision(17)
           << meta.physical_time
           << ",\"dt\":" << meta.dt
           << ",\"latticeSize\":" << meta.lattice_size
           << '}';
    }
}

std::string json_telemetry_envelope(
    const char* type,
    std::uint64_t snapshot_version,
    std::uint64_t source_epoch,
    std::uint64_t epoch,
    int tick,
    std::uint32_t available_mask,
    std::uint32_t fresh_mask,
    std::uint32_t pending_mask,
    std::uint32_t payload_mask,
    const ftd::TelemetrySnapshot& snapshot,
    const std::array<std::uint64_t, 4>& group_snapshot_versions,
    const std::array<std::uint32_t, 4>& min_interval_ms,
    std::optional<std::uint32_t> requested_mask = std::nullopt,
    std::optional<std::uint32_t> published_mask = std::nullopt) {
    std::ostringstream ss;
    ss << "{\"type\":\"" << type << "\""
       << ",\"snapshotVersion\":" << snapshot_version
       << ",\"sourceEpoch\":" << source_epoch
       << ",\"epoch\":" << epoch
       << ",\"tick\":" << tick
       << ",\"availableMask\":" << available_mask
       << ",\"freshMask\":" << fresh_mask
       << ",\"pendingMask\":" << pending_mask
       // This is the native producer's minimum wall-clock spacing for each
       // full-grid group.  It is a backpressure policy, not a browser-panel
       // refresh rate, and makes a delayed group explainable to the UI.
       << ",\"minIntervalMs\":{\"diagnostics\":" << min_interval_ms[0]
       << ",\"audit\":" << min_interval_ms[1]
       << ",\"gravity\":" << min_interval_ms[2]
       << ",\"lagrangian\":" << min_interval_ms[3] << '}';
    if (requested_mask) ss << ",\"requestedMask\":" << *requested_mask;
    if (published_mask) ss << ",\"publishedMask\":" << *published_mask;
    ss << ",\"groups\":{";
    append_telemetry_groups(ss, snapshot, payload_mask);
    ss << "},\"groupMeta\":{";
    append_telemetry_group_meta(ss, snapshot, group_snapshot_versions,
                                payload_mask, epoch);
    ss << '}';
    ss << "}";
    return ss.str();
}

std::string json_telemetry_cached(
    const ftd::NativeTelemetryScheduler::CachedView& view,
    std::uint32_t requested_mask) {
    const std::uint32_t payload_mask = requested_mask & view.available_mask;
    return json_telemetry_envelope(
        "telemetry", view.snapshot_version, view.source_epoch, view.epoch, view.tick,
        view.available_mask, view.fresh_mask, view.pending_mask, payload_mask,
        view.snapshot, view.group_snapshot_versions, view.min_interval_ms,
        requested_mask, std::nullopt);
}

std::string json_telemetry_publication(
    const ftd::NativeTelemetryScheduler::Publication& publication) {
    return json_telemetry_envelope(
        "telemetry_snapshot", publication.snapshot_version,
        publication.source_epoch, publication.epoch, publication.tick,
        publication.available_mask,
        publication.fresh_mask, publication.pending_mask,
        publication.published_mask, publication.snapshot,
        publication.group_snapshot_versions, publication.min_interval_ms,
        std::nullopt, publication.published_mask);
}

std::string json_telemetry_invalidation(
    const ftd::NativeTelemetryScheduler::Invalidation& invalidation) {
    // This is deliberately separate from telemetry_snapshot: it changes
    // freshness only. In particular, snapshotVersion remains the last
    // completed full-grid publication and must not be mistaken for a sample
    // of the newly-mutated state.
    std::ostringstream ss;
    ss << "{\"type\":\"telemetry_invalidated\""
       << ",\"sourceEpoch\":" << invalidation.source_epoch
       << ",\"epoch\":" << invalidation.epoch
       << ",\"tick\":" << invalidation.tick
       << ",\"snapshotVersion\":" << invalidation.snapshot_version
       << ",\"availableMask\":" << invalidation.available_mask
       << ",\"freshMask\":0"
       << ",\"pendingMask\":" << invalidation.pending_mask
       << ",\"reason\":\"" << json_escape(invalidation.reason) << "\"}";
    return ss.str();
}

std::string json_cached_legacy_group(
    const ftd::NativeTelemetryScheduler::CachedView& view,
    std::uint32_t bit) {
    if ((view.available_mask & bit) == 0) return {};
    if (bit == ftd::TELEMETRY_DIAGNOSTICS)
        return json_diagnostics_value(view.snapshot.diagnostics,
                                      view.snapshot.diagnostics_meta);
    if (bit == ftd::TELEMETRY_AUDIT)
        return json_energy_audit_value(view.snapshot.audit);
    if (bit == ftd::TELEMETRY_GRAVITY)
        return json_gravity_metric_value(view.snapshot.gravity);
    return json_lagrangian_value(view.snapshot.lagrangian);
}

std::string json_voxel(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto& read_rb = std::as_const(rb);
    const auto sample = read_rb.inspect_voxel(x, y, z);
    const auto& v = sample.voxel;
    const double div = sample.divergence;
    const auto& curl = sample.curl;
    const auto& em = sample.em;
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"x\":" << read_rb.lattice().wrap(x)
       << ",\"y\":" << read_rb.lattice().wrap(y)
       << ",\"z\":" << read_rb.lattice().wrap(z)
       << ",\"state\":" << static_cast<int>(v.state)
       << ",\"particleId\":" << v.particle_id
       << ",\"pairId\":" << v.pair_id
       << ",\"locked\":" << (v.locked ? "true" : "false")
       << ",\"spin\":" << static_cast<int>(v.spin)
       << ",\"color\":" << static_cast<int>(v.color)
       << ",\"fluxX\":" << v.flux.x << ",\"fluxY\":" << v.flux.y
       << ",\"fluxZ\":" << v.flux.z << ",\"density\":" << v.density()
       << ",\"phase\":" << v.phase << ",\"tau\":" << v.tau
       << ",\"latency\":" << v.latency
       << ",\"waveVelX\":" << v.wave_vel.x << ",\"waveVelY\":" << v.wave_vel.y
       << ",\"waveVelZ\":" << v.wave_vel.z
       << ",\"velX\":" << v.velocity.x << ",\"velY\":" << v.velocity.y
       << ",\"velZ\":" << v.velocity.z << ",\"speed\":" << v.speed()
       << ",\"accelMag\":" << v.accel_mag
       << ",\"divJ\":" << div << ",\"curlX\":" << curl.x
       << ",\"curlY\":" << curl.y << ",\"curlZ\":" << curl.z
       << ",\"Ex\":" << em.E.x << ",\"Ey\":" << em.E.y
       << ",\"Ez\":" << em.E.z << ",\"Emag\":" << em.E_mag
       << ",\"Bx\":" << em.B.x << ",\"By\":" << em.B.y
       << ",\"Bz\":" << em.B.z << ",\"Bmag\":" << em.B_mag << "}";
    return ss.str();
}

std::string json_force_at(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto fd = std::as_const(rb).inspect_force(x, y, z);
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"x\":" << rb.lattice().wrap(x)
       << ",\"y\":" << rb.lattice().wrap(y)
       << ",\"z\":" << rb.lattice().wrap(z)
       << ",\"coulombX\":" << fd.f_coulomb.x
       << ",\"coulombY\":" << fd.f_coulomb.y
       << ",\"coulombZ\":" << fd.f_coulomb.z
       << ",\"coulombMag\":" << fd.f_coulomb.mag()
       << ",\"strongX\":" << fd.f_strong.x
       << ",\"strongY\":" << fd.f_strong.y
       << ",\"strongZ\":" << fd.f_strong.z
       << ",\"strongMag\":" << fd.f_strong.mag()
       << ",\"magneticX\":" << fd.f_magnetic.x
       << ",\"magneticY\":" << fd.f_magnetic.y
       << ",\"magneticZ\":" << fd.f_magnetic.z
       << ",\"magneticMag\":" << fd.f_magnetic.mag()
       << ",\"gravityX\":" << fd.f_gravity.x
       << ",\"gravityY\":" << fd.f_gravity.y
       << ",\"gravityZ\":" << fd.f_gravity.z
       << ",\"gravityMag\":" << fd.f_gravity.mag()
       << ",\"exchangeX\":" << fd.f_exchange.x
       << ",\"exchangeY\":" << fd.f_exchange.y
       << ",\"exchangeZ\":" << fd.f_exchange.z
       << ",\"exchangeMag\":" << fd.f_exchange.mag() << "}";
    return ss.str();
}

std::string json_info(ftd::RenderBridge& rb,
                      const ftd::NativeTelemetryScheduler& telemetry) {
    const bool gpu_active = rb.backend_kind() == ftd::Backend::Kind::Gpu;
    const ResourceBudget current_budget = resource_budget(rb.lattice().size());
    std::ostringstream ss;
    ss << "{";
    ss << "\"latticeSize\":" << rb.lattice().size();
    ss << ",\"tick\":"       << rb.current_tick();
    ss << ",\"gpu\":"        << (gpu_active ? "true" : "false");
    ss << ",\"backend\":\"" << (gpu_active ? "cuda" : "cpu") << "\"";
    ss << ",\"version\":\"" << ftd::ENGINE_VERSION << "\"";
    ss << ",\"maxLatticeSize\":" << kMaxLatticeSize;
    ss << ",\"availableHostBytes\":" << current_budget.host_available;
    ss << ",\"availableGpuBytes\":" << current_budget.gpu_available;
    ss << ",\"interactiveGpuMode\":"
       << (rb.interactive_gpu_mode() ? "true" : "false");
    ss << ",\"maxVisualParticles\":" << kMaxVisualParticles;
    // Protocol v2 turns telemetry into a native publisher/cache rather than
    // an RPC that performs a fresh reduction for every side panel.
    ss << ",\"telemetryProtocolVersion\":2"
       << ",\"telemetryPush\":true"
       << ",\"telemetryRecoveryRequired\":"
       << (telemetry.suspended() ? "true" : "false")
       << ",\"restartRequired\":"
       << (telemetry.restart_required() ? "true" : "false")
       << ",\"sourceEpoch\":" << telemetry.source_epoch()
       << ",\"telemetrySourceEpoch\":" << telemetry.source_epoch()
       << ",\"telemetryEpoch\":" << telemetry.epoch()
       << ",\"telemetrySnapshotVersion\":" << telemetry.snapshot_version();
    if (telemetry.suspended()) {
        ss << ",\"telemetryRecoveryReason\":\""
           << json_escape(telemetry.suspension_reason()) << "\"";
    }
    ss << "}";
    return ss.str();
}

std::string json_visual_deferred(
    const char* operation,
    const ftd::NativeTelemetryScheduler& telemetry) {
    const auto view = telemetry.latest();
    std::ostringstream ss;
    ss << "{\"type\":\"visual_deferred\""
       << ",\"operation\":\"" << operation << "\""
       << ",\"reason\":\"telemetry_priority\""
       << ",\"sourceEpoch\":" << view.source_epoch
       << ",\"epoch\":" << view.epoch
       << ",\"tick\":" << view.tick
       << ",\"snapshotVersion\":" << view.snapshot_version
       << ",\"pendingMask\":" << view.pending_mask
       // Hint only: a GPU fence can take longer; the normal retry trigger is
       // the next telemetry_snapshot publication.
       << ",\"retryAfterMs\":16}";
    return ss.str();
}

// --------------------------------------------------------------------------
// Telemetry control-plane JSON helpers
// --------------------------------------------------------------------------
// ws_protocol intentionally provides only flat string-search helpers for the
// legacy command surface. The demand command has one small nested object
// (`everyTicks`), so parse only that controlled top-level shape here rather
// than adding a general JSON dependency to the standalone native server.

bool json_top_level_value_start(const std::string& json, const char* key,
                                std::size_t& value_start) {
    int depth = 0;
    for (std::size_t i = 0; i < json.size(); ++i) {
        const char c = json[i];
        if (c == '{' || c == '[') {
            ++depth;
            continue;
        }
        if (c == '}' || c == ']') {
            --depth;
            continue;
        }
        if (c != '"') continue;

        const std::size_t begin = i + 1;
        std::size_t end = begin;
        for (; end < json.size(); ++end) {
            if (json[end] == '\\') {
                ++end;
                continue;
            }
            if (json[end] == '"') break;
        }
        if (end >= json.size()) return false;
        if (depth == 1 && json.compare(begin, end - begin, key) == 0) {
            std::size_t cursor = end + 1;
            while (cursor < json.size()
                   && std::isspace(static_cast<unsigned char>(json[cursor]))) {
                ++cursor;
            }
            if (cursor < json.size() && json[cursor] == ':') {
                ++cursor;
                while (cursor < json.size()
                       && std::isspace(static_cast<unsigned char>(json[cursor]))) {
                    ++cursor;
                }
                value_start = cursor;
                return cursor < json.size();
            }
        }
        i = end;
    }
    return false;
}

std::optional<bool> json_top_level_bool(const std::string& json,
                                        const char* key) {
    std::size_t start = 0;
    if (!json_top_level_value_start(json, key, start)) return std::nullopt;
    if (json.compare(start, 4, "true") == 0) return true;
    if (json.compare(start, 5, "false") == 0) return false;
    return std::nullopt;
}

std::optional<double> json_top_level_number(const std::string& json,
                                            const char* key) {
    std::size_t start = 0;
    if (!json_top_level_value_start(json, key, start)) return std::nullopt;
    const char* first = json.c_str() + start;
    char* end = nullptr;
    const double value = std::strtod(first, &end);
    if (end == first || !std::isfinite(value)) return std::nullopt;
    return value;
}

std::optional<std::string> json_top_level_object(const std::string& json,
                                                  const char* key) {
    std::size_t start = 0;
    if (!json_top_level_value_start(json, key, start)
        || start >= json.size() || json[start] != '{') return std::nullopt;

    int depth = 0;
    bool in_string = false;
    for (std::size_t i = start; i < json.size(); ++i) {
        const char c = json[i];
        if (in_string) {
            if (c == '\\') {
                ++i;
            } else if (c == '"') {
                in_string = false;
            }
            continue;
        }
        if (c == '"') {
            in_string = true;
        } else if (c == '{') {
            ++depth;
        } else if (c == '}') {
            if (--depth == 0) return json.substr(start, i - start + 1);
        }
    }
    return std::nullopt;
}

std::uint32_t telemetry_selection_mask(const std::string& json) {
    std::uint32_t mask = 0;
    bool explicit_selection = false;
    const std::array<const char*, 4> names{{
        "diagnostics", "audit", "gravity", "lagrangian",
    }};
    for (std::size_t i = 0; i < names.size(); ++i) {
        const auto value = json_top_level_bool(json, names[i]);
        if (!value) continue;
        explicit_selection = true;
        if (*value) mask |= kTelemetryGroupBits[i];
    }
    // Preserve the original endpoint's convenient summary default. The
    // distinction is only response selection; no call here changes demand.
    return explicit_selection ? mask : ftd::TELEMETRY_DIAGNOSTICS;
}

bool parse_telemetry_demand(const std::string& json,
                            const ftd::NativeTelemetryScheduler& scheduler,
                            ftd::NativeTelemetryScheduler::Demand& out,
                            std::string& error) {
    out = scheduler.demand();

    if (const auto raw_mask = json_top_level_number(json, "mask")) {
        if (*raw_mask < 0.0
            || *raw_mask > static_cast<double>(ftd::TELEMETRY_ALL)
            || std::floor(*raw_mask) != *raw_mask) {
            error = "telemetry mask must be within the supported group bits";
            return false;
        }
        out.enabled_mask = static_cast<std::uint32_t>(*raw_mask)
                         & ftd::TELEMETRY_ALL;
    }

    const std::array<const char*, 4> names{{
        "diagnostics", "audit", "gravity", "lagrangian",
    }};
    for (std::size_t i = 0; i < names.size(); ++i) {
        const auto value = json_top_level_bool(json, names[i]);
        if (!value) continue;
        if (*value) out.enabled_mask |= kTelemetryGroupBits[i];
        else        out.enabled_mask &= ~kTelemetryGroupBits[i];
    }

    if (const auto cadence = json_top_level_object(json, "everyTicks")) {
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (!ftd::json_has_key(*cadence, names[i])) continue;
            const double raw = ftd::json_number(*cadence, names[i]);
            if (!std::isfinite(raw) || raw < 1.0
                || raw > static_cast<double>(
                    ftd::NativeTelemetryScheduler::kMaxCadenceTicks)
                || std::floor(raw) != raw) {
                error = std::string("everyTicks.") + names[i]
                      + " must be an integer in [1,65535]";
                return false;
            }
            out.every_ticks[i] = static_cast<std::uint32_t>(raw);
        }
    }

    return true;
}

std::string json_telemetry_demand_ack(
    const ftd::NativeTelemetryScheduler& scheduler) {
    const auto& demand = scheduler.demand();
    const auto view = scheduler.latest();
    std::ostringstream ss;
    ss << "{\"type\":\"telemetry_demand\""
       << ",\"enabledMask\":" << demand.enabled_mask
       << ",\"everyTicks\":{"
       << "\"diagnostics\":" << demand.every_ticks[0]
       << ",\"audit\":" << demand.every_ticks[1]
       << ",\"gravity\":" << demand.every_ticks[2]
       << ",\"lagrangian\":" << demand.every_ticks[3]
       << "},\"minIntervalMs\":{"
       << "\"diagnostics\":" << view.min_interval_ms[0]
       << ",\"audit\":" << view.min_interval_ms[1]
       << ",\"gravity\":" << view.min_interval_ms[2]
       << ",\"lagrangian\":" << view.min_interval_ms[3]
       << "},\"snapshotVersion\":" << view.snapshot_version
       << ",\"sourceEpoch\":" << view.source_epoch
       << ",\"telemetrySourceEpoch\":" << view.source_epoch
       << ",\"epoch\":" << view.epoch
       << ",\"tick\":" << view.tick << '}';
    return ss.str();
}

bool flush_telemetry_publications(
    SOCKET client, ftd::NativeTelemetryScheduler& scheduler) {
    // Invalidation is a smaller, earlier state-boundary signal. Write it
    // before a later snapshot delta from the same single writer so a browser
    // cannot briefly label an old cached group fresh after a direct edit.
    while (const auto invalidation = scheduler.take_invalidation()) {
        if (!ftd::ws_send_text(client, json_telemetry_invalidation(*invalidation))) {
            return false;
        }
    }
    while (const auto publication = scheduler.take_publication()) {
        if (!ftd::ws_send_text(client, json_telemetry_publication(*publication))) {
            return false;
        }
    }
    return true;
}

std::string json_flux_slice(ftd::RenderBridge& rb, int axis, int index) {
    std::vector<float> magnitudes;
    rb.copy_visual_flux_magnitude_plane(axis, index, magnitudes);
    std::ostringstream ss;
    ss << std::setprecision(6);
    ss << "{\"type\":\"flux_slice\",\"axis\":" << axis << ",\"index\":" << index << ",\"data\":[";
    bool first = true;
    for (const float magnitude : magnitudes) {
        if (!first) ss << ",";
        ss << magnitude;
        first = false;
    }
    ss << "]}";
    return ss.str();
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
    else if (cmd == "get_field_sample") {
        if (telemetry.has_pending_or_due_observation()) {
            return send_json_response(
                client, json_visual_deferred("get_field_sample", telemetry), request_id);
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
        return ftd::ws_send_binary(client, pack_field_sample(*rb, kind, stride, token));
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
            return true;
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
        }
        if (changed) telemetry.on_state_mutated(*rb);
        return true;  // Fire-and-forget
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

// The original server blocked indefinitely in ws_read_frame(), which meant an
// already-completed CUDA event could not be published until the browser sent
// another command.  Keep the WebSocket transport single-writer/ordered, but
// wake periodically to poll the non-blocking native snapshot fence.
enum class ClientPollResult { readable, timeout, error };
enum class ClientFrameReadiness { ready, incomplete, error };

ClientPollResult wait_for_client_activity(SOCKET client, int timeout_ms) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(client, &read_set);

    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
#ifdef _WIN32
    const int result = ::select(0, &read_set, nullptr, nullptr, &timeout);
#else
    const int result = ::select(client + 1, &read_set, nullptr, nullptr, &timeout);
#endif
    if (result == 0) return ClientPollResult::timeout;
    if (result == SOCKET_ERROR || result < 0) return ClientPollResult::error;
    return FD_ISSET(client, &read_set)
        ? ClientPollResult::readable
        : ClientPollResult::timeout;
}

bool socket_recv_would_block() {
#ifdef _WIN32
    const int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

// ws_read_frame() correctly uses recv_exact() for a complete WebSocket frame,
// but it is intentionally blocking. A TCP socket can become readable after
// only the first few bytes have arrived, so gate the legacy parser on a
// non-consuming, complete-frame check. This keeps telemetry fence polling
// alive during a slow/partial client upload without changing ws_protocol.
ClientFrameReadiness complete_client_frame_available(SOCKET client) {
    std::uint64_t available = 0;
#ifdef _WIN32
    u_long buffered = 0;
    if (::ioctlsocket(client, FIONREAD, &buffered) == SOCKET_ERROR)
        return ClientFrameReadiness::error;
    available = buffered;
#else
    int buffered = 0;
    if (::ioctl(client, FIONREAD, &buffered) < 0)
        return ClientFrameReadiness::error;
    available = buffered > 0 ? static_cast<std::uint64_t>(buffered) : 0u;
#endif

    // select() also reports a clean close as readable. Let ws_read_frame()
    // consume that immediately; it cannot block because recv() returns zero.
    if (available == 0) return ClientFrameReadiness::ready;
    if (available < 2) return ClientFrameReadiness::incomplete;

    std::array<std::uint8_t, 14> header{};
    const int peeked = ::recv(
        client, reinterpret_cast<char*>(header.data()),
        static_cast<int>((std::min)(available,
                                    static_cast<std::uint64_t>(header.size()))),
        MSG_PEEK);
    if (peeked == 0) return ClientFrameReadiness::ready;
    if (peeked < 0) {
        return socket_recv_would_block()
            ? ClientFrameReadiness::incomplete
            : ClientFrameReadiness::error;
    }
    if (peeked < 2) return ClientFrameReadiness::incomplete;

    const bool fin = (header[0] & 0x80u) != 0;
    const bool reserved = (header[0] & 0x70u) != 0;
    const bool masked = (header[1] & 0x80u) != 0;
    const std::uint8_t length_code = header[1] & 0x7fu;
    std::size_t extended_bytes = 0;
    if (length_code == 126u) extended_bytes = 2;
    else if (length_code == 127u) extended_bytes = 8;
    const std::size_t prefix_bytes = 2u + extended_bytes;

    if (available < prefix_bytes
        || static_cast<std::size_t>(peeked) < prefix_bytes) {
        return ClientFrameReadiness::incomplete;
    }

    // Invalid control bits/masking are rejected by ws_read_frame() before it
    // reads a mask/payload, so dispatch it now rather than wait for bytes that
    // well-formed browsers will never send.
    if (!fin || reserved || !masked) return ClientFrameReadiness::ready;

    std::uint64_t payload_bytes = length_code;
    if (length_code == 126u) {
        payload_bytes = (static_cast<std::uint64_t>(header[2]) << 8u)
                      | static_cast<std::uint64_t>(header[3]);
    } else if (length_code == 127u) {
        payload_bytes = 0;
        for (std::size_t i = 0; i < 8; ++i) {
            payload_bytes = (payload_bytes << 8u) | header[2u + i];
        }
    }

    // Match ws_protocol's 64 KiB bound. It rejects this before reading the
    // mask/payload, so the existing parser remains safe to enter.
    constexpr std::uint64_t kMaxClientFrameBytes = 64ull * 1024ull;
    if (payload_bytes > kMaxClientFrameBytes) return ClientFrameReadiness::ready;

    const std::uint64_t frame_bytes = static_cast<std::uint64_t>(prefix_bytes)
                                    + 4u + payload_bytes;
    return available >= frame_bytes
        ? ClientFrameReadiness::ready
        : ClientFrameReadiness::incomplete;
}

}  // anonymous namespace

// ============================================================================
//  Main
// ============================================================================

int main(int argc, char* argv[]) {
    // The desktop host redirects stdout/stderr to its persistent session log.
    // Line-buffer explicitly so startup, allocation, and failure messages are
    // visible immediately instead of appearing only when the process exits.
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    int lattice_size = 32;
    int port = 9100;
    // Revision 1.4 hardening: default to loopback. The protocol has NO
    // authentication and no Origin check, so the previous INADDR_ANY default
    // let any LAN host (or any webpage — same-origin policy does not block
    // cross-origin WebSocket) drive the engine. LAN/remote use is preserved
    // via an explicit opt-in flag: --bind <addr> (e.g. --bind 0.0.0.0).
    std::string bind_addr = "127.0.0.1";

    std::vector<const char*> positional;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--bind") == 0 && i + 1 < argc) {
            bind_addr = argv[++i];
        } else {
            positional.push_back(argv[i]);
        }
    }
    if (positional.size() >= 1) lattice_size = std::atoi(positional[0]);
    if (positional.size() >= 2) port = std::atoi(positional[1]);
    if (lattice_size < 4) lattice_size = 4;
    if (lattice_size > 256) lattice_size = 256;
    if (port < 1 || port > 65535) port = 9100;

    std::cout << "================================================================\n";
    std::cout << "  FTD WebSocket Server\n";
    std::cout << "  G* = " << std::setprecision(10) << ftd::G_STAR
              << "  alpha^-1 = " << ftd::X_PLUS << "\n";
    std::cout << "================================================================\n\n";

    // Create engine through the same conservative budget/error boundary used
    // by runtime replacements. Startup failures now produce a useful log and
    // exit code instead of std::terminate or a CUDA macro exit from deep inside
    // the allocation stack.
    std::cout << "[ws_server] Creating RenderBridge(" << lattice_size << ")...\n";
    const ResourceBudget startup_budget = resource_budget(lattice_size);
    if (!startup_budget.accepted()) {
        std::cerr << "[ws_server] " << budget_error(startup_budget) << "\n";
        return 2;
    }

    std::unique_ptr<ftd::RenderBridge> rb;
    try {
        rb = make_interactive_bridge(lattice_size);
    } catch (const std::exception& ex) {
        std::cerr << "[ws_server] Engine startup failed: " << ex.what() << "\n";
        return 2;
    } catch (...) {
        std::cerr << "[ws_server] Engine startup failed: unknown native exception\n";
        return 2;
    }

    // Report the backend that is actually active, not merely whether this
    // executable was compiled with CUDA support. Desktop and browser clients
    // use the same runtime truth through the `info` command below.
    if (rb->backend_kind() == ftd::Backend::Kind::Gpu) {
        std::cout << "[ws_server] GPU backend active\n";
    } else {
        std::cout << "[ws_server] CPU mode\n";
    }

    // The source exists before any dashboard attaches.  Demand is initially
    // empty, so this only establishes a generation boundary; no telemetry
    // reduction is launched until the client subscribes.
    ftd::NativeTelemetryScheduler telemetry;
    telemetry.on_source_replaced(*rb);

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[ws_server] WSAStartup failed\n";
        return 1;
    }
#endif

    // socklen_t must match the system ABI; on Windows the socket headers
    // define it as `int`, so mirror that here for the accept() call.
#ifdef _WIN32
    using socklen_t = int;
#endif

    // Create server socket
    SOCKET server_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "[ws_server] socket() failed\n";
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    // Bind (loopback by default — see the --bind flag in main; revision 1.4)
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (bind_addr == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else if (::inet_pton(AF_INET, bind_addr.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "[ws_server] invalid --bind address '" << bind_addr << "'\n";
        closesocket(server_sock);
        return 1;
    }
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(server_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ws_server] bind() failed on " << bind_addr << ":" << port << "\n";
        closesocket(server_sock);
        return 1;
    }

    if (::listen(server_sock, 1) == SOCKET_ERROR) {
        std::cerr << "[ws_server] listen() failed\n";
        closesocket(server_sock);
        return 1;
    }

    std::cout << "[ws_server] Listening on " << bind_addr << ":" << port << "\n";
    if (bind_addr != "127.0.0.1") {
        std::cout << "[ws_server] *** WARNING: bound to " << bind_addr << " — this protocol has\n"
                     "[ws_server] *** NO authentication and NO Origin check; any host or webpage\n"
                     "[ws_server] *** that can reach this port can drive the engine (toggles,\n"
                     "[ws_server] *** scenarios, injection). Use only on trusted networks.\n";
    }

    // Main accept loop: one client at a time. Keep polling a retired snapshot
    // while no client is attached, so a CUDA event failure is converted into
    // a suspended/recoverable scheduler before a reconnect reaches `info`.
    std::cout << "[ws_server] Waiting for client...\n";
    while (true) {
        const ClientPollResult accept_poll = wait_for_client_activity(
            server_sock, 8);
        if (accept_poll == ClientPollResult::error) {
            std::cerr << "[ws_server] accept readiness poll failed\n";
            continue;
        }
        if (accept_poll == ClientPollResult::timeout) {
            try {
                telemetry.pump(*rb);
                // There is intentionally no observer while disconnected.
                // Demand has been retired, but drain defensively if a future
                // backend changes that behavior.
                while (telemetry.take_publication()) {}
                while (telemetry.take_invalidation()) {}
            } catch (const std::exception& ex) {
                std::cerr << "[ws_server] Retired telemetry publisher failed: "
                          << ex.what() << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
            } catch (...) {
                const std::string message = "unknown retired native telemetry publisher failure";
                std::cerr << "[ws_server] " << message << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
            }
            continue;
        }

        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        SOCKET client = ::accept(server_sock,
                                 reinterpret_cast<sockaddr*>(&client_addr),
                                 &client_len);
        if (client == INVALID_SOCKET) {
            std::cerr << "[ws_server] accept() failed\n";
            continue;
        }

        std::cout << "[ws_server] Client connected\n";

        // WebSocket handshake
        if (!ftd::ws_handshake(client)) {
            std::cerr << "[ws_server] Handshake failed\n";
            closesocket(client);
            continue;
        }

        std::cout << "[ws_server] WebSocket handshake complete\n";

        // Message loop.  Snapshot publication is intentionally serviced from
        // this same single transport writer: an unsolicited JSON delta cannot
        // interleave with a binary response frame or be mistaken for a
        // request-correlated response.
        bool connected = true;
        constexpr int kTelemetryPollIntervalMs = 8;
        const auto service_telemetry = [&]() -> bool {
            telemetry.pump(*rb);
            return flush_telemetry_publications(client, telemetry);
        };
        while (connected) {
            try {
                if (!service_telemetry()) {
                    connected = false;
                    break;
                }
            } catch (const std::exception& ex) {
                std::cerr << "[ws_server] Telemetry publisher failed: "
                          << ex.what() << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
                send_json_response(
                    client, json_native_recovery_required("telemetry", telemetry), 0);
                connected = false;
                break;
            } catch (...) {
                const std::string message = "unknown native telemetry publisher failure";
                std::cerr << "[ws_server] " << message << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
                send_json_response(
                    client, json_native_recovery_required("telemetry", telemetry), 0);
                connected = false;
                break;
            }

            const ClientPollResult poll = wait_for_client_activity(
                client, kTelemetryPollIntervalMs);
            if (poll == ClientPollResult::error) {
                connected = false;
                break;
            }
            if (poll == ClientPollResult::timeout) continue;

            const ClientFrameReadiness frame =
                complete_client_frame_available(client);
            if (frame == ClientFrameReadiness::error) {
                connected = false;
                break;
            }
            if (frame == ClientFrameReadiness::incomplete) {
                // The socket remains readable while a partial frame sits in
                // its receive buffer. Avoid a tight spin but keep snapshot
                // fence latency bounded to a millisecond in this rare case.
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            std::vector<uint8_t> payload;
            uint8_t opcode = ftd::ws_read_frame(client, payload);

            switch (opcode) {
            case WS_TEXT: {
                std::string msg(payload.begin(), payload.end());
                try {
                    if (!handle_command(msg, client, rb, telemetry, lattice_size))
                        connected = false;
                } catch (const std::exception& ex) {
                    // CUDA/kernel/validation failures are surfaced through the
                    // protocol. Allocation commands are transactional, but a
                    // tick/run exception may leave a partially advanced CUDA
                    // state. Send the typed error, then break this client
                    // connection so the dashboard reconnect path rebuilds the
                    // selected scenario instead of immediately retrying the
                    // poisoned bridge in an rAF error loop.
                    std::cerr << "[ws_server] Command failed: " << ex.what() << "\n";
                    const std::string operation = ftd::json_string(msg, "cmd");
                    if (operation == "tick" || operation == "run") {
                        if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
                        if (!send_json_response(
                                client,
                                json_native_recovery_required(operation, telemetry),
                                request_id_from(msg))) {
                            connected = false;
                        }
                        connected = false;
                    } else if (!send_json_response(
                                   client, json_error(ex.what(), operation),
                                   request_id_from(msg))) {
                        connected = false;
                    }
                } catch (...) {
                    const std::string message = "unknown native command failure";
                    std::cerr << "[ws_server] " << message << "\n";
                    const std::string operation = ftd::json_string(msg, "cmd");
                    if (operation == "tick" || operation == "run") {
                        if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
                        if (!send_json_response(
                                client,
                                json_native_recovery_required(operation, telemetry),
                                request_id_from(msg))) {
                            connected = false;
                        }
                        connected = false;
                    } else if (!send_json_response(
                                   client, json_error(message, operation),
                                   request_id_from(msg))) {
                        connected = false;
                    }
                }
                break;
            }
            case WS_BINARY:
                // Binary frames from client not expected; ignore
                break;
            case WS_PING: {
                // Respond with pong (same payload)
                ftd::ws_send_frame(client, WS_PONG, payload.data(), payload.size());
                break;
            }
            case WS_CLOSE:
                // Send close frame back
                ftd::ws_send_frame(client, WS_CLOSE, nullptr, 0);
                connected = false;
                break;
            default:
                // 0xFF = disconnect/error, anything else = unknown
                connected = false;
                break;
            }

            // A command's normal response (including any binary frame) is
            // completely written before an optional publisher delta.  CPU
            // snapshots may be ready immediately; CUDA snapshots will be
            // picked up by a later idle poll without stalling the command.
            if (connected) {
                try {
                    if (!service_telemetry()) connected = false;
                } catch (const std::exception& ex) {
                    std::cerr << "[ws_server] Telemetry publisher failed: "
                              << ex.what() << "\n";
                    if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
                    send_json_response(
                        client, json_native_recovery_required("telemetry", telemetry), 0);
                    connected = false;
                } catch (...) {
                    const std::string message = "unknown native telemetry publisher failure";
                    std::cerr << "[ws_server] " << message << "\n";
                    if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
                    send_json_response(
                        client, json_native_recovery_required("telemetry", telemetry), 0);
                    connected = false;
                }
            }
        }

        closesocket(client);
        telemetry.on_client_disconnected();
        std::cout << "[ws_server] Client disconnected\n";
        std::cout << "[ws_server] Waiting for client...\n";
    }

    closesocket(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
