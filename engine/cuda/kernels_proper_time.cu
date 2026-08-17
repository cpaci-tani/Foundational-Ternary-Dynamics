/**
 * @file kernels_proper_time.cu
 * @brief Device-resident proper-time accumulation for latency scenarios.
 *
 * CPU contract: for every manifested site, once at the end of each tick,
 *   tau += proper_time_rate(latency, |velocity|^2).
 * The shared host/device function is used verbatim.  Both tau and the optional
 * de Broglie phase live on-device, so interactive clock scenarios do not need
 * a full SoA -> AoS download/upload round trip on every tick.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/proper_time_rate.h"

#include <cuda_runtime.h>

#include "cuda_error.cuh"

namespace ftd {
namespace gpu {
namespace kernels {

__global__ void accumulate_proper_time_kernel(
    const int8_t* __restrict__ state,
    const double* __restrict__ latency,
    const double* __restrict__ velocity_x,
    const double* __restrict__ velocity_y,
    const double* __restrict__ velocity_z,
    double* __restrict__ tau,
    double* __restrict__ phase,
    bool update_phase,
    double omega0,
    int count) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= count || state[i] == 0) return;
    const double vx = velocity_x[i];
    const double vy = velocity_y[i];
    const double vz = velocity_z[i];
    const double delta_tau = ::ftd::proper_time_rate(
        latency[i], vx * vx + vy * vy + vz * vz);
    if (delta_tau > 0.0) {
        tau[i] += delta_tau;
        if (update_phase) phase[i] += omega0 * delta_tau;
    }
}

void launch_accumulate_proper_time(GpuBuffers& b, bool update_phase,
                                   double omega0) {
    constexpr int threads = 256;
    const int blocks = (b.N + threads - 1) / threads;
    accumulate_proper_time_kernel<<<blocks, threads>>>(
        b.d_state, b.d_latency,
        b.d_velocity_x, b.d_velocity_y, b.d_velocity_z,
        b.d_tau, b.d_phase, update_phase, omega0, b.N);
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
