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
 *
 * Phase 8: AtomToggles struct + AtomForceDiag for per-force decomposition.
 * Mirrors Scale 0 TermToggles pattern. Future forces (Phase 3) stubbed as toggles.
 */

#include "voxel.h"       // Vec3
#include "constants.h"   // ALPHA, K_B, PI, C_SPEED, DAMPING, R_BOHR, N_BASE, etc.
#include "scale.h"       // OnticEntity, ScaleLevel
#include "scale_engine.h"  // ScaleEngine base class
#include "barnes_hut.h"    // FTD Generalized Barnes-Hut Tree
#include <vector>
#include <cstdint>
#include <cmath>
#include <memory>
#include <string>

namespace ftd {

// ============================================================================
// Toggle struct — controls which forces are active (mirrors TermToggles pattern)
// ============================================================================

struct AtomToggles {
    // Currently implemented
    bool ionic = true;
    bool van_der_waals = true;
    bool covalent_bonds = true;
    bool auto_bonding = true;
    bool damping = false;         // Off by default for energy conservation

    // Phase 3 and 4 Extensions (OFF by default)
    bool h_bonds = false;
    bool dipole_dipole = false;
    bool angle_strain = false;
    bool torsional = false;
    bool improper_torsional = false;
    bool thermostat = false;
    bool electronegativity = false;

    // Validates known dependency constraints between toggles.
    // Returns true if the combination is valid.
    // If err != nullptr, appends a human-readable description of each violation.
    bool validate(std::string* err = nullptr) const {
        std::string msg;
        if (angle_strain && !covalent_bonds)
            msg += "angle_strain requires covalent_bonds (needs bond geometry)\n";
        if (torsional && !covalent_bonds)
            msg += "torsional requires covalent_bonds (needs dihedral chain)\n";
        if (improper_torsional && !covalent_bonds)
            msg += "improper_torsional requires covalent_bonds (needs bond topology)\n";
        if (thermostat && !damping)
            msg += "thermostat requires damping (Berendsen rescaling applies velocity damping)\n";
        if (dipole_dipole && !electronegativity)
            msg += "dipole_dipole requires electronegativity (dipole moments computed from chi)\n";
        if (err) *err = msg;
        return msg.empty();
    }

    void enable_all() {
        ionic = van_der_waals = covalent_bonds = auto_bonding = damping = true;
        h_bonds = dipole_dipole = angle_strain = torsional = improper_torsional = true;
        thermostat = electronegativity = true;
    }
    void minimal() {
        ionic = van_der_waals = covalent_bonds = auto_bonding = true;
        damping = false;
        h_bonds = dipole_dipole = angle_strain = torsional = improper_torsional = false;
        thermostat = electronegativity = false;
    }
};

// ============================================================================
// Per-atom force decomposition (for diagnostics + visualization)
// ============================================================================

struct AtomForceDiag {
    Vec3 f_ionic;
    Vec3 f_vdw;
    Vec3 f_bond;
    Vec3 f_hbond;
    Vec3 f_dipole;
    Vec3 f_angle;
    Vec3 f_torsion;
    Vec3 f_improper;

    Vec3 total() const {
        return f_ionic + f_vdw + f_bond + f_hbond + f_dipole + f_angle + f_torsion + f_improper;
    }
};

// ============================================================================
// Atomic property helper — computes mass, radius, vdW params from Z
// ============================================================================

struct AtomicProperties {
    double mass;          // total mass in MeV
    double radius;        // effective radius (FTD Bohr-scaled)
    double vdw_epsilon;   // LJ well depth (from α² perturbation theory)
    double vdw_sigma;     // LJ zero-crossing distance
    int max_bonds;        // maximum covalent bonds (valence)
    int valence_e;        // valence electrons (for VSEPR lone-pair counting)
    double electronegativity; // Pauling chi value
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

    // Valence electron count (electrons in outermost shell for VSEPR geometry)
    // Required for correct lone-pair counting: lone_pairs = valence_electrons - bonds
    // Example: O (Z=8) has 6 valence electrons, 2 bonds → 2 lone pairs → bent geometry
    static constexpr int valence_table[119] = {
    //  0   1   2   3   4   5   6   7   8   9
        0,  1,  2,  1,  2,  3,  4,  5,  6,  7,  // Z=0-9   (H=1,He=2,Li=1,Be=2,B=3,C=4,N=5,O=6,F=7)
        8,  1,  2,  3,  4,  5,  6,  7,  8,  1,  // Z=10-19 (Ne=8,Na=1,...,Ar=8,K=1)
        2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  // Z=20-29 (Ca=2, transition metals approx)
        2,  3,  4,  5,  6,  7,  8,  1,  2,  3,  // Z=30-39 (Zn=2,Ga=3,...,Y=3)
        4,  5,  6,  7,  8,  9, 10, 11,  2,  3,  // Z=40-49
        4,  5,  6,  7,  8,  1,  2,  3,  4,  4,  // Z=50-59
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  // Z=60-69 (lanthanides ~3)
        3,  3,  4,  5,  6,  7,  8,  9, 10, 11,  // Z=70-79
        2,  3,  4,  5,  6,  7,  8,  1,  2,  3,  // Z=80-89
        4,  5,  6,  5,  4,  3,  3,  3,  3,  3,  // Z=90-99
        3,  3,  2,  3,  4,  5,  6,  7,  8,  9,  // Z=100-109
       10, 11,  2,  3,  4,  5,  6,  7,  8        // Z=110-118
    };
    p.valence_e = (Z >= 1 && Z <= 118) ? valence_table[Z] : 4;

    // Pauling electronegativity lookup (common elements)
    static constexpr double chi_table[19] = {
    //  0     1     2     3     4     5     6     7     8     9
        0.0,  2.20, 0.0,  0.98, 1.57, 2.04, 2.55, 3.04, 3.44, 3.98,  // Z=0-9
        0.0,  0.93, 1.31, 1.61, 1.90, 2.19, 2.58, 3.16, 0.0           // Z=10-18
    };
    if (Z >= 1 && Z <= 18) {
        p.electronegativity = chi_table[Z];
    } else if (Z > 18) {
        // Rough approximation for heavier elements: chi ~ 1.5 + 0.3*log(Z)
        p.electronegativity = 1.5 + 0.3 * std::log(static_cast<double>(Z));
    } else {
        p.electronegativity = 0.0;
    }

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

    // Phase 3 fields
    double electronegativity = 0.0;  // Pauling chi value (set from Z)
    Vec3 dipole_moment;               // Electric dipole (computed from bonds + chi)

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
    ~AtomEngine();  // Out-of-line so the forward-declared GpuBackend pimpl
                    // doesn't trip incomplete-type unique_ptr deletion.

    // Toggle struct — public for direct access (like TermToggles on RenderBridge)
    AtomToggles toggles;

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

    // Backward-compatible toggle accessors (delegate to toggles struct)
    bool damping_enabled() const { return toggles.damping; }
    void set_damping_enabled(bool e) { toggles.damping = e; }
    bool bonding_enabled() const { return toggles.auto_bonding; }
    void set_bonding_enabled(bool e) { toggles.auto_bonding = e; }

    // Thermostat control
    double target_temperature() const { return target_temperature_; }
    void set_target_temperature(double T) { target_temperature_ = T; }
    double thermostat_tau() const { return thermostat_tau_; }
    void set_thermostat_tau(double tau) { thermostat_tau_ = tau; }

    // Per-atom force decomposition (populated after compute_all_forces)
    const std::vector<AtomForceDiag>& force_diag() const { return force_diag_; }

    /// Advance one time step (Velocity Verlet)
    void tick();

    /// Advance N time steps
    void run(int num_ticks);

    /// Compute diagnostics for current state
    AtomDiagnostics diagnostics() const;

    // Fetch forces exact
    Vec3 compute_pairwise_force(int i, int j) const;

    // Barnes hut force approximation
    Vec3 tree_force(int i, int node_idx) const;

    /// Compute force on atom i from all others
    Vec3 compute_force(int i) const;

    /// Clear all atoms
    void clear() { atoms_.clear(); forces_.clear(); force_diag_.clear(); tick_ = 0; next_id_ = 0; }

private:
    // Velocity Verlet phases
    void compute_all_forces();
    void half_kick();
    void drift();
    void check_bonding();       // Auto-detect bond formation/breaking
    void enforce_speed_limit();
    void apply_damping();
    void apply_thermostat();    // Berendsen velocity rescaling
    void compute_dipole_moments(); // Compute dipole_moment from bonds + chi

    // Index helpers
    int index_of(int id) const;

    std::vector<Atom> atoms_;
    std::vector<Vec3> forces_;             // Total force buffer (parallel to atoms_)
    mutable std::vector<AtomForceDiag> force_diag_;  // Per-force decomposition (mutable: written by const compute_force)

    using AtomTree = BarnesHutTree<Atom, 
        Vec3(*)(const Atom&), 
        double(*)(const Atom&), 
        double(*)(const Atom&)>;
    AtomTree octree_;

    int tick_ = 0;
    int next_id_ = 0;
    double dt_ = 1.0;          // Time step (default 1 tick = ~1 fs/10)
    double soft_ = 0.5;         // Softening length (smaller than Scale 1)
    double target_temperature_ = 0.0;  // Thermostat target (0 = disabled)
    double thermostat_tau_ = THERMOSTAT_TAU_DEFAULT; // Coupling timescale

public:
    // Wave 5.3 Phase 1: GPU acceleration for pair forces (ionic + vdW).
    // When use_gpu_ is true AND FTD_ENABLE_CUDA is defined, compute_all_forces
    // uploads atoms to the device, runs an O(N²) CUDA kernel for pair forces,
    // and downloads the results. Multi-body forces (bonds, angle strain,
    // dipole-dipole, thermostat) still run on CPU in Phase 1.
    //
    // Defaults to FTD_ENABLE_CUDA (opt-in GPU-first) — tests with < 16 atoms
    // stay on CPU for perf (upload/download dominates).
    void set_use_gpu(bool b) { use_gpu_ = b; }
    bool use_gpu() const { return use_gpu_; }

private:
#ifdef FTD_ENABLE_CUDA
    bool use_gpu_ = true;
    // Opaque pointer to gpu::AtomEngineGpu — avoids pulling in cuda_runtime.h
    // into this header, which would infect every CPU test of ftd_core.
    struct GpuBackend;
    std::unique_ptr<GpuBackend> gpu_backend_;
#else
    bool use_gpu_ = false;
#endif
};

}  // namespace ftd
