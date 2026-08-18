/**
 * @file diagnostic_reductions.cu
 * @brief Fixed-size CUDA reductions for native interactive diagnostics.
 *
 * The native WebSocket UI polls diagnostics while the canonical voxel state
 * remains device-resident.  Walking RenderBridge::voxels() here used to copy
 * 333 bytes/site for the voxel mirror, followed by potential and force arrays;
 * at L=256 a single poll could therefore move several GiB.  These kernels
 * reduce each public snapshot to at most 25 doubles on device and download a
 * lattice-size-independent scalar payload.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/telemetry_snapshot.h"
#include "ftd/lagrangian.h"  // LagrangianDiag POD
#include "ftd/term_toggles.h"
#include "ftd/causal_kinematics.h"
#include "ftd/constants.h"
#include "ftd/volumetric_measure.h"

#include <cuda_runtime.h>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <vector>

#include "cuda_error.cuh"

namespace ftd {
namespace gpu {

std::size_t g_gpu_compact_diagnostic_download_bytes = 0;
std::size_t g_gpu_telemetry_snapshot_download_bytes = 0;
std::size_t g_gpu_telemetry_snapshot_launches = 0;

namespace kernels {
namespace {

constexpr int THREADS = 256;
constexpr int MAX_REDUCTION_BLOCKS = 1024;

// Kernel argument view.  GpuBuffers itself is an owning, non-copyable RAII
// type and must never be passed by value through a launch configuration.
struct DiagnosticView {
    int N = 0, L = 0;
    int8_t *d_state = nullptr, *d_spin = nullptr, *d_color = nullptr;
    uint8_t* d_locked = nullptr;
    int32_t *d_particle_id = nullptr, *d_pair_id = nullptr;
    double *d_flux_x = nullptr, *d_flux_y = nullptr, *d_flux_z = nullptr;
    double *d_wave_vel_x = nullptr, *d_wave_vel_y = nullptr, *d_wave_vel_z = nullptr;
    double *d_velocity_x = nullptr, *d_velocity_y = nullptr, *d_velocity_z = nullptr;
    double* d_latency = nullptr;
    double *d_tau = nullptr, *d_phase = nullptr, *d_accel_mag = nullptr;
    double* d_phi_coulomb = nullptr;
    double *d_flux_L_x = nullptr, *d_flux_L_y = nullptr, *d_flux_L_z = nullptr;
    double *d_flux_R_x = nullptr, *d_flux_R_y = nullptr, *d_flux_R_z = nullptr;
    double *d_wave_vel_L_x = nullptr, *d_wave_vel_L_y = nullptr, *d_wave_vel_L_z = nullptr;
    double *d_wave_vel_R_x = nullptr, *d_wave_vel_R_y = nullptr, *d_wave_vel_R_z = nullptr;
    double *d_flux_strong_x = nullptr, *d_flux_strong_y = nullptr, *d_flux_strong_z = nullptr;
    double *d_flux_weak_x = nullptr, *d_flux_weak_y = nullptr, *d_flux_weak_z = nullptr;
    unsigned long long* d_causal_projection_events = nullptr;
    double *d_fd_coulomb_x = nullptr, *d_fd_coulomb_y = nullptr, *d_fd_coulomb_z = nullptr;
    double *d_fd_strong_x = nullptr, *d_fd_strong_y = nullptr, *d_fd_strong_z = nullptr;
    double *d_fd_magnetic_x = nullptr, *d_fd_magnetic_y = nullptr, *d_fd_magnetic_z = nullptr;
    double *d_fd_gravity_x = nullptr, *d_fd_gravity_y = nullptr, *d_fd_gravity_z = nullptr;
    double *d_fd_exchange_x = nullptr, *d_fd_exchange_y = nullptr, *d_fd_exchange_z = nullptr;
};

DiagnosticView diagnostic_view(GpuBuffers& b) {
    DiagnosticView v;
    v.N = b.N; v.L = b.L;
    v.d_state = b.d_state; v.d_spin = b.d_spin; v.d_color = b.d_color;
    v.d_locked = b.d_locked;
    v.d_particle_id = b.d_particle_id; v.d_pair_id = b.d_pair_id;
    v.d_flux_x = b.d_flux_x; v.d_flux_y = b.d_flux_y; v.d_flux_z = b.d_flux_z;
    v.d_wave_vel_x = b.d_wave_vel_x; v.d_wave_vel_y = b.d_wave_vel_y; v.d_wave_vel_z = b.d_wave_vel_z;
    v.d_velocity_x = b.d_velocity_x; v.d_velocity_y = b.d_velocity_y; v.d_velocity_z = b.d_velocity_z;
    v.d_latency = b.d_latency; v.d_tau = b.d_tau; v.d_phase = b.d_phase;
    v.d_accel_mag = b.d_accel_mag;
    v.d_phi_coulomb = b.d_phi_coulomb;
    v.d_flux_L_x = b.d_flux_L_x; v.d_flux_L_y = b.d_flux_L_y; v.d_flux_L_z = b.d_flux_L_z;
    v.d_flux_R_x = b.d_flux_R_x; v.d_flux_R_y = b.d_flux_R_y; v.d_flux_R_z = b.d_flux_R_z;
    v.d_wave_vel_L_x = b.d_wave_vel_L_x; v.d_wave_vel_L_y = b.d_wave_vel_L_y; v.d_wave_vel_L_z = b.d_wave_vel_L_z;
    v.d_wave_vel_R_x = b.d_wave_vel_R_x; v.d_wave_vel_R_y = b.d_wave_vel_R_y; v.d_wave_vel_R_z = b.d_wave_vel_R_z;
    v.d_flux_strong_x = b.d_flux_strong_x; v.d_flux_strong_y = b.d_flux_strong_y; v.d_flux_strong_z = b.d_flux_strong_z;
    v.d_flux_weak_x = b.d_flux_weak_x; v.d_flux_weak_y = b.d_flux_weak_y; v.d_flux_weak_z = b.d_flux_weak_z;
    v.d_causal_projection_events = b.d_causal_projection_events;
    v.d_fd_coulomb_x = b.d_fd_coulomb_x; v.d_fd_coulomb_y = b.d_fd_coulomb_y; v.d_fd_coulomb_z = b.d_fd_coulomb_z;
    v.d_fd_strong_x = b.d_fd_strong_x; v.d_fd_strong_y = b.d_fd_strong_y; v.d_fd_strong_z = b.d_fd_strong_z;
    v.d_fd_magnetic_x = b.d_fd_magnetic_x; v.d_fd_magnetic_y = b.d_fd_magnetic_y; v.d_fd_magnetic_z = b.d_fd_magnetic_z;
    v.d_fd_gravity_x = b.d_fd_gravity_x; v.d_fd_gravity_y = b.d_fd_gravity_y; v.d_fd_gravity_z = b.d_fd_gravity_z;
    v.d_fd_exchange_x = b.d_fd_exchange_x; v.d_fd_exchange_y = b.d_fd_exchange_y; v.d_fd_exchange_z = b.d_fd_exchange_z;
    return v;
}

__device__ __forceinline__ double warp_sum(double value) {
    constexpr unsigned mask = 0xffffffffu;
    for (int offset = 16; offset > 0; offset >>= 1)
        value += __shfl_down_sync(mask, value, offset);
    return value;
}

__device__ __forceinline__ double warp_max(double value) {
    constexpr unsigned mask = 0xffffffffu;
    for (int offset = 16; offset > 0; offset >>= 1)
        value = fmax(value, __shfl_down_sync(mask, value, offset));
    return value;
}

__device__ __forceinline__ void reduce_sum_to(double* out, int slot,
                                               double value) {
    value = warp_sum(value);
    if ((threadIdx.x & 31) == 0) atomicAdd(out + slot, value);
}

__device__ __forceinline__ void atomic_max_nonnegative(double* address,
                                                        double value) {
    auto* bits = reinterpret_cast<unsigned long long*>(address);
    unsigned long long old = *bits;
    while (__longlong_as_double(static_cast<long long>(old)) < value) {
        const unsigned long long assumed = old;
        old = atomicCAS(bits, assumed,
                        static_cast<unsigned long long>(__double_as_longlong(value)));
        if (old == assumed) break;
    }
}

__device__ __forceinline__ void reduce_max_to(double* out, int slot,
                                               double value) {
    value = warp_max(value);
    if ((threadIdx.x & 31) == 0) atomic_max_nonnegative(out + slot, value);
}

__device__ __forceinline__ int wrap_coord(int value, int L) {
    value %= L;
    return value < 0 ? value + L : value;
}

__device__ __forceinline__ int index_3d(int x, int y, int z, int L) {
    return wrap_coord(x, L) * L * L + wrap_coord(y, L) * L + wrap_coord(z, L);
}

__device__ __forceinline__ void coordinates(int index, int L,
                                             int& x, int& y, int& z) {
    const int L2 = L * L;
    x = index / L2;
    const int rem = index - x * L2;
    y = rem / L;
    z = rem - y * L;
}

__device__ __forceinline__ void divergence_and_curl(
    const DiagnosticView b, int x, int y, int z,
    double& div, double& curl_x, double& curl_y, double& curl_z) {
    const int xp = index_3d(x + 1, y, z, b.L);
    const int xm = index_3d(x - 1, y, z, b.L);
    const int yp = index_3d(x, y + 1, z, b.L);
    const int ym = index_3d(x, y - 1, z, b.L);
    const int zp = index_3d(x, y, z + 1, b.L);
    const int zm = index_3d(x, y, z - 1, b.L);

    div = 0.5 * ((b.d_flux_x[xp] - b.d_flux_x[xm])
               + (b.d_flux_y[yp] - b.d_flux_y[ym])
               + (b.d_flux_z[zp] - b.d_flux_z[zm]));
    curl_x = 0.5 * ((b.d_flux_z[yp] - b.d_flux_z[ym])
                  - (b.d_flux_y[zp] - b.d_flux_y[zm]));
    curl_y = 0.5 * ((b.d_flux_x[zp] - b.d_flux_x[zm])
                  - (b.d_flux_z[xp] - b.d_flux_z[xm]));
    curl_z = 0.5 * ((b.d_flux_y[xp] - b.d_flux_y[xm])
                  - (b.d_flux_x[yp] - b.d_flux_x[ym]));
}

enum DiagnosticSlot : int {
    D_TOTAL_FLUX, D_BI_ABS, D_MAX_BANDWIDTH, D_MAX_BUDGET,
    D_MANIFESTED, D_POSITIVE, D_NEGATIVE, D_SPIN_UP, D_SPIN_DOWN,
    D_COLOR_0, D_COLOR_1, D_COLOR_2, D_COLOR_3,
    D_RHO2_SUM, D_RHO2_LOG_SUM,
    D_COORD_X, D_COORD_Y, D_COORD_Z,
    D_VEL_X, D_VEL_Y, D_VEL_Z,
    D_RXV_X, D_RXV_Y, D_RXV_Z,
    D_CAUSAL_PROJECTIONS,
    D_COUNT
};

__global__ void compact_diagnostics_kernel(DiagnosticView b, bool movement,
                                            double* out) {
    double total_flux = 0.0, bi_abs = 0.0;
    double max_bandwidth = 0.0, max_budget = 0.0;
    double manifested = 0.0, positive = 0.0, negative = 0.0;
    double spin_up = 0.0, spin_down = 0.0;
    double color0 = 0.0, color1 = 0.0, color2 = 0.0, color3 = 0.0;
    double rho2_sum = 0.0, rho2_log_sum = 0.0;
    double coord_x = 0.0, coord_y = 0.0, coord_z = 0.0;
    double vel_x = 0.0, vel_y = 0.0, vel_z = 0.0;
    double rxv_x = 0.0, rxv_y = 0.0, rxv_z = 0.0;

    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        const double fx = b.d_flux_x[i], fy = b.d_flux_y[i], fz = b.d_flux_z[i];
        const double vx = b.d_velocity_x[i], vy = b.d_velocity_y[i], vz = b.d_velocity_z[i];
        const double flux2 = fx * fx + fy * fy + fz * fz;
        const double speed2 = vx * vx + vy * vy + vz * vz;
        const double latency = b.d_latency[i];
        total_flux += sqrt(flux2);
        bi_abs += fabs(born_infeld_core(latency, speed2));
        max_bandwidth = fmax(max_bandwidth, bandwidth_fraction(latency, speed2));
        max_budget = fmax(max_budget, causal_budget(latency, speed2));
        rho2_sum += flux2;
        if (flux2 > EPSILON_FLUX_SQ) rho2_log_sum += flux2 * log(flux2);

        const int state = static_cast<int>(b.d_state[i]);
        if (state == 0) continue;
        manifested += 1.0;
        positive += state > 0 ? 1.0 : 0.0;
        negative += state < 0 ? 1.0 : 0.0;
        const int spin = static_cast<int>(b.d_spin[i]);
        spin_up += spin > 0 ? 1.0 : 0.0;
        spin_down += spin < 0 ? 1.0 : 0.0;
        const int color = static_cast<int>(b.d_color[i]);
        color0 += color == 0 ? 1.0 : 0.0;
        color1 += color == 1 ? 1.0 : 0.0;
        color2 += color == 2 ? 1.0 : 0.0;
        color3 += color == 3 ? 1.0 : 0.0;

        int x, y, z;
        coordinates(i, b.L, x, y, z);
        coord_x += x; coord_y += y; coord_z += z;
        vel_x += vx; vel_y += vy; vel_z += vz;
        rxv_x += static_cast<double>(y) * vz - static_cast<double>(z) * vy;
        rxv_y += static_cast<double>(z) * vx - static_cast<double>(x) * vz;
        rxv_z += static_cast<double>(x) * vy - static_cast<double>(y) * vx;
    }

    reduce_sum_to(out, D_TOTAL_FLUX, total_flux);
    reduce_sum_to(out, D_BI_ABS, bi_abs);
    reduce_max_to(out, D_MAX_BANDWIDTH, max_bandwidth);
    reduce_max_to(out, D_MAX_BUDGET, max_budget);
    reduce_sum_to(out, D_MANIFESTED, manifested);
    reduce_sum_to(out, D_POSITIVE, positive);
    reduce_sum_to(out, D_NEGATIVE, negative);
    reduce_sum_to(out, D_SPIN_UP, spin_up);
    reduce_sum_to(out, D_SPIN_DOWN, spin_down);
    reduce_sum_to(out, D_COLOR_0, color0);
    reduce_sum_to(out, D_COLOR_1, color1);
    reduce_sum_to(out, D_COLOR_2, color2);
    reduce_sum_to(out, D_COLOR_3, color3);
    reduce_sum_to(out, D_RHO2_SUM, rho2_sum);
    reduce_sum_to(out, D_RHO2_LOG_SUM, rho2_log_sum);
    reduce_sum_to(out, D_COORD_X, coord_x);
    reduce_sum_to(out, D_COORD_Y, coord_y);
    reduce_sum_to(out, D_COORD_Z, coord_z);
    reduce_sum_to(out, D_VEL_X, vel_x);
    reduce_sum_to(out, D_VEL_Y, vel_y);
    reduce_sum_to(out, D_VEL_Z, vel_z);
    reduce_sum_to(out, D_RXV_X, rxv_x);
    reduce_sum_to(out, D_RXV_Y, rxv_y);
    reduce_sum_to(out, D_RXV_Z, rxv_z);
    const double causal = (movement && begin == 0)
        ? static_cast<double>(*b.d_causal_projection_events) : 0.0;
    reduce_sum_to(out, D_CAUSAL_PROJECTIONS, causal);
}

__global__ void charge_sum_kernel(const int8_t* state, int N,
                                  long long* charge_sum) {
    long long local = 0;
    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < N; i += stride)
        local += static_cast<long long>(state[i]);
    constexpr unsigned mask = 0xffffffffu;
    for (int offset = 16; offset > 0; offset >>= 1)
        local += __shfl_down_sync(mask, local, offset);
    if ((threadIdx.x & 31) == 0) {
        // Signed addition via modulo-2^64 arithmetic is exact for |Q|<=N.
        atomicAdd(reinterpret_cast<unsigned long long*>(charge_sum),
                  static_cast<unsigned long long>(local));
    }
}

enum EnergySlot : int {
    E_FIELD, E_WAVE, E_PARTICLE_KE, E_PARTICLE_REST,
    E_MOMENTUM_X, E_MOMENTUM_Y, E_MOMENTUM_Z,
    E_MANIFESTED, E_CHARGE,
    E_LEFT, E_RIGHT, E_WAVE_LEFT, E_WAVE_RIGHT, E_CHIRALITY,
    E_STRONG, E_WEAK, E_ELECTRIC, E_MAGNETIC,
    E_POYNTING_X, E_POYNTING_Y, E_POYNTING_Z,
    E_GAUSS_SUM, E_GAUSS_MAX, E_COULOMB_PE,
    E_COUNT
};

__global__ void compact_energy_kernel(DiagnosticView b,
                                      bool dual_substrate,
                                      bool strong_field,
                                      double charge_coupling,
                                      const long long* charge_sum,
                                      double* out) {
    double field = 0.0, wave = 0.0, particle_ke = 0.0, particle_rest = 0.0;
    double momentum_x = 0.0, momentum_y = 0.0, momentum_z = 0.0;
    double manifested = 0.0, charge = 0.0;
    double left = 0.0, right = 0.0, wave_left = 0.0, wave_right = 0.0;
    double chirality = 0.0, strong = 0.0, weak = 0.0;
    double electric = 0.0, magnetic = 0.0;
    double poynting_x = 0.0, poynting_y = 0.0, poynting_z = 0.0;
    double gauss_sum = 0.0, gauss_max = 0.0, coulomb_pe = 0.0;
    const double mean_charge = static_cast<double>(*charge_sum)
                             / static_cast<double>(b.N);
    constexpr double C2 = C_SPEED * C_SPEED;

    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        const double fx = b.d_flux_x[i], fy = b.d_flux_y[i], fz = b.d_flux_z[i];
        const double wx = b.d_wave_vel_x[i], wy = b.d_wave_vel_y[i], wz = b.d_wave_vel_z[i];
        const double flux2 = fx * fx + fy * fy + fz * fz;
        const double wave2 = wx * wx + wy * wy + wz * wz;
        field += quadratic_field_energy_density(flux2);
        wave += quadratic_field_energy_density(wave2);
        electric += quadratic_field_energy_density(wave2);

        int x, y, z;
        coordinates(i, b.L, x, y, z);
        double div, bx, by, bz;
        divergence_and_curl(b, x, y, z, div, bx, by, bz);
        magnetic += C2 * quadratic_field_energy_density(bx * bx + by * by + bz * bz);
        // E = -wave_vel; S = c^2 E x B.
        poynting_x += C2 * ((-wy) * bz - (-wz) * by);
        poynting_y += C2 * ((-wz) * bx - (-wx) * bz);
        poynting_z += C2 * ((-wx) * by - (-wy) * bx);

        const int state = static_cast<int>(b.d_state[i]);
        if (state == 0) {
            const double err = div + charge_coupling * mean_charge;
            gauss_sum += err * err;
            gauss_max = fmax(gauss_max, fabs(err));
        } else {
            const double vx = b.d_velocity_x[i], vy = b.d_velocity_y[i], vz = b.d_velocity_z[i];
            const double speed2 = vx * vx + vy * vy + vz * vz;
            const double gamma0 = flat_gamma(speed2);
            particle_ke += flat_particle_kinetic_energy(speed2);
            particle_rest += E_REST;
            momentum_x += vx * (gamma0 * M_INERTIAL);
            momentum_y += vy * (gamma0 * M_INERTIAL);
            momentum_z += vz * (gamma0 * M_INERTIAL);
            manifested += 1.0;
            charge += state;
            coulomb_pe += 0.5 * ALPHA * static_cast<double>(state)
                        * b.d_phi_coulomb[i];
        }

        if (dual_substrate) {
            const double flx = b.d_flux_L_x[i], fly = b.d_flux_L_y[i], flz = b.d_flux_L_z[i];
            const double frx = b.d_flux_R_x[i], fry = b.d_flux_R_y[i], frz = b.d_flux_R_z[i];
            const double wlx = b.d_wave_vel_L_x[i], wly = b.d_wave_vel_L_y[i], wlz = b.d_wave_vel_L_z[i];
            const double wrx = b.d_wave_vel_R_x[i], wry = b.d_wave_vel_R_y[i], wrz = b.d_wave_vel_R_z[i];
            left += 0.5 * (flx * flx + fly * fly + flz * flz);
            right += 0.5 * (frx * frx + fry * fry + frz * frz);
            wave_left += 0.5 * (wlx * wlx + wly * wly + wlz * wlz);
            wave_right += 0.5 * (wrx * wrx + wry * wry + wrz * wrz);

            const double vx = b.d_velocity_x[i], vy = b.d_velocity_y[i], vz = b.d_velocity_z[i];
            const double speed2 = vx * vx + vy * vy + vz * vz;
            if (speed2 > 1e-12) {
                const double inv_speed = 1.0 / sqrt(speed2);
                const double ldot = (flx * vx + fly * vy + flz * vz) * inv_speed;
                const double rdot = (frx * vx + fry * vy + frz * vz) * inv_speed;
                chirality += (flx * flx + fly * fly + flz * flz - ldot * ldot)
                           - (frx * frx + fry * fry + frz * frz - rdot * rdot);
            } else {
                chirality += (flx * flx + fly * fly) - (frx * frx + fry * fry);
            }
        }

        if (strong_field) {
            const double sx = b.d_flux_strong_x[i], sy = b.d_flux_strong_y[i], sz = b.d_flux_strong_z[i];
            strong += 0.5 * (sx * sx + sy * sy + sz * sz);
        }
        const double ux = b.d_flux_weak_x[i], uy = b.d_flux_weak_y[i], uz = b.d_flux_weak_z[i];
        weak += 0.5 * (ux * ux + uy * uy + uz * uz);
    }

    reduce_sum_to(out, E_FIELD, field);
    reduce_sum_to(out, E_WAVE, wave);
    reduce_sum_to(out, E_PARTICLE_KE, particle_ke);
    reduce_sum_to(out, E_PARTICLE_REST, particle_rest);
    reduce_sum_to(out, E_MOMENTUM_X, momentum_x);
    reduce_sum_to(out, E_MOMENTUM_Y, momentum_y);
    reduce_sum_to(out, E_MOMENTUM_Z, momentum_z);
    reduce_sum_to(out, E_MANIFESTED, manifested);
    reduce_sum_to(out, E_CHARGE, charge);
    reduce_sum_to(out, E_LEFT, left);
    reduce_sum_to(out, E_RIGHT, right);
    reduce_sum_to(out, E_WAVE_LEFT, wave_left);
    reduce_sum_to(out, E_WAVE_RIGHT, wave_right);
    reduce_sum_to(out, E_CHIRALITY, chirality);
    reduce_sum_to(out, E_STRONG, strong);
    reduce_sum_to(out, E_WEAK, weak);
    reduce_sum_to(out, E_ELECTRIC, electric);
    reduce_sum_to(out, E_MAGNETIC, magnetic);
    reduce_sum_to(out, E_POYNTING_X, poynting_x);
    reduce_sum_to(out, E_POYNTING_Y, poynting_y);
    reduce_sum_to(out, E_POYNTING_Z, poynting_z);
    reduce_sum_to(out, E_GAUSS_SUM, gauss_sum);
    reduce_max_to(out, E_GAUSS_MAX, gauss_max);
    reduce_sum_to(out, E_COULOMB_PE, coulomb_pe);
}

enum GravitySlot : int {
    G_LATENCY_MAX, G_LATENCY_SUM, G_GAMMA_MAX, G_VOXEL_COUNT, G_SLOT_COUNT
};

__global__ void compact_gravity_kernel(DiagnosticView b, double* out) {
    double latency_max = 0.0, latency_sum = 0.0, gamma_max = 0.0, count = 0.0;
    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        const double latency = b.d_latency[i];
        if (latency <= 0.0) continue;
        const double vx = b.d_velocity_x[i], vy = b.d_velocity_y[i], vz = b.d_velocity_z[i];
        latency_max = fmax(latency_max, latency);
        latency_sum += latency;
        gamma_max = fmax(gamma_max,
                         transport_gamma(latency, vx * vx + vy * vy + vz * vz));
        count += 1.0;
    }
    reduce_max_to(out, G_LATENCY_MAX, latency_max);
    reduce_sum_to(out, G_LATENCY_SUM, latency_sum);
    reduce_max_to(out, G_GAMMA_MAX, gamma_max);
    reduce_sum_to(out, G_VOXEL_COUNT, count);
}

enum VoxelSlot : int {
    V_STATE, V_PARTICLE_ID, V_PAIR_ID, V_LOCKED, V_SPIN, V_COLOR,
    V_FLUX_X, V_FLUX_Y, V_FLUX_Z,
    V_WAVE_X, V_WAVE_Y, V_WAVE_Z,
    V_VELOCITY_X, V_VELOCITY_Y, V_VELOCITY_Z,
    V_TAU, V_PHASE, V_LATENCY, V_ACCELERATION,
    V_DIVERGENCE, V_CURL_X, V_CURL_Y, V_CURL_Z,
    V_COUNT
};

__global__ void compact_voxel_kernel(DiagnosticView b, int index, double* out) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    int x, y, z;
    coordinates(index, b.L, x, y, z);
    double div, curl_x, curl_y, curl_z;
    divergence_and_curl(b, x, y, z, div, curl_x, curl_y, curl_z);
    out[V_STATE] = static_cast<double>(b.d_state[index]);
    out[V_PARTICLE_ID] = static_cast<double>(b.d_particle_id[index]);
    out[V_PAIR_ID] = static_cast<double>(b.d_pair_id[index]);
    out[V_LOCKED] = b.d_locked[index] ? 1.0 : 0.0;
    out[V_SPIN] = static_cast<double>(b.d_spin[index]);
    out[V_COLOR] = static_cast<double>(b.d_color[index]);
    out[V_FLUX_X] = b.d_flux_x[index]; out[V_FLUX_Y] = b.d_flux_y[index];
    out[V_FLUX_Z] = b.d_flux_z[index];
    out[V_WAVE_X] = b.d_wave_vel_x[index]; out[V_WAVE_Y] = b.d_wave_vel_y[index];
    out[V_WAVE_Z] = b.d_wave_vel_z[index];
    out[V_VELOCITY_X] = b.d_velocity_x[index];
    out[V_VELOCITY_Y] = b.d_velocity_y[index];
    out[V_VELOCITY_Z] = b.d_velocity_z[index];
    out[V_TAU] = b.d_tau[index]; out[V_PHASE] = b.d_phase[index];
    out[V_LATENCY] = b.d_latency[index];
    out[V_ACCELERATION] = b.d_accel_mag[index];
    out[V_DIVERGENCE] = div;
    out[V_CURL_X] = curl_x; out[V_CURL_Y] = curl_y; out[V_CURL_Z] = curl_z;
}

enum ForceSlot : int {
    F_COULOMB_X, F_COULOMB_Y, F_COULOMB_Z,
    F_STRONG_X, F_STRONG_Y, F_STRONG_Z,
    F_MAGNETIC_X, F_MAGNETIC_Y, F_MAGNETIC_Z,
    F_GRAVITY_X, F_GRAVITY_Y, F_GRAVITY_Z,
    F_EXCHANGE_X, F_EXCHANGE_Y, F_EXCHANGE_Z,
    F_COUNT
};

__global__ void compact_force_kernel(DiagnosticView b, int index, double* out) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    out[F_COULOMB_X] = b.d_fd_coulomb_x[index];
    out[F_COULOMB_Y] = b.d_fd_coulomb_y[index];
    out[F_COULOMB_Z] = b.d_fd_coulomb_z[index];
    out[F_STRONG_X] = b.d_fd_strong_x[index];
    out[F_STRONG_Y] = b.d_fd_strong_y[index];
    out[F_STRONG_Z] = b.d_fd_strong_z[index];
    out[F_MAGNETIC_X] = b.d_fd_magnetic_x[index];
    out[F_MAGNETIC_Y] = b.d_fd_magnetic_y[index];
    out[F_MAGNETIC_Z] = b.d_fd_magnetic_z[index];
    out[F_GRAVITY_X] = b.d_fd_gravity_x[index];
    out[F_GRAVITY_Y] = b.d_fd_gravity_y[index];
    out[F_GRAVITY_Z] = b.d_fd_gravity_z[index];
    out[F_EXCHANGE_X] = b.d_fd_exchange_x[index];
    out[F_EXCHANGE_Y] = b.d_fd_exchange_y[index];
    out[F_EXCHANGE_Z] = b.d_fd_exchange_z[index];
}

enum LagrangianSlot : int {
    L_FIELD_KINETIC, L_FIELD_GRADIENT, L_BORN_INFELD, L_COUPLING,
    L_VELOCITY_COUPLING, L_GAUSS, L_DISSIPATION, L_TOTAL, L_HAMILTONIAN,
    L_GAUSS_VIOLATION, L_GAUSS_MAX, L_TOTAL_FLUX, L_TOTAL_WAVE,
    L_MANIFESTED, L_LOCKED, L_COUNT
};

__device__ __forceinline__ double flux_difference_sq(const DiagnosticView b,
                                                      int a, int c) {
    const double dx = b.d_flux_x[c] - b.d_flux_x[a];
    const double dy = b.d_flux_y[c] - b.d_flux_y[a];
    const double dz = b.d_flux_z[c] - b.d_flux_z[a];
    return dx * dx + dy * dy + dz * dz;
}

__global__ void compact_lagrangian_kernel(DiagnosticView b, double* out) {
    double fk_sum = 0.0, fg_sum = 0.0, bi_sum = 0.0, coupling_sum = 0.0;
    double velocity_coupling_sum = 0.0, gauss_sum = 0.0, dissipation_sum = 0.0;
    double total = 0.0, hamiltonian = 0.0, violation_sum = 0.0, violation_max = 0.0;
    double total_flux = 0.0, total_wave = 0.0, manifested = 0.0, locked = 0.0;
    constexpr double C2 = C_SPEED * C_SPEED;
    constexpr double LAMBDA_G_DIAGNOSTIC = 100.0;

    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        int x, y, z;
        coordinates(i, b.L, x, y, z);
        double div, unused_x, unused_y, unused_z;
        divergence_and_curl(b, x, y, z, div, unused_x, unused_y, unused_z);

        double grad_sq = 0.0;
        grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x + 1, y, z, b.L));
        grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x - 1, y, z, b.L));
        grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y + 1, z, b.L));
        grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y - 1, z, b.L));
        grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y, z + 1, b.L));
        grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y, z - 1, b.L));
        for (int sx = -1; sx <= 1; sx += 2)
        for (int sy = -1; sy <= 1; sy += 2) {
            grad_sq += (1.0 / 6.0) * flux_difference_sq(b, i, index_3d(x + sx, y + sy, z, b.L));
            grad_sq += (1.0 / 6.0) * flux_difference_sq(b, i, index_3d(x + sx, y, z + sy, b.L));
            grad_sq += (1.0 / 6.0) * flux_difference_sq(b, i, index_3d(x, y + sx, z + sy, b.L));
        }

        const double fx = b.d_flux_x[i], fy = b.d_flux_y[i], fz = b.d_flux_z[i];
        const double wx = b.d_wave_vel_x[i], wy = b.d_wave_vel_y[i], wz = b.d_wave_vel_z[i];
        const double vx = b.d_velocity_x[i], vy = b.d_velocity_y[i], vz = b.d_velocity_z[i];
        const double speed2 = vx * vx + vy * vy + vz * vz;
        const double wave2 = wx * wx + wy * wy + wz * wz;
        const double rho = static_cast<double>(b.d_state[i]);
        const double fk = 0.5 * wave2;
        const double fg = -0.25 * C2 * grad_sq;
        const double bi = born_infeld_core(b.d_latency[i], speed2);
        const double coupling = G_C * rho * div;
        const double velocity_coupling = -G_C * rho * (vx * fx + vy * fy + vz * fz);
        const double violation = div - rho;
        const double gauss = -LAMBDA_G_DIAGNOSTIC * violation * violation;
        const double dissipation = 0.5 * DAMPING * wave2;

        fk_sum += fk; fg_sum += fg; bi_sum += bi; coupling_sum += coupling;
        velocity_coupling_sum += velocity_coupling; gauss_sum += gauss;
        dissipation_sum += dissipation;
        total += fk + fg + bi + coupling + velocity_coupling + gauss;
        hamiltonian += born_infeld_hamiltonian(b.d_latency[i], speed2)
                     - coupling - velocity_coupling - gauss;
        violation_sum += violation * violation;
        violation_max = fmax(violation_max, fabs(violation));
        total_flux += sqrt(fx * fx + fy * fy + fz * fz);
        total_wave += 0.5 * wave2;
        if (b.d_state[i] != 0) {
            manifested += 1.0;
            locked += b.d_locked[i] ? 1.0 : 0.0;
        }
    }

    reduce_sum_to(out, L_FIELD_KINETIC, fk_sum);
    reduce_sum_to(out, L_FIELD_GRADIENT, fg_sum);
    reduce_sum_to(out, L_BORN_INFELD, bi_sum);
    reduce_sum_to(out, L_COUPLING, coupling_sum);
    reduce_sum_to(out, L_VELOCITY_COUPLING, velocity_coupling_sum);
    reduce_sum_to(out, L_GAUSS, gauss_sum);
    reduce_sum_to(out, L_DISSIPATION, dissipation_sum);
    reduce_sum_to(out, L_TOTAL, total);
    reduce_sum_to(out, L_HAMILTONIAN, hamiltonian);
    reduce_sum_to(out, L_GAUSS_VIOLATION, violation_sum);
    reduce_max_to(out, L_GAUSS_MAX, violation_max);
    reduce_sum_to(out, L_TOTAL_FLUX, total_flux);
    reduce_sum_to(out, L_TOTAL_WAVE, total_wave);
    reduce_sum_to(out, L_MANIFESTED, manifested);
    reduce_sum_to(out, L_LOCKED, locked);
}

// ──────────────────────────────────────────────────────────────────────────
// Coherent telemetry snapshot reduction
// ──────────────────────────────────────────────────────────────────────────
// Legacy compact getters intentionally keep their own kernels below for API
// compatibility. Native interactive telemetry uses this fused pass instead:
// diagnostics + audit + gravity share every field load and traverse N sites
// once. The optional Lagrangian section remains conditional because its
// 18-point stencil is materially more expensive than the dashboard summary.

enum TelemetrySlot : int {
    T_DIAGNOSTIC_BASE = 0,
    T_ENERGY_BASE = T_DIAGNOSTIC_BASE + D_COUNT,
    T_GRAVITY_BASE = T_ENERGY_BASE + E_COUNT,
    T_LAGRANGIAN_BASE = T_GRAVITY_BASE + G_SLOT_COUNT,
    T_COUNT = T_LAGRANGIAN_BASE + L_COUNT,
};

static_assert(T_COUNT <= GpuBuffers::COMPACT_TELEMETRY_SCALARS,
              "telemetry snapshot scratch overflow");

__global__ void compact_telemetry_kernel(
    DiagnosticView b, bool want_diagnostics, bool want_audit,
    bool want_gravity, bool want_lagrangian, bool dual_substrate,
    bool strong_field, bool movement, double charge_coupling,
    const long long* charge_sum, double* out) {
    // Diagnostics accumulators.
    double d_total_flux = 0.0, d_bi_abs = 0.0;
    double d_max_bandwidth = 0.0, d_max_budget = 0.0;
    double d_manifested = 0.0, d_positive = 0.0, d_negative = 0.0;
    double d_spin_up = 0.0, d_spin_down = 0.0;
    double d_color0 = 0.0, d_color1 = 0.0, d_color2 = 0.0, d_color3 = 0.0;
    double d_rho2_sum = 0.0, d_rho2_log_sum = 0.0;
    double d_coord_x = 0.0, d_coord_y = 0.0, d_coord_z = 0.0;
    double d_vel_x = 0.0, d_vel_y = 0.0, d_vel_z = 0.0;
    double d_rxv_x = 0.0, d_rxv_y = 0.0, d_rxv_z = 0.0;

    // Energy-audit accumulators.
    double e_field = 0.0, e_wave = 0.0, e_particle_ke = 0.0;
    double e_particle_rest = 0.0;
    double e_momentum_x = 0.0, e_momentum_y = 0.0, e_momentum_z = 0.0;
    double e_manifested = 0.0, e_charge = 0.0;
    double e_left = 0.0, e_right = 0.0, e_wave_left = 0.0, e_wave_right = 0.0;
    double e_chirality = 0.0, e_strong = 0.0, e_weak = 0.0;
    double e_electric = 0.0, e_magnetic = 0.0;
    double e_poynting_x = 0.0, e_poynting_y = 0.0, e_poynting_z = 0.0;
    double e_gauss_sum = 0.0, e_gauss_max = 0.0, e_coulomb_pe = 0.0;

    // Gravity accumulators.
    double g_latency_max = 0.0, g_latency_sum = 0.0;
    double g_gamma_max = 0.0, g_voxel_count = 0.0;

    // Lagrangian accumulators.
    double l_fk_sum = 0.0, l_fg_sum = 0.0, l_bi_sum = 0.0;
    double l_coupling_sum = 0.0, l_velocity_coupling_sum = 0.0;
    double l_gauss_sum = 0.0, l_dissipation_sum = 0.0;
    double l_total = 0.0, l_hamiltonian = 0.0;
    double l_violation_sum = 0.0, l_violation_max = 0.0;
    double l_total_flux = 0.0, l_total_wave = 0.0;
    double l_manifested = 0.0, l_locked = 0.0;

    const double mean_charge = want_audit
        ? static_cast<double>(*charge_sum) / static_cast<double>(b.N) : 0.0;
    constexpr double C2 = C_SPEED * C_SPEED;
    constexpr double LAMBDA_G_DIAGNOSTIC = 100.0;

    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        const double fx = b.d_flux_x[i], fy = b.d_flux_y[i], fz = b.d_flux_z[i];
        const double wx = b.d_wave_vel_x[i], wy = b.d_wave_vel_y[i], wz = b.d_wave_vel_z[i];
        const double vx = b.d_velocity_x[i], vy = b.d_velocity_y[i], vz = b.d_velocity_z[i];
        const double flux2 = fx * fx + fy * fy + fz * fz;
        const double wave2 = wx * wx + wy * wy + wz * wz;
        const double speed2 = vx * vx + vy * vy + vz * vz;
        const double latency = b.d_latency[i];
        const int state = static_cast<int>(b.d_state[i]);

        int x, y, z;
        coordinates(i, b.L, x, y, z);

        double div = 0.0, curl_x = 0.0, curl_y = 0.0, curl_z = 0.0;
        if (want_audit || want_lagrangian) {
            divergence_and_curl(b, x, y, z, div, curl_x, curl_y, curl_z);
        }

        if (want_diagnostics) {
            d_total_flux += sqrt(flux2);
            d_bi_abs += fabs(born_infeld_core(latency, speed2));
            d_max_bandwidth = fmax(d_max_bandwidth,
                                   bandwidth_fraction(latency, speed2));
            d_max_budget = fmax(d_max_budget, causal_budget(latency, speed2));
            d_rho2_sum += flux2;
            if (flux2 > EPSILON_FLUX_SQ) d_rho2_log_sum += flux2 * log(flux2);

            if (state != 0) {
                d_manifested += 1.0;
                d_positive += state > 0 ? 1.0 : 0.0;
                d_negative += state < 0 ? 1.0 : 0.0;
                const int spin = static_cast<int>(b.d_spin[i]);
                d_spin_up += spin > 0 ? 1.0 : 0.0;
                d_spin_down += spin < 0 ? 1.0 : 0.0;
                const int color = static_cast<int>(b.d_color[i]);
                d_color0 += color == 0 ? 1.0 : 0.0;
                d_color1 += color == 1 ? 1.0 : 0.0;
                d_color2 += color == 2 ? 1.0 : 0.0;
                d_color3 += color == 3 ? 1.0 : 0.0;
                d_coord_x += x; d_coord_y += y; d_coord_z += z;
                d_vel_x += vx; d_vel_y += vy; d_vel_z += vz;
                d_rxv_x += static_cast<double>(y) * vz - static_cast<double>(z) * vy;
                d_rxv_y += static_cast<double>(z) * vx - static_cast<double>(x) * vz;
                d_rxv_z += static_cast<double>(x) * vy - static_cast<double>(y) * vx;
            }
        }

        if (want_audit) {
            e_field += quadratic_field_energy_density(flux2);
            e_wave += quadratic_field_energy_density(wave2);
            e_electric += quadratic_field_energy_density(wave2);
            e_magnetic += C2 * quadratic_field_energy_density(
                curl_x * curl_x + curl_y * curl_y + curl_z * curl_z);
            // E = -wave_vel; S = c^2 E x B.
            e_poynting_x += C2 * ((-wy) * curl_z - (-wz) * curl_y);
            e_poynting_y += C2 * ((-wz) * curl_x - (-wx) * curl_z);
            e_poynting_z += C2 * ((-wx) * curl_y - (-wy) * curl_x);

            if (state == 0) {
                const double err = div + charge_coupling * mean_charge;
                e_gauss_sum += err * err;
                e_gauss_max = fmax(e_gauss_max, fabs(err));
            } else {
                const double gamma0 = flat_gamma(speed2);
                e_particle_ke += flat_particle_kinetic_energy(speed2);
                e_particle_rest += E_REST;
                e_momentum_x += vx * (gamma0 * M_INERTIAL);
                e_momentum_y += vy * (gamma0 * M_INERTIAL);
                e_momentum_z += vz * (gamma0 * M_INERTIAL);
                e_manifested += 1.0;
                e_charge += state;
                e_coulomb_pe += 0.5 * ALPHA * static_cast<double>(state)
                              * b.d_phi_coulomb[i];
            }

            if (dual_substrate) {
                const double flx = b.d_flux_L_x[i], fly = b.d_flux_L_y[i], flz = b.d_flux_L_z[i];
                const double frx = b.d_flux_R_x[i], fry = b.d_flux_R_y[i], frz = b.d_flux_R_z[i];
                const double wlx = b.d_wave_vel_L_x[i], wly = b.d_wave_vel_L_y[i], wlz = b.d_wave_vel_L_z[i];
                const double wrx = b.d_wave_vel_R_x[i], wry = b.d_wave_vel_R_y[i], wrz = b.d_wave_vel_R_z[i];
                e_left += 0.5 * (flx * flx + fly * fly + flz * flz);
                e_right += 0.5 * (frx * frx + fry * fry + frz * frz);
                e_wave_left += 0.5 * (wlx * wlx + wly * wly + wlz * wlz);
                e_wave_right += 0.5 * (wrx * wrx + wry * wry + wrz * wrz);
                if (speed2 > 1e-12) {
                    const double inv_speed = 1.0 / sqrt(speed2);
                    const double ldot = (flx * vx + fly * vy + flz * vz) * inv_speed;
                    const double rdot = (frx * vx + fry * vy + frz * vz) * inv_speed;
                    e_chirality += (flx * flx + fly * fly + flz * flz - ldot * ldot)
                                 - (frx * frx + fry * fry + frz * frz - rdot * rdot);
                } else {
                    e_chirality += (flx * flx + fly * fly) - (frx * frx + fry * fry);
                }
            }
            if (strong_field) {
                const double sx = b.d_flux_strong_x[i], sy = b.d_flux_strong_y[i], sz = b.d_flux_strong_z[i];
                e_strong += 0.5 * (sx * sx + sy * sy + sz * sz);
            }
            const double ux = b.d_flux_weak_x[i], uy = b.d_flux_weak_y[i], uz = b.d_flux_weak_z[i];
            e_weak += 0.5 * (ux * ux + uy * uy + uz * uz);
        }

        if (want_gravity && latency > 0.0) {
            g_latency_max = fmax(g_latency_max, latency);
            g_latency_sum += latency;
            g_gamma_max = fmax(g_gamma_max, transport_gamma(latency, speed2));
            g_voxel_count += 1.0;
        }

        if (want_lagrangian) {
            double grad_sq = 0.0;
            grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x + 1, y, z, b.L));
            grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x - 1, y, z, b.L));
            grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y + 1, z, b.L));
            grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y - 1, z, b.L));
            grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y, z + 1, b.L));
            grad_sq += (1.0 / 3.0) * flux_difference_sq(b, i, index_3d(x, y, z - 1, b.L));
            for (int sx = -1; sx <= 1; sx += 2)
            for (int sy = -1; sy <= 1; sy += 2) {
                grad_sq += (1.0 / 6.0) * flux_difference_sq(b, i, index_3d(x + sx, y + sy, z, b.L));
                grad_sq += (1.0 / 6.0) * flux_difference_sq(b, i, index_3d(x + sx, y, z + sy, b.L));
                grad_sq += (1.0 / 6.0) * flux_difference_sq(b, i, index_3d(x, y + sx, z + sy, b.L));
            }
            const double rho = static_cast<double>(state);
            const double fk = 0.5 * wave2;
            const double fg = -0.25 * C2 * grad_sq;
            const double bi = born_infeld_core(latency, speed2);
            const double coupling = G_C * rho * div;
            const double velocity_coupling = -G_C * rho * (vx * fx + vy * fy + vz * fz);
            const double violation = div - rho;
            const double gauss = -LAMBDA_G_DIAGNOSTIC * violation * violation;
            const double dissipation = 0.5 * DAMPING * wave2;
            l_fk_sum += fk; l_fg_sum += fg; l_bi_sum += bi;
            l_coupling_sum += coupling;
            l_velocity_coupling_sum += velocity_coupling;
            l_gauss_sum += gauss; l_dissipation_sum += dissipation;
            l_total += fk + fg + bi + coupling + velocity_coupling + gauss;
            l_hamiltonian += born_infeld_hamiltonian(latency, speed2)
                          - coupling - velocity_coupling - gauss;
            l_violation_sum += violation * violation;
            l_violation_max = fmax(l_violation_max, fabs(violation));
            l_total_flux += sqrt(flux2);
            l_total_wave += 0.5 * wave2;
            if (state != 0) {
                l_manifested += 1.0;
                l_locked += b.d_locked[i] ? 1.0 : 0.0;
            }
        }
    }

    if (want_diagnostics) {
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_TOTAL_FLUX, d_total_flux);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_BI_ABS, d_bi_abs);
        reduce_max_to(out, T_DIAGNOSTIC_BASE + D_MAX_BANDWIDTH, d_max_bandwidth);
        reduce_max_to(out, T_DIAGNOSTIC_BASE + D_MAX_BUDGET, d_max_budget);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_MANIFESTED, d_manifested);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_POSITIVE, d_positive);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_NEGATIVE, d_negative);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_SPIN_UP, d_spin_up);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_SPIN_DOWN, d_spin_down);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COLOR_0, d_color0);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COLOR_1, d_color1);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COLOR_2, d_color2);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COLOR_3, d_color3);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_RHO2_SUM, d_rho2_sum);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_RHO2_LOG_SUM, d_rho2_log_sum);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COORD_X, d_coord_x);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COORD_Y, d_coord_y);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_COORD_Z, d_coord_z);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_VEL_X, d_vel_x);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_VEL_Y, d_vel_y);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_VEL_Z, d_vel_z);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_RXV_X, d_rxv_x);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_RXV_Y, d_rxv_y);
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_RXV_Z, d_rxv_z);
        const double causal = (movement && begin == 0)
            ? static_cast<double>(*b.d_causal_projection_events) : 0.0;
        reduce_sum_to(out, T_DIAGNOSTIC_BASE + D_CAUSAL_PROJECTIONS, causal);
    }
    if (want_audit) {
        reduce_sum_to(out, T_ENERGY_BASE + E_FIELD, e_field);
        reduce_sum_to(out, T_ENERGY_BASE + E_WAVE, e_wave);
        reduce_sum_to(out, T_ENERGY_BASE + E_PARTICLE_KE, e_particle_ke);
        reduce_sum_to(out, T_ENERGY_BASE + E_PARTICLE_REST, e_particle_rest);
        reduce_sum_to(out, T_ENERGY_BASE + E_MOMENTUM_X, e_momentum_x);
        reduce_sum_to(out, T_ENERGY_BASE + E_MOMENTUM_Y, e_momentum_y);
        reduce_sum_to(out, T_ENERGY_BASE + E_MOMENTUM_Z, e_momentum_z);
        reduce_sum_to(out, T_ENERGY_BASE + E_MANIFESTED, e_manifested);
        reduce_sum_to(out, T_ENERGY_BASE + E_CHARGE, e_charge);
        reduce_sum_to(out, T_ENERGY_BASE + E_LEFT, e_left);
        reduce_sum_to(out, T_ENERGY_BASE + E_RIGHT, e_right);
        reduce_sum_to(out, T_ENERGY_BASE + E_WAVE_LEFT, e_wave_left);
        reduce_sum_to(out, T_ENERGY_BASE + E_WAVE_RIGHT, e_wave_right);
        reduce_sum_to(out, T_ENERGY_BASE + E_CHIRALITY, e_chirality);
        reduce_sum_to(out, T_ENERGY_BASE + E_STRONG, e_strong);
        reduce_sum_to(out, T_ENERGY_BASE + E_WEAK, e_weak);
        reduce_sum_to(out, T_ENERGY_BASE + E_ELECTRIC, e_electric);
        reduce_sum_to(out, T_ENERGY_BASE + E_MAGNETIC, e_magnetic);
        reduce_sum_to(out, T_ENERGY_BASE + E_POYNTING_X, e_poynting_x);
        reduce_sum_to(out, T_ENERGY_BASE + E_POYNTING_Y, e_poynting_y);
        reduce_sum_to(out, T_ENERGY_BASE + E_POYNTING_Z, e_poynting_z);
        reduce_sum_to(out, T_ENERGY_BASE + E_GAUSS_SUM, e_gauss_sum);
        reduce_max_to(out, T_ENERGY_BASE + E_GAUSS_MAX, e_gauss_max);
        reduce_sum_to(out, T_ENERGY_BASE + E_COULOMB_PE, e_coulomb_pe);
    }
    if (want_gravity) {
        reduce_max_to(out, T_GRAVITY_BASE + G_LATENCY_MAX, g_latency_max);
        reduce_sum_to(out, T_GRAVITY_BASE + G_LATENCY_SUM, g_latency_sum);
        reduce_max_to(out, T_GRAVITY_BASE + G_GAMMA_MAX, g_gamma_max);
        reduce_sum_to(out, T_GRAVITY_BASE + G_VOXEL_COUNT, g_voxel_count);
    }
    if (want_lagrangian) {
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_FIELD_KINETIC, l_fk_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_FIELD_GRADIENT, l_fg_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_BORN_INFELD, l_bi_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_COUPLING, l_coupling_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_VELOCITY_COUPLING, l_velocity_coupling_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_GAUSS, l_gauss_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_DISSIPATION, l_dissipation_sum);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_TOTAL, l_total);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_HAMILTONIAN, l_hamiltonian);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_GAUSS_VIOLATION, l_violation_sum);
        reduce_max_to(out, T_LAGRANGIAN_BASE + L_GAUSS_MAX, l_violation_max);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_TOTAL_FLUX, l_total_flux);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_TOTAL_WAVE, l_total_wave);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_MANIFESTED, l_manifested);
        reduce_sum_to(out, T_LAGRANGIAN_BASE + L_LOCKED, l_locked);
    }
}

int reduction_grid(int N) {
    return std::max(1, std::min(MAX_REDUCTION_BLOCKS, (N + THREADS - 1) / THREADS));
}

template <int Count>
std::vector<double> download_result(GpuBuffers& b) {
    static_assert(Count <= GpuBuffers::COMPACT_DIAGNOSTIC_SCALARS,
                  "compact diagnostic scratch overflow");
    std::vector<double> host(Count, 0.0);
    const std::size_t bytes = static_cast<std::size_t>(Count) * sizeof(double);
    CUDA_CHECK(cudaMemcpy(host.data(), b.d_compact_diagnostics, bytes,
                          cudaMemcpyDeviceToHost));
    g_gpu_compact_diagnostic_download_bytes += bytes;
    return host;
}

template <int Count>
void clear_result(GpuBuffers& b) {
    CUDA_CHECK(cudaMemset(b.d_compact_diagnostics, 0,
                          static_cast<std::size_t>(Count) * sizeof(double)));
}

}  // namespace

void launch_compact_diagnostics(GpuBuffers& b, int tick, bool movement,
                                Diagnostics& d) {
    clear_result<D_COUNT>(b);
    compact_diagnostics_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), movement, b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<D_COUNT>(b);

    d = Diagnostics{};
    d.tick = tick;
    d.total_flux = h[D_TOTAL_FLUX];
    d.total_energy = h[D_BI_ABS];
    d.max_bandwidth = h[D_MAX_BANDWIDTH];
    d.max_causal_budget = h[D_MAX_BUDGET];
    d.manifested_count = static_cast<int>(llround(h[D_MANIFESTED]));
    d.positive_count = static_cast<int>(llround(h[D_POSITIVE]));
    d.negative_count = static_cast<int>(llround(h[D_NEGATIVE]));
    d.spin_up_count = static_cast<int>(llround(h[D_SPIN_UP]));
    d.spin_down_count = static_cast<int>(llround(h[D_SPIN_DOWN]));
    d.color_count[0] = static_cast<int>(llround(h[D_COLOR_0]));
    d.color_count[1] = static_cast<int>(llround(h[D_COLOR_1]));
    d.color_count[2] = static_cast<int>(llround(h[D_COLOR_2]));
    d.color_count[3] = static_cast<int>(llround(h[D_COLOR_3]));
    const double rho2 = h[D_RHO2_SUM];
    if (rho2 >= EPSILON_FLUX_SQ)
        d.total_entropy = log(rho2) - h[D_RHO2_LOG_SUM] / rho2;
    d.causal_projection_events = static_cast<long long>(llround(h[D_CAUSAL_PROJECTIONS]));

    if (d.manifested_count > 0) {
        const double inv_n = 1.0 / static_cast<double>(d.manifested_count);
        const double cx = h[D_COORD_X] * inv_n;
        const double cy = h[D_COORD_Y] * inv_n;
        const double cz = h[D_COORD_Z] * inv_n;
        d.total_angular_momentum.x = h[D_RXV_X] - (cy * h[D_VEL_Z] - cz * h[D_VEL_Y]);
        d.total_angular_momentum.y = h[D_RXV_Y] - (cz * h[D_VEL_X] - cx * h[D_VEL_Z]);
        d.total_angular_momentum.z = h[D_RXV_Z] - (cx * h[D_VEL_Y] - cy * h[D_VEL_X]);
    }
}

void launch_compact_energy_audit(GpuBuffers& b, const TermToggles& toggles,
                                 EnergyAudit& a) {
    CUDA_CHECK(cudaMemset(b.d_compact_charge_sum, 0, sizeof(long long)));
    charge_sum_kernel<<<reduction_grid(b.N), THREADS>>>(
        b.d_state, b.N, b.d_compact_charge_sum);
    CUDA_CHECK(cudaGetLastError());
    clear_result<E_COUNT>(b);
    compact_energy_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), toggles.dual_substrate,
        toggles.color_forces || toggles.strong_force,
        toggles.coulomb_charge_coupling, b.d_compact_charge_sum,
        b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<E_COUNT>(b);

    a = EnergyAudit{};
    a.field_energy = h[E_FIELD];
    a.wave_energy = h[E_WAVE];
    a.field_energy_density_sum = h[E_FIELD];
    a.wave_energy_density_sum = h[E_WAVE];
    a.particle_ke = h[E_PARTICLE_KE];
    a.particle_rest_energy = h[E_PARTICLE_REST];
    a.particle_momentum = {h[E_MOMENTUM_X], h[E_MOMENTUM_Y], h[E_MOMENTUM_Z]};
    a.manifested_count = static_cast<int>(llround(h[E_MANIFESTED]));
    a.charge_total = static_cast<int>(llround(h[E_CHARGE]));
    a.E_L_total = h[E_LEFT];
    a.E_R_total = h[E_RIGHT];
    a.wv_L_total = h[E_WAVE_LEFT];
    a.wv_R_total = h[E_WAVE_RIGHT];
    a.chirality_total = h[E_CHIRALITY];
    a.strong_energy = h[E_STRONG];
    a.weak_energy = h[E_WEAK];
    a.E_field_energy = h[E_ELECTRIC];
    a.B_field_energy = h[E_MAGNETIC];
    a.total_poynting = {h[E_POYNTING_X], h[E_POYNTING_Y], h[E_POYNTING_Z]};
    a.gauss_violation = h[E_GAUSS_SUM];
    a.max_gauss_error = h[E_GAUSS_MAX];
    a.coulomb_pe = h[E_COULOMB_PE];
    a.particle_energy = a.particle_rest_energy + a.particle_ke;
    a.dynamic_energy = a.field_energy + a.wave_energy + a.particle_ke;
    a.total_energy = a.field_energy + a.wave_energy + a.particle_energy;
}

void launch_compact_gravity_metric(GpuBuffers& b, const TermToggles& toggles,
                                   GravityMetricAgg& a) {
    clear_result<G_SLOT_COUNT>(b);
    compact_gravity_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<G_SLOT_COUNT>(b);

    a = GravityMetricAgg{};
    a.latency_max = h[G_LATENCY_MAX];
    a.voxel_count = static_cast<int>(llround(h[G_VOXEL_COUNT]));
    a.gamma_max = a.voxel_count > 0 ? h[G_GAMMA_MAX] : 1.0;
    if (a.voxel_count > 0) {
        a.latency_mean = h[G_LATENCY_SUM] / static_cast<double>(a.voxel_count);
        a.f_min = 1.0 - a.latency_max * a.latency_max;
        a.dilation_max_pct = (1.0 - sqrt(fmax(0.0, a.f_min))) * 100.0;
    }
    a.requested = toggles.latency_field || toggles.field_energy_gravity;
    a.active = a.requested && a.voxel_count > 0;
}

void launch_compact_voxel(GpuBuffers& b, int index, VoxelInspection& out) {
    index = std::max(0, std::min(b.N - 1, index));
    clear_result<V_COUNT>(b);
    compact_voxel_kernel<<<1, 1>>>(diagnostic_view(b), index,
                                   b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<V_COUNT>(b);

    out = VoxelInspection{};
    auto& v = out.voxel;
    v.state = static_cast<std::int8_t>(llround(h[V_STATE]));
    v.particle_id = static_cast<std::int32_t>(llround(h[V_PARTICLE_ID]));
    v.pair_id = static_cast<int>(llround(h[V_PAIR_ID]));
    v.locked = h[V_LOCKED] != 0.0;
    v.spin = static_cast<std::int8_t>(llround(h[V_SPIN]));
    v.color = static_cast<std::int8_t>(llround(h[V_COLOR]));
    v.flux = {h[V_FLUX_X], h[V_FLUX_Y], h[V_FLUX_Z]};
    v.wave_vel = {h[V_WAVE_X], h[V_WAVE_Y], h[V_WAVE_Z]};
    v.velocity = {h[V_VELOCITY_X], h[V_VELOCITY_Y], h[V_VELOCITY_Z]};
    v.tau = h[V_TAU]; v.phase = h[V_PHASE];
    v.latency = h[V_LATENCY]; v.accel_mag = h[V_ACCELERATION];
    out.divergence = h[V_DIVERGENCE];
    out.curl = {h[V_CURL_X], h[V_CURL_Y], h[V_CURL_Z]};
    out.em.E = v.wave_vel * -1.0;
    out.em.B = out.curl;
    out.em.E_mag = out.em.E.mag();
    out.em.B_mag = out.em.B.mag();
}

void launch_compact_force(GpuBuffers& b, int index, ForceDiag& out) {
    index = std::max(0, std::min(b.N - 1, index));
    clear_result<F_COUNT>(b);
    compact_force_kernel<<<1, 1>>>(diagnostic_view(b), index,
                                   b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<F_COUNT>(b);
    out = ForceDiag{};
    out.f_coulomb = {h[F_COULOMB_X], h[F_COULOMB_Y], h[F_COULOMB_Z]};
    out.f_strong = {h[F_STRONG_X], h[F_STRONG_Y], h[F_STRONG_Z]};
    out.f_magnetic = {h[F_MAGNETIC_X], h[F_MAGNETIC_Y], h[F_MAGNETIC_Z]};
    out.f_gravity = {h[F_GRAVITY_X], h[F_GRAVITY_Y], h[F_GRAVITY_Z]};
    out.f_exchange = {h[F_EXCHANGE_X], h[F_EXCHANGE_Y], h[F_EXCHANGE_Z]};
}

void launch_compact_lagrangian(GpuBuffers& b, LagrangianDiag& d) {
    clear_result<L_COUNT>(b);
    compact_lagrangian_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<L_COUNT>(b);

    d = LagrangianDiag{};
    d.field_kinetic_sum = h[L_FIELD_KINETIC];
    d.field_gradient_sum = h[L_FIELD_GRADIENT];
    d.born_infeld_sum = h[L_BORN_INFELD];
    d.coupling_sum = h[L_COUPLING];
    d.velocity_coupling_sum = h[L_VELOCITY_COUPLING];
    d.gauss_sum = h[L_GAUSS];
    d.dissipation_sum = h[L_DISSIPATION];
    d.total_lagrangian = h[L_TOTAL];
    d.total_hamiltonian = h[L_HAMILTONIAN];
    d.total_action = d.total_lagrangian;
    d.gauss_violation = h[L_GAUSS_VIOLATION];
    d.max_gauss_error = h[L_GAUSS_MAX];
    d.total_flux_mag = h[L_TOTAL_FLUX];
    d.total_wave_energy = h[L_TOTAL_WAVE];
    d.manifested_count = static_cast<int>(llround(h[L_MANIFESTED]));
    d.locked_count = static_cast<int>(llround(h[L_LOCKED]));
}

void launch_telemetry_snapshot(GpuBuffers& b, std::uint32_t groups,
                               const TermToggles& toggles,
                               cudaEvent_t ready_event) {
    const bool want_diagnostics = (groups & TELEMETRY_DIAGNOSTICS) != 0;
    const bool want_audit = (groups & TELEMETRY_AUDIT) != 0;
    const bool want_gravity = (groups & TELEMETRY_GRAVITY) != 0;
    const bool want_lagrangian = (groups & TELEMETRY_LAGRANGIAN) != 0;

    CUDA_CHECK(cudaMemset(b.d_telemetry_snapshot, 0,
                          static_cast<std::size_t>(T_COUNT) * sizeof(double)));
    // The audit's neutral-background Gauss residual needs the exact global
    // charge. This is the only prepass; diagnostics/gravity/Lagrangian then
    // share one whole-grid traversal.
    if (want_audit) {
        CUDA_CHECK(cudaMemset(b.d_compact_charge_sum, 0, sizeof(long long)));
        charge_sum_kernel<<<reduction_grid(b.N), THREADS>>>(
            b.d_state, b.N, b.d_compact_charge_sum);
        CUDA_CHECK(cudaGetLastError());
    }

    compact_telemetry_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), want_diagnostics, want_audit, want_gravity,
        want_lagrangian, toggles.dual_substrate,
        toggles.color_forces || toggles.strong_force, toggles.movement,
        toggles.coulomb_charge_coupling, b.d_compact_charge_sum,
        b.d_telemetry_snapshot);
    CUDA_CHECK(cudaGetLastError());

    const std::size_t bytes = static_cast<std::size_t>(T_COUNT) * sizeof(double);
    CUDA_CHECK(cudaMemcpyAsync(b.h_telemetry_snapshot, b.d_telemetry_snapshot,
                               bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaEventRecord(ready_event));
    g_gpu_telemetry_snapshot_download_bytes += bytes;
    ++g_gpu_telemetry_snapshot_launches;
}

void decode_telemetry_snapshot(const GpuBuffers& b,
                               const TelemetrySnapshotRequest& request,
                               int tick, std::uint64_t state_version,
                               bool gravity_requested,
                               TelemetrySnapshot& out) {
    const double* h = b.h_telemetry_snapshot;
    out = TelemetrySnapshot{};
    out.epoch = request.epoch;
    out.state_version = state_version;
    out.tick = tick;
    out.physical_time = request.physical_time;
    out.dt = request.dt;
    out.lattice_size = request.lattice_size;
    out.groups = request.groups;
    const TelemetryGroupMeta meta{request.epoch, state_version, tick,
                                  request.physical_time, request.dt,
                                  request.lattice_size};

    if (request.groups & TELEMETRY_DIAGNOSTICS) {
        const int o = T_DIAGNOSTIC_BASE;
        auto& d = out.diagnostics;
        d.tick = tick;
        d.total_flux = h[o + D_TOTAL_FLUX];
        d.total_energy = h[o + D_BI_ABS];
        d.max_bandwidth = h[o + D_MAX_BANDWIDTH];
        d.max_causal_budget = h[o + D_MAX_BUDGET];
        d.manifested_count = static_cast<int>(llround(h[o + D_MANIFESTED]));
        d.positive_count = static_cast<int>(llround(h[o + D_POSITIVE]));
        d.negative_count = static_cast<int>(llround(h[o + D_NEGATIVE]));
        d.spin_up_count = static_cast<int>(llround(h[o + D_SPIN_UP]));
        d.spin_down_count = static_cast<int>(llround(h[o + D_SPIN_DOWN]));
        d.color_count[0] = static_cast<int>(llround(h[o + D_COLOR_0]));
        d.color_count[1] = static_cast<int>(llround(h[o + D_COLOR_1]));
        d.color_count[2] = static_cast<int>(llround(h[o + D_COLOR_2]));
        d.color_count[3] = static_cast<int>(llround(h[o + D_COLOR_3]));
        const double rho2 = h[o + D_RHO2_SUM];
        if (rho2 >= EPSILON_FLUX_SQ)
            d.total_entropy = log(rho2) - h[o + D_RHO2_LOG_SUM] / rho2;
        d.causal_projection_events = static_cast<long long>(
            llround(h[o + D_CAUSAL_PROJECTIONS]));
        if (d.manifested_count > 0) {
            const double inv_n = 1.0 / static_cast<double>(d.manifested_count);
            const double cx = h[o + D_COORD_X] * inv_n;
            const double cy = h[o + D_COORD_Y] * inv_n;
            const double cz = h[o + D_COORD_Z] * inv_n;
            d.total_angular_momentum.x = h[o + D_RXV_X]
                - (cy * h[o + D_VEL_Z] - cz * h[o + D_VEL_Y]);
            d.total_angular_momentum.y = h[o + D_RXV_Y]
                - (cz * h[o + D_VEL_X] - cx * h[o + D_VEL_Z]);
            d.total_angular_momentum.z = h[o + D_RXV_Z]
                - (cx * h[o + D_VEL_Y] - cy * h[o + D_VEL_X]);
        }
        out.diagnostics_meta = meta;
    }

    if (request.groups & TELEMETRY_AUDIT) {
        const int o = T_ENERGY_BASE;
        auto& a = out.audit;
        a.field_energy = h[o + E_FIELD];
        a.wave_energy = h[o + E_WAVE];
        a.field_energy_density_sum = h[o + E_FIELD];
        a.wave_energy_density_sum = h[o + E_WAVE];
        a.particle_ke = h[o + E_PARTICLE_KE];
        a.particle_rest_energy = h[o + E_PARTICLE_REST];
        a.particle_momentum = {h[o + E_MOMENTUM_X], h[o + E_MOMENTUM_Y],
                               h[o + E_MOMENTUM_Z]};
        a.manifested_count = static_cast<int>(llround(h[o + E_MANIFESTED]));
        a.charge_total = static_cast<int>(llround(h[o + E_CHARGE]));
        a.E_L_total = h[o + E_LEFT];
        a.E_R_total = h[o + E_RIGHT];
        a.wv_L_total = h[o + E_WAVE_LEFT];
        a.wv_R_total = h[o + E_WAVE_RIGHT];
        a.chirality_total = h[o + E_CHIRALITY];
        a.strong_energy = h[o + E_STRONG];
        a.weak_energy = h[o + E_WEAK];
        a.E_field_energy = h[o + E_ELECTRIC];
        a.B_field_energy = h[o + E_MAGNETIC];
        a.total_poynting = {h[o + E_POYNTING_X], h[o + E_POYNTING_Y],
                            h[o + E_POYNTING_Z]};
        a.gauss_violation = h[o + E_GAUSS_SUM];
        a.max_gauss_error = h[o + E_GAUSS_MAX];
        a.coulomb_pe = h[o + E_COULOMB_PE];
        a.particle_energy = a.particle_rest_energy + a.particle_ke;
        a.dynamic_energy = a.field_energy + a.wave_energy + a.particle_ke;
        a.total_energy = a.field_energy + a.wave_energy + a.particle_energy;
        out.audit_meta = meta;
    }

    if (request.groups & TELEMETRY_GRAVITY) {
        const int o = T_GRAVITY_BASE;
        auto& a = out.gravity;
        a.latency_max = h[o + G_LATENCY_MAX];
        a.voxel_count = static_cast<int>(llround(h[o + G_VOXEL_COUNT]));
        a.gamma_max = a.voxel_count > 0 ? h[o + G_GAMMA_MAX] : 1.0;
        if (a.voxel_count > 0) {
            a.latency_mean = h[o + G_LATENCY_SUM]
                           / static_cast<double>(a.voxel_count);
            a.f_min = 1.0 - a.latency_max * a.latency_max;
            a.dilation_max_pct = (1.0 - sqrt(fmax(0.0, a.f_min))) * 100.0;
        }
        a.requested = gravity_requested;
        a.active = a.requested && a.voxel_count > 0;
        out.gravity_meta = meta;
    }

    if (request.groups & TELEMETRY_LAGRANGIAN) {
        const int o = T_LAGRANGIAN_BASE;
        auto& d = out.lagrangian;
        d.field_kinetic_sum = h[o + L_FIELD_KINETIC];
        d.field_gradient_sum = h[o + L_FIELD_GRADIENT];
        d.born_infeld_sum = h[o + L_BORN_INFELD];
        d.coupling_sum = h[o + L_COUPLING];
        d.velocity_coupling_sum = h[o + L_VELOCITY_COUPLING];
        d.gauss_sum = h[o + L_GAUSS];
        d.dissipation_sum = h[o + L_DISSIPATION];
        d.total_lagrangian = h[o + L_TOTAL];
        d.total_hamiltonian = h[o + L_HAMILTONIAN];
        d.total_action = d.total_lagrangian;
        d.gauss_violation = h[o + L_GAUSS_VIOLATION];
        d.max_gauss_error = h[o + L_GAUSS_MAX];
        d.total_flux_mag = h[o + L_TOTAL_FLUX];
        d.total_wave_energy = h[o + L_TOTAL_WAVE];
        d.manifested_count = static_cast<int>(llround(h[o + L_MANIFESTED]));
        d.locked_count = static_cast<int>(llround(h[o + L_LOCKED]));
        out.lagrangian_meta = meta;
    }
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
