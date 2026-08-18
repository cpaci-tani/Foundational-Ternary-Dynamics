/**
 * @file kernels_forces.cu
 * @brief GPU kernels for Phase 4 (Forces) and Phase 5 (Movement).
 *
 * [EXTENDED] Forces: Coulomb (from Poisson potential), gravity (density gradient),
 * Lorentz (v × B where B = curl(J)). 
 * Movement: remainder accumulation, speed clamping, collision detection.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "ftd/constants_shared.h"
#include "ftd/causal_kinematics.h"
#include "../cuda/cuda_index.cuh"   // ftd::wrap, ftd::idx3d, ftd::decode_xyz, ftd::periodic_delta
#include <cuda_runtime.h>
#include <cub/device/device_select.cuh>
#include <thrust/iterator/counting_iterator.h>
#include <cmath>
#include <cstdio>   // fprintf — Linux/clang stricter than MSVC
#include <cstdlib>  // exit

#include "cuda_error.cuh"  // CUDA_CHECK (revision C1 consolidation)

namespace ftd {
namespace gpu {
namespace kernels {

// ---------- Device helpers ----------
// F-8 (2026-04-27): shared shims onto ::ftd helpers in
// engine/cuda/cuda_index.cuh. Local _d-suffixed names are preserved so
// existing call sites in this TU compile unchanged.

__device__ __forceinline__
int wrap_d(int x, int L) {
    return ::ftd::wrap(x, L);
}

__device__ __forceinline__
int idx3d_d(int x, int y, int z, int L) {
    return ::ftd::idx3d(x, y, z, L);
}

// X-major flat-index decomposition: i = ix*L*L + iy*L + iz.
__device__ __forceinline__
void decode_xyz_d(int idx, int L, int& ix, int& iy, int& iz) {
    ::ftd::decode_xyz(idx, L, ix, iy, iz);
}

// Shortest-path delta on a periodic L^3 lattice. Outputs dx/dy/dz in
// (-L/2, L/2]. Replaces the 3-line wrap pattern that previously appeared
// 4× in this file (color, yukawa, exchange, triad) — single point of
// correctness for periodic geometry.
__device__ __forceinline__
void periodic_delta_d(int ix, int iy, int iz,
                      int jx, int jy, int jz,
                      int L,
                      int& dx, int& dy, int& dz) {
    dx = ::ftd::periodic_delta(jx, ix, L);
    dy = ::ftd::periodic_delta(jy, iy, L);
    dz = ::ftd::periodic_delta(jz, iz, L);
}

// atomicCAS_byte now lives in cuda_index.cuh (revision C3, ADR-0007).

__device__ __forceinline__
void ledger_add_face_current(double* current_x,
                             double* current_y,
                             double* current_z,
                             int L,
                             int x, int y, int z,
                             int axis, int dir, int charge) {
    if (dir == 0) return;
    if (axis == 0) {
        if (dir > 0) {
            atomicAdd(&current_x[idx3d_d(x, y, z, L)], static_cast<double>(charge));
        } else {
            atomicAdd(&current_x[idx3d_d(x - 1, y, z, L)], -static_cast<double>(charge));
        }
    } else if (axis == 1) {
        if (dir > 0) {
            atomicAdd(&current_y[idx3d_d(x, y, z, L)], static_cast<double>(charge));
        } else {
            atomicAdd(&current_y[idx3d_d(x, y - 1, z, L)], -static_cast<double>(charge));
        }
    } else {
        if (dir > 0) {
            atomicAdd(&current_z[idx3d_d(x, y, z, L)], static_cast<double>(charge));
        } else {
            atomicAdd(&current_z[idx3d_d(x, y, z - 1, L)], -static_cast<double>(charge));
        }
    }
}

__device__ __forceinline__
void ledger_route_moore_current(double* current_x,
                                double* current_y,
                                double* current_z,
                                int L,
                                int x, int y, int z,
                                int dx, int dy, int dz,
                                int charge) {
    ledger_add_face_current(current_x, current_y, current_z, L,
                            x, y, z, 0, dx, charge);
    x = wrap_d(x + dx, L);
    ledger_add_face_current(current_x, current_y, current_z, L,
                            x, y, z, 1, dy, charge);
    y = wrap_d(y + dy, L);
    ledger_add_face_current(current_x, current_y, current_z, L,
                            x, y, z, 2, dz, charge);
}

// ---------- Force Kernel ----------
// Computes forces on all manifested particles and updates velocity

__global__ void phase_forces_kernel(
    const int8_t* __restrict__ state,
    const uint8_t* __restrict__ locked,
    const double* __restrict__ phi_coulomb,
    const double* __restrict__ latency,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    double* __restrict__ accel_mag,
    // Force-diag SoA mirrors (each may be nullptr when diagnostics disabled).
    // Each thread writes its own site once, so plain stores suffice — no
    // atomics needed.
    double* __restrict__ fd_coulomb_x,
    double* __restrict__ fd_coulomb_y,
    double* __restrict__ fd_coulomb_z,
    double* __restrict__ fd_gravity_x,
    double* __restrict__ fd_gravity_y,
    double* __restrict__ fd_gravity_z,
    double* __restrict__ fd_magnetic_x,
    double* __restrict__ fd_magnetic_y,
    double* __restrict__ fd_magnetic_z,
    bool poisson_coulomb,
    bool emergent_forces,
    bool gravity,
    bool lorentz_force,
    double dt,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] == 0) return;  // Only manifested particles

    double s = static_cast<double>(state[i]);
    double fx = 0.0, fy = 0.0, fz = 0.0;
    // Per-component accumulators for force_diag mirror (parity with CPU
    // RenderBridge::phase_forces, which records f_coulomb / f_gravity /
    // f_magnetic separately even though only the sum drives velocity).
    double f_em_x = 0.0,    f_em_y = 0.0,    f_em_z = 0.0;
    double f_grav_x = 0.0,  f_grav_y = 0.0,  f_grav_z = 0.0;
    double f_mag_x = 0.0,   f_mag_y = 0.0,   f_mag_z = 0.0;

    // Neighbor indices
    int xp = idx3d_d(x+1,y,z,L), xm = idx3d_d(x-1,y,z,L);
    int yp = idx3d_d(x,y+1,z,L), ym = idx3d_d(x,y-1,z,L);
    int zp = idx3d_d(x,y,z+1,L), zm = idx3d_d(x,y,z-1,L);

    // --- EM force: emergent / poisson / legacy gradient ---
    // BH-F12 (2026-05-05): emergent_forces mode ported from CPU
    // phase_forces.cpp:80-99. Mutually exclusive with poisson_coulomb
    // (toggles.validate() rejects both true). Branch order matches CPU.
    if (emergent_forces) {
        // EFT mode: F = G_C * state * grad_t2(|J|). Tier-2 stencil reads
        // r=2 face-neighbours to avoid self-field contamination from the
        // particle's own r=1 wake. Coupling is G_C (one vertex coupling;
        // the other G_C is already embedded in the wave-equation flux).
        // alpha = G_C^2 emerges from the two-vertex Lagrangian.
        int x2p = idx3d_d(x+2,y,z,L), x2m = idx3d_d(x-2,y,z,L);
        int y2p = idx3d_d(x,y+2,z,L), y2m = idx3d_d(x,y-2,z,L);
        int z2p = idx3d_d(x,y,z+2,L), z2m = idx3d_d(x,y,z-2,L);
        auto density = [&](int j) -> double {
            return sqrt(flux_x[j]*flux_x[j] + flux_y[j]*flux_y[j] + flux_z[j]*flux_z[j]);
        };
        double grad_x = GRAD_TIER2_SCALE * (density(x2p) - density(x2m));
        double grad_y = GRAD_TIER2_SCALE * (density(y2p) - density(y2m));
        double grad_z = GRAD_TIER2_SCALE * (density(z2p) - density(z2m));
        f_em_x = G_C * s * grad_x;
        f_em_y = G_C * s * grad_y;
        f_em_z = G_C * s * grad_z;
        fx += f_em_x; fy += f_em_y; fz += f_em_z;
    } else if (poisson_coulomb) {
        // Poisson mode: F = -alpha * s * gradient(phi_coulomb)
        double grad_phi_x = GRAD_TIER1_SCALE * (phi_coulomb[xp] - phi_coulomb[xm]);
        double grad_phi_y = GRAD_TIER1_SCALE * (phi_coulomb[yp] - phi_coulomb[ym]);
        double grad_phi_z = GRAD_TIER1_SCALE * (phi_coulomb[zp] - phi_coulomb[zm]);

        f_em_x = -ALPHA * s * grad_phi_x;
        f_em_y = -ALPHA * s * grad_phi_y;
        f_em_z = -ALPHA * s * grad_phi_z;
        fx += f_em_x; fy += f_em_y; fz += f_em_z;
    } else {
        // Legacy mode: F = -alpha * s * gradient(div(J))
        // Compute divergence at each face neighbor, then take gradient
        auto div_J = [&](int j) -> double {
            int jx = j / (L * L), jy = (j / L) % L, jz = j % L;  // X-major decomposition
            int jp_x = idx3d_d(jx+1,jy,jz,L), jm_x = idx3d_d(jx-1,jy,jz,L);
            int jp_y = idx3d_d(jx,jy+1,jz,L), jm_y = idx3d_d(jx,jy-1,jz,L);
            int jp_z = idx3d_d(jx,jy,jz+1,L), jm_z = idx3d_d(jx,jy,jz-1,L);
            return GRAD_TIER1_SCALE * ((flux_x[jp_x] - flux_x[jm_x])
                        + (flux_y[jp_y] - flux_y[jm_y])
                        + (flux_z[jp_z] - flux_z[jm_z]));
        };
        double grad_div_x = GRAD_TIER1_SCALE * (div_J(xp) - div_J(xm));
        double grad_div_y = GRAD_TIER1_SCALE * (div_J(yp) - div_J(ym));
        double grad_div_z = GRAD_TIER1_SCALE * (div_J(zp) - div_J(zm));

        f_em_x = -ALPHA * s * grad_div_x;
        f_em_y = -ALPHA * s * grad_div_y;
        f_em_z = -ALPHA * s * grad_div_z;
        fx += f_em_x; fy += f_em_y; fz += f_em_z;
    }

    // --- Gravity: F = G_N * gradient(density) using tier-2 stencil ---
    if (gravity) {
        // Tier-2 gradient: use r=2 neighbors to avoid self-field contamination
        int x2p = idx3d_d(x+2,y,z,L), x2m = idx3d_d(x-2,y,z,L);
        int y2p = idx3d_d(x,y+2,z,L), y2m = idx3d_d(x,y-2,z,L);
        int z2p = idx3d_d(x,y,z+2,L), z2m = idx3d_d(x,y,z-2,L);

        auto density = [&](int j) -> double {
            return sqrt(flux_x[j]*flux_x[j] + flux_y[j]*flux_y[j] + flux_z[j]*flux_z[j]);
        };

        double gx = GRAD_TIER2_SCALE * (density(x2p) - density(x2m));
        double gy = GRAD_TIER2_SCALE * (density(y2p) - density(y2m));
        double gz = GRAD_TIER2_SCALE * (density(z2p) - density(z2m));

        f_grav_x = G_N * gx;
        f_grav_y = G_N * gy;
        f_grav_z = G_N * gz;
        fx += f_grav_x; fy += f_grav_y; fz += f_grav_z;
    }

    // --- Lorentz force: F = alpha * s * (v × B) where B = curl(J) ---
    if (lorentz_force) {
        double vx = vel_x[i], vy = vel_y[i], vz = vel_z[i];
        double speed = sqrt(vx*vx + vy*vy + vz*vz);

        if (speed > 1e-15) {
            // B = curl(J)
            double Bx = GRAD_TIER1_SCALE * ((flux_z[yp] - flux_z[ym]) - (flux_y[zp] - flux_y[zm]));
            double By = GRAD_TIER1_SCALE * ((flux_x[zp] - flux_x[zm]) - (flux_z[xp] - flux_z[xm]));
            double Bz = GRAD_TIER1_SCALE * ((flux_y[xp] - flux_y[xm]) - (flux_x[yp] - flux_x[ym]));

            // v × B
            double cross_x = vy * Bz - vz * By;
            double cross_y = vz * Bx - vx * Bz;
            double cross_z = vx * By - vy * Bx;

            f_mag_x = ALPHA * s * cross_x;
            f_mag_y = ALPHA * s * cross_y;
            f_mag_z = ALPHA * s * cross_z;
            fx += f_mag_x; fy += f_mag_y; fz += f_mag_z;
        }
    }

    // --- Mirror per-component forces into force_diag SoA (matches CPU
    // RenderBridge::phase_forces, which writes force_diag_[i].f_* before
    // applying the velocity update). f_strong is populated by
    // color_force_kernel; f_exchange stays at the reset_force_diag()
    // zero baseline (CPU explicitly assigns f_exchange = {}).
    if (fd_coulomb_x) {
        fd_coulomb_x[i] = f_em_x;
        fd_coulomb_y[i] = f_em_y;
        fd_coulomb_z[i] = f_em_z;
    }
    if (fd_gravity_x) {
        fd_gravity_x[i] = f_grav_x;
        fd_gravity_y[i] = f_grav_y;
        fd_gravity_z[i] = f_grav_z;
    }
    if (fd_magnetic_x) {
        fd_magnetic_x[i] = f_mag_x;
        fd_magnetic_y[i] = f_mag_y;
        fd_magnetic_z[i] = f_mag_z;
    }

    // BH-F3 (2026-05-05): record raw force magnitude (EM + grav + Lorentz)
    // BEFORE the velocity update / clamp. Matches CPU phase_forces.cpp:195.
    // Pre-fix this used post-clamp |dv|/dt which underestimated accel at the
    // bandwidth edge and silently excluded color force on both backends.
    // Color force is computed in color_force_kernel and intentionally NOT
    // reflected here — Larmor radiation (the only consumer) is electromagnetic,
    // so color shouldn't contribute. accel_mag is now bit-exact CPU↔GPU under
    // unit mass at the same call site.
    accel_mag[i] = sqrt(fx*fx + fy*fy + fz*fz);

    // FTD-0402: this kernel only accumulates force components. A single
    // integrate_forces_kernel runs after base, color, Yukawa, and exchange
    // contributions, so no force path can bypass the causal budget.
}

__global__ void integrate_forces_kernel(
    const int8_t* __restrict__ state,
    const uint8_t* __restrict__ locked,
    const double* __restrict__ latency,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    const double* __restrict__ fd_coulomb_x,
    const double* __restrict__ fd_coulomb_y,
    const double* __restrict__ fd_coulomb_z,
    const double* __restrict__ fd_gravity_x,
    const double* __restrict__ fd_gravity_y,
    const double* __restrict__ fd_gravity_z,
    const double* __restrict__ fd_magnetic_x,
    const double* __restrict__ fd_magnetic_y,
    const double* __restrict__ fd_magnetic_z,
    const double* __restrict__ fd_strong_x,
    const double* __restrict__ fd_strong_y,
    const double* __restrict__ fd_strong_z,
    const double* __restrict__ fd_exchange_x,
    const double* __restrict__ fd_exchange_y,
    const double* __restrict__ fd_exchange_z,
    double dt,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N || state[i] == 0 || locked[i]) return;

    const double fx = fd_coulomb_x[i] + fd_gravity_x[i]
                    + fd_magnetic_x[i] + fd_strong_x[i] + fd_exchange_x[i];
    const double fy = fd_coulomb_y[i] + fd_gravity_y[i]
                    + fd_magnetic_y[i] + fd_strong_y[i] + fd_exchange_y[i];
    const double fz = fd_coulomb_z[i] + fd_gravity_z[i]
                    + fd_magnetic_z[i] + fd_strong_z[i] + fd_exchange_z[i];

    const double vx = vel_x[i], vy = vel_y[i], vz = vel_z[i];
    const double gamma_in = ::ftd::momentum_input_gamma(
        latency[i], vx*vx + vy*vy + vz*vz);
    const double force_scale = dt / M_INERTIAL;
    const double qx = vx * gamma_in + fx * force_scale;
    const double qy = vy * gamma_in + fy * force_scale;
    const double qz = vz * gamma_in + fz * force_scale;
    const double scale = ::ftd::specific_momentum_velocity_scale(
        latency[i], qx*qx + qy*qy + qz*qz);
    if (scale > 0.0) {
        vel_x[i] = qx * scale;
        vel_y[i] = qy * scale;
        vel_z[i] = qz * scale;
    } else {
        vel_x[i] = 0.0;
        vel_y[i] = 0.0;
        vel_z[i] = 0.0;
    }
}

// ---------- Movement transaction ----------
//
// CPU movement is a greedy ascending X-major transaction: every committed
// source mutates state that later sources must observe.  A thread-per-site
// CUDA kernel cannot preserve that order -- target CAS only serializes one
// byte, while metadata, flux transport, annihilation scatter, and the source
// clear remain mutually racy.  It also lets a newly arrived particle execute
// again when its target thread has not yet run.
//
// The common case is sub-cell drift with no integer hop. A parallel prepass
// classifies original candidates and crossing sites, resets moved[], and
// reduces one crossing byte per contiguous 256-site block. A compact second
// kernel applies all non-crossing drift in parallel. The final one-thread
// kernel scans only flagged blocks and commits crossing sources in ascending
// index order. Keeping the large collision body out of the parallel kernel is
// important: otherwise NVCC provisions its local state for every lane. This
// removes unconditional CUB work while retaining the exact greedy CPU order.

constexpr uint8_t MOVEMENT_CANDIDATE = 0x1;
constexpr uint8_t MOVEMENT_CROSSING  = 0x2;
constexpr uint8_t MOVEMENT_PROJECTED = 0x4;
constexpr int MOVEMENT_BLOCK_SIZE = 256;

struct PreparedMovement {
    double vx, vy, vz;
    double rx, ry, rz;
    int dx, dy, dz;
    bool projected;
};

__device__ __forceinline__ PreparedMovement prepare_movement(
    double vx, double vy, double vz,
    double rx, double ry, double rz,
    double latency, double dt) {
    PreparedMovement p{vx, vy, vz, rx, ry, rz, 0, 0, 0, false};
    const double speed2 = vx*vx + vy*vy + vz*vz;
    const double projection =
        ::ftd::movement_projection_scale(latency, speed2);
    if (projection < 1.0) {
        p.projected = true;
        if (projection > 0.0) {
            p.vx *= projection;
            p.vy *= projection;
            p.vz *= projection;
        } else {
            p.vx = 0.0;
            p.vy = 0.0;
            p.vz = 0.0;
        }
    }

    p.rx += p.vx * dt;
    p.ry += p.vy * dt;
    p.rz += p.vz * dt;
    if (p.rx >= 1.0) { p.dx = 1; p.rx -= 1.0; }
    else if (p.rx <= -1.0) { p.dx = -1; p.rx += 1.0; }
    if (p.ry >= 1.0) { p.dy = 1; p.ry -= 1.0; }
    else if (p.ry <= -1.0) { p.dy = -1; p.ry += 1.0; }
    if (p.rz >= 1.0) { p.dz = 1; p.rz -= 1.0; }
    else if (p.rz <= -1.0) { p.dz = -1; p.rz += 1.0; }
    return p;
}

__global__ void movement_prepass_kernel(
    const int8_t* __restrict__ state,
    const uint8_t* __restrict__ locked,
    const double* __restrict__ vel_x,
    const double* __restrict__ vel_y,
    const double* __restrict__ vel_z,
    const double* __restrict__ rem_x,
    const double* __restrict__ rem_y,
    const double* __restrict__ rem_z,
    const double* __restrict__ latency,
    uint8_t* __restrict__ site_flags,
    uint8_t* __restrict__ moved,
    uint8_t* __restrict__ block_has_crossing,
    double dt,
    int N
) {
    __shared__ unsigned int has_crossing;
    if (threadIdx.x == 0) has_crossing = 0;
    __syncthreads();

    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        moved[i] = 0;
        uint8_t flags = 0;
        if (state[i] != 0 && !locked[i]) {
            flags = MOVEMENT_CANDIDATE;
            const PreparedMovement p = prepare_movement(
                vel_x[i], vel_y[i], vel_z[i],
                rem_x[i], rem_y[i], rem_z[i], latency[i], dt);
            if (p.projected) flags |= MOVEMENT_PROJECTED;
            if (p.dx != 0 || p.dy != 0 || p.dz != 0) {
                flags |= MOVEMENT_CROSSING;
                atomicExch(&has_crossing, 1u);
            }
        }
        site_flags[i] = flags;
    }

    __syncthreads();
    if (threadIdx.x == 0)
        block_has_crossing[blockIdx.x] =
            static_cast<uint8_t>(has_crossing != 0);
}

__global__ void apply_non_crossing_movement_kernel(
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    double* __restrict__ rem_x,
    double* __restrict__ rem_y,
    double* __restrict__ rem_z,
    const double* __restrict__ latency,
    const uint8_t* __restrict__ site_flags,
    unsigned long long* __restrict__ causal_projection_events,
    double dt,
    int N
) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    const uint8_t flags = site_flags[i];
    if ((flags & MOVEMENT_CANDIDATE) == 0
        || (flags & MOVEMENT_CROSSING) != 0) return;
    const PreparedMovement p = prepare_movement(
        vel_x[i], vel_y[i], vel_z[i],
        rem_x[i], rem_y[i], rem_z[i], latency[i], dt);
    vel_x[i] = p.vx; vel_y[i] = p.vy; vel_z[i] = p.vz;
    rem_x[i] = p.rx; rem_y[i] = p.ry; rem_z[i] = p.rz;
    if (p.projected) atomicAdd(causal_projection_events, 1ULL);
}

__global__ void phase_movement_commit_crossings_kernel(
    int8_t* __restrict__ state,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    double* __restrict__ rem_x,
    double* __restrict__ rem_y,
    double* __restrict__ rem_z,
    const double* __restrict__ latency,
    double* __restrict__ flux_x,
    double* __restrict__ flux_y,
    double* __restrict__ flux_z,
    const uint8_t* __restrict__ locked,
    int32_t* __restrict__ particle_id,
    int8_t* __restrict__ spin,
    int8_t* __restrict__ color,
    int32_t* __restrict__ pair_id,
    double* __restrict__ accel_mag,
    int* __restrict__ ledger_reaction,
    double* __restrict__ ledger_current_x,
    double* __restrict__ ledger_current_y,
    double* __restrict__ ledger_current_z,
    unsigned long long* __restrict__ causal_projection_events,
    bool dual_substrate,
    double* __restrict__ fL_x,
    double* __restrict__ fL_y,
    double* __restrict__ fL_z,
    double* __restrict__ fR_x,
    double* __restrict__ fR_y,
    double* __restrict__ fR_z,
    const uint8_t* __restrict__ site_flags,
    const uint8_t* __restrict__ block_has_crossing,
    uint8_t* __restrict__ moved,
    double dt,
    int L,
    int N,
    int movement_blocks,
    bool reflective_boundary
) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;

    for (int movement_block = 0;
         movement_block < movement_blocks; ++movement_block) {
        if (!block_has_crossing[movement_block]) continue;
        const int begin = movement_block * MOVEMENT_BLOCK_SIZE;
        const int block_end = begin + MOVEMENT_BLOCK_SIZE;
        const int end = block_end < N ? block_end : N;
        for (int i = begin; i < end; ++i) {
        if ((site_flags[i] & MOVEMENT_CROSSING) == 0) continue;
        // Re-read live state. Earlier commits may have moved into, moved out
        // of, or annihilated this original candidate site.
        if (state[i] == 0 || locked[i] || moved[i]) continue;
        const int q = static_cast<int>(state[i]);

        int x, y, z;
        decode_xyz_d(i, L, x, y, z);

        // Last-resort repair + exact one-axis-per-tick remainder extraction.
        const PreparedMovement p = prepare_movement(
            vel_x[i], vel_y[i], vel_z[i],
            rem_x[i], rem_y[i], rem_z[i], latency[i], dt);
        vel_x[i] = p.vx; vel_y[i] = p.vy; vel_z[i] = p.vz;
        rem_x[i] = p.rx; rem_y[i] = p.ry; rem_z[i] = p.rz;
        if (p.projected) ++(*causal_projection_events);
        const int dx = p.dx, dy = p.dy, dz = p.dz;

        if (dx == 0 && dy == 0 && dz == 0) continue;

        const int nx = x + dx;
        const int ny = y + dy;
        const int nz = z + dz;
        const bool crosses = nx < 0 || nx >= L || ny < 0 || ny >= L
                          || nz < 0 || nz >= L;
        if (crosses) {
            if (reflective_boundary) {
                if (dx != 0) vel_x[i] = -vel_x[i];
                if (dy != 0) vel_y[i] = -vel_y[i];
                if (dz != 0) vel_z[i] = -vel_z[i];
                rem_x[i] = 0.0; rem_y[i] = 0.0; rem_z[i] = 0.0;
                continue;
            }

            // Open particle boundary: exhaust into the void. accel_mag is
            // intentionally left untouched, matching CPU handle_face_crossing.
            state[i] = 0;
            vel_x[i] = 0.0; vel_y[i] = 0.0; vel_z[i] = 0.0;
            rem_x[i] = 0.0; rem_y[i] = 0.0; rem_z[i] = 0.0;
            particle_id[i] = -1;
            pair_id[i] = -1;
            spin[i] = 0;
            color[i] = 0;
            flux_x[i] = 0.0; flux_y[i] = 0.0; flux_z[i] = 0.0;
            if (dual_substrate) {
                fL_x[i] = 0.0; fL_y[i] = 0.0; fL_z[i] = 0.0;
                fR_x[i] = 0.0; fR_y[i] = 0.0; fR_z[i] = 0.0;
            }
            continue;
        }

        const int target = nx * L * L + ny * L + nz;
        if (state[target] == 0) {
            ledger_route_moore_current(ledger_current_x, ledger_current_y,
                                       ledger_current_z, L,
                                       x, y, z, dx, dy, dz, q);

            state[target] = state[i];
            vel_x[target] = vel_x[i];
            vel_y[target] = vel_y[i];
            vel_z[target] = vel_z[i];
            rem_x[target] = rem_x[i];
            rem_y[target] = rem_y[i];
            rem_z[target] = rem_z[i];
            particle_id[target] = particle_id[i];
            pair_id[target] = pair_id[i];
            accel_mag[target] = accel_mag[i];
            spin[target] = spin[i];
            color[target] = color[i];

            const double old_rho = sqrt(flux_x[i]*flux_x[i]
                                      + flux_y[i]*flux_y[i]
                                      + flux_z[i]*flux_z[i]);
            if (old_rho > 1e-15) {
                const double transfer = fmin(old_rho, K_B);
                const double ratio = transfer / old_rho;
                const double sfx = flux_x[i] * ratio;
                const double sfy = flux_y[i] * ratio;
                const double sfz = flux_z[i] * ratio;
                flux_x[i] -= sfx; flux_y[i] -= sfy; flux_z[i] -= sfz;
                flux_x[target] += sfx;
                flux_y[target] += sfy;
                flux_z[target] += sfz;

                if (dual_substrate) {
                    const double sfLx = fL_x[i] * ratio;
                    const double sfLy = fL_y[i] * ratio;
                    const double sfLz = fL_z[i] * ratio;
                    const double sfRx = fR_x[i] * ratio;
                    const double sfRy = fR_y[i] * ratio;
                    const double sfRz = fR_z[i] * ratio;
                    fL_x[i] -= sfLx; fL_y[i] -= sfLy; fL_z[i] -= sfLz;
                    fR_x[i] -= sfRx; fR_y[i] -= sfRy; fR_z[i] -= sfRz;
                    fL_x[target] += sfLx;
                    fL_y[target] += sfLy;
                    fL_z[target] += sfLz;
                    fR_x[target] += sfRx;
                    fR_y[target] += sfRy;
                    fR_z[target] += sfRz;
                }
            }

            state[i] = 0;
            vel_x[i] = 0.0; vel_y[i] = 0.0; vel_z[i] = 0.0;
            rem_x[i] = 0.0; rem_y[i] = 0.0; rem_z[i] = 0.0;
            particle_id[i] = -1;
            pair_id[i] = -1;
            spin[i] = 0;
            color[i] = 0;
            moved[target] = 1;
        } else if (state[target] == state[i]) {
            if (dx != 0) vel_x[i] = -vel_x[i];
            if (dy != 0) vel_y[i] = -vel_y[i];
            if (dz != 0) vel_z[i] = -vel_z[i];
            rem_x[i] = 0.0; rem_y[i] = 0.0; rem_z[i] = 0.0;
        } else {
            // Opposite signs annihilate. Snapshot both fields before clearing,
            // then scatter each burst to its own periodic 6-neighbor shell.
            ledger_reaction[i] -= q;
            ledger_reaction[target] += q;

            // Non-crossing projections were applied/count-staged in parallel
            // before this ordered transaction. If this target is an original
            // later non-crosser, CPU order would annihilate it before its turn;
            // physical cleanup below overwrites the tentative drift, and this
            // removes its tentative projection telemetry. moved[target]
            // distinguishes a later arrival from that original candidate.
            const uint8_t target_flags = site_flags[target];
            if (target > i && !moved[target]
                && (target_flags & MOVEMENT_CANDIDATE) != 0
                && (target_flags & MOVEMENT_CROSSING) == 0
                && (target_flags & MOVEMENT_PROJECTED) != 0) {
                --(*causal_projection_events);
            }

            const double src_fx = flux_x[i];
            const double src_fy = flux_y[i];
            const double src_fz = flux_z[i];
            const double tgt_fx = flux_x[target];
            const double tgt_fy = flux_y[target];
            const double tgt_fz = flux_z[target];
            const double sLx = dual_substrate ? fL_x[i] : 0.0;
            const double sLy = dual_substrate ? fL_y[i] : 0.0;
            const double sLz = dual_substrate ? fL_z[i] : 0.0;
            const double sRx = dual_substrate ? fR_x[i] : 0.0;
            const double sRy = dual_substrate ? fR_y[i] : 0.0;
            const double sRz = dual_substrate ? fR_z[i] : 0.0;
            const double tLx = dual_substrate ? fL_x[target] : 0.0;
            const double tLy = dual_substrate ? fL_y[target] : 0.0;
            const double tLz = dual_substrate ? fL_z[target] : 0.0;
            const double tRx = dual_substrate ? fR_x[target] : 0.0;
            const double tRy = dual_substrate ? fR_y[target] : 0.0;
            const double tRz = dual_substrate ? fR_z[target] : 0.0;

            state[i] = 0;
            state[target] = 0;
            vel_x[i] = 0.0; vel_y[i] = 0.0; vel_z[i] = 0.0;
            vel_x[target] = 0.0;
            vel_y[target] = 0.0;
            vel_z[target] = 0.0;
            rem_x[i] = 0.0; rem_y[i] = 0.0; rem_z[i] = 0.0;
            rem_x[target] = 0.0;
            rem_y[target] = 0.0;
            rem_z[target] = 0.0;
            particle_id[i] = -1;
            particle_id[target] = -1;
            pair_id[i] = -1;
            pair_id[target] = -1;
            accel_mag[i] = 0.0;
            accel_mag[target] = 0.0;
            spin[i] = 0; spin[target] = 0;
            color[i] = 0; color[target] = 0;
            flux_x[i] = 0.0; flux_y[i] = 0.0; flux_z[i] = 0.0;
            flux_x[target] = 0.0;
            flux_y[target] = 0.0;
            flux_z[target] = 0.0;
            if (dual_substrate) {
                fL_x[i] = 0.0; fL_y[i] = 0.0; fL_z[i] = 0.0;
                fR_x[i] = 0.0; fR_y[i] = 0.0; fR_z[i] = 0.0;
                fL_x[target] = 0.0;
                fL_y[target] = 0.0;
                fL_z[target] = 0.0;
                fR_x[target] = 0.0;
                fR_y[target] = 0.0;
                fR_z[target] = 0.0;
            }

            const int nbrs_src[6] = {
                idx3d_d(x + 1, y, z, L), idx3d_d(x - 1, y, z, L),
                idx3d_d(x, y + 1, z, L), idx3d_d(x, y - 1, z, L),
                idx3d_d(x, y, z + 1, L), idx3d_d(x, y, z - 1, L)
            };
            const int nbrs_tgt[6] = {
                idx3d_d(nx + 1, ny, nz, L),
                idx3d_d(nx - 1, ny, nz, L),
                idx3d_d(nx, ny + 1, nz, L),
                idx3d_d(nx, ny - 1, nz, L),
                idx3d_d(nx, ny, nz + 1, L),
                idx3d_d(nx, ny, nz - 1, L)
            };
            const double sixth = 1.0 / 6.0;
            for (int n = 0; n < 6; ++n) {
                const int j = nbrs_src[n];
                flux_x[j] += src_fx * sixth;
                flux_y[j] += src_fy * sixth;
                flux_z[j] += src_fz * sixth;
            }
            for (int n = 0; n < 6; ++n) {
                const int j = nbrs_tgt[n];
                flux_x[j] += tgt_fx * sixth;
                flux_y[j] += tgt_fy * sixth;
                flux_z[j] += tgt_fz * sixth;
            }
            if (dual_substrate) {
                for (int n = 0; n < 6; ++n) {
                    const int j = nbrs_src[n];
                    fL_x[j] += sLx * sixth;
                    fL_y[j] += sLy * sixth;
                    fL_z[j] += sLz * sixth;
                    fR_x[j] += sRx * sixth;
                    fR_y[j] += sRy * sixth;
                    fR_z[j] += sRz * sixth;
                }
                for (int n = 0; n < 6; ++n) {
                    const int j = nbrs_tgt[n];
                    fL_x[j] += tLx * sixth;
                    fL_y[j] += tLy * sixth;
                    fL_z[j] += tLz * sixth;
                    fR_x[j] += tRx * sixth;
                    fR_y[j] += tRy * sixth;
                    fR_z[j] += tRz * sixth;
                }
            }
        }
    }
    }
}

// ============================================================================
// PARTICLE LIST — compact indices of all manifested particles
// ============================================================================
//
// DETERMINISTIC COMPACTION (replaces an atomic-race scatter, 2026-08-17).
// The original kernel assigned each manifested particle's plist_idx[] slot
// via atomicAdd(num_particles, 1). GPU thread-scheduling order for that
// race is NOT guaranteed identical between separate kernel launches, even
// from bit-identical prior device state, so the ORDER of particles within
// plist_idx[] varied run to run. Downstream color_force_kernel/
// yukawa_force_kernel/exchange_force_kernel accumulate each particle's
// force with `for (pj = 0; pj < num_particles; ++pj)` in exactly that
// (nondeterministic) order, and double-precision addition is not
// associative, so a different accumulation order flipped the last bit(s)
// of the summed force — divergence that compounds across many ticks into
// full state divergence. Two direct-launch (no graph capture) engines
// running an identical QCD profile from an identical seed diverged
// bit-for-bit by tick 24.
//
// Fixed with the same cub::DeviceSelect::Flagged compaction pattern already
// used for pair-production/lifecycle candidates
// (launch_pair_production/launch_canonical_lifecycle, kernels_aux.cu):
// flag manifested sites, compact deterministically (CUB's Flagged select
// preserves ascending input-index order and is reproducible run to run for
// identical input, unlike an atomic race), then transfer into the real
// capacity-bounded plist_idx.
//
// d_plist_idx is capacity-capped at MAX_PARTICLES (8192) — that is exactly
// the condition Task 5's overflow flag exists to detect and report, and the
// true manifested count can exceed it. CUB's compacted output therefore
// cannot be written directly into d_plist_idx (a lattice with >8192
// manifested particles would silently overrun an 8192-capacity buffer). It
// is written instead into the UNCAPPED N-sized scratch pair
// (d_particle_candidate_indices/d_particle_candidate_count), and
// finalize_particle_list_kernel below clamps/copies into the unchanged
// d_plist_idx/d_num_particles/d_particle_overflow contract that
// color_force_kernel etc. already read.

// Pass 1: per-site is-manifested flag (mirrors pair_production_candidate_kernel's
// candidate flag, but with a trivial predicate).
__global__ void mark_manifested_particles_kernel(
    const int8_t* __restrict__ state,
    uint8_t* __restrict__ flags,
    int N
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    flags[i] = (state[i] != 0) ? 1 : 0;
}

// Pass 3 (after the cub::DeviceSelect::Flagged compaction, issued between
// passes 1 and 3 in launch_build_particle_list): clamp
// the uncapped compacted list into the real capacity-bounded plist_idx.
// Launched with a fixed MAX_PARTICLES-sized grid (matching the
// color/yukawa/exchange/triad launch idiom below — a constant launch
// topology, each thread bounding itself from a device-resident count, so
// this stays graph-capture-eligible), rather than the single-thread
// commit-kernel idiom used for pair production, because this pass has no
// live-order dependency between destination slots: source order is already
// fixed and stable coming out of CUB, and every copied slot is independent,
// so a parallel copy is both safe and considerably cheaper than a serial
// 8192-iteration loop in one thread.
//
// Preserves Task 5's exact contract: num_particles ends up holding the RAW
// (possibly over-capacity) count — exactly what the old atomicAdd scatter
// left there, and exactly what color_force_kernel/yukawa_force_kernel/
// exchange_force_kernel/triad_detection_kernel already clamp against
// (`raw < max_particles ? raw : max_particles`) — and overflow is set
// whenever the raw count exceeds max_particles. This kernel only ever
// WRITES 1 to the flag, never 0 — it has no way to know the count has
// since dropped back under capacity, and clearing it here on an
// under-capacity tick would just race the read in
// GpuBuffers::throw_if_particle_overflow(), which is the sole place that
// clears it back to 0, immediately after observing it at a host
// synchronization boundary (sticky-until-acknowledged, not sticky-forever
// at the system level — see the comment on d_particle_overflow in
// gpu_buffers.h).
__global__ void finalize_particle_list_kernel(
    const int32_t* __restrict__ candidate_indices,
    const int32_t* __restrict__ candidate_count,
    int* __restrict__ plist_idx,
    int* __restrict__ num_particles,
    int* __restrict__ overflow,
    int max_particles
) {
    const int count = *candidate_count;
    if (blockIdx.x == 0 && threadIdx.x == 0) {
        *num_particles = count;
        if (count > max_particles) *overflow = 1;
    }
    const int copy_count = count < max_particles ? count : max_particles;
    int pi = blockIdx.x * blockDim.x + threadIdx.x;
    if (pi >= copy_count) return;
    plist_idx[pi] = candidate_indices[pi];
}

// ============================================================================
// DEVICE-SIDE RUNNING STRONG COUPLING [mirrors ontic::alpha_s_running]
// ============================================================================

// Device-side running coupling: uses canonical ontic constants directly
// (accessible via --expt-relaxed-constexpr NVCC flag)
__device__ __forceinline__
double alpha_s_running_d(double Q_GeV) {
    if (Q_GeV <= ftd::LAMBDA_QCD) return 1.0;
    double log_ratio = log(Q_GeV * Q_GeV / (ftd::LAMBDA_QCD * ftd::LAMBDA_QCD));
    if (log_ratio <= 0.0) return 1.0;
    return 4.0 * ftd::PI / (ftd::B0_NF5 * log_ratio);
}

__device__ __forceinline__
double alpha_s_lattice_d(double r_voxels) {
    if (r_voxels <= 0.0) return ftd::ALPHA_S;
    double Q = ftd::Q_LATTICE / r_voxels;
    double as = alpha_s_running_d(Q);
    return fmin(as, ftd::ALPHA_S);
}

// ============================================================================
// COLOR FORCE KERNEL [CLAUDE.md §6.4 — SU(3) color-dependent pairwise force]
// ============================================================================
// Thread per particle i, iterates over all other particles j.
// Same-color pairs: REPULSIVE (cf=+0.5); diff-color: ATTRACTIVE (cf=-1.0).
// Matches CPU convention in render_bridge.cpp phase_forces().
// Three regimes: r<3 (Coulomb), 3<=r<8 (transition), r>=8 (Harmonic confinement: F∝r, V∝r²).

__global__ void color_force_kernel(
    const int* __restrict__ plist_idx,
    const int* __restrict__ num_particles_ptr,
    const int  max_particles,
    const int8_t* __restrict__ state,
    const int8_t* __restrict__ color_arr,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    // Force-diag mirror (one site per particle, plain stores).
    double* __restrict__ fd_strong_x,
    double* __restrict__ fd_strong_y,
    double* __restrict__ fd_strong_z,
    int L
) {
    const int raw = *num_particles_ptr;
    const int num_particles = raw < max_particles ? raw : max_particles;
    int pi = blockIdx.x * blockDim.x + threadIdx.x;
    if (pi >= num_particles) return;

    int i = plist_idx[pi];
    int8_t ci = color_arr[i];
    // CPU parity (phase_forces.cpp:144): the color force acts ONLY on colored
    // probes (`v.color != 0`). Colorless particles feel no color force on CPU,
    // so skip them entirely here rather than admitting every state!=0 voxel.
    if (ci == 0) return;

    int ix, iy, iz;
    decode_xyz_d(i, L, ix, iy, iz);

    double fx = 0.0, fy = 0.0, fz = 0.0;

    for (int pj = 0; pj < num_particles; ++pj) {
        if (pj == pi) continue;
        int j = plist_idx[pj];
        // CPU parity (phase_forces.cpp:56): the colored-sites cache only holds
        // sources with `color != 0`. Skip colorless sources so they exert no
        // color force, matching CPU exactly.
        int8_t cj = color_arr[j];
        if (cj == 0) continue;
        int jx, jy, jz;
        decode_xyz_d(j, L, jx, jy, jz);

        int dx, dy, dz;
        periodic_delta_d(ix, iy, iz, jx, jy, jz, L, dx, dy, dz);
        double r2 = (double)(dx*dx + dy*dy + dz*dz);
        double r = sqrt(r2);
        if (r < 1.0) r = 1.0;

        // Color factor: same color → repulsive (+0.5), diff color → attractive (-1.0)
        // Matches CPU sign convention in phase_forces.cpp:162.
        // Both ci and cj are guaranteed nonzero here (colorless probes return
        // early; colorless sources are skipped above), matching the CPU
        // colored-sites cache which only contains color != 0 entries.
        double cf = (ci == cj) ? 0.5 : -1.0;

        double as = alpha_s_lattice_d(r);

        // Three-regime force profile (magnitude; sign from cf)
        // Regime boundaries from constants_shared.h (shared single source of truth).
        double f_mag;
        if (r < COLOR_COULOMB_RADIUS) {
            f_mag = as * cf / r2;                            // Coulomb
        } else if (r < COLOR_TRANSITION_RADIUS) {
            f_mag = as * cf / (COLOR_TRANSITION_DENOM * r); // Transition
        } else {
            f_mag = as * cf * r / COLOR_LINEAR_DENOM;       // Harmonic confinement (F∝r, V∝r²)
        }

        // Direction: cf>0 pushes AWAY (repulsive), cf<0 pulls TOWARD (attractive)
        // Negate to match CPU: f_color -= F_mag * d/r
        double inv_r = 1.0 / r;
        fx -= f_mag * dx * inv_r;
        fy -= f_mag * dy * inv_r;
        fz -= f_mag * dz * inv_r;
    }

    // Mirror per-particle color force into force_diag (parity with CPU
    // RenderBridge::phase_forces: force_diag_[i].f_strong = f_color).
    // Each thread owns a unique particle index, so plain stores are safe.
    if (fd_strong_x) {
        fd_strong_x[i] = fx;
        fd_strong_y[i] = fy;
        fd_strong_z[i] = fz;
    }
}

// ============================================================================
// YUKAWA (STRONG) FORCE KERNEL [CLAUDE.md §6.4]
// ============================================================================
// F = ALPHA_S * exp(-M_YUKAWA * r) / r² * (1 + M_YUKAWA * r) — attractive, all particles.

__global__ void yukawa_force_kernel(
    const int* __restrict__ plist_idx,
    const int* __restrict__ num_particles_ptr,
    const int  max_particles,
    const int8_t* __restrict__ state,
    double* __restrict__ fd_strong_x,
    double* __restrict__ fd_strong_y,
    double* __restrict__ fd_strong_z,
    int L
) {
    const int raw = *num_particles_ptr;
    const int num_particles = raw < max_particles ? raw : max_particles;
    int pi = blockIdx.x * blockDim.x + threadIdx.x;
    if (pi >= num_particles) return;

    int i = plist_idx[pi];
    int ix, iy, iz;
    decode_xyz_d(i, L, ix, iy, iz);

    double fx = 0.0, fy = 0.0, fz = 0.0;

    // Use canonical ontic constants (via --expt-relaxed-constexpr)
    const double AS = ftd::ALPHA_S;     // = 1.0 (Planck-scale strong coupling)
    const double MY = ftd::M_YUKAWA;   // = 1.0 (inverse meson mass in lattice units)

    for (int pj = 0; pj < num_particles; ++pj) {
        if (pj == pi) continue;
        int j = plist_idx[pj];
        int jx, jy, jz;
        decode_xyz_d(j, L, jx, jy, jz);

        int dx, dy, dz;
        periodic_delta_d(ix, iy, iz, jx, jy, jz, L, dx, dy, dz);
        double r2 = (double)(dx*dx + dy*dy + dz*dz);
        double r = sqrt(r2);
        if (r < 1.0) r = 1.0;

        // Yukawa force: attractive, short range
        double f_mag = AS * exp(-MY * r) / r2 * (1.0 + MY * r);

        // Attractive: toward j
        double inv_r = 1.0 / r;
        fx += f_mag * dx * inv_r;
        fy += f_mag * dy * inv_r;
        fz += f_mag * dz * inv_r;
    }

    fd_strong_x[i] += fx;
    fd_strong_y[i] += fy;
    fd_strong_z[i] += fz;
}

// ============================================================================
// EXCHANGE (PAULI) FORCE KERNEL [CLAUDE.md §11]
// ============================================================================
// Same-spin repulsion: F = ALPHA_EXCHANGE * exp(-r²/r_ex²) / r² (repulsive)
// Only between same-spin particles. Very short range.

__global__ void exchange_force_kernel(
    const int* __restrict__ plist_idx,
    const int* __restrict__ num_particles_ptr,
    const int  max_particles,
    const int8_t* __restrict__ state,
    const int8_t* __restrict__ spin_arr,
    double* __restrict__ fd_exchange_x,
    double* __restrict__ fd_exchange_y,
    double* __restrict__ fd_exchange_z,
    int L
) {
    const int raw = *num_particles_ptr;
    const int num_particles = raw < max_particles ? raw : max_particles;
    int pi = blockIdx.x * blockDim.x + threadIdx.x;
    if (pi >= num_particles) return;

    int i = plist_idx[pi];
    int ix, iy, iz;
    decode_xyz_d(i, L, ix, iy, iz);
    int8_t si = spin_arr[i];
    if (si == 0) return;  // No spin → no exchange

    double fx = 0.0, fy = 0.0, fz = 0.0;

    const double AE = ftd::ALPHA_EXCHANGE;  // α² (from ontic chain)
    const double R_EX = ftd::EXCHANGE_RANGE;         // Exchange range (voxels)
    const double R_EX2 = ftd::EXCHANGE_RANGE_SQ;     // R_EX²

    for (int pj = 0; pj < num_particles; ++pj) {
        if (pj == pi) continue;
        int j = plist_idx[pj];
        if (spin_arr[j] != si) continue;  // Only same-spin

        int jx, jy, jz;
        decode_xyz_d(j, L, jx, jy, jz);
        int dx, dy, dz;
        periodic_delta_d(ix, iy, iz, jx, jy, jz, L, dx, dy, dz);
        double r2 = (double)(dx*dx + dy*dy + dz*dz);
        double r = sqrt(r2);
        if (r < 1.0) r = 1.0;

        // Repulsive short-range: away from j
        double f_mag = AE * exp(-r2 / R_EX2) / (r * r);

        double inv_r = 1.0 / r;
        fx -= f_mag * dx * inv_r;
        fy -= f_mag * dy * inv_r;
        fz -= f_mag * dz * inv_r;
    }

    fd_exchange_x[i] = fx;
    fd_exchange_y[i] = fy;
    fd_exchange_z[i] = fz;
}

// ============================================================================
// TRIAD BINDING DETECTION [CLAUDE.md §8.1]
// ============================================================================
// For each particle, find 2 nearest same-sign neighbors. If all pairwise
// distances within 20% AND all < TRIAD_RADIUS → set locked=true.

__global__ void triad_detection_kernel(
    const int* __restrict__ plist_idx,
    const int* __restrict__ num_particles_ptr,
    const int  max_particles,
    const int8_t* __restrict__ state,
    uint8_t* __restrict__ locked,
    int L
) {
    const int raw = *num_particles_ptr;
    const int num_particles = raw < max_particles ? raw : max_particles;
    int pi = blockIdx.x * blockDim.x + threadIdx.x;
    if (pi >= num_particles) return;

    int i = plist_idx[pi];
    int8_t si = state[i];
    int ix, iy, iz;
    decode_xyz_d(i, L, ix, iy, iz);

    const double TRIAD_RADIUS = ftd::TRIAD_RADIUS;

    // Find 2 nearest same-sign neighbors
    double best1_r2 = 1e30, best2_r2 = 1e30;
    int best1_j = -1, best2_j = -1;
    int best1_dx = 0, best1_dy = 0, best1_dz = 0;
    int best2_dx = 0, best2_dy = 0, best2_dz = 0;

    for (int pj = 0; pj < num_particles; ++pj) {
        if (pj == pi) continue;
        int j = plist_idx[pj];
        if (state[j] != si) continue;

        int jx, jy, jz;
        decode_xyz_d(j, L, jx, jy, jz);
        int dx, dy, dz;
        periodic_delta_d(ix, iy, iz, jx, jy, jz, L, dx, dy, dz);
        double r2 = (double)(dx*dx + dy*dy + dz*dz);

        if (r2 < best1_r2) {
            best2_r2 = best1_r2; best2_j = best1_j;
            best2_dx = best1_dx; best2_dy = best1_dy; best2_dz = best1_dz;
            best1_r2 = r2; best1_j = j;
            best1_dx = dx; best1_dy = dy; best1_dz = dz;
        } else if (r2 < best2_r2) {
            best2_r2 = r2; best2_j = j;
            best2_dx = dx; best2_dy = dy; best2_dz = dz;
        }
    }

    if (best1_j < 0 || best2_j < 0) return;

    double r_a = sqrt(best1_r2);
    double r_b = sqrt(best2_r2);

    // Distance between the two neighbors
    int dx_ab = best2_dx - best1_dx;
    int dy_ab = best2_dy - best1_dy;
    int dz_ab = best2_dz - best1_dz;
    double r_c = sqrt((double)(dx_ab*dx_ab + dy_ab*dy_ab + dz_ab*dz_ab));

    // Check: all within TRIAD_RADIUS
    if (r_a > TRIAD_RADIUS || r_b > TRIAD_RADIUS || r_c > TRIAD_RADIUS) return;

    // Check: near-equilateral (pairwise distances within 20% of each other)
    double r_max = fmax(r_a, fmax(r_b, r_c));
    double r_min = fmin(r_a, fmin(r_b, r_c));
    if (r_max <= 0.0) return;
    double ratio = r_min / r_max;
    if (ratio < ftd::TRIAD_RATIO_THRESHOLD) return;

    // Triad detected — lock all three
    locked[i] = 1;
    locked[best1_j] = 1;
    locked[best2_j] = 1;
}

// ---------- Launcher Functions ----------

void launch_phase_forces(GpuBuffers& bufs, bool poisson_coulomb,
                         bool emergent_forces,
                         bool gravity, bool lorentz_force, double dt) {
    const cudaStream_t stream = bufs.stream;
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy than 512
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    phase_forces_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state, bufs.d_locked, bufs.d_phi_coulomb,
        bufs.d_latency,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_accel_mag,
        bufs.d_fd_coulomb_x,  bufs.d_fd_coulomb_y,  bufs.d_fd_coulomb_z,
        bufs.d_fd_gravity_x,  bufs.d_fd_gravity_y,  bufs.d_fd_gravity_z,
        bufs.d_fd_magnetic_x, bufs.d_fd_magnetic_y, bufs.d_fd_magnetic_z,
        poisson_coulomb, emergent_forces, gravity, lorentz_force, dt, L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_integrate_forces(GpuBuffers& bufs, double dt) {
    const cudaStream_t stream = bufs.stream;
    const int block = 256;
    const int grid = (bufs.N + block - 1) / block;
    integrate_forces_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state, bufs.d_locked, bufs.d_latency,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_fd_coulomb_x, bufs.d_fd_coulomb_y, bufs.d_fd_coulomb_z,
        bufs.d_fd_gravity_x, bufs.d_fd_gravity_y, bufs.d_fd_gravity_z,
        bufs.d_fd_magnetic_x, bufs.d_fd_magnetic_y, bufs.d_fd_magnetic_z,
        bufs.d_fd_strong_x, bufs.d_fd_strong_y, bufs.d_fd_strong_z,
        bufs.d_fd_exchange_x, bufs.d_fd_exchange_y, bufs.d_fd_exchange_z,
        dt, bufs.N);
    CUDA_CHECK(cudaGetLastError());
}

void launch_phase_movement(GpuBuffers& bufs, double dt, bool reflective_boundary,
                           bool dual_substrate) {
    const cudaStream_t stream = bufs.stream;
    const int L = bufs.L;
    constexpr int block = MOVEMENT_BLOCK_SIZE;
    const int grid = (bufs.N + block - 1) / block;
    // Pair/lifecycle selection and movement are serialized on the default
    // stream. Reuse the leading bytes of the 4N-byte index scratch for the
    // ceil(N/256) block flags, avoiding a costly extra cudaMalloc per engine.
    auto* movement_block_flags =
        reinterpret_cast<uint8_t*>(bufs.d_pair_candidate_indices);

    CUDA_CHECK(cudaMemsetAsync(bufs.d_causal_projection_events, 0,
                               sizeof(unsigned long long), stream));
    movement_prepass_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state, bufs.d_locked,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
        bufs.d_latency,
        bufs.d_pair_candidate_flags, bufs.d_movement_moved,
        movement_block_flags,
        dt, bufs.N);
    CUDA_CHECK(cudaGetLastError());

    apply_non_crossing_movement_kernel<<<grid, block, 0, stream>>>(
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
        bufs.d_latency, bufs.d_pair_candidate_flags,
        bufs.d_causal_projection_events, dt, bufs.N);
    CUDA_CHECK(cudaGetLastError());

    phase_movement_commit_crossings_kernel<<<1, 1, 0, stream>>>(
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
        bufs.d_latency,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_locked,
        bufs.d_particle_id, bufs.d_spin, bufs.d_color,
        bufs.d_pair_id, bufs.d_accel_mag,
        bufs.d_ledger_reaction,
        bufs.d_ledger_current_x, bufs.d_ledger_current_y, bufs.d_ledger_current_z,
        bufs.d_causal_projection_events,
        dual_substrate,
        bufs.d_flux_L_x, bufs.d_flux_L_y, bufs.d_flux_L_z,
        bufs.d_flux_R_x, bufs.d_flux_R_y, bufs.d_flux_R_z,
        bufs.d_pair_candidate_flags,
        movement_block_flags,
        bufs.d_movement_moved,
        dt, L, bufs.N, grid, reflective_boundary
    );
    CUDA_CHECK(cudaGetLastError());
}

// Shared fixed launch shape for everything that walks the capacity-bounded
// plist_idx (the finalize pass below and the pairwise/triad launches
// further down) — a constant MAX_PARTICLES threads, each bounding itself
// against a device-resident count, so the launch topology never depends on
// a host read (required for CUDA graph capture) and no thread can read past
// the d_plist_idx allocation.
namespace {
constexpr int PARTICLE_FORCE_BLOCK = 256;
constexpr int PARTICLE_FORCE_GRID =
    (GpuBuffers::MAX_PARTICLES + PARTICLE_FORCE_BLOCK - 1)
    / PARTICLE_FORCE_BLOCK;
}  // namespace

void launch_build_particle_list(GpuBuffers& bufs) {
    const cudaStream_t stream = bufs.stream;

    // Pass 1: flag every manifested site (parallel, full lattice).
    constexpr int block = 256;
    const int grid = (bufs.N + block - 1) / block;
    mark_manifested_particles_kernel<<<grid, block, 0, stream>>>(
        bufs.d_state, bufs.d_particle_flags, bufs.N);
    CUDA_CHECK(cudaGetLastError());

    // Pass 2: deterministic compaction (ascending lattice-index order,
    // stable/reproducible for identical input — unlike the atomic-race slot
    // assignment this replaces). Output is UNCAPPED (sized to N, not
    // MAX_PARTICLES) because the true manifested count can exceed capacity;
    // see the block comment above mark_manifested_particles_kernel. Reuses
    // the pair-production/lifecycle CUB scratch workspace
    // (d_pair_select_temp/pair_select_temp_bytes) — GpuBuffers::allocate()
    // explicitly sizes that workspace to cover this call too (same
    // (thrust::counting_iterator<int32_t>, uint8_t* flags, int32_t* output,
    // int32_t* count, N) shape), and reuse is safe because every kernel and
    // CUB call in a tick is issued to this one bufs.stream, which CUDA
    // serializes in issue order — this call never overlaps the
    // pair-production/lifecycle calls that also use the workspace.
    thrust::counting_iterator<int32_t> indices(0);
    CUDA_CHECK(cub::DeviceSelect::Flagged(
        bufs.d_pair_select_temp, bufs.pair_select_temp_bytes,
        indices, bufs.d_particle_flags,
        bufs.d_particle_candidate_indices, bufs.d_particle_candidate_count,
        bufs.N, stream));

    // Pass 3: clamp/copy into the real capacity-bounded plist_idx and
    // reproduce the num_particles/overflow contract (see the block comment
    // above finalize_particle_list_kernel).
    finalize_particle_list_kernel<<<PARTICLE_FORCE_GRID, PARTICLE_FORCE_BLOCK, 0, stream>>>(
        bufs.d_particle_candidate_indices, bufs.d_particle_candidate_count,
        bufs.d_plist_idx, bufs.d_num_particles, bufs.d_particle_overflow,
        GpuBuffers::MAX_PARTICLES);
    CUDA_CHECK(cudaGetLastError());
}

// ---------- Fixed-capacity pairwise/triad launches (Component A) ----------
//
// These used to be sized from a host-side count obtained with a blocking
// cudaMemcpy of d_num_particles, which forced a host/device round trip 1-4x
// per tick and made the launch topology data-dependent (fatal for graph
// capture). Each launch is now a constant MAX_PARTICLES threads — 32 blocks
// of 256, ~3% of one full-lattice L=64 kernel — and every thread bounds
// itself against the device counter, clamped to MAX_PARTICLES so no thread
// can read past the d_plist_idx allocation.

void launch_color_force(GpuBuffers& bufs, double dt) {
    (void)dt;
    const cudaStream_t stream = bufs.stream;
    color_force_kernel<<<PARTICLE_FORCE_GRID, PARTICLE_FORCE_BLOCK, 0, stream>>>(
        bufs.d_plist_idx, bufs.d_num_particles, GpuBuffers::MAX_PARTICLES,
        bufs.d_state, bufs.d_color,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_fd_strong_x, bufs.d_fd_strong_y, bufs.d_fd_strong_z,
        bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_yukawa_force(GpuBuffers& bufs, double dt) {
    (void)dt;
    const cudaStream_t stream = bufs.stream;
    yukawa_force_kernel<<<PARTICLE_FORCE_GRID, PARTICLE_FORCE_BLOCK, 0, stream>>>(
        bufs.d_plist_idx, bufs.d_num_particles, GpuBuffers::MAX_PARTICLES,
        bufs.d_state,
        bufs.d_fd_strong_x, bufs.d_fd_strong_y, bufs.d_fd_strong_z,
        bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_exchange_force(GpuBuffers& bufs, double dt) {
    (void)dt;
    const cudaStream_t stream = bufs.stream;
    exchange_force_kernel<<<PARTICLE_FORCE_GRID, PARTICLE_FORCE_BLOCK, 0, stream>>>(
        bufs.d_plist_idx, bufs.d_num_particles, GpuBuffers::MAX_PARTICLES,
        bufs.d_state, bufs.d_spin,
        bufs.d_fd_exchange_x, bufs.d_fd_exchange_y, bufs.d_fd_exchange_z,
        bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_triad_detection(GpuBuffers& bufs) {
    const cudaStream_t stream = bufs.stream;
    triad_detection_kernel<<<PARTICLE_FORCE_GRID, PARTICLE_FORCE_BLOCK, 0, stream>>>(
        bufs.d_plist_idx, bufs.d_num_particles, GpuBuffers::MAX_PARTICLES,
        bufs.d_state, bufs.d_locked, bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
