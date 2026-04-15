#pragma once
/**
 * GPU-accelerated AtomEngine backend (Wave 5.3 Phase 1).
 *
 * Drop-in acceleration for ftd::AtomEngine::compute_all_forces() +
 * integration. When enabled, the pair-force hot loop (ionic + van der
 * Waals) runs as an O(N²) CUDA kernel on the device, then downloads
 * results back to host. Multi-body forces (bonds, angle strain,
 * dipole-dipole, thermostat) still run on CPU in Phase 1.
 *
 * Phase 2 will port bond/angle/dipole/thermostat kernels.
 * Phase 3 will port Barnes-Hut octree for O(N log N) at large N.
 *
 * Design: pure additive. Includes are #ifdef FTD_ENABLE_CUDA — no
 * new runtime dependencies when CUDA is disabled.
 */

#ifdef FTD_ENABLE_CUDA

#include "voxel.h"        // Vec3
#include <vector>
#include <cstdint>
#include <memory>

namespace ftd {

struct Atom;                  // forward declaration from atom_engine.h
struct AtomToggles;
struct AtomForceDiag;

namespace gpu {

// ============================================================================
// SoA device buffers for Atom fields
// ============================================================================

struct AtomBuffers {
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

    // Scalar per-atom fields used by pair forces
    double*  d_mass     = nullptr;
    int32_t* d_charge   = nullptr;  // net ionic charge (from Atom.charge)
    double*  d_radius   = nullptr;
    double*  d_vdw_eps  = nullptr;
    double*  d_vdw_sig  = nullptr;
    uint8_t* d_locked   = nullptr;

    // Diagnostics shadow (for pulling force decomposition back to host)
    double*  d_f_ionic_x = nullptr;
    double*  d_f_ionic_y = nullptr;
    double*  d_f_ionic_z = nullptr;
    double*  d_f_vdw_x   = nullptr;
    double*  d_f_vdw_y   = nullptr;
    double*  d_f_vdw_z   = nullptr;

    void allocate(int capacity);
    void free();
    void ensure_capacity(int n);

    // Upload from AoS host atoms to SoA device buffers. Sets N.
    void upload_atoms(const std::vector<Atom>& host_atoms);

    // Download per-atom force components back into a user vector
    // (SoA on device → AoS on host).
    void download_forces(std::vector<Vec3>& out_forces,
                         std::vector<AtomForceDiag>& out_diag) const;

    // Download updated positions + velocities back to host atoms after
    // GPU-side integration steps.
    void download_kinematics(std::vector<Atom>& host_atoms) const;
};

// ============================================================================
// gpu::AtomEngine — host wrapper around the SoA buffers and CUDA kernel
// launchers. Not a full ftd::AtomEngine replacement; designed to be called
// from inside ftd::AtomEngine::compute_all_forces() as a drop-in accelerator
// for the pair-force hot loop.
// ============================================================================

class AtomEngineGpu {
public:
    AtomEngineGpu();
    ~AtomEngineGpu();

    AtomEngineGpu(const AtomEngineGpu&) = delete;
    AtomEngineGpu& operator=(const AtomEngineGpu&) = delete;

    // Upload current atom state, run O(N²) pair-force kernel with the
    // given toggle set, download forces into out_forces + out_diag.
    void compute_pair_forces(const std::vector<Atom>& host_atoms,
                             const AtomToggles& toggles,
                             double soft,
                             std::vector<Vec3>& out_forces,
                             std::vector<AtomForceDiag>& out_diag);

    // For future phases
    bool supports_angle_strain()  const { return false; }  // Phase 2
    bool supports_dipole_dipole() const { return false; }  // Phase 2
    bool supports_thermostat()    const { return false; }  // Phase 2

private:
    std::unique_ptr<AtomBuffers> bufs_;
};

}  // namespace gpu
}  // namespace ftd

#endif  // FTD_ENABLE_CUDA
