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

// ──────────────────────────────────────────────────────────────────────────
// Shared per-site sample, accumulators, and slot emitters
// ──────────────────────────────────────────────────────────────────────────
// The four legacy split kernels and the fused telemetry kernel perform the
// same per-site arithmetic; only the traversal they share and the slot base
// they write to differ.  accumulate_*() and emit_*() below are the single
// definition of that arithmetic, so a kernel only decides which sections run.
// Each accumulate_*() documents the SiteSample members it reads, which lets a
// caller populate exactly the fields it needs and keep its own load set.

struct SiteSample {
    int index;
    int x, y, z;
    int state;
    double fx, fy, fz;
    double wx, wy, wz;
    double vx, vy, vz;
    double flux2, wave2, speed2;
    double latency;
};

/// Site identity: linear index, lattice coordinates, manifestation state.
__device__ __forceinline__ SiteSample site_at(const DiagnosticView& b, int i) {
    SiteSample s{};
    s.index = i;
    coordinates(i, b.L, s.x, s.y, s.z);
    s.state = static_cast<int>(b.d_state[i]);
    return s;
}

__device__ __forceinline__ void load_flux(const DiagnosticView& b,
                                          SiteSample& s) {
    s.fx = b.d_flux_x[s.index];
    s.fy = b.d_flux_y[s.index];
    s.fz = b.d_flux_z[s.index];
    s.flux2 = s.fx * s.fx + s.fy * s.fy + s.fz * s.fz;
}

__device__ __forceinline__ void load_wave(const DiagnosticView& b,
                                          SiteSample& s) {
    s.wx = b.d_wave_vel_x[s.index];
    s.wy = b.d_wave_vel_y[s.index];
    s.wz = b.d_wave_vel_z[s.index];
    s.wave2 = s.wx * s.wx + s.wy * s.wy + s.wz * s.wz;
}

__device__ __forceinline__ void load_velocity(const DiagnosticView& b,
                                              SiteSample& s) {
    s.vx = b.d_velocity_x[s.index];
    s.vy = b.d_velocity_y[s.index];
    s.vz = b.d_velocity_z[s.index];
    s.speed2 = s.vx * s.vx + s.vy * s.vy + s.vz * s.vz;
}

__device__ __forceinline__ void load_latency(const DiagnosticView& b,
                                             SiteSample& s) {
    s.latency = b.d_latency[s.index];
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

struct DiagnosticAccum {
    double total_flux, bi_abs, max_bandwidth, max_budget;
    double manifested, positive, negative, spin_up, spin_down;
    double color0, color1, color2, color3;
    double rho2_sum, rho2_log_sum;
    double coord_x, coord_y, coord_z;
    double vel_x, vel_y, vel_z;
    double rxv_x, rxv_y, rxv_z;
};

/// Reads s.{index, x, y, z, state, flux2, speed2, latency, vx, vy, vz}.
__device__ __forceinline__ void accumulate_diagnostics(DiagnosticAccum& a,
                                                       const DiagnosticView& b,
                                                       const SiteSample& s) {
    a.total_flux += sqrt(s.flux2);
    a.bi_abs += fabs(born_infeld_core(s.latency, s.speed2));
    a.max_bandwidth = fmax(a.max_bandwidth,
                           bandwidth_fraction(s.latency, s.speed2));
    a.max_budget = fmax(a.max_budget, causal_budget(s.latency, s.speed2));
    a.rho2_sum += s.flux2;
    if (s.flux2 > EPSILON_FLUX_SQ) a.rho2_log_sum += s.flux2 * log(s.flux2);

    if (s.state == 0) return;
    a.manifested += 1.0;
    a.positive += s.state > 0 ? 1.0 : 0.0;
    a.negative += s.state < 0 ? 1.0 : 0.0;
    const int spin = static_cast<int>(b.d_spin[s.index]);
    a.spin_up += spin > 0 ? 1.0 : 0.0;
    a.spin_down += spin < 0 ? 1.0 : 0.0;
    const int color = static_cast<int>(b.d_color[s.index]);
    a.color0 += color == 0 ? 1.0 : 0.0;
    a.color1 += color == 1 ? 1.0 : 0.0;
    a.color2 += color == 2 ? 1.0 : 0.0;
    a.color3 += color == 3 ? 1.0 : 0.0;
    a.coord_x += s.x; a.coord_y += s.y; a.coord_z += s.z;
    a.vel_x += s.vx; a.vel_y += s.vy; a.vel_z += s.vz;
    a.rxv_x += static_cast<double>(s.y) * s.vz - static_cast<double>(s.z) * s.vy;
    a.rxv_y += static_cast<double>(s.z) * s.vx - static_cast<double>(s.x) * s.vz;
    a.rxv_z += static_cast<double>(s.x) * s.vy - static_cast<double>(s.y) * s.vx;
}

/// Warp-collective: every thread of the block must reach this call.
__device__ __forceinline__ void emit_diagnostics(const DiagnosticAccum& a,
                                                 const DiagnosticView& b,
                                                 bool movement, int begin,
                                                 double* out, int base) {
    reduce_sum_to(out, base + D_TOTAL_FLUX, a.total_flux);
    reduce_sum_to(out, base + D_BI_ABS, a.bi_abs);
    reduce_max_to(out, base + D_MAX_BANDWIDTH, a.max_bandwidth);
    reduce_max_to(out, base + D_MAX_BUDGET, a.max_budget);
    reduce_sum_to(out, base + D_MANIFESTED, a.manifested);
    reduce_sum_to(out, base + D_POSITIVE, a.positive);
    reduce_sum_to(out, base + D_NEGATIVE, a.negative);
    reduce_sum_to(out, base + D_SPIN_UP, a.spin_up);
    reduce_sum_to(out, base + D_SPIN_DOWN, a.spin_down);
    reduce_sum_to(out, base + D_COLOR_0, a.color0);
    reduce_sum_to(out, base + D_COLOR_1, a.color1);
    reduce_sum_to(out, base + D_COLOR_2, a.color2);
    reduce_sum_to(out, base + D_COLOR_3, a.color3);
    reduce_sum_to(out, base + D_RHO2_SUM, a.rho2_sum);
    reduce_sum_to(out, base + D_RHO2_LOG_SUM, a.rho2_log_sum);
    reduce_sum_to(out, base + D_COORD_X, a.coord_x);
    reduce_sum_to(out, base + D_COORD_Y, a.coord_y);
    reduce_sum_to(out, base + D_COORD_Z, a.coord_z);
    reduce_sum_to(out, base + D_VEL_X, a.vel_x);
    reduce_sum_to(out, base + D_VEL_Y, a.vel_y);
    reduce_sum_to(out, base + D_VEL_Z, a.vel_z);
    reduce_sum_to(out, base + D_RXV_X, a.rxv_x);
    reduce_sum_to(out, base + D_RXV_Y, a.rxv_y);
    reduce_sum_to(out, base + D_RXV_Z, a.rxv_z);
    const double causal = (movement && begin == 0)
        ? static_cast<double>(*b.d_causal_projection_events) : 0.0;
    reduce_sum_to(out, base + D_CAUSAL_PROJECTIONS, causal);
}

__global__ void compact_diagnostics_kernel(DiagnosticView b, bool movement,
                                            double* out) {
    DiagnosticAccum acc{};
    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        SiteSample s = site_at(b, i);
        load_flux(b, s);
        load_velocity(b, s);
        load_latency(b, s);
        accumulate_diagnostics(acc, b, s);
    }
    emit_diagnostics(acc, b, movement, begin, out, /*base=*/0);
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

struct EnergyAccum {
    double field, wave, particle_ke, particle_rest;
    double momentum_x, momentum_y, momentum_z;
    double manifested, charge;
    double left, right, wave_left, wave_right;
    double chirality, strong, weak;
    double electric, magnetic;
    double poynting_x, poynting_y, poynting_z;
    double gauss_sum, gauss_max, coulomb_pe;
};

/// Reads s.{index, state, flux2, wave2, speed2, wx, wy, wz, vx, vy, vz} plus
/// the site's divergence and curl (bx, by, bz), which the caller supplies.
__device__ __forceinline__ void accumulate_energy(
    EnergyAccum& a, const DiagnosticView& b, const SiteSample& s,
    double div, double bx, double by, double bz,
    double mean_charge, double charge_coupling,
    bool dual_substrate, bool strong_field) {
    constexpr double C2 = C_SPEED * C_SPEED;
    const int i = s.index;

    a.field += quadratic_field_energy_density(s.flux2);
    a.wave += quadratic_field_energy_density(s.wave2);
    a.electric += quadratic_field_energy_density(s.wave2);
    a.magnetic += C2 * quadratic_field_energy_density(bx * bx + by * by + bz * bz);
    // E = -wave_vel; S = c^2 E x B.
    a.poynting_x += C2 * ((-s.wy) * bz - (-s.wz) * by);
    a.poynting_y += C2 * ((-s.wz) * bx - (-s.wx) * bz);
    a.poynting_z += C2 * ((-s.wx) * by - (-s.wy) * bx);

    if (s.state == 0) {
        const double err = div + charge_coupling * mean_charge;
        a.gauss_sum += err * err;
        a.gauss_max = fmax(a.gauss_max, fabs(err));
    } else {
        const double gamma0 = flat_gamma(s.speed2);
        a.particle_ke += flat_particle_kinetic_energy(s.speed2);
        a.particle_rest += E_REST;
        a.momentum_x += s.vx * (gamma0 * M_INERTIAL);
        a.momentum_y += s.vy * (gamma0 * M_INERTIAL);
        a.momentum_z += s.vz * (gamma0 * M_INERTIAL);
        a.manifested += 1.0;
        a.charge += s.state;
        a.coulomb_pe += 0.5 * ALPHA * static_cast<double>(s.state)
                      * b.d_phi_coulomb[i];
    }

    if (dual_substrate) {
        const double flx = b.d_flux_L_x[i], fly = b.d_flux_L_y[i], flz = b.d_flux_L_z[i];
        const double frx = b.d_flux_R_x[i], fry = b.d_flux_R_y[i], frz = b.d_flux_R_z[i];
        const double wlx = b.d_wave_vel_L_x[i], wly = b.d_wave_vel_L_y[i], wlz = b.d_wave_vel_L_z[i];
        const double wrx = b.d_wave_vel_R_x[i], wry = b.d_wave_vel_R_y[i], wrz = b.d_wave_vel_R_z[i];
        a.left += 0.5 * (flx * flx + fly * fly + flz * flz);
        a.right += 0.5 * (frx * frx + fry * fry + frz * frz);
        a.wave_left += 0.5 * (wlx * wlx + wly * wly + wlz * wlz);
        a.wave_right += 0.5 * (wrx * wrx + wry * wry + wrz * wrz);

        if (s.speed2 > 1e-12) {
            const double inv_speed = 1.0 / sqrt(s.speed2);
            const double ldot = (flx * s.vx + fly * s.vy + flz * s.vz) * inv_speed;
            const double rdot = (frx * s.vx + fry * s.vy + frz * s.vz) * inv_speed;
            a.chirality += (flx * flx + fly * fly + flz * flz - ldot * ldot)
                         - (frx * frx + fry * fry + frz * frz - rdot * rdot);
        } else {
            a.chirality += (flx * flx + fly * fly) - (frx * frx + fry * fry);
        }
    }

    if (strong_field) {
        const double sx = b.d_flux_strong_x[i], sy = b.d_flux_strong_y[i], sz = b.d_flux_strong_z[i];
        a.strong += 0.5 * (sx * sx + sy * sy + sz * sz);
    }
    const double ux = b.d_flux_weak_x[i], uy = b.d_flux_weak_y[i], uz = b.d_flux_weak_z[i];
    a.weak += 0.5 * (ux * ux + uy * uy + uz * uz);
}

/// Warp-collective: every thread of the block must reach this call.
__device__ __forceinline__ void emit_energy(const EnergyAccum& a, double* out,
                                            int base) {
    reduce_sum_to(out, base + E_FIELD, a.field);
    reduce_sum_to(out, base + E_WAVE, a.wave);
    reduce_sum_to(out, base + E_PARTICLE_KE, a.particle_ke);
    reduce_sum_to(out, base + E_PARTICLE_REST, a.particle_rest);
    reduce_sum_to(out, base + E_MOMENTUM_X, a.momentum_x);
    reduce_sum_to(out, base + E_MOMENTUM_Y, a.momentum_y);
    reduce_sum_to(out, base + E_MOMENTUM_Z, a.momentum_z);
    reduce_sum_to(out, base + E_MANIFESTED, a.manifested);
    reduce_sum_to(out, base + E_CHARGE, a.charge);
    reduce_sum_to(out, base + E_LEFT, a.left);
    reduce_sum_to(out, base + E_RIGHT, a.right);
    reduce_sum_to(out, base + E_WAVE_LEFT, a.wave_left);
    reduce_sum_to(out, base + E_WAVE_RIGHT, a.wave_right);
    reduce_sum_to(out, base + E_CHIRALITY, a.chirality);
    reduce_sum_to(out, base + E_STRONG, a.strong);
    reduce_sum_to(out, base + E_WEAK, a.weak);
    reduce_sum_to(out, base + E_ELECTRIC, a.electric);
    reduce_sum_to(out, base + E_MAGNETIC, a.magnetic);
    reduce_sum_to(out, base + E_POYNTING_X, a.poynting_x);
    reduce_sum_to(out, base + E_POYNTING_Y, a.poynting_y);
    reduce_sum_to(out, base + E_POYNTING_Z, a.poynting_z);
    reduce_sum_to(out, base + E_GAUSS_SUM, a.gauss_sum);
    reduce_max_to(out, base + E_GAUSS_MAX, a.gauss_max);
    reduce_sum_to(out, base + E_COULOMB_PE, a.coulomb_pe);
}

__global__ void compact_energy_kernel(DiagnosticView b,
                                      bool dual_substrate,
                                      bool strong_field,
                                      double charge_coupling,
                                      const long long* charge_sum,
                                      double* out) {
    EnergyAccum acc{};
    const double mean_charge = static_cast<double>(*charge_sum)
                             / static_cast<double>(b.N);

    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        SiteSample s = site_at(b, i);
        load_flux(b, s);
        load_wave(b, s);
        load_velocity(b, s);
        double div, bx, by, bz;
        divergence_and_curl(b, s.x, s.y, s.z, div, bx, by, bz);
        accumulate_energy(acc, b, s, div, bx, by, bz, mean_charge,
                          charge_coupling, dual_substrate, strong_field);
    }
    emit_energy(acc, out, /*base=*/0);
}

enum GravitySlot : int {
    G_LATENCY_MAX, G_LATENCY_SUM, G_GAMMA_MAX, G_VOXEL_COUNT, G_SLOT_COUNT
};

struct GravityAccum {
    double latency_max, latency_sum, gamma_max, voxel_count;
};

/// Reads s.{latency, speed2}.  The caller applies its own latency guard: the
/// legacy kernel skips `latency <= 0.0` while the telemetry kernel admits
/// `latency > 0.0`, and the two spellings disagree on a NaN latency.  That
/// divergence predates this consolidation, so each call site keeps its own
/// predicate verbatim rather than having one silently adopt the other's.
__device__ __forceinline__ void accumulate_gravity(GravityAccum& a,
                                                   const SiteSample& s) {
    a.latency_max = fmax(a.latency_max, s.latency);
    a.latency_sum += s.latency;
    a.gamma_max = fmax(a.gamma_max, transport_gamma(s.latency, s.speed2));
    a.voxel_count += 1.0;
}

/// Warp-collective: every thread of the block must reach this call.
__device__ __forceinline__ void emit_gravity(const GravityAccum& a, double* out,
                                             int base) {
    reduce_max_to(out, base + G_LATENCY_MAX, a.latency_max);
    reduce_sum_to(out, base + G_LATENCY_SUM, a.latency_sum);
    reduce_max_to(out, base + G_GAMMA_MAX, a.gamma_max);
    reduce_sum_to(out, base + G_VOXEL_COUNT, a.voxel_count);
}

__global__ void compact_gravity_kernel(DiagnosticView b, double* out) {
    GravityAccum acc{};
    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        SiteSample s{};
        s.index = i;
        load_latency(b, s);
        if (s.latency <= 0.0) continue;
        load_velocity(b, s);
        accumulate_gravity(acc, s);
    }
    emit_gravity(acc, out, /*base=*/0);
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

struct LagrangianAccum {
    double fk_sum, fg_sum, bi_sum, coupling_sum, velocity_coupling_sum;
    double gauss_sum, dissipation_sum, total, hamiltonian;
    double violation_sum, violation_max;
    double total_flux, total_wave, manifested, locked;
};

/// Reads s.{index, x, y, z, state, flux2, wave2, speed2, latency, fx, fy, fz,
/// vx, vy, vz} plus the site's divergence, which the caller supplies.
__device__ __forceinline__ void accumulate_lagrangian(LagrangianAccum& a,
                                                      const DiagnosticView& b,
                                                      const SiteSample& s,
                                                      double div) {
    constexpr double C2 = C_SPEED * C_SPEED;
    constexpr double LAMBDA_G_DIAGNOSTIC = 100.0;
    const int i = s.index;
    const int x = s.x, y = s.y, z = s.z;

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

    const double rho = static_cast<double>(s.state);
    const double fk = 0.5 * s.wave2;
    const double fg = -0.25 * C2 * grad_sq;
    const double bi = born_infeld_core(s.latency, s.speed2);
    const double coupling = G_C * rho * div;
    const double velocity_coupling =
        -G_C * rho * (s.vx * s.fx + s.vy * s.fy + s.vz * s.fz);
    const double violation = div - rho;
    const double gauss = -LAMBDA_G_DIAGNOSTIC * violation * violation;
    const double dissipation = 0.5 * DAMPING * s.wave2;

    a.fk_sum += fk; a.fg_sum += fg; a.bi_sum += bi;
    a.coupling_sum += coupling;
    a.velocity_coupling_sum += velocity_coupling;
    a.gauss_sum += gauss;
    a.dissipation_sum += dissipation;
    a.total += fk + fg + bi + coupling + velocity_coupling + gauss;
    a.hamiltonian += born_infeld_hamiltonian(s.latency, s.speed2)
                   - coupling - velocity_coupling - gauss;
    a.violation_sum += violation * violation;
    a.violation_max = fmax(a.violation_max, fabs(violation));
    a.total_flux += sqrt(s.flux2);
    a.total_wave += 0.5 * s.wave2;
    if (s.state != 0) {
        a.manifested += 1.0;
        a.locked += b.d_locked[i] ? 1.0 : 0.0;
    }
}

/// Warp-collective: every thread of the block must reach this call.
__device__ __forceinline__ void emit_lagrangian(const LagrangianAccum& a,
                                                double* out, int base) {
    reduce_sum_to(out, base + L_FIELD_KINETIC, a.fk_sum);
    reduce_sum_to(out, base + L_FIELD_GRADIENT, a.fg_sum);
    reduce_sum_to(out, base + L_BORN_INFELD, a.bi_sum);
    reduce_sum_to(out, base + L_COUPLING, a.coupling_sum);
    reduce_sum_to(out, base + L_VELOCITY_COUPLING, a.velocity_coupling_sum);
    reduce_sum_to(out, base + L_GAUSS, a.gauss_sum);
    reduce_sum_to(out, base + L_DISSIPATION, a.dissipation_sum);
    reduce_sum_to(out, base + L_TOTAL, a.total);
    reduce_sum_to(out, base + L_HAMILTONIAN, a.hamiltonian);
    reduce_sum_to(out, base + L_GAUSS_VIOLATION, a.violation_sum);
    reduce_max_to(out, base + L_GAUSS_MAX, a.violation_max);
    reduce_sum_to(out, base + L_TOTAL_FLUX, a.total_flux);
    reduce_sum_to(out, base + L_TOTAL_WAVE, a.total_wave);
    reduce_sum_to(out, base + L_MANIFESTED, a.manifested);
    reduce_sum_to(out, base + L_LOCKED, a.locked);
}

__global__ void compact_lagrangian_kernel(DiagnosticView b, double* out) {
    LagrangianAccum acc{};
    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        SiteSample s = site_at(b, i);
        load_flux(b, s);
        load_wave(b, s);
        load_velocity(b, s);
        load_latency(b, s);
        double div, unused_x, unused_y, unused_z;
        divergence_and_curl(b, s.x, s.y, s.z, div, unused_x, unused_y, unused_z);
        accumulate_lagrangian(acc, b, s, div);
    }
    emit_lagrangian(acc, out, /*base=*/0);
}

// ──────────────────────────────────────────────────────────────────────────
// Coherent telemetry snapshot reduction
// ──────────────────────────────────────────────────────────────────────────
// Legacy compact getters intentionally keep their own kernels above for API
// compatibility. Native interactive telemetry uses this fused pass instead:
// diagnostics + audit + gravity share every field load and traverse N sites
// once. The optional Lagrangian section remains conditional because its
// 18-point stencil is materially more expensive than the dashboard summary.
// Both entry points run the same accumulate_*()/emit_*() helpers, so the two
// paths cannot drift; only the slot base differs.

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
    DiagnosticAccum diagnostics{};
    EnergyAccum energy{};
    GravityAccum gravity{};
    LagrangianAccum lagrangian{};

    const double mean_charge = want_audit
        ? static_cast<double>(*charge_sum) / static_cast<double>(b.N) : 0.0;

    const int begin = blockIdx.x * blockDim.x + threadIdx.x;
    const int stride = blockDim.x * gridDim.x;
    for (int i = begin; i < b.N; i += stride) {
        SiteSample s = site_at(b, i);
        load_flux(b, s);
        load_wave(b, s);
        load_velocity(b, s);
        load_latency(b, s);

        double div = 0.0, curl_x = 0.0, curl_y = 0.0, curl_z = 0.0;
        if (want_audit || want_lagrangian) {
            divergence_and_curl(b, s.x, s.y, s.z, div, curl_x, curl_y, curl_z);
        }

        if (want_diagnostics) accumulate_diagnostics(diagnostics, b, s);
        if (want_audit) {
            accumulate_energy(energy, b, s, div, curl_x, curl_y, curl_z,
                              mean_charge, charge_coupling, dual_substrate,
                              strong_field);
        }
        if (want_gravity && s.latency > 0.0) accumulate_gravity(gravity, s);
        if (want_lagrangian) accumulate_lagrangian(lagrangian, b, s, div);
    }

    // The want_* flags are launch-uniform, so every thread of a warp reaches
    // the same emit_*() calls -- a precondition of the warp-collective
    // reductions inside them.
    if (want_diagnostics) {
        emit_diagnostics(diagnostics, b, movement, begin, out,
                         T_DIAGNOSTIC_BASE);
    }
    if (want_audit) emit_energy(energy, out, T_ENERGY_BASE);
    if (want_gravity) emit_gravity(gravity, out, T_GRAVITY_BASE);
    if (want_lagrangian) emit_lagrangian(lagrangian, out, T_LAGRANGIAN_BASE);
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

// ──────────────────────────────────────────────────────────────────────────
// Host-side slot decoding
// ──────────────────────────────────────────────────────────────────────────
// The legacy per-group getters and the fused telemetry snapshot read the same
// slot layout out of different buffers; `base` is the group's offset, zero for
// the legacy kernels whose scratch holds one group at a time.

void decode_diagnostics(const double* h, int base, int tick, Diagnostics& d) {
    d = Diagnostics{};
    d.tick = tick;
    d.total_flux = h[base + D_TOTAL_FLUX];
    d.total_energy = h[base + D_BI_ABS];
    d.max_bandwidth = h[base + D_MAX_BANDWIDTH];
    d.max_causal_budget = h[base + D_MAX_BUDGET];
    d.manifested_count = static_cast<int>(llround(h[base + D_MANIFESTED]));
    d.positive_count = static_cast<int>(llround(h[base + D_POSITIVE]));
    d.negative_count = static_cast<int>(llround(h[base + D_NEGATIVE]));
    d.spin_up_count = static_cast<int>(llround(h[base + D_SPIN_UP]));
    d.spin_down_count = static_cast<int>(llround(h[base + D_SPIN_DOWN]));
    d.color_count[0] = static_cast<int>(llround(h[base + D_COLOR_0]));
    d.color_count[1] = static_cast<int>(llround(h[base + D_COLOR_1]));
    d.color_count[2] = static_cast<int>(llround(h[base + D_COLOR_2]));
    d.color_count[3] = static_cast<int>(llround(h[base + D_COLOR_3]));
    const double rho2 = h[base + D_RHO2_SUM];
    if (rho2 >= EPSILON_FLUX_SQ)
        d.total_entropy = log(rho2) - h[base + D_RHO2_LOG_SUM] / rho2;
    d.causal_projection_events =
        static_cast<long long>(llround(h[base + D_CAUSAL_PROJECTIONS]));

    if (d.manifested_count > 0) {
        const double inv_n = 1.0 / static_cast<double>(d.manifested_count);
        const double cx = h[base + D_COORD_X] * inv_n;
        const double cy = h[base + D_COORD_Y] * inv_n;
        const double cz = h[base + D_COORD_Z] * inv_n;
        d.total_angular_momentum.x = h[base + D_RXV_X]
            - (cy * h[base + D_VEL_Z] - cz * h[base + D_VEL_Y]);
        d.total_angular_momentum.y = h[base + D_RXV_Y]
            - (cz * h[base + D_VEL_X] - cx * h[base + D_VEL_Z]);
        d.total_angular_momentum.z = h[base + D_RXV_Z]
            - (cx * h[base + D_VEL_Y] - cy * h[base + D_VEL_X]);
    }
}

void decode_energy_audit(const double* h, int base, EnergyAudit& a) {
    a = EnergyAudit{};
    a.field_energy = h[base + E_FIELD];
    a.wave_energy = h[base + E_WAVE];
    a.field_energy_density_sum = h[base + E_FIELD];
    a.wave_energy_density_sum = h[base + E_WAVE];
    a.particle_ke = h[base + E_PARTICLE_KE];
    a.particle_rest_energy = h[base + E_PARTICLE_REST];
    a.particle_momentum = {h[base + E_MOMENTUM_X], h[base + E_MOMENTUM_Y],
                           h[base + E_MOMENTUM_Z]};
    a.manifested_count = static_cast<int>(llround(h[base + E_MANIFESTED]));
    a.charge_total = static_cast<int>(llround(h[base + E_CHARGE]));
    a.E_L_total = h[base + E_LEFT];
    a.E_R_total = h[base + E_RIGHT];
    a.wv_L_total = h[base + E_WAVE_LEFT];
    a.wv_R_total = h[base + E_WAVE_RIGHT];
    a.chirality_total = h[base + E_CHIRALITY];
    a.strong_energy = h[base + E_STRONG];
    a.weak_energy = h[base + E_WEAK];
    a.E_field_energy = h[base + E_ELECTRIC];
    a.B_field_energy = h[base + E_MAGNETIC];
    a.total_poynting = {h[base + E_POYNTING_X], h[base + E_POYNTING_Y],
                        h[base + E_POYNTING_Z]};
    a.gauss_violation = h[base + E_GAUSS_SUM];
    a.max_gauss_error = h[base + E_GAUSS_MAX];
    a.coulomb_pe = h[base + E_COULOMB_PE];
    a.particle_energy = a.particle_rest_energy + a.particle_ke;
    a.dynamic_energy = a.field_energy + a.wave_energy + a.particle_ke;
    a.total_energy = a.field_energy + a.wave_energy + a.particle_energy;
}

void decode_gravity_metric(const double* h, int base, bool requested,
                           GravityMetricAgg& a) {
    a = GravityMetricAgg{};
    a.latency_max = h[base + G_LATENCY_MAX];
    a.voxel_count = static_cast<int>(llround(h[base + G_VOXEL_COUNT]));
    a.gamma_max = a.voxel_count > 0 ? h[base + G_GAMMA_MAX] : 1.0;
    if (a.voxel_count > 0) {
        a.latency_mean = h[base + G_LATENCY_SUM]
                       / static_cast<double>(a.voxel_count);
        a.f_min = 1.0 - a.latency_max * a.latency_max;
        a.dilation_max_pct = (1.0 - sqrt(fmax(0.0, a.f_min))) * 100.0;
    }
    a.requested = requested;
    a.active = a.requested && a.voxel_count > 0;
}

/// `Lagrangian` is LagrangianDiag for the legacy getter and the POD
/// TelemetryLagrangian for the snapshot; both carry the same slot fields.
template <class Lagrangian>
void decode_lagrangian(const double* h, int base, Lagrangian& d) {
    d = Lagrangian{};
    d.field_kinetic_sum = h[base + L_FIELD_KINETIC];
    d.field_gradient_sum = h[base + L_FIELD_GRADIENT];
    d.born_infeld_sum = h[base + L_BORN_INFELD];
    d.coupling_sum = h[base + L_COUPLING];
    d.velocity_coupling_sum = h[base + L_VELOCITY_COUPLING];
    d.gauss_sum = h[base + L_GAUSS];
    d.dissipation_sum = h[base + L_DISSIPATION];
    d.total_lagrangian = h[base + L_TOTAL];
    d.total_hamiltonian = h[base + L_HAMILTONIAN];
    d.total_action = d.total_lagrangian;
    d.gauss_violation = h[base + L_GAUSS_VIOLATION];
    d.max_gauss_error = h[base + L_GAUSS_MAX];
    d.total_flux_mag = h[base + L_TOTAL_FLUX];
    d.total_wave_energy = h[base + L_TOTAL_WAVE];
    d.manifested_count = static_cast<int>(llround(h[base + L_MANIFESTED]));
    d.locked_count = static_cast<int>(llround(h[base + L_LOCKED]));
}

}  // namespace

void launch_compact_diagnostics(GpuBuffers& b, int tick, bool movement,
                                Diagnostics& d) {
    clear_result<D_COUNT>(b);
    compact_diagnostics_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), movement, b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<D_COUNT>(b);
    decode_diagnostics(h.data(), /*base=*/0, tick, d);
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
    decode_energy_audit(h.data(), /*base=*/0, a);
}

void launch_compact_gravity_metric(GpuBuffers& b, const TermToggles& toggles,
                                   GravityMetricAgg& a) {
    clear_result<G_SLOT_COUNT>(b);
    compact_gravity_kernel<<<reduction_grid(b.N), THREADS>>>(
        diagnostic_view(b), b.d_compact_diagnostics);
    CUDA_CHECK(cudaGetLastError());
    const auto h = download_result<G_SLOT_COUNT>(b);
    decode_gravity_metric(h.data(), /*base=*/0,
                          toggles.latency_field || toggles.field_energy_gravity,
                          a);
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
    decode_lagrangian(h.data(), /*base=*/0, d);
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
        decode_diagnostics(h, T_DIAGNOSTIC_BASE, tick, out.diagnostics);
        out.diagnostics_meta = meta;
    }
    if (request.groups & TELEMETRY_AUDIT) {
        decode_energy_audit(h, T_ENERGY_BASE, out.audit);
        out.audit_meta = meta;
    }
    if (request.groups & TELEMETRY_GRAVITY) {
        decode_gravity_metric(h, T_GRAVITY_BASE, gravity_requested,
                              out.gravity);
        out.gravity_meta = meta;
    }
    if (request.groups & TELEMETRY_LAGRANGIAN) {
        decode_lagrangian(h, T_LAGRANGIAN_BASE, out.lagrangian);
        out.lagrangian_meta = meta;
    }
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
