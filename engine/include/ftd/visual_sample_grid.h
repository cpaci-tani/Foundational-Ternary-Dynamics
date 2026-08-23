#pragma once

#include <algorithm>
#include <cstddef>

// ─────────────────────────────────────────────────────────────────────────────
// Shared sampling grid for the visual field-overlay samplers.
//
// Three backends materialize the overlay samples from different data — the CPU
// SoA (src/visual_field_sample.cpp), the GPU device buffers
// (cuda/visual_field_sample.cu), and the WASM RenderBridge with zero-copy JS
// interop (wasm/ftd_wasm.cpp). Their loops differ, but the *grid* they iterate —
// which voxels, at what stride, anchored where — is identical integer math.
// That math used to live copy-pasted in all three, which is how the
// center-anchoring bug below shipped in three places at once. It lives here now.
// ─────────────────────────────────────────────────────────────────────────────

namespace ftd {

// Cap on samples-per-axis³ before the requested stride is auto-raised. 64³.
constexpr std::size_t kMaxDenseVisualSamples = 262144u;

// Raise `requested_stride` until an N-lattice yields at most kMaxDenseVisualSamples
// sample points, so a fine stride on a large lattice can't blow the point budget.
// `interior` kinds skip the two boundary voxels per axis (usable extent N-2).
inline int bounded_visual_stride(int N, int requested_stride, bool interior) {
    int stride = std::max(1, requested_stride);
    const int extent = std::max(0, N - (interior ? 2 : 0));
    const auto samples_for = [extent](int s) -> std::size_t {
        const std::size_t per_axis = static_cast<std::size_t>((extent + s - 1) / s);
        return per_axis * per_axis * per_axis;
    };
    while (samples_for(stride) > kMaxDenseVisualSamples) ++stride;
    return stride;
}

// The regular per-axis sample grid: `count` voxel indices
// { origin, origin+stride, …, origin+(count-1)*stride }, CENTER-ANCHORED on the
// geometric center voxel (N-1)/2 and symmetric about it.
//
// Why center-anchored: every lattice size is odd, so (N-1)/2 is a true center on
// each axis. Anchoring the grid at 0/1 instead (as the old code did) meant that
// an INTERIOR field at stride 2 sampled the odd voxels {1,3,…,N-2} — an even
// count that SKIPS the (even) center voxel. The overlay then had no sample on the
// axis and read as shifted toward high indices, and the mid-plane resolver
// rounded the shown slice up to N/2+1. Anchoring on the center guarantees a
// sample lands on (N-1)/2 and keeps the grid symmetric for any size and stride.
struct VisualSampleGrid {
    int stride = 1;   // effective per-axis stride (already budget-clamped)
    int origin = 0;   // first sampled voxel index on each axis (>= lo, on center)
    int count = 0;    // number of samples on each axis (0 if the lattice is too small)

    // One-past-the-last sampled index, so callers can write the natural loop:
    //   for (int v = grid.origin; v < grid.end(); v += grid.stride) { … }
    int end() const { return origin + count * stride; }
};

// Build the center-anchored grid for a lattice of size `N`. `interior` kinds use
// the range [1, N-2] (skipping the boundary voxels their neighbour stencils would
// wrap across); all others use [0, N-1].
inline VisualSampleGrid visual_sample_grid(int N, int requested_stride, bool interior) {
    VisualSampleGrid grid;
    grid.stride = bounded_visual_stride(N, requested_stride, interior);
    const int lo = interior ? 1 : 0;
    const int hi = interior ? N - 2 : N - 1;
    if (hi < lo) return grid;  // lattice too small (e.g. interior on N < 3) → count 0
    const int center = (N - 1) / 2;
    // Largest multiple of stride at or below the center that is still >= lo.
    grid.origin = center - ((center - lo) / grid.stride) * grid.stride;
    grid.count = (hi - grid.origin) / grid.stride + 1;
    return grid;
}

}  // namespace ftd
