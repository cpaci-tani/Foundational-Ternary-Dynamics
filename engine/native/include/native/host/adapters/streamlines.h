#pragma once
//
// host/adapters/streamlines.h — CPU RK4 field-line integrator for the Scale-0
// STREAMLINE overlays (Flux Lines · E Field · B Field), ported from the web
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
    Electric,  // E Field: particle-anchored (6/particle, offset 2) + importance
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

}  // namespace ftd::native::streamlines
