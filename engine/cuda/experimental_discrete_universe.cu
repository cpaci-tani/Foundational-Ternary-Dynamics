/**
 * @file experimental_discrete_universe.cu
 * @brief Standalone, hyper-optimized GPU Discrete Universe simulation engine prototype.
 */

#include "ftd/eft/gpu_discrete_universe.h"
#include <cuda_runtime.h>
#include <cuda_fp16.h>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <vector>
#include <iostream>
#include <random>
#include <string>

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
int wrap_mask(int coord, int L) {
    return coord & (L - 1);
}

__device__ __forceinline__
int idx3d(int x, int y, int z, int L) {
    return wrap_mask(x, L) * L * L + wrap_mask(y, L) * L + wrap_mask(z, L);
}

// ---------- Atomic Trit Packing & Unpacking ----------

__device__ __forceinline__
void pack_trit_device(uint8_t* d_states, int idx, int8_t state) {
    int byte_idx = idx / 5;
    int trit_pos = idx % 5;
    
    // Maps {-1, 0, +1} -> {0, 1, 2}
    int val = state + 1;
    
    int power = 1;
    if (trit_pos == 1) power = 3;
    else if (trit_pos == 2) power = 9;
    else if (trit_pos == 3) power = 27;
    else if (trit_pos == 4) power = 81;
    
    // Use CAS loop to handle thread concurrency on byte boundaries safely
    unsigned int* address = (unsigned int*)((size_t)&d_states[byte_idx] & ~3);
    int shift = ((size_t)&d_states[byte_idx] & 3) * 8;
    
    unsigned int old = *address, assumed;
    do {
        assumed = old;
        uint8_t byte_val = (assumed >> shift) & 0xFF;
        int current_trit = (byte_val / power) % 3;
        uint8_t new_byte_val = byte_val - (current_trit * power) + (val * power);
        
        unsigned int new_val = (assumed & ~(0xFF << shift)) | (new_byte_val << shift);
        old = atomicCAS(address, assumed, new_val);
    } while (assumed != old);
}

__device__ __forceinline__
int8_t unpack_trit_device(const uint8_t* d_states, int idx) {
    int byte_idx = idx / 5;
    int trit_pos = idx % 5;
    
    uint8_t byte_val = d_states[byte_idx];
    
    int power = 1;
    if (trit_pos == 1) power = 3;
    else if (trit_pos == 2) power = 9;
    else if (trit_pos == 3) power = 27;
    else if (trit_pos == 4) power = 81;
    
    int val = (byte_val / power) % 3;
    return static_cast<int8_t>(val - 1);
}

// ---------- Initial Trit Injection Kernel ----------

__global__ void inject_trits_kernel(uint8_t* d_states, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    // Setup a few test charges: positive at i % 53 == 0, negative at i % 71 == 0
    int8_t s = 0;
    if (i % 53 == 0) s = 1;
    else if (i % 71 == 0) s = -1;
    
    pack_trit_device(d_states, i, s);
}

// ---------- 26-Connected Shared Memory Wave Kernel ----------

__global__ void discrete_universe_wave_kernel(
    const uint8_t* __restrict__ d_states,
    const half* __restrict__ flux_x,
    const half* __restrict__ flux_y,
    const half* __restrict__ flux_z,
    half* __restrict__ new_flux_x,
    half* __restrict__ new_flux_y,
    half* __restrict__ new_flux_z,
    half* __restrict__ wave_vel_x,
    half* __restrict__ wave_vel_y,
    half* __restrict__ wave_vel_z,
    int L
) {
    // Allocate 8 KB of shared memory per block
    __shared__ float s_flux_x[10][10][10];
    __shared__ float s_flux_y[10][10][10];
    __shared__ float s_flux_z[10][10][10];

    int tx = threadIdx.x;
    int ty = threadIdx.y;
    int tz = threadIdx.z;

    int bx = blockIdx.x * blockDim.x;
    int by = blockIdx.y * blockDim.y;
    int bz = blockIdx.z * blockDim.z;

    // Tiled Cooperative Loading of the 10x10x10 sub-grid (with Halos)
    for (int dz = tz; dz < 10; dz += blockDim.z) {
        for (int dy = ty; dy < 10; dy += blockDim.y) {
            for (int dx = tx; dx < 10; dx += blockDim.x) {
                int gx = bx + dx - 1;
                int gy = by + dy - 1;
                int gz = bz + dz - 1;
                int idx = idx3d(gx, gy, gz, L);
                s_flux_x[dx][dy][dz] = __half2float(flux_x[idx]);
                s_flux_y[dx][dy][dz] = __half2float(flux_y[idx]);
                s_flux_z[dx][dy][dz] = __half2float(flux_z[idx]);
            }
        }
    }
    __syncthreads();

    // Threads in the active 8x8x8 block evaluate the Laplacian
    int x = bx + tx;
    int y = by + ty;
    int z = bz + tz;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;
    int sx = tx + 1;
    int sy = ty + 1;
    int sz = tz + 1;

    float center_x = s_flux_x[sx][sy][sz];
    float center_y = s_flux_y[sx][sy][sz];
    float center_z = s_flux_z[sx][sy][sz];

    // Isotropic 26-neighbor weights
    float lap_x = 0.0f;
    float lap_y = 0.0f;
    float lap_z = 0.0f;

    // 6 Face Neighbors (Weight = 1.0)
    int faces[6][3] = {
        {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}
    };
    for (int f = 0; f < 6; ++f) {
        lap_x += (s_flux_x[sx + faces[f][0]][sy + faces[f][1]][sz + faces[f][2]] - center_x) * 1.0f;
        lap_y += (s_flux_y[sx + faces[f][0]][sy + faces[f][1]][sz + faces[f][2]] - center_y) * 1.0f;
        lap_z += (s_flux_z[sx + faces[f][0]][sy + faces[f][1]][sz + faces[f][2]] - center_z) * 1.0f;
    }

    // 12 Edge Neighbors (Weight = 0.5)
    int edges[12][3] = {
        {1,1,0}, {1,-1,0}, {-1,1,0}, {-1,-1,0},
        {1,0,1}, {1,0,-1}, {-1,0,1}, {-1,0,-1},
        {0,1,1}, {0,1,-1}, {0,-1,1}, {0,-1,-1}
    };
    for (int e = 0; e < 12; ++e) {
        lap_x += (s_flux_x[sx + edges[e][0]][sy + edges[e][1]][sz + edges[e][2]] - center_x) * 0.5f;
        lap_y += (s_flux_y[sx + edges[e][0]][sy + edges[e][1]][sz + edges[e][2]] - center_y) * 0.5f;
        lap_z += (s_flux_z[sx + edges[e][0]][sy + edges[e][1]][sz + edges[e][2]] - center_z) * 0.5f;
    }

    // 8 Corner Neighbors (Weight = 0.25)
    int corners[8][3] = {
        {1,1,1}, {1,1,-1}, {1,-1,1}, {1,-1,-1},
        {-1,1,1}, {-1,1,-1}, {-1,-1,1}, {-1,-1,-1}
    };
    for (int c = 0; c < 8; ++c) {
        lap_x += (s_flux_x[sx + corners[c][0]][sy + corners[c][1]][sz + corners[c][2]] - center_x) * 0.25f;
        lap_y += (s_flux_y[sx + corners[c][0]][sy + corners[c][1]][sz + corners[c][2]] - center_y) * 0.25f;
        lap_z += (s_flux_z[sx + corners[c][0]][sy + corners[c][1]][sz + corners[c][2]] - center_z) * 0.25f;
    }

    // c^2 = 1/3 (wave velocity)
    float c2 = 1.0f / 3.0f;
    float delta_J_x = c2 * lap_x;
    float delta_J_y = c2 * lap_y;
    float delta_J_z = c2 * lap_z;

    // Coupling to active states: g_c * grad(s)
    int8_t s_center = unpack_trit_device(d_states, i);
    if (s_center != 0) {
        float g_c = 0.085f;
        float ds_dx = 0.5f * (unpack_trit_device(d_states, idx3d(x+1,y,z,L)) - unpack_trit_device(d_states, idx3d(x-1,y,z,L)));
        float ds_dy = 0.5f * (unpack_trit_device(d_states, idx3d(x,y+1,z,L)) - unpack_trit_device(d_states, idx3d(x,y-1,z,L)));
        float ds_dz = 0.5f * (unpack_trit_device(d_states, idx3d(x,y,z+1,L)) - unpack_trit_device(d_states, idx3d(x,y,z-1,L)));
        delta_J_x += g_c * ds_dx;
        delta_J_y += g_c * ds_dy;
        delta_J_z += g_c * ds_dz;
    }

    // Step leapfrog wave velocity
    float vx = __half2float(wave_vel_x[i]) + delta_J_x;
    float vy = __half2float(wave_vel_y[i]) + delta_J_y;
    float vz = __half2float(wave_vel_z[i]) + delta_J_z;

    // Apply vacuum damping (1 - alpha)
    float damping = 1.0f - 0.007297f;
    float fx = (center_x + vx) * damping;
    float fy = (center_y + vy) * damping;
    float fz = (center_z + vz) * damping;

    // Write-back back to device arrays
    wave_vel_x[i] = __float2half(vx);
    wave_vel_y[i] = __float2half(vy);
    wave_vel_z[i] = __float2half(vz);

    new_flux_x[i] = __float2half(fx);
    new_flux_y[i] = __float2half(fy);
    new_flux_z[i] = __float2half(fz);
}

// ---------- Red-Black SOR Local Solver Kernel ----------

__global__ void discrete_universe_sor_kernel(
    const uint8_t* __restrict__ d_states,
    const half* __restrict__ flux_x,
    const half* __restrict__ flux_y,
    const half* __restrict__ flux_z,
    half* __restrict__ phi,
    int L, int color
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    // Execute checkerboard filtering to ensure parallel lockless updates
    if (((x + y + z) & 1) != color) return;

    int i = x * L * L + y * L + z;

    int xp = idx3d(x+1,y,z,L), xm = idx3d(x-1,y,z,L);
    int yp = idx3d(x,y+1,z,L), ym = idx3d(x,y-1,z,L);
    int zp = idx3d(x,y,z+1,L), zm = idx3d(x,y,z-1,L);

    float div = 0.5f * (__half2float(flux_x[xp]) - __half2float(flux_x[xm]) +
                        __half2float(flux_y[yp]) - __half2float(flux_y[ym]) +
                        __half2float(flux_z[zp]) - __half2float(flux_z[zm]));
    float s_val = static_cast<float>(unpack_trit_device(d_states, i));
    float rhs = div - s_val;

    float sum_phi = __half2float(phi[xp]) + __half2float(phi[xm]) +
                    __half2float(phi[yp]) + __half2float(phi[ym]) +
                    __half2float(phi[zp]) + __half2float(phi[zm]);
    float omega = 1.75f;
    float current_phi = __half2float(phi[i]);
    float new_phi = (1.0f - omega) * current_phi + (omega / 6.0f) * (sum_phi - rhs);

    phi[i] = __float2half(new_phi);
}

// ---------- GpuDiscreteUniverse Lifecycle ----------

void GpuDiscreteUniverse::allocate(int size) {
    L = size;
    N = size * size * size;
    num_bytes_state = (N + 4) / 5; // Pack 5 trits per byte

    CUDA_CHECK(cudaMalloc(&d_packed_states, num_bytes_state * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_flux_x, N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_flux_y, N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_flux_z, N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_x, N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_y, N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_z, N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_phi, N * sizeof(half)));

    // Zero-out all allocations
    CUDA_CHECK(cudaMemset(d_packed_states, 0, num_bytes_state * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_flux_x, 0, N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_flux_y, 0, N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_flux_z, 0, N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_wave_vel_x, 0, N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_wave_vel_y, 0, N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_wave_vel_z, 0, N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_phi, 0, N * sizeof(half)));
}

void GpuDiscreteUniverse::free() {
    if (d_packed_states) cudaFree(d_packed_states);
    if (d_flux_x) cudaFree(d_flux_x);
    if (d_flux_y) cudaFree(d_flux_y);
    if (d_flux_z) cudaFree(d_flux_z);
    if (d_wave_vel_x) cudaFree(d_wave_vel_x);
    if (d_wave_vel_y) cudaFree(d_wave_vel_y);
    if (d_wave_vel_z) cudaFree(d_wave_vel_z);
    if (d_phi) cudaFree(d_phi);

    d_packed_states = nullptr;
    d_flux_x = nullptr;
    d_flux_y = nullptr;
    d_flux_z = nullptr;
    d_wave_vel_x = nullptr;
    d_wave_vel_y = nullptr;
    d_wave_vel_z = nullptr;
    d_phi = nullptr;
}

// ---------- High-Throughput GVS Benchmark Runner ----------

void run_discrete_universe_benchmark(int lattice_size, int ticks) {
    std::printf("\n============================================================\n");
    std::printf("  GPU Discrete Universe Benchmark — Peak Performance Mode\n");
    std::printf("============================================================\n");
    std::printf("  Lattice Size L = %d  (%d cells)\n", lattice_size, lattice_size*lattice_size*lattice_size);
    std::printf("  Precision: Packed Trits (State) + FP16 Half (Fields)\n");
    std::printf("  DDR5 Bandwidth Footprint: ~12.2 Bytes/Voxel\n");

    GpuDiscreteUniverse uni;
    uni.allocate(lattice_size);

    // 1. Inject test configuration
    int threads_inject = 256;
    int blocks_inject = (uni.N + threads_inject - 1) / threads_inject;
    inject_trits_kernel<<<blocks_inject, threads_inject>>>(uni.d_packed_states, uni.N);
    CUDA_CHECK(cudaDeviceSynchronize());

    // 2. Setup execution double-buffers
    half* d_next_flux_x = nullptr;
    half* d_next_flux_y = nullptr;
    half* d_next_flux_z = nullptr;
    CUDA_CHECK(cudaMalloc(&d_next_flux_x, uni.N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_next_flux_y, uni.N * sizeof(half)));
    CUDA_CHECK(cudaMalloc(&d_next_flux_z, uni.N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_next_flux_x, 0, uni.N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_next_flux_y, 0, uni.N * sizeof(half)));
    CUDA_CHECK(cudaMemset(d_next_flux_z, 0, uni.N * sizeof(half)));

    // 3. Size blocks (8x8x8 CTA tile)
    dim3 block(8, 8, 8);
    dim3 grid((lattice_size + 7) / 8, (lattice_size + 7) / 8, (lattice_size + 7) / 8);

    // Warm-up run to bypass CUDA lazy initialization/context loading overhead
    std::printf("  Warming up device kernels...\n");
    discrete_universe_wave_kernel<<<grid, block>>>(
        uni.d_packed_states,
        uni.d_flux_x, uni.d_flux_y, uni.d_flux_z,
        d_next_flux_x, d_next_flux_y, d_next_flux_z,
        uni.d_wave_vel_x, uni.d_wave_vel_y, uni.d_wave_vel_z,
        lattice_size
    );
    CUDA_CHECK(cudaDeviceSynchronize());

    // 4. Benchmarking clock timing
    std::printf("  Starting performance sweeps over %d ticks...\n", ticks);
    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < ticks; ++t) {
        // Step 1: 26-neighbor shared memory tiled wave kernel
        discrete_universe_wave_kernel<<<grid, block>>>(
            uni.d_packed_states,
            uni.d_flux_x, uni.d_flux_y, uni.d_flux_z,
            d_next_flux_x, d_next_flux_y, d_next_flux_z,
            uni.d_wave_vel_x, uni.d_wave_vel_y, uni.d_wave_vel_z,
            lattice_size
        );

        // Ping-pong double buffers
        std::swap(uni.d_flux_x, d_next_flux_x);
        std::swap(uni.d_flux_y, d_next_flux_y);
        std::swap(uni.d_flux_z, d_next_flux_z);

        // Step 2: Red-Black checkerboard SOR potential updates (1 iteration)
        discrete_universe_sor_kernel<<<grid, block>>>(
            uni.d_packed_states,
            uni.d_flux_x, uni.d_flux_y, uni.d_flux_z,
            uni.d_phi,
            lattice_size,
            0 // Red sites
        );
        discrete_universe_sor_kernel<<<grid, block>>>(
            uni.d_packed_states,
            uni.d_flux_x, uni.d_flux_y, uni.d_flux_z,
            uni.d_phi,
            lattice_size,
            1 // Black sites
        );
    }
    CUDA_CHECK(cudaDeviceSynchronize());

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    // 5. Output GVS metrics
    double seconds = duration.count() / 1000.0;
    double cells_computed = static_cast<double>(uni.N) * static_cast<double>(ticks);
    double gvs = (cells_computed / seconds) / 1e9;

    std::printf("\n  Benchmark Complete:\n");
    std::printf("    Total execution time: %7.3f ms\n", duration.count());
    std::printf("    Average tick latency: %7.3f ms/tick\n", duration.count() / ticks);
    std::printf("    Lattice updates count: %7.2e updates\n", cells_computed);
    std::printf("    RTX 5090 Throughput:   %7.3f GVS (Giga-Voxels per Second)\n", gvs);
    std::printf("============================================================\n\n");

    // Cleanup double-buffers
    cudaFree(d_next_flux_x);
    cudaFree(d_next_flux_y);
    cudaFree(d_next_flux_z);
    uni.free();
}

} // namespace gpu
} // namespace eft
} // namespace ftd

// ---------- Standalone Main Entry Point ----------

int main(int argc, char* argv[]) {
    int L = 128;
    int ticks = 100;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("--L=", 0) == 0) {
            L = std::atoi(arg.substr(4).c_str());
        } else if (arg.rfind("--ticks=", 0) == 0) {
            ticks = std::atoi(arg.substr(8).c_str());
        }
    }

    // Ensure size is power-of-two (for wrapping masks)
    if ((L & (L - 1)) != 0) {
        std::printf("  WARN: Lattice size L=%d is not a power of two. Clamping to 128...\n", L);
        L = 128;
    }

    ftd::eft::gpu::run_discrete_universe_benchmark(L, ticks);
    return 0;
}
