#pragma once
/**
 * AtomEngine: Scale 2 simulation
 *
 * Atoms as composite objects with inter-atomic forces:
 *   Ionic:      F = -ALPHA * Q_i * Q_j / (4*pi * r²_soft) * r_hat
 *   Van der Waals: Lennard-Jones 12-6 with eps/sigma from ontic chain
 *   Covalent:   Harmonic bond F = -k*(r - r_eq) * r_hat
 *
 * All force parameters derive from ontic constants {ALPHA, K_B, N_BASE, R_BOHR}.
 * Velocity Verlet integration (same symplectic integrator as ParticleEngine).
 * No gravity — alpha_G ~ 6e-39 is negligible at atomic scales.
 */

#include "voxel.h"       // Vec3
#include "constants.h"   // ALPHA, K_B, PI, C_SPEED, DAMPING, R_BOHR, N_BASE, etc.
#include "scale.h"       // OnticEntity, ScaleLevel
#include <vector>
#include <cstdint>
#include <cmath>

namespace ftd {

// ============================================================================
// Atomic property helper — computes mass, radius, vdW params from Z
// ============================================================================

struct AtomicProperties {
    double mass;          // total mass in MeV
    double radius;        // effective radius (FTD Bohr-scaled)
    double vdw_epsilon;   // LJ well depth (from α² perturbation theory)
    double vdw_sigma;     // LJ zero-crossing distance
    int max_bonds;        // maximum covalent bonds (valence)
};

/// Compute atomic properties from atomic number Z and neutron count N.
/// All parameters derive from ontic constants — no free parameters.
///
///   mass    = Z * M_PROTON + N * M_PROTON * (1 + ALPHA)  [neutron heavier by ~α]
///   radius  = R_BOHR / Z^(1/3)
///   vdw_eps = K_B * ALPHA^2 * Z^(2/3) / (4*PI)  [2nd-order EM]
///   vdw_sig = radius * N_BASE  [electron cloud extends ~4x nucleus]
inline AtomicProperties compute_atomic_properties(int Z, int N = 0) {
    AtomicProperties p;

    // Mass: Z protons + N neutrons (neutron ~(1+α) × proton mass)
    p.mass = Z * M_PROTON + N * M_PROTON * (1.0 + ALPHA);

    // Radius: Bohr-scaled by Z^(1/3) (Thomas-Fermi screening)
    double z_cbrt = std::cbrt(static_cast<double>(Z));
    p.radius = (z_cbrt > 0.0) ? R_BOHR / z_cbrt : R_BOHR;

    // Van der Waals epsilon: 2nd-order perturbation in α
    p.vdw_epsilon = K_B * ALPHA * ALPHA * std::pow(static_cast<double>(Z), 2.0/3.0)
                    / (4.0 * PI);

    // Van der Waals sigma: electron cloud size
    p.vdw_sigma = p.radius * N_BASE;

    // Max bonds from periodic table lookup (typical maximum covalent bonds)
    // Noble gases (group 18): 0, Alkali metals (group 1): 1, etc.
    // Transition metals: typical coordination, not necessarily max oxidation state
    static constexpr int max_bonds_table[119] = {
    //  0   1   2   3   4   5   6   7   8   9
        0,  1,  0,  1,  2,  3,  4,  3,  2,  1,  // Z=0-9   (0 unused)
        0,  1,  2,  3,  4,  3,  2,  1,  0,  1,  // Z=10-19
        2,  3,  4,  5,  6,  4,  3,  3,  3,  2,  // Z=20-29
        2,  3,  4,  3,  2,  1,  0,  1,  2,  3,  // Z=30-39
        4,  5,  6,  4,  4,  3,  4,  1,  2,  3,  // Z=40-49
        4,  3,  2,  1,  0,  1,  2,  3,  4,  4,  // Z=50-59
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  // Z=60-69
        3,  3,  4,  5,  6,  4,  4,  4,  4,  3,  // Z=70-79
        2,  3,  4,  3,  2,  1,  0,  1,  2,  3,  // Z=80-89
        4,  5,  6,  5,  4,  3,  3,  3,  3,  3,  // Z=90-99
        3,  3,  2,  3,  4,  5,  6,  4,  4,  3,  // Z=100-109
        3,  3,  2,  3,  4,  3,  2,  1,  0        // Z=110-118
    };
    p.max_bonds = (Z >= 1 && Z <= 118) ? max_bonds_table[Z] : ((Z <= 2) ? Z : 4);

    return p;
}

// ============================================================================
// Bond structure
// ============================================================================

struct Bond {
    int partner_id = -1;   // ID of bonded partner atom
    double r_eq;           // equilibrium bond length
    double k_bond;         // spring constant = ALPHA * K_B / r_eq²
    int order = 1;         // 1=single, 2=double, 3=triple
};

// ============================================================================
// Atom structure — OnticEntity triple: {Z, mass, radius}
// ============================================================================

struct Atom {
    int32_t id = -1;
    int Z = 0;                  // Atomic number (= OnticEntity.state)
    int N = 0;                  // Neutron count
    int charge = 0;             // Net ionic charge (0 = neutral)
    double mass = 0.0;          // Total mass (MeV) (= OnticEntity.energy)
    double radius = 0.0;        // Effective radius (= OnticEntity.boundary)
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    bool locked = false;

    // Valence / bonding
    int valence_electrons = 0;
    int max_bonds = 0;
    std::vector<Bond> bonds;

    // Cached van der Waals parameters
    double vdw_epsilon = 0.0;
    double vdw_sigma = 0.0;

    // Convert to universal ternary triple
    OnticEntity as_ontic() const {
        return {Z, mass, radius};
    }
};

// ============================================================================
// Diagnostics
// ============================================================================

struct AtomDiagnostics {
    int tick = 0;
    int atom_count = 0;
    int bond_count = 0;
    double total_ke = 0.0;
    double total_pe_ionic = 0.0;
    double total_pe_vdw = 0.0;
    double total_pe_bond = 0.0;
    double total_energy = 0.0;    // KE + all PE terms
    Vec3 total_momentum;
    double temperature = 0.0;     // T = 2*KE / (3*N*k_B) in FTD units
};

// ============================================================================
// AtomEngine — Scale 2 simulation
// ============================================================================

class AtomEngine {
public:
    AtomEngine();

    /// Add an atom. Returns assigned id.
    /// Properties (mass, radius, vdW params) computed from Z, N automatically.
    int add_atom(int Z, Vec3 position, Vec3 velocity = {},
                 int charge = 0, int N = -1);

    /// Add a locked (immobile) atom. Returns assigned id.
    int add_locked_atom(int Z, Vec3 position, int charge = 0, int N = -1);

    /// Create a covalent bond between two atoms by id.
    /// Returns true if bond was created, false if already bonded or invalid.
    bool create_bond(int id_a, int id_b, int order = 1);

    /// Remove bond between two atoms by id.
    bool remove_bond(int id_a, int id_b);

    // Access
    std::vector<Atom>& atoms() { return atoms_; }
    const std::vector<Atom>& atoms() const { return atoms_; }
    int current_tick() const { return tick_; }
    double dt() const { return dt_; }
    void set_dt(double d) { dt_ = d; }
    double softening() const { return soft_; }
    void set_softening(double s) { soft_ = s; }
    bool damping_enabled() const { return damping_enabled_; }
    void set_damping_enabled(bool e) { damping_enabled_ = e; }
    bool bonding_enabled() const { return bonding_enabled_; }
    void set_bonding_enabled(bool e) { bonding_enabled_ = e; }

    /// Advance one time step (Velocity Verlet)
    void tick();

    /// Advance N time steps
    void run(int num_ticks);

    /// Compute diagnostics for current state
    AtomDiagnostics diagnostics() const;

    /// Compute force on atom i from all others
    Vec3 compute_force(int i) const;

    /// Clear all atoms
    void clear() { atoms_.clear(); forces_.clear(); tick_ = 0; next_id_ = 0; }

private:
    // Velocity Verlet phases
    void compute_all_forces();
    void half_kick();
    void drift();
    void check_bonding();       // Auto-detect bond formation/breaking
    void enforce_speed_limit();
    void apply_damping();

    // Index helpers
    int index_of(int id) const;

    std::vector<Atom> atoms_;
    std::vector<Vec3> forces_;  // Force buffer (parallel to atoms_)
    int tick_ = 0;
    int next_id_ = 0;
    double dt_ = 0.01;          // Smaller than PE (stiffer forces)
    double soft_ = 0.5;         // Softening length
    bool damping_enabled_ = false;  // Off by default for energy conservation
    bool bonding_enabled_ = true;   // Auto-bonding on by default
};

}  // namespace ftd
