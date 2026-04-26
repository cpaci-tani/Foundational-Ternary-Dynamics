/**
 * SoA device buffer management for FTD GPU engine.
 * Handles allocation, deallocation, and AoS↔SoA conversions.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include <cuda_runtime.h>
#include <cufft.h>
#include <cstdio>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Check CUDA errors with file/line info
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

// ---------- Allocation ----------

void GpuBuffers::allocate(int lattice_size) {
    L = lattice_size;
    N = L * L * L;

    // State
    CUDA_CHECK(cudaMalloc(&d_state, N * sizeof(int8_t)));

    // Flux (3 components)
    CUDA_CHECK(cudaMalloc(&d_flux_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_z, N * sizeof(double)));

    // Wave velocity (3 components)
    CUDA_CHECK(cudaMalloc(&d_wave_vel_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_z, N * sizeof(double)));

    // Particle velocity (3 components)
    CUDA_CHECK(cudaMalloc(&d_velocity_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_velocity_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_velocity_z, N * sizeof(double)));

    // Remainder (3 components)
    CUDA_CHECK(cudaMalloc(&d_remainder_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_remainder_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_remainder_z, N * sizeof(double)));

    // Scalar fields
    CUDA_CHECK(cudaMalloc(&d_locked, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_particle_id, N * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_spin, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_color, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_flavor, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_accel_mag, N * sizeof(double)));

    // Solver potentials
    CUDA_CHECK(cudaMalloc(&d_phi, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_phi_coulomb, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_phi_latency, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_latency, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_tau, N * sizeof(double)));
    // Zero-initialize the latency fields (warm-start = 0)
    CUDA_CHECK(cudaMemset(d_phi_latency, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_latency, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_tau, 0, N * sizeof(double)));

    // Read-phase temporaries
    CUDA_CHECK(cudaMalloc(&d_delta_j_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_z, N * sizeof(double)));

    // Dual-substrate fields (18 arrays)
    CUDA_CHECK(cudaMalloc(&d_flux_L_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_L_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_L_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_R_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_R_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_R_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_L_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_L_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_L_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_R_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_R_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_R_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_L_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_L_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_L_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_R_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_R_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_R_z, N * sizeof(double)));

    // Strong field (Stella Octangula)
    CUDA_CHECK(cudaMalloc(&d_flux_strong_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_strong_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_strong_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_strong_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_strong_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_strong_z, N * sizeof(double)));

    // Weak field (Cuboctahedron)
    CUDA_CHECK(cudaMalloc(&d_flux_weak_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_weak_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_weak_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_weak_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_weak_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_weak_z, N * sizeof(double)));

    // Selective damping mask + Larmor acceleration
    CUDA_CHECK(cudaMalloc(&d_near_particle, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_near_accel, N * sizeof(double)));

    // FFT workspace
    CUDA_CHECK(cudaMalloc(&d_fft_buf, N * sizeof(cufftDoubleComplex)));
    CUDA_CHECK(cudaMalloc(&d_fft_buf_f, N * sizeof(cufftComplex)));
    CUDA_CHECK(cudaMalloc(&d_green, N * sizeof(double)));

    // cuRAND workspace
    CUDA_CHECK(cudaMalloc(&d_random, N * sizeof(double)));

    // Langevin thermostat noise (3N normals per tick when toggle active).
    // Allocate eagerly so the buffer is always available; wasted bytes are
    // 24·N = 24 MB at L=128, negligible. Zeroed at init and at free.
    CUDA_CHECK(cudaMalloc(&d_langevin_noise, 3 * N * sizeof(double)));

    // Particle list
    CUDA_CHECK(cudaMalloc(&d_plist_idx, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_num_particles, sizeof(int)));

    // Pair production tracking
    CUDA_CHECK(cudaMalloc(&d_pair_id, N * sizeof(int32_t)));

    // Native EFT continuity event ledger
    CUDA_CHECK(cudaMalloc(&d_ledger_rho_before, N * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_ledger_reaction, N * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_ledger_current_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_ledger_current_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_ledger_current_z, N * sizeof(double)));

    // Zero-initialize all buffers
    CUDA_CHECK(cudaMemset(d_state, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_flux_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_velocity_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_velocity_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_velocity_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_remainder_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_remainder_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_remainder_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_locked, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_particle_id, 0xFF, N * sizeof(int32_t))); // -1
    CUDA_CHECK(cudaMemset(d_spin, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_color, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_flavor, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_accel_mag, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_phi, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_phi_coulomb, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_L_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_L_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_L_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_R_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_R_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_R_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_L_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_L_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_L_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_R_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_R_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_R_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_L_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_L_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_L_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_R_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_R_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_R_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_strong_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_strong_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_strong_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_strong_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_strong_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_strong_z, 0, N * sizeof(double)));

    CUDA_CHECK(cudaMemset(d_flux_weak_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_weak_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_weak_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_weak_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_weak_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_weak_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_near_particle, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_near_accel, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_random, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_langevin_noise, 0, 3 * N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_plist_idx, 0, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_num_particles, 0, sizeof(int)));
    CUDA_CHECK(cudaMemset(d_pair_id, 0xFF, N * sizeof(int32_t))); // -1
    CUDA_CHECK(cudaMemset(d_ledger_rho_before, 0, N * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_ledger_reaction, 0, N * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_ledger_current_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_ledger_current_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_ledger_current_z, 0, N * sizeof(double)));
}

void GpuBuffers::free() {
    if (d_state)         { cudaFree(d_state); d_state = nullptr; }
    if (d_flux_x)        { cudaFree(d_flux_x); d_flux_x = nullptr; }
    if (d_flux_y)        { cudaFree(d_flux_y); d_flux_y = nullptr; }
    if (d_flux_z)        { cudaFree(d_flux_z); d_flux_z = nullptr; }
    if (d_wave_vel_x)    { cudaFree(d_wave_vel_x); d_wave_vel_x = nullptr; }
    if (d_wave_vel_y)    { cudaFree(d_wave_vel_y); d_wave_vel_y = nullptr; }
    if (d_wave_vel_z)    { cudaFree(d_wave_vel_z); d_wave_vel_z = nullptr; }
    if (d_velocity_x)    { cudaFree(d_velocity_x); d_velocity_x = nullptr; }
    if (d_velocity_y)    { cudaFree(d_velocity_y); d_velocity_y = nullptr; }
    if (d_velocity_z)    { cudaFree(d_velocity_z); d_velocity_z = nullptr; }
    if (d_remainder_x)   { cudaFree(d_remainder_x); d_remainder_x = nullptr; }
    if (d_remainder_y)   { cudaFree(d_remainder_y); d_remainder_y = nullptr; }
    if (d_remainder_z)   { cudaFree(d_remainder_z); d_remainder_z = nullptr; }
    if (d_locked)        { cudaFree(d_locked); d_locked = nullptr; }
    if (d_particle_id)   { cudaFree(d_particle_id); d_particle_id = nullptr; }
    if (d_spin)          { cudaFree(d_spin); d_spin = nullptr; }
    if (d_color)         { cudaFree(d_color); d_color = nullptr; }
    if (d_flavor)        { cudaFree(d_flavor); d_flavor = nullptr; }
    if (d_accel_mag)     { cudaFree(d_accel_mag); d_accel_mag = nullptr; }
    if (d_phi)           { cudaFree(d_phi); d_phi = nullptr; }
    if (d_phi_coulomb)   { cudaFree(d_phi_coulomb); d_phi_coulomb = nullptr; }
    if (d_phi_latency)   { cudaFree(d_phi_latency); d_phi_latency = nullptr; }
    if (d_latency)       { cudaFree(d_latency); d_latency = nullptr; }
    if (d_tau)           { cudaFree(d_tau); d_tau = nullptr; }
    if (d_delta_j_x)     { cudaFree(d_delta_j_x); d_delta_j_x = nullptr; }
    if (d_delta_j_y)     { cudaFree(d_delta_j_y); d_delta_j_y = nullptr; }
    if (d_delta_j_z)     { cudaFree(d_delta_j_z); d_delta_j_z = nullptr; }
    if (d_flux_L_x)      { cudaFree(d_flux_L_x); d_flux_L_x = nullptr; }
    if (d_flux_L_y)      { cudaFree(d_flux_L_y); d_flux_L_y = nullptr; }
    if (d_flux_L_z)      { cudaFree(d_flux_L_z); d_flux_L_z = nullptr; }
    if (d_flux_R_x)      { cudaFree(d_flux_R_x); d_flux_R_x = nullptr; }
    if (d_flux_R_y)      { cudaFree(d_flux_R_y); d_flux_R_y = nullptr; }
    if (d_flux_R_z)      { cudaFree(d_flux_R_z); d_flux_R_z = nullptr; }
    if (d_wave_vel_L_x)  { cudaFree(d_wave_vel_L_x); d_wave_vel_L_x = nullptr; }
    if (d_wave_vel_L_y)  { cudaFree(d_wave_vel_L_y); d_wave_vel_L_y = nullptr; }
    if (d_wave_vel_L_z)  { cudaFree(d_wave_vel_L_z); d_wave_vel_L_z = nullptr; }
    if (d_wave_vel_R_x)  { cudaFree(d_wave_vel_R_x); d_wave_vel_R_x = nullptr; }
    if (d_wave_vel_R_y)  { cudaFree(d_wave_vel_R_y); d_wave_vel_R_y = nullptr; }
    if (d_wave_vel_R_z)  { cudaFree(d_wave_vel_R_z); d_wave_vel_R_z = nullptr; }
    if (d_delta_j_L_x)   { cudaFree(d_delta_j_L_x); d_delta_j_L_x = nullptr; }
    if (d_delta_j_L_y)   { cudaFree(d_delta_j_L_y); d_delta_j_L_y = nullptr; }
    if (d_delta_j_L_z)   { cudaFree(d_delta_j_L_z); d_delta_j_L_z = nullptr; }
    if (d_delta_j_R_x)   { cudaFree(d_delta_j_R_x); d_delta_j_R_x = nullptr; }
    if (d_delta_j_R_y)   { cudaFree(d_delta_j_R_y); d_delta_j_R_y = nullptr; }
    if (d_delta_j_R_z)   { cudaFree(d_delta_j_R_z); d_delta_j_R_z = nullptr; }
    if (d_flux_strong_x)     { cudaFree(d_flux_strong_x); d_flux_strong_x = nullptr; }
    if (d_flux_strong_y)     { cudaFree(d_flux_strong_y); d_flux_strong_y = nullptr; }
    if (d_flux_strong_z)     { cudaFree(d_flux_strong_z); d_flux_strong_z = nullptr; }
    if (d_wave_vel_strong_x) { cudaFree(d_wave_vel_strong_x); d_wave_vel_strong_x = nullptr; }
    if (d_wave_vel_strong_y) { cudaFree(d_wave_vel_strong_y); d_wave_vel_strong_y = nullptr; }
    if (d_wave_vel_strong_z) { cudaFree(d_wave_vel_strong_z); d_wave_vel_strong_z = nullptr; }

    if (d_flux_weak_x)     { cudaFree(d_flux_weak_x); d_flux_weak_x = nullptr; }
    if (d_flux_weak_y)     { cudaFree(d_flux_weak_y); d_flux_weak_y = nullptr; }
    if (d_flux_weak_z)     { cudaFree(d_flux_weak_z); d_flux_weak_z = nullptr; }
    if (d_wave_vel_weak_x) { cudaFree(d_wave_vel_weak_x); d_wave_vel_weak_x = nullptr; }
    if (d_wave_vel_weak_y) { cudaFree(d_wave_vel_weak_y); d_wave_vel_weak_y = nullptr; }
    if (d_wave_vel_weak_z) { cudaFree(d_wave_vel_weak_z); d_wave_vel_weak_z = nullptr; }
    if (d_near_particle) { cudaFree(d_near_particle); d_near_particle = nullptr; }
    if (d_near_accel)    { cudaFree(d_near_accel); d_near_accel = nullptr; }
    if (d_fft_buf)       { cudaFree(d_fft_buf); d_fft_buf = nullptr; }
    if (d_fft_buf_f)     { cudaFree(d_fft_buf_f); d_fft_buf_f = nullptr; }
    if (d_green)         { cudaFree(d_green); d_green = nullptr; }
    if (d_random)        { cudaFree(d_random); d_random = nullptr; }
    if (d_langevin_noise) { cudaFree(d_langevin_noise); d_langevin_noise = nullptr; }
    if (d_plist_idx)     { cudaFree(d_plist_idx); d_plist_idx = nullptr; }
    if (d_num_particles) { cudaFree(d_num_particles); d_num_particles = nullptr; }
    if (d_pair_id)       { cudaFree(d_pair_id); d_pair_id = nullptr; }
    if (d_ledger_rho_before) { cudaFree(d_ledger_rho_before); d_ledger_rho_before = nullptr; }
    if (d_ledger_reaction)   { cudaFree(d_ledger_reaction); d_ledger_reaction = nullptr; }
    if (d_ledger_current_x)  { cudaFree(d_ledger_current_x); d_ledger_current_x = nullptr; }
    if (d_ledger_current_y)  { cudaFree(d_ledger_current_y); d_ledger_current_y = nullptr; }
    if (d_ledger_current_z)  { cudaFree(d_ledger_current_z); d_ledger_current_z = nullptr; }
    N = 0;
    L = 0;
}

// ---------- AoS → SoA Upload ----------

void GpuBuffers::upload(const std::vector<Voxel>& host_voxels,
                        const std::vector<double>& host_phi,
                        const std::vector<double>& host_phi_coulomb) {
    upload_voxels(host_voxels);
    CUDA_CHECK(cudaMemcpy(d_phi, host_phi.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_phi_coulomb, host_phi_coulomb.data(), N * sizeof(double), cudaMemcpyHostToDevice));
}

void GpuBuffers::upload_voxels(const std::vector<Voxel>& host_voxels) {
    // Scatter AoS fields into separate host staging arrays, then upload
    std::vector<int8_t>  h_state(N);
    std::vector<double>  h_fx(N), h_fy(N), h_fz(N);
    std::vector<double>  h_wvx(N), h_wvy(N), h_wvz(N);
    std::vector<double>  h_vx(N), h_vy(N), h_vz(N);
    std::vector<double>  h_rx(N), h_ry(N), h_rz(N);
    std::vector<bool>    h_locked(N);
    std::vector<int32_t> h_pid(N);
    std::vector<int8_t>  h_spin(N), h_color(N), h_flavor(N);
    std::vector<double>  h_accel(N);
    std::vector<int32_t> h_pair_id(N);
    std::vector<double>  h_latency(N);
    std::vector<double>  h_tau(N);
    // Dual-substrate staging
    std::vector<double>  h_fLx(N), h_fLy(N), h_fLz(N);
    std::vector<double>  h_fRx(N), h_fRy(N), h_fRz(N);
    std::vector<double>  h_wvLx(N), h_wvLy(N), h_wvLz(N);
    std::vector<double>  h_wvRx(N), h_wvRy(N), h_wvRz(N);

    // Strong field staging
    std::vector<double>  h_fsx(N), h_fsy(N), h_fsz(N);
    std::vector<double>  h_wvsx(N), h_wvsy(N), h_wvsz(N);

    // Weak field staging
    std::vector<double>  h_fwx(N), h_fwy(N), h_fwz(N);
    std::vector<double>  h_wvwx(N), h_wvwy(N), h_wvwz(N);

    for (int i = 0; i < N; ++i) {
        const auto& v = host_voxels[i];
        h_state[i]  = v.state;
        h_color[i]  = v.color;
        h_flavor[i] = v.flavor;
        h_fx[i]     = v.flux.x;
        h_fy[i]     = v.flux.y;
        h_fz[i]     = v.flux.z;
        h_wvx[i]    = v.wave_vel.x;
        h_wvy[i]    = v.wave_vel.y;
        h_wvz[i]    = v.wave_vel.z;
        h_vx[i]     = v.velocity.x;
        h_vy[i]     = v.velocity.y;
        h_vz[i]     = v.velocity.z;
        h_rx[i]     = v.remainder.x;
        h_ry[i]     = v.remainder.y;
        h_rz[i]     = v.remainder.z;
        h_locked[i] = v.locked;
        h_pid[i]    = v.particle_id;
        h_spin[i]   = v.spin;
        h_color[i]  = v.color;
        h_accel[i]  = v.accel_mag;
        h_pair_id[i] = v.pair_id;
        h_latency[i] = v.latency;
        h_tau[i]     = v.tau;
        // Dual-substrate
        h_fLx[i]  = v.flux_L.x;
        h_fLy[i]  = v.flux_L.y;
        h_fLz[i]  = v.flux_L.z;
        h_fRx[i]  = v.flux_R.x;
        h_fRy[i]  = v.flux_R.y;
        h_fRz[i]  = v.flux_R.z;
        h_wvLx[i] = v.wave_vel_L.x;
        h_wvLy[i] = v.wave_vel_L.y;
        h_wvLz[i] = v.wave_vel_L.z;
        h_wvRx[i] = v.wave_vel_R.x;
        h_wvRy[i] = v.wave_vel_R.y;
        h_wvRz[i] = v.wave_vel_R.z;
        // Strong field
        h_fsx[i]  = v.flux_strong.x;
        h_fsy[i]  = v.flux_strong.y;
        h_fsz[i]  = v.flux_strong.z;
        h_wvsx[i] = v.wave_vel_strong.x;
        h_wvsy[i] = v.wave_vel_strong.y;
        h_wvsz[i] = v.wave_vel_strong.z;
        // Weak field
        h_fwx[i]  = v.flux_weak.x;
        h_fwy[i]  = v.flux_weak.y;
        h_fwz[i]  = v.flux_weak.z;
        h_wvwx[i] = v.wave_vel_weak.x;
        h_wvwy[i] = v.wave_vel_weak.y;
        h_wvwz[i] = v.wave_vel_weak.z;
    }

    CUDA_CHECK(cudaMemcpy(d_state, h_state.data(), N * sizeof(int8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_x, h_fx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_y, h_fy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_z, h_fz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_x, h_wvx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_y, h_wvy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_z, h_wvz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_velocity_x, h_vx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_velocity_y, h_vy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_velocity_z, h_vz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_remainder_x, h_rx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_remainder_y, h_ry.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_remainder_z, h_rz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    // Note: bool vector is not guaranteed contiguous in all implementations.
    // Use uint8_t staging buffer for safe upload.
    {
        std::vector<uint8_t> h_locked_u8(N);
        for (int i = 0; i < N; ++i) h_locked_u8[i] = h_locked[i] ? 1 : 0;
        CUDA_CHECK(cudaMemcpy(d_locked, h_locked_u8.data(), N * sizeof(uint8_t), cudaMemcpyHostToDevice));
    }
    CUDA_CHECK(cudaMemcpy(d_particle_id, h_pid.data(), N * sizeof(int32_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_spin, h_spin.data(), N * sizeof(int8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_color, h_color.data(), N * sizeof(int8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flavor, h_flavor.data(), N * sizeof(int8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_accel_mag, h_accel.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pair_id, h_pair_id.data(), N * sizeof(int32_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_latency, h_latency.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_tau, h_tau.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    // Dual-substrate
    CUDA_CHECK(cudaMemcpy(d_flux_L_x, h_fLx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_L_y, h_fLy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_L_z, h_fLz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_R_x, h_fRx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_R_y, h_fRy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_R_z, h_fRz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_L_x, h_wvLx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_L_y, h_wvLy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_L_z, h_wvLz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_R_x, h_wvRx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_R_y, h_wvRy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_R_z, h_wvRz.data(), N * sizeof(double), cudaMemcpyHostToDevice));

    // Strong field
    CUDA_CHECK(cudaMemcpy(d_flux_strong_x, h_fsx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_strong_y, h_fsy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_strong_z, h_fsz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_strong_x, h_wvsx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_strong_y, h_wvsy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_strong_z, h_wvsz.data(), N * sizeof(double), cudaMemcpyHostToDevice));

    // Weak field
    CUDA_CHECK(cudaMemcpy(d_flux_weak_x, h_fwx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_weak_y, h_fwy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_flux_weak_z, h_fwz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_weak_x, h_wvwx.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_weak_y, h_wvwy.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_wave_vel_weak_z, h_wvwz.data(), N * sizeof(double), cudaMemcpyHostToDevice));
}

// ---------- SoA → AoS Download ----------

void GpuBuffers::download(std::vector<Voxel>& host_voxels,
                          std::vector<double>& host_phi,
                          std::vector<double>& host_phi_coulomb) const {
    download_voxels(host_voxels);
    host_phi.resize(N);
    host_phi_coulomb.resize(N);
    CUDA_CHECK(cudaMemcpy(host_phi.data(), d_phi, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(host_phi_coulomb.data(), d_phi_coulomb, N * sizeof(double), cudaMemcpyDeviceToHost));
}

void GpuBuffers::download_phi_latency(std::vector<double>& out) const {
    out.resize(N);
    CUDA_CHECK(cudaMemcpy(out.data(), d_phi_latency, N * sizeof(double),
                          cudaMemcpyDeviceToHost));
}

__global__ void reset_continuity_ledger_kernel(
    const int8_t* __restrict__ state,
    int* __restrict__ rho_before,
    int* __restrict__ reaction,
    double* __restrict__ current_x,
    double* __restrict__ current_y,
    double* __restrict__ current_z,
    int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    rho_before[i] = static_cast<int>(state[i]);
    reaction[i] = 0;
    current_x[i] = 0.0;
    current_y[i] = 0.0;
    current_z[i] = 0.0;
}

void GpuBuffers::reset_continuity_ledger() {
    constexpr int block = 256;
    const int grid = (N + block - 1) / block;
    reset_continuity_ledger_kernel<<<grid, block>>>(
        d_state, d_ledger_rho_before, d_ledger_reaction,
        d_ledger_current_x, d_ledger_current_y, d_ledger_current_z, N);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}

void GpuBuffers::download_continuity_ledger(
    std::vector<int>& rho_before,
    std::vector<int>& rho_after,
    std::vector<int>& reaction,
    std::vector<double>& current_x,
    std::vector<double>& current_y,
    std::vector<double>& current_z) const {
    rho_before.resize(N);
    rho_after.resize(N);
    reaction.resize(N);
    current_x.resize(N);
    current_y.resize(N);
    current_z.resize(N);

    std::vector<int8_t> state_after(static_cast<size_t>(N));
    CUDA_CHECK(cudaMemcpy(rho_before.data(), d_ledger_rho_before,
                          N * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(state_after.data(), d_state,
                          N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(reaction.data(), d_ledger_reaction,
                          N * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(current_x.data(), d_ledger_current_x,
                          N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(current_y.data(), d_ledger_current_y,
                          N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(current_z.data(), d_ledger_current_z,
                          N * sizeof(double), cudaMemcpyDeviceToHost));

    for (int i = 0; i < N; ++i) {
        rho_after[static_cast<size_t>(i)] =
            static_cast<int>(state_after[static_cast<size_t>(i)]);
    }
}

void GpuBuffers::download_voxels(std::vector<Voxel>& host_voxels) const {
    host_voxels.resize(N);

    // Download SoA arrays to host staging
    std::vector<int8_t>  h_state(N);
    std::vector<double>  h_fx(N), h_fy(N), h_fz(N);
    std::vector<double>  h_wvx(N), h_wvy(N), h_wvz(N);
    std::vector<double>  h_vx(N), h_vy(N), h_vz(N);
    std::vector<double>  h_rx(N), h_ry(N), h_rz(N);
    std::vector<uint8_t> h_locked(N);
    std::vector<int32_t> h_pid(N);
    std::vector<int8_t>  h_spin(N), h_color(N), h_flavor(N);
    std::vector<double>  h_accel(N);
    std::vector<int32_t> h_pair_id_dl(N);
    std::vector<double>  h_latency(N);
    std::vector<double>  h_tau(N);

    CUDA_CHECK(cudaMemcpy(h_state.data(), d_state, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_latency.data(), d_latency, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_tau.data(), d_tau, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fx.data(), d_flux_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fy.data(), d_flux_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fz.data(), d_flux_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvx.data(), d_wave_vel_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvy.data(), d_wave_vel_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvz.data(), d_wave_vel_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vx.data(), d_velocity_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vy.data(), d_velocity_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vz.data(), d_velocity_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_rx.data(), d_remainder_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_ry.data(), d_remainder_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_rz.data(), d_remainder_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_locked.data(), d_locked, N * sizeof(uint8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pid.data(), d_particle_id, N * sizeof(int32_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_spin.data(), d_spin, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_color.data(), d_color, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_flavor.data(), d_flavor, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_accel.data(), d_accel_mag, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pair_id_dl.data(), d_pair_id, N * sizeof(int32_t), cudaMemcpyDeviceToHost));

    // Download dual-substrate arrays
    std::vector<double> h_fLx(N), h_fLy(N), h_fLz(N);
    std::vector<double> h_fRx(N), h_fRy(N), h_fRz(N);
    std::vector<double> h_wvLx(N), h_wvLy(N), h_wvLz(N);
    std::vector<double> h_wvRx(N), h_wvRy(N), h_wvRz(N);

    // Strong field
    std::vector<double> h_fsx(N), h_fsy(N), h_fsz(N);
    std::vector<double> h_wvsx(N), h_wvsy(N), h_wvsz(N);
    // Weak field
    std::vector<double> h_fwx(N), h_fwy(N), h_fwz(N);
    std::vector<double> h_wvwx(N), h_wvwy(N), h_wvwz(N);

    CUDA_CHECK(cudaMemcpy(h_fLx.data(), d_flux_L_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fLy.data(), d_flux_L_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fLz.data(), d_flux_L_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fRx.data(), d_flux_R_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fRy.data(), d_flux_R_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fRz.data(), d_flux_R_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvLx.data(), d_wave_vel_L_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvLy.data(), d_wave_vel_L_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvLz.data(), d_wave_vel_L_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvRx.data(), d_wave_vel_R_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvRy.data(), d_wave_vel_R_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvRz.data(), d_wave_vel_R_z, N * sizeof(double), cudaMemcpyDeviceToHost));

    // Strong field
    CUDA_CHECK(cudaMemcpy(h_fsx.data(), d_flux_strong_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fsy.data(), d_flux_strong_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fsz.data(), d_flux_strong_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvsx.data(), d_wave_vel_strong_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvsy.data(), d_wave_vel_strong_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvsz.data(), d_wave_vel_strong_z, N * sizeof(double), cudaMemcpyDeviceToHost));

    // Weak field
    CUDA_CHECK(cudaMemcpy(h_fwx.data(), d_flux_weak_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fwy.data(), d_flux_weak_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fwz.data(), d_flux_weak_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvwx.data(), d_wave_vel_weak_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvwy.data(), d_wave_vel_weak_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvwz.data(), d_wave_vel_weak_z, N * sizeof(double), cudaMemcpyDeviceToHost));

    // Gather into AoS
    for (int i = 0; i < N; ++i) {
        auto& v = host_voxels[i];
        v.state       = h_state[i];
        v.flux        = {h_fx[i], h_fy[i], h_fz[i]};
        v.wave_vel    = {h_wvx[i], h_wvy[i], h_wvz[i]};
        v.velocity    = {h_vx[i], h_vy[i], h_vz[i]};
        v.remainder   = {h_rx[i], h_ry[i], h_rz[i]};
        v.locked      = h_locked[i] != 0;
        v.particle_id = h_pid[i];
        v.spin        = h_spin[i];
        v.color       = h_color[i];
        v.flavor      = h_flavor[i];
        v.accel_mag   = h_accel[i];
        // Dual-substrate fields
        v.flux_L      = {h_fLx[i], h_fLy[i], h_fLz[i]};
        v.flux_R      = {h_fRx[i], h_fRy[i], h_fRz[i]};
        v.wave_vel_L  = {h_wvLx[i], h_wvLy[i], h_wvLz[i]};
        v.wave_vel_R  = {h_wvRx[i], h_wvRy[i], h_wvRz[i]};
        // Strong field
        v.flux_strong      = {h_fsx[i], h_fsy[i], h_fsz[i]};
        v.wave_vel_strong  = {h_wvsx[i], h_wvsy[i], h_wvsz[i]};
        // Weak field
        v.flux_weak        = {h_fwx[i], h_fwy[i], h_fwz[i]};
        v.wave_vel_weak    = {h_wvwx[i], h_wvwy[i], h_wvwz[i]};
        // Pair ID from device
        v.pair_id     = h_pair_id_dl[i];
        // Latency field + proper time from GPU (when latency_field toggle is on)
        v.latency     = h_latency[i];
        v.tau         = h_tau[i];
    }
}

// ---------- Green's Function Precomputation ----------

__global__ void kernel_precompute_green(double* green, int L) {
    int kx = blockIdx.x * blockDim.x + threadIdx.x;
    int ky = blockIdx.y * blockDim.y + threadIdx.y;
    int kz = blockIdx.z * blockDim.z + threadIdx.z;
    if (kx >= L || ky >= L || kz >= L) return;

    int idx = kx * L * L + ky * L + kz;  // X-major (matches CPU)

    // Isotropic 18-point Laplacian eigenvalue for periodic BC:
    // G(k) = (2/3)(cx+cy+cz-3) + (2/3)(cx*cy+cy*cz+cz*cx-3)
    // Cancels O(k^4) anisotropic term of the 6-point stencil.
    // Real-space: face weight=1/3, edge weight=1/6, center=-4
    double cx = cos(2.0 * M_PI * kx / L);
    double cy = cos(2.0 * M_PI * ky / L);
    double cz = cos(2.0 * M_PI * kz / L);
    double face = cx + cy + cz - 3.0;
    double edge = cx*cy + cy*cz + cz*cx - 3.0;
    double G = (2.0/3.0) * face + (2.0/3.0) * edge;

    // Store 1/G for spectral division. DC component (k=0,0,0) → 0 (gauge freedom)
    green[idx] = (kx == 0 && ky == 0 && kz == 0) ? 0.0 : 1.0 / G;
}

void GpuBuffers::precompute_green_function() {
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);
    kernel_precompute_green<<<grid, block>>>(d_green, L);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}

}  // namespace gpu
}  // namespace ftd
