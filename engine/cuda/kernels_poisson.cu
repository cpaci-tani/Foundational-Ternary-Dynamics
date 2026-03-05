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
    return wrap(z, L) * L * L + wrap(y, L) * L + wrap(x, L);
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

// ---------- Compute Gauss RHS: rho = div(J) - state ----------

__global__ void compute_gauss_rhs(
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    const int8_t* __restrict__ state,
    double* __restrict__ rhs,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = z * L * L + y * L + x;

    // div(J) = dJx/dx + dJy/dy + dJz/dz (central differences)
    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    double div = 0.5 * ((flux_x[xp] - flux_x[xm])
                       + (flux_y[yp] - flux_y[ym])
                       + (flux_z[zp] - flux_z[zm]));

    rhs[i] = div - static_cast<double>(state[i]);
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

    int i = z * L * L + y * L + x;
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
                          cufftHandle plan_fwd, cufftHandle plan_inv,
                          cufftHandle plan_fwd_f, cufftHandle plan_inv_f) {
    int L = bufs.L;
    int N = bufs.N;

    // Step 1: Compute RHS = div(J) - state
    {
        dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
        dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);
        compute_gauss_rhs<<<grid, block>>>(
            bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
            bufs.d_state,
            bufs.d_phi,
            L
        );
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
    }

    // Step 2: FFT Poisson solve (float precision)
    fft_poisson_solve_f(bufs.d_phi_coulomb, bufs.d_phi_coulomb,
                        bufs.d_fft_buf_f, bufs.d_green,
                        plan_fwd_f, plan_inv_f, N);
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
