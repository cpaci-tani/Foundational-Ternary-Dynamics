/**
 * FFT-based Poisson solver for FTD GPU engine.
 *
 * Replaces the SOR iterative solver with exact spectral solution:
 *   ∇²φ = ρ  →  φ̂(k) = ρ̂(k) / G(k)
 *
 * where G(k) is the discrete Laplacian eigenvalue for periodic BC.
 * Uses cuFFT for O(N log N) computation vs O(N × 30 iters) for SOR.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/volumetric_measure.h"
#include <cuda_runtime.h>
#include <cufft.h>
#include <cstdio>

#define FTD_CUDA_ERROR_WANT_CUFFT
#include "cuda_error.cuh"  // CUDA_CHECK + CUFFT_CHECK (revision C1)


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

// ---------- Pack real RHS into complex buffer for FFT ----------

__global__ void pack_real_to_complex(
    const double* __restrict__ rhs,
    cufftDoubleComplex* __restrict__ out,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    out[i].x = rhs[i];
    out[i].y = 0.0;
}

// ---------- Apply Green's function in k-space ----------
// φ̂(k) = ρ̂(k) * green(k)  where green(k) = 1/G(k)

__global__ void apply_green(
    cufftDoubleComplex* __restrict__ data,
    const double* __restrict__ green,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    double g = green[i];
    data[i].x *= g;
    data[i].y *= g;
}

// ---------- Unpack complex result to real + normalize ----------

__global__ void unpack_complex_to_real(
    const cufftDoubleComplex* __restrict__ in,
    double* __restrict__ out,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    // cuFFT does unnormalized FFT; divide by N for inverse
    out[i] = in[i].x / static_cast<double>(N);
}

// ============================================================================
// SINGLE-PRECISION (C2C) FFT PATH — 2× faster than Z2Z on Blackwell
// ============================================================================

// Pack double RHS into float complex buffer
__global__ void pack_real_to_complex_f(
    const double* __restrict__ rhs,
    cufftComplex* __restrict__ out,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    out[i].x = static_cast<float>(rhs[i]);
    out[i].y = 0.0f;
}

// Apply Green's function in k-space (float complex × double green → float complex)
__global__ void apply_green_f(
    cufftComplex* __restrict__ data,
    const double* __restrict__ green,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    float g = static_cast<float>(green[i]);
    data[i].x *= g;
    data[i].y *= g;
}

// Unpack float complex result to double + normalize
__global__ void unpack_complex_to_real_f(
    const cufftComplex* __restrict__ in,
    double* __restrict__ out,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    out[i] = static_cast<double>(in[i].x) / static_cast<double>(N);
}

// Float-precision FFT Poisson solve
static void fft_poisson_solve_f(
    double* d_rhs,           // input: RHS on device (N doubles)
    double* d_phi,           // output: solution on device (N doubles)
    cufftComplex* d_fft_buf, // workspace (N float complex)
    const double* d_green,   // precomputed 1/G(k) (double — computed once)
    cufftHandle plan_fwd,
    cufftHandle plan_inv,
    int N,
    cudaStream_t stream
) {
    int threads = 256;
    int blocks = (N + threads - 1) / threads;

    // 1. Pack double RHS into float complex
    pack_real_to_complex_f<<<blocks, threads, 0, stream>>>(d_rhs, d_fft_buf, N);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently

    // 2. Forward C2C FFT (in-place, single precision)
    CUFFT_CHECK(cufftExecC2C(plan_fwd, d_fft_buf, d_fft_buf, CUFFT_FORWARD));

    // 3. Apply Green's function
    apply_green_f<<<blocks, threads, 0, stream>>>(d_fft_buf, d_green, N);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently

    // 4. Inverse C2C FFT (in-place)
    CUFFT_CHECK(cufftExecC2C(plan_inv, d_fft_buf, d_fft_buf, CUFFT_INVERSE));

    // 5. Unpack float to double + normalize
    unpack_complex_to_real_f<<<blocks, threads, 0, stream>>>(d_fft_buf, d_phi, N);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently
}

// ============================================================================
// RHS COMPUTATION AND CORRECTION KERNELS (shared by both precision paths)
// ============================================================================

// ---------- Sum ternary state into an integer (exact, deterministic) ----------
//
// Mirrors CPU TernaryField::charge_sum() (poisson_solvers.cpp:148). Integer
// atomicAdd is associative/exact, so the reduction is order-independent and
// bit-deterministic — unlike a float reduction. The host divides by N to form
// mean_charge for the Gauss RHS, matching gauss_project_cpu exactly.

__global__ void sum_state_kernel(
    const int8_t* __restrict__ state,
    long long* __restrict__ out_sum,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    int s = static_cast<int>(state[i]);
    // CUDA provides atomicAdd only for unsigned long long, not signed. Two's-
    // complement addition is bit-identical for signed/unsigned, so accumulate
    // through an unsigned reinterpret; the host reads the same bits back as a
    // signed long long. Exactness/determinism (associativity) is preserved and
    // |sum| <= N can never overflow int64.
    if (s != 0) {
        atomicAdd(reinterpret_cast<unsigned long long*>(out_sum),
                  static_cast<unsigned long long>(static_cast<long long>(s)));
    }
}

// ---------- Finalize mean_charge on the device ----------
//
// mean_charge = charge_sum / N. Previously the host read charge_sum back and
// divided; that blocking D2H ran 1-3x per tick. One thread now does the same
// division on-device and the RHS kernels read the result through a pointer.
// The arithmetic is identical (exact int64 sum, one binary64 division), so
// every downstream value is bit-identical to the previous host scalar.

__global__ void finalize_mean_charge_kernel(
    const long long* __restrict__ charge_sum,
    double* __restrict__ mean_charge,
    int N
) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    *mean_charge = static_cast<double>(*charge_sum) / static_cast<double>(N);
}

// ---------- Compute Gauss RHS: rho = div(J) - charge_coupling*(state - mean_charge) ----------
//
// Mirrors CPU gauss_project_cpu (poisson_solvers.cpp:164):
//   sor_source[i] = div - charge_coupling * (state[i] - mean_charge)
// Previously the GPU hardcoded charge_coupling=1 and mean_charge=0, silently
// dropping the coulomb_charge_coupling knob and the mean-charge subtraction.

__global__ void compute_gauss_rhs(
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const int8_t* __restrict__ state,
    double* __restrict__ rhs,
    double charge_coupling,
    const double* __restrict__ mean_charge,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)

    // div(J) = dJx/dx + dJy/dy + dJz/dz (central differences)
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    double div = 0.5 * ((flux_x[xp] - flux_x[xm])
                       + (flux_y[yp] - flux_y[ym])
                       + (flux_z[zp] - flux_z[zm]));

    rhs[i] = div - charge_coupling * (static_cast<double>(state[i]) - *mean_charge);
}

// ---------- Compute Coulomb RHS: rho = -state (+ mean charge subtracted) ----------

__global__ void compute_coulomb_rhs(
    const int8_t* __restrict__ state,
    double* __restrict__ rhs,
    const double* __restrict__ mean_charge,
    double charge_scale,   // FTD-0281 helium extension: nuclear-charge Z
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    // rho = -charge_scale·(s − mean_charge). charge_scale=1.0 reproduces the
    // legacy rho = -(s − mean_charge); charge_scale=2 doubles the He+ well.
    rhs[i] = -charge_scale * (static_cast<double>(state[i]) - *mean_charge);
}

// ---------- Compute Latency RHS: 4*pi*G * rho ----------
//
// The latency Poisson equation is:
//   laplacian(phi) = 4*pi*G * rho_mass
// where rho = M_GRAVITATIONAL*|state| and, when selected, the local field
// energy density 1/2(|J|^2+|W|^2). This mirrors solve_latency_poisson_cpu.
//
// The FFT Poisson solver automatically zeroes the DC Fourier mode
// (Green[0]=0 in gpu_buffers.cu precompute_green_function), which is
// equivalent to the CPU's mean-subtraction of both the RHS and phi.
// So we can feed the raw mass density directly without subtracting
// the mean — the periodic BC gauge fixing handles it.

__global__ void compute_latency_rhs(
    const int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const double* __restrict__ wave_x,
    const double* __restrict__ wave_y,
    const double* __restrict__ wave_z,
    double* __restrict__ rhs,
    double four_pi_G,
    bool include_field_energy,
    const double* __restrict__ strong_t00,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    int s = static_cast<int>(state[i]);
    double abs_s = static_cast<double>(s < 0 ? -s : s);
    double rho = M_GRAVITATIONAL * abs_s;
    if (include_field_energy) {
        const double flux2 = flux_x[i] * flux_x[i]
                           + flux_y[i] * flux_y[i]
                           + flux_z[i] * flux_z[i];
        const double wave2 = wave_x[i] * wave_x[i]
                           + wave_y[i] * wave_y[i]
                           + wave_z[i] * wave_z[i];
        rho += ::ftd::local_field_wave_energy_density(flux2, wave2);
    }
    if (strong_t00) rho += strong_t00[i] / (C_SPEED * C_SPEED);
    rhs[i] = four_pi_G * rho;
}

// ---------- Convert phi_latency to voxel.latency = sqrt(clamp(|phi|, 0, LATENCY_HORIZON_CLAMP)) ----------

__global__ void latency_to_voxel_kernel(
    double* __restrict__ voxel_latency,
    const double* __restrict__ phi_latency,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    // P6 (2026-07-26): mirror the CPU fix -- map the WELL (-phi), not |phi|.
    // The mean-subtracted periodic potential is signed; |phi| turned the
    // under-dense majority of the box into a positive, non-monotone
    // pseudo-well. See poisson_solvers.cpp for the full note and measurements.
    double well = -phi_latency[i];
    double clamped = well < 0.0 ? 0.0
                   : (well > LATENCY_HORIZON_CLAMP ? LATENCY_HORIZON_CLAMP : well);
    voxel_latency[i] = sqrt(clamped);
}

// ---------- Gauss correction: subtract gradient(phi) from selected sites ----------

__global__ void gauss_correction_kernel(
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    const double* __restrict__ phi,
    const int8_t* __restrict__ state,
    bool exact_dual_gauss,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    // Production mode preserves the self-field at manifested sites. Exact
    // dual-Gauss mode applies the same correction everywhere, matching CPU
    // gauss_project_cpu's manifested-site semantics.
    if (!exact_dual_gauss && state[i] != 0) return;

    // Gradient of phi (central differences)
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    flux_x[i] -= 0.5 * (phi[xp] - phi[xm]);
    flux_y[i] -= 0.5 * (phi[yp] - phi[ym]);
    flux_z[i] -= 0.5 * (phi[zp] - phi[zm]);
}

// ---------- FFT Poisson solve (generic) ----------
// Solves ∇²φ = rhs using precomputed Green's function

static void fft_poisson_solve(
    double* d_rhs,        // input: RHS on device (N doubles)
    double* d_phi,        // output: solution on device (N doubles)
    cufftDoubleComplex* d_fft_buf,  // workspace (N complex)
    const double* d_green,           // precomputed 1/G(k)
    cufftHandle plan_fwd,
    cufftHandle plan_inv,
    int N,
    cudaStream_t stream
) {
    int threads = 256;
    int blocks = (N + threads - 1) / threads;

    // 1. Pack real RHS into complex buffer
    pack_real_to_complex<<<blocks, threads, 0, stream>>>(d_rhs, d_fft_buf, N);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently

    // 2. Forward FFT (in-place)
    CUFFT_CHECK(cufftExecZ2Z(plan_fwd, d_fft_buf, d_fft_buf, CUFFT_FORWARD));

    // 3. Apply Green's function in k-space
    apply_green<<<blocks, threads, 0, stream>>>(d_fft_buf, d_green, N);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently

    // 4. Inverse FFT (in-place)
    CUFFT_CHECK(cufftExecZ2Z(plan_inv, d_fft_buf, d_fft_buf, CUFFT_INVERSE));

    // 5. Unpack + normalize
    unpack_complex_to_real<<<blocks, threads, 0, stream>>>(d_fft_buf, d_phi, N);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently
}

// ---------- Launcher: Gauss Projection ----------
// Uses float-precision C2C FFT path for 2× faster Poisson solve.
// Float precision (7 digits) is more than sufficient for the correction gradient ∇φ.

void launch_gauss_project(GpuBuffers& bufs,
                          double charge_coupling,
                          bool exact_dual_gauss,
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    const cudaStream_t stream = bufs.stream;
    int L = bufs.L;
    int N = bufs.N;

    // Step 0: mean_charge = charge_sum / N (mirrors gauss_project_cpu:148-149).
    // Exact integer reduction → bit-deterministic. Fully device-resident:
    // no cudaMalloc/cudaFree and no blocking D2H in the tick path.
    {
        int threads = 256;
        int blocks = (N + threads - 1) / threads;
        CUDA_CHECK(cudaMemsetAsync(bufs.d_poisson_charge_sum, 0,
                                   sizeof(long long), stream));
        sum_state_kernel<<<blocks, threads, 0, stream>>>(
            bufs.d_state, bufs.d_poisson_charge_sum, N);
        CUDA_CHECK(cudaGetLastError());
        finalize_mean_charge_kernel<<<1, 1, 0, stream>>>(
            bufs.d_poisson_charge_sum, bufs.d_poisson_mean_charge, N);
        CUDA_CHECK(cudaGetLastError());
    }

    // Step 1: Compute RHS = div(J) - charge_coupling*(state - mean_charge)
    {
        dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
        dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);
        compute_gauss_rhs<<<grid, block, 0, stream>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_state,
            bufs.d_phi,
            charge_coupling,
            bufs.d_poisson_mean_charge,
            L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Step 2: FFT Poisson solve (float precision)
    fft_poisson_solve_f(bufs.d_phi, bufs.d_phi,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N, stream);

    // Step 3: Gauss correction — void-only normally, all sites in exact mode.
    {
        dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
        dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);
        gauss_correction_kernel<<<grid, block, 0, stream>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_phi, bufs.d_state, exact_dual_gauss, L
        );
        CUDA_CHECK(cudaGetLastError());
    }
}

// ---------- Launcher: Coulomb Potential ----------

void launch_solve_coulomb(GpuBuffers& bufs,
                          double charge_scale,
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    const cudaStream_t stream = bufs.stream;
    int N = bufs.N;

    // Step 0: mean_charge = charge_sum / N (mirrors solve_coulomb_poisson_cpu:232).
    // Exact integer reduction → bit-deterministic, fully device-resident.
    {
        int threads = 256;
        int blocks = (N + threads - 1) / threads;
        CUDA_CHECK(cudaMemsetAsync(bufs.d_poisson_charge_sum, 0,
                                   sizeof(long long), stream));
        sum_state_kernel<<<blocks, threads, 0, stream>>>(
            bufs.d_state, bufs.d_poisson_charge_sum, N);
        CUDA_CHECK(cudaGetLastError());
        finalize_mean_charge_kernel<<<1, 1, 0, stream>>>(
            bufs.d_poisson_charge_sum, bufs.d_poisson_mean_charge, N);
        CUDA_CHECK(cudaGetLastError());
    }

    // Step 1: Compute RHS = -charge_scale·(state − mean_charge)
    {
        int threads = 256;
        int blocks = (N + threads - 1) / threads;
        compute_coulomb_rhs<<<blocks, threads, 0, stream>>>(
            bufs.d_state, bufs.d_phi_coulomb, bufs.d_poisson_mean_charge,
            charge_scale, N
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Step 2: FFT Poisson solve (float precision)
    fft_poisson_solve_f(bufs.d_phi_coulomb, bufs.d_phi_coulomb,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N, stream);
}

// ---------- Launcher: Latency Poisson ----------
//
// Solves laplacian(phi) = 4*pi*G*rho for the gravitational potential, then
// writes voxel.latency = sqrt(clamp(-phi, 0, LATENCY_HORIZON_CLAMP)).
//
// This is the GPU counterpart of RenderBridge::solve_latency_poisson()
// (engine/src/render_bridge.cpp:696-747). It unblocks every test that
// previously had to call rb.force_cpu() because CUDA lacked this feature.
// Wave 5 GPU-first sweep (2026-04-14).

void launch_solve_latency(GpuBuffers& bufs,
                          bool include_field_energy,
                          const double* strong_t00,
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    const cudaStream_t stream = bufs.stream;
    int N = bufs.N;

    int threads = 256;
    int blocks = (N + threads - 1) / threads;

    // Step 1: Compute RHS = 4*pi*G *
    //   (M_GRAVITATIONAL*|state| + selected local field/wave energy)
    // (DC mode is automatically zeroed by Green's function, equivalent
    // to the CPU's mean-subtraction of rho_mass.)
    const double FOUR_PI_G = 4.0 * PI * G_N;
    compute_latency_rhs<<<blocks, threads, 0, stream>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z,
        bufs.d_phi_latency, FOUR_PI_G, include_field_energy, strong_t00, N
    );
    CUDA_CHECK(cudaGetLastError());

    // Step 2: FFT Poisson solve (float precision — same as Coulomb/Gauss)
    fft_poisson_solve_f(bufs.d_phi_latency, bufs.d_phi_latency,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N, stream);

    // Step 3: Convert phi_latency → voxel.latency via sqrt(clamp(|phi|, 0, LATENCY_HORIZON_CLAMP))
    latency_to_voxel_kernel<<<blocks, threads, 0, stream>>>(
        bufs.d_latency, bufs.d_phi_latency, N
    );
    CUDA_CHECK(cudaGetLastError());

    // Proper time is advanced exactly once by RenderBridge's common host
    // post-pass after device state is synchronized (FTD-0402). Movement owns
    // the only external-state causal projection; this solver never clamps u.
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
