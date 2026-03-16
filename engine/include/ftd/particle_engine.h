#pragma once
/**
 * ParticleEngine: Scale 1 simulation
 *
 * Phase 7: Lattice-free engine with continuous positions and analytical forces.
 * All constants from ontic.h. Velocity Verlet integration (symplectic).
 *
 * Force convention (matches Scale 0 Poisson solver):
 *   F_EM  = alpha * q_i * q_j * r_hat / (4*pi * (r^2 + soft^2))
 *   F_grav = G_N * m_i * m_j * r_hat / (r^2 + soft^2)
 *
 * Like signs repel, opposite signs attract (Coulomb).
 * Gravity is always attractive.
 *
 * Phase 8: ParticleToggles struct + ParticleForceDiag for per-force decomposition.
 * Mirrors Scale 0 TermToggles pattern. Future forces (Phase 2) stubbed as toggles.
 */

#include "voxel.h"       // Vec3
#include "constants.h"   // ALPHA, G_N, K_B, PI, C_SPEED, DAMPING
#include "scale.h"       // OnticEntity, ScaleLevel
#include <vector>
#include <cstdint>

namespace ftd {

// ============================================================================
// Toggle struct — controls which forces are active (mirrors TermToggles pattern)
// ============================================================================

struct ParticleToggles {
    // Currently implemented
    bool coulomb = true;
    bool gravity = true;
    bool damping = true;

    // Phase 2 stubs (OFF by default until implemented)
    bool lorentz = false;
    bool exchange = false;
    bool strong = false;
    bool radiation = false;
    bool spin_orbit = false;
    bool relativistic = false;
    bool magnetic_dipole = false;

    void enable_all() {
        coulomb = gravity = damping = true;
        lorentz = exchange = strong = radiation = true;
        spin_orbit = relativistic = magnetic_dipole = true;
    }
    void minimal() {
        coulomb = gravity = damping = true;
        lorentz = exchange = strong = radiation = false;
        spin_orbit = relativistic = magnetic_dipole = false;
    }
};

// ============================================================================
// Per-particle force decomposition (for diagnostics + visualization)
// ============================================================================

struct ParticleForceDiag {
    Vec3 f_coulomb;
    Vec3 f_gravity;
    Vec3 f_lorentz;
    Vec3 f_exchange;
    Vec3 f_strong;
    Vec3 f_radiation;
    Vec3 f_spin_orbit;
    Vec3 f_relativistic;
    Vec3 f_magnetic_dipole;

    Vec3 total() const {
        return f_coulomb + f_gravity + f_lorentz + f_exchange + f_strong
             + f_radiation + f_spin_orbit + f_relativistic + f_magnetic_dipole;
    }
};

// ============================================================================
// Particle struct
// ============================================================================

struct Particle {
    int32_t id = -1;
    int8_t charge = 0;        // State: +1 or -1
    double mass = K_B;         // Energy (default = electron mass)
    double r_eff = 2.48;       // Boundary (effective radius from Phase 6)
    // NOTE: KCOMP shell at 128³ measures r_eff ≈ 11.61 (DERIV_KCOMP_VOLUMETRIC_SHELL.md).
    // The 2.48 default is from the old CFL speed era. ParticleEngine (Scale 1) uses
    // analytical forces, not lattice dynamics, so r_eff is only used for annihilation
    // distance. A dynamic measurement would be more accurate but isn't critical here.
    Vec3 position;             // Continuous coordinates (lattice units)
    Vec3 velocity;
    Vec3 acceleration;         // Current acceleration (for Verlet)
    Vec3 prev_acceleration;    // Previous tick acceleration (for radiation reaction)
    int8_t spin = 0;
    int8_t color = 0;
    int32_t pair_id = -1;      // Entanglement partner
    bool locked = false;       // Infinite mass (e.g., proton in hydrogen)
    Vec3 spin_axis;            // Spin direction (for magnetic/exchange forces)

    // Convert to universal ternary triple
    OnticEntity as_ontic() const {
        return {charge, mass, r_eff};
    }
};

// ============================================================================
// Diagnostics
// ============================================================================

struct ParticleDiagnostics {
    int tick = 0;
    int particle_count = 0;
    double total_ke = 0.0;       // sum 0.5 * m * v^2
    double total_pe = 0.0;       // sum alpha * qi * qj / (4*pi*rij)
    double total_energy = 0.0;   // KE + PE
    Vec3 total_momentum;         // sum m * v
    Vec3 total_angular_momentum; // sum r x (m*v)
};

// ============================================================================
// ParticleEngine
// ============================================================================

class ParticleEngine {
public:
    ParticleEngine();

    // Toggle struct — public for direct access (like TermToggles on RenderBridge)
    ParticleToggles toggles;

    // Add a particle, returns its assigned id
    int add_particle(int8_t charge, Vec3 position, Vec3 velocity = {},
                     double mass = K_B, double r_eff = 2.48,
                     int8_t spin = 0, int8_t color = 0);

    // Add a locked (infinite mass) particle — does not move
    int add_locked_particle(int8_t charge, Vec3 position, double mass = K_B,
                            int8_t spin = 0, int8_t color = 0);

    // Access
    std::vector<Particle>& particles() { return particles_; }
    const std::vector<Particle>& particles() const { return particles_; }
    int current_tick() const { return tick_; }
    double dt() const { return dt_; }
    void set_dt(double dt) { dt_ = dt; }
    double softening() const { return soft_; }
    void set_softening(double s) { soft_ = s; }

    // Backward-compatible toggle accessors (delegate to toggles struct)
    bool damping_enabled() const { return toggles.damping; }
    void set_damping_enabled(bool e) { toggles.damping = e; }
    bool gravity_enabled() const { return toggles.gravity; }
    void set_gravity_enabled(bool e) { toggles.gravity = e; }

    // Per-particle force decomposition (populated after compute_all_forces)
    const std::vector<ParticleForceDiag>& force_diag() const { return force_diag_; }

    // Advance one time step (Velocity Verlet)
    void tick();

    // Advance N time steps
    void run(int num_ticks);

    // Compute diagnostics for current state
    ParticleDiagnostics diagnostics() const;

    // Compute force on particle i from all others
    Vec3 compute_force(int i) const;

private:
    // Velocity Verlet integration
    void compute_all_forces();
    void half_kick();
    void drift();
    void check_annihilation();
    void enforce_speed_limit();
    void apply_damping();

    std::vector<Particle> particles_;
    std::vector<Vec3> forces_;             // Total force buffer (parallel to particles_)
    mutable std::vector<ParticleForceDiag> force_diag_;  // Per-force decomposition (mutable: written by const compute_force)
    int tick_ = 0;
    int next_id_ = 0;
    double dt_ = 1.0;           // Time step (default 1, can increase for Scale 1)
    double soft_ = 1.0;         // Softening length (matches lattice minimum)
};

}  // namespace ftd
