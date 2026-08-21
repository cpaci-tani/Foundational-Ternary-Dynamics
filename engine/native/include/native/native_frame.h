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

// One vertex of a rubber-sheet (topology / stress-energy) surface: a world
// position plus a per-vertex RGBA colour (alpha carries the translucent-sheet
// opacity). Drawn through the presenter's triangle-mesh vertex-colour PSO
// (double-sided, alpha-blended, depth-tested) — the app's first non-billboard
// surface. See engine/web/js/viewport/topology-sheet-renderer.js.
struct NativeSheetVertex {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
};

// One force-Glyph instance: an oriented cone at `x,y,z` (voxel centre) pointing
// along the unit direction `dx,dy,dz` (the force direction), sized by `scale`
// and coloured `r,g,b`. The presenter tessellates each instance into a small
// world-space cone (base circle + apex) and shades it, drawn through the new
// glyph triangle PSO (per the web updateForceGlyphs InstancedMesh of cones).
struct NativeGlyph {
    float x = 0.0f, y = 0.0f, z = 0.0f;      // cone centre (world / voxel centre)
    float dx = 0.0f, dy = 1.0f, dz = 0.0f;   // unit direction (force direction)
    float scale = 1.0f;                       // cone size
    float r = 1.0f, g = 1.0f, b = 1.0f;       // colour
};

// One rubber-sheet surface: an indexed triangle mesh (a deformed ~40×40 grid).
// `indices` are three-per-triangle into `vertices`. Built CPU-side by the
// Scale-0 adapter's heightfield pipeline (slice → scatter → box-blur → deform)
// and uploaded whole to the presenter's sheet vertex/index buffers each frame.
struct NativeSheet {
    std::vector<NativeSheetVertex> vertices;
    std::vector<std::uint32_t>     indices;
};

// The adapter's authoritative current slice height (fraction of the lattice box)
// for one active sheet overlay. Exposed each capture so the panel can reflect a
// height set by the CLI / adjusted at runtime, and so a headless verify can read
// it back.
struct NativeSheetHeight {
    std::uint32_t overlay_id = 0;  // OverlayId
    float         height = 0.0f;
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
    // Force-Heatmap sprite points (empty unless a Force overlay is showing in the
    // Heatmap style). Drawn through the presenter's additive gaussian-falloff
    // sprite PSO (a separate group from `flux`, which is alpha-blended).
    std::vector<NativeParticle> flux_heat;
    // Force-Glyph cone instances (empty unless a Force overlay is showing in the
    // Glyphs style). Each is tessellated + shaded by the presenter's glyph PSO.
    std::vector<NativeGlyph> field_glyphs;
    // Vector field overlay geometry (empty unless a 3-vector overlay is active).
    std::vector<NativeLine> field_lines;
    // "On-top" overlay lines drawn AFTER the opaque particles (Force-Flow dashed
    // streamlines): the force field lives at the charge sites, so its streamlines
    // would be occluded by the particles if drawn in the normal (pre-particle)
    // line pass. Empty unless a Force overlay is in the Flow style.
    std::vector<NativeLine> field_lines_top;
    // Rubber-sheet surface geometry (empty unless a Sheet overlay — Φ potential,
    // EM energy, Charge ρ, Vorticity ω, P_E, P_B — is active). Each entry is one
    // deformed grid drawn through the presenter's sheet mesh PSO (+ wireframe).
    std::vector<NativeSheet> field_sheets;
    // Current slice height per active sheet overlay (adapter-authoritative), so
    // the panel reflects CLI / runtime height changes. Empty when no sheet active.
    std::vector<NativeSheetHeight> sheet_heights;
};

}  // namespace ftd::native
