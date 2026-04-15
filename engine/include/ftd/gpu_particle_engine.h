#pragma once
/**
 * GPU-accelerated ParticleEngine backend (Wave 5.4 Phase 1).
 *
 * Drop-in acceleration for ftd::ParticleEngine::compute_all_forces().
 * When enabled, the pair-force hot loop (Coulomb + Newtonian gravity)
 * runs as an O(N²) CUDA kernel on the device, then downloads results
 * back to host. Non-pairwise terms (radiation reaction, relativistic
 * correction) and exotic pair forces (strong, exchange, lorentz,
 * magnetic_dipole, spin_orbit) still run on CPU in Phase 1.
 *
 * Phase 2 will port strong + exchange + lorentz + magnetic_dipole +
 *         spin_orbit pair kernels.
 * Phase 3 will port Barnes-Hut octree for O(N log N) at large N.
 *
 * Design: pure additive. Includes are #ifdef FTD_ENABLE_CUDA — no new
 * runtime dependencies when CUDA is disabled.
 */

#ifdef FTD_ENABLE_CUDA

#include "voxel.h"        // Vec3
#include <vector>
#include <cstdint>
#include <memory>

namespace ftd {

struct Particle;                  // forward declaration from particle_engine.h
struct ParticleToggles;
struct ParticleForceDiag;

namespace gpu {

// ============================================================================
// SoA device buffers for Particle fields
// ============================================================================

struct ParticleBuffers {
    int N_alloc = 0;  // allocated capacity
    int N = 0;        // currently-used count (from host)

    // Position / velocity / force (double × 3)
    double* d_pos_x   = nullptr;
    double* d_pos_y   = nullptr;
    double* d_pos_z   = nullptr;
    double* d_vel_x   = nullptr;
    double* d_vel_y   = nullptr;
    double* d_vel_z   = nullptr;
    double* d_force_x = nullptr;
    double* d_force_y = nullptr;
    double* d_force_z = nullptr;

    // Scalar per-particle fields used by pair forces
    double*  d_mass    = nullptr;
    int8_t*  d_charge  = nullptr;
    uint8_t* d_locked  = nullptr;

    // Diagnostics shadow (for pulling force decomposition back to host)
    double*  d_f_coulomb_x = nullptr;
    double*  d_f_coulomb_y = nullptr;
    double*  d_f_coulomb_z = nullptr;
    double*  d_f_gravity_x = nullptr;
    double*  d_f_gravity_y = nullptr;
    double*  d_f_gravity_z = nullptr;

    void allocate(int capacity);
    void free();
    void ensure_capacity(int n);

    // Upload from AoS host particles to SoA device buffers. Sets N.
    void upload_particles(const std::vector<Particle>& host_particles);

    // Download per-particle force components back into a user vector
    // (SoA on device → AoS on host).
    void download_forces(std::vector<Vec3>& out_forces,
                         std::vector<ParticleForceDiag>& out_diag) const;
};

// ============================================================================
// gpu::ParticleEngineGpu — host wrapper around the SoA buffers and CUDA kernel
// launchers. Not a full ftd::ParticleEngine replacement; designed to be called
// from inside ftd::ParticleEngine::compute_all_forces() as a drop-in
// accelerator for the pair-force hot loop.
// ============================================================================

class ParticleEngineGpu {
public:
    ParticleEngineGpu();
    ~ParticleEngineGpu();

    ParticleEngineGpu(const ParticleEngineGpu&) = delete;
    ParticleEngineGpu& operator=(const ParticleEngineGpu&) = delete;

    // Upload current particle state, run O(N²) pair-force kernel with the
    // given toggle set, download forces into out_forces + out_diag.
    //
    // Phase 1 kernel handles: coulomb, gravity.
    // Other toggles are silently ignored (host code must have checked first
    // and fallen back to the CPU Barnes-Hut path).
    void compute_pair_forces(const std::vector<Particle>& host_particles,
                             const ParticleToggles& toggles,
                             double soft,
                             std::vector<Vec3>& out_forces,
                             std::vector<ParticleForceDiag>& out_diag);

    // For future phases
    bool supports_strong()          const { return false; }  // Phase 2
    bool supports_exchange()        const { return false; }  // Phase 2
    bool supports_lorentz()         const { return false; }  // Phase 2
    bool supports_magnetic_dipole() const { return false; }  // Phase 2
    bool supports_spin_orbit()      const { return false; }  // Phase 2
    bool supports_radiation()       const { return false; }  // Phase 3 (self-force)
    bool supports_relativistic()    const { return false; }  // Phase 3 (post-process)

private:
    std::unique_ptr<ParticleBuffers> bufs_;
};

}  // namespace gpu
}  // namespace ftd

#endif  // FTD_ENABLE_CUDA
