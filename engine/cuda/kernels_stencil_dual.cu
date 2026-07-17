/**
 * @file kernels_stencil_dual.cu
 * @brief Dual-substrate Phase Read / Phase Write kernels (FTD tick cycle).
 *
 * Phase 5 split (2026-04-27): extracted verbatim from kernels_stencil.cu.
 * Contains the dual-substrate path:
 *   - phase_read_dual_kernel      (independent Laplacians on L/R + 50/50 coupling)
 *   - strong_field_stencil_kernel (stella octangula, 8 vertex neighbors)
 *   - weak_field_stencil_kernel   (cuboctahedron, 12 edge neighbors)
 *   - phase_write_dual_kernel     (independent leapfrog + observable sync)
 *   - genesis_dual_kernel         (chirality-based polarity assignment)
 *   - gauss_sync_dual_kernel      (post-Gauss-projection L/R sync)
 * plus their host-side launchers.
 *
 * Helper functions effective_damping / scale_field_pair / wrap / idx3d live in
 * kernels_stencil_common.cuh so single + dual paths share one source of truth.
 *
 * The dual launcher reuses evaporation_kernel from kernels_stencil_single.cu;
 * declared at the top of this TU.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/voxel_rng.h"   // BH-F5/F8/F9 (2026-05-05): shared SplitMix64 RNG
#include "kernels_stencil_common.cuh"   // wrap, idx3d, effective_damping, scale_field_pair
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "cuda_error.cuh"  // CUDA_CHECK (revision C1 consolidation)

namespace ftd {
namespace gpu {
namespace kernels {

// Forward declarations: these kernels are defined in kernels_stencil_single.cu
// and reused by dual-substrate launchers below. Both are in the same
// ftd::gpu::kernels namespace so symbol resolution at device-link time
// is direct (CUDA_SEPARABLE_COMPILATION ON in CMakeLists.txt).
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
    int L,
    unsigned long long rng_seed, int tick
);

__global__ void compute_near_particle_kernel(
    const int8_t* __restrict__ state,
    const double* __restrict__ accel_mag,
    uint8_t* __restrict__ near_particle,
    double* __restrict__ near_accel,
    bool do_larmor,
    int L
);

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
    int L, bool do_wave, bool do_coupling,
    // FTD-0271/0281 de Broglie clock (GPU port, 2026-06-20). Mirrors the CPU
    // dual branch in engine/src/render_bridge_phases/phase_read.cpp:133-140.
    bool do_db_clock, bool do_db_clock_coulomb, double omega0,
    const double* __restrict__ phi_coulomb
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

        constexpr double WF = LAPLACIAN_FACE_WEIGHT, WE = LAPLACIAN_EDGE_WEIGHT;
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

    // FTD-0271/0281: de Broglie clock — Klein-Gordon −ω_eff²·J on each substrate.
    // Bit-for-bit mirror of the CPU dual branch (phase_read.cpp:133-140); both
    // toggles default OFF ⇒ dead branch ⇒ byte-identical to the pre-port kernel.
    if (do_db_clock_coulomb) {
        const double omega0_sq = omega0 * omega0;
        const double omega_eff_sq = omega0_sq - 2.0 * omega0 * phi_coulomb[i];
        dLx -= fL_x[i] * omega_eff_sq;  dLy -= fL_y[i] * omega_eff_sq;  dLz -= fL_z[i] * omega_eff_sq;
        dRx -= fR_x[i] * omega_eff_sq;  dRy -= fR_y[i] * omega_eff_sq;  dRz -= fR_z[i] * omega_eff_sq;
    } else if (do_db_clock && state[i] != 0) {
        const double omega0_sq = omega0 * omega0;
        dLx -= fL_x[i] * omega0_sq;  dLy -= fL_y[i] * omega0_sq;  dLz -= fL_z[i] * omega0_sq;
        dRx -= fR_x[i] * omega0_sq;  dRy -= fR_y[i] * omega0_sq;  dRz -= fR_z[i] * omega0_sq;
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
        // Single source of truth: include/ftd/constants_shared.h
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
        // Single source of truth: include/ftd/constants_shared.h
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
    double dt,
    bool symplectic_leapfrog,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // Independent leapfrog on L
    if (symplectic_leapfrog) {
        wvL_x[i] += djL_x[i] * dt;  wvL_y[i] += djL_y[i] * dt;  wvL_z[i] += djL_z[i] * dt;
        fL_x[i] += wvL_x[i] * dt;   fL_y[i] += wvL_y[i] * dt;   fL_z[i] += wvL_z[i] * dt;

        // Independent leapfrog on R
        wvR_x[i] += djR_x[i] * dt;  wvR_y[i] += djR_y[i] * dt;  wvR_z[i] += djR_z[i] * dt;
        fR_x[i] += wvR_x[i] * dt;   fR_y[i] += wvR_y[i] * dt;   fR_z[i] += wvR_z[i] * dt;
    } else {
        wvL_x[i] += djL_x[i];  wvL_y[i] += djL_y[i];  wvL_z[i] += djL_z[i];
        fL_x[i] += wvL_x[i];   fL_y[i] += wvL_y[i];   fL_z[i] += wvL_z[i];

        // Independent leapfrog on R
        wvR_x[i] += djR_x[i];  wvR_y[i] += djR_y[i];  wvR_z[i] += djR_z[i];
        fR_x[i] += wvR_x[i];   fR_y[i] += wvR_y[i];   fR_z[i] += wvR_z[i];
    }

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
    int8_t* __restrict__ spin, int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int* __restrict__ ledger_reaction,
    int L,
    // BH-F5/F8/F9 (2026-05-05): SplitMix64 RNG via shared voxel_rng.h.
    unsigned long long rng_seed,
    int                tick
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

    constexpr double k_genesis = K_GENESIS;   // = N_c·K_MANIFEST (kinetics trigger)
    if (density <= k_genesis) return;

    // Exponential CDF genesis probability (matches CPU, dual-substrate)
    double z_gen = density - k_genesis;
    double p = 1.0 - exp(-z_gen / K_MANIFEST);
    // BH-F5 (2026-05-05): SplitMix64 stream replaces curand. Bit-exact CPU↔GPU.
    double r = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::GenesisManifest));
    if (r >= p) return;

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
    } else {
        // BH-F8 (2026-05-05): zero-curl spin fallback. Matches CPU
        // phase_write.cpp:104-106 + the single-substrate genesis_kernel
        // post-fix.
        double rs = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::GenesisSpin));
        spin[i] = (rs < 0.5) ? 1 : -1;
    }

    // Color from dominant observable axis
    double afx = fabs(fx), afy = fabs(fy), afz = fabs(fz);
    if (afx >= afy && afx >= afz) color[i] = 1;
    else if (afy >= afz) color[i] = 2;
    else color[i] = 3;

    particle_id[i] = i;
}

// ---------- Dual-Substrate Launchers ----------

void launch_phase_read_dual(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                            bool do_db_clock, bool do_db_clock_coulomb, double omega0) {
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
        L, do_wave, do_coupling,
        do_db_clock, do_db_clock_coulomb, omega0, bufs.d_phi_coulomb
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_phase_write_dual(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                              bool larmor_radiation, double damping_factor,
                              bool do_genesis, bool do_evaporation, double dt, bool symplectic_leapfrog,
                              unsigned long long rng_seed, int tick) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    // Compute near-particle mask (+ Larmor accel) if selective damping.
    // compute_near_particle_kernel lives in kernels_stencil_single.cu;
    // forward-declared at the top of this TU.
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
        damping_factor,
        dt, symplectic_leapfrog,
        L
    );
    CUDA_CHECK(cudaGetLastError());

    // Dual genesis (chirality-based) — BH-F5/F8/F9 (2026-05-05): SplitMix64
    // stream replaces cuRAND pre-fill.
    if (do_genesis) {
        genesis_dual_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
            bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction, L,
            rng_seed, tick
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Evaporation — gate on (do_genesis || do_evaporation) for parity with
    // single-substrate launch_phase_write and CPU phase_write.cpp:291. Pre-fix
    // this ran unconditionally on the dual path (the F6 single-substrate fix
    // was not propagated here); fixed 2026-05-05 alongside the toggles.evaporation
    // flag introduction. Evaporation uses observable field (same as legacy).
    // Defined in kernels_stencil_single.cu; forward-declared at the top of this
    // TU. Stochastic since the BH-F5 completion (2026-07-16): rng_seed/tick
    // feed the shared SplitMix64 Evaporation draw (CPU evaporation is shared
    // single+dual, so one kernel serves both paths here too).
    if (do_genesis || do_evaporation) {
        evaporation_kernel<<<grid, block>>>(
            bufs.d_state,
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            bufs.d_locked,
            bufs.d_spin, bufs.d_color, bufs.d_particle_id,
            bufs.d_ledger_reaction, L,
            rng_seed, tick
        );
        CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently
    }
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
