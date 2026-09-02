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
    // NOT the Kretschmann scalar R_abcd R^abcd -- there is no metric or
    // curvature tensor computed here. Actually computes the square of the
    // 18-point Laplacian of a normalized flux magnitude (see the
    // Kretschmann case in copy_visual_field_sample). Historical misnomer;
    // kept for wire compatibility (see the Latency/PoissonLatency note below
    // for the same pattern).
    Kretschmann = 7,
    Latency = 8,
    Fisher = 9,
    // NOT a coherence measure. Actually computes cos(angle) between the
    // flux J and curl(J) -- a normalized helicity density. Historical
    // misnomer; kept for wire compatibility.
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

// True for the 3-component (vector) fields; the rest are scalar.
inline bool is_vector_field_kind(VisualFieldKind kind) {
    switch (kind) {
        case VisualFieldKind::Electric:
        case VisualFieldKind::Magnetic:
        case VisualFieldKind::Poynting:
        case VisualFieldKind::FluxVector:
        case VisualFieldKind::Curl:
        case VisualFieldKind::EmForce:
        case VisualFieldKind::GravityForce:
        case VisualFieldKind::StrongForce:
            return true;
        default:
            return false;
    }
}

// True for the neighbour-stencil fields (curl/divergence-based) that must skip
// the periodic boundary voxels — their stencils would wrap across the seam and
// manufacture spurious edge spikes. Consumed by visual_sample_grid()'s `interior`
// argument. NOTE: Divergence is deliberately NOT interior here (it is sampled on
// the full grid); only the curl-derived scalars are.
inline bool is_interior_field_kind(VisualFieldKind kind) {
    switch (kind) {
        case VisualFieldKind::Vorticity:
        case VisualFieldKind::Helicity:
        case VisualFieldKind::Kretschmann:
        case VisualFieldKind::Fisher:
        case VisualFieldKind::Coherence:
        case VisualFieldKind::Curl:
            return true;
        default:
            return false;
    }
}

}  // namespace ftd
