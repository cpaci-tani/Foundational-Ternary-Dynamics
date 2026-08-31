/**
 * @file kernels_aux.cu
 * @brief Auxiliary physics kernels (drives, boundaries, reactions).
 *
 * Phase 5 split (2026-04-27): extracted verbatim from kernels_stencil.cu.
 * Contains the auxiliary physics kernels that don't fit into the single- or
 * dual-substrate stencil paths:
 *   - ew_background_sweep_kernel   (pre-read uniform field drive)
 *   - absorbing_boundary_kernel    (post-movement quadratic sponge)
 *   - flux_boundary_kernel         (post-movement reflective/dispersal shell)
 *   - weak_transmutation_decide/apply_kernel (field-stress-driven polarity flip)
 *   - pair_production_kernel     (correlated +/- pair from high flux density)
 * plus their host-side launchers (launch_weak_transmutation,
 * launch_pair_production).
 *
 * The byte-level atomicCAS shim atomicCAS_byte is shared via cuda_index.cuh
 * (revision C3 — header-inlining needs no cross-TU device-symbol
 * resolution, so the old keep-local rationale no longer applies).
 *
 * Helper functions wrap / idx3d live in kernels_stencil_common.cuh so all
 * stencil-flavoured TUs share one X-major index source of truth.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/proper_time_rate.h"
#include "ftd/voxel_rng.h"
#include "kernels_stencil_common.cuh"   // wrap, idx3d (+ cuda_index.cuh: atomicCAS_byte)
#include <cuda_runtime.h>
#include <cub/device/device_select.cuh>
#include <thrust/iterator/counting_iterator.h>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "cuda_error.cuh"  // CUDA_CHECK (revision C1 consolidation)

namespace ftd {
namespace gpu {
namespace kernels {

namespace {

// Apply a scalar to every transported field register mutated by the CPU pass:
// observable, dual-substrate, strong-substrate, and weak-substrate J/W.
__device__ __forceinline__ void scale_boundary_fields(
    int i, double scale,
    double* flux_x, double* flux_y, double* flux_z,
    double* wave_x, double* wave_y, double* wave_z,
    double* flux_L_x, double* flux_L_y, double* flux_L_z,
    double* flux_R_x, double* flux_R_y, double* flux_R_z,
    double* wave_L_x, double* wave_L_y, double* wave_L_z,
    double* wave_R_x, double* wave_R_y, double* wave_R_z,
    double* strong_x, double* strong_y, double* strong_z,
    double* strong_wave_x, double* strong_wave_y, double* strong_wave_z,
    double* weak_x, double* weak_y, double* weak_z,
    double* weak_wave_x, double* weak_wave_y, double* weak_wave_z) {
    flux_x[i] *= scale; flux_y[i] *= scale; flux_z[i] *= scale;
    wave_x[i] *= scale; wave_y[i] *= scale; wave_z[i] *= scale;
    flux_L_x[i] *= scale; flux_L_y[i] *= scale; flux_L_z[i] *= scale;
    flux_R_x[i] *= scale; flux_R_y[i] *= scale; flux_R_z[i] *= scale;
    wave_L_x[i] *= scale; wave_L_y[i] *= scale; wave_L_z[i] *= scale;
    wave_R_x[i] *= scale; wave_R_y[i] *= scale; wave_R_z[i] *= scale;
    strong_x[i] *= scale; strong_y[i] *= scale; strong_z[i] *= scale;
    strong_wave_x[i] *= scale; strong_wave_y[i] *= scale; strong_wave_z[i] *= scale;
    weak_x[i] *= scale; weak_y[i] *= scale; weak_z[i] *= scale;
    weak_wave_x[i] *= scale; weak_wave_y[i] *= scale; weak_wave_z[i] *= scale;
}

__device__ __forceinline__ void copy_boundary_fields(
    int dst, int src,
    double* flux_x, double* flux_y, double* flux_z,
    double* wave_x, double* wave_y, double* wave_z,
    double* flux_L_x, double* flux_L_y, double* flux_L_z,
    double* flux_R_x, double* flux_R_y, double* flux_R_z,
    double* wave_L_x, double* wave_L_y, double* wave_L_z,
    double* wave_R_x, double* wave_R_y, double* wave_R_z,
    double* strong_x, double* strong_y, double* strong_z,
    double* strong_wave_x, double* strong_wave_y, double* strong_wave_z,
    double* weak_x, double* weak_y, double* weak_z,
    double* weak_wave_x, double* weak_wave_y, double* weak_wave_z) {
    flux_x[dst] = flux_x[src]; flux_y[dst] = flux_y[src]; flux_z[dst] = flux_z[src];
    wave_x[dst] = wave_x[src]; wave_y[dst] = wave_y[src]; wave_z[dst] = wave_z[src];
    flux_L_x[dst] = flux_L_x[src]; flux_L_y[dst] = flux_L_y[src]; flux_L_z[dst] = flux_L_z[src];
    flux_R_x[dst] = flux_R_x[src]; flux_R_y[dst] = flux_R_y[src]; flux_R_z[dst] = flux_R_z[src];
    wave_L_x[dst] = wave_L_x[src]; wave_L_y[dst] = wave_L_y[src]; wave_L_z[dst] = wave_L_z[src];
    wave_R_x[dst] = wave_R_x[src]; wave_R_y[dst] = wave_R_y[src]; wave_R_z[dst] = wave_R_z[src];
    strong_x[dst] = strong_x[src]; strong_y[dst] = strong_y[src]; strong_z[dst] = strong_z[src];
    strong_wave_x[dst] = strong_wave_x[src]; strong_wave_y[dst] = strong_wave_y[src]; strong_wave_z[dst] = strong_wave_z[src];
    weak_x[dst] = weak_x[src]; weak_y[dst] = weak_y[src]; weak_z[dst] = weak_z[src];
    weak_wave_x[dst] = weak_wave_x[src]; weak_wave_y[dst] = weak_wave_y[src]; weak_wave_z[dst] = weak_wave_z[src];
}

__device__ __forceinline__ void copy_outflow_boundary_pair(
    int dst, int src, double normal_step,
    double* field_x, double* field_y, double* field_z,
    double* wave_x, double* wave_y, double* wave_z) {
    const double scale = normal_step / C_WAVE;
    field_x[dst] = field_x[src] - wave_x[src] * scale;
    field_y[dst] = field_y[src] - wave_y[src] * scale;
    field_z[dst] = field_z[src] - wave_z[src] * scale;
    wave_x[dst] = wave_x[src];
    wave_y[dst] = wave_y[src];
    wave_z[dst] = wave_z[src];
}

}  // namespace

// ============================================================================
// DEVICE TICK COUNTER (Component A)
// ============================================================================
// Issued at the very end of the tick body, exactly where the host does
// `tick_++`. Because it lives inside the recorded region, a replayed graph
// advances the RNG salt just as a direct-launch tick does.

__global__ void advance_device_tick_kernel(int* __restrict__ tick) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    *tick += 1;
}

void launch_advance_device_tick(GpuBuffers& bufs) {
    advance_device_tick_kernel<<<1, 1, 0, bufs.stream>>>(bufs.d_tick);
    CUDA_CHECK(cudaGetLastError());
}

// ============================================================================
// PRE-READ ELECTROWEAK BACKGROUND DRIVE
// ============================================================================

__global__ void ew_background_sweep_kernel(
    double* flux_x,
    double* flux_L_x,
    double* flux_R_x,
    const int* __restrict__ tick_ptr,
    bool dual_substrate,
    int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    const int tick = *tick_ptr;
    const double drive = (sin(static_cast<double>(tick) * 0.01) + 1.0) * 0.5 * 0.05;
    flux_x[i] += drive;
    if (dual_substrate) {
        const double half = drive * 0.5;
        flux_L_x[i] += half;
        flux_R_x[i] += half;
    }
}

void launch_ew_background_sweep(GpuBuffers& bufs, bool dual_substrate) {
    const cudaStream_t stream = bufs.stream;
    constexpr int threads = 256;
    const int blocks = (bufs.N + threads - 1) / threads;
    ew_background_sweep_kernel<<<blocks, threads, 0, stream>>>(
        bufs.d_flux_x, bufs.d_flux_L_x, bufs.d_flux_R_x,
        bufs.d_tick, dual_substrate, bufs.N);
    CUDA_CHECK(cudaGetLastError());
}

// ============================================================================
// POST-MOVEMENT FIELD BOUNDARIES
// ============================================================================

__global__ void absorbing_boundary_kernel(
    double* flux_x, double* flux_y, double* flux_z,
    double* wave_x, double* wave_y, double* wave_z,
    double* flux_L_x, double* flux_L_y, double* flux_L_z,
    double* flux_R_x, double* flux_R_y, double* flux_R_z,
    double* wave_L_x, double* wave_L_y, double* wave_L_z,
    double* wave_R_x, double* wave_R_y, double* wave_R_z,
    double* strong_x, double* strong_y, double* strong_z,
    double* strong_wave_x, double* strong_wave_y, double* strong_wave_z,
    double* weak_x, double* weak_y, double* weak_z,
    double* weak_wave_x, double* weak_wave_y, double* weak_wave_z,
    int L, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    const int LL = L * L;
    const int x = i / LL;
    const int y = (i / L) % L;
    const int z = i % L;
    const int Nm1 = L - 1;
    const int dx = min(x, Nm1 - x);
    const int dy = min(y, Nm1 - y);
    const int dz = min(z, Nm1 - z);
    const int d = min(dx, min(dy, dz));
    const int depth = min(6, max(2, L / 4));
    if (d >= depth) return;
    const double r = static_cast<double>(d) / static_cast<double>(depth);
    scale_boundary_fields(
        i, r * r,
        flux_x, flux_y, flux_z, wave_x, wave_y, wave_z,
        flux_L_x, flux_L_y, flux_L_z, flux_R_x, flux_R_y, flux_R_z,
        wave_L_x, wave_L_y, wave_L_z, wave_R_x, wave_R_y, wave_R_z,
        strong_x, strong_y, strong_z,
        strong_wave_x, strong_wave_y, strong_wave_z,
        weak_x, weak_y, weak_z, weak_wave_x, weak_wave_y, weak_wave_z);
}

// mode: 0 = periodic, 1 = reflective, 2 = dispersal. Reflective refreshes
// its Neumann ghost shell; Dispersal reconstructs an outward-only Sommerfeld
// trace on the complete outer shell and excises every non-field face record.
// Pre-read and post-writer calls intentionally apply the same operator.
// Every mode owns all six faces. Periodic requires no kernel because the
// lattice storage topology already identifies every opposite-face pair.
__global__ void flux_boundary_kernel(
    double* flux_x, double* flux_y, double* flux_z,
    double* wave_x, double* wave_y, double* wave_z,
    double* flux_L_x, double* flux_L_y, double* flux_L_z,
    double* flux_R_x, double* flux_R_y, double* flux_R_z,
    double* wave_L_x, double* wave_L_y, double* wave_L_z,
    double* wave_R_x, double* wave_R_y, double* wave_R_z,
    double* strong_x, double* strong_y, double* strong_z,
    double* strong_wave_x, double* strong_wave_y, double* strong_wave_z,
    double* weak_x, double* weak_y, double* weak_z,
    double* weak_wave_x, double* weak_wave_y, double* weak_wave_z,
    int mode, int L, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    const int LL = L * L;
    const int x = i / LL;
    const int y = (i / L) % L;
    const int z = i % L;
    const int Nm1 = L - 1;
    if (mode == 1) {
        if (x > 0 && x < Nm1 && y > 0 && y < Nm1 && z > 0 && z < Nm1)
            return;
        if (L < 3) return;
        // Every source coordinate is strictly interior, including faces,
        // edges, and corners. No thread writes a source read by another.
        const int sx = (x == 0) ? 1 : (x == Nm1 ? Nm1 - 1 : x);
        const int sy = (y == 0) ? 1 : (y == Nm1 ? Nm1 - 1 : y);
        const int sz = (z == 0) ? 1 : (z == Nm1 ? Nm1 - 1 : z);
        const int src = sx * LL + sy * L + sz;
        copy_boundary_fields(
            i, src,
            flux_x, flux_y, flux_z, wave_x, wave_y, wave_z,
            flux_L_x, flux_L_y, flux_L_z, flux_R_x, flux_R_y, flux_R_z,
            wave_L_x, wave_L_y, wave_L_z, wave_R_x, wave_R_y, wave_R_z,
            strong_x, strong_y, strong_z,
            strong_wave_x, strong_wave_y, strong_wave_z,
            weak_x, weak_y, weak_z, weak_wave_x, weak_wave_y, weak_wave_z);
        return;
    }

    if (mode == 0) return;
    if (x > 0 && x < Nm1 && y > 0 && y < Nm1 && z > 0 && z < Nm1)
        return;
    if (L < 3) {
        scale_boundary_fields(
            i, 0.0,
            flux_x, flux_y, flux_z, wave_x, wave_y, wave_z,
            flux_L_x, flux_L_y, flux_L_z, flux_R_x, flux_R_y, flux_R_z,
            wave_L_x, wave_L_y, wave_L_z, wave_R_x, wave_R_y, wave_R_z,
            strong_x, strong_y, strong_z,
            strong_wave_x, strong_wave_y, strong_wave_z,
            weak_x, weak_y, weak_z, weak_wave_x, weak_wave_y, weak_wave_z);
        return;
    }
    const int sx = (x == 0) ? 1 : (x == Nm1 ? Nm1 - 1 : x);
    const int sy = (y == 0) ? 1 : (y == Nm1 ? Nm1 - 1 : y);
    const int sz = (z == 0) ? 1 : (z == Nm1 ? Nm1 - 1 : z);
    const int src = sx * LL + sy * L + sz;
    const int face_count = (x == 0 || x == Nm1)
                         + (y == 0 || y == Nm1)
                         + (z == 0 || z == Nm1);
    const double normal_step = sqrt(static_cast<double>(face_count));
    copy_outflow_boundary_pair(i, src, normal_step,
        flux_x, flux_y, flux_z, wave_x, wave_y, wave_z);
    copy_outflow_boundary_pair(i, src, normal_step,
        flux_L_x, flux_L_y, flux_L_z, wave_L_x, wave_L_y, wave_L_z);
    copy_outflow_boundary_pair(i, src, normal_step,
        flux_R_x, flux_R_y, flux_R_z, wave_R_x, wave_R_y, wave_R_z);
    copy_outflow_boundary_pair(i, src, normal_step,
        strong_x, strong_y, strong_z,
        strong_wave_x, strong_wave_y, strong_wave_z);
    copy_outflow_boundary_pair(i, src, normal_step,
        weak_x, weak_y, weak_z, weak_wave_x, weak_wave_y, weak_wave_z);
}

// Dispersal clears every non-field dynamical register on the outer shell. The
// field families are replaced by the one-way ghost trace in the same stream.
__global__ void dispersal_record_excision_kernel(
    int8_t* state,
    double* velocity_x, double* velocity_y, double* velocity_z,
    double* remainder_x, double* remainder_y, double* remainder_z,
    uint8_t* locked, int32_t* particle_id, int32_t* pair_id,
    int8_t* spin, int8_t* color, int8_t* flavor,
    double* accel_mag, double* latency, double* tau, double* phase,
    double* phi, double* phi_coulomb, double* phi_latency,
    double* delta_x, double* delta_y, double* delta_z,
    double* delta_L_x, double* delta_L_y, double* delta_L_z,
    double* delta_R_x, double* delta_R_y, double* delta_R_z,
    uint8_t* near_particle, double* near_accel,
    double* fd_coulomb_x, double* fd_coulomb_y, double* fd_coulomb_z,
    double* fd_strong_x, double* fd_strong_y, double* fd_strong_z,
    double* fd_magnetic_x, double* fd_magnetic_y, double* fd_magnetic_z,
    double* fd_gravity_x, double* fd_gravity_y, double* fd_gravity_z,
    double* fd_exchange_x, double* fd_exchange_y, double* fd_exchange_z,
    int L, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    const int LL = L * L;
    const int x = i / LL;
    const int y = (i / L) % L;
    const int z = i % L;
    const int Nm1 = L - 1;
    if (x > 0 && x < Nm1 && y > 0 && y < Nm1 && z > 0 && z < Nm1)
        return;

    state[i] = 0;
    velocity_x[i] = velocity_y[i] = velocity_z[i] = 0.0;
    remainder_x[i] = remainder_y[i] = remainder_z[i] = 0.0;
    locked[i] = 0;
    particle_id[i] = -1;
    pair_id[i] = -1;
    spin[i] = color[i] = flavor[i] = 0;
    accel_mag[i] = latency[i] = tau[i] = phase[i] = 0.0;
    phi[i] = phi_coulomb[i] = phi_latency[i] = 0.0;
    delta_x[i] = delta_y[i] = delta_z[i] = 0.0;
    delta_L_x[i] = delta_L_y[i] = delta_L_z[i] = 0.0;
    delta_R_x[i] = delta_R_y[i] = delta_R_z[i] = 0.0;
    near_particle[i] = 0;
    near_accel[i] = 0.0;
    fd_coulomb_x[i] = fd_coulomb_y[i] = fd_coulomb_z[i] = 0.0;
    fd_strong_x[i] = fd_strong_y[i] = fd_strong_z[i] = 0.0;
    fd_magnetic_x[i] = fd_magnetic_y[i] = fd_magnetic_z[i] = 0.0;
    fd_gravity_x[i] = fd_gravity_y[i] = fd_gravity_z[i] = 0.0;
    fd_exchange_x[i] = fd_exchange_y[i] = fd_exchange_z[i] = 0.0;
}

namespace {

void launch_boundary_kernel(GpuBuffers& bufs, bool absorbing, int flux_mode) {
    const cudaStream_t stream = bufs.stream;
    constexpr int threads = 256;
    const int blocks = (bufs.N + threads - 1) / threads;
    if (absorbing) {
        absorbing_boundary_kernel<<<blocks, threads, 0, stream>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
            bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
            bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
            bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
            bufs.d_flux_strong_x, bufs.d_flux_strong_y, bufs.d_flux_strong_z,
            bufs.d_wave_vel_strong_x, bufs.d_wave_vel_strong_y, bufs.d_wave_vel_strong_z,
            bufs.d_flux_weak_x, bufs.d_flux_weak_y, bufs.d_flux_weak_z,
            bufs.d_wave_vel_weak_x, bufs.d_wave_vel_weak_y, bufs.d_wave_vel_weak_z,
            bufs.L, bufs.N);
    } else {
        flux_boundary_kernel<<<blocks, threads, 0, stream>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
            bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
            bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
            bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
            bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
            bufs.d_flux_strong_x, bufs.d_flux_strong_y, bufs.d_flux_strong_z,
            bufs.d_wave_vel_strong_x, bufs.d_wave_vel_strong_y, bufs.d_wave_vel_strong_z,
            bufs.d_flux_weak_x, bufs.d_flux_weak_y, bufs.d_flux_weak_z,
            bufs.d_wave_vel_weak_x, bufs.d_wave_vel_weak_y, bufs.d_wave_vel_weak_z,
            flux_mode, bufs.L, bufs.N);
        if (flux_mode == 2) {
            dispersal_record_excision_kernel<<<blocks, threads, 0, stream>>>(
                bufs.d_state,
                bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
                bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
                bufs.d_locked, bufs.d_particle_id, bufs.d_pair_id,
                bufs.d_spin, bufs.d_color, bufs.d_flavor,
                bufs.d_accel_mag, bufs.d_latency, bufs.d_tau, bufs.d_phase,
                bufs.d_phi, bufs.d_phi_coulomb, bufs.d_phi_latency,
                bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
                bufs.d_delta_j_L_x, bufs.d_delta_j_L_y, bufs.d_delta_j_L_z,
                bufs.d_delta_j_R_x, bufs.d_delta_j_R_y, bufs.d_delta_j_R_z,
                bufs.d_near_particle, bufs.d_near_accel,
                bufs.d_fd_coulomb_x, bufs.d_fd_coulomb_y, bufs.d_fd_coulomb_z,
                bufs.d_fd_strong_x, bufs.d_fd_strong_y, bufs.d_fd_strong_z,
                bufs.d_fd_magnetic_x, bufs.d_fd_magnetic_y, bufs.d_fd_magnetic_z,
                bufs.d_fd_gravity_x, bufs.d_fd_gravity_y, bufs.d_fd_gravity_z,
                bufs.d_fd_exchange_x, bufs.d_fd_exchange_y, bufs.d_fd_exchange_z,
                bufs.L, bufs.N);
        }
    }
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace

void launch_absorbing_boundary(GpuBuffers& bufs) {
    launch_boundary_kernel(bufs, true, 0);
}

void launch_prepare_flux_boundary(GpuBuffers& bufs, int mode) {
    launch_boundary_kernel(bufs, false, mode);
}

void launch_apply_flux_boundary(GpuBuffers& bufs, int mode) {
    launch_boundary_kernel(bufs, false, mode);
}

// ============================================================================
// SPECTROSCOPY PROBE GATHER (FTD-0281 rung-b, 2026-06-20)
// ============================================================================
// Gathers the (single-substrate observable) flux at a scattered probe-index set
// into a compact contiguous device array. The host then downloads only the
// compact array (n_probe doubles × 3) and sums J(0)·J(t) in fixed probe order —
// DETERMINISTIC (no float-order atomicAdd). This avoids the per-tick full-lattice
// device→host download that RenderBridge::tick() performs (1.3 GB/tick at L=256),
// the bottleneck that made large-L spectroscopy infeasible. Read-only: it only
// reads d_flux_* and writes the scratch gather arrays.
__global__ void gather_probe_flux_kernel(
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const int* __restrict__ probe_idx,
    double* __restrict__ out_x,
    double* __restrict__ out_y,
    double* __restrict__ out_z,
    int n_probe
) {
    int p = blockIdx.x * blockDim.x + threadIdx.x;
    if (p >= n_probe) return;
    const int i = probe_idx[p];
    out_x[p] = flux_x[i];
    out_y[p] = flux_y[i];
    out_z[p] = flux_z[i];
}

void launch_gather_probe_flux(const double* d_flux_x, const double* d_flux_y,
                              const double* d_flux_z, const int* d_probe_idx,
                              double* d_out_x, double* d_out_y, double* d_out_z,
                              int n_probe, cudaStream_t stream) {
    if (n_probe <= 0) return;
    int threads = 256;
    int blocks = (n_probe + threads - 1) / threads;
    gather_probe_flux_kernel<<<blocks, threads, 0, stream>>>(
        d_flux_x, d_flux_y, d_flux_z, d_probe_idx,
        d_out_x, d_out_y, d_out_z, n_probe);
    CUDA_CHECK(cudaGetLastError());
}

// ============================================================================
// WEAK TRANSMUTATION KERNEL [CLAUDE.md §6.5]
// ============================================================================
// When field stress |div(J)| + |curl(J)| + |grad(rho)| exceeds WEAK_THRESHOLD,
// manifested particles may flip polarity (+1 <-> -1).
//
// Dual-substrate convention (reconciled 2026-07-17, census EXPLR_DUAL_
// SUBSTRATE_STAGGERED_ENCODING §5.3): in dual mode ALL THREE stress terms —
// div, curl, AND ∇ρ — read J_L, matching the CPU reference
// (compute_stress_left = stress_field<&Voxel::flux_L>) and the declared
// L-only weak trigger (campaign_parity_violation.cpp). An earlier revision
// computed ∇ρ from the observable J; that mixed convention was a CPU/GPU
// parity gap in the weak firing rate. The K_GENESIS threshold applies
// unhalved: the L-substrate carries (1+δ)/2 ≈ 0.978 of the flux at +1
// particle sites (δ = DELTA_APPROX ≈ 0.957), so stress(J_L) stays near
// stress(J_obs) for the dominant chirality.

// Split into two kernels (decide → apply) to eliminate a read-after-write race
// the single in-place kernel had: in dual mode fL_x served as BOTH the const
// read alias (neighbor stencil) AND the swap target (fL_x_mut === d_flux_L_x at
// the launch site), so a neighbor thread's read of fL[i] raced this cell's
// swap-write of fL[i]. That made dual-substrate weak firing nondeterministic
// run-to-run. The decide kernel is pure read + writes only a per-site flip flag;
// the apply kernel writes only cell-local state/flux/wave_vel. Semantics are now
// snapshot (every site decides against the phase-entry flux) rather than the CPU
// reference's sequential-in-place order — the two were never bit-parity anyway
// (GPU thread order ≠ CPU active order), and GPC-17 validates weak transmutation
// at a 10% energy band, not to the bit.

// Decide phase: read-only over flux; sets flip_flags[i]=1 for a site that will
// transmute this tick. Assumes flip_flags is pre-zeroed (memset in the launcher),
// so early-returns leave the flag at 0.
__global__ void weak_transmutation_decide_kernel(
    const int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    bool dual_substrate,
    const double* __restrict__ fL_x,
    const double* __restrict__ fL_y,
    const double* __restrict__ fL_z,
    uint8_t* __restrict__ flip_flags,
    int L,
    unsigned long long rng_seed,
    const int* __restrict__ tick_ptr
) {
    const int tick = *tick_ptr;
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] == 0) return;  // Only manifested particles transmute (flag stays 0)

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

    // Gradient of density: ∇ρ where ρ = |J_L| in dual mode — fx/fy/fz are
    // the selected arrays, the same field the div/curl terms read (CPU
    // parity: stress_field<&Voxel::flux_L> applies all three terms to J_L).
    auto density = [&](int j) -> double {
        return sqrt(fx[j]*fx[j] + fy[j]*fy[j] + fz[j]*fz[j]);
    };
    double gx = 0.5 * (density(xp) - density(xm));
    double gy = 0.5 * (density(yp) - density(ym));
    double gz = 0.5 * (density(zp) - density(zm));
    double grad_mag = sqrt(gx*gx + gy*gy + gz*gz);

    double stress = div_mag + curl_mag + grad_mag;

    constexpr double weak_threshold = K_GENESIS;  // = N_c·K_MANIFEST
    if (stress <= weak_threshold) return;  // flag stays 0

    // Probabilistic flip: p = 1 - exp(-(stress - threshold) / K_MANIFEST)
    double p = 1.0 - exp(-(stress - weak_threshold) / K_MANIFEST);
    double r = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::WeakTransmutation));
    if (r < p) flip_flags[i] = 1;  // FIRE (RNG salt identical to the CPU path)
}

// Apply phase: each flagged site flips its own polarity and (dual mode) swaps its
// own L/R flux + wave_vel. Every write is cell-local (index i only) and the only
// read is flip_flags[i], so there is no cross-thread race — deterministic.
__global__ void weak_transmutation_apply_kernel(
    int8_t* __restrict__ state,
    const uint8_t* __restrict__ flip_flags,
    bool dual_substrate,
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
    if (!flip_flags[i]) return;

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
// CANONICAL GENESIS / EVAPORATION LIFECYCLE
// ============================================================================
// CPU phase_write intentionally interleaves these operations in ascending
// voxel order: an early genesis drain changes the live neighbour energy read by
// a later evaporation decision.  CUDA therefore detects the sparse event set
// in parallel, stably compacts it, and commits that ordered set on one device
// thread.  The post-write observable snapshot lives in d_delta_j_* after the
// leapfrog pass; those temporaries are dead until the next tick.

__global__ void lifecycle_candidate_kernel(
    const int8_t* __restrict__ state,
    const uint8_t* __restrict__ locked,
    const double* __restrict__ snapshot_x,
    const double* __restrict__ snapshot_y,
    const double* __restrict__ snapshot_z,
    uint8_t* __restrict__ flags,
    bool dual_substrate,
    bool do_genesis,
    bool do_evaporation,
    double genesis_threshold,
    double manifest_scale,
    int N,
    unsigned long long rng_seed,
    const int* __restrict__ tick_ptr) {
    const int tick = *tick_ptr;
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    uint8_t code = 0;
    if (do_genesis && state[i] == 0) {
        const double fx = snapshot_x[i];
        const double fy = snapshot_y[i];
        const double fz = snapshot_z[i];
        const double density2 = fx*fx + fy*fy + fz*fz;
        const double threshold = dual_substrate ? K_GENESIS : genesis_threshold;
        const double scale = dual_substrate ? K_MANIFEST : manifest_scale;
        // CPU tests the squared magnitude before taking sqrt.  Preserve that
        // exact threshold ordering at the one-ULP boundary.
        if (density2 > threshold * threshold) {
            const double density = sqrt(density2);
            const double p = 1.0 - exp(-(density - threshold) / scale);
            const double r = ::ftd::voxel_uniform(
                rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::GenesisManifest));
            if (r < p) code = 2;  // accepted genesis (then evaporation check)
        }
    } else if ((do_genesis || do_evaporation)
               && state[i] != 0 && !locked[i]) {
        code = 1;  // original manifested site: evaporation check only
    }
    flags[i] = code;
}

__device__ __forceinline__ double lifecycle_chirality_density(
    int i,
    const double* fL_x, const double* fL_y, const double* fL_z,
    const double* fR_x, const double* fR_y, const double* fR_z,
    const double* velocity_x, const double* velocity_y,
    const double* velocity_z) {
    const double vx = velocity_x[i], vy = velocity_y[i], vz = velocity_z[i];
    const double speed2 = vx*vx + vy*vy + vz*vz;
    if (speed2 > 1e-12) {
        const double inv_speed = 1.0 / sqrt(speed2);
        const double ex = vx * inv_speed;
        const double ey = vy * inv_speed;
        const double ez = vz * inv_speed;
        const double jl_dot = fL_x[i]*ex + fL_y[i]*ey + fL_z[i]*ez;
        const double jr_dot = fR_x[i]*ex + fR_y[i]*ey + fR_z[i]*ez;
        const double psi_l2 = fL_x[i]*fL_x[i] + fL_y[i]*fL_y[i]
                            + fL_z[i]*fL_z[i] - jl_dot*jl_dot;
        const double psi_r2 = fR_x[i]*fR_x[i] + fR_y[i]*fR_y[i]
                            + fR_z[i]*fR_z[i] - jr_dot*jr_dot;
        return psi_l2 - psi_r2;
    }
    return (fL_x[i]*fL_x[i] + fL_y[i]*fL_y[i])
         - (fR_x[i]*fR_x[i] + fR_y[i]*fR_y[i]);
}

__global__ void lifecycle_commit_kernel(
    int8_t* __restrict__ state,
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    double* __restrict__ wave_x,
    double* __restrict__ wave_y,
    double* __restrict__ wave_z,
    double* __restrict__ flux_L_x,
    double* __restrict__ flux_L_y,
    double* __restrict__ flux_L_z,
    double* __restrict__ flux_R_x,
    double* __restrict__ flux_R_y,
    double* __restrict__ flux_R_z,
    const double* __restrict__ velocity_x,
    const double* __restrict__ velocity_y,
    const double* __restrict__ velocity_z,
    const double* __restrict__ latency,
    const uint8_t* __restrict__ locked,
    int8_t* __restrict__ spin,
    int8_t* __restrict__ color,
    int32_t* __restrict__ particle_id,
    int32_t* __restrict__ pair_id,
    int32_t* __restrict__ next_particle_id,
    int32_t* __restrict__ identity_error,
    int* __restrict__ ledger_reaction,
    const double* __restrict__ snapshot_x,
    const double* __restrict__ snapshot_y,
    const double* __restrict__ snapshot_z,
    const uint8_t* __restrict__ event_code,
    int32_t* __restrict__ event_indices,
    const int32_t* __restrict__ event_count,
    bool dual_substrate,
    double kinetic_drain,
    double genesis_threshold,
    int L,
    unsigned long long rng_seed,
    const int* __restrict__ tick_ptr) {
    const int tick = *tick_ptr;
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    const int count = *event_count;
    int survivor_count = 0;
    for (int q = 0; q < count; ++q) {
        const int i = event_indices[q];
        const uint8_t code = event_code[i];
        const int z = i % L;
        const int y = (i / L) % L;
        const int x = i / (L * L);

        if (code == 2 && state[i] == 0) {
            const double sx = snapshot_x[i];
            const double sy = snapshot_y[i];
            const double sz = snapshot_z[i];
            const double density = sqrt(sx*sx + sy*sy + sz*sz);
            int8_t new_state = -1;
            if (dual_substrate) {
                const double chi = lifecycle_chirality_density(
                    i, flux_L_x, flux_L_y, flux_L_z,
                    flux_R_x, flux_R_y, flux_R_z,
                    velocity_x, velocity_y, velocity_z);
                new_state = chi >= 0.0 ? 1 : -1;
            } else {
                const int xp = idx3d(x+1, y, z, L);
                const int xm = idx3d(x-1, y, z, L);
                const int yp = idx3d(x, y+1, z, L);
                const int ym = idx3d(x, y-1, z, L);
                const int zp = idx3d(x, y, z+1, L);
                const int zm = idx3d(x, y, z-1, L);
                double div = 0.0;
                div += (snapshot_x[xp] - snapshot_x[xm]) * 0.5;
                div += (snapshot_y[yp] - snapshot_y[ym]) * 0.5;
                div += (snapshot_z[zp] - snapshot_z[zm]) * 0.5;
                new_state = div > 0.0 ? 1 : -1;
                wave_x[i] *= (1.0 - kinetic_drain);
                wave_y[i] *= (1.0 - kinetic_drain);
                wave_z[i] *= (1.0 - kinetic_drain);
                if (density > K_GENESIS_FLUX_EPSILON) {
                    const double drain = fmax(
                        0.0, 1.0 - genesis_threshold / density);
                    flux_x[i] *= drain;
                    flux_y[i] *= drain;
                    flux_z[i] *= drain;
                }
            }

            state[i] = new_state;
            ledger_reaction[i] += static_cast<int>(new_state);
            particle_id[i] = -2;
            pair_id[i] = -1;

            const int xp = idx3d(x+1, y, z, L);
            const int xm = idx3d(x-1, y, z, L);
            const int yp = idx3d(x, y+1, z, L);
            const int ym = idx3d(x, y-1, z, L);
            const int zp = idx3d(x, y, z+1, L);
            const int zm = idx3d(x, y, z-1, L);
            const double curl_x =
                (snapshot_z[yp] - snapshot_z[ym]) * 0.5
              - (snapshot_y[zp] - snapshot_y[zm]) * 0.5;
            const double curl_y =
                (snapshot_x[zp] - snapshot_x[zm]) * 0.5
              - (snapshot_z[xp] - snapshot_z[xm]) * 0.5;
            const double curl_z =
                (snapshot_y[xp] - snapshot_y[xm]) * 0.5
              - (snapshot_x[yp] - snapshot_x[ym]) * 0.5;
            const double acx = fabs(curl_x), acy = fabs(curl_y), acz = fabs(curl_z);
            const double max_curl = fmax(acx, fmax(acy, acz));
            if (max_curl > 1e-15) {
                if (acz >= acx && acz >= acy) spin[i] = curl_z > 0.0 ? 1 : -1;
                else if (acy >= acx) spin[i] = curl_y > 0.0 ? 1 : -1;
                else spin[i] = curl_x > 0.0 ? 1 : -1;
            } else {
                const double rs = ::ftd::voxel_uniform(
                    rng_seed, i, tick,
                    static_cast<unsigned long long>(::ftd::VoxelRng::GenesisSpin));
                spin[i] = rs < 0.5 ? 1 : -1;
            }
            const double afx = fabs(flux_x[i]);
            const double afy = fabs(flux_y[i]);
            const double afz = fabs(flux_z[i]);
            if (afx >= afy && afx >= afz) color[i] = 1;
            else if (afy >= afx && afy >= afz) color[i] = 2;
            else color[i] = 3;
        }

        // Genesis implies the sister evaporation pass, exactly as on CPU.
        if (state[i] != 0 && !locked[i]) {
            const double local_flux2 = flux_x[i]*flux_x[i]
                                     + flux_y[i]*flux_y[i]
                                     + flux_z[i]*flux_z[i];
            const double local_wave2 = wave_x[i]*wave_x[i]
                                     + wave_y[i]*wave_y[i]
                                     + wave_z[i]*wave_z[i];
            double local_energy = local_flux2 + local_wave2;
            const int neighbors[6] = {
                idx3d(x+1,y,z,L), idx3d(x-1,y,z,L),
                idx3d(x,y+1,z,L), idx3d(x,y-1,z,L),
                idx3d(x,y,z+1,L), idx3d(x,y,z-1,L)
            };
            for (int n = 0; n < 6; ++n) {
                const int j = neighbors[n];
                const double neighbour_flux2 = flux_x[j]*flux_x[j]
                                             + flux_y[j]*flux_y[j]
                                             + flux_z[j]*flux_z[j];
                const double neighbour_wave2 = wave_x[j]*wave_x[j]
                                             + wave_y[j]*wave_y[j]
                                             + wave_z[j]*wave_z[j];
                local_energy += neighbour_flux2 + neighbour_wave2;
            }
            const double speed2 = velocity_x[i]*velocity_x[i]
                                + velocity_y[i]*velocity_y[i]
                                + velocity_z[i]*velocity_z[i];
            const double dtau = ::ftd::proper_time_rate(latency[i], speed2);
            const double evap_prob = exp(
                -local_energy / (K_MANIFEST * K_MANIFEST));
            const double u = ::ftd::voxel_uniform(
                rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::Evaporation));
            if (u < evap_prob * K_EVAP_RATE * dtau) {
                const int8_t old_state = state[i];
                state[i] = 0;
                ledger_reaction[i] -= static_cast<int>(old_state);
                particle_id[i] = -1;
                pair_id[i] = -1;
                spin[i] = 0;
                color[i] = 0;
            }
        }

        // event_indices is already stable and ascending. Compact surviving
        // genesis sentinels in-place while this serial canonical transaction
        // owns the list; destination <= source, so unread entries are never
        // overwritten. This replaces a second full-grid flag/select pass.
        if (particle_id[i] == -2) {
            event_indices[survivor_count++] = i;
        }
    }

    const int32_t next = *next_particle_id;
    if (survivor_count < 0 || next < 0
        || survivor_count > INT_MAX - next) {
        *identity_error = 1;
        return;
    }
    *next_particle_id = next + survivor_count;
    for (int q = 0; q < survivor_count; ++q) {
        particle_id[event_indices[q]] = next + q;
    }
}

void launch_canonical_lifecycle(
    GpuBuffers& bufs, bool dual_substrate,
    bool do_genesis, bool do_evaporation,
    double kinetic_drain, double genesis_threshold, double manifest_scale,
    unsigned long long rng_seed) {
    const int* const tick = bufs.d_tick;
    if (!do_genesis && !do_evaporation) return;
    const cudaStream_t stream = bufs.stream;
    const std::size_t field_bytes = static_cast<std::size_t>(bufs.N) * sizeof(double);
    if (do_genesis) {
        CUDA_CHECK(cudaMemcpyAsync(bufs.d_delta_j_x, bufs.d_flux_x, field_bytes,
                                   cudaMemcpyDeviceToDevice, stream));
        CUDA_CHECK(cudaMemcpyAsync(bufs.d_delta_j_y, bufs.d_flux_y, field_bytes,
                                   cudaMemcpyDeviceToDevice, stream));
        CUDA_CHECK(cudaMemcpyAsync(bufs.d_delta_j_z, bufs.d_flux_z, field_bytes,
                                   cudaMemcpyDeviceToDevice, stream));
    }

    constexpr int block = 256;
    const int grid = (bufs.N + block - 1) / block;
    lifecycle_candidate_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state, bufs.d_locked,
        bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
        bufs.d_pair_candidate_flags,
        dual_substrate, do_genesis, do_evaporation,
        genesis_threshold, manifest_scale, bufs.N, rng_seed, tick);
    CUDA_CHECK(cudaGetLastError());

    thrust::counting_iterator<int32_t> indices(0);
    CUDA_CHECK(cub::DeviceSelect::Flagged(
        bufs.d_pair_select_temp, bufs.pair_select_temp_bytes,
        indices, bufs.d_pair_candidate_flags,
        bufs.d_pair_candidate_indices, bufs.d_pair_candidate_count, bufs.N,
        stream));

    lifecycle_commit_kernel<<<1, 1, 0, stream>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_latency, bufs.d_locked,
        bufs.d_spin, bufs.d_color, bufs.d_particle_id, bufs.d_pair_id,
        bufs.d_next_particle_id, bufs.d_identity_error,
        bufs.d_ledger_reaction,
        bufs.d_delta_j_x, bufs.d_delta_j_y, bufs.d_delta_j_z,
        bufs.d_pair_candidate_flags,
        bufs.d_pair_candidate_indices, bufs.d_pair_candidate_count,
        dual_substrate, kinetic_drain, genesis_threshold,
        bufs.L, rng_seed, tick);
    CUDA_CHECK(cudaGetLastError());
}

// Pair production has a real live-order dependency: two eligible sources can
// target the same void partner.  The CPU contract is a greedy ascending-index
// walk.  Detect eligibility in parallel, use CUB's stable Flagged selection to
// compact X-major indices, then let one device thread commit that sparse list
// in order.  This preserves GPU throughput for the expensive predicate while
// making conflicts and identity labels independent of CUDA scheduling.

__device__ __forceinline__ int pair_partner_index(
    int i, double fx, double fy, double fz, int L) {
    const int z = i % L;
    const int y = (i / L) % L;
    const int x = i / (L * L);
    int dx = 0, dy = 0, dz = 0;
    const double afx = fabs(fx), afy = fabs(fy), afz = fabs(fz);
    if (afx >= afy && afx >= afz) dx = (fx > 0.0) ? 1 : -1;
    else if (afy >= afx && afy >= afz) dy = (fy > 0.0) ? 1 : -1;
    else dz = (fz > 0.0) ? 1 : -1;
    return idx3d(x + dx, y + dy, z + dz, L);
}

__global__ void pair_production_candidate_kernel(
    const int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    uint8_t* __restrict__ candidate,
    int L, unsigned long long rng_seed,
    const int* __restrict__ tick_ptr) {
    const int tick = *tick_ptr;
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    candidate[i] = 0;
    if (state[i] != 0) return;

    const double fx = flux_x[i];
    const double fy = flux_y[i];
    const double fz = flux_z[i];
    const double jmag = sqrt(fx*fx + fy*fy + fz*fz);
    if (jmag <= K_GENESIS) return;

    // Same selected hazard and deterministic stream as the CPU path.
    const double p = 1.0 - exp(-(jmag - K_GENESIS) / K_MANIFEST);
    const double r = ::ftd::voxel_uniform(rng_seed, i, tick,
                static_cast<unsigned long long>(::ftd::VoxelRng::PairProduction));
    if (r >= p) return;

    const int partner = pair_partner_index(i, fx, fy, fz, L);
    if (state[partner] != 0) return;
    candidate[i] = 1;
}

__global__ void pair_production_commit_kernel(
    int8_t* __restrict__ state,
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    double* __restrict__ wave_vel_x,
    double* __restrict__ wave_vel_y,
    double* __restrict__ wave_vel_z,
    double* __restrict__ flux_L_x,
    double* __restrict__ flux_L_y,
    double* __restrict__ flux_L_z,
    double* __restrict__ flux_R_x,
    double* __restrict__ flux_R_y,
    double* __restrict__ flux_R_z,
    double* __restrict__ wave_vel_L_x,
    double* __restrict__ wave_vel_L_y,
    double* __restrict__ wave_vel_L_z,
    double* __restrict__ wave_vel_R_x,
    double* __restrict__ wave_vel_R_y,
    double* __restrict__ wave_vel_R_z,
    int32_t* __restrict__ particle_id,
    int32_t* __restrict__ pair_id,
    int32_t* __restrict__ next_particle_id,
    int32_t* __restrict__ next_pair_id,
    int32_t* __restrict__ identity_error,
    int* __restrict__ ledger_reaction,
    const int32_t* __restrict__ candidates,
    const int32_t* __restrict__ candidate_count,
    bool dual_substrate,
    int L) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    const int count = *candidate_count;
    for (int q = 0; q < count; ++q) {
        const int i = candidates[q];
        if (state[i] != 0) continue;

        const double fx = flux_x[i];
        const double fy = flux_y[i];
        const double fz = flux_z[i];
        const double jmag = sqrt(fx*fx + fy*fy + fz*fz);
        const int partner = pair_partner_index(i, fx, fy, fz, L);
        if (state[partner] != 0) continue;

        // Leave the transaction untouched if the representable identity
        // namespace is exhausted.  The final INT_MAX sentinel is deliberately
        // not issued because no non-wrapping successor could be recorded.
        if (*next_particle_id < 0 || *next_pair_id < 0
            || *next_particle_id > INT_MAX - 2
            || *next_pair_id >= INT_MAX) {
            *identity_error = 1;
            return;
        }

        state[i] = -1;
        state[partner] = 1;

        const double drain = fmax(0.0, 1.0 - K_GENESIS / jmag);
        wave_vel_x[i] *= 0.5; wave_vel_y[i] *= 0.5; wave_vel_z[i] *= 0.5;
        wave_vel_x[partner] *= 0.5;
        wave_vel_y[partner] *= 0.5;
        wave_vel_z[partner] *= 0.5;
        flux_x[i] *= drain; flux_y[i] *= drain; flux_z[i] *= drain;
        flux_x[partner] = -flux_x[i];
        flux_y[partner] = -flux_y[i];
        flux_z[partner] = -flux_z[i];

        if (dual_substrate) {
            flux_L_x[i] *= drain; flux_L_y[i] *= drain; flux_L_z[i] *= drain;
            flux_R_x[i] *= drain; flux_R_y[i] *= drain; flux_R_z[i] *= drain;
            flux_L_x[partner] = -flux_L_x[i];
            flux_L_y[partner] = -flux_L_y[i];
            flux_L_z[partner] = -flux_L_z[i];
            flux_R_x[partner] = -flux_R_x[i];
            flux_R_y[partner] = -flux_R_y[i];
            flux_R_z[partner] = -flux_R_z[i];

            wave_vel_L_x[i] *= 0.5; wave_vel_L_y[i] *= 0.5; wave_vel_L_z[i] *= 0.5;
            wave_vel_R_x[i] *= 0.5; wave_vel_R_y[i] *= 0.5; wave_vel_R_z[i] *= 0.5;
            wave_vel_L_x[partner] *= 0.5;
            wave_vel_L_y[partner] *= 0.5;
            wave_vel_L_z[partner] *= 0.5;
            wave_vel_R_x[partner] *= 0.5;
            wave_vel_R_y[partner] *= 0.5;
            wave_vel_R_z[partner] *= 0.5;

            flux_x[i] = flux_L_x[i] + flux_R_x[i];
            flux_y[i] = flux_L_y[i] + flux_R_y[i];
            flux_z[i] = flux_L_z[i] + flux_R_z[i];
            flux_x[partner] = flux_L_x[partner] + flux_R_x[partner];
            flux_y[partner] = flux_L_y[partner] + flux_R_y[partner];
            flux_z[partner] = flux_L_z[partner] + flux_R_z[partner];
            wave_vel_x[i] = wave_vel_L_x[i] + wave_vel_R_x[i];
            wave_vel_y[i] = wave_vel_L_y[i] + wave_vel_R_y[i];
            wave_vel_z[i] = wave_vel_L_z[i] + wave_vel_R_z[i];
            wave_vel_x[partner] = wave_vel_L_x[partner] + wave_vel_R_x[partner];
            wave_vel_y[partner] = wave_vel_L_y[partner] + wave_vel_R_y[partner];
            wave_vel_z[partner] = wave_vel_L_z[partner] + wave_vel_R_z[partner];
        }

        const int32_t primary_pid = *next_particle_id;
        const int32_t partner_pid = primary_pid + 1;
        *next_particle_id += 2;
        const int32_t shared_pair_id = (*next_pair_id)++;
        particle_id[i] = primary_pid;
        particle_id[partner] = partner_pid;
        pair_id[i] = shared_pair_id;
        pair_id[partner] = shared_pair_id;
        if (ledger_reaction) {
            ledger_reaction[i] -= 1;
            ledger_reaction[partner] += 1;
        }
    }
}

void launch_pair_production(GpuBuffers& bufs, bool dual_substrate,
                            unsigned long long rng_seed) {
    const cudaStream_t stream = bufs.stream;
    const int* const tick = bufs.d_tick;
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    pair_production_candidate_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state, bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_pair_candidate_flags, L, rng_seed, tick);
    CUDA_CHECK(cudaGetLastError());

    thrust::counting_iterator<int32_t> indices(0);
    CUDA_CHECK(cub::DeviceSelect::Flagged(
        bufs.d_pair_select_temp, bufs.pair_select_temp_bytes,
        indices, bufs.d_pair_candidate_flags,
        bufs.d_pair_candidate_indices, bufs.d_pair_candidate_count, bufs.N,
        stream));

    pair_production_commit_kernel<<<1, 1, 0, stream>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
        bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
        bufs.d_particle_id,
        bufs.d_pair_id,
        bufs.d_next_particle_id, bufs.d_next_pair_id,
        bufs.d_identity_error,
        bufs.d_ledger_reaction,
        bufs.d_pair_candidate_indices, bufs.d_pair_candidate_count,
        dual_substrate, L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate, unsigned long long rng_seed) {
    const cudaStream_t stream = bufs.stream;
    const int* const tick = bufs.d_tick;
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    // Flip-flag scratch (uint8[N]): reuse the movement arrival-guard buffer.
    // Movement (gpu_phase_movement, tick step ~7) is sequenced before weak
    // transmutation (tick step ~9) on this stream and consumes d_movement_moved
    // entirely within launch_phase_movement, so it is free here — the same
    // cross-phase scratch reuse the movement/pair compaction already relies on.
    // Zeroed first; weak_transmutation_decide_kernel only ever writes 1 (fire),
    // so its early-returns leave 0. Same-stream ordering (incl. under graph
    // capture) keeps memset → decide → apply correctly sequenced.
    uint8_t* flip_flags = bufs.d_movement_moved;
    CUDA_CHECK(cudaMemsetAsync(flip_flags, 0,
                               static_cast<size_t>(bufs.N) * sizeof(uint8_t), stream));

    weak_transmutation_decide_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        dual_substrate,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        flip_flags,
        L,
        rng_seed, tick
    );
    CUDA_CHECK(cudaGetLastError());

    weak_transmutation_apply_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state,
        flip_flags,
        dual_substrate,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_wave_vel_L_x, bufs.d_wave_vel_L_y, bufs.d_wave_vel_L_z,
        bufs.d_wave_vel_R_x, bufs.d_wave_vel_R_y, bufs.d_wave_vel_R_z,
        bufs.d_ledger_reaction,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
