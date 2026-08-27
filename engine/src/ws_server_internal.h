#pragma once
/**
 * @file ws_server_internal.h
 * @brief Private module boundary for the native WebSocket server.
 *
 * This header is intentionally scoped to the ws_server executable. It keeps
 * command dispatch, payload encoding, telemetry JSON, and the socket runtime
 * in separate translation units without publishing another engine API.
 */

#include "ftd/native_telemetry_scheduler.h"
#include "ftd/render_bridge.h"
#include "ftd/visual_field_sample.h"
#include "ftd/ws_protocol.h"

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace ftd::ws_server_detail {

inline constexpr int kMinLatticeSize = 4;
inline constexpr int kMaxLatticeSize = 256;
inline constexpr std::size_t kMaxVisualParticles = 100000;

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

ResourceBudget resource_budget(int requested_size);
std::string budget_error(const ResourceBudget& budget);
std::unique_ptr<RenderBridge> make_interactive_bridge(int lattice_size);

std::vector<std::uint8_t> pack_particle_data(RenderBridge& bridge);
std::vector<std::uint8_t> pack_flux_volume(RenderBridge& bridge,
                                           int requested_axis_samples);
std::vector<std::uint8_t> pack_field_sample(RenderBridge& bridge,
                                            VisualFieldKind kind,
                                            int stride,
                                            std::uint32_t token,
                                            int planes_mid);

std::string json_escape(const std::string& value);
std::string json_error(const std::string& message,
                       const std::string& operation);
std::string json_native_recovery_required(
    const std::string& operation,
    const NativeTelemetryScheduler& telemetry);
std::uint64_t request_id_from(const std::string& json);
bool send_json_response(SOCKET client, std::string response,
                        std::uint64_t request_id);

std::string json_telemetry_cached(
    const NativeTelemetryScheduler::CachedView& view,
    std::uint32_t requested_mask);
std::string json_cached_legacy_group(
    const NativeTelemetryScheduler::CachedView& view,
    std::uint32_t bit);
std::string json_voxel(RenderBridge& bridge, int x, int y, int z);
std::string json_force_at(RenderBridge& bridge, int x, int y, int z);
std::string json_info(RenderBridge& bridge,
                      const NativeTelemetryScheduler& telemetry);
std::string json_visual_deferred(
    const char* operation,
    const NativeTelemetryScheduler& telemetry);
std::uint32_t telemetry_selection_mask(const std::string& json);
bool parse_telemetry_demand(
    const std::string& json,
    const NativeTelemetryScheduler& scheduler,
    NativeTelemetryScheduler::Demand& out,
    std::string& error);
std::string json_telemetry_demand_ack(
    const NativeTelemetryScheduler& scheduler);
bool flush_telemetry_publications(
    SOCKET client, NativeTelemetryScheduler& scheduler);
std::string json_flux_slice(RenderBridge& bridge, int axis, int index);

bool handle_command(const std::string& json, SOCKET client,
                    std::unique_ptr<RenderBridge>& bridge,
                    NativeTelemetryScheduler& telemetry,
                    int& lattice_size);

int run_server(int argc, char* argv[]);

}  // namespace ftd::ws_server_detail
