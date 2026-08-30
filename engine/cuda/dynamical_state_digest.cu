/**
 * @file dynamical_state_digest.cu
 * @brief Device-resident canonical Scale-0 state reduction.
 */

#include "ftd/gpu_buffers.h"

#include "cuda_error.cuh"

#include <cuda_runtime.h>

#include <cstddef>
#include <cstdint>

namespace ftd::gpu {

std::size_t g_gpu_dynamical_digest_download_bytes = 0;
std::size_t g_gpu_dynamical_digest_download_calls = 0;

namespace {

struct DigestDeviceView {
    const std::int8_t* state;
    const double* flux_x;
    const double* flux_y;
    const double* flux_z;
    const double* wave_vel_x;
    const double* wave_vel_y;
    const double* wave_vel_z;
    const double* flux_L_x;
    const double* flux_L_y;
    const double* flux_L_z;
    const double* flux_R_x;
    const double* flux_R_y;
    const double* flux_R_z;
    const double* wave_vel_L_x;
    const double* wave_vel_L_y;
    const double* wave_vel_L_z;
    const double* wave_vel_R_x;
    const double* wave_vel_R_y;
    const double* wave_vel_R_z;
    const double* velocity_x;
    const double* velocity_y;
    const double* velocity_z;
    const double* remainder_x;
    const double* remainder_y;
    const double* remainder_z;
    const double* latency;
    const std::uint8_t* locked;
    const std::int8_t* spin;
    const std::int8_t* color;
    const std::int8_t* flavor;
    const double* accel_mag;
    const double* flux_strong_x;
    const double* flux_strong_y;
    const double* flux_strong_z;
    const double* wave_vel_strong_x;
    const double* wave_vel_strong_y;
    const double* wave_vel_strong_z;
    const double* flux_weak_x;
    const double* flux_weak_y;
    const double* flux_weak_z;
    const double* wave_vel_weak_x;
    const double* wave_vel_weak_y;
    const double* wave_vel_weak_z;
    const double* phi_coulomb;
    const double* phi_latency;
};

__device__ inline void accumulate_vector(
    DynamicalStateDigestAccumulator& accumulator,
    DynamicalStateField field,
    std::uint64_t index,
    double x,
    double y,
    double z) {
    digest_detail::accumulate_double(accumulator, field, 0, index, x);
    digest_detail::accumulate_double(accumulator, field, 1, index, y);
    digest_detail::accumulate_double(accumulator, field, 2, index, z);
}

__device__ inline DynamicalStateDigestAccumulator digest_site(
    const DigestDeviceView& view,
    int site) {
    DynamicalStateDigestAccumulator accumulator{};
    const auto index = static_cast<std::uint64_t>(site);

    digest_detail::accumulate_integer(
        accumulator, DynamicalStateField::State, index, view.state[site]);
    accumulate_vector(accumulator, DynamicalStateField::Flux, index,
                      view.flux_x[site], view.flux_y[site], view.flux_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::WaveVelocity, index,
                      view.wave_vel_x[site], view.wave_vel_y[site],
                      view.wave_vel_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::FluxLeft, index,
                      view.flux_L_x[site], view.flux_L_y[site],
                      view.flux_L_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::FluxRight, index,
                      view.flux_R_x[site], view.flux_R_y[site],
                      view.flux_R_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::WaveVelocityLeft, index,
                      view.wave_vel_L_x[site], view.wave_vel_L_y[site],
                      view.wave_vel_L_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::WaveVelocityRight, index,
                      view.wave_vel_R_x[site], view.wave_vel_R_y[site],
                      view.wave_vel_R_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::Velocity, index,
                      view.velocity_x[site], view.velocity_y[site],
                      view.velocity_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::Remainder, index,
                      view.remainder_x[site], view.remainder_y[site],
                      view.remainder_z[site]);
    digest_detail::accumulate_double(accumulator, DynamicalStateField::Latency,
                                     0, index, view.latency[site]);
    digest_detail::accumulate_integer(accumulator, DynamicalStateField::Locked,
                                      index, view.locked[site]);
    digest_detail::accumulate_integer(accumulator, DynamicalStateField::Spin,
                                      index, view.spin[site]);
    digest_detail::accumulate_integer(accumulator, DynamicalStateField::Color,
                                      index, view.color[site]);
    digest_detail::accumulate_integer(accumulator, DynamicalStateField::Flavor,
                                      index, view.flavor[site]);
    digest_detail::accumulate_double(accumulator,
        DynamicalStateField::AccelerationMagnitude, 0, index,
        view.accel_mag[site]);
    accumulate_vector(accumulator, DynamicalStateField::StrongFlux, index,
                      view.flux_strong_x[site], view.flux_strong_y[site],
                      view.flux_strong_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::StrongWaveVelocity,
                      index, view.wave_vel_strong_x[site],
                      view.wave_vel_strong_y[site],
                      view.wave_vel_strong_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::WeakFlux, index,
                      view.flux_weak_x[site], view.flux_weak_y[site],
                      view.flux_weak_z[site]);
    accumulate_vector(accumulator, DynamicalStateField::WeakWaveVelocity,
                      index, view.wave_vel_weak_x[site],
                      view.wave_vel_weak_y[site],
                      view.wave_vel_weak_z[site]);

    // RenderBridge's schema-1 dJ_ buffer is persistent but currently has no
    // producer after construction/reset. CUDA therefore represents the same
    // named channel as its defined all-zero record rather than aliasing the
    // live d_delta_j_* read-phase scratch (which is explicitly excluded).
    accumulate_vector(accumulator, DynamicalStateField::ConjugateVelocity,
                      index, 0.0, 0.0, 0.0);
    digest_detail::accumulate_double(accumulator,
        DynamicalStateField::CoulombPotential, 0, index,
        view.phi_coulomb[site]);
    digest_detail::accumulate_double(accumulator,
        DynamicalStateField::LatencyPotential, 0, index,
        view.phi_latency[site]);
    return accumulator;
}

template <int BlockSize>
__global__ void dynamical_state_digest_kernel(
    DigestDeviceView view,
    int site_count,
    DynamicalStateDigestAccumulator* result) {
    DynamicalStateDigestAccumulator local{};
    for (int site = blockIdx.x * BlockSize + threadIdx.x;
         site < site_count;
         site += gridDim.x * BlockSize) {
        local = digest_detail::combine(local, digest_site(view, site));
    }

    __shared__ DynamicalStateDigestAccumulator scratch[BlockSize];
    scratch[threadIdx.x] = local;
    __syncthreads();
    for (int stride = BlockSize / 2; stride > 0; stride >>= 1) {
        if (threadIdx.x < stride) {
            scratch[threadIdx.x] = digest_detail::combine(
                scratch[threadIdx.x], scratch[threadIdx.x + stride]);
        }
        __syncthreads();
    }

    if (threadIdx.x == 0) {
        atomicAdd(reinterpret_cast<unsigned long long*>(&result->hash_lo),
                  static_cast<unsigned long long>(scratch[0].hash_lo));
        atomicAdd(reinterpret_cast<unsigned long long*>(&result->hash_hi),
                  static_cast<unsigned long long>(scratch[0].hash_hi));
        atomicAdd(reinterpret_cast<unsigned long long*>(
                      &result->nonfinite_value_count),
                  static_cast<unsigned long long>(
                      scratch[0].nonfinite_value_count));
        atomicAdd(reinterpret_cast<unsigned long long*>(
                      &result->nondefault_value_count),
                  static_cast<unsigned long long>(
                      scratch[0].nondefault_value_count));
    }
}

DigestDeviceView make_view(const GpuBuffers& buffers) {
    return {
        buffers.d_state,
        buffers.d_flux_x, buffers.d_flux_y, buffers.d_flux_z,
        buffers.d_wave_vel_x, buffers.d_wave_vel_y, buffers.d_wave_vel_z,
        buffers.d_flux_L_x, buffers.d_flux_L_y, buffers.d_flux_L_z,
        buffers.d_flux_R_x, buffers.d_flux_R_y, buffers.d_flux_R_z,
        buffers.d_wave_vel_L_x, buffers.d_wave_vel_L_y,
        buffers.d_wave_vel_L_z,
        buffers.d_wave_vel_R_x, buffers.d_wave_vel_R_y,
        buffers.d_wave_vel_R_z,
        buffers.d_velocity_x, buffers.d_velocity_y, buffers.d_velocity_z,
        buffers.d_remainder_x, buffers.d_remainder_y, buffers.d_remainder_z,
        buffers.d_latency, buffers.d_locked, buffers.d_spin, buffers.d_color,
        buffers.d_flavor, buffers.d_accel_mag,
        buffers.d_flux_strong_x, buffers.d_flux_strong_y,
        buffers.d_flux_strong_z,
        buffers.d_wave_vel_strong_x, buffers.d_wave_vel_strong_y,
        buffers.d_wave_vel_strong_z,
        buffers.d_flux_weak_x, buffers.d_flux_weak_y, buffers.d_flux_weak_z,
        buffers.d_wave_vel_weak_x, buffers.d_wave_vel_weak_y,
        buffers.d_wave_vel_weak_z,
        buffers.d_phi_coulomb, buffers.d_phi_latency,
    };
}

}  // namespace

DynamicalStateDigest GpuBuffers::dynamical_state_digest(
    std::int64_t tick,
    std::uint64_t state_version) const {
    constexpr int block_size = 256;
    int block_count = (N + block_size - 1) / block_size;
    if (block_count > 4096) block_count = 4096;

    CUDA_CHECK(cudaMemsetAsync(d_dynamical_state_digest, 0,
                               sizeof(DynamicalStateDigestAccumulator),
                               stream));
    dynamical_state_digest_kernel<block_size>
        <<<block_count, block_size, 0, stream>>>(
            make_view(*this), N, d_dynamical_state_digest);
    CUDA_CHECK(cudaGetLastError());

    DynamicalStateDigestAccumulator host_accumulator{};
    CUDA_CHECK(cudaMemcpyAsync(&host_accumulator, d_dynamical_state_digest,
                               sizeof(host_accumulator),
                               cudaMemcpyDeviceToHost, stream));
    CUDA_CHECK(cudaStreamSynchronize(stream));

    g_gpu_dynamical_digest_download_bytes += sizeof(host_accumulator);
    ++g_gpu_dynamical_digest_download_calls;
    return digest_detail::finalize(
        host_accumulator, L, static_cast<std::uint64_t>(N), tick,
        state_version, sizeof(host_accumulator));
}

}  // namespace ftd::gpu
