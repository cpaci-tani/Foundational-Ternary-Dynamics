#pragma once
/**
 * ParticleEngine: Scale 1 simulation
 *
 * Phase 7: Effective, lattice-free engine with continuous positions and
 * analytical forces. The records advance on the framework's discrete ordinal
 * clock, but they are not primitive lattice records. All constants come from
 * ontic.h. Velocity Verlet is symplectic only while non-conservative toggles
 * and the hard speed projection remain inactive.
 *
 * Force convention (matches Scale 0 Poisson solver):
 *   F_EM  = alpha * q_i * q_j * r_hat / (4*pi * (r^2 + soft^2))
 *   F_grav = G_PE * m_i * m_j * r_hat / (r^2 + soft^2)
 *
 * Like signs repel, opposite signs attract (Coulomb).
 * Gravity is always attractive. G_PE = G_DERIVED (FTD-0131 physical α_G).
 *
 * Phase 8: ParticleToggles struct + ParticleForceDiag for per-force decomposition.
 * Mirrors the Scale 0 TermToggles pattern.
 */

#include "voxel.h"         // Vec3
#include "constants.h"     // ALPHA, G_PE, K_B, PI, C_SPEED, DAMPING
#include "scale.h"         // OnticEntity, ScaleLevel
#include "scale_engine.h"  // ScaleEngine base class
#include "barnes_hut.h"    // O(N log N) spatial partitioning
#include "scale1/domain.h" // shared Scale-1 records/registry/provenance
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
    // Verified effective-lab baseline.  These are not v3 primitives.
    bool coulomb = true;
    bool gravity = false;
    bool damping = false;

    // Selected and quarantined effective extensions. These are available for
    // controlled experiments but are outside the verified baseline profile.
    bool lorentz = false;
    bool exchange = false;
    bool strong = false;
    bool radiation = false;
    bool spin_orbit = false;
    bool relativistic = false;  // retired compatibility key; enabling is rejected
    bool magnetic_dipole = false;
    bool relativistic_verlet = true;
    bool contact_events = false;

    // ── Table-managed helpers (ADR-0013; bodies below PARTICLE_TOGGLE_SPECS) ──
    // Adding a toggle is a 2-place edit: the field above + one table row. The
    // struct fields are preserved verbatim so `pe.toggles.coulomb = false` and
    // every other direct consumer keeps compiling.
    bool validate(std::string* err = nullptr) const; // false + message on violation
    void enable_all();                               // every applicable toggle ON
    void verified();                                 // verified/applicable profile only
    void minimal();                                  // compatibility alias for verified()
    bool get_toggle(std::string_view name) const;    // false if unknown
    bool set_toggle(std::string_view name, bool value,
                    std::string* err = nullptr); // transactional; false if unknown/invalid
};

// ─────────────────────────────────────────────────────────────────────
// ParticleToggleSpec — one row per boolean toggle (ADR-0013 pattern,
// ported from term_toggles.h). The helpers above iterate this table.
//   default_value — constructor default (copied verbatim from the fields)
//   verified_value — value applied by verified()/minimal(); only modules with
//                    an audited, applicable Scale-1 role are enabled
//   requires_     — single dependency name that must also be ON (empty = none).
//                   No current particle term depends on another term being ON:
//                   magnetic-dipole and spin-orbit forces are independently
//                   implemented and are intentionally isolatable in tests.
// ─────────────────────────────────────────────────────────────────────
struct ParticleToggleSpec {
    const char* name;
    bool ParticleToggles::* field;
    bool default_value;
    bool verified_value;
    bool available;
    const char* requires_;
    const char* description;
};

inline constexpr ParticleToggleSpec PARTICLE_TOGGLE_SPECS[] = {
    // {name, field, default, verified, available, requires_, description}
    {"coulomb",             &ParticleToggles::coulomb,             true,  true,  true,  "", "Effective softened Coulomb force"},
    {"gravity",             &ParticleToggles::gravity,             false, false, true,  "", "Effective Newtonian comparison force"},
    {"damping",             &ParticleToggles::damping,             false, false, true,  "", "Imposed environment damping"},
    {"lorentz",             &ParticleToggles::lorentz,             false, false, true,  "", "Imported magnetic Lorentz extension"},
    {"exchange",            &ParticleToggles::exchange,            false, false, true,  "", "Quarantined Pauli-style repulsion toy"},
    {"strong",              &ParticleToggles::strong,              false, false, true,  "", "Quarantined color-force toy"},
    {"radiation",           &ParticleToggles::radiation,           false, false, true,  "", "Quarantined radiation-reaction toy"},
    {"spin_orbit",          &ParticleToggles::spin_orbit,          false, false, true,  "", "Imported spin-orbit extension"},
    {"relativistic",        &ParticleToggles::relativistic,        false, false, false, "", "Retired non-covariant isotropic force rescale"},
    {"magnetic_dipole",     &ParticleToggles::magnetic_dipole,     false, false, true,  "", "Imported magnetic dipole extension"},
    {"relativistic_verlet", &ParticleToggles::relativistic_verlet, true,  true,  true,  "", "Relativistic momentum-Verlet integrator"},
    {"contact_events",      &ParticleToggles::contact_events,      false, false, true,  "", "Selected opposite-charge contact-removal event"},
};
static_assert(sizeof(PARTICLE_TOGGLE_SPECS) / sizeof(PARTICLE_TOGGLE_SPECS[0]) == 12,
              "ParticleToggles has 12 registry keys — update the table and this pin together");

inline bool ParticleToggles::get_toggle(std::string_view name) const {
    for (const auto& s : PARTICLE_TOGGLE_SPECS)
        if (name == s.name) return this->*(s.field);
    return false;
}
inline bool ParticleToggles::set_toggle(std::string_view name, bool value,
                                        std::string* err) {
    for (const auto& s : PARTICLE_TOGGLE_SPECS) {
        if (name != s.name) continue;
        if (value && !s.available) {
            if (err) *err = std::string(s.name) + " is retired/unavailable";
            return false;
        }
        ParticleToggles staged = *this;
        staged.*(s.field) = value;
        if (!staged.validate(err)) return false;
        *this = staged;
        if (err) err->clear();
        return true;
    }
    if (err) *err = "unknown particle toggle";
    return false;
}
inline void ParticleToggles::enable_all() {
    for (const auto& s : PARTICLE_TOGGLE_SPECS) this->*(s.field) = s.available;
}
inline void ParticleToggles::verified() {
    for (const auto& s : PARTICLE_TOGGLE_SPECS) this->*(s.field) = s.verified_value;
}
inline void ParticleToggles::minimal() {
    verified();
}
inline bool ParticleToggles::validate(std::string* err) const {
    std::string msg;
    // Pass 1: table-driven single-name requires_ (none declared for Particle).
    for (const auto& s : PARTICLE_TOGGLE_SPECS) {
        if (!(this->*(s.field))) continue;
        if (!s.available) {
            msg += s.name; msg += " is retired/unavailable\n";
            continue;
        }
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
 * @brief Effective continuous-coordinate record representing one Scale-1 body.
 *
 * [IMPOSED representation] This is a coarse-grained/effective record advanced
 * once per discrete global tick. It is not a primitive FTD record and its
 * continuous coordinates and analytical force laws are not axioms of the
 * strict-discrete substrate.
 */
struct Particle {
    int32_t id = -1;
    int8_t charge = 0;        // Effective signed charge; composite references may have |q| > 1
    double mass = K_B;         // Energy (default = electron mass)
    double r_eff = 2.48;       // Boundary (effective radius from Phase 6)
    // NOTE: KCOMP shell at 128³ measures r_eff ≈ 11.61 (DERIV_KCOMP_VOLUMETRIC_SHELL.md).
    // The 2.48 default is from the old CFL speed era. ParticleEngine (Scale 1) uses
    // analytical forces, not lattice dynamics, so r_eff is only used for the
    // explicitly selected contact-removal event
    // distance. A dynamic measurement would be more accurate but isn't critical here.
    Vec3 position;             // Continuous coordinates (lattice units)
    Vec3 velocity;
    Vec3 acceleration;         // Current acceleration (for Verlet)
    Vec3 prev_acceleration;    // Previous tick acceleration (for radiation reaction)
    int8_t spin = 0;
    int8_t color = 0;
    int32_t pair_id = -1;      // Entanglement partner
    bool locked = false;       // [IMPOSED] kinematic anchor; finite mass, motion suppressed
    Vec3 spin_axis;            // Spin direction (for magnetic/exchange forces)
    Vec3 momentum;             // Relativistic momentum p = gamma * m * v
    Scale1Provenance provenance;

    // Convert to the scale-local presentation summary
    OnticEntity as_ontic() const {
        return {charge, mass, r_eff};
    }
};

// ============================================================================
// Scenario-owned insulating geometry
// ============================================================================

/**
 * @brief One rectangular opening in a face of an insulating box.
 *
 * [IMPOSED effective environment] `axis` selects the face normal (0=x,
 * 1=y, 2=z), `side` selects the negative or positive face, and the remaining
 * coordinates describe a face-local rectangular aperture relative to the box
 * center. A particle fits only when its effective radius clears both aperture
 * half-extents. `required_charge_sign` is -1, 0 (any), or +1;
 * `crossing_direction` is -1 (outside to inside), 0 (bidirectional), or +1
 * (inside to outside).
 */
struct ParticleInsulatingPort {
    int axis = 0;
    int side = 1;
    double center_u = 0.0;
    double center_v = 0.0;
    double half_u = 0.0;
    double half_v = 0.0;
    int required_charge_sign = 0;
    int crossing_direction = 0;
};

/**
 * @brief Axis-aligned, perfectly insulating scenario volume.
 *
 * The wall is an ideal, zero-thickness, energy-conserving surface. Dynamic
 * particle centers reflect specularly from every face and may cross the
 * surface only through a declared port. Locked source records remain fixed.
 */
struct ParticleInsulatingBox {
    bool enabled = false;
    Vec3 center;
    Vec3 half_extents;
    std::vector<ParticleInsulatingPort> ports;
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
    Vec3 center_of_mass;
    bool state_energy_complete = false;
    bool drift_eligible = false;
    std::uint32_t covered_mask = 0;
    std::uint32_t missing_mask = 0;
    std::uint32_t nonconservative_mask = 0;
    double cumulative_damping_sink = 0.0;
    double cumulative_radiation_sink = 0.0;
    double cumulative_speed_projection_sink = 0.0;
    double cumulative_contact_delta = 0.0;
    std::uint64_t contact_event_count = 0;
    std::uint64_t speed_projection_count = 0;
    std::uint64_t insulator_collision_count = 0;
    std::uint64_t insulator_port_crossing_count = 0;
    Vec3 cumulative_insulator_impulse;
};

/**
 * @brief Versioned, state-complete effective ParticleEngine checkpoint.
 *
 * [IMPOSED effective-lab infrastructure] This record captures every mutable
 * value that can affect the next ParticleEngine transaction.  Force and
 * diagnostic caches are intentionally excluded because they are pure
 * observations of the captured state and are rebuilt after restoration.
 */
struct ParticleEngineCheckpoint {
    static constexpr int SCHEMA_VERSION = 1;
    int schema_version = SCHEMA_VERSION;
    int tick = 0;
    int next_id = 0;
    std::uint64_t next_event_sequence = 0;
    double dt = 1.0;
    double softening = 1.0;
    ParticleToggles toggles;
    std::vector<Particle> particles;
    std::vector<Scale1EventRecord> events;
    ParticleInsulatingBox insulating_box;
    double cumulative_damping_sink = 0.0;
    double cumulative_radiation_sink = 0.0;
    double cumulative_speed_projection_sink = 0.0;
    double cumulative_contact_delta = 0.0;
    std::uint64_t contact_event_count = 0;
    std::uint64_t speed_projection_count = 0;
    std::uint64_t insulator_collision_count = 0;
    std::uint64_t insulator_port_crossing_count = 0;
    Vec3 cumulative_insulator_impulse;
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
 * [IMPOSED effective model] Solves interacting continuous-coordinate records
 * using O(N log N) Barnes-Hut or exact O(N^2) pairwise force accumulation.
 * This is a laboratory model, not the primitive FTD substrate and not a claim
 * that Standard Model particles have been recovered. The momentum-Verlet
 * baseline is conservative only while event/sink/projection terms are absent.
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

    // Add an [IMPOSED] kinematic anchor. Its mass remains finite but motion is
    // suppressed by the effective scenario definition.
    int add_locked_particle(int8_t charge, Vec3 position, double mass = K_B,
                            int8_t spin = 0, int8_t color = 0);

    /// Scenario/setup mutation by stable id. Keeps velocity and relativistic
    /// momentum coherent; returns false when the id is absent.
    bool set_particle_velocity(int id, Vec3 velocity);

    // Access
    std::vector<Particle>& particles() {
        invalidate_observation();
        return particles_;
    }
    const std::vector<Particle>& particles() const { return particles_; }
    int current_tick() const override { return tick_; }
    std::uint64_t observation_revision() const { return observation_revision_; }
    double dt() const override { return dt_; }
    void set_dt(double d) override;
    double softening() const { return soft_; }
    void set_softening(double s);

    /// Configure an [IMPOSED] axis-aligned perfect-insulator surface.
    void configure_insulating_box(Vec3 center, Vec3 half_extents);
    /// Add a face-local rectangular pass-through aperture to that surface.
    void add_insulating_port(int axis, int side,
                             double center_u, double center_v,
                             double half_u, double half_v,
                             int required_charge_sign = 0,
                             int crossing_direction = 0);
    void clear_insulating_box();
    const ParticleInsulatingBox& insulating_box() const { return insulating_box_; }

    /// Validate the effective record before/after a transaction. This guards
    /// native callers as well as the WASM adapter from propagating NaN/Inf or
    /// nonphysical mass/radius values through the force kernel.
    bool validate_state(std::string* err = nullptr) const;

    // Backward-compatible toggle accessors (delegate to toggles struct)
    bool damping_enabled() const { return toggles.damping; }
    void set_damping_enabled(bool e) { (void)try_set_toggle("damping", e); }
    bool gravity_enabled() const { return toggles.gravity; }
    void set_gravity_enabled(bool e) { (void)try_set_toggle("gravity", e); }

    // Per-particle force decomposition (populated after compute_all_forces)
    const std::vector<ParticleForceDiag>& force_diag() const { return force_diag_; }
    /// Integrator-aligned force observation. Reuses the most recent force
    /// phase and performs at most one refresh after an external mutation.
    const std::vector<ParticleForceDiag>& observation_force_diag();
    const std::vector<Scale1EventRecord>& event_history() const { return events_; }
    // Advance one time step (Velocity Verlet)
    void tick() override;

    // Advance N time steps
    void run(int num_ticks) override;

    // Compute diagnostics for current state
    ParticleDiagnostics diagnostics() const;

    /// Versioned shared snapshot consumed by native and WASM frontends.
    Scale1Snapshot snapshot(const std::string& scenario = {},
                            const std::string& backend = "cpu",
                            bool include_forces = true);

    /// Capture/restore every mutable value that can affect the next tick.
    ParticleEngineCheckpoint checkpoint() const;
    bool restore_checkpoint(const ParticleEngineCheckpoint& checkpoint,
                            std::string* err = nullptr);

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
        events_.clear();
        tick_ = 0;
        next_id_ = 0;
        next_event_sequence_ = 0;
        cumulative_damping_sink_ = 0.0;
        cumulative_radiation_sink_ = 0.0;
        cumulative_speed_projection_sink_ = 0.0;
        cumulative_contact_delta_ = 0.0;
        contact_event_count_ = 0;
        speed_projection_count_ = 0;
        insulating_box_ = {};
        insulator_collision_count_ = 0;
        insulator_port_crossing_count_ = 0;
        cumulative_insulator_impulse_ = {};
        last_relativistic_verlet_ = toggles.relativistic_verlet;
        invalidate_observation();
    }

    // Table-backed (ADR-0013): delegates to ParticleToggles, which iterates
    // PARTICLE_TOGGLE_SPECS. Unknown names read false / are ignored, exactly
    // as the former hand-written if-ladders did.
    bool get_toggle(const std::string& name) const override {
        return toggles.get_toggle(name);
    }

    bool try_set_toggle(std::string_view name, bool value,
                        std::string* err = nullptr) {
        const bool changed = toggles.get_toggle(name) != value;
        const bool accepted = toggles.set_toggle(name, value, err);
        if (accepted && changed) invalidate_observation();
        return accepted;
    }

    void set_toggle(const std::string& name, bool value) override {
        (void)try_set_toggle(name, value);
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

    /// Compute an exact, self-contained per-term force snapshot for particle i.
    /// Unlike force_diag(), this does not require a prior tick and clears the
    /// target diagnostic row before evaluating it.
    ParticleForceDiag compute_force_diagnostic(int i) const;

    /// Compute one exact pair contribution without requiring a prior tick.
    ParticleForceDiag compute_pair_force_diagnostic(int i, int j) const;

private:
    void invalidate_observation();
    void invalidate_diagnostics_cache();
    std::uint32_t toggle_observation_mask() const;

    // Velocity Verlet integration
    void compute_all_forces();
    void half_kick();
    void drift();
    void drift_with_insulating_box(Particle& p);
    bool port_allows_crossing(const Particle& p, int axis, int side,
                              bool started_inside,
                              const Vec3& hit_position) const;
    void process_contact_events();
    void enforce_speed_limit();
    void apply_damping();
    void evolve_spin_axes();
    void synchronize_momentum_from_velocity();
    static Vec3 momentum_from_velocity(const Particle& p);
    static double kinetic_energy(const Particle& p, bool relativistic);
    void append_event(Scale1EventRecord event);

    std::vector<Particle> particles_;
    std::vector<Vec3> forces_;             // Total force buffer (parallel to particles_)
    mutable std::vector<ParticleForceDiag> force_diag_;  // Per-force decomposition (mutable: written by const compute_force)
    mutable bool force_diag_ready_ = false;
    std::uint32_t force_diag_toggle_mask_ = 0;

    // Exact state-energy diagnostics are O(N^2). Cache them until the particle
    // state or active physics profile changes so multiple dashboard consumers
    // share one observation instead of independently repeating the reduction.
    std::uint64_t observation_revision_ = 1;
    mutable std::uint64_t diagnostics_revision_ = 0;
    mutable std::uint32_t diagnostics_toggle_mask_ = 0;
    mutable bool diagnostics_cache_valid_ = false;
    mutable ParticleDiagnostics diagnostics_cache_;

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
    bool last_relativistic_verlet_ = true;
    std::vector<Scale1EventRecord> events_;
    std::uint64_t next_event_sequence_ = 0;
    double cumulative_damping_sink_ = 0.0;
    double cumulative_radiation_sink_ = 0.0;
    double cumulative_speed_projection_sink_ = 0.0;
    double cumulative_contact_delta_ = 0.0;
    std::uint64_t contact_event_count_ = 0;
    std::uint64_t speed_projection_count_ = 0;
    ParticleInsulatingBox insulating_box_;
    std::uint64_t insulator_collision_count_ = 0;
    std::uint64_t insulator_port_crossing_count_ = 0;
    Vec3 cumulative_insulator_impulse_;

public:
    // Wave 5.4 Phase 1: GPU acceleration for pair forces (coulomb + gravity).
    // When use_gpu_ is true AND FTD_ENABLE_CUDA is defined, compute_all_forces
    // uploads particles to the device, runs an O(N²) CUDA kernel for coulomb
    // + gravity pair forces, and downloads the results. Extended forces
    // (strong, exchange, lorentz, magnetic_dipole, spin_orbit) and
    // non-pairwise post-processing (radiation) still runs on
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
