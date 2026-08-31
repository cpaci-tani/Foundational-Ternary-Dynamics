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
#include "ftd/lorentz_period2.h"
#include "ftd/lorentz_bcc_time.h"
#include "ftd/sublattice.h"
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

// FTD-0408 / FTD-0411: period-two free-wave kick, read from the live device
// tick so graph replay does not bake even-tick kappa. BCC-time wins if both
// flags are somehow set (same ternary as CPU phase_read).
__device__ __forceinline__
double wave_kick_cw2(int tick, bool period2, bool bcc_time) {
    if (bcc_time)
        return ((tick & 1) == 0)
            ? ::ftd::LORENTZ_BCC_TIME_KAPPA_EVEN
            : ::ftd::LORENTZ_BCC_TIME_KAPPA_ODD;
    if (period2)
        return ((tick & 1) == 0)
            ? ::ftd::LORENTZ_PERIOD2_KAPPA_EVEN
            : ::ftd::LORENTZ_PERIOD2_KAPPA_ODD;
    return C_WAVE * C_WAVE;
}

// Target-local one-way closure for Dispersal. The outer shell stays exact
// void; a boundary-adjacent stencil target receives a virtual Sommerfeld
// sample derived only from its own field/pseudo-velocity. The impedance is
// normalized over the active outward stencil measure so faces, edges, and
// corners get the same damping impulse.
__device__ __forceinline__
double dispersal_laplacian_component(
        const double* __restrict__ field,
        const double* __restrict__ wave,
        int i, int x, int y, int z, int L, uint8_t stencil_mode) {
    const int Nm1 = L - 1;
    if (x <= 0 || x >= Nm1 || y <= 0 || y >= Nm1
        || z <= 0 || z >= Nm1) {
        return 0.0;
    }

    constexpr int faces[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    constexpr int edges[12][3] = {
        {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
        {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
        {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}};
    constexpr int corners[8][3] = {
        {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
        {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}};

    const double face_weight = stencil_mode == 1u ? W_SC_FACE : 1.0 / 3.0;
    const double edge_weight = stencil_mode == 2u ? W_FCC_EDGE : 1.0 / 6.0;
    double outward_measure = 0.0;
    if (stencil_mode == 0u || stencil_mode == 1u) {
        for (int n = 0; n < 6; ++n) {
            const int nx = x + faces[n][0];
            const int ny = y + faces[n][1];
            const int nz = z + faces[n][2];
            if (nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
                || nz == 0 || nz == Nm1) {
                outward_measure += face_weight;
            }
        }
    }
    if (stencil_mode == 0u || stencil_mode == 2u) {
        for (int n = 0; n < 12; ++n) {
            const int nx = x + edges[n][0];
            const int ny = y + edges[n][1];
            const int nz = z + edges[n][2];
            if (nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
                || nz == 0 || nz == Nm1) {
                outward_measure += edge_weight * sqrt(2.0);
            }
        }
    }
    if (stencil_mode == 3u) {
        for (int n = 0; n < 8; ++n) {
            const int nx = x + corners[n][0];
            const int ny = y + corners[n][1];
            const int nz = z + corners[n][2];
            if (nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
                || nz == 0 || nz == Nm1) {
                outward_measure += W_BCC_CORNER * sqrt(3.0);
            }
        }
    }
    const double inverse_measure = outward_measure > 0.0
        ? 1.0 / outward_measure : 0.0;
    const double center = field[i];
    const double velocity = wave[i];
    double face_sum = 0.0;
    double edge_sum = 0.0;
    double corner_sum = 0.0;

    if (stencil_mode == 0u || stencil_mode == 1u) {
        for (int n = 0; n < 6; ++n) {
            const int nx = x + faces[n][0];
            const int ny = y + faces[n][1];
            const int nz = z + faces[n][2];
            const bool shell = nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
                            || nz == 0 || nz == Nm1;
            face_sum += shell
                ? center - velocity * (inverse_measure / C_WAVE)
                : field[nx * L * L + ny * L + nz];
        }
    }
    if (stencil_mode == 0u || stencil_mode == 2u) {
        for (int n = 0; n < 12; ++n) {
            const int nx = x + edges[n][0];
            const int ny = y + edges[n][1];
            const int nz = z + edges[n][2];
            const bool shell = nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
                            || nz == 0 || nz == Nm1;
            edge_sum += shell
                ? center - velocity * (sqrt(2.0) * inverse_measure / C_WAVE)
                : field[nx * L * L + ny * L + nz];
        }
    }
    if (stencil_mode == 3u) {
        for (int n = 0; n < 8; ++n) {
            const int nx = x + corners[n][0];
            const int ny = y + corners[n][1];
            const int nz = z + corners[n][2];
            const bool shell = nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
                            || nz == 0 || nz == Nm1;
            corner_sum += shell
                ? center - velocity * (sqrt(3.0) * inverse_measure / C_WAVE)
                : field[nx * L * L + ny * L + nz];
        }
    }

    if (stencil_mode == 1u) return W_SC_FACE * face_sum - center;
    if (stencil_mode == 2u) return W_FCC_EDGE * edge_sum - center;
    if (stencil_mode == 3u) return W_BCC_CORNER * corner_sum - center;
    return face_sum * (1.0 / 3.0) + edge_sum * (1.0 / 6.0)
         - 4.0 * center;
}

} // namespace kernels
} // namespace gpu
} // namespace ftd

#endif // FTD_KERNELS_STENCIL_COMMON_CUH
