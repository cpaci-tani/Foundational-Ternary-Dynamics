/**
 * @file kernels_aux.cu
 * @brief Auxiliary physics kernels (weak transmutation, pair production).
 *
 * Phase 5 split (2026-04-27): extracted verbatim from kernels_stencil.cu.
 * Contains the auxiliary physics kernels that don't fit into the single- or
 * dual-substrate stencil paths:
 *   - weak_transmutation_kernel  (field-stress-driven polarity flip)
 *   - pair_production_kernel     (correlated +/- pair from high flux density)
 * plus their host-side launchers (launch_weak_transmutation,
 * launch_pair_production).
 *
 * The byte-level atomicCAS shim atomicCAS_byte_stencil is local to this TU;
 * the same pattern lives in kernels_forces.cu (atomicCAS_byte). Keeping each
 * copy local avoids cross-TU device-symbol resolution for an inlinable helper.
 *
 * Helper functions wrap / idx3d live in kernels_stencil_common.cuh so all
 * stencil-flavoured TUs share one X-major index source of truth.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/voxel_rng.h"
#include "kernels_stencil_common.cuh"   // wrap, idx3d (atomicCAS_byte_stencil is local)
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

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
    int L,
    unsigned long long rng_seed,
    int                tick
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

    constexpr double weak_threshold = K_GENESIS;  // = N_c·K_MANIFEST
    if (stress <= weak_threshold) return;

    // Probabilistic flip: p = 1 - exp(-(stress - threshold) / K_MANIFEST)
    double p = 1.0 - exp(-(stress - weak_threshold) / K_MANIFEST);
    double r = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::WeakTransmutation));
    if (r >= p) return;

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
    int32_t* __restrict__ pair_id,
    int* __restrict__ ledger_reaction,
    int L,
    unsigned long long rng_seed,
    int                tick
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

    // Probabilistic: p = 1 - exp(-(rho - threshold) / K_MANIFEST)
    double p = 1.0 - exp(-(rho - PAIR_THRESHOLD) / K_MANIFEST);
    double r = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::PairProduction));
    if (r >= p) return;

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

void launch_pair_production(GpuBuffers& bufs, unsigned long long rng_seed, int tick) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    pair_production_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_pair_id,
        bufs.d_ledger_reaction,
        L,
        rng_seed, tick
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate, unsigned long long rng_seed, int tick) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    weak_transmutation_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        dual_substrate,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
        bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
        bufs.d_ledger_reaction,
        L,
        rng_seed, tick
    );
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
