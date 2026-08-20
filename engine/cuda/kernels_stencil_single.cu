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
 *   - evaporation_kernel            (stochastic Boltzmann de-manifestation)
 * plus their host-side launchers (launch_phase_read, launch_phase_write,
 * launch_wave_update).
 *
 * Helper functions effective_damping / scale_field_pair / wrap / idx3d live in
 * kernels_stencil_common.cuh so single + dual paths share one source of truth.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/proper_time_rate.h"  // proper-time hazard (2026-07-19): shared dτ/dt
#include "ftd/sublattice.h"   // N_FACE, N_EDGE, N_CORNER, W_SC_FACE, W_FCC_EDGE, W_BCC_CORNER
#include "ftd/voxel_rng.h"    // BH-F5/F8/F9 (2026-05-05): shared SplitMix64 RNG
#include "kernels_stencil_common.cuh"   // wrap, idx3d, effective_damping, scale_field_pair
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>   // fprintf — Linux/clang stricter than MSVC
#include <cstdlib>  // exit

#include "cuda_error.cuh"  // CUDA_CHECK (revision C1 consolidation)

namespace ftd {
namespace gpu {
namespace kernels {

// Defined in kernels_aux.cu.  The lifecycle launcher snapshots the post-write
// field, stably compacts accepted genesis/original evaporation candidates, and
// commits them in canonical X-major order before assigning surviving IDs by
// stable rank.  This preserves the CPU's live-neighbour evaporation semantics
// without an O(N) serial device scan.
void launch_canonical_lifecycle(
    GpuBuffers& bufs, bool dual_substrate,
    bool do_genesis, bool do_evaporation,
    double kinetic_drain, double genesis_threshold, double manifest_scale,
    unsigned long long rng_seed);

// ---------- Phase Read Kernel ----------
// Computes delta_j = C_WAVE^2 * Laplacian(flux) - G_C * gradient(state)
//                   + G_C * curl(state * velocity)
// (electric coupling sign per lagrangian.h Term 2, amended 2026-07-18)

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
    uint8_t bcc_stencil_mode,   // Cluster A FTD-0093: 0=FULL, 1=SC, 2=FCC, 3=BCC
    // FTD-0271/0281 de Broglie clock (GPU port, 2026-06-20). Mirrors the CPU
    // branch in engine/src/render_bridge_phases/phase_read.cpp:193-200.
    bool do_db_clock,
    bool do_db_clock_coulomb,
    double omega0,
    const double* __restrict__ phi_coulomb,   // pre-solved Coulomb potential (db_clock_coulomb)
    bool period2_floquet,
    bool bcc_time_floquet,
    const int* __restrict__ tick_ptr
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

        const int tick = *tick_ptr;
        const double cw2 = wave_kick_cw2(tick, period2_floquet, bcc_time_floquet);
        dx += cw2 * lap_x;
        dy += cw2 * lap_y;
        dz += cw2 * lap_z;
    }

    if (do_coupling) {
        // Gradient of state: -g_c * ∇(s) — electric sign per lagrangian.h
        // Term 2 amendment 2026-07-18 (mirror of phase_read.cpp; the drive
        // points OUTWARD at a +1 charge, cooperating with the Gauss target).
        int xp = idx3d(x+1, y, z, L);
        int xm = idx3d(x-1, y, z, L);
        int yp = idx3d(x, y+1, z, L);
        int ym = idx3d(x, y-1, z, L);
        int zp = idx3d(x, y, z+1, L);
        int zm = idx3d(x, y, z-1, L);

        double gs_x = 0.5 * (static_cast<double>(state[xp]) - static_cast<double>(state[xm]));
        double gs_y = 0.5 * (static_cast<double>(state[yp]) - static_cast<double>(state[ym]));
        double gs_z = 0.5 * (static_cast<double>(state[zp]) - static_cast<double>(state[zm]));

        dx -= G_C * gs_x;
        dy -= G_C * gs_y;
        dz -= G_C * gs_z;

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

    // FTD-0271/0281: de Broglie internal clock — Klein-Gordon rest-mass term
    // −ω_eff²·J. Bit-for-bit mirror of the CPU branch (phase_read.cpp:193-200):
    //   db_clock_coulomb ⇒ all-site ω_eff² = ω₀² − 2·ω₀·phi_C  (V = −phi_C)
    //   de_broglie_clock alone ⇒ ω_eff² = ω₀² at manifested (state≠0) voxels.
    // Both toggles default OFF, so this is a dead branch with the spectroscopy
    // toggles off ⇒ the GPU phase_read is byte-identical to its pre-port form.
    if (do_db_clock_coulomb) {
        const double omega0_sq = omega0 * omega0;
        const double omega_eff_sq = omega0_sq - 2.0 * omega0 * phi_coulomb[i];
        dx -= flux_x[i] * omega_eff_sq;
        dy -= flux_y[i] * omega_eff_sq;
        dz -= flux_z[i] * omega_eff_sq;
    } else if (do_db_clock && state[i] != 0) {
        const double omega0_sq = omega0 * omega0;
        dx -= flux_x[i] * omega0_sq;
        dy -= flux_y[i] * omega0_sq;
        dz -= flux_z[i] * omega0_sq;
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
    bool verlet_wave,
    // Langevin thermostat (FTD-0051).
    bool do_langevin,
    double langevin_gamma,
    double langevin_T,
    uint8_t langevin_site_filter,    // Cluster A FTD-0093: 0=SC, 1=BCC, 2=FCC, 3=ALL
    int L,
    // BH-F9 (2026-05-05): Langevin noise via shared SplitMix64+Box-Muller.
    // Replaces the d_langevin_noise buffer pre-filled by curandGenerateNormalDouble.
    unsigned long long rng_seed,
    const int* __restrict__ tick_ptr
) {
    const int tick = *tick_ptr;
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // Leapfrog: default unit-step, symplectic (full dt), or Verlet KDK part 1.
    if (verlet_wave) {
        const double half_dt = 0.5 * dt;
        wv_x[i] += djx[i] * half_dt;
        wv_y[i] += djy[i] * half_dt;
        wv_z[i] += djz[i] * half_dt;

        flux_x[i] += wv_x[i] * dt;
        flux_y[i] += wv_y[i] * dt;
        flux_z[i] += wv_z[i] * dt;
    } else if (symplectic_leapfrog) {
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
        // FDT-consistent discrete OU: Var_stationary = sigma^2/(1-(1-gamma)^2) = T
        // exactly (was sqrt(2*gamma*T), the Euler-Maruyama form biased hot to
        // T/(1-gamma/2)). Mirrors the CPU fix in phase_write.cpp:237.
        const double sigma = sqrt(langevin_gamma * (2.0 - langevin_gamma) * langevin_T);
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
    int32_t* __restrict__ pair_id,
    int* __restrict__ ledger_reaction,
    int L,
    double kinetic_drain,  // FTD-0276: runtime drain fraction (default 0.5)
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

    constexpr double k_genesis = K_GENESIS;   // = N_c·K_MANIFEST (kinetics trigger)
    if (density <= k_genesis) return;

    // Exponential CDF genesis probability (matches CPU)
    // p = 1 - exp(-(density - K_GENESIS) / K_MANIFEST)
    double z_gen = density - k_genesis;
    double p = 1.0 - exp(-z_gen / K_MANIFEST);
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
    // genesis event. Drain factor kinetic_drain (default 0.5, runtime toggle
    // since FTD-0276) on wave_vel; flux scaled by max(0, 1 - K_GENESIS/|J|) to
    // reduce density at the new particle's site to the K_GENESIS threshold.
    wave_vel_x[i] *= (1.0 - kinetic_drain);
    wave_vel_y[i] *= (1.0 - kinetic_drain);
    wave_vel_z[i] *= (1.0 - kinetic_drain);
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

    // Match the CPU ARCH-7 contract: parallel genesis marks a pending identity;
    // a post-evaporation device scan resolves surviving sentinels in ascending
    // X-major voxel order.  This is deterministic across CUDA schedules.
    particle_id[i] = -2;
    pair_id[i] = -1;
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
    const double* __restrict__ velocity_x,   // proper-time hazard (2026-07-19)
    const double* __restrict__ velocity_y,
    const double* __restrict__ velocity_z,
    const double* __restrict__ latency,
    const uint8_t* __restrict__ locked,
    int8_t* __restrict__ spin,
    int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int32_t* __restrict__ pair_id,
    int* __restrict__ ledger_reaction,
    int L,
    unsigned long long rng_seed, int tick
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

    // Stochastic Boltzmann evaporation — BH-F5 completion (2026-07-16).
    // Mirrors the canonical CPU rule (phase_write.cpp Loop 2, stochastic since
    // 15882e98 2026-04-23): evap_prob = exp(-E_local/K_MANIFEST²)·K_EVAP_RATE,
    // gated by the shared SplitMix64 stream (voxel_rng.h, salt Evaporation) so
    // the draw is bit-exact with the CPU's at identical (seed, voxel, tick).
    // Pre-fix this kernel kept the pre-2026-04-23 deterministic threshold
    // (E_local < K_MANIFEST²·1e-6), under which a settled particle
    // (E_local ~ 0.03 ≫ 2.6e-7) never evaporated — isolated-particle lifetime
    // was ~8 ticks on CPU vs infinite on GPU. CUDA exp() vs std::exp can
    // differ sub-ULP; a CPU↔GPU decision flip needs |u − p·K_EVAP_RATE|
    // ≲ 1e-16 (same accepted caveat as voxel_normal's transcendentals).
    double evap_prob = exp(-local_energy / (K_MANIFEST * K_MANIFEST));
    // Proper-time hazard (2026-07-19 amendment; mirrors CPU phase_write.cpp):
    // the decay clock integrates the SAME dτ as the proper-time accumulator
    // (ftd/proper_time_rate.h, shared __host__ __device__ definition). At
    // L=0, v=0 the factor is exactly 1 — bit-identical to the pre-amendment
    // rule. The RNG draw and stream are unchanged; only the threshold scales.
    const double speed2 = velocity_x[i]*velocity_x[i]
                        + velocity_y[i]*velocity_y[i]
                        + velocity_z[i]*velocity_z[i];
    const double dtau = ::ftd::proper_time_rate(latency[i], speed2);
    double u = ::ftd::voxel_uniform(rng_seed, i, tick,
            static_cast<unsigned long long>(::ftd::VoxelRng::Evaporation));
    if (u < evap_prob * K_EVAP_RATE * dtau) {
        const int8_t old_state = state[i];
        state[i] = 0;
        if (ledger_reaction) {
            atomicAdd(&ledger_reaction[i], -static_cast<int>(old_state));
        }
        spin[i] = 0;
        color[i] = 0;
        particle_id[i] = -1;
        pair_id[i] = -1;
    }
}

// ---------- Launcher Functions ----------

void launch_phase_read(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                        uint8_t bcc_stencil_mode,
                        bool do_db_clock, bool do_db_clock_coulomb, double omega0,
                        bool period2_floquet, bool bcc_time_floquet) {
    const cudaStream_t stream = bufs.stream;
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);

    phase_read_kernel<<<grid, block, 0, stream>>>(
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
        L, do_wave, do_coupling, bcc_stencil_mode,
        do_db_clock, do_db_clock_coulomb, omega0, bufs.d_phi_coulomb,
        period2_floquet, bcc_time_floquet, bufs.d_tick
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_phase_write(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                        bool larmor_radiation, double damping_factor,
                        bool do_genesis, bool do_evaporation, double dt,
                        bool symplectic_leapfrog, bool verlet_wave_integrator,
                        bool do_langevin, double langevin_gamma, double langevin_T,
                        uint8_t langevin_site_filter,
                        double kinetic_drain,
                        double genesis_threshold,
                        double manifest_scale,
                        unsigned long long rng_seed) {
    const cudaStream_t stream = bufs.stream;
    const int* const tick = bufs.d_tick;
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);

    // Compute near-particle mask (+ Larmor accel) if selective damping
    if (selective_damping) {
        compute_near_particle_kernel<<<grid, block, 0, stream>>>(
            bufs.d_state, bufs.d_accel_mag,
            bufs.d_near_particle, bufs.d_near_accel,
            larmor_radiation, L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Leapfrog + damping (with optional Larmor modulation)
    phase_write_kernel<<<grid, block, 0, stream>>>(
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
        bufs.d_near_particle, bufs.d_near_accel,
        do_damping, selective_damping, larmor_radiation,
        damping_factor,
        dt, symplectic_leapfrog, verlet_wave_integrator,
        do_langevin, langevin_gamma, langevin_T,
        langevin_site_filter,
        L,
        rng_seed, tick
    );  // `tick` is now bufs.d_tick (const int*)
    CUDA_CHECK(cudaGetLastError());

    launch_canonical_lifecycle(
        bufs, /*dual_substrate=*/false,
        do_genesis, do_evaporation,
        kinetic_drain, genesis_threshold, manifest_scale,
        rng_seed);
    CUDA_CHECK(cudaGetLastError());
}

__global__ void verlet_second_half_kick_kernel(
    double* __restrict__ wv_x,
    double* __restrict__ wv_y,
    double* __restrict__ wv_z,
    const double* __restrict__ djx,
    const double* __restrict__ djy,
    const double* __restrict__ djz,
    double half_dt,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;
    wv_x[i] += djx[i] * half_dt;
    wv_y[i] += djy[i] * half_dt;
    wv_z[i] += djz[i] * half_dt;
}

__global__ void verlet_second_half_kick_dual_kernel(
    double* __restrict__ wvL_x, double* __restrict__ wvL_y, double* __restrict__ wvL_z,
    double* __restrict__ wvR_x, double* __restrict__ wvR_y, double* __restrict__ wvR_z,
    const double* __restrict__ djL_x, const double* __restrict__ djL_y, const double* __restrict__ djL_z,
    const double* __restrict__ djR_x, const double* __restrict__ djR_y, const double* __restrict__ djR_z,
    double* __restrict__ wv_x, double* __restrict__ wv_y, double* __restrict__ wv_z,
    double half_dt,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;
    wvL_x[i] += djL_x[i] * half_dt;
    wvL_y[i] += djL_y[i] * half_dt;
    wvL_z[i] += djL_z[i] * half_dt;
    wvR_x[i] += djR_x[i] * half_dt;
    wvR_y[i] += djR_y[i] * half_dt;
    wvR_z[i] += djR_z[i] * half_dt;
    wv_x[i] = wvL_x[i] + wvR_x[i];
    wv_y[i] = wvL_y[i] + wvR_y[i];
    wv_z[i] = wvL_z[i] + wvR_z[i];
}

void launch_verlet_second_half_kick(GpuBuffers& bufs, double dt, bool dual) {
    const cudaStream_t stream = bufs.stream;
    const int L = bufs.L;
    const double half_dt = 0.5 * dt;
    dim3 block(4, 8, 8);
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);
    if (dual) {
        verlet_second_half_kick_dual_kernel<<<grid, block, 0, stream>>>(
            bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
            bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
            bufs.d_delta_j_L_x, bufs.d_delta_j_L_y, bufs.d_delta_j_L_z,
            bufs.d_delta_j_R_x, bufs.d_delta_j_R_y, bufs.d_delta_j_R_z,
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            half_dt, L);
    } else {
        verlet_second_half_kick_kernel<<<grid, block, 0, stream>>>(
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
            half_dt, L);
    }
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
