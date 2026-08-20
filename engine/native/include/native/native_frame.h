#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ftd::native {

struct NativeParticle {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float size = 0.45f;
};

// One field-overlay vector rendered as a coloured line segment (a 3-vector
// field sample: base at the voxel centre, tip at base + dir*scale). Endpoint
// colours encode magnitude; the base is dimmed and the tip is bright so the
// segment reads as an arrow (direction cue without a separate arrowhead PSO).
// Drawn through the presenter's existing LINE pipeline alongside the box.
struct NativeLine {
    float x0 = 0.0f, y0 = 0.0f, z0 = 0.0f;
    float r0 = 1.0f, g0 = 1.0f, b0 = 1.0f;
    float x1 = 0.0f, y1 = 0.0f, z1 = 0.0f;
    float r1 = 1.0f, g1 = 1.0f, b1 = 1.0f;
};

struct NativeFrame {
    int tick = 0;
    int lattice_size = 0;
    int flux_boundary = 2;
    std::uint32_t total_manifested = 0;
    std::string scenario;
    std::string backend;
    std::string status;
    std::vector<NativeParticle> particles;
    // Point cloud drawn through the sprite path: the ambient flux cloud by
    // default, or a scalar field overlay's magnitude-coloured points when one
    // is active.
    std::vector<NativeParticle> flux;
    // Vector field overlay geometry (empty unless a 3-vector overlay is active).
    std::vector<NativeLine> field_lines;
};

}  // namespace ftd::native
