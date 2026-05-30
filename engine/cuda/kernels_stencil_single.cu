/**
 * @file kernels_stencil_single.cu
 * @brief Single-substrate Phase Read / Phase Write kernels (FTD tick cycle).
 *
 * Phase 5 split (2026-04-27): extracted verbatim from kernels_stencil.cu.
 * Contains the single-substrate path:
 *   - phase_read_kernel             (Laplacian + state-flux coupling)
 *   - compute_near_particle_kernel  (selective damping mask + Larmor accel)
 *   - phase_write_kernel            (leapfrog + damping + Langevin OU)
 *   - wave_update_kernel            (fused phase_read + phase_write)
 *   - genesis_kernel                (stochastic manifestation)
 *   - evaporation_kernel            (energy-threshold de-manifestation)
 * plus their host-side launchers (launch_phase_read, launch_phase_write,
 * launch_wave_update).
 *
 * Helper functions effective_damping / scale_field_pair / wrap / idx3d live in
 * kernels_stencil_common.cuh so single + dual paths share one source of truth.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/sublattice.h"   // N_FACE, N_EDGE, N_CORNER, W_SC_FACE, W_FCC_EDGE, W_BCC_CORNER
#include "ftd/voxel_rng.h"    // BH-F5/F8/F9 (2026-05-05): shared SplitMix64 RNG
#include "kernels_stencil_common.cuh"   // wrap, idx3d, effective_damping, scale_field_pair
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
            constexpr double WF = LAPLACIAN_FACE_WEIGHT;
            constexpr double WE = LAPLACIAN_EDGE_WEIGHT;
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
    double dt,
    bool symplectic_leapfrog,
    // Langevin thermostat (FTD-0051).
    bool do_langevin,
    double langevin_gamma,
    double langevin_T,
    uint8_t langevin_site_filter,    // Cluster A FTD-0093: 0=SC, 1=BCC, 2=FCC, 3=ALL
    int L,
    // BH-F9 (2026-05-05): Langevin noise via shared SplitMix64+Box-Muller.
    // Replaces the d_langevin_noise buffer pre-filled by curandGenerateNormalDouble.
    unsigned long long rng_seed,
    int                tick
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // Leapfrog: wave_vel += delta_j; flux += wave_vel
    if (symplectic_leapfrog) {
        wv_x[i] += djx[i] * dt;
        wv_y[i] += djy[i] * dt;
        wv_z[i] += djz[i] * dt;

        flux_x[i] += wv_x[i] * dt;
        flux_y[i] += wv_y[i] * dt;
        flux_z[i] += wv_z[i] * dt;
    } else {
        wv_x[i] += djx[i];
        wv_y[i] += djy[i];
        wv_z[i] += djz[i];

        flux_x[i] += wv_x[i];
        flux_y[i] += wv_y[i];
        flux_z[i] += wv_z[i];
    }

    // Cluster A: Langevin OU update gated on site-class filter.
    const bool langevin_active =
        do_langevin && langevin_site_match(x, y, z, langevin_site_filter);

    if (langevin_active) {
        // Ornstein-Uhlenbeck on wave_vel; replaces deterministic damping.
        const double one_minus_gamma = 1.0 - langevin_gamma;
        const double sigma = sqrt(2.0 * langevin_gamma * langevin_T);
        // BH-F9: Box-Muller-derived N(0,1) per axis via SplitMix64.
        const double nx = ::ftd::voxel_normal(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::LangevinNoiseX));
        const double ny = ::ftd::voxel_normal(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::LangevinNoiseY));
        const double nz = ::ftd::voxel_normal(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::LangevinNoiseZ));
        wv_x[i] = one_minus_gamma * wv_x[i] + sigma * nx;
        wv_y[i] = one_minus_gamma * wv_y[i] + sigma * ny;
        wv_z[i] = one_minus_gamma * wv_z[i] + sigma * nz;
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



// ---------- Genesis Kernel ----------
// Stochastic manifestation: void sites with density > K_GENESIS may manifest

__global__ void genesis_kernel(
    int8_t* __restrict__ state,
    double* __restrict__ flux_x,        // mutable: latent-heat drain (audit F4)
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    double* __restrict__ wave_vel_x,    // mutable: latent-heat drain (audit F4)
    double* __restrict__ wave_vel_y,
    double* __restrict__ wave_vel_z,
    int8_t* __restrict__ spin,
    int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int* __restrict__ ledger_reaction,
    int next_pid,  // starting particle ID for this batch
    int L,
    // BH-F5/F8/F9 (2026-05-05): per-voxel deterministic SplitMix64 RNG via
    // shared engine/include/ftd/voxel_rng.h. Replaces the d_random buffer
    // pre-filled by curandGenerateUniformDouble. Bit-exact CPU↔GPU.
    unsigned long long rng_seed,
    int                tick
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
    // BH-F5 (2026-05-05): SplitMix64 stream replaces curand. Bit-exact CPU↔GPU.
    double r = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::GenesisManifest));
    if (r >= p) return;

    // Determine polarity from divergence sign
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    double div = 0.5 * ((flux_x[xp] - flux_x[xm])
                       + (flux_y[yp] - flux_y[ym])
                       + (flux_z[zp] - flux_z[zm]));

    const int8_t new_state = (div > 0) ? 1 : -1;  // Matches CPU: strictly > 0 (not >=)

    // Latent Heat of Manifestation: drain wave + flux energy at the
    // manifesting site. Parity with CPU phase_write.cpp:269-273 (audit F4,
    // 2026-05-04). Pre-fix the GPU created particles "for free" — no
    // wave-energy or flux cost — breaking energy conservation at every
    // genesis event. Drain factor K_GENESIS_KINETIC_DRAIN (= 0.5) on
    // wave_vel; flux scaled by max(0, 1 - K_GENESIS/|J|) to reduce density
    // at the new particle's site to the K_GENESIS threshold.
    wave_vel_x[i] *= (1.0 - K_GENESIS_KINETIC_DRAIN);
    wave_vel_y[i] *= (1.0 - K_GENESIS_KINETIC_DRAIN);
    wave_vel_z[i] *= (1.0 - K_GENESIS_KINETIC_DRAIN);
    double jmag = sqrt(fx*fx + fy*fy + fz*fz);
    if (jmag > K_GENESIS_FLUX_EPSILON) {
        double drain_scale = fmax(0.0, 1.0 - k_genesis / jmag);
        flux_x[i] *= drain_scale;
        flux_y[i] *= drain_scale;
        flux_z[i] *= drain_scale;
    }

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
        // BH-F8 (2026-05-05): zero-curl spin fallback. CPU at
        // phase_write.cpp:104-106 uses voxel_uniform(...) < 0.5 ? +1 : -1
        // with VoxelRng::GenesisSpin = 2. Pre-fix the GPU assigned a
        // deterministic +1 — divergent with CPU. Now both backends share the
        // SplitMix64 stream and assign ±1 deterministically per (seed, voxel, tick).
        double rs = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::GenesisSpin));
        spin[i] = (rs < 0.5) ? 1 : -1;
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
                        bool do_genesis, bool do_evaporation, double dt, bool symplectic_leapfrog,
                        bool do_langevin, double langevin_gamma, double langevin_T,
                        uint8_t langevin_site_filter,
                        unsigned long long rng_seed, int tick) {
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
        dt, symplectic_leapfrog,
        do_langevin, langevin_gamma, langevin_T,
        langevin_site_filter,
        L,
        rng_seed, tick
    );
    CUDA_CHECK(cudaGetLastError());

    // Genesis (stochastic) — BH-F5/F8/F9 (2026-05-05): SplitMix64 stream
    // replaces the cuRAND pre-fill. Per-voxel deterministic via shared
    // engine/include/ftd/voxel_rng.h. Bit-exact CPU↔GPU.
    if (do_genesis) {
        genesis_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction,
            0, L,
            rng_seed, tick
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Evaporation — gate on (do_genesis || do_evaporation). The genesis path
    // implies evaporation by design (manifestation + evaporation are sister
    // operations on a single threshold, see CPU phase_write.cpp:291). The
    // do_evaporation flag lets tests exercise the evaporation path in
    // isolation without enabling genesis. Pre-F6 this ran every tick
    // regardless of toggle (audit F6, 2026-05-04).
    if (do_genesis || do_evaporation) {
        evaporation_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            bufs.d_locked,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction,
            L
        );
    }
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
