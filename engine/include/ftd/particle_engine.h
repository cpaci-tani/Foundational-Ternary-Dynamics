#pragma once
/**
 * ParticleEngine: Scale 1 simulation
 *
 * Phase 7: Lattice-free engine with continuous positions and analytical forces.
 * All constants from ontic.h. Velocity Verlet integration (symplectic).
 *
 * Force convention (matches Scale 0 Poisson solver):
 *   F_EM  = alpha * q_i * q_j * r_hat / (4*pi * (r^2 + soft^2))
 *   F_grav = G_PE * m_i * m_j * r_hat / (r^2 + soft^2)
 *
 * Like signs repel, opposite signs attract (Coulomb).
 * Gravity is always attractive. G_PE = G_DERIVED (FTD-0131 physical α_G).
 *
 * Phase 8: ParticleToggles struct + ParticleForceDiag for per-force decomposition.
 * Mirrors Scale 0 TermToggles pattern. Future forces (Phase 2) stubbed as toggles.
 */

#include "voxel.h"         // Vec3
#include "constants.h"     // ALPHA, G_PE, K_B, PI, C_SPEED, DAMPING
#include "scale.h"         // OnticEntity, ScaleLevel
#include "scale_engine.h"  // ScaleEngine base class
#include "barnes_hut.h"    // O(N log N) spatial partitioning
#include <vector>
#include <cstdint>
#include <string>
#include <string_view>
#include <memory>

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
    bool relativistic_verlet = false;

    // ── Table-managed helpers (ADR-0013; bodies below PARTICLE_TOGGLE_SPECS) ──
    // Adding a toggle is a 2-place edit: the field above + one table row. The
    // struct fields are preserved verbatim so `pe.toggles.coulomb = false` and
    // every other direct consumer keeps compiling.
    bool validate(std::string* err = nullptr) const; // false + message on violation
    void enable_all();                               // every toggle ON
    void minimal();                                  // recommended-default profile
    bool get_toggle(std::string_view name) const;    // false if unknown
    bool set_toggle(std::string_view name, bool value); // false if unknown
};

// ─────────────────────────────────────────────────────────────────────
// ParticleToggleSpec — one row per boolean toggle (ADR-0013 pattern,
// ported from term_toggles.h). The helpers above iterate this table.
//   default_value — constructor default (copied verbatim from the fields)
//   minimal_value — value applied by minimal(); differs from default only for
//                   relativistic_verlet (default OFF, minimal ON — the legacy
//                   minimal() turned it on)
//   requires_     — single dependency name that must also be ON (empty = none).
//                   Particle's OR-dependencies (spin_orbit / magnetic_dipole need
//                   coulomb OR gravity) cannot be expressed by a single-name AND
//                   requires_, so they live in validate()'s Pass 2, exactly like
//                   term_toggles.h keeps cross-cutting rules hand-rolled.
// ─────────────────────────────────────────────────────────────────────
struct ParticleToggleSpec {
    const char* name;
    bool ParticleToggles::* field;
    bool default_value;
    bool minimal_value;
    const char* requires_;
    const char* description;
};

inline constexpr ParticleToggleSpec PARTICLE_TOGGLE_SPECS[] = {
    // {name, field, default, minimal, requires_, description}
    {"coulomb",             &ParticleToggles::coulomb,             true,  true,  "", "Electrostatic Coulomb force"},
    {"gravity",             &ParticleToggles::gravity,             true,  true,  "", "Newtonian gravity (alpha_G scale)"},
    {"damping",             &ParticleToggles::damping,             true,  true,  "", "Velocity damping"},
    {"lorentz",             &ParticleToggles::lorentz,             false, false, "", "Magnetic Lorentz force F = alpha*s*(v x B)"},
    {"exchange",            &ParticleToggles::exchange,            false, false, "", "Pauli exclusion repulsion (same-spin)"},
    {"strong",              &ParticleToggles::strong,              false, false, "", "Color/strong short-range force"},
    {"radiation",           &ParticleToggles::radiation,           false, false, "", "Radiation-reaction (acceleration) damping"},
    {"spin_orbit",          &ParticleToggles::spin_orbit,          false, false, "", "Spin-orbit coupling"},
    {"relativistic",        &ParticleToggles::relativistic,        false, false, "", "Relativistic gamma force correction"},
    {"magnetic_dipole",     &ParticleToggles::magnetic_dipole,     false, false, "", "Magnetic dipole-dipole force"},
    {"relativistic_verlet", &ParticleToggles::relativistic_verlet, false, true,  "", "Relativistic velocity-Verlet integrator"},
};
static_assert(sizeof(PARTICLE_TOGGLE_SPECS) / sizeof(PARTICLE_TOGGLE_SPECS[0]) == 11,
              "ParticleToggles has 11 boolean toggles — update the table and this pin together");

inline bool ParticleToggles::get_toggle(std::string_view name) const {
    for (const auto& s : PARTICLE_TOGGLE_SPECS)
        if (name == s.name) return this->*(s.field);
    return false;
}
inline bool ParticleToggles::set_toggle(std::string_view name, bool value) {
    for (const auto& s : PARTICLE_TOGGLE_SPECS)
        if (name == s.name) { this->*(s.field) = value; return true; }
    return false;
}
inline void ParticleToggles::enable_all() {
    for (const auto& s : PARTICLE_TOGGLE_SPECS) this->*(s.field) = true;
}
inline void ParticleToggles::minimal() {
    for (const auto& s : PARTICLE_TOGGLE_SPECS) this->*(s.field) = s.minimal_value;
}
inline bool ParticleToggles::validate(std::string* err) const {
    std::string msg;
    // Pass 1: table-driven single-name requires_ (none declared for Particle).
    for (const auto& s : PARTICLE_TOGGLE_SPECS) {
        if (!(this->*(s.field))) continue;
        if (s.requires_ && *s.requires_) {
            for (const auto& dep : PARTICLE_TOGGLE_SPECS) {
                if (std::string_view(s.requires_) == dep.name) {
                    if (!(this->*(dep.field))) {
                        msg += s.name; msg += " requires "; msg += dep.name; msg += "\n";
                    }
                    break;
                }
            }
        }
    }
    // Pass 2: OR-dependencies the single-name requires_ column cannot express.
    // Preserves the exact verdicts of the pre-refactor validate().
    if (spin_orbit && !coulomb && !gravity)
        msg += "spin_orbit has no effect without coulomb or gravity\n";
    if (magnetic_dipole && !coulomb && !gravity)
        msg += "magnetic_dipole has no effect without coulomb or gravity\n";
    if (err) *err = msg;
    return msg.empty();
}

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

/**
 * @brief Continuous space entity representing a manifested particle.
 * 
 * [AXIOM] Evaluated natively in continuous floating-point space rather than
 * on a discrete lattice, serving as the foundational element of Scale 1.
 */
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
    Vec3 momentum;             // Relativistic momentum p = gamma * m * v

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
    double total_pe = 0.0;       // active potential terms only
    double coulomb_pe = 0.0;     // active Coulomb PE (zero when disabled)
    double gravity_pe = 0.0;     // active Newtonian gravity PE (zero when disabled)
    double total_energy = 0.0;   // KE + PE
    Vec3 total_momentum;         // sum m * v
    Vec3 total_angular_momentum; // sum r x (m*v)
};

// ============================================================================
// Orbital element extraction — Kepler orbit characterization
// ============================================================================

struct OrbitalElements {
    double semi_major_axis = 0.0;   // a: half the major axis
    double eccentricity = 0.0;      // e: 0=circle, 0<e<1 ellipse
    double periapsis = 0.0;         // closest approach = a*(1-e)
    double apoapsis = 0.0;          // farthest point = a*(1+e)
    double period = 0.0;            // T = 2*pi*sqrt(a^3 / alpha_eff)
    double specific_energy = 0.0;   // E/m (should be negative for bound)
    double specific_angular_momentum = 0.0; // |L|/m
    bool bound = false;             // E < 0
};

// Compute orbital elements for particle orbiting a locked center.
// alpha_eff = effective coupling (EM + gravity) = ALPHA/(4*PI) + G_PE*m_center*m_orbiter
OrbitalElements compute_orbital_elements(const Particle& orbiter,
                                          const Particle& center,
                                          double alpha_eff);

/**
 * @brief Continuous Space / Analytical Force Engine (Scale 1).
 * 
 * [EXTENDED] Solves interacting particles using O(N log N) Barnes-Hut or 
 * exact O(N^2) pairwise force accumulation. Integrates state via Symplectic 
 * Velocity-Verlet.
 */
class ParticleEngine : public ScaleEngine {
public:
    ParticleEngine();
    ~ParticleEngine() override;  // Out-of-line so the forward-declared
                                 // GpuBackend pimpl doesn't trip
                                 // incomplete-type unique_ptr deletion.

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
    int current_tick() const override { return tick_; }
    double dt() const override { return dt_; }
    void set_dt(double d) override { dt_ = d; }
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
    void tick() override;

    // Advance N time steps
    void run(int num_ticks) override;

    // Compute diagnostics for current state
    ParticleDiagnostics diagnostics() const;

    // ====================================================================
    // ScaleEngine overrides — polymorphic interface for bridge dispatch
    // ====================================================================

    int scale_level() const override { return static_cast<int>(ScaleLevel::PARTICLE); }
    const char* scale_name() const override { return "ParticleEngine"; }
    int entity_count() const override { return static_cast<int>(particles_.size()); }

    void clear() override {
        particles_.clear();
        forces_.clear();
        force_diag_.clear();
        tick_ = 0;
        next_id_ = 0;
    }

    // Table-backed (ADR-0013): delegates to ParticleToggles, which iterates
    // PARTICLE_TOGGLE_SPECS. Unknown names read false / are ignored, exactly
    // as the former hand-written if-ladders did.
    bool get_toggle(const std::string& name) const override {
        return toggles.get_toggle(name);
    }

    void set_toggle(const std::string& name, bool value) override {
        toggles.set_toggle(name, value);
    }

    ScaleBaseDiagnostics base_diagnostics() const override {
        auto d = diagnostics();
        ScaleBaseDiagnostics b;
        b.tick = d.tick;
        b.entity_count = d.particle_count;
        b.total_energy = d.total_energy;
        b.total_ke = d.total_ke;
        b.total_pe = d.total_pe;
        b.total_momentum = d.total_momentum;
        return b;
    }

    // Compute exact 1-to-1 force
    Vec3 compute_pairwise_force(int i, int j) const;

    // Compute Barnes-Hut tree force
    Vec3 tree_force(int i, int node_idx) const;

    /// Compute specific exact force on particle i
    Vec3 compute_force(int i) const;

private:
    // Velocity Verlet integration
    void compute_all_forces();
    void half_kick();
    void drift();
    void check_annihilation();
    void enforce_speed_limit();
    void apply_damping();
    void evolve_spin_axes();

    std::vector<Particle> particles_;
    std::vector<Vec3> forces_;             // Total force buffer (parallel to particles_)
    mutable std::vector<ParticleForceDiag> force_diag_;  // Per-force decomposition (mutable: written by const compute_force)

    using ParticleTree = BarnesHutTree<Particle,
        Vec3(*)(const Particle&),
        double(*)(const Particle&),
        double(*)(const Particle&)>;
    ParticleTree octree_;

    int tick_ = 0;
    int next_id_ = 0;
    double dt_ = 1.0;           // Time step (default 1, can increase for Scale 1)
    // Softening length. INTENTIONALLY scale-dependent across engines
    // (revision 2.4 documentation — do not "unify"): Scale 1 uses 1.0
    // (one lattice unit, the minimum resolvable separation), Scale 2
    // (AtomEngine) uses 0.5 (sub-lattice atomic separations), Scale 5
    // (CosmicEngine) recomputes as box_size * SOFTENING_SCALE(0.01) per
    // scenario. Each is an [IMPOSED] regularization matched to its scale's
    // typical separations, not a shared derived constant.
    double soft_ = 1.0;         // Softening length (matches lattice minimum)

public:
    // Wave 5.4 Phase 1: GPU acceleration for pair forces (coulomb + gravity).
    // When use_gpu_ is true AND FTD_ENABLE_CUDA is defined, compute_all_forces
    // uploads particles to the device, runs an O(N²) CUDA kernel for coulomb
    // + gravity pair forces, and downloads the results. Extended forces
    // (strong, exchange, lorentz, magnetic_dipole, spin_orbit) and
    // non-pairwise post-processing (radiation, relativistic) still run on
    // CPU in Phase 1.
    //
    // Falls back to CPU automatically when: any advanced toggle is on,
    // or particles_.size() < 8 (upload/download overhead dominates).
    void set_use_gpu(bool b) { use_gpu_ = b; }
    bool use_gpu() const { return use_gpu_; }

private:
#ifdef FTD_ENABLE_CUDA
    bool use_gpu_ = true;
    // Opaque pointer to gpu::ParticleEngineGpu — avoids pulling cuda_runtime.h
    // into this header, which would infect every CPU test of ftd_core.
    struct GpuBackend;
    std::unique_ptr<GpuBackend> gpu_backend_;
#else
    bool use_gpu_ = false;
#endif
};

}  // namespace ftd
