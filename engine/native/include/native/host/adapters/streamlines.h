#pragma once
//
// host/adapters/streamlines.h — CPU RK4 field-line integrator for the Scale-0
// STREAMLINE overlays (Flux Lines · Radiative E · B Field), ported from the web
// dashboard's engine/web/js/fieldlines.js `computeStreamlines` + the four seed
// generators (generateImportanceSeeds / generateEFieldSeeds / generateBFieldSeeds
// / generateBImportanceSeeds) and the per-overlay colour ramps in
// flux-renderer.js / field-em-renderer.js.
//
// The web traces streamlines through a SPARSE sampled field via a spatial index
// (nearest-sample lookup). The native port instead SCATTERS a stride-1
// VisualFieldSample into a dense L³ voxel grid and looks the field up by
// nearest voxel (floor+clamp) — index = x + y·L + z·L² — which is exactly the
// nearest-sample semantics computeStreamlines uses when the samples ARE the
// voxel centres (the task's specified design). Unsampled voxels (the sample
// compacts away near-zero sites) read back as zero → the trace stops there
// (minMag), matching the web's line-truncation behaviour.
//
// This TU names only ftd::VisualFieldSample (a plain data struct) and
// NativeLine/NativeParticle (plain data structs) — it does NOT name
// RenderBridge/TermToggles/any Scale-0 command, so it stays outside the
// adapter's concrete-type boundary.
//
#include "native/native_frame.h"
#include "ftd/visual_field_sample.h"

#include <array>
#include <vector>

namespace ftd::native::streamlines {

// Which streamline overlay's seeding + colour profile to run.
enum class Overlay {
    Flux,      // Flux Lines: importance seeds ∝|J|^1.5; flux colormap by local |J|
    Electric,  // Radiative E (−∂J/∂t): particle-anchored (6/particle, offset 2) + importance
               //          fallback; cyan fade by arc-length
    Magnetic,  // B Field: perpendicular ring seeds (8/particle, radius 4) +
               //          perpendicular-importance fallback; maxSteps×1.5 to close
               //          loops; green fade by arc-length
};

// Append RK4-traced, per-vertex-coloured field-line geometry for one streamline
// overlay into `out_lines` (each polyline becomes consecutive NativeLine
// segments; composites through the presenter's existing LINE PSO alongside every
// other overlay). `field` is the stride-1 3-vector VisualFieldSample of the
// field to trace (FluxVector for Flux, Electric for E, Magnetic for B). `L` is
// the lattice size (dense-grid extent + integration bound). `particles` are the
// frame's manifested particle centres in voxel-centre coordinates, used by the
// E/B particle-anchored seed generators; an empty list falls back to
// importance seeding. Bounded by the web caps (maxLines 200, maxSteps 100 —
// ×1.5 for Magnetic).
void append(const ftd::VisualFieldSample& field,
            const std::vector<std::array<float, 3>>& particles,
            int L, Overlay overlay,
            std::vector<NativeLine>& out_lines);

// Force-Flow render-style: importance-seeded (∝|force|^1.5) RK4 streamlines,
// traced at ~40% of the field-line length and rendered DASHED (dash 1.5 / gap
// 0.8 voxels) with a `phase` offset so the dashes animate as the sim advances
// (port of updateForceStreamlines + LineDashedMaterial). Each retained segment
// is coloured by the local |force| through the 3-stop force palette
// (`low/mid/high`). Appends into `out_lines` (the shared LINE PSO group).
void append_force_flow(const ftd::VisualFieldSample& field, int L, float phase,
                       const float low[3], const float mid[3], const float high[3],
                       std::vector<NativeLine>& out_lines);

// One detected knot zone: a density-clustered bunch of field-line segments,
// reported as an axis-aligned box (centroid ± half-extents) for the wireframe
// overlay. `index` is the knot's ordinal within its family (E or B) — the input
// to the per-knot hue.
struct KnotBox {
    float cx = 0.0f, cy = 0.0f, cz = 0.0f;   // density-weighted centroid
    float hx = 1.0f, hy = 1.0f, hz = 1.0f;   // half-extents (≥ 1 voxel)
    int   index = 0;                          // ordinal within the family
};

// Detect field-line Knot Zones over a set of already-traced streamline segments
// (port of the DEFAULT gate in engine/web/js/scales/scale0/runtime/
// field-line-knots.js: bin segment midpoints into a coarse cell grid, threshold
// on adaptive local density, 26-neighbour flood-fill the hot cells into knots,
// then report each knot's centroid + bounding-box extents). `sensitivity`∈[0,1]
// scales the adaptive density threshold (higher → more knots). At most
// `max_knots` (largest first). The web's optional crossing gate is OFF by
// default (requireCrossings=false), so density + flood-fill matches it exactly.
std::vector<KnotBox> detect_knots(const std::vector<NativeLine>& segments, int L,
                                  float sensitivity = 0.5f, int max_knots = 32);

}  // namespace ftd::native::streamlines
