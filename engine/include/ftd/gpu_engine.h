#pragma once
/**
 * GPU-Accelerated FTD Engine
 *
 * Drop-in alternative to RenderBridge that executes the tick cycle
 * on NVIDIA GPU via CUDA. All field data lives on the device; host
 * transfers happen only for diagnostics or injection.
 *
 * Requires: CUDA Toolkit 12.8+, cuFFT, cuRAND
 * Target: GPU (SM 120, 32 GB VRAM)
 */

#include "voxel.h"
#include "render_bridge.h"  // for Diagnostics, EnergyAudit, TermToggles
#include "gpu_buffers.h"
#include <vector>
#include <cufft.h>
#include <curand.h>

namespace ftd {
namespace gpu {

class GpuEngine {
public:
    explicit GpuEngine(int lattice_size);
    ~GpuEngine();

    // Non-copyable
    GpuEngine(const GpuEngine&) = delete;
    GpuEngine& operator=(const GpuEngine&) = delete;

    // --- Core simulation ---
    void tick();
    void run(int num_ticks);

    // --- Diagnostics (downloads from GPU) ---
    Diagnostics diagnostics();
    EnergyAudit energy_audit();

    // --- Particle injection (uploads to GPU) ---
    void inject_flux(int x, int y, int z, const Vec3& flux_val);
    void inject_particle(int x, int y, int z, int8_t state,
                         const Vec3& flux_val,
                         int8_t spin = 0, int8_t color = 0);
    void inject_wavepacket(int cx, int cy, int cz, int8_t state,
                           double sigma = 3.0, double amplitude = K_B);

    // --- Sync to host for inspection ---
    void sync_to_host(std::vector<Voxel>& out);

    // --- Bulk upload from host (for test setup with custom initial conditions) ---
    void upload_from_host(const std::vector<Voxel>& voxels);

    // --- Accessors ---
    int lattice_size() const { return size_; }
    int current_tick() const { return tick_; }
    int total_sites() const { return N_; }

    // Access Coulomb potential (downloads from GPU if stale)
    const std::vector<double>& phi_coulomb() { ensure_host_synced(); return host_phi_coulomb_; }

    // Physics toggles (same as CPU engine)
    TermToggles toggles;

private:
    // GPU tick sub-phases
    void gpu_phase_read();
    void gpu_phase_write();
    void gpu_wave_update();  // fused read+write (single-substrate only)
    void gpu_gauss_project();
    void gpu_solve_coulomb();
    void gpu_phase_forces();
    void gpu_phase_movement();

    // Extended physics sub-phases
    void gpu_weak_transmutation();
    void gpu_build_particle_list();
    void gpu_particle_forces();
    void gpu_triad_detection();
    void gpu_pair_production();

    int size_;              // lattice side length
    int N_;                 // total sites (size^3)
    int tick_ = 0;
    double dt_ = 1.0;

    GpuBuffers bufs_;

    // cuFFT plans (created once, reused every tick)
    cufftHandle fft_plan_forward_  = 0;   // Z2Z double (legacy)
    cufftHandle fft_plan_inverse_  = 0;   // Z2Z double (legacy)
    cufftHandle fft_plan_forward_f_ = 0;  // C2C float (primary — 2× faster)
    cufftHandle fft_plan_inverse_f_ = 0;  // C2C float (primary — 2× faster)

    // cuRAND generator (for genesis probability)
    curandGenerator_t rng_ = nullptr;

    // Host-side shadow for injection and diagnostics
    // Lazily allocated on first use
    std::vector<Voxel> host_voxels_;
    std::vector<double> host_phi_;
    std::vector<double> host_phi_coulomb_;
    bool host_dirty_ = true;  // true = device has newer data than host

    int next_particle_id_ = 0;
    int next_pair_id_ = 0;
    int host_num_particles_ = 0;  // cached particle count from device

    // Helper: ensure host shadow is up-to-date
    void ensure_host_synced();
    // Helper: push host changes to device
    void push_to_device();
};

}  // namespace gpu
}  // namespace ftd
