/**
 * @file kernels_stencil.cu
 * @brief GPU kernels for Phase 1 (Read) and Phase 2 (Write) of the FTD tick cycle.
 *
 * [THEOREM] Phase Read: Isotropic 18-point Laplacian stencil + state-flux coupling.
 * [EXTENDED] Phase Write: Leapfrog integration, damping, near-particle mask,
 * genesis (stochastic manifestation), evaporation.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/sublattice.h"   // N_FACE, N_EDGE, N_CORNER, W_SC_FACE, W_FCC_EDGE, W_BCC_CORNER
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>   // fprintf — Linux/clang stricter than MSVC
#include <cstdlib>  // exit

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

namespace ftd {
namespace gpu {
namespace kernels {

// ---------- Device helpers ----------

__device__ __forceinline__
int wrap(int x, int L) {
    return ((x % L) + L) % L;
}

__device__ __forceinline__
int idx3d(int x, int y, int z, int L) {
    // CRIT-3 fix: match CPU X-major layout (was Z-major: z*L²+y*L+x)
    return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

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

// ---------- Phase Read Kernel ----------
// Computes delta_j = C_WAVE^2 * Laplacian(flux) + G_C * gradient(state)
//                   + G_C * curl(state * velocity)

__global__ void phase_read_kernel(
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const int8_t* __restrict__ state,
    const double* __restrict__ vel_x,
    const double* __restrict__ vel_y,
    const double* __restrict__ vel_z,
    double* __restrict__ djx,
    double* __restrict__ djy,
    double* __restrict__ djz,
    int L,
    bool do_wave,
    bool do_coupling,
    uint8_t bcc_stencil_mode    // Cluster A FTD-0093: 0=FULL, 1=SC, 2=FCC, 3=BCC
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    double dx = 0.0, dy = 0.0, dz = 0.0;

    if (do_wave) {
        // Cluster A: Laplacian dispatched by bcc_stencil_mode.
        //   0 (FULL) — legacy 18-pt: (1/3)·face + (1/6)·edge − 4·center
        //   1 (SC)   — 6 face nbrs only,    weight 1/6,  center −1
        //   2 (FCC)  — 12 edge nbrs only,   weight 1/12, center −1
        //   3 (BCC)  — 8 corner nbrs only,  weight 1/8,  center −1
        // See engine/include/ftd/sublattice.h for the closed-form weights and
        // Watson-integral pedigree (I_1_BCC = G*²/(2π)).
        double lap_x = 0.0, lap_y = 0.0, lap_z = 0.0;
        if (bcc_stencil_mode == 0u) {
            // FULL (legacy fast path)
            int xp = idx3d(x+1, y, z, L);
            int xm = idx3d(x-1, y, z, L);
            int yp = idx3d(x, y+1, z, L);
            int ym = idx3d(x, y-1, z, L);
            int zp = idx3d(x, y, z+1, L);
            int zm = idx3d(x, y, z-1, L);
            int xy_pp = idx3d(x+1,y+1,z,L), xy_pm = idx3d(x+1,y-1,z,L);
            int xy_mp = idx3d(x-1,y+1,z,L), xy_mm = idx3d(x-1,y-1,z,L);
            int xz_pp = idx3d(x+1,y,z+1,L), xz_pm = idx3d(x+1,y,z-1,L);
            int xz_mp = idx3d(x-1,y,z+1,L), xz_mm = idx3d(x-1,y,z-1,L);
            int yz_pp = idx3d(x,y+1,z+1,L), yz_pm = idx3d(x,y+1,z-1,L);
            int yz_mp = idx3d(x,y-1,z+1,L), yz_mm = idx3d(x,y-1,z-1,L);
            constexpr double WF = 1.0/3.0;
            constexpr double WE = 1.0/6.0;
            double face_x = flux_x[xp]+flux_x[xm]+flux_x[yp]+flux_x[ym]+flux_x[zp]+flux_x[zm];
            double edge_x = flux_x[xy_pp]+flux_x[xy_pm]+flux_x[xy_mp]+flux_x[xy_mm]
                          + flux_x[xz_pp]+flux_x[xz_pm]+flux_x[xz_mp]+flux_x[xz_mm]
                          + flux_x[yz_pp]+flux_x[yz_pm]+flux_x[yz_mp]+flux_x[yz_mm];
            double face_y = flux_y[xp]+flux_y[xm]+flux_y[yp]+flux_y[ym]+flux_y[zp]+flux_y[zm];
            double edge_y = flux_y[xy_pp]+flux_y[xy_pm]+flux_y[xy_mp]+flux_y[xy_mm]
                          + flux_y[xz_pp]+flux_y[xz_pm]+flux_y[xz_mp]+flux_y[xz_mm]
                          + flux_y[yz_pp]+flux_y[yz_pm]+flux_y[yz_mp]+flux_y[yz_mm];
            double face_z = flux_z[xp]+flux_z[xm]+flux_z[yp]+flux_z[ym]+flux_z[zp]+flux_z[zm];
            double edge_z = flux_z[xy_pp]+flux_z[xy_pm]+flux_z[xy_mp]+flux_z[xy_mm]
                          + flux_z[xz_pp]+flux_z[xz_pm]+flux_z[xz_mp]+flux_z[xz_mm]
                          + flux_z[yz_pp]+flux_z[yz_pm]+flux_z[yz_mp]+flux_z[yz_mm];
            lap_x = WF*face_x + WE*edge_x - 4.0*flux_x[i];
            lap_y = WF*face_y + WE*edge_y - 4.0*flux_y[i];
            lap_z = WF*face_z + WE*edge_z - 4.0*flux_z[i];
        } else if (bcc_stencil_mode == 1u) {
            // SC: N_FACE face nbrs, weight 1/N_FACE, center -1
            int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
            int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
            int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);
            constexpr double W = ftd::W_SC_FACE;     // 1.0 / N_FACE = 1/(2D)
            lap_x = W*(flux_x[xp]+flux_x[xm]+flux_x[yp]+flux_x[ym]+flux_x[zp]+flux_x[zm]) - flux_x[i];
            lap_y = W*(flux_y[xp]+flux_y[xm]+flux_y[yp]+flux_y[ym]+flux_y[zp]+flux_y[zm]) - flux_y[i];
            lap_z = W*(flux_z[xp]+flux_z[xm]+flux_z[yp]+flux_z[ym]+flux_z[zp]+flux_z[zm]) - flux_z[i];
        } else if (bcc_stencil_mode == 2u) {
            // FCC: N_EDGE edge nbrs, weight 1/N_EDGE, center -1
            int xy_pp = idx3d(x+1,y+1,z,L), xy_pm = idx3d(x+1,y-1,z,L);
            int xy_mp = idx3d(x-1,y+1,z,L), xy_mm = idx3d(x-1,y-1,z,L);
            int xz_pp = idx3d(x+1,y,z+1,L), xz_pm = idx3d(x+1,y,z-1,L);
            int xz_mp = idx3d(x-1,y,z+1,L), xz_mm = idx3d(x-1,y,z-1,L);
            int yz_pp = idx3d(x,y+1,z+1,L), yz_pm = idx3d(x,y+1,z-1,L);
            int yz_mp = idx3d(x,y-1,z+1,L), yz_mm = idx3d(x,y-1,z-1,L);
            constexpr double W = ftd::W_FCC_EDGE;    // 1.0 / N_EDGE = 1/(2D(D-1))
            lap_x = W*(flux_x[xy_pp]+flux_x[xy_pm]+flux_x[xy_mp]+flux_x[xy_mm]
                     + flux_x[xz_pp]+flux_x[xz_pm]+flux_x[xz_mp]+flux_x[xz_mm]
                     + flux_x[yz_pp]+flux_x[yz_pm]+flux_x[yz_mp]+flux_x[yz_mm]) - flux_x[i];
            lap_y = W*(flux_y[xy_pp]+flux_y[xy_pm]+flux_y[xy_mp]+flux_y[xy_mm]
                     + flux_y[xz_pp]+flux_y[xz_pm]+flux_y[xz_mp]+flux_y[xz_mm]
                     + flux_y[yz_pp]+flux_y[yz_pm]+flux_y[yz_mp]+flux_y[yz_mm]) - flux_y[i];
            lap_z = W*(flux_z[xy_pp]+flux_z[xy_pm]+flux_z[xy_mp]+flux_z[xy_mm]
                     + flux_z[xz_pp]+flux_z[xz_pm]+flux_z[xz_mp]+flux_z[xz_mm]
                     + flux_z[yz_pp]+flux_z[yz_pm]+flux_z[yz_mp]+flux_z[yz_mm]) - flux_z[i];
        } else {
            // BCC: N_CORNER corner nbrs (±1,±1,±1), weight 1/N_CORNER, center -1
            int c_ppp = idx3d(x+1,y+1,z+1,L), c_ppm = idx3d(x+1,y+1,z-1,L);
            int c_pmp = idx3d(x+1,y-1,z+1,L), c_pmm = idx3d(x+1,y-1,z-1,L);
            int c_mpp = idx3d(x-1,y+1,z+1,L), c_mpm = idx3d(x-1,y+1,z-1,L);
            int c_mmp = idx3d(x-1,y-1,z+1,L), c_mmm = idx3d(x-1,y-1,z-1,L);
            constexpr double W = ftd::W_BCC_CORNER;  // 1.0 / N_CORNER = 1/2^D
            lap_x = W*(flux_x[c_ppp]+flux_x[c_ppm]+flux_x[c_pmp]+flux_x[c_pmm]
                     + flux_x[c_mpp]+flux_x[c_mpm]+flux_x[c_mmp]+flux_x[c_mmm]) - flux_x[i];
            lap_y = W*(flux_y[c_ppp]+flux_y[c_ppm]+flux_y[c_pmp]+flux_y[c_pmm]
                     + flux_y[c_mpp]+flux_y[c_mpm]+flux_y[c_mmp]+flux_y[c_mmm]) - flux_y[i];
            lap_z = W*(flux_z[c_ppp]+flux_z[c_ppm]+flux_z[c_pmp]+flux_z[c_pmm]
                     + flux_z[c_mpp]+flux_z[c_mpm]+flux_z[c_mmp]+flux_z[c_mmm]) - flux_z[i];
        }

        constexpr double cw2 = C_WAVE * C_WAVE;
        dx += cw2 * lap_x;
        dy += cw2 * lap_y;
        dz += cw2 * lap_z;
    }

    if (do_coupling) {
        // Gradient of state: g_c * ∇(s)
        int xp = idx3d(x+1, y, z, L);
        int xm = idx3d(x-1, y, z, L);
        int yp = idx3d(x, y+1, z, L);
        int ym = idx3d(x, y-1, z, L);
        int zp = idx3d(x, y, z+1, L);
        int zm = idx3d(x, y, z-1, L);

        double gs_x = 0.5 * (static_cast<double>(state[xp]) - static_cast<double>(state[xm]));
        double gs_y = 0.5 * (static_cast<double>(state[yp]) - static_cast<double>(state[ym]));
        double gs_z = 0.5 * (static_cast<double>(state[zp]) - static_cast<double>(state[zm]));

        dx += G_C * gs_x;
        dy += G_C * gs_y;
        dz += G_C * gs_z;

        // Curl of (state * velocity): g_c * ∇×(s·v)
        // (∇×F)_x = dFz/dy - dFy/dz, etc.
        // F_i = state * velocity_i at each site
        auto sv = [&](int idx_j, int comp) -> double {
            double s = static_cast<double>(state[idx_j]);
            if (comp == 0) return s * vel_x[idx_j];
            if (comp == 1) return s * vel_y[idx_j];
            return s * vel_z[idx_j];
        };

        double curl_x = 0.5 * (sv(yp, 2) - sv(ym, 2)) - 0.5 * (sv(zp, 1) - sv(zm, 1));
        double curl_y = 0.5 * (sv(zp, 0) - sv(zm, 0)) - 0.5 * (sv(xp, 2) - sv(xm, 2));
        double curl_z = 0.5 * (sv(xp, 1) - sv(xm, 1)) - 0.5 * (sv(yp, 0) - sv(ym, 0));

        dx += G_C * curl_x;
        dy += G_C * curl_y;
        dz += G_C * curl_z;
    }

    djx[i] = dx;
    djy[i] = dy;
    djz[i] = dz;
}

// ---------- Near-Particle Mask + Larmor Accel Kernel ----------
// Computes near_particle mask and, when do_larmor=true, also propagates
// the max accel_mag of nearby particles to each near-particle site.

__global__ void compute_near_particle_kernel(
    const int8_t* __restrict__ state,
    const double* __restrict__ accel_mag,
    uint8_t* __restrict__ near_particle,
    double* __restrict__ near_accel,
    bool do_larmor,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // Check self and 6 face neighbors for particles
    int nbrs[6] = {
        idx3d(x+1,y,z,L), idx3d(x-1,y,z,L),
        idx3d(x,y+1,z,L), idx3d(x,y-1,z,L),
        idx3d(x,y,z+1,L), idx3d(x,y,z-1,L)
    };

    bool near = (state[i] != 0);
    double max_a = 0.0;

    if (near && do_larmor) {
        max_a = accel_mag[i];
    }

    for (int n = 0; n < 6; ++n) {
        int j = nbrs[n];
        if (state[j] != 0) {
            near = true;
            if (do_larmor) {
                double a = accel_mag[j];
                if (a > max_a) max_a = a;
            }
        }
    }

    near_particle[i] = near ? 1 : 0;
    if (do_larmor) {
        near_accel[i] = max_a;
    }
}

// ---------- Phase Write Kernel ----------
// Leapfrog integration + conditional damping (with optional Larmor modulation)

// Cluster A site filter:  0=ALL_SITES, 1=BCC_SITES (odd,odd,odd),
// 2=FCC_SITES (mixed parity), 3=SC_SITES (even,even,even).
// Matches ftd::SiteClass at the value level: SC_SITES=0, BCC_SITES=1,
// FCC_SITES=2, ALL_SITES=3 — see sublattice.h. We dispatch by passing
// the SiteClass uint8_t value directly.
__device__ inline bool langevin_site_match(int x, int y, int z, uint8_t filter) {
    if (filter == 3) return true;                              // ALL_SITES
    const int px = x & 1, py = y & 1, pz = z & 1;
    const bool is_sc  = (px == 0 && py == 0 && pz == 0);
    const bool is_bcc = (px == 1 && py == 1 && pz == 1);
    if (filter == 0) return is_sc;                              // SC_SITES
    if (filter == 1) return is_bcc;                             // BCC_SITES
    return !is_sc && !is_bcc;                                   // FCC_SITES = remainder
}

__global__ void phase_write_kernel(
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    double* __restrict__ wv_x,
    double* __restrict__ wv_y,
    double* __restrict__ wv_z,
    const double* __restrict__ djx,
    const double* __restrict__ djy,
    const double* __restrict__ djz,
    const uint8_t* __restrict__ near_particle,
    const double* __restrict__ near_accel,
    bool do_damping,
    bool selective_damping,
    bool do_larmor,
    double damp,
    // Langevin thermostat (FTD-0051).
    bool do_langevin,
    double langevin_gamma,
    double langevin_T,
    const double* __restrict__ langevin_noise,
    uint8_t langevin_site_filter,    // Cluster A FTD-0093: 0=SC, 1=BCC, 2=FCC, 3=ALL
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // Leapfrog: wave_vel += delta_j; flux += wave_vel
    wv_x[i] += djx[i];
    wv_y[i] += djy[i];
    wv_z[i] += djz[i];

    flux_x[i] += wv_x[i];
    flux_y[i] += wv_y[i];
    flux_z[i] += wv_z[i];

    // Cluster A: Langevin OU update gated on site-class filter.
    const bool langevin_active =
        do_langevin && langevin_site_match(x, y, z, langevin_site_filter);

    if (langevin_active) {
        // Ornstein-Uhlenbeck on wave_vel; replaces deterministic damping.
        const int N = L * L * L;
        const double one_minus_gamma = 1.0 - langevin_gamma;
        const double sigma = sqrt(2.0 * langevin_gamma * langevin_T);
        wv_x[i] = one_minus_gamma * wv_x[i] + sigma * langevin_noise[0*N + i];
        wv_y[i] = one_minus_gamma * wv_y[i] + sigma * langevin_noise[1*N + i];
        wv_z[i] = one_minus_gamma * wv_z[i] + sigma * langevin_noise[2*N + i];
    } else if (do_damping) {
        const bool should_damp = !selective_damping || (near_particle[i] != 0);
        if (should_damp) {
            const double eff_damp = effective_damping(i, damp, do_larmor,
                                                       selective_damping,
                                                       near_particle, near_accel);
            scale_field_pair(flux_x[i], flux_y[i], flux_z[i],
                              wv_x[i], wv_y[i], wv_z[i],
                              eff_damp);
        }
    }
}

// ---------- Fused Wave Update Kernel (phase_read + phase_write) ----------
// Computes Laplacian + coupling in registers and immediately applies leapfrog,
// eliminating the intermediate delta_j global memory round-trip.
// Single-substrate only; dual-substrate uses separate read/write kernels.

__global__ void wave_update_kernel(
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    double* __restrict__ wv_x,
    double* __restrict__ wv_y,
    double* __restrict__ wv_z,
    const int8_t* __restrict__ state,
    const double* __restrict__ vel_x,
    const double* __restrict__ vel_y,
    const double* __restrict__ vel_z,
    const uint8_t* __restrict__ near_particle,
    const double* __restrict__ near_accel,
    bool do_wave,
    bool do_coupling,
    bool do_damping,
    bool selective_damping,
    bool do_larmor,
    double damp,
    // Langevin thermostat (FTD-0051). When do_langevin is true the OU step
    // replaces the deterministic damping block:
    //    v <- (1 - gamma) v + sqrt(2 gamma T) * eta,   eta ~ N(0,1) per comp.
    // Noise buffer layout: [3*N] doubles, flattened as (comp, i) with
    // comp ∈ {0=x, 1=y, 2=z}; stride N per component.
    bool do_langevin,
    double langevin_gamma,
    double langevin_T,
    const double* __restrict__ langevin_noise,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // --- Phase Read: compute delta_j in registers ---
    double djx = 0.0, djy = 0.0, djz = 0.0;

    // Neighbor indices (shared between wave and coupling)
    int xp = idx3d(x+1, y, z, L);
    int xm = idx3d(x-1, y, z, L);
    int yp = idx3d(x, y+1, z, L);
    int ym = idx3d(x, y-1, z, L);
    int zp = idx3d(x, y, z+1, L);
    int zm = idx3d(x, y, z-1, L);

    if (do_wave) {
        // 12 edge neighbors
        int xy_pp = idx3d(x+1,y+1,z,L), xy_pm = idx3d(x+1,y-1,z,L);
        int xy_mp = idx3d(x-1,y+1,z,L), xy_mm = idx3d(x-1,y-1,z,L);
        int xz_pp = idx3d(x+1,y,z+1,L), xz_pm = idx3d(x+1,y,z-1,L);
        int xz_mp = idx3d(x-1,y,z+1,L), xz_mm = idx3d(x-1,y,z-1,L);
        int yz_pp = idx3d(x,y+1,z+1,L), yz_pm = idx3d(x,y+1,z-1,L);
        int yz_mp = idx3d(x,y-1,z+1,L), yz_mm = idx3d(x,y-1,z-1,L);

        constexpr double WF = 1.0/3.0;
        constexpr double WE = 1.0/6.0;
        constexpr double cw2 = C_WAVE * C_WAVE;

        double face_x = flux_x[xp] + flux_x[xm] + flux_x[yp] + flux_x[ym]
                       + flux_x[zp] + flux_x[zm];
        double edge_x = flux_x[xy_pp] + flux_x[xy_pm] + flux_x[xy_mp] + flux_x[xy_mm]
                      + flux_x[xz_pp] + flux_x[xz_pm] + flux_x[xz_mp] + flux_x[xz_mm]
                      + flux_x[yz_pp] + flux_x[yz_pm] + flux_x[yz_mp] + flux_x[yz_mm];
        djx += cw2 * (WF * face_x + WE * edge_x - 4.0 * flux_x[i]);

        double face_y = flux_y[xp] + flux_y[xm] + flux_y[yp] + flux_y[ym]
                       + flux_y[zp] + flux_y[zm];
        double edge_y = flux_y[xy_pp] + flux_y[xy_pm] + flux_y[xy_mp] + flux_y[xy_mm]
                      + flux_y[xz_pp] + flux_y[xz_pm] + flux_y[xz_mp] + flux_y[xz_mm]
                      + flux_y[yz_pp] + flux_y[yz_pm] + flux_y[yz_mp] + flux_y[yz_mm];
        djy += cw2 * (WF * face_y + WE * edge_y - 4.0 * flux_y[i]);

        double face_z = flux_z[xp] + flux_z[xm] + flux_z[yp] + flux_z[ym]
                       + flux_z[zp] + flux_z[zm];
        double edge_z = flux_z[xy_pp] + flux_z[xy_pm] + flux_z[xy_mp] + flux_z[xy_mm]
                      + flux_z[xz_pp] + flux_z[xz_pm] + flux_z[xz_mp] + flux_z[xz_mm]
                      + flux_z[yz_pp] + flux_z[yz_pm] + flux_z[yz_mp] + flux_z[yz_mm];
        djz += cw2 * (WF * face_z + WE * edge_z - 4.0 * flux_z[i]);
    }

    if (do_coupling) {
        double gs_x = 0.5 * (static_cast<double>(state[xp]) - static_cast<double>(state[xm]));
        double gs_y = 0.5 * (static_cast<double>(state[yp]) - static_cast<double>(state[ym]));
        double gs_z = 0.5 * (static_cast<double>(state[zp]) - static_cast<double>(state[zm]));

        djx += G_C * gs_x;
        djy += G_C * gs_y;
        djz += G_C * gs_z;

        auto sv = [&](int idx_j, int comp) -> double {
            double s = static_cast<double>(state[idx_j]);
            if (comp == 0) return s * vel_x[idx_j];
            if (comp == 1) return s * vel_y[idx_j];
            return s * vel_z[idx_j];
        };

        double curl_x = 0.5 * (sv(yp, 2) - sv(ym, 2)) - 0.5 * (sv(zp, 1) - sv(zm, 1));
        double curl_y = 0.5 * (sv(zp, 0) - sv(zm, 0)) - 0.5 * (sv(xp, 2) - sv(xm, 2));
        double curl_z = 0.5 * (sv(xp, 1) - sv(xm, 1)) - 0.5 * (sv(yp, 0) - sv(ym, 0));

        djx += G_C * curl_x;
        djy += G_C * curl_y;
        djz += G_C * curl_z;
    }

    // --- Phase Write: leapfrog integration + damping or Langevin OU ---
    wv_x[i] += djx;
    wv_y[i] += djy;
    wv_z[i] += djz;

    flux_x[i] += wv_x[i];
    flux_y[i] += wv_y[i];
    flux_z[i] += wv_z[i];

    if (do_langevin) {
        // Ornstein-Uhlenbeck on wave_vel per-component. Replaces deterministic
        // damping. Field rescaling of J is NOT applied here (Langevin acts on
        // the "momentum" DoF; position J thermalizes via coupled dynamics).
        const int N = L * L * L;
        const double one_minus_gamma = 1.0 - langevin_gamma;
        const double sigma = sqrt(2.0 * langevin_gamma * langevin_T);
        wv_x[i] = one_minus_gamma * wv_x[i] + sigma * langevin_noise[0*N + i];
        wv_y[i] = one_minus_gamma * wv_y[i] + sigma * langevin_noise[1*N + i];
        wv_z[i] = one_minus_gamma * wv_z[i] + sigma * langevin_noise[2*N + i];
    } else if (do_damping) {
        bool should_damp = !selective_damping || (near_particle[i] != 0);
        if (should_damp) {
            double eff_damp = damp;
            if (do_larmor && selective_damping && near_particle[i]) {
                double a2 = near_accel[i] * near_accel[i];
                double larmor_mod = fmin(1.0, LARMOR_FLOOR + K_LARMOR * a2);
                eff_damp = 1.0 - DAMPING * larmor_mod;
            }
            flux_x[i] *= eff_damp;
            flux_y[i] *= eff_damp;
            flux_z[i] *= eff_damp;
            wv_x[i] *= eff_damp;
            wv_y[i] *= eff_damp;
            wv_z[i] *= eff_damp;
        }
    }
}

// ---------- Genesis Kernel ----------
// Stochastic manifestation: void sites with density > K_GENESIS may manifest

__global__ void genesis_kernel(
    int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const double* __restrict__ random,  // pre-generated uniform [0,1)
    int8_t* __restrict__ spin,
    int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int* __restrict__ ledger_reaction,
    int next_pid,  // starting particle ID for this batch
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] != 0) return;  // already manifested

    double fx = flux_x[i], fy = flux_y[i], fz = flux_z[i];
    double density = sqrt(fx*fx + fy*fy + fz*fz);

    constexpr double k_genesis = 3.0 * K_B;
    if (density <= k_genesis) return;

    // Exponential CDF genesis probability (matches CPU)
    // p = 1 - exp(-(density - K_GENESIS) / K_B)
    double z_gen = density - k_genesis;
    double p = 1.0 - exp(-z_gen / K_B);
    if (random[i] >= p) return;

    // Determine polarity from divergence sign
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    double div = 0.5 * ((flux_x[xp] - flux_x[xm])
                       + (flux_y[yp] - flux_y[ym])
                       + (flux_z[zp] - flux_z[zm]));

    const int8_t new_state = (div > 0) ? 1 : -1;  // Matches CPU: strictly > 0 (not >=)
    state[i] = new_state;
    if (ledger_reaction) {
        atomicAdd(&ledger_reaction[i], static_cast<int>(new_state));
    }

    // Assign spin from dominant curl component (z-first priority, matches CPU)
    double curl_x = 0.5 * ((flux_z[yp] - flux_z[ym]) - (flux_y[zp] - flux_y[zm]));
    double curl_y = 0.5 * ((flux_x[zp] - flux_x[zm]) - (flux_z[xp] - flux_z[xm]));
    double curl_z = 0.5 * ((flux_y[xp] - flux_y[xm]) - (flux_x[yp] - flux_x[ym]));

    double acx = fabs(curl_x), acy = fabs(curl_y), acz = fabs(curl_z);
    double max_curl = fmax(acx, fmax(acy, acz));
    if (max_curl > 1e-15) {
        // z-first priority to match CPU render_bridge.cpp
        if (acz >= acx && acz >= acy) spin[i] = (curl_z > 0) ? 1 : -1;
        else if (acy >= acx)          spin[i] = (curl_y > 0) ? 1 : -1;
        else                          spin[i] = (curl_x > 0) ? 1 : -1;
    } else {
        // Zero curl: assign +1 (deterministic fallback, matches CPU's random with seed)
        spin[i] = 1;
    }

    // Assign color from dominant flux axis
    double afx = fabs(fx), afy = fabs(fy), afz = fabs(fz);
    if (afx >= afy && afx >= afz) color[i] = 1;       // red
    else if (afy >= afz)          color[i] = 2;        // green
    else                          color[i] = 3;        // blue

    // Particle ID: use lattice index as unique ID (no two particles share a site)
    particle_id[i] = i;
}

// ---------- Evaporation Kernel ----------

__global__ void evaporation_kernel(
    int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const double* __restrict__ wv_x,
    const double* __restrict__ wv_y,
    const double* __restrict__ wv_z,
    const uint8_t* __restrict__ locked,
    int8_t* __restrict__ spin,
    int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int* __restrict__ ledger_reaction,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] == 0 || locked[i]) return;

    // Neighborhood energy check (same as CPU: particle + 6 face neighbors)
    double local_energy = flux_x[i]*flux_x[i] + flux_y[i]*flux_y[i] + flux_z[i]*flux_z[i]
                        + wv_x[i]*wv_x[i] + wv_y[i]*wv_y[i] + wv_z[i]*wv_z[i];

    int nbrs[6] = {
        idx3d(x+1,y,z,L), idx3d(x-1,y,z,L),
        idx3d(x,y+1,z,L), idx3d(x,y-1,z,L),
        idx3d(x,y,z+1,L), idx3d(x,y,z-1,L)
    };
    for (int n = 0; n < 6; ++n) {
        int j = nbrs[n];
        local_energy += flux_x[j]*flux_x[j] + flux_y[j]*flux_y[j] + flux_z[j]*flux_z[j]
                      + wv_x[j]*wv_x[j] + wv_y[j]*wv_y[j] + wv_z[j]*wv_z[j];
    }

    constexpr double EVAP_THRESHOLD = K_B * K_B * 1e-6;
    if (local_energy < EVAP_THRESHOLD) {
        const int8_t old_state = state[i];
        state[i] = 0;
        if (ledger_reaction) {
            atomicAdd(&ledger_reaction[i], -static_cast<int>(old_state));
        }
        spin[i] = 0;
        color[i] = 0;
        particle_id[i] = -1;
    }
}

// ============================================================================
// DUAL-SUBSTRATE KERNELS
// ============================================================================

// ---------- Dual Phase Read Kernel ----------
// Computes independent Laplacians on L and R substrates, splits coupling 50/50

__global__ void phase_read_dual_kernel(
    const double* __restrict__ fL_x, const double* __restrict__ fL_y, const double* __restrict__ fL_z,
    const double* __restrict__ fR_x, const double* __restrict__ fR_y, const double* __restrict__ fR_z,
    const int8_t* __restrict__ state,
    const double* __restrict__ vel_x, const double* __restrict__ vel_y, const double* __restrict__ vel_z,
    double* __restrict__ djL_x, double* __restrict__ djL_y, double* __restrict__ djL_z,
    double* __restrict__ djR_x, double* __restrict__ djR_y, double* __restrict__ djR_z,
    int L, bool do_wave, bool do_coupling
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    double dLx = 0, dLy = 0, dLz = 0;
    double dRx = 0, dRy = 0, dRz = 0;

    if (do_wave) {
        // Isotropic 18-point Laplacian on each substrate independently
        int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
        int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
        int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);
        int xy_pp = idx3d(x+1,y+1,z,L), xy_pm = idx3d(x+1,y-1,z,L);
        int xy_mp = idx3d(x-1,y+1,z,L), xy_mm = idx3d(x-1,y-1,z,L);
        int xz_pp = idx3d(x+1,y,z+1,L), xz_pm = idx3d(x+1,y,z-1,L);
        int xz_mp = idx3d(x-1,y,z+1,L), xz_mm = idx3d(x-1,y,z-1,L);
        int yz_pp = idx3d(x,y+1,z+1,L), yz_pm = idx3d(x,y+1,z-1,L);
        int yz_mp = idx3d(x,y-1,z+1,L), yz_mm = idx3d(x,y-1,z-1,L);

        constexpr double WF = 1.0/3.0, WE = 1.0/6.0;
        constexpr double cw2 = C_WAVE * C_WAVE;

        // Macro for 18-point Laplacian on a single component array
        #define LAP18(arr, idx_center) \
            (WF * (arr[xp] + arr[xm] + arr[yp] + arr[ym] + arr[zp] + arr[zm]) \
           + WE * (arr[xy_pp] + arr[xy_pm] + arr[xy_mp] + arr[xy_mm] \
                 + arr[xz_pp] + arr[xz_pm] + arr[xz_mp] + arr[xz_mm] \
                 + arr[yz_pp] + arr[yz_pm] + arr[yz_mp] + arr[yz_mm]) \
           - 4.0 * arr[idx_center])

        dLx += cw2 * LAP18(fL_x, i);
        dLy += cw2 * LAP18(fL_y, i);
        dLz += cw2 * LAP18(fL_z, i);
        dRx += cw2 * LAP18(fR_x, i);
        dRy += cw2 * LAP18(fR_y, i);
        dRz += cw2 * LAP18(fR_z, i);

        #undef LAP18
    }

    if (do_coupling) {
        // Split coupling source 50/50 between L and R
        int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
        int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
        int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

        double gs_x = 0.5 * (static_cast<double>(state[xp]) - static_cast<double>(state[xm]));
        double gs_y = 0.5 * (static_cast<double>(state[yp]) - static_cast<double>(state[ym]));
        double gs_z = 0.5 * (static_cast<double>(state[zp]) - static_cast<double>(state[zm]));

        double half_gc = 0.5 * G_C;
        dLx += half_gc * gs_x;  dLy += half_gc * gs_y;  dLz += half_gc * gs_z;
        dRx += half_gc * gs_x;  dRy += half_gc * gs_y;  dRz += half_gc * gs_z;

        // Split curl coupling 50/50
        auto sv = [&](int idx_j, int comp) -> double {
            double s = static_cast<double>(state[idx_j]);
            if (comp == 0) return s * vel_x[idx_j];
            if (comp == 1) return s * vel_y[idx_j];
            return s * vel_z[idx_j];
        };
        double cx = 0.5 * (sv(yp,2) - sv(ym,2)) - 0.5 * (sv(zp,1) - sv(zm,1));
        double cy = 0.5 * (sv(zp,0) - sv(zm,0)) - 0.5 * (sv(xp,2) - sv(xm,2));
        double cz = 0.5 * (sv(xp,1) - sv(xm,1)) - 0.5 * (sv(yp,0) - sv(ym,0));

        dLx += half_gc * cx;  dLy += half_gc * cy;  dLz += half_gc * cz;
        dRx += half_gc * cx;  dRy += half_gc * cy;  dRz += half_gc * cz;
    }

    djL_x[i] = dLx;  djL_y[i] = dLy;  djL_z[i] = dLz;
    djR_x[i] = dRx;  djR_y[i] = dRy;  djR_z[i] = dRz;
}

// ---------- Strong Field (Stella Octangula) Kernel ----------
// Propagates strong flux along the 8 vertex neighbors
__global__ void strong_field_stencil_kernel(
    double* __restrict__ fs_x, double* __restrict__ fs_y, double* __restrict__ fs_z,
    double* __restrict__ wvs_x, double* __restrict__ wvs_y, double* __restrict__ wvs_z,
    const int8_t* __restrict__ state, const int8_t* __restrict__ color,
    double damp, int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;

    // 8 vertex neighbors
    int v1 = idx3d(x+1, y+1, z+1, L);
    int v2 = idx3d(x+1, y+1, z-1, L);
    int v3 = idx3d(x+1, y-1, z+1, L);
    int v4 = idx3d(x+1, y-1, z-1, L);
    int v5 = idx3d(x-1, y+1, z+1, L);
    int v6 = idx3d(x-1, y+1, z-1, L);
    int v7 = idx3d(x-1, y-1, z+1, L);
    int v8 = idx3d(x-1, y-1, z-1, L);

    // Laplacian for vertex neighbors: (1/8) * sum_vertices - center
    constexpr double WV = 1.0 / 8.0;
    constexpr double cw2 = C_WAVE * C_WAVE;

    double lap_x = WV * (fs_x[v1] + fs_x[v2] + fs_x[v3] + fs_x[v4] + fs_x[v5] + fs_x[v6] + fs_x[v7] + fs_x[v8]) - fs_x[i];
    double lap_y = WV * (fs_y[v1] + fs_y[v2] + fs_y[v3] + fs_y[v4] + fs_y[v5] + fs_y[v6] + fs_y[v7] + fs_y[v8]) - fs_y[i];
    double lap_z = WV * (fs_z[v1] + fs_z[v2] + fs_z[v3] + fs_z[v4] + fs_z[v5] + fs_z[v6] + fs_z[v7] + fs_z[v8]) - fs_z[i];

    double djx = cw2 * lap_x;
    double djy = cw2 * lap_y;
    double djz = cw2 * lap_z;

    // Source term: color charge acts as gradient source along its respective axis
    // color: 1=red(x), 2=green(y), 3=blue(z)
    int8_t c = color[i];
    if (state[i] != 0 && c > 0) {
        // VERTEX_GAUGE = (11/6) / sqrt(8) — Watson c3 loop gauge, stella-octangula
        // Single source of truth: include/ftd/constants_gpu.cuh
        double src = G_C * state[i] * VERTEX_GAUGE;
        if (c == 1) djx += src;
        else if (c == 2) djy += src;
        else if (c == 3) djz += src;
    }

    wvs_x[i] += djx;
    wvs_y[i] += djy;
    wvs_z[i] += djz;

    // Implicit leapfrog write
    fs_x[i] += wvs_x[i];
    fs_y[i] += wvs_y[i];
    fs_z[i] += wvs_z[i];

    // Damping
    fs_x[i] *= damp;
    fs_y[i] *= damp;
    fs_z[i] *= damp;
    wvs_x[i] *= damp;
    wvs_y[i] *= damp;
    wvs_z[i] *= damp;
}

// ---------- Weak Field (Cuboctahedron) Kernel ----------
// Propagates weak flux along the 12 edge neighbors
__global__ void weak_field_stencil_kernel(
    double* __restrict__ fw_x, double* __restrict__ fw_y, double* __restrict__ fw_z,
    double* __restrict__ wvw_x, double* __restrict__ wvw_y, double* __restrict__ wvw_z,
    const int8_t* __restrict__ state, const int8_t* __restrict__ flavor,
    double damp, int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;

    // 12 edge neighbors
    int e1 = idx3d(x+1, y+1, z, L);  int e2 = idx3d(x+1, y-1, z, L);
    int e3 = idx3d(x-1, y+1, z, L);  int e4 = idx3d(x-1, y-1, z, L);
    int e5 = idx3d(x+1, y, z+1, L);  int e6 = idx3d(x+1, y, z-1, L);
    int e7 = idx3d(x-1, y, z+1, L);  int e8 = idx3d(x-1, y, z-1, L);
    int e9 = idx3d(x, y+1, z+1, L);  int e10= idx3d(x, y+1, z-1, L);
    int e11= idx3d(x, y-1, z+1, L);  int e12= idx3d(x, y-1, z-1, L);

    constexpr double WV = 1.0 / 12.0;
    constexpr double cw2 = C_WAVE * C_WAVE;

    double lap_x = WV * (fw_x[e1]+fw_x[e2]+fw_x[e3]+fw_x[e4]+fw_x[e5]+fw_x[e6]+fw_x[e7]+fw_x[e8]+fw_x[e9]+fw_x[e10]+fw_x[e11]+fw_x[e12]) - fw_x[i];
    double lap_y = WV * (fw_y[e1]+fw_y[e2]+fw_y[e3]+fw_y[e4]+fw_y[e5]+fw_y[e6]+fw_y[e7]+fw_y[e8]+fw_y[e9]+fw_y[e10]+fw_y[e11]+fw_y[e12]) - fw_y[i];
    double lap_z = WV * (fw_z[e1]+fw_z[e2]+fw_z[e3]+fw_z[e4]+fw_z[e5]+fw_z[e6]+fw_z[e7]+fw_z[e8]+fw_z[e9]+fw_z[e10]+fw_z[e11]+fw_z[e12]) - fw_z[i];

    double djx = cw2 * lap_x;
    double djy = cw2 * lap_y;
    double djz = cw2 * lap_z;

    // Source term: flavor (chirality) acts as isotropic source
    if (state[i] != 0 && flavor[i] != 0) {
        // EDGE_GAUGE = (13/9) / sqrt(12) — Watson c2 loop gauge, cuboctahedron
        // Single source of truth: include/ftd/constants_gpu.cuh
        double src = G_C * state[i] * flavor[i] * EDGE_GAUGE;
        djx += src;
        djy += src;
        djz += src;
    }

    wvw_x[i] += djx;
    wvw_y[i] += djy;
    wvw_z[i] += djz;

    // Implicit leapfrog write
    fw_x[i] += wvw_x[i];
    fw_y[i] += wvw_y[i];
    fw_z[i] += wvw_z[i];

    // Damping
    fw_x[i] *= damp;
    fw_y[i] *= damp;
    fw_z[i] *= damp;
    wvw_x[i] *= damp;
    wvw_y[i] *= damp;
    wvw_z[i] *= damp;
}

// ---------- Dual Phase Write Kernel ----------
// Independent leapfrog on L/R, sync observable (flux = L + R)

__global__ void phase_write_dual_kernel(
    double* __restrict__ fL_x, double* __restrict__ fL_y, double* __restrict__ fL_z,
    double* __restrict__ fR_x, double* __restrict__ fR_y, double* __restrict__ fR_z,
    double* __restrict__ wvL_x, double* __restrict__ wvL_y, double* __restrict__ wvL_z,
    double* __restrict__ wvR_x, double* __restrict__ wvR_y, double* __restrict__ wvR_z,
    const double* __restrict__ djL_x, const double* __restrict__ djL_y, const double* __restrict__ djL_z,
    const double* __restrict__ djR_x, const double* __restrict__ djR_y, const double* __restrict__ djR_z,
    double* __restrict__ obs_x, double* __restrict__ obs_y, double* __restrict__ obs_z,
    double* __restrict__ wv_x, double* __restrict__ wv_y, double* __restrict__ wv_z,
    const uint8_t* __restrict__ near_particle,
    const double* __restrict__ near_accel,
    bool do_damping, bool selective_damping, bool do_larmor, double damp,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // Independent leapfrog on L
    wvL_x[i] += djL_x[i];  wvL_y[i] += djL_y[i];  wvL_z[i] += djL_z[i];
    fL_x[i] += wvL_x[i];   fL_y[i] += wvL_y[i];   fL_z[i] += wvL_z[i];

    // Independent leapfrog on R
    wvR_x[i] += djR_x[i];  wvR_y[i] += djR_y[i];  wvR_z[i] += djR_z[i];
    fR_x[i] += wvR_x[i];   fR_y[i] += wvR_y[i];   fR_z[i] += wvR_z[i];

    // Conditional damping on L and R independently
    if (do_damping) {
        const bool should = !selective_damping || (near_particle[i] != 0);
        if (should) {
            const double eff_damp = effective_damping(i, damp, do_larmor,
                                                       selective_damping,
                                                       near_particle, near_accel);
            scale_field_pair(fL_x[i], fL_y[i], fL_z[i],
                              wvL_x[i], wvL_y[i], wvL_z[i],
                              eff_damp);
            scale_field_pair(fR_x[i], fR_y[i], fR_z[i],
                              wvR_x[i], wvR_y[i], wvR_z[i],
                              eff_damp);
        }
    }

    // Sync observable: flux = L + R, wave_vel = L + R
    obs_x[i] = fL_x[i] + fR_x[i];
    obs_y[i] = fL_y[i] + fR_y[i];
    obs_z[i] = fL_z[i] + fR_z[i];
    wv_x[i] = wvL_x[i] + wvR_x[i];
    wv_y[i] = wvL_y[i] + wvR_y[i];
    wv_z[i] = wvL_z[i] + wvR_z[i];
}

// ---------- Dual Genesis Kernel (chirality-based polarity) ----------

__global__ void genesis_dual_kernel(
    int8_t* __restrict__ state,
    const double* __restrict__ fL_x, const double* __restrict__ fL_y, const double* __restrict__ fL_z,
    const double* __restrict__ fR_x, const double* __restrict__ fR_y, const double* __restrict__ fR_z,
    const double* __restrict__ obs_x, const double* __restrict__ obs_y, const double* __restrict__ obs_z,
    const double* __restrict__ random,
    int8_t* __restrict__ spin, int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int* __restrict__ ledger_reaction,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] != 0) return;

    // Observable density
    double fx = obs_x[i], fy = obs_y[i], fz = obs_z[i];
    double density = sqrt(fx*fx + fy*fy + fz*fz);

    constexpr double k_genesis = 3.0 * K_B;
    if (density <= k_genesis) return;

    // Exponential CDF genesis probability (matches CPU, dual-substrate)
    double z_gen = density - k_genesis;
    double p = 1.0 - exp(-z_gen / K_B);
    if (random[i] >= p) return;

    // Polarity from chirality (|psi_L|^2 - |psi_R|^2)
    double psiL2 = fL_x[i]*fL_x[i] + fL_y[i]*fL_y[i];
    double psiR2 = fR_x[i]*fR_x[i] + fR_y[i]*fR_y[i];
    double chi = psiL2 - psiR2;
    const int8_t new_state = (chi >= 0) ? 1 : -1;
    state[i] = new_state;
    if (ledger_reaction) {
        atomicAdd(&ledger_reaction[i], static_cast<int>(new_state));
    }

    // Spin from curl of observable
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    double curl_x = 0.5*((obs_z[yp]-obs_z[ym]) - (obs_y[zp]-obs_y[zm]));
    double curl_y = 0.5*((obs_x[zp]-obs_x[zm]) - (obs_z[xp]-obs_z[xm]));
    double curl_z = 0.5*((obs_y[xp]-obs_y[xm]) - (obs_x[yp]-obs_x[ym]));

    double max_curl = fmax(fabs(curl_x), fmax(fabs(curl_y), fabs(curl_z)));
    if (max_curl > 1e-15) {
        double dominant = (fabs(curl_x) >= fabs(curl_y) && fabs(curl_x) >= fabs(curl_z)) ? curl_x
                        : (fabs(curl_y) >= fabs(curl_z)) ? curl_y : curl_z;
        spin[i] = (dominant > 0) ? 1 : -1;
    }

    // Color from dominant observable axis
    double afx = fabs(fx), afy = fabs(fy), afz = fabs(fz);
    if (afx >= afy && afx >= afz) color[i] = 1;
    else if (afy >= afz) color[i] = 2;
    else color[i] = 3;

    particle_id[i] = i;
}

// ---------- Launcher Functions ----------

void launch_phase_read(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                        uint8_t bcc_stencil_mode) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);

    phase_read_kernel<<<grid, block>>>(
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
        L, do_wave, do_coupling, bcc_stencil_mode
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_phase_write(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                        bool larmor_radiation, double damping_factor,
                        bool do_genesis, double dt,
                        bool do_langevin, double langevin_gamma, double langevin_T,
                        uint8_t langevin_site_filter) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);

    // Compute near-particle mask (+ Larmor accel) if selective damping
    if (selective_damping) {
        compute_near_particle_kernel<<<grid, block>>>(
            bufs.d_state, bufs.d_accel_mag,
            bufs.d_near_particle, bufs.d_near_accel,
            larmor_radiation, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Leapfrog + damping (with optional Larmor modulation)
    phase_write_kernel<<<grid, block>>>(
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
        bufs.d_near_particle, bufs.d_near_accel,
        do_damping, selective_damping, larmor_radiation,
        damping_factor,
        do_langevin, langevin_gamma, langevin_T, bufs.d_langevin_noise,
        langevin_site_filter,
        L
    );
    CUDA_CHECK(cudaGetLastError());

    // Genesis (stochastic) — requires cuRAND pre-fill
    if (do_genesis) {
        // Random numbers are generated in gpu_engine.cu before calling this
        genesis_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_random,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction,
            0, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Evaporation
    evaporation_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_locked,
        bufs.d_spin, bufs.d_color, bufs.d_particle_id,
        bufs.d_ledger_reaction,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

// ---------- Fused Wave Update Launcher (single-substrate) ----------
// Replaces launch_phase_read + launch_phase_write for single-substrate path.
// Eliminates delta_j global memory round-trip.

void launch_wave_update(GpuBuffers& bufs, bool do_wave, bool do_coupling,
                        bool do_damping, bool selective_damping,
                        bool larmor_radiation, double damping_factor,
                        bool do_genesis, double dt,
                        bool do_langevin, double langevin_gamma, double langevin_T) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);

    // Compute near-particle mask (+ Larmor accel) if selective damping
    if (selective_damping) {
        compute_near_particle_kernel<<<grid, block>>>(
            bufs.d_state, bufs.d_accel_mag,
            bufs.d_near_particle, bufs.d_near_accel,
            larmor_radiation, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Fused Laplacian + coupling + leapfrog + damping (or Langevin OU)
    wave_update_kernel<<<grid, block>>>(
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_near_particle, bufs.d_near_accel,
        do_wave, do_coupling,
        do_damping, selective_damping, larmor_radiation,
        damping_factor,
        do_langevin, langevin_gamma, langevin_T, bufs.d_langevin_noise,
        L
    );
    CUDA_CHECK(cudaGetLastError());

    // Genesis (stochastic) — requires cuRAND pre-fill
    if (do_genesis) {
        genesis_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_random,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction,
            0, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Evaporation
    evaporation_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_locked,
        bufs.d_spin, bufs.d_color, bufs.d_particle_id,
        bufs.d_ledger_reaction,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

// ---------- Dual-Substrate Launchers ----------

void launch_phase_read_dual(const GpuBuffers& bufs, bool do_wave, bool do_coupling) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    phase_read_dual_kernel<<<grid, block>>>(
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_delta_j_L_x, bufs.d_delta_j_L_y, bufs.d_delta_j_L_z,
        bufs.d_delta_j_R_x, bufs.d_delta_j_R_y, bufs.d_delta_j_R_z,
        L, do_wave, do_coupling
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_phase_write_dual(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                              bool larmor_radiation, double damping_factor,
                              bool do_genesis, double dt) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    // Compute near-particle mask (+ Larmor accel) if selective damping
    if (selective_damping) {
        compute_near_particle_kernel<<<grid, block>>>(
            bufs.d_state, bufs.d_accel_mag,
            bufs.d_near_particle, bufs.d_near_accel,
            larmor_radiation, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Dual leapfrog + sync observable (with optional Larmor modulation)
    phase_write_dual_kernel<<<grid, block>>>(
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
        bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
        bufs.d_delta_j_L_x, bufs.d_delta_j_L_y, bufs.d_delta_j_L_z,
        bufs.d_delta_j_R_x, bufs.d_delta_j_R_y, bufs.d_delta_j_R_z,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_near_particle, bufs.d_near_accel,
        do_damping, selective_damping, larmor_radiation,
        damping_factor, L
    );
    CUDA_CHECK(cudaGetLastError());

    // Dual genesis (chirality-based)
    if (do_genesis) {
        genesis_dual_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
            bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_random,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Evaporation uses observable field (same as legacy)
    evaporation_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_locked,
        bufs.d_spin, bufs.d_color, bufs.d_particle_id,
        bufs.d_ledger_reaction, L
    );
    CUDA_CHECK(cudaGetLastError());
}

// ---------- Dual-substrate Gauss sync kernel ----------
// After Gauss projection modifies the observable (d_flux_x/y/z),
// propagate the correction back to L and R substrates equally:
//   delta = J_new - (J_L + J_R)
//   J_L += delta/2,  J_R += delta/2
// This preserves J_L + J_R = J and keeps chirality unchanged.

__global__ void gauss_sync_dual_kernel(
    double* fL_x, double* fL_y, double* fL_z,
    double* fR_x, double* fR_y, double* fR_z,
    const double* obs_x, const double* obs_y, const double* obs_z,
    int L)
{
    int bx = blockIdx.x * blockDim.x + threadIdx.x;
    int by = blockIdx.y * blockDim.y + threadIdx.y;
    int bz = blockIdx.z * blockDim.z + threadIdx.z;
    if (bx >= L || by >= L || bz >= L) return;
    int i = bx * L * L + by * L + bz;  // X-major (matches CPU)

    double dx = (obs_x[i] - (fL_x[i] + fR_x[i])) * 0.5;
    double dy = (obs_y[i] - (fL_y[i] + fR_y[i])) * 0.5;
    double dz = (obs_z[i] - (fL_z[i] + fR_z[i])) * 0.5;

    fL_x[i] += dx;  fL_y[i] += dy;  fL_z[i] += dz;
    fR_x[i] += dx;  fR_y[i] += dy;  fR_z[i] += dz;
}

void launch_gauss_sync_dual(GpuBuffers& bufs) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    gauss_sync_dual_kernel<<<grid, block>>>(
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

// ============================================================================
// WEAK TRANSMUTATION KERNEL [CLAUDE.md §6.5]
// ============================================================================
// When field stress |div(J)| + |curl(J)| + |grad(rho)| exceeds WEAK_THRESHOLD,
// manifested particles may flip polarity (+1 <-> -1).
//
// Dual-substrate threshold analysis:
//   In dual mode, div and curl use J_L only (parity violation), while ∇ρ uses
//   the observable J (scale-independent). The same K_GENESIS threshold applies
//   without halving because the L-substrate carries ~98% of the flux at positive
//   particle sites: (1+δ)/2 ≈ 0.978 where δ = DELTA_APPROX ≈ 0.957.
//   So div(J_L) ≈ 0.978 × div(J_obs) — the asymmetric splitting means the
//   dominant substrate is nearly identical to the observable.

__global__ void weak_transmutation_kernel(
    int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const double* __restrict__ random,
    bool dual_substrate,
    const double* __restrict__ fL_x,
    const double* __restrict__ fL_y,
    const double* __restrict__ fL_z,
    double* __restrict__ fL_x_mut,   // mutable L substrate (for swap)
    double* __restrict__ fL_y_mut,
    double* __restrict__ fL_z_mut,
    double* __restrict__ fR_x_mut,   // mutable R substrate (for swap)
    double* __restrict__ fR_y_mut,
    double* __restrict__ fR_z_mut,
    double* __restrict__ wvL_x_mut,
    double* __restrict__ wvL_y_mut,
    double* __restrict__ wvL_z_mut,
    double* __restrict__ wvR_x_mut,
    double* __restrict__ wvR_y_mut,
    double* __restrict__ wvR_z_mut,
    int* __restrict__ ledger_reaction,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] == 0) return;  // Only manifested particles transmute

    // Neighbor indices
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    // Select flux arrays: J_L in dual mode, J in single mode
    const double* fx = dual_substrate ? fL_x : flux_x;
    const double* fy = dual_substrate ? fL_y : flux_y;
    const double* fz = dual_substrate ? fL_z : flux_z;

    // Divergence: div(J) = dJx/dx + dJy/dy + dJz/dz
    double div_J = 0.5 * ((fx[xp] - fx[xm]) + (fy[yp] - fy[ym]) + (fz[zp] - fz[zm]));
    double div_mag = fabs(div_J);

    // Curl: (∇×J)_x = dJz/dy - dJy/dz, etc.
    double curl_x = 0.5 * ((fz[yp] - fz[ym]) - (fy[zp] - fy[zm]));
    double curl_y = 0.5 * ((fx[zp] - fx[zm]) - (fz[xp] - fz[xm]));
    double curl_z = 0.5 * ((fy[xp] - fy[xm]) - (fx[yp] - fx[ym]));
    double curl_mag = sqrt(curl_x*curl_x + curl_y*curl_y + curl_z*curl_z);

    // Gradient of density: ∇ρ where ρ = |J|
    auto density = [&](int j) -> double {
        return sqrt(flux_x[j]*flux_x[j] + flux_y[j]*flux_y[j] + flux_z[j]*flux_z[j]);
    };
    double gx = 0.5 * (density(xp) - density(xm));
    double gy = 0.5 * (density(yp) - density(ym));
    double gz = 0.5 * (density(zp) - density(zm));
    double grad_mag = sqrt(gx*gx + gy*gy + gz*gz);

    double stress = div_mag + curl_mag + grad_mag;

    constexpr double weak_threshold = K_GENESIS;  // = 3 * K_B
    if (stress <= weak_threshold) return;

    // Probabilistic flip: p = 1 - exp(-(stress - threshold) / K_B)
    double p = 1.0 - exp(-(stress - weak_threshold) / K_B);
    if (random[i] >= p) return;

    // Flip polarity
    const int8_t old_state = state[i];
    const int8_t new_state = -old_state;
    state[i] = new_state;
    if (ledger_reaction) {
        atomicAdd(&ledger_reaction[i],
                  static_cast<int>(new_state) - static_cast<int>(old_state));
    }

    // In dual mode, swap L/R flux to match new chirality
    if (dual_substrate) {
        // Swap flux L <-> R
        double tmp;
        tmp = fL_x_mut[i]; fL_x_mut[i] = fR_x_mut[i]; fR_x_mut[i] = tmp;
        tmp = fL_y_mut[i]; fL_y_mut[i] = fR_y_mut[i]; fR_y_mut[i] = tmp;
        tmp = fL_z_mut[i]; fL_z_mut[i] = fR_z_mut[i]; fR_z_mut[i] = tmp;
        // Swap wave_vel L <-> R
        tmp = wvL_x_mut[i]; wvL_x_mut[i] = wvR_x_mut[i]; wvR_x_mut[i] = tmp;
        tmp = wvL_y_mut[i]; wvL_y_mut[i] = wvR_y_mut[i]; wvR_y_mut[i] = tmp;
        tmp = wvL_z_mut[i]; wvL_z_mut[i] = wvR_z_mut[i]; wvR_z_mut[i] = tmp;
    }
}

// ============================================================================
// PAIR PRODUCTION KERNEL [CLAUDE.md §4.1, §12.1]
// ============================================================================
// Enhanced genesis: when flux > 2×K_GENESIS at a void site, produce correlated
// +1/-1 pair at adjacent sites. Uses atomicCAS_byte to claim two sites atomically.

// Byte-level atomicCAS (duplicate from kernels_forces.cu — not shared across TUs)
__device__ __forceinline__
int8_t atomicCAS_byte_stencil(int8_t* addr, int8_t compare, int8_t val) {
    unsigned int* word_addr = reinterpret_cast<unsigned int*>(
        reinterpret_cast<size_t>(addr) & ~3ULL);
    unsigned int byte_offset = (reinterpret_cast<size_t>(addr) & 3) * 8;
    unsigned int byte_mask = 0xFFu << byte_offset;

    unsigned int old_word = *word_addr;
    unsigned int assumed;
    do {
        assumed = old_word;
        unsigned int old_byte = (assumed >> byte_offset) & 0xFF;
        if (old_byte != static_cast<unsigned char>(compare))
            return static_cast<int8_t>(old_byte);
        unsigned int new_word = (assumed & ~byte_mask)
                              | (static_cast<unsigned int>(static_cast<unsigned char>(val)) << byte_offset);
        old_word = atomicCAS(word_addr, assumed, new_word);
    } while (old_word != assumed);
    return compare;
}

__global__ void pair_production_kernel(
    int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const double* __restrict__ random,
    int32_t* __restrict__ pair_id,
    int* __restrict__ ledger_reaction,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] != 0) return;  // Only void sites can produce pairs

    // Check flux magnitude
    double rho = sqrt(flux_x[i]*flux_x[i] + flux_y[i]*flux_y[i] + flux_z[i]*flux_z[i]);
    constexpr double PAIR_THRESHOLD = 2.0 * K_GENESIS;
    if (rho < PAIR_THRESHOLD) return;

    // Probabilistic: p = 1 - exp(-(rho - threshold) / K_B)
    double p = 1.0 - exp(-(rho - PAIR_THRESHOLD) / K_B);
    if (random[i] >= p) return;

    // Find best adjacent void site for partner
    // Check 6 face neighbors
    int nbrs[6] = {
        idx3d(x+1,y,z,L), idx3d(x-1,y,z,L),
        idx3d(x,y+1,z,L), idx3d(x,y-1,z,L),
        idx3d(x,y,z+1,L), idx3d(x,y,z-1,L)
    };

    // Pick neighbor with highest flux (most energetic)
    int best_j = -1;
    double best_rho = -1.0;
    for (int n = 0; n < 6; ++n) {
        int j = nbrs[n];
        if (state[j] != 0) continue;  // Must be void
        double rj = sqrt(flux_x[j]*flux_x[j] + flux_y[j]*flux_y[j] + flux_z[j]*flux_z[j]);
        if (rj > best_rho) {
            best_rho = rj;
            best_j = j;
        }
    }
    if (best_j < 0) return;  // No adjacent void site

    // Atomically claim both sites: first site → +1, second → -1
    int8_t old_i = atomicCAS_byte_stencil(&state[i], 0, 1);
    if (old_i != 0) return;  // Someone else claimed it

    int8_t old_j = atomicCAS_byte_stencil(&state[best_j], 0, -1);
    if (old_j != 0) {
        // Rollback: release first site
        state[i] = 0;
        return;
    }

    // Both claimed — assign matching pair_id (use lattice index as unique ID)
    pair_id[i] = i;
    pair_id[best_j] = i;
    if (ledger_reaction) {
        atomicAdd(&ledger_reaction[i], 1);
        atomicAdd(&ledger_reaction[best_j], -1);
    }
}

void launch_pair_production(GpuBuffers& bufs) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    pair_production_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_random,
        bufs.d_pair_id,
        bufs.d_ledger_reaction,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    weak_transmutation_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_random,
        dual_substrate,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
        bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
        bufs.d_ledger_reaction,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_strong_field_stencil(GpuBuffers& bufs, double damp) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    strong_field_stencil_kernel<<<grid, block>>>(
        bufs.d_flux_strong_x, bufs.d_flux_strong_y, bufs.d_flux_strong_z,
        bufs.d_wave_vel_strong_x, bufs.d_wave_vel_strong_y, bufs.d_wave_vel_strong_z,
        bufs.d_state, bufs.d_color,
        damp, L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_weak_field_stencil(GpuBuffers& bufs, double damp) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    weak_field_stencil_kernel<<<grid, block>>>(
        bufs.d_flux_weak_x, bufs.d_flux_weak_y, bufs.d_flux_weak_z,
        bufs.d_wave_vel_weak_x, bufs.d_wave_vel_weak_y, bufs.d_wave_vel_weak_z,
        bufs.d_state, bufs.d_flavor,
        damp, L
    );
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
