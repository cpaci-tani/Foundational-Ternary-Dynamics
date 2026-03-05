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
 */

#include "voxel.h"       // Vec3
#include "constants.h"   // ALPHA, G_N, K_B, PI, C_SPEED, DAMPING
#include "scale.h"       // OnticEntity, ScaleLevel
#include <vector>
#include <cstdint>

namespace ftd {

struct Particle {
    int32_t id = -1;
    int8_t charge = 0;        // State: +1 or -1
    double mass = K_B;         // Energy (default = electron mass)
    double r_eff = 2.48;       // Boundary (effective radius from Phase 6)
    Vec3 position;             // Continuous coordinates (lattice units)
    Vec3 velocity;
    Vec3 acceleration;         // Current acceleration (for Verlet)
    int8_t spin = 0;
    int8_t color = 0;
    int32_t pair_id = -1;      // Entanglement partner
    bool locked = false;       // Infinite mass (e.g., proton in hydrogen)

    // Convert to universal ternary triple
    OnticEntity as_ontic() const {
        return {charge, mass, r_eff};
    }
};

struct ParticleDiagnostics {
    int tick = 0;
    int particle_count = 0;
    double total_ke = 0.0;       // sum 0.5 * m * v^2
    double total_pe = 0.0;       // sum alpha * qi * qj / (4*pi*rij)
    double total_energy = 0.0;   // KE + PE
    Vec3 total_momentum;         // sum m * v
    Vec3 total_angular_momentum; // sum r x (m*v)
};

class ParticleEngine {
public:
    ParticleEngine();

    // Add a particle, returns its assigned id
    int add_particle(int8_t charge, Vec3 position, Vec3 velocity = {},
                     double mass = K_B, double r_eff = 2.48);

    // Add a locked (infinite mass) particle — does not move
    int add_locked_particle(int8_t charge, Vec3 position, double mass = K_B);

    // Access
    std::vector<Particle>& particles() { return particles_; }
    const std::vector<Particle>& particles() const { return particles_; }
    int current_tick() const { return tick_; }
    double dt() const { return dt_; }
    void set_dt(double dt) { dt_ = dt; }
    double softening() const { return soft_; }
    void set_softening(double s) { soft_ = s; }
    bool damping_enabled() const { return damping_enabled_; }
    void set_damping_enabled(bool e) { damping_enabled_ = e; }
    bool gravity_enabled() const { return gravity_enabled_; }
    void set_gravity_enabled(bool e) { gravity_enabled_ = e; }

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
    std::vector<Vec3> forces_;  // Force buffer (parallel to particles_)
    int tick_ = 0;
    int next_id_ = 0;
    double dt_ = 1.0;           // Time step (default 1, can increase for Scale 1)
    double soft_ = 1.0;         // Softening length (matches lattice minimum)
    bool damping_enabled_ = true;
    bool gravity_enabled_ = true;
};

}  // namespace ftd
