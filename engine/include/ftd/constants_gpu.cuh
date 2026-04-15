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
