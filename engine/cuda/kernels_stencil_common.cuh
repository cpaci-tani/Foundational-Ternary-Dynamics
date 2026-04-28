#ifndef FTD_KERNELS_STENCIL_COMMON_CUH
#define FTD_KERNELS_STENCIL_COMMON_CUH

// Shared device-side helpers for the stencil kernel TUs.
//
// Phase 5 split (2026-04-27): kernels_stencil.cu (1530 LOC) was decomposed into
//   - kernels_stencil_single.cu  (single-substrate path)
//   - kernels_stencil_dual.cu    (dual-substrate path)
//   - kernels_aux.cu             (weak transmutation, pair production)
// to halve partial CUDA rebuild time. The damping/leapfrog helpers used by both
// phase_write_kernel (single) and phase_write_dual_kernel (dual) live here so
// the formulas have one source of truth.
//
// Pattern mirrors engine/cuda/cuda_index.cuh (consolidated in F-8 / ADR-0007).
//
// All helpers are __device__ __forceinline__ — zero-overhead inlining at every
// call site; safe to include from any .cu translation unit.

#include "ftd/constants.h"
#include "../cuda/cuda_index.cuh"   // ::ftd::wrap, ::ftd::idx3d (X-major)
#include <cstdint>
#include <cmath>

namespace ftd {
namespace gpu {
namespace kernels {

// ---------- Local namespace shims ----------
// F-8: shared shims onto ::ftd::wrap / ::ftd::idx3d in cuda_index.cuh. All
// existing call sites in the kernel TUs use unqualified `wrap(...)` /
// `idx3d(...)`, so we re-export them inside the kernels namespace.

__device__ __forceinline__
int wrap(int x, int L) {
    return ::ftd::wrap(x, L);
}

__device__ __forceinline__
int idx3d(int x, int y, int z, int L) {
    return ::ftd::idx3d(x, y, z, L);
}

// ---------- Damping helpers ----------
// Compute effective damping coefficient at site i. Centralised so the
// single-substrate phase_write, dual-substrate phase_write, and any future
// stencil that needs damping share one source of truth for the Larmor
// modulation formula.
//
//   eff_damp = (1 - DAMPING * larmor_mod)  if larmor active at i
//            = damp                        otherwise
//
// where larmor_mod = clamp(LARMOR_FLOOR + K_LARMOR * a², 0, 1) and a is
// the magnitude of the local acceleration (provided in `near_accel`).
__device__ __forceinline__
double effective_damping(int i, double damp,
                         bool do_larmor, bool selective_damping,
                         const uint8_t* __restrict__ near_particle,
                         const double*  __restrict__ near_accel) {
    if (do_larmor && selective_damping && near_particle[i]) {
        const double a2 = near_accel[i] * near_accel[i];
        const double larmor_mod = fmin(1.0, LARMOR_FLOOR + K_LARMOR * a2);
        return 1.0 - DAMPING * larmor_mod;
    }
    return damp;
}

// Apply damping factor to a (flux, wave_vel) pair of 3-vectors. The single
// and dual substrate kernels both perform this multiplicatively across all
// 6 components; centralising it removes a 6-line block from each kernel.
__device__ __forceinline__
void scale_field_pair(double& fx, double& fy, double& fz,
                      double& wx, double& wy, double& wz,
                      double k) {
    fx *= k; fy *= k; fz *= k;
    wx *= k; wy *= k; wz *= k;
}

} // namespace kernels
} // namespace gpu
} // namespace ftd

#endif // FTD_KERNELS_STENCIL_COMMON_CUH
