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
#include "ftd/constants_gpu.cuh"
#include "../cuda/cuda_index.cuh"   // ftd::wrap, ftd::idx3d, ftd::decode_xyz, ftd::periodic_delta
#include <cuda_runtime.h>
#include <cmath>
#include <cstdio>   // fprintf — Linux/clang stricter than MSVC
#include <cstdlib>  // exit

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

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

// Byte-level atomicCAS: CUDA only supports 32-bit+ atomicCAS,
// so we operate on the containing 32-bit word.
__device__ __forceinline__
int8_t atomicCAS_byte(int8_t* addr, int8_t compare, int8_t val) {
    // Find the 4-byte aligned word containing our byte
    unsigned int* word_addr = reinterpret_cast<unsigned int*>(
        reinterpret_cast<size_t>(addr) & ~3ULL);
    unsigned int byte_offset = (reinterpret_cast<size_t>(addr) & 3) * 8;
    unsigned int byte_mask = 0xFFu << byte_offset;

    unsigned int old_word = *word_addr;
    unsigned int assumed;
    do {
        assumed = old_word;
        unsigned int old_byte = (assumed >> byte_offset) & 0xFF;
        if (old_byte != static_cast<unsigned char>(compare))
            return static_cast<int8_t>(old_byte);
        unsigned int new_word = (assumed & ~byte_mask)
                              | (static_cast<unsigned int>(static_cast<unsigned char>(val)) << byte_offset);
        old_word = atomicCAS(word_addr, assumed, new_word);
    } while (old_word != assumed);
    return compare;  // Success: old value was indeed `compare`
}

// Unconditional atomic byte store: avoids races with atomicCAS_byte
// on bytes sharing the same 32-bit word.
__device__ __forceinline__
void atomicStore_byte(int8_t* addr, int8_t val) {
    unsigned int* word_addr = reinterpret_cast<unsigned int*>(
        reinterpret_cast<size_t>(addr) & ~3ULL);
    unsigned int byte_offset = (reinterpret_cast<size_t>(addr) & 3) * 8;
    unsigned int byte_mask = 0xFFu << byte_offset;

    unsigned int old_word = *word_addr;
    unsigned int assumed;
    do {
        assumed = old_word;
        unsigned int new_word = (assumed & ~byte_mask)
                              | (static_cast<unsigned int>(static_cast<unsigned char>(val)) << byte_offset);
        old_word = atomicCAS(word_addr, assumed, new_word);
    } while (old_word != assumed);
}

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

    // --- Coulomb force ---
    if (poisson_coulomb) {
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

    // --- Update velocity (skip locked particles) ---
    // 2026-05-04: parity with CPU phase_forces.cpp:198 — locked
    // particles get force_diag populated and accel_mag computed (above)
    // but their velocity is NOT updated. Without this guard, locked
    // particles' velocity drifted under Coulomb/Lorentz forces despite
    // being structurally "fixed" — caught by test_logic_engine C7.
    if (locked[i]) return;

    double old_vx = vel_x[i], old_vy = vel_y[i], old_vz = vel_z[i];
    vel_x[i] += fx * dt;
    vel_y[i] += fy * dt;
    vel_z[i] += fz * dt;

    // Speed clamping (nothing outruns light)
    double speed2 = vel_x[i]*vel_x[i] + vel_y[i]*vel_y[i] + vel_z[i]*vel_z[i];
    if (speed2 > C_SPEED * C_SPEED) {
        double scale = C_SPEED / sqrt(speed2);
        vel_x[i] *= scale;
        vel_y[i] *= scale;
        vel_z[i] *= scale;
    }

    // Store acceleration magnitude (for Larmor radiation)
    double ax = vel_x[i] - old_vx, ay = vel_y[i] - old_vy, az = vel_z[i] - old_vz;
    accel_mag[i] = sqrt(ax*ax + ay*ay + az*az) / dt;
}

// ---------- Movement Kernel ----------
// Accumulate remainder, compute integer moves.
// For Phase 1 simplicity: movement resolved on CPU (download particle list).
// This kernel only does remainder accumulation and speed clamping.

__global__ void phase_movement_kernel(
    int8_t* __restrict__ state,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    double* __restrict__ rem_x,
    double* __restrict__ rem_y,
    double* __restrict__ rem_z,
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
    double dt,
    int L
) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;

    int i = x * L * L + y * L + z;  // X-major (matches CPU)
    if (state[i] == 0 || locked[i]) return;
    const int q = static_cast<int>(state[i]);

    // Accumulate remainder
    rem_x[i] += vel_x[i] * dt;
    rem_y[i] += vel_y[i] * dt;
    rem_z[i] += vel_z[i] * dt;

    // Compute integer displacement
    int dx = 0, dy = 0, dz = 0;
    if (rem_x[i] >= 1.0) { dx = 1; rem_x[i] -= 1.0; }
    else if (rem_x[i] <= -1.0) { dx = -1; rem_x[i] += 1.0; }
    if (rem_y[i] >= 1.0) { dy = 1; rem_y[i] -= 1.0; }
    else if (rem_y[i] <= -1.0) { dy = -1; rem_y[i] += 1.0; }
    if (rem_z[i] >= 1.0) { dz = 1; rem_z[i] -= 1.0; }
    else if (rem_z[i] <= -1.0) { dz = -1; rem_z[i] += 1.0; }

    if (dx == 0 && dy == 0 && dz == 0) return;  // No movement

    int tx = wrap_d(x + dx, L);
    int ty = wrap_d(y + dy, L);
    int tz = wrap_d(z + dz, L);
    int target = tx * L * L + ty * L + tz;  // X-major (matches CPU)

    // Collision resolution via byte-level atomicCAS on state
    // Try to claim target site (only if currently void)
    int8_t old = atomicCAS_byte(&state[target], 0, state[i]);

    if (old == 0) {
        ledger_route_moore_current(ledger_current_x, ledger_current_y,
                                   ledger_current_z, L,
                                   x, y, z, dx, dy, dz, q);

        // Successfully claimed target — transfer particle data
        vel_x[target] = vel_x[i];
        vel_y[target] = vel_y[i];
        vel_z[target] = vel_z[i];
        rem_x[target] = rem_x[i];
        rem_y[target] = rem_y[i];
        rem_z[target] = rem_z[i];
        particle_id[target] = particle_id[i];
        spin[target] = spin[i];
        color[target] = color[i];
        pair_id[target] = pair_id[i];
        accel_mag[target] = accel_mag[i];

        // Portable self-field transfer
        double old_rho = sqrt(flux_x[i]*flux_x[i] + flux_y[i]*flux_y[i] + flux_z[i]*flux_z[i]);
        if (old_rho > 1e-15) {
            double transfer = fmin(old_rho, K_B);
            double ratio = transfer / old_rho;
            double sfx = flux_x[i] * ratio;
            double sfy = flux_y[i] * ratio;
            double sfz = flux_z[i] * ratio;

            // Atomic subtract from source, add to target (prevents races)
            atomicAdd(&flux_x[target], sfx);
            atomicAdd(&flux_y[target], sfy);
            atomicAdd(&flux_z[target], sfz);
            atomicAdd(&flux_x[i], -sfx);
            atomicAdd(&flux_y[i], -sfy);
            atomicAdd(&flux_z[i], -sfz);
        }

        // Clear source (use atomic store to avoid races with adjacent CAS ops)
        atomicStore_byte(&state[i], 0);
        vel_x[i] = 0; vel_y[i] = 0; vel_z[i] = 0;
        rem_x[i] = 0; rem_y[i] = 0; rem_z[i] = 0;
        particle_id[i] = -1;
        spin[i] = 0;
        color[i] = 0;
        pair_id[i] = -1;
        accel_mag[i] = 0.0;
    } else if (old == -state[i]) {
        atomicAdd(&ledger_reaction[i], -q);
        atomicAdd(&ledger_reaction[target], q);

        // Opposite charge at target: annihilation
        // Both particles return to void (use atomic store for thread safety)
        atomicStore_byte(&state[i], 0);
        atomicStore_byte(&state[target], 0);
        vel_x[i] = 0; vel_y[i] = 0; vel_z[i] = 0;
        vel_x[target] = 0; vel_y[target] = 0; vel_z[target] = 0;
        rem_x[i] = 0; rem_y[i] = 0; rem_z[i] = 0;
        rem_x[target] = 0; rem_y[target] = 0; rem_z[target] = 0;
        particle_id[i] = -1;
        particle_id[target] = -1;
        spin[i] = 0; spin[target] = 0;
        color[i] = 0; color[target] = 0;

        // Snapshot source/target flux into registers BEFORE scatter to avoid
        // torn reads from concurrent threads that may also be writing to these
        // sites. Without this, a second annihilating thread (e.g. the partner
        // particle running its own movement step) can interleave atomicAdd
        // operations on flux[i]/flux[target] with the read here, producing
        // double-scatter and energy non-conservation.
        const double src_fx = flux_x[i],     src_fy = flux_y[i],     src_fz = flux_z[i];
        const double tgt_fx = flux_x[target], tgt_fy = flux_y[target], tgt_fz = flux_z[target];

        // Zero source and target flux up-front so any concurrent reader sees a
        // consistent post-annihilation state.
        flux_x[i] = 0; flux_y[i] = 0; flux_z[i] = 0;
        flux_x[target] = 0; flux_y[target] = 0; flux_z[target] = 0;

        // Scatter source flux to its 6 face neighbors
        const double sixth = 1.0 / 6.0;
        int nbrs_src[6] = {
            idx3d_d(x+1,y,z,L), idx3d_d(x-1,y,z,L),
            idx3d_d(x,y+1,z,L), idx3d_d(x,y-1,z,L),
            idx3d_d(x,y,z+1,L), idx3d_d(x,y,z-1,L)
        };
        for (int n = 0; n < 6; ++n) {
            atomicAdd(&flux_x[nbrs_src[n]], src_fx * sixth);
            atomicAdd(&flux_y[nbrs_src[n]], src_fy * sixth);
            atomicAdd(&flux_z[nbrs_src[n]], src_fz * sixth);
        }

        // Scatter target flux to its 6 face neighbors
        int nbrs_tgt[6] = {
            idx3d_d(tx+1,ty,tz,L), idx3d_d(tx-1,ty,tz,L),
            idx3d_d(tx,ty+1,tz,L), idx3d_d(tx,ty-1,tz,L),
            idx3d_d(tx,ty,tz+1,L), idx3d_d(tx,ty,tz-1,L)
        };
        for (int n = 0; n < 6; ++n) {
            atomicAdd(&flux_x[nbrs_tgt[n]], tgt_fx * sixth);
            atomicAdd(&flux_y[nbrs_tgt[n]], tgt_fy * sixth);
            atomicAdd(&flux_z[nbrs_tgt[n]], tgt_fz * sixth);
        }
    } else {
        // Same-sign collision → elastic bounce: reverse velocity along movement axis
        if (dx != 0) vel_x[i] = -vel_x[i];
        if (dy != 0) vel_y[i] = -vel_y[i];
        if (dz != 0) vel_z[i] = -vel_z[i];
        rem_x[i] = 0; rem_y[i] = 0; rem_z[i] = 0;
    }
}

// ============================================================================
// PARTICLE LIST — compact indices of all manifested particles
// ============================================================================

__global__ void build_particle_list_kernel(
    const int8_t* __restrict__ state,
    int* __restrict__ plist_idx,
    int* __restrict__ num_particles,
    int N, int max_particles
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    if (state[i] == 0) return;

    int slot = atomicAdd(num_particles, 1);
    if (slot < max_particles) {
        plist_idx[slot] = i;
    }
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
// Three regimes: r<3 (Coulomb), 3<=r<8 (transition), r>=8 (linear confinement).

__global__ void color_force_kernel(
    const int* __restrict__ plist_idx,
    const int  num_particles,
    const int8_t* __restrict__ state,
    const int8_t* __restrict__ color_arr,
    const double* __restrict__ flux_x,
    const double* __restrict__ flux_y,
    const double* __restrict__ flux_z,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    // Force-diag mirror (one site per particle, plain stores).
    double* __restrict__ fd_strong_x,
    double* __restrict__ fd_strong_y,
    double* __restrict__ fd_strong_z,
    double dt,
    int L
) {
    int pi = blockIdx.x * blockDim.x + threadIdx.x;
    if (pi >= num_particles) return;

    int i = plist_idx[pi];
    int ix, iy, iz;
    decode_xyz_d(i, L, ix, iy, iz);
    int8_t ci = color_arr[i];

    double fx = 0.0, fy = 0.0, fz = 0.0;

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

        // Color factor: same color → repulsive (+0.5), diff color → attractive (-1.0)
        // Matches CPU sign convention in phase_forces.cpp:160.
        // 2026-05-04: removed extra `&& ci > 0` guard. Pre-fix two
        // colorless particles (color==0) got cf=-1.0 (attractive) on GPU
        // but cf=+0.5 (repulsive) on CPU — exact sign disagreement.
        int8_t cj = color_arr[j];
        double cf = (ci == cj) ? 0.5 : -1.0;

        double as = alpha_s_lattice_d(r);

        // Three-regime force profile (magnitude; sign from cf)
        // Regime boundaries from constants_gpu.cuh (shared single source of truth).
        double f_mag;
        if (r < COLOR_COULOMB_RADIUS) {
            f_mag = as * cf / r2;                            // Coulomb
        } else if (r < COLOR_TRANSITION_RADIUS) {
            f_mag = as * cf / (COLOR_TRANSITION_DENOM * r); // Transition
        } else {
            f_mag = as * cf * r / COLOR_LINEAR_DENOM;       // Linear confinement
        }

        // Direction: cf>0 pushes AWAY (repulsive), cf<0 pulls TOWARD (attractive)
        // Negate to match CPU: f_color -= F_mag * d/r
        double inv_r = 1.0 / r;
        fx -= f_mag * dx * inv_r;
        fy -= f_mag * dy * inv_r;
        fz -= f_mag * dz * inv_r;
    }

    atomicAdd(&vel_x[i], fx * dt);
    atomicAdd(&vel_y[i], fy * dt);
    atomicAdd(&vel_z[i], fz * dt);

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
    const int  num_particles,
    const int8_t* __restrict__ state,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    double dt,
    int L
) {
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

    atomicAdd(&vel_x[i], fx * dt);
    atomicAdd(&vel_y[i], fy * dt);
    atomicAdd(&vel_z[i], fz * dt);
}

// ============================================================================
// EXCHANGE (PAULI) FORCE KERNEL [CLAUDE.md §11]
// ============================================================================
// Same-spin repulsion: F = ALPHA_EXCHANGE * exp(-r²/r_ex²) / r² (repulsive)
// Only between same-spin particles. Very short range.

__global__ void exchange_force_kernel(
    const int* __restrict__ plist_idx,
    const int  num_particles,
    const int8_t* __restrict__ state,
    const int8_t* __restrict__ spin_arr,
    double* __restrict__ vel_x,
    double* __restrict__ vel_y,
    double* __restrict__ vel_z,
    double dt,
    int L
) {
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

    atomicAdd(&vel_x[i], fx * dt);
    atomicAdd(&vel_y[i], fy * dt);
    atomicAdd(&vel_z[i], fz * dt);
}

// ============================================================================
// TRIAD BINDING DETECTION [CLAUDE.md §8.1]
// ============================================================================
// For each particle, find 2 nearest same-sign neighbors. If all pairwise
// distances within 20% AND all < TRIAD_RADIUS → set locked=true.

__global__ void triad_detection_kernel(
    const int* __restrict__ plist_idx,
    const int  num_particles,
    const int8_t* __restrict__ state,
    uint8_t* __restrict__ locked,
    int L
) {
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
                         bool gravity, bool lorentz_force, double dt) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy than 512
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    phase_forces_kernel<<<grid, block>>>(
        bufs.d_state, bufs.d_locked, bufs.d_phi_coulomb,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_accel_mag,
        bufs.d_fd_coulomb_x,  bufs.d_fd_coulomb_y,  bufs.d_fd_coulomb_z,
        bufs.d_fd_gravity_x,  bufs.d_fd_gravity_y,  bufs.d_fd_gravity_z,
        bufs.d_fd_magnetic_x, bufs.d_fd_magnetic_y, bufs.d_fd_magnetic_z,
        poisson_coulomb, gravity, lorentz_force, dt, L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_phase_movement(GpuBuffers& bufs, double dt) {
    int L = bufs.L;
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy than 512
    dim3 grid((L+3)/4, (L+7)/8, (L+7)/8);

    phase_movement_kernel<<<grid, block>>>(
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_locked,
        bufs.d_particle_id, bufs.d_spin, bufs.d_color,
        bufs.d_pair_id, bufs.d_accel_mag,
        bufs.d_ledger_reaction,
        bufs.d_ledger_current_x, bufs.d_ledger_current_y, bufs.d_ledger_current_z,
        dt, L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_build_particle_list(GpuBuffers& bufs) {
    // Reset counter
    CUDA_CHECK(cudaMemset(bufs.d_num_particles, 0, sizeof(int)));

    int block = 256;
    int grid = (bufs.N + block - 1) / block;
    build_particle_list_kernel<<<grid, block>>>(
        bufs.d_state, bufs.d_plist_idx, bufs.d_num_particles,
        bufs.N, GpuBuffers::MAX_PARTICLES
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_color_force(GpuBuffers& bufs, int num_particles, double dt) {
    if (num_particles <= 0) return;
    int block = 256;
    int grid = (num_particles + block - 1) / block;
    color_force_kernel<<<grid, block>>>(
        bufs.d_plist_idx, num_particles,
        bufs.d_state, bufs.d_color,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        bufs.d_fd_strong_x, bufs.d_fd_strong_y, bufs.d_fd_strong_z,
        dt, bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_yukawa_force(GpuBuffers& bufs, int num_particles, double dt) {
    if (num_particles <= 0) return;
    int block = 256;
    int grid = (num_particles + block - 1) / block;
    yukawa_force_kernel<<<grid, block>>>(
        bufs.d_plist_idx, num_particles,
        bufs.d_state,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        dt, bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_exchange_force(GpuBuffers& bufs, int num_particles, double dt) {
    if (num_particles <= 0) return;
    int block = 256;
    int grid = (num_particles + block - 1) / block;
    exchange_force_kernel<<<grid, block>>>(
        bufs.d_plist_idx, num_particles,
        bufs.d_state, bufs.d_spin,
        bufs.d_velocity_x, bufs.d_velocity_y, bufs.d_velocity_z,
        dt, bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

void launch_triad_detection(GpuBuffers& bufs, int num_particles) {
    if (num_particles <= 0) return;
    int block = 256;
    int grid = (num_particles + block - 1) / block;
    triad_detection_kernel<<<grid, block>>>(
        bufs.d_plist_idx, num_particles,
        bufs.d_state, bufs.d_locked, bufs.L
    );
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd
