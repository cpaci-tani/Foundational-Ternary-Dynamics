/**
 * @file kernels_eft.cu
 * @brief GPU-native EFT calculations: face-flux conversion, blocking, operator evaluation, and parallel reductions.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/eft/gpu_dual_cell_fields.cuh"
#include "ftd/constants.h"
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

namespace ftd {
namespace eft {
namespace gpu {

// ---------- Device helpers ----------

__device__ __forceinline__
int wrap(int x, int L) {
    return ((x % L) + L) % L;
}

__device__ __forceinline__
int idx3d(int x, int y, int z, int L) {
    return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

// ---------- Face-Flux Conversion Kernel ----------

__global__ void render_bridge_to_dual_cell_fields_gpu_kernel(
    const int8_t* __restrict__ state,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    int* __restrict__ d_rho_cell,
    double* __restrict__ d_phi_x,
    double* __restrict__ d_phi_y,
    double* __restrict__ d_phi_z,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;
    d_rho_cell[i] = static_cast<int>(state[i]);

    int ipx = idx3d(x + 1, y, z, L);
    int ipy = idx3d(x, y + 1, z, L);
    int ipz = idx3d(x, y, z + 1, L);

    d_phi_x[i] = 0.5 * (flux_x[i] + flux_x[ipx]);
    d_phi_y[i] = 0.5 * (flux_y[i] + flux_y[ipy]);
    d_phi_z[i] = 0.5 * (flux_z[i] + flux_z[ipz]);
}

// ---------- Dual-Cell Blocking (Coarse-Graining b=2) Kernel ----------

__global__ void block_dual_cell_b2_gpu_kernel(
    const int* __restrict__ fine_rho,
    const double* __restrict__ fine_phi_x,
    const double* __restrict__ fine_phi_y,
    const double* __restrict__ fine_phi_z,
    int* __restrict__ coarse_rho,
    double* __restrict__ coarse_phi_x,
    double* __restrict__ coarse_phi_y,
    double* __restrict__ coarse_phi_z,
    int fine_L, int coarse_L
) {
    int X = blockIdx.x * blockDim.x + threadIdx.x;
    int Y = blockIdx.y * blockDim.y + threadIdx.y;
    int Z = blockIdx.z * blockDim.z + threadIdx.z;
    if (X >= coarse_L || Y >= coarse_L || Z >= coarse_L) return;

    constexpr int b = 2;
    int q = 0;
    for (int dz = 0; dz < b; ++dz)
        for (int dy = 0; dy < b; ++dy)
            for (int dx = 0; dx < b; ++dx) {
                int fx = b * X + dx;
                int fy = b * Y + dy;
                int fz = b * Z + dz;
                q += fine_rho[fx * fine_L * fine_L + fy * fine_L + fz];
            }

    double phix = 0.0;
    double phiy = 0.0;
    double phiz = 0.0;

    for (int dz = 0; dz < b; ++dz)
        for (int dy = 0; dy < b; ++dy) {
            int fx = b * X + (b - 1);
            int fy = b * Y + dy;
            int fz = b * Z + dz;
            phix += fine_phi_x[fx * fine_L * fine_L + fy * fine_L + fz];
        }

    for (int dz = 0; dz < b; ++dz)
        for (int dx = 0; dx < b; ++dx) {
            int fx = b * X + dx;
            int fy = b * Y + (b - 1);
            int fz = b * Z + dz;
            phiy += fine_phi_y[fx * fine_L * fine_L + fy * fine_L + fz];
        }

    for (int dy = 0; dy < b; ++dy)
        for (int dx = 0; dx < b; ++dx) {
            int fx = b * X + dx;
            int fy = b * Y + dy;
            int fz = b * Z + (b - 1);
            phiz += fine_phi_z[fx * fine_L * fine_L + fy * fine_L + fz];
        }

    int ci = X * coarse_L * coarse_L + Y * coarse_L + Z;
    coarse_rho[ci] = q;
    coarse_phi_x[ci] = phix;
    coarse_phi_y[ci] = phiy;
    coarse_phi_z[ci] = phiz;
}

// ---------- EFT Operator Helper Functions ----------

__device__ __forceinline__ double cell_J(
    const double* __restrict__ phi_x,
    const double* __restrict__ phi_y,
    const double* __restrict__ phi_z,
    int x, int y, int z, int L, int axis
) {
    if (axis == 0) return 0.5 * (phi_x[idx3d(x, y, z, L)] + phi_x[idx3d(x - 1, y, z, L)]);
    if (axis == 1) return 0.5 * (phi_y[idx3d(x, y, z, L)] + phi_y[idx3d(x, y - 1, z, L)]);
    return 0.5 * (phi_z[idx3d(x, y, z, L)] + phi_z[idx3d(x, y, z - 1, L)]);
}

__device__ __forceinline__ double div_face_at(
    const double* __restrict__ phi_x,
    const double* __restrict__ phi_y,
    const double* __restrict__ phi_z,
    int x, int y, int z, int L
) {
    int i = idx3d(x, y, z, L);
    return (phi_x[i] - phi_x[idx3d(x - 1, y, z, L)]) +
           (phi_y[i] - phi_y[idx3d(x, y - 1, z, L)]) +
           (phi_z[i] - phi_z[idx3d(x, y, z - 1, L)]);
}

// ---------- EFT Operator Evaluation Kernel ----------

__global__ void compute_eft_operators_gpu_kernel(
    const int* __restrict__ before_rho,
    const double* __restrict__ before_phi_x,
    const double* __restrict__ before_phi_y,
    const double* __restrict__ before_phi_z,
    const int* __restrict__ after_rho,
    const double* __restrict__ after_phi_x,
    const double* __restrict__ after_phi_y,
    const double* __restrict__ after_phi_z,
    double* __restrict__ d_op_results,  // Pre-allocated size: 10 * N
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;
    int N = L * L * L;

    // --- Spatial Operator Components ---
    double Jx = cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, z, L, 0);
    double Jy = cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, z, L, 1);
    double Jz = cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, z, L, 2);

    // O1: J²
    double op1 = Jx * Jx + Jy * Jy + Jz * Jz;

    // O2: divJ²
    double d = div_face_at(before_phi_x, before_phi_y, before_phi_z, x, y, z, L);
    double op2 = d * d;

    // O3: curlJ²
    double dJz_dy = 0.5 * (cell_J(before_phi_x, before_phi_y, before_phi_z, x, wrap(y + 1, L), z, L, 2) - 
                           cell_J(before_phi_x, before_phi_y, before_phi_z, x, wrap(y - 1, L), z, L, 2));
    double dJy_dz = 0.5 * (cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, wrap(z + 1, L), L, 1) - 
                           cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, wrap(z - 1, L), L, 1));
    double cx = dJz_dy - dJy_dz;

    double dJx_dz = 0.5 * (cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, wrap(z + 1, L), L, 0) - 
                           cell_J(before_phi_x, before_phi_y, before_phi_z, x, y, wrap(z - 1, L), L, 0));
    double dJz_dx = 0.5 * (cell_J(before_phi_x, before_phi_y, before_phi_z, wrap(x + 1, L), y, z, L, 2) - 
                           cell_J(before_phi_x, before_phi_y, before_phi_z, wrap(x - 1, L), y, z, L, 2));
    double cy = dJx_dz - dJz_dx;

    double dJy_dx = 0.5 * (cell_J(before_phi_x, before_phi_y, before_phi_z, wrap(x + 1, L), y, z, L, 1) - 
                           cell_J(before_phi_x, before_phi_y, before_phi_z, wrap(x - 1, L), y, z, L, 1));
    double dJx_dy = 0.5 * (cell_J(before_phi_x, before_phi_y, before_phi_z, x, wrap(y + 1, L), z, L, 0) - 
                           cell_J(before_phi_x, before_phi_y, before_phi_z, x, wrap(y - 1, L), z, L, 0));
    double cz = dJy_dx - dJx_dy;
    double op3 = cx * cx + cy * cy + cz * cz;

    // O4: J · ∇(div J)
    double gx = 0.5 * (div_face_at(before_phi_x, before_phi_y, before_phi_z, wrap(x + 1, L), y, z, L) - 
                       div_face_at(before_phi_x, before_phi_y, before_phi_z, wrap(x - 1, L), y, z, L));
    double gy = 0.5 * (div_face_at(before_phi_x, before_phi_y, before_phi_z, x, wrap(y + 1, L), z, L) - 
                       div_face_at(before_phi_x, before_phi_y, before_phi_z, x, wrap(y - 1, L), z, L));
    double gz = 0.5 * (div_face_at(before_phi_x, before_phi_y, before_phi_z, x, y, wrap(z + 1, L), L) - 
                       div_face_at(before_phi_x, before_phi_y, before_phi_z, x, y, wrap(z - 1, L), L));
    double op4 = Jx * gx + Jy * gy + Jz * gz;

    // O5: J⁴
    double op5 = op1 * op1;

    // O6: stateSq
    double s = static_cast<double>(before_rho[i]);
    double op6 = s * s;

    // --- Reaction Operator Components ---
    int s_before = before_rho[i];
    int s_after = after_rho[i];
    double ds = static_cast<double>(s_after - s_before);

    // O7: reactionDensity = (δs)²
    double op7 = ds * ds;

    // O8: genesisFlux = |δs| * θ(s_before == 0) * |J_before|
    double op8 = 0.0;
    if (s_before == 0 && s_after != 0) {
        op8 = std::abs(ds) * std::sqrt(op1);
    }

    // O9: evapFlux = |s_before| * θ(s_before != 0 ∧ s_after == 0) * |J_before|
    double op9 = 0.0;
    if (s_before != 0 && s_after == 0) {
        op9 = std::abs(static_cast<double>(s_before)) * std::sqrt(op1);
    }

    // O10: JdotDeltaS = J_before · ∇(δs)
    auto get_ds = [&](int xx, int yy, int zz) {
        int idx = idx3d(xx, yy, zz, L);
        return after_rho[idx] - before_rho[idx];
    };
    double dsdx = 0.5 * (get_ds(x + 1, y, z) - get_ds(x - 1, y, z));
    double dsdy = 0.5 * (get_ds(x, y + 1, z) - get_ds(x, y - 1, z));
    double dsdz = 0.5 * (get_ds(x, y, z + 1) - get_ds(x, y, z - 1));
    double op10 = Jx * dsdx + Jy * dsdy + Jz * dsdz;

    // Write components to global memory SoA
    d_op_results[0 * N + i] = op1;
    d_op_results[1 * N + i] = op2;
    d_op_results[2 * N + i] = op3;
    d_op_results[3 * N + i] = op4;
    d_op_results[4 * N + i] = op5;
    d_op_results[5 * N + i] = op6;
    d_op_results[6 * N + i] = op7;
    d_op_results[7 * N + i] = op8;
    d_op_results[8 * N + i] = op9;
    d_op_results[9 * N + i] = op10;
}

// ---------- Parallel Tree Reduction Kernel ----------

__global__ void reduce_blocks_kernel(
    const double* __restrict__ input,
    double* __restrict__ output,
    int N
) {
    extern __shared__ double sdata[];

    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * (blockDim.x * 2) + threadIdx.x;

    double mySum = 0.0;
    if (i < N) mySum += input[i];
    if (i + blockDim.x < N) mySum += input[i + blockDim.x];

    sdata[tid] = mySum;
    __syncthreads();

    // In-block tree reduction
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        output[blockIdx.x] = sdata[0];
    }
}

// ---------- C++ Orchestration Implementations ----------

void gpu_render_bridge_to_dual_cell_fields(const ftd::gpu::GpuBuffers& bufs, GpuDualCellFields& out) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);

    render_bridge_to_dual_cell_fields_gpu_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        out.d_rho_cell,
        out.d_phi_x, out.d_phi_y, out.d_phi_z,
        L
    );
    CUDA_CHECK(cudaGetLastError());
}

void gpu_block_dual_cell_b2(const GpuDualCellFields& fine, GpuDualCellFields& coarse) {
    int fine_L = fine.L;
    int coarse_L = coarse.L;

    dim3 block(4, 8, 8);  // 256 threads
    dim3 grid((coarse_L + 3) / 4, (coarse_L + 7) / 8, (coarse_L + 7) / 8);

    block_dual_cell_b2_gpu_kernel<<<grid, block>>>(
        fine.d_rho_cell, fine.d_phi_x, fine.d_phi_y, fine.d_phi_z,
        coarse.d_rho_cell, coarse.d_phi_x, coarse.d_phi_y, coarse.d_phi_z,
        fine_L, coarse_L
    );
    CUDA_CHECK(cudaGetLastError());
}

void gpu_compute_eft_means(const GpuSnapshotPair& p, double* out_means) {
    int L = p.L();
    int N = p.total_sites();

    // 1. Allocate intermediate GPU results buffer (10 operators × N)
    double* d_op_results = nullptr;
    CUDA_CHECK(cudaMalloc(&d_op_results, 10 * N * sizeof(double)));

    // 2. Launch the operator evaluation kernel
    dim3 block(4, 8, 8);
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);
    compute_eft_operators_gpu_kernel<<<grid, block>>>(
        p.before.d_rho_cell, p.before.d_phi_x, p.before.d_phi_y, p.before.d_phi_z,
        p.after.d_rho_cell, p.after.d_phi_x, p.after.d_phi_y, p.after.d_phi_z,
        d_op_results, L
    );
    CUDA_CHECK(cudaGetLastError());

    // 3. Setup temporary reduction workspace
    // Maximum blocks for 1st pass: (N + 511) / 512
    int max_blocks = (N + 511) / 512;
    double* d_temp = nullptr;
    CUDA_CHECK(cudaMalloc(&d_temp, max_blocks * sizeof(double)));

    double* d_scalar = nullptr;
    CUDA_CHECK(cudaMalloc(&d_scalar, sizeof(double)));

    // 4. Reduce each of the 10 operators independently
    for (int op = 0; op < 10; ++op) {
        double* d_input = d_op_results + op * N;

        // Pass 1: Reduce N to block-sums
        int threads1 = 256;
        int blocks1 = (N + (threads1 * 2) - 1) / (threads1 * 2);
        reduce_blocks_kernel<<<blocks1, threads1, threads1 * sizeof(double)>>>(d_input, d_temp, N);
        CUDA_CHECK(cudaGetLastError());

        // Pass 2: Reduce block-sums to final scalar sum
        int threads2 = 256;
        int blocks2 = 1;
        reduce_blocks_kernel<<<blocks2, threads2, threads2 * sizeof(double)>>>(d_temp, d_scalar, blocks1);
        CUDA_CHECK(cudaGetLastError());

        // Copy final sum to host and divide by N to get the mean
        double sum_val = 0.0;
        CUDA_CHECK(cudaMemcpy(&sum_val, d_scalar, sizeof(double), cudaMemcpyDeviceToHost));
        out_means[op] = sum_val / static_cast<double>(N);
    }

    // 5. Clean up device memory
    cudaFree(d_op_results);
    cudaFree(d_temp);
    cudaFree(d_scalar);
}

} // namespace gpu
} // namespace eft
} // namespace ftd
