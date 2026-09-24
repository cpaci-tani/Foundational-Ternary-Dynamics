#include "ftd/gpu_engine.h"
#include "cuda_device_buffer.cuh"
#include "cuda_error.cuh"
#include <cuda_runtime.h>
#include <algorithm>

namespace ftd::gpu {
namespace {
constexpr int kThreads = 256;
constexpr int kMaxBlocks = 1024;
struct Accumulator {
    unsigned long long total[2], nonzero[2], invalid;
    double norm[2], maximum[2];
};
__device__ void merge(Accumulator& a, const Accumulator& b) {
    a.invalid += b.invalid;
    for (int p = 0; p < 2; ++p) {
        a.total[p] += b.total[p]; a.nonzero[p] += b.nonzero[p];
        a.norm[p] += b.norm[p]; a.maximum[p] = fmax(a.maximum[p], b.maximum[p]);
    }
}
__global__ void reduce_sites(const double* fx, const double* fy, const double* fz,
                             int n, int l, Accumulator* output) {
    Accumulator value{};
    for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < n;
         i += gridDim.x * blockDim.x) {
        const int p = ((i % l) + ((i / l) % l) + i / (l*l)) & 1;
        ++value.total[p];
        const double x = fx[i], y = fy[i], z = fz[i];
        if (x != 0.0 || y != 0.0 || z != 0.0) ++value.nonzero[p];
        const unsigned invalid = !isfinite(x) + !isfinite(y) + !isfinite(z);
        value.invalid += invalid;
        if (invalid) continue;
        const double norm = x*x + y*y + z*z;
        if (!isfinite(norm)) { ++value.invalid; continue; }
        value.norm[p] += norm;
        value.maximum[p] = fmax(value.maximum[p], fmax(fabs(x), fmax(fabs(y), fabs(z))));
    }
    __shared__ Accumulator partial[kThreads];
    partial[threadIdx.x] = value;
    __syncthreads();
    for (int offset = kThreads / 2; offset > 0; offset /= 2) {
        if (threadIdx.x < offset) merge(partial[threadIdx.x], partial[threadIdx.x + offset]);
        __syncthreads();
    }
    if (threadIdx.x == 0) output[blockIdx.x] = partial[0];
}
__global__ void reduce_blocks(const Accumulator* partial, int count, Accumulator* output) {
    if (threadIdx.x || blockIdx.x) return;
    Accumulator value{};
    for (int i = 0; i < count; ++i) merge(value, partial[i]);
    *output = value;
}
} // namespace

FluxSectors GpuEngine::flux_sectors() const {
    const int blocks = std::min(kMaxBlocks, (N_ + kThreads - 1) / kThreads);
    CudaDeviceBuffer<Accumulator> partial(blocks), result(1);
    // Legacy default stream is ordered against the engine's blocking stream,
    // matching the existing compact diagnostic observers. No canonical write.
    reduce_sites<<<blocks, kThreads>>>(bufs_.d_flux_x, bufs_.d_flux_y, bufs_.d_flux_z,
                                      N_, size_, partial.get());
    CUDA_CHECK(cudaGetLastError());
    reduce_blocks<<<1, 1>>>(partial.get(), blocks, result.get());
    CUDA_CHECK(cudaGetLastError());
    Accumulator host{};
    CUDA_CHECK(cudaMemcpy(&host, result.get(), sizeof(host), cudaMemcpyDeviceToHost));
    FluxSectors out;
    out.lattice_size = size_; out.tick = tick_; out.state_version = state_version_;
    out.nonfinite_value_count = host.invalid;
    for (int p = 0; p < 2; ++p) {
        out.sectors[p] = {host.total[p], host.nonzero[p], host.norm[p], host.maximum[p]};
    }
    validate_flux_sector_sums(out);
    return out;
}
} // namespace ftd::gpu
