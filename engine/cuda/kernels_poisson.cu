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
#include <cuda_runtime.h>
#include <cufft.h>
#include <cstdio>

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

#define CUFFT_CHECK(call) do { \
    cufftResult err = (call); \
    if (err != CUFFT_SUCCESS) { \
        fprintf(stderr, "cuFFT error at %s:%d: %d\n", \
                __FILE__, __LINE__, (int)err); \
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
    int N
) {
    int threads = 256;
    int blocks = (N + threads - 1) / threads;

    // 1. Pack double RHS into float complex
    pack_real_to_complex_f<<<blocks, threads>>>(d_rhs, d_fft_buf, N);

    // 2. Forward C2C FFT (in-place, single precision)
    CUFFT_CHECK(cufftExecC2C(plan_fwd, d_fft_buf, d_fft_buf, CUFFT_FORWARD));

    // 3. Apply Green's function
    apply_green_f<<<blocks, threads>>>(d_fft_buf, d_green, N);

    // 4. Inverse C2C FFT (in-place)
    CUFFT_CHECK(cufftExecC2C(plan_inv, d_fft_buf, d_fft_buf, CUFFT_INVERSE));

    // 5. Unpack float to double + normalize
    unpack_complex_to_real_f<<<blocks, threads>>>(d_fft_buf, d_phi, N);
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
    double mean_charge,
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

    rhs[i] = div - charge_coupling * (static_cast<double>(state[i]) - mean_charge);
}

// ---------- Compute Coulomb RHS: rho = -state (+ mean charge subtracted) ----------

__global__ void compute_coulomb_rhs(
    const int8_t* __restrict__ state,
    double* __restrict__ rhs,
    double mean_charge,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    rhs[i] = -(static_cast<double>(state[i]) - mean_charge);
}

// ---------- Compute Latency RHS: 4*pi*G * K_B * |state| ----------
//
// The latency Poisson equation is:
//   laplacian(phi) = 4*pi*G * rho_mass
// where rho_mass = K_B * |state| (mass density of manifested sites).
//
// The FFT Poisson solver automatically zeroes the DC Fourier mode
// (Green[0]=0 in gpu_buffers.cu precompute_green_function), which is
// equivalent to the CPU's mean-subtraction of both the RHS and phi.
// So we can feed the raw mass density directly without subtracting
// the mean — the periodic BC gauge fixing handles it.

__global__ void compute_latency_rhs(
    const int8_t* __restrict__ state,
    double* __restrict__ rhs,
    double four_pi_G_kB,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    int s = static_cast<int>(state[i]);
    double abs_s = static_cast<double>(s < 0 ? -s : s);
    rhs[i] = four_pi_G_kB * abs_s;
}

// ---------- Convert phi_latency to voxel.latency = sqrt(clamp(|phi|, 0, LATENCY_HORIZON_CLAMP)) ----------

__global__ void latency_to_voxel_kernel(
    double* __restrict__ voxel_latency,
    const double* __restrict__ phi_latency,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    double phi_val = phi_latency[i];
    double abs_phi = phi_val < 0.0 ? -phi_val : phi_val;
    double clamped = abs_phi > LATENCY_HORIZON_CLAMP ? LATENCY_HORIZON_CLAMP : abs_phi;
    voxel_latency[i] = sqrt(clamped);
}

// ---------- Proper time accumulation + bandwidth speed limit ----------
//
// CPU counterpart: render_bridge.cpp:1152-1178 (Rule 8).
//   For each manifested site (state != 0):
//     L = latency[i]
//     f = 1 - L²
//     if (f > 0):
//       v2 = velocity.mag²
//       arg = f² - v²
//       if (arg > 0): tau[i] += sqrt(arg) / sqrt(f)
//       v_max = C_SPEED * max(f, 0.001)
//       if (|v| > v_max): v *= (v_max / |v|)
//
// This implements gravitational time dilation AND the bandwidth speed
// limit (speed < f × C_SPEED near mass, where f shrinks as L grows).

__global__ void latency_tau_bandwidth_kernel(
    double* __restrict__ tau,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    const double* __restrict__ latency,
    const int8_t* __restrict__ state,
    double c_speed,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    if (state[i] == 0) return;  // Only manifested sites

    double L = latency[i];
    double f = 1.0 - L * L;
    if (f <= 0.0) return;

    double vx = vel_x[i];
    double vy = vel_y[i];
    double vz = vel_z[i];
    double v2 = vx * vx + vy * vy + vz * vz;
    double speed = sqrt(v2);

    // Proper time: dτ/dt = √(f² - v²) / √f
    double arg = f * f - v2;
    if (arg > 0.0) {
        tau[i] += sqrt(arg) / sqrt(f);
    }

    // Bandwidth constraint: effective speed limit is f × C_SPEED
    double f_clamped = f > 0.001 ? f : 0.001;
    double v_max = c_speed * f_clamped;
    if (speed > v_max) {
        double scale = v_max / speed;
        vel_x[i] = vx * scale;
        vel_y[i] = vy * scale;
        vel_z[i] = vz * scale;
    }
}

// ---------- Gauss correction: subtract gradient(phi) from flux at void sites ----------

__global__ void gauss_correction_kernel(
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    const double* __restrict__ phi,
    const int8_t* __restrict__ state,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] != 0) return;  // Only correct void sites

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
    int N
) {
    int threads = 256;
    int blocks = (N + threads - 1) / threads;

    // 1. Pack real RHS into complex buffer
    pack_real_to_complex<<<blocks, threads>>>(d_rhs, d_fft_buf, N);

    // 2. Forward FFT (in-place)
    CUFFT_CHECK(cufftExecZ2Z(plan_fwd, d_fft_buf, d_fft_buf, CUFFT_FORWARD));

    // 3. Apply Green's function in k-space
    apply_green<<<blocks, threads>>>(d_fft_buf, d_green, N);

    // 4. Inverse FFT (in-place)
    CUFFT_CHECK(cufftExecZ2Z(plan_inv, d_fft_buf, d_fft_buf, CUFFT_INVERSE));

    // 5. Unpack + normalize
    unpack_complex_to_real<<<blocks, threads>>>(d_fft_buf, d_phi, N);
}

// ---------- Launcher: Gauss Projection ----------
// Uses float-precision C2C FFT path for 2× faster Poisson solve.
// Float precision (7 digits) is more than sufficient for the correction gradient ∇φ.

void launch_gauss_project(GpuBuffers& bufs,
                          double charge_coupling,
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    int L = bufs.L;
    int N = bufs.N;

    // Step 0: mean_charge = charge_sum / N (mirrors gauss_project_cpu:148-149).
    // Exact integer reduction → bit-deterministic; copy to host to form the
    // scalar mean_charge passed into the RHS kernel.
    double mean_charge = 0.0;
    {
        long long* d_charge_sum = nullptr;
        CUDA_CHECK(cudaMalloc(&d_charge_sum, sizeof(long long)));
        CUDA_CHECK(cudaMemset(d_charge_sum, 0, sizeof(long long)));
        int threads = 256;
        int blocks = (N + threads - 1) / threads;
        sum_state_kernel<<<blocks, threads>>>(bufs.d_state, d_charge_sum, N);
        CUDA_CHECK(cudaGetLastError());
        long long charge_sum = 0;
        CUDA_CHECK(cudaMemcpy(&charge_sum, d_charge_sum, sizeof(long long),
                              cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaFree(d_charge_sum));
        mean_charge = static_cast<double>(charge_sum) / static_cast<double>(N);
    }

    // Step 1: Compute RHS = div(J) - charge_coupling*(state - mean_charge)
    {
        dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
        dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);
        compute_gauss_rhs<<<grid, block>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_state,
            bufs.d_phi,
            charge_coupling,
            mean_charge,
            L
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Step 2: FFT Poisson solve (float precision)
    fft_poisson_solve_f(bufs.d_phi, bufs.d_phi,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N);

    // Step 3: Gauss correction — subtract gradient(phi) from flux at void sites
    {
        dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
        dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);
        gauss_correction_kernel<<<grid, block>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_phi, bufs.d_state, L
        );
        CUDA_CHECK(cudaGetLastError());
    }
}

// ---------- Launcher: Coulomb Potential ----------

void launch_solve_coulomb(GpuBuffers& bufs,
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    int N = bufs.N;

    // Step 1: Compute RHS = -state (mean-subtracted)
    {
        int threads = 256;
        int blocks = (N + threads - 1) / threads;
        compute_coulomb_rhs<<<blocks, threads>>>(
            bufs.d_state, bufs.d_phi_coulomb, 0.0, N
        );
        CUDA_CHECK(cudaGetLastError());
    }

    // Step 2: FFT Poisson solve (float precision)
    fft_poisson_solve_f(bufs.d_phi_coulomb, bufs.d_phi_coulomb,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N);
}

// ---------- Launcher: Latency Poisson ----------
//
// Solves laplacian(phi) = 4*pi*G * K_B * |state| for the gravitational
// potential phi_latency, then writes voxel.latency = sqrt(clamp(|phi|, 0, LATENCY_HORIZON_CLAMP)).
//
// This is the GPU counterpart of RenderBridge::solve_latency_poisson()
// (engine/src/render_bridge.cpp:696-747). It unblocks every test that
// previously had to call rb.force_cpu() because CUDA lacked this feature.
// Wave 5 GPU-first sweep (2026-04-14).

void launch_solve_latency(GpuBuffers& bufs,
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    int N = bufs.N;

    int threads = 256;
    int blocks = (N + threads - 1) / threads;

    // Step 1: Compute RHS = 4*pi*G * M_REST * |state|
    // (DC mode is automatically zeroed by Green's function, equivalent
    // to the CPU's mean-subtraction of rho_mass.)
    const double FOUR_PI_G_K_B = 4.0 * PI * G_N * M_REST;
    compute_latency_rhs<<<blocks, threads>>>(
        bufs.d_state, bufs.d_phi_latency, FOUR_PI_G_K_B, N
    );
    CUDA_CHECK(cudaGetLastError());

    // Step 2: FFT Poisson solve (float precision — same as Coulomb/Gauss)
    fft_poisson_solve_f(bufs.d_phi_latency, bufs.d_phi_latency,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N);

    // Step 3: Convert phi_latency → voxel.latency via sqrt(clamp(|phi|, 0, LATENCY_HORIZON_CLAMP))
    latency_to_voxel_kernel<<<blocks, threads>>>(
        bufs.d_latency, bufs.d_phi_latency, N
    );
    CUDA_CHECK(cudaGetLastError());

    // Step 4: Accumulate proper time tau and apply bandwidth speed limit.
    // Matches CPU Rule 8 (render_bridge.cpp:1152-1178).
    latency_tau_bandwidth_kernel<<<blocks, threads>>>(
        bufs.d_tau,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_latency, bufs.d_state,
        C_SPEED, N
    );
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
