#pragma once
/**
 * SoA (Structure-of-Arrays) device buffers for GPU-accelerated FTD engine.
 *
 * The CPU engine uses AoS (Voxel struct ~154 bytes). For GPU memory
 * coalescence, we decompose into separate arrays per field.
 * Upload/download functions convert between AoS (host) and SoA (device).
 */

#include "voxel.h"
#include <cstddef>   // std::size_t
#include <cstdint>   // uint8_t etc. — Linux/clang require explicit include
#include <vector>
#include <cufft.h>

namespace ftd {
namespace gpu {

// ─── C5 (CUDA ticket): host→device upload instrumentation + test knob ───────
// g_gpu_upload_bytes accumulates the bytes actually memcpy'd host→device by
// upload_voxels_range() — i.e. by BOTH the full and the delta upload paths —
// so campaigns/tests can record transfer volume before/after. Reset it to 0
// before a measured operation and read it afterwards.
//
// g_gpu_force_full_upload forces upload_voxels_delta() to fall back to a full
// upload. It exists ONLY so test_gpu_delta_upload can capture the pre-C5
// (full-upload) reference and prove the delta path is byte-identical. Default
// false; pure host-side state that never changes any device byte, so it is
// golden-neutral.
extern std::size_t g_gpu_upload_bytes;
extern bool        g_gpu_force_full_upload;

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
    int8_t*   d_flavor      = nullptr;
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

    // --- Strong Substrate Field (Stella Octangula) ---
    // Note: strong_field_stencil_kernel writes wvs_* / fs_* in-place (leapfrog
    // fuses the delta_j accumulator into the velocity update), so no separate
    // d_delta_j_strong_* buffers are needed. Same for weak below.
    double*   d_flux_strong_x     = nullptr;
    double*   d_flux_strong_y     = nullptr;
    double*   d_flux_strong_z     = nullptr;
    double*   d_wave_vel_strong_x = nullptr;
    double*   d_wave_vel_strong_y = nullptr;
    double*   d_wave_vel_strong_z = nullptr;

    // --- Weak Substrate Field (Cuboctahedron) ---
    double*   d_flux_weak_x     = nullptr;
    double*   d_flux_weak_y     = nullptr;
    double*   d_flux_weak_z     = nullptr;
    double*   d_wave_vel_weak_x = nullptr;
    double*   d_wave_vel_weak_y = nullptr;
    double*   d_wave_vel_weak_z = nullptr;

    // --- Selective damping mask ---
    uint8_t*  d_near_particle = nullptr;
    double*   d_near_accel    = nullptr;  // max accel_mag of nearby particles (for Larmor)

    // --- Per-site force diagnostics (mirror of CPU RenderBridge::force_diag_) ---
    // Five components × 3 axes, indexed by lattice site. Populated by the
    // force kernels (phase_forces, color_force) so GpuBackend::sync_to_host()
    // can scatter them back into RenderBridge::force_diag_. Allocated
    // unconditionally — modest cost (15 doubles × N ≈ 1.9 MB at L=64,
    // 122 MB at L=256) — keeps the kernel signature simple.
    double*   d_fd_coulomb_x  = nullptr;
    double*   d_fd_coulomb_y  = nullptr;
    double*   d_fd_coulomb_z  = nullptr;
    double*   d_fd_strong_x   = nullptr;
    double*   d_fd_strong_y   = nullptr;
    double*   d_fd_strong_z   = nullptr;
    double*   d_fd_magnetic_x = nullptr;
    double*   d_fd_magnetic_y = nullptr;
    double*   d_fd_magnetic_z = nullptr;
    double*   d_fd_gravity_x  = nullptr;
    double*   d_fd_gravity_y  = nullptr;
    double*   d_fd_gravity_z  = nullptr;
    double*   d_fd_exchange_x = nullptr;
    double*   d_fd_exchange_y = nullptr;
    double*   d_fd_exchange_z = nullptr;

    // Per-tick count of movement-entry repairs for externally mutated
    // out-of-budget velocities (FTD-0402). Normal force evolution leaves zero.
    unsigned long long* d_causal_projection_events = nullptr;

    // --- FFT workspace ---
    // Both precisions are active: float (C2C) is the default 2× faster path;
    // double (Z2Z) is used by high-accuracy callsites in kernels_poisson.cu.
    cufftDoubleComplex* d_fft_buf   = nullptr;  // N complex doubles (high-accuracy path)
    cufftComplex*       d_fft_buf_f = nullptr;  // N complex floats (default, 2× faster C2C)
    double*             d_green     = nullptr;   // precomputed 1/G(k) (double precision, computed once)



    // --- Particle list (compact indices of manifested particles) ---
    // Scales with lattice: enough for ~1.5% occupation at any size
    static constexpr int MAX_PARTICLES = 8192;
    int*      d_plist_idx     = nullptr;  // lattice indices [MAX_PARTICLES]
    int*      d_num_particles = nullptr;  // count (single int on device)

    // --- Pair production tracking ---
    int32_t*  d_pair_id       = nullptr;  // pair ID (-1 = unpaired) [N]

    // --- Native EFT continuity event ledger ---
    // Reset immediately before GPU movement. Kernels write integrated
    // one-tick currents/reactions directly, avoiding host snapshot inference.
    int*      d_ledger_rho_before = nullptr;
    int*      d_ledger_reaction   = nullptr;
    double*   d_ledger_current_x  = nullptr;
    double*   d_ledger_current_y  = nullptr;
    double*   d_ledger_current_z  = nullptr;

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

    // Upload all voxel fields for the whole lattice (equivalent to
    // upload_voxels_range(host_voxels, 0, N)). Used by inject_particle /
    // inject_wavepacket / the full-upload fallback.
    void upload_voxels(const std::vector<Voxel>& host_voxels);

    // Upload the AoS voxel fields for the contiguous index range [lo, lo+count)
    // into the SoA device arrays. The field set copied here is the SINGLE
    // SOURCE OF TRUTH for a voxel upload; upload_voxels() is the lo=0,count=N
    // case and the C5 delta path (upload_voxels_delta) calls this once per
    // contiguous dirty run. Adds count·sizeof(voxel-fields) to
    // g_gpu_upload_bytes.
    void upload_voxels_range(const std::vector<Voxel>& host_voxels,
                             int lo, int count);

    // C5: partial host→device upload. `shadow` is the host mirror of the
    // CURRENT device SoA state (the caller guarantees device == shadow at every
    // index). Diffs `host_voxels` against `shadow`, uploads only the changed
    // voxels (coalesced into contiguous runs), and is byte-identical to
    // upload_voxels(host_voxels) because every unchanged index already holds
    // the correct bytes. Falls back to a full upload on cold start
    // (shadow.size()!=N), when a large fraction changed, or when
    // g_gpu_force_full_upload is set.
    void upload_voxels_delta(const std::vector<Voxel>& host_voxels,
                             const std::vector<Voxel>& shadow);

    // Download only voxels (for diagnostics)
    void download_voxels(std::vector<Voxel>& host_voxels) const;

    // Download phi_latency from device (Wave 5: GPU latency Poisson)
    void download_phi_latency(std::vector<double>& out) const;

    // Download per-site force diagnostics. Each output vector is resized to N
    // and filled in voxel-major order so callers can scatter into
    // RenderBridge::force_diag_[i] directly (one ForceDiag per site).
    void download_force_diag(std::vector<double>& fc_x, std::vector<double>& fc_y, std::vector<double>& fc_z,
                             std::vector<double>& fs_x, std::vector<double>& fs_y, std::vector<double>& fs_z,
                             std::vector<double>& fm_x, std::vector<double>& fm_y, std::vector<double>& fm_z,
                             std::vector<double>& fg_x, std::vector<double>& fg_y, std::vector<double>& fg_z,
                             std::vector<double>& fe_x, std::vector<double>& fe_y, std::vector<double>& fe_z) const;

    // Zero all force-diag arrays (called once per tick before force kernels).
    void reset_force_diag();
    unsigned long long download_causal_projection_events() const;

    // Native EFT continuity event ledger helpers
    void reset_continuity_ledger();
    void download_continuity_ledger(std::vector<int>& rho_before,
                                    std::vector<int>& rho_after,
                                    std::vector<int>& reaction,
                                    std::vector<double>& current_x,
                                    std::vector<double>& current_y,
                                    std::vector<double>& current_z) const;

    // Precompute Green's function for FFT Poisson solver
    void precompute_green_function();
};

}  // namespace gpu
}  // namespace ftd
