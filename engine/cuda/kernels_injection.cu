/**
 * @file kernels_injection.cu
 * @brief Device-resident interactive and scenario injection primitives.
 *
 * A single native click must not download and re-upload the whole lattice.
 * These kernels mutate only the selected site or bounded wavepacket support;
 * the canonical host mirror remains dirty until an explicit host inspection.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"

#include <cuda_runtime.h>
#include <climits>
#include <cmath>

#include "cuda_error.cuh"

namespace ftd {
namespace gpu {
namespace kernels {
namespace {

__global__ void inject_flux_kernel(
    double* fx, double* fy, double* fz,
    double* flx, double* fly, double* flz,
    double* frx, double* fry, double* frz,
    int index, double x, double y, double z,
    bool dual, bool additive) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    if (additive) {
        fx[index] += x; fy[index] += y; fz[index] += z;
        if (dual) {
            flx[index] += 0.5 * x; fly[index] += 0.5 * y; flz[index] += 0.5 * z;
            frx[index] += 0.5 * x; fry[index] += 0.5 * y; frz[index] += 0.5 * z;
        }
    } else {
        fx[index] = x; fy[index] = y; fz[index] = z;
        if (dual) {
            flx[index] = 0.5 * x; fly[index] = 0.5 * y; flz[index] = 0.5 * z;
            frx[index] = 0.5 * x; fry[index] = 0.5 * y; frz[index] = 0.5 * z;
        }
    }
}

__global__ void inject_wave_velocity_kernel(
    double* wx, double* wy, double* wz,
    double* wlx, double* wly, double* wlz,
    double* wrx, double* wry, double* wrz,
    int index, double x, double y, double z, bool dual) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    wx[index] += x; wy[index] += y; wz[index] += z;
    if (dual) {
        wlx[index] += 0.5 * x; wly[index] += 0.5 * y; wlz[index] += 0.5 * z;
        wrx[index] += 0.5 * x; wry[index] += 0.5 * y; wrz[index] += 0.5 * z;
    }
}

__global__ void inject_particle_kernel(
    int8_t* state,
    double* fx, double* fy, double* fz,
    int8_t* spin, int8_t* color, int8_t* flavor,
    int32_t* particle_id, int32_t* pair_id, int32_t* next_particle_id,
    int32_t* identity_error,
    double* flx, double* fly, double* flz,
    double* frx, double* fry, double* frz,
    int index, int8_t state_value,
    double x, double y, double z,
    int8_t spin_value, int8_t color_value, int8_t flavor_value,
    bool dual) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    if (*next_particle_id < 0 || *next_particle_id >= INT_MAX) {
        *identity_error = 1;
        return;
    }
    state[index] = state_value;
    fx[index] = x; fy[index] = y; fz[index] = z;
    spin[index] = spin_value;
    color[index] = color_value;
    flavor[index] = flavor_value;
    particle_id[index] = (*next_particle_id)++;
    pair_id[index] = -1;
    if (dual) {
        const double left = state_value > 0
            ? (1.0 + DELTA_APPROX) * 0.5
            : (1.0 - DELTA_APPROX) * 0.5;
        const double right = 1.0 - left;
        flx[index] = left * x; fly[index] = left * y; flz[index] = left * z;
        frx[index] = right * x; fry[index] = right * y; frz[index] = right * z;
    }
}

__global__ void inject_wavepacket_center_kernel(
    int8_t* state, int32_t* particle_id, int32_t* pair_id,
    int32_t* next_particle_id, int32_t* allocation_base,
    int32_t* identity_error,
    int center_index, int8_t state_value) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    if (*next_particle_id < 0 || *next_particle_id >= INT_MAX) {
        *allocation_base = -1;
        *identity_error = 1;
        return;
    }
    state[center_index] = state_value;
    const int32_t id = (*next_particle_id)++;
    *allocation_base = id;
    particle_id[center_index] = id;
    pair_id[center_index] = -1;
}

__device__ __forceinline__ int wrap_coordinate(int value, int L) {
    value %= L;
    return value < 0 ? value + L : value;
}

__global__ void inject_wavepacket_support_kernel(
    double* fx, double* fy, double* fz,
    double* flx, double* fly, double* flz,
    double* frx, double* fry, double* frz,
    const int32_t* allocation_base,
    int cx, int cy, int cz, int radius, int diameter, int L,
    double sigma, double scale, double left_fraction,
    bool dual) {
    const int q = blockIdx.x * blockDim.x + threadIdx.x;
    if (*allocation_base < 0) return;
    const int count = diameter * diameter * diameter;
    if (q >= count) return;
    const int iz = q % diameter;
    const int iy = (q / diameter) % diameter;
    const int ix = q / (diameter * diameter);
    const int dx = ix - radius;
    const int dy = iy - radius;
    const int dz = iz - radius;
    if (dx == 0 && dy == 0 && dz == 0) return;
    const double r2 = static_cast<double>(dx * dx + dy * dy + dz * dz);
    const double r = sqrt(r2);
    if (r > GAUSSIAN_CUTOFF_SIGMA * sigma) return;

    const int x = wrap_coordinate(cx + dx, L);
    const int y = wrap_coordinate(cy + dy, L);
    const int z = wrap_coordinate(cz + dz, L);
    const int index = x * L * L + y * L + z;
    const double g = exp(-r2 / (2.0 * sigma * sigma));
    const double mag = scale * g;
    const double jx = mag * dx / r;
    const double jy = mag * dy / r;
    const double jz = mag * dz / r;
    atomicAdd(fx + index, jx);
    atomicAdd(fy + index, jy);
    atomicAdd(fz + index, jz);
    if (dual) {
        const double right_fraction = 1.0 - left_fraction;
        atomicAdd(flx + index, left_fraction * jx);
        atomicAdd(fly + index, left_fraction * jy);
        atomicAdd(flz + index, left_fraction * jz);
        atomicAdd(frx + index, right_fraction * jx);
        atomicAdd(fry + index, right_fraction * jy);
        atomicAdd(frz + index, right_fraction * jz);
    }
}

__global__ void inject_entangled_pair_kernel(
    int8_t* state,
    double* fx, double* fy, double* fz,
    double* flx, double* fly, double* flz,
    double* frx, double* fry, double* frz,
    int32_t* particle_id, int32_t* pair_id,
    int32_t* next_particle_id, int32_t* next_pair_id,
    int32_t* identity_error,
    int* partner_out,
    int primary, double jx, double jy, double jz, bool dual, int L) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    const int z = primary % L;
    const int y = (primary / L) % L;
    const int x = primary / (L * L);
    const int xp = x + 1 == L ? 0 : x + 1;
    const int xm = x == 0 ? L - 1 : x - 1;
    const int yp = y + 1 == L ? 0 : y + 1;
    const int ym = y == 0 ? L - 1 : y - 1;
    const int zp = z + 1 == L ? 0 : z + 1;
    const int zm = z == 0 ? L - 1 : z - 1;
    const int candidates[6] = {
        xp * L * L + y * L + z,
        xm * L * L + y * L + z,
        x * L * L + yp * L + z,
        x * L * L + ym * L + z,
        x * L * L + y * L + zp,
        x * L * L + y * L + zm,
    };
    int partner = -1;
    for (int k = 0; k < 6; ++k) {
        if (state[candidates[k]] == 0) { partner = candidates[k]; break; }
    }
    const int32_t particle_count = partner >= 0 ? 2 : 1;
    if (*next_particle_id < 0 || *next_pair_id < 0
        || *next_particle_id > INT_MAX - particle_count
        || *next_pair_id >= INT_MAX) {
        *identity_error = 1;
        return;
    }

    const double major = (1.0 + DELTA_APPROX) * 0.5;
    const double minor = (1.0 - DELTA_APPROX) * 0.5;
    // This one-thread command kernel is issued on the engine's serialized
    // stream.  Allocate in the same order as create_entangled_pair_cpu:
    // pair identity, primary particle, then (only if found) partner particle.
    const int32_t shared_pair_id = (*next_pair_id)++;
    const int32_t primary_pid = (*next_particle_id)++;
    state[primary] = 1;
    fx[primary] = jx; fy[primary] = jy; fz[primary] = jz;
    particle_id[primary] = primary_pid;
    pair_id[primary] = shared_pair_id;
    if (dual) {
        flx[primary] = major * jx; fly[primary] = major * jy; flz[primary] = major * jz;
        frx[primary] = minor * jx; fry[primary] = minor * jy; frz[primary] = minor * jz;
    }
    if (partner < 0) return;
    *partner_out = partner;
    const int32_t partner_pid = (*next_particle_id)++;
    state[partner] = -1;
    fx[partner] = -jx; fy[partner] = -jy; fz[partner] = -jz;
    particle_id[partner] = partner_pid;
    pair_id[partner] = shared_pair_id;
    if (dual) {
        flx[partner] = -minor * jx; fly[partner] = -minor * jy; flz[partner] = -minor * jz;
        frx[partner] = -major * jx; fry[partner] = -major * jy; frz[partner] = -major * jz;
    }
}

}  // namespace

void launch_inject_flux(GpuBuffers& b, int index, const Vec3& value,
                        bool dual, bool additive) {
    inject_flux_kernel<<<1, 1>>>(
        b.d_flux_x, b.d_flux_y, b.d_flux_z,
        b.d_flux_L_x, b.d_flux_L_y, b.d_flux_L_z,
        b.d_flux_R_x, b.d_flux_R_y, b.d_flux_R_z,
        index, value.x, value.y, value.z, dual, additive);
    CUDA_CHECK(cudaGetLastError());
}

void launch_inject_wave_velocity(GpuBuffers& b, int index, const Vec3& value,
                                 bool dual) {
    inject_wave_velocity_kernel<<<1, 1>>>(
        b.d_wave_vel_x, b.d_wave_vel_y, b.d_wave_vel_z,
        b.d_wave_vel_L_x, b.d_wave_vel_L_y, b.d_wave_vel_L_z,
        b.d_wave_vel_R_x, b.d_wave_vel_R_y, b.d_wave_vel_R_z,
        index, value.x, value.y, value.z, dual);
    CUDA_CHECK(cudaGetLastError());
}

void launch_inject_particle(GpuBuffers& b, int index, int8_t state,
                            const Vec3& flux, int8_t spin, int8_t color,
                            int8_t flavor, bool dual) {
    inject_particle_kernel<<<1, 1>>>(
        b.d_state, b.d_flux_x, b.d_flux_y, b.d_flux_z,
        b.d_spin, b.d_color, b.d_flavor, b.d_particle_id, b.d_pair_id,
        b.d_next_particle_id, b.d_identity_error,
        b.d_flux_L_x, b.d_flux_L_y, b.d_flux_L_z,
        b.d_flux_R_x, b.d_flux_R_y, b.d_flux_R_z,
        index, state, flux.x, flux.y, flux.z, spin, color, flavor, dual);
    CUDA_CHECK(cudaGetLastError());
}

void launch_inject_wavepacket(GpuBuffers& b, int cx, int cy, int cz,
                              int8_t state, double sigma, double scale,
                              int radius, bool dual) {
    const auto host_wrap = [&](int value) {
        value %= b.L;
        return value < 0 ? value + b.L : value;
    };
    const int center = host_wrap(cx) * b.L * b.L
                     + host_wrap(cy) * b.L
                     + host_wrap(cz);
    inject_wavepacket_center_kernel<<<1, 1>>>(
        b.d_state, b.d_particle_id, b.d_pair_id,
        b.d_next_particle_id, b.d_identity_allocation_base,
        b.d_identity_error, center, state);
    CUDA_CHECK(cudaGetLastError());
    const int diameter = 2 * radius + 1;
    const int count = diameter * diameter * diameter;
    constexpr int threads = 256;
    const int blocks = (count + threads - 1) / threads;
    const double left = dual
        ? (state > 0 ? (1.0 + DELTA_APPROX) * 0.5
                     : (1.0 - DELTA_APPROX) * 0.5)
        : 0.5;
    inject_wavepacket_support_kernel<<<blocks, threads>>>(
        b.d_flux_x, b.d_flux_y, b.d_flux_z,
        b.d_flux_L_x, b.d_flux_L_y, b.d_flux_L_z,
        b.d_flux_R_x, b.d_flux_R_y, b.d_flux_R_z,
        b.d_identity_allocation_base,
        cx, cy, cz, radius, diameter, b.L, sigma, scale, left, dual);
    CUDA_CHECK(cudaGetLastError());
}

bool launch_inject_entangled_pair(
    GpuBuffers& b, int primary, const Vec3& flux, bool dual) {
    CUDA_CHECK(cudaMemset(b.d_num_particles, 0xFF, sizeof(int)));
    inject_entangled_pair_kernel<<<1, 1>>>(
        b.d_state, b.d_flux_x, b.d_flux_y, b.d_flux_z,
        b.d_flux_L_x, b.d_flux_L_y, b.d_flux_L_z,
        b.d_flux_R_x, b.d_flux_R_y, b.d_flux_R_z,
        b.d_particle_id, b.d_pair_id,
        b.d_next_particle_id, b.d_next_pair_id,
        b.d_identity_error,
        b.d_num_particles, primary,
        flux.x, flux.y, flux.z, dual, b.L);
    CUDA_CHECK(cudaGetLastError());
    int partner = -1;
    CUDA_CHECK(cudaMemcpy(&partner, b.d_num_particles, sizeof(int),
                          cudaMemcpyDeviceToHost));
    return partner >= 0;
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
