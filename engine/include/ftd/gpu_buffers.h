#pragma once
/**
 * SoA (Structure-of-Arrays) device buffers for GPU-accelerated FTD engine.
 *
 * The CPU engine uses AoS (Voxel struct ~154 bytes). For GPU memory
 * coalescence, we decompose into separate arrays per field.
 * Upload/download functions convert between AoS (host) and SoA (device).
 */

#include "voxel.h"
#include <vector>
#include <cufft.h>

namespace ftd {
namespace gpu {

struct GpuBuffers {
    int N = 0;    // total sites (L^3)
    int L = 0;    // lattice side length

    // --- Per-voxel state ---
    int8_t*   d_state       = nullptr;  // ternary state {-1,0,+1}

    // --- Flux field (Vec3) ---
    double*   d_flux_x      = nullptr;
    double*   d_flux_y      = nullptr;
    double*   d_flux_z      = nullptr;

    // --- Wave velocity (Vec3) ---
    double*   d_wave_vel_x  = nullptr;
    double*   d_wave_vel_y  = nullptr;
    double*   d_wave_vel_z  = nullptr;

    // --- Particle velocity (Vec3) ---
    double*   d_velocity_x  = nullptr;
    double*   d_velocity_y  = nullptr;
    double*   d_velocity_z  = nullptr;

    // --- Sub-lattice remainder (Vec3) ---
    double*   d_remainder_x = nullptr;
    double*   d_remainder_y = nullptr;
    double*   d_remainder_z = nullptr;

    // --- Scalar fields ---
    uint8_t*  d_locked      = nullptr;  // I2 fix: match kernel/memcpy usage (was bool*)
    int32_t*  d_particle_id = nullptr;
    int8_t*   d_spin        = nullptr;
    int8_t*   d_color       = nullptr;
    double*   d_accel_mag   = nullptr;

    // --- Solver fields ---
    double*   d_phi         = nullptr;  // Gauss potential (warm-started)
    double*   d_phi_coulomb = nullptr;  // Coulomb potential (warm-started)
    double*   d_phi_latency = nullptr;  // Latency Poisson potential (warm-started)
    double*   d_latency     = nullptr;  // voxel.latency = sqrt(clamp(|phi_latency|, 0, 0.998))
    double*   d_tau         = nullptr;  // voxel.tau: accumulated proper time

    // --- Read-phase temporary (delta_j) ---
    double*   d_delta_j_x   = nullptr;
    double*   d_delta_j_y   = nullptr;
    double*   d_delta_j_z   = nullptr;

    // --- Dual-substrate fields (active when dual_substrate toggle = true) ---
    double*   d_flux_L_x     = nullptr;
    double*   d_flux_L_y     = nullptr;
    double*   d_flux_L_z     = nullptr;
    double*   d_flux_R_x     = nullptr;
    double*   d_flux_R_y     = nullptr;
    double*   d_flux_R_z     = nullptr;
    double*   d_wave_vel_L_x = nullptr;
    double*   d_wave_vel_L_y = nullptr;
    double*   d_wave_vel_L_z = nullptr;
    double*   d_wave_vel_R_x = nullptr;
    double*   d_wave_vel_R_y = nullptr;
    double*   d_wave_vel_R_z = nullptr;
    double*   d_delta_j_L_x  = nullptr;
    double*   d_delta_j_L_y  = nullptr;
    double*   d_delta_j_L_z  = nullptr;
    double*   d_delta_j_R_x  = nullptr;
    double*   d_delta_j_R_y  = nullptr;
    double*   d_delta_j_R_z  = nullptr;

    // --- Selective damping mask ---
    uint8_t*  d_near_particle = nullptr;
    double*   d_near_accel    = nullptr;  // max accel_mag of nearby particles (for Larmor)

    // --- FFT workspace ---
    cufftDoubleComplex* d_fft_buf   = nullptr;  // N complex doubles (legacy, kept for reference)
    cufftComplex*       d_fft_buf_f = nullptr;  // N complex floats (primary — 2× faster C2C)
    double*             d_green     = nullptr;   // precomputed 1/G(k) (double precision, computed once)

    // --- cuRAND workspace ---
    double*   d_random      = nullptr;  // N uniform random doubles

    // --- Particle list (compact indices of manifested particles) ---
    // Scales with lattice: enough for ~1.5% occupation at any size
    static constexpr int MAX_PARTICLES = 8192;
    int*      d_plist_idx     = nullptr;  // lattice indices [MAX_PARTICLES]
    int*      d_num_particles = nullptr;  // count (single int on device)

    // --- Pair production tracking ---
    int32_t*  d_pair_id       = nullptr;  // pair ID (-1 = unpaired) [N]

    // Lifecycle
    void allocate(int lattice_size);
    void free();

    // AoS ↔ SoA transfers
    void upload(const std::vector<Voxel>& host_voxels,
                const std::vector<double>& host_phi,
                const std::vector<double>& host_phi_coulomb);

    void download(std::vector<Voxel>& host_voxels,
                  std::vector<double>& host_phi,
                  std::vector<double>& host_phi_coulomb) const;

    // Upload only state + flux (for inject_particle / inject_wavepacket)
    void upload_voxels(const std::vector<Voxel>& host_voxels);

    // Download only voxels (for diagnostics)
    void download_voxels(std::vector<Voxel>& host_voxels) const;

    // Download phi_latency from device (Wave 5: GPU latency Poisson)
    void download_phi_latency(std::vector<double>& out) const;

    // Precompute Green's function for FFT Poisson solver
    void precompute_green_function();
};

}  // namespace gpu
}  // namespace ftd
