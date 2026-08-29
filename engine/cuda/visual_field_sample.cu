#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include "ftd/visual_field_sample.h"
#include "ftd/visual_sample_grid.h"

#include "cuda_error.cuh"
#include "cuda_device_buffer.cuh"

#include <cuda_runtime.h>
#include <math_constants.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace ftd {
namespace gpu {
namespace {

// Trivially-copyable kernel view. GpuBuffers owns resources and deliberately
// deletes its copy constructor, so it cannot itself be a CUDA kernel argument.
struct VisualDeviceView {
    int N = 0;
    int L = 0;
    const std::int8_t* state = nullptr;
    const double* flux_x = nullptr;
    const double* flux_y = nullptr;
    const double* flux_z = nullptr;
    const double* wave_x = nullptr;
    const double* wave_y = nullptr;
    const double* wave_z = nullptr;
    const double* latency = nullptr;
    const double* remainder_x = nullptr;
    const double* remainder_y = nullptr;
    const double* remainder_z = nullptr;
    const std::int8_t* spin = nullptr;
    const std::int8_t* color = nullptr;
    const double* fd_coulomb_x = nullptr;
    const double* fd_coulomb_y = nullptr;
    const double* fd_coulomb_z = nullptr;
    const double* fd_magnetic_x = nullptr;
    const double* fd_magnetic_y = nullptr;
    const double* fd_magnetic_z = nullptr;
    const double* fd_gravity_x = nullptr;
    const double* fd_gravity_y = nullptr;
    const double* fd_gravity_z = nullptr;
    const double* fd_strong_x = nullptr;
    const double* fd_strong_y = nullptr;
    const double* fd_strong_z = nullptr;
};

VisualDeviceView make_visual_view(const GpuBuffers& b) {
    VisualDeviceView v;
    v.N = b.N; v.L = b.L; v.state = b.d_state;
    v.flux_x = b.d_flux_x; v.flux_y = b.d_flux_y; v.flux_z = b.d_flux_z;
    v.wave_x = b.d_wave_vel_x; v.wave_y = b.d_wave_vel_y; v.wave_z = b.d_wave_vel_z;
    v.latency = b.d_latency;
    v.remainder_x = b.d_remainder_x; v.remainder_y = b.d_remainder_y;
    v.remainder_z = b.d_remainder_z; v.spin = b.d_spin; v.color = b.d_color;
    v.fd_coulomb_x = b.d_fd_coulomb_x; v.fd_coulomb_y = b.d_fd_coulomb_y;
    v.fd_coulomb_z = b.d_fd_coulomb_z;
    v.fd_magnetic_x = b.d_fd_magnetic_x; v.fd_magnetic_y = b.d_fd_magnetic_y;
    v.fd_magnetic_z = b.d_fd_magnetic_z;
    v.fd_gravity_x = b.d_fd_gravity_x; v.fd_gravity_y = b.d_fd_gravity_y;
    v.fd_gravity_z = b.d_fd_gravity_z;
    v.fd_strong_x = b.d_fd_strong_x; v.fd_strong_y = b.d_fd_strong_y;
    v.fd_strong_z = b.d_fd_strong_z;
    return v;
}

__device__ __forceinline__ int wrap_coord(int value, int L) {
    if (value < 0) return value + L;
    if (value >= L) return value - L;
    return value;
}

__device__ __forceinline__ int site_index(int x, int y, int z, int L) {
    return wrap_coord(x, L) * L * L + wrap_coord(y, L) * L + wrap_coord(z, L);
}

__device__ __forceinline__ double rho_at(const VisualDeviceView& b, int i) {
    const double x = b.flux_x[i];
    const double y = b.flux_y[i];
    const double z = b.flux_z[i];
    return x * x + y * y + z * z;
}

__device__ __forceinline__ double density_at(const VisualDeviceView& b, int i) {
    return sqrt(rho_at(b, i));
}

__device__ __forceinline__ void curl_at(
    const VisualDeviceView& b, int x, int y, int z,
    double& cx, double& cy, double& cz) {
    const int L = b.L;
    const int xp = site_index(x + 1, y, z, L);
    const int xm = site_index(x - 1, y, z, L);
    const int yp = site_index(x, y + 1, z, L);
    const int ym = site_index(x, y - 1, z, L);
    const int zp = site_index(x, y, z + 1, L);
    const int zm = site_index(x, y, z - 1, L);
    cx = 0.5 * ((b.flux_z[yp] - b.flux_z[ym])
              - (b.flux_y[zp] - b.flux_y[zm]));
    cy = 0.5 * ((b.flux_x[zp] - b.flux_x[zm])
              - (b.flux_z[xp] - b.flux_z[xm]));
    cz = 0.5 * ((b.flux_y[xp] - b.flux_y[xm])
              - (b.flux_x[yp] - b.flux_x[ym]));
}

__device__ __forceinline__ double divergence_at(
    const VisualDeviceView& b, int x, int y, int z) {
    const int L = b.L;
    return 0.5 * ((b.flux_x[site_index(x + 1, y, z, L)]
                 - b.flux_x[site_index(x - 1, y, z, L)])
                + (b.flux_y[site_index(x, y + 1, z, L)]
                 - b.flux_y[site_index(x, y - 1, z, L)])
                + (b.flux_z[site_index(x, y, z + 1, L)]
                 - b.flux_z[site_index(x, y, z - 1, L)]));
}

__device__ __forceinline__ double latency_proxy_at(
    const VisualDeviceView& b, int idx, double max_rho) {
    return sqrt(fmin(rho_at(b, idx) / max_rho, LATENCY_HORIZON_CLAMP));
}

__device__ __forceinline__ void inactive(float* out, int q, int components) {
    for (int c = 0; c < components; ++c)
        out[q * components + c] = CUDART_NAN_F;
}

__device__ __forceinline__ void write_scalar(float* out, int q, double value) {
    out[q] = static_cast<float>(value);
}

__device__ __forceinline__ void write_vector(
    float* out, int q, double x, double y, double z) {
    out[q * 3 + 0] = static_cast<float>(x);
    out[q * 3 + 1] = static_cast<float>(y);
    out[q * 3 + 2] = static_cast<float>(z);
}

__global__ void reduce_max_rho_kernel(
    VisualDeviceView b, unsigned long long* max_bits) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= b.N) return;
    atomicMax(max_bits, static_cast<unsigned long long>(__double_as_longlong(rho_at(b, i))));
}

__global__ void visual_field_kernel(
    VisualDeviceView b, int kind_value, int start, int stride, int axis_count,
    int components, double max_rho, bool geometric_gravity, float* out) {
    const int q = blockIdx.x * blockDim.x + threadIdx.x;
    const int count = axis_count * axis_count * axis_count;
    if (q >= count) return;

    const int xi = q % axis_count;
    const int yi = (q / axis_count) % axis_count;
    const int zi = q / (axis_count * axis_count);
    const int x = start + xi * stride;
    const int y = start + yi * stride;
    const int z = start + zi * stride;
    const int idx = site_index(x, y, z, b.L);
    const auto kind = static_cast<VisualFieldKind>(kind_value);

    double vx = 0.0, vy = 0.0, vz = 0.0;
    double cx = 0.0, cy = 0.0, cz = 0.0;
    double value = 0.0;

    switch (kind) {
        case VisualFieldKind::Electric:
            vx = -b.wave_x[idx];
            vy = -b.wave_y[idx];
            vz = -b.wave_z[idx];
            if (sqrt(vx * vx + vy * vy + vz * vz) < 1e-15) return inactive(out, q, components);
            return write_vector(out, q, vx, vy, vz);

        case VisualFieldKind::Magnetic:
        case VisualFieldKind::Curl:
            curl_at(b, x, y, z, cx, cy, cz);
            if (sqrt(cx * cx + cy * cy + cz * cz) < 1e-15) return inactive(out, q, components);
            return write_vector(out, q, cx, cy, cz);

        case VisualFieldKind::Poynting: {
            const double ex = -b.wave_x[idx];
            const double ey = -b.wave_y[idx];
            const double ez = -b.wave_z[idx];
            curl_at(b, x, y, z, cx, cy, cz);
            constexpr double c2 = C_SPEED * C_SPEED;
            vx = c2 * (ey * cz - ez * cy);
            vy = c2 * (ez * cx - ex * cz);
            vz = c2 * (ex * cy - ey * cx);
            if (sqrt(vx * vx + vy * vy + vz * vz) < 1e-15) return inactive(out, q, components);
            return write_vector(out, q, vx, vy, vz);
        }

        case VisualFieldKind::Divergence:
            value = divergence_at(b, x, y, z);
            if (fabs(value) < 1e-15) return inactive(out, q, components);
            return write_scalar(out, q, value);

        case VisualFieldKind::FluxVector:
        {
            // Represent this regular output cell by the strongest canonical
            // flux in its source block.  Sampling only the anchor aliases away
            // thin Wilson loops / IC4 point seeds / vortex cores as soon as
            // the large-lattice traffic cap raises stride above one.
            double best_rho = 0.0;
            const int x_end = x + stride;
            const int y_end = y + stride;
            const int z_end = z + stride;
            for (int bz = z; bz < z_end; ++bz) {
                for (int by = y; by < y_end; ++by) {
                    for (int bx = x; bx < x_end; ++bx) {
                        const int block_idx = site_index(bx, by, bz, b.L);
                        const double block_rho = rho_at(b, block_idx);
                        if (block_rho > best_rho) {
                            best_rho = block_rho;
                            vx = b.flux_x[block_idx];
                            vy = b.flux_y[block_idx];
                            vz = b.flux_z[block_idx];
                        }
                    }
                }
            }
            if (best_rho < 1e-30) return inactive(out, q, components);
            return write_vector(out, q, vx, vy, vz);
        }

        case VisualFieldKind::Vorticity:
            curl_at(b, x, y, z, cx, cy, cz);
            value = sqrt(cx * cx + cy * cy + cz * cz);
            if (value < 1e-15) return inactive(out, q, components);
            return write_scalar(out, q, value);

        case VisualFieldKind::Helicity:
            curl_at(b, x, y, z, cx, cy, cz);
            value = b.flux_x[idx] * cx + b.flux_y[idx] * cy + b.flux_z[idx] * cz;
            if (fabs(value) < 1e-15) return inactive(out, q, components);
            return write_scalar(out, q, value);

        case VisualFieldKind::Coherence: {
            curl_at(b, x, y, z, cx, cy, cz);
            const double jm = sqrt(rho_at(b, idx));
            const double cm = sqrt(cx * cx + cy * cy + cz * cz);
            if (jm < 1e-10 || cm < 1e-10) return inactive(out, q, components);
            value = (b.flux_x[idx] * cx + b.flux_y[idx] * cy + b.flux_z[idx] * cz)
                  / (jm * cm);
            return write_scalar(out, q, value);
        }

        case VisualFieldKind::Fisher: {
            const double rho = rho_at(b, idx);
            if (rho < 1e-8) return inactive(out, q, components);
            const double dx = 0.5 * (rho_at(b, site_index(x + 1, y, z, b.L))
                                   - rho_at(b, site_index(x - 1, y, z, b.L)));
            const double dy = 0.5 * (rho_at(b, site_index(x, y + 1, z, b.L))
                                   - rho_at(b, site_index(x, y - 1, z, b.L)));
            const double dz = 0.5 * (rho_at(b, site_index(x, y, z + 1, b.L))
                                   - rho_at(b, site_index(x, y, z - 1, b.L)));
            value = (dx * dx + dy * dy + dz * dz) / rho;
            if (value < 1e-12) return inactive(out, q, components);
            return write_scalar(out, q, value);
        }

        case VisualFieldKind::Latency:
            value = latency_proxy_at(b, idx, max_rho);
            if (value < 1e-6) return inactive(out, q, components);
            return write_scalar(out, q, value);

        case VisualFieldKind::Kretschmann: {
            double face = 0.0;
            face += latency_proxy_at(b, site_index(x + 1, y, z, b.L), max_rho);
            face += latency_proxy_at(b, site_index(x - 1, y, z, b.L), max_rho);
            face += latency_proxy_at(b, site_index(x, y + 1, z, b.L), max_rho);
            face += latency_proxy_at(b, site_index(x, y - 1, z, b.L), max_rho);
            face += latency_proxy_at(b, site_index(x, y, z + 1, b.L), max_rho);
            face += latency_proxy_at(b, site_index(x, y, z - 1, b.L), max_rho);
            double edge = 0.0;
            for (int dx = -1; dx <= 1; ++dx)
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dz = -1; dz <= 1; ++dz)
                        if (abs(dx) + abs(dy) + abs(dz) == 2)
                            edge += latency_proxy_at(
                                b, site_index(x + dx, y + dy, z + dz, b.L), max_rho);
            const double lap = face / 3.0 + edge / 6.0
                             - 4.0 * latency_proxy_at(b, idx, max_rho);
            value = lap * lap;
            if (value < 1e-18) return inactive(out, q, components);
            return write_scalar(out, q, value);
        }

        case VisualFieldKind::State:
            value = static_cast<double>(b.state[idx]);
            if (value == 0.0) return inactive(out, q, components);
            return write_scalar(out, q, value);

        case VisualFieldKind::GaussResidual:
            value = divergence_at(b, x, y, z) - static_cast<double>(b.state[idx]);
            if (fabs(value) < 1e-6) return inactive(out, q, components);
            return write_scalar(out, q, value);

        case VisualFieldKind::EmForce:
            vx = b.fd_coulomb_x[idx] + b.fd_magnetic_x[idx];
            vy = b.fd_coulomb_y[idx] + b.fd_magnetic_y[idx];
            vz = b.fd_coulomb_z[idx] + b.fd_magnetic_z[idx];
            if (sqrt(vx * vx + vy * vy + vz * vz) < 1e-15) return inactive(out, q, components);
            return write_vector(out, q, vx, vy, vz);

        case VisualFieldKind::GravityForce: {
            const int xp = site_index(x + 2, y, z, b.L);
            const int xm = site_index(x - 2, y, z, b.L);
            const int yp = site_index(x, y + 2, z, b.L);
            const int ym = site_index(x, y - 2, z, b.L);
            const int zp = site_index(x, y, z + 2, b.L);
            const int zm = site_index(x, y, z - 2, b.L);
            if (geometric_gravity) {
                const double pre = M_INERTIAL * C_SPEED * C_SPEED * b.latency[idx];
                vx = pre * GRAD_TIER2_SCALE * (b.latency[xp] - b.latency[xm]);
                vy = pre * GRAD_TIER2_SCALE * (b.latency[yp] - b.latency[ym]);
                vz = pre * GRAD_TIER2_SCALE * (b.latency[zp] - b.latency[zm]);
            } else {
                vx = G_N * GRAD_TIER2_SCALE * (density_at(b, xp) - density_at(b, xm));
                vy = G_N * GRAD_TIER2_SCALE * (density_at(b, yp) - density_at(b, ym));
                vz = G_N * GRAD_TIER2_SCALE * (density_at(b, zp) - density_at(b, zm));
            }
            if (sqrt(vx * vx + vy * vy + vz * vz) < 1e-15) return inactive(out, q, components);
            return write_vector(out, q, vx, vy, vz);
        }

        case VisualFieldKind::StrongForce:
            vx = b.fd_strong_x[idx]; vy = b.fd_strong_y[idx]; vz = b.fd_strong_z[idx];
            if (sqrt(vx * vx + vy * vy + vz * vz) < 1e-15) return inactive(out, q, components);
            return write_vector(out, q, vx, vy, vz);

        case VisualFieldKind::PoissonLatency:
            value = b.latency[idx];
            if (value < 1e-15) return inactive(out, q, components);
            return write_scalar(out, q, value);
    }
    inactive(out, q, components);
}

__global__ void visual_particle_attributes_kernel(
    VisualDeviceView b, const int* indices, int count, float* out) {
    const int q = blockIdx.x * blockDim.x + threadIdx.x;
    if (q >= count) return;
    const int idx = indices[q];
    out[q * 5 + 0] = static_cast<float>(b.remainder_x[idx]);
    out[q * 5 + 1] = static_cast<float>(b.remainder_y[idx]);
    out[q * 5 + 2] = static_cast<float>(b.remainder_z[idx]);
    out[q * 5 + 3] = static_cast<float>(b.spin[idx]);
    out[q * 5 + 4] = static_cast<float>(b.color[idx]);
}

}  // namespace

void GpuEngine::copy_visual_field_sample(VisualFieldKind kind, int requested_stride,
                                         VisualFieldSample& out) const {
    out = {};
    out.components = is_vector_field_kind(kind) ? 3u : 1u;

    // Center-anchored sample grid, shared with the CPU + WASM samplers so all
    // three agree on which voxels are sampled (see visual_sample_grid.h).
    const VisualSampleGrid grid =
        visual_sample_grid(size_, requested_stride, is_interior_field_kind(kind));
    out.effective_stride = grid.stride;
    out.origin = grid.origin;
    if (grid.count == 0) return;  // lattice too small (e.g. interior kind on size_ < 3)

    const int start = grid.origin;
    const int axis_count = grid.count;
    const int candidate_count = axis_count * axis_count * axis_count;
    const VisualDeviceView view = make_visual_view(bufs_);

    double max_rho = 1.0;
    if (kind == VisualFieldKind::Latency || kind == VisualFieldKind::Kretschmann) {
        CudaDeviceBuffer<unsigned long long> d_max(1);
        CUDA_CHECK(cudaMemset(d_max.get(), 0, sizeof(unsigned long long)));
        constexpr int block = 256;
        reduce_max_rho_kernel<<<(N_ + block - 1) / block, block>>>(view, d_max.get());
        CUDA_CHECK(cudaGetLastError());
        std::uint64_t bits = 0;
        CUDA_CHECK(cudaMemcpy(&bits, d_max.get(), sizeof(bits), cudaMemcpyDeviceToHost));
        std::memcpy(&max_rho, &bits, sizeof(max_rho));
        if (max_rho < 1e-30) return;
    }

    const std::size_t raw_count = static_cast<std::size_t>(candidate_count)
                                * out.components;
    CudaDeviceBuffer<float> d_values(raw_count);
    constexpr int block = 256;
    visual_field_kernel<<<(candidate_count + block - 1) / block, block>>>(
        view, static_cast<int>(kind), start, out.effective_stride, axis_count,
        static_cast<int>(out.components), max_rho, toggles.geometric_gravity,
        d_values.get());
    CUDA_CHECK(cudaGetLastError());

    std::vector<float> raw(raw_count);
    CUDA_CHECK(cudaMemcpy(raw.data(), d_values.get(), raw_count * sizeof(float),
                          cudaMemcpyDeviceToHost));

    out.positions.reserve(static_cast<std::size_t>(candidate_count) * 3u);
    out.data.reserve(raw_count);
    for (int q = 0; q < candidate_count; ++q) {
        if (!std::isfinite(raw[static_cast<std::size_t>(q) * out.components])) continue;
        const int xi = q % axis_count;
        const int yi = (q / axis_count) % axis_count;
        const int zi = q / (axis_count * axis_count);
        out.positions.push_back(static_cast<float>(start + xi * out.effective_stride) + 0.5f);
        out.positions.push_back(static_cast<float>(start + yi * out.effective_stride) + 0.5f);
        out.positions.push_back(static_cast<float>(start + zi * out.effective_stride) + 0.5f);
        for (std::uint32_t c = 0; c < out.components; ++c)
            out.data.push_back(raw[static_cast<std::size_t>(q) * out.components + c]);
    }
}

void GpuEngine::copy_visual_particle_attributes(
    const std::vector<int>& indices, std::vector<float>& out) const {
    out.clear();
    if (indices.empty()) return;
    CudaDeviceBuffer<int> d_indices(indices.size());
    CudaDeviceBuffer<float> d_values(indices.size() * 5u);
    const VisualDeviceView view = make_visual_view(bufs_);
    CUDA_CHECK(cudaMemcpy(d_indices.get(), indices.data(), indices.size() * sizeof(int),
                          cudaMemcpyHostToDevice));
    constexpr int block = 256;
    visual_particle_attributes_kernel<<<
        (static_cast<int>(indices.size()) + block - 1) / block, block>>>(
            view, d_indices.get(), static_cast<int>(indices.size()), d_values.get());
    CUDA_CHECK(cudaGetLastError());
    out.resize(indices.size() * 5u);
    CUDA_CHECK(cudaMemcpy(out.data(), d_values.get(), out.size() * sizeof(float),
                          cudaMemcpyDeviceToHost));
}

}  // namespace gpu
}  // namespace ftd
