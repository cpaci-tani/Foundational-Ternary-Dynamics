#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace ftd {

// Stable wire identifiers used by the native WebSocket FTS1/FTS2 frames. Keep the
// numeric values synchronized with engine/web/js/ws-bridge.js.
enum class VisualFieldKind : std::uint32_t {
    Electric = 0,
    Magnetic = 1,
    Poynting = 2,
    Divergence = 3,
    FluxVector = 4,
    Vorticity = 5,
    Helicity = 6,
    Kretschmann = 7,
    Latency = 8,
    Fisher = 9,
    Coherence = 10,
    Curl = 11,
    State = 12,
    GaussResidual = 13,
    EmForce = 14,
    GravityForce = 15,
    StrongForce = 16,
    // Real gravitational/metric latency produced by the Poisson solve and
    // stored in voxel.latency (CUDA: d_latency).  Keep Latency=8 as the
    // historical normalized |J|^2 visualization proxy for wire compatibility.
    PoissonLatency = 17,
};

struct VisualFieldSample {
    std::uint32_t components = 0;  // 1 for scalar fields, 3 for vectors
    int effective_stride = 1;
    int origin = 0;                // first coordinate represented on each axis
    std::vector<float> positions;  // xyz voxel centres, 3 * count
    std::vector<float> data;       // components * count

    std::size_t count() const {
        return components == 0 ? 0 : data.size() / components;
    }
};

bool parse_visual_field_kind(std::string_view name, VisualFieldKind& out);
const char* visual_field_kind_name(VisualFieldKind kind);
std::uint32_t visual_field_components(VisualFieldKind kind);

}  // namespace ftd
