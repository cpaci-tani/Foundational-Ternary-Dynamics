#pragma once
/**
 * Shared physics constants — included by both constants.h and CUDA kernels.
 *
 * KEEP THIS FILE FREE of any host-only stdlib includes (<iostream>, <string>,
 * <algorithm>, etc.).  It must compile cleanly under both g++ and nvcc.
 *
 * Color force regime boundaries must also match the JS mock values used in
 * the WASM bridge (engine/web/).
 */

// Color force regime boundaries (must match GPU kernels_forces.cu).
// Three-regime profile: Coulomb (r<3) → transition (3-8) → linear confinement (r>=8).
inline constexpr double COLOR_COULOMB_RADIUS    = 3.0;  // r < 3:       F = α_s·cf / r²
inline constexpr double COLOR_TRANSITION_RADIUS = 8.0;  // 3 <= r < 8:  F = α_s·cf / (denom·r)
inline constexpr double COLOR_TRANSITION_DENOM  = 3.0;  // denominator in transition formula
inline constexpr double COLOR_LINEAR_DENOM      = 64.0; // r >= 8:      F = α_s·cf·r / denom

// ----------------------------------------------------------------------------
// Watson-integral gauge factors for the new substrate kernels in
// cuda/kernels_stencil.cu (strong_field / weak_field).
//
// Numerators are framework gauge ratios from the Watson-integral loop
// expansion (CLAUDE.md: c2 loop uses 13/9, c3 loop uses 11/6).
// Denominators normalize by sqrt(N_neighbors) so the per-voxel source
// amplitude is invariant when switching between sub-lattice stencils.
//
// STELLA_OCTA: 8 vertex neighbors  → denom = sqrt(8) = 2·sqrt(2)
// CUBOCTA:    12 edge   neighbors  → denom = sqrt(12) = 2·sqrt(3)
// ----------------------------------------------------------------------------
inline constexpr double STELLA_OCTA_NEIGHBOR_NORM = 2.8284271247461900976; // sqrt(8)
inline constexpr double CUBOCTA_NEIGHBOR_NORM     = 3.4641016151377545871; // sqrt(12)
inline constexpr double VERTEX_GAUGE_NUM          = 11.0 / 6.0;            // c3 loop gauge
inline constexpr double EDGE_GAUGE_NUM            = 13.0 / 9.0;            // c2 loop gauge
inline constexpr double VERTEX_GAUGE = VERTEX_GAUGE_NUM / STELLA_OCTA_NEIGHBOR_NORM;
inline constexpr double EDGE_GAUGE   = EDGE_GAUGE_NUM   / CUBOCTA_NEIGHBOR_NORM;

// NOTE: GRAD_TIER1_SCALE, GRAD_TIER2_SCALE, LAPLACIAN_FACE_WEIGHT, and
// LAPLACIAN_EDGE_WEIGHT are defined in include/ftd/constants.h (host) inside
// `namespace ftd`. The CUDA kernels in this directory live in
// `namespace ftd::gpu::kernels` and resolve them via ordinary unqualified
// name lookup, so we deliberately do NOT redeclare them here — doing so
// at global scope would create a second entity with the same simple name
// and make every consumer ambiguous when both headers are in scope.
