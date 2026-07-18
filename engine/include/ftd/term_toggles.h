#pragma once
// Runtime toggles for the logic-first engine.
// 34 boolean toggles + 6 non-bool config fields.
//
// Phase 6 (2026-04-27): redesign as a TABLE-DRIVEN registry. Adding a new
// boolean toggle now requires ONE edit (a row in TOGGLE_SPECS[]) instead
// of the legacy four edits (struct field + validate() case + enable_all()
// + disable_all() + cpu_runtime_warnings()).
//
// The struct fields themselves are preserved verbatim so every existing
// consumer (`toggles.wave_propagation = true`, `rb.toggles.langevin_T`,
// etc.) continues to compile unchanged. The table just gives us the
// metadata to auto-generate the helper bodies.

#include <cstdint>
#include <string>
#include <string_view>
#include "sublattice.h"   // BccStencilMode, SiteClass

namespace ftd {

// Flux-field boundary law (non-bool config; like bcc_stencil it lives outside
// TOGGLE_SPECS[]). Default Periodic preserves the toroidal wrap that every
// existing test + the golden-tick hash were written against — so adding this
// field is golden-neutral. The web dashboard defaults its selector to Dispersal.
//   Periodic   — toroidal wrap (current behaviour; energy conserved + trapped)
//   Reflective — Neumann mirror at each face (perfect cavity; energy conserved)
//   Dispersal  — first-order radiating (Mur) outflow; outgoing flux leaves the
//                box and is removed (no graduated sponge layer)
enum class FluxBoundaryMode : int { Periodic = 0, Reflective = 1, Dispersal = 2 };

// Backend bitmask used by ToggleSpec::backends. CPU = 0b001, GPU = 0b010,
// JS  = 0b100, ANY = 0b111. Currently informational — not enforced at the
// binding layer — but the WASM map below filters by `(backends & 0b100)`
// so a future GPU-only toggle won't accidentally appear in the JS surface.
namespace ToggleBackend {
    constexpr uint8_t CPU = 0b001;
    constexpr uint8_t GPU = 0b010;
    constexpr uint8_t JS  = 0b100;
    constexpr uint8_t ANY = CPU | GPU | JS;
}

struct TermToggles {
    // ── Boolean toggles (table-managed; see TOGGLE_SPECS[] below) ─────
    bool wave_propagation = true;   // phase_read: Laplacian wave equation
    bool coupling = true;           // phase_read: g_c * grad(s) source term
    bool damping = true;            // phase_write: energy dissipation
    bool genesis = true;            // phase_write: manifestation + evaporation (master)
    bool evaporation = false;       // phase_write: evaporation alone (test isolation; OR'd with genesis)
    bool gauss_projection = true;   // gauss_project: div(J) = s constraint
    bool forces = true;             // phase_forces: field-mediated EM + gravity
    bool gravity = true;            // phase_forces: F = G_N·∇ρ gravitational force
    bool poisson_coulomb = true;    // Poisson-based Coulomb (Phase 3). false = legacy grad(div J)
    bool movement = true;           // phase_movement: velocity integration + collisions
    bool lorentz_force = true;      // phase_forces: F = α·s·(v×B) magnetic force
    bool selective_damping = true;  // phase_write: damp only near particles (true = vacuum EM lossless)
    bool larmor_radiation = false;  // phase_write: acceleration-dependent damping at particle sites
    bool dual_substrate = true;     // dual-substrate mode: J_L, J_R independent fields
    bool color_forces = false;      // phase_forces: SU(3)-inspired color-dependent pairwise force
    bool weak_transmutation = true; // tick: chirality/stress polarity flip (+1 ↔ -1)
    bool strong_force = false;      // phase_forces: Yukawa short-range nuclear force
    bool triad_binding = false;     // tick: detect 3-particle triads, set locked=true
    bool pair_production = false;   // genesis: correlated +1/-1 pairs from high-flux void
    bool exchange_force = false;    // phase_forces: Pauli exclusion repulsion (same-spin)
    bool latency_field = false;     // Poisson-based latency field ∇²L = 4πGρ (gravity potential)
    bool exact_dual_gauss = false;  // gauss_project: exact dual-cell face-flux projection
    bool emergent_forces = false;   // EFT mode: force from flux gradient (no Poisson), alpha = G_C²
    bool langevin = false;          // Stochastic thermalization: OU process on wave_vel with (gamma, T)
    bool symplectic_leapfrog = false; // Scale 0: Symplectic Leapfrog wave propagation
    bool verlet_wave_integrator = false; // [E1, FTD-0333 §5.1 / FTD-0337] Velocity-Verlet (KDK) bare-wave
                                    // integrator: phase_write applies half-kick + drift
                                    // (wave_vel += ½·dt·ΔJ; flux += dt·wave_vel), then tick() re-runs
                                    // phase_read on the post-drift field and applies the second
                                    // half-kick (wave_vel += ½·dt·ΔJ'). Targets the FTD-0337
                                    // bare-wave leapfrog amplitude growth (the corrected FTD-0308
                                    // mechanism): a synchronized, symplectic wave update whose dt<1
                                    // is honored (see set_dt). CPU path only; conflicts with
                                    // symplectic_leapfrog (both own the wave update). Default OFF ⇒
                                    // dead branch ⇒ golden hash 0xb604d81a3d79366e untouched.
    bool su2_gauge = false;         // tick Rule 7b: per-tick SU(2) link staple relaxation ([IMPOSED] Wilson-action import; links are write-only — no substrate feedback, see test_gauge_links G1)
    bool su3_gauge = false;         // tick Rule 7b: per-tick SU(3) link staple relaxation ([IMPOSED] Wilson-action import; links are write-only — no substrate feedback, see test_gauge_links G1)
    bool symmetric_movement_order = false; // phase_movement: coordinate-independent update traversal & axis ordering
    bool absorbing_boundary = false; // tick: graduated sponge layer — outgoing waves disperse into the void at lattice faces (no reflect/wrap)
    bool reflective_boundary = false; // phase_movement: mirror-bounce at faces when on; particles exhaust into the void when off (no periodic wrap)
    bool field_energy_gravity = false; // [IMPOSED] latency Poisson also sources from field-energy density ½(|J|²+|wave_vel|²), not only particle rest mass, so flux-only configs (gravity waves) carry a real potential. Requires latency_field.
    bool cluster_inertia = false;   // [IMPOSED] phase_forces: rigid-body integrate LOCKED clusters at inertial mass N·M_REST (a_COM = F_cluster/(N·M_REST)). Unified-mass Phase 2. Additive (per-voxel loop already skips locked); requires forces.
    bool de_broglie_clock = false;  // [IMPOSED] phase_read: Klein-Gordon rest-mass term −ω₀²·J at manifested (state!=0) voxels, so a static cluster's flux oscillates at the de Broglie internal clock frequency ω₀∝M_REST (FTD-0271). Native flux is massless (A0), so the clock is imposed, not forced. Additive; default OFF ⇒ golden-neutral. GPU-ported 2026-06-20 (CUDA phase_read kernel applies the same KG term, toggle-gated).
    bool db_clock_coulomb = false;  // [IMPOSED diagnostic] FTD-0281: pre-solve the live Coulomb Poisson field and apply omega_eff^2 = omega0^2 + 2*omega0*V to the clocked flux field at every site, with V=-phi_coulomb in the engine force convention. CPU + GPU (CUDA gpu_phase_read pre-solves d_phi_coulomb via FFT, then the kernel applies the all-site KG term); default OFF => golden-neutral.
    bool knot_tracking = false;     // [OBSERVATION-ONLY] tick-end: record per-knot telemetry from settled state. Reads voxels()/lattice()/current_tick() only ⇒ golden-neutral by construction.

    // D-3 / E-1 (2026-04-27): JS scale-0 scenario library has been pushing a
    // `confinement` bool through setToggle(); without a backing field the
    // value was silently dropped by the binding map. The C++ confinement
    // physics is implemented inside compute_color_force()'s three-regime
    // profile (Coulomb / transition / linear), gated on `color_forces` and
    // `strong_force`. This field exists so JS overrides land somewhere
    // observable; it is ALIASED to "color_forces && strong_force linear regime
    // active" and is not yet consumed by any C++ branch. Treat it as an
    // intent flag.
    bool confinement = false;

    // ARCH-3 (2026-04-25): when true, RenderBridge::tick() THROWS on the
    // first validate() failure instead of printing to stderr and continuing.
    bool strict_validation = false;

    // EW phase-transition background sweep: sinusoidal uniform +x flux drive
    // D(t)=(sin(tick*0.01)+1)/2*0.05, runs each tick before phase_read so the
    // driven field sees the wave propagation update in the same cycle.
    bool ew_background_sweep = false;

    // ── Non-bool config fields (NOT in TOGGLE_SPECS[]) ────────────────
    // These are typed parameters / enum modes, not boolean toggles, so
    // they live outside the table. Direct field access only.

    // Flux-field boundary law (see FluxBoundaryMode above). Default Periodic
    // (toroidal wrap) ⇒ golden-neutral. Non-bool config, like bcc_stencil.
    FluxBoundaryMode flux_boundary = FluxBoundaryMode::Periodic;

    // Cluster A (FTD-0093 / Mechanism C): sublattice stencil mode for phase_read.
    BccStencilMode bcc_stencil = BccStencilMode::FULL;

    // Cluster A: voxel-parity filter for the Langevin thermostat.
    SiteClass langevin_site_filter = SiteClass::ALL_SITES;

    // Langevin thermalization parameters (used only when langevin == true).
    double langevin_T     = 0.0;
    double langevin_gamma = 0.01;
    unsigned int langevin_seed = 1;  // RNG seed for reproducibility

    // Phase H (Apr 2026): explicit coupling constant in the Gauss law source.
    double coulomb_charge_coupling = 1.0;

    // FTD-0281 helium extension (2026-06-20): nuclear-charge scale Z applied to
    // the *Coulomb* Poisson source (solve_coulomb_poisson), which produces
    // phi_coulomb_ — the field that drives the db_clock_coulomb KG term at
    // phase_read (omega_eff² = ω₀² − 2ω₀·phi_C). The Coulomb RHS becomes
    // rho = −Z·(state − mean_charge), so phi_C → Z·phi_C and the well depth is
    // ×Z (Z=2 = He+). This is DISTINCT from coulomb_charge_coupling, which only
    // scales the Gauss *flux-projection* source (gauss_project, the phi_ buffer)
    // and does NOT touch phi_coulomb_. Default 1.0 = hydrogen, leaving the
    // Coulomb RHS bit-identical (golden-neutral; the db_clock_coulomb / Coulomb
    // solve path is dormant in the golden profile). Honored on CPU and GPU.
    double coulomb_source_scale = 1.0;

    // FTD-0271 (2026-06-11): de Broglie internal-clock frequency ω₀ [rad/tick],
    // used only when de_broglie_clock == true. The KG mass term is −ω₀²·J.
    // ω₀∝M_REST is [IMPOSED] (native flux is massless); M_REST→ω₀ scale is
    // [SELECTION] (no ℏ in the substrate). Stability bound: ω₀·dt < 2.
    double omega0 = 1.0;

    // FTD-0276 (2026-06-12): runtime kinetic-drain knob. Fraction of wave_vel
    // consumed at a genesis manifestation event (latent heat of mass-gap
    // creation; v.wave_vel *= (1 − kinetic_drain)). Default 0.5 reproduces the
    // legacy constexpr K_GENESIS_KINETIC_DRAIN exactly (golden-neutral). Honored
    // on both CPU and GPU single-substrate paths. Exposed to test whether the
    // cluster-efficiency k_eff scales as drain² (Leg A of FTD-0276).
    double kinetic_drain = 0.5;

    // ── Generated helpers — bodies live in this header (header-only,
    // POD struct preserved). Implementations below TOGGLE_SPECS[]. ────
    bool validate(std::string* err = nullptr) const;
    std::string cpu_runtime_warnings() const;
    void enable_all();
    void disable_all();
};

// ─────────────────────────────────────────────────────────────────────
// ToggleSpec — one row per boolean toggle. Adding a toggle = one new
// row here (and one new field in TermToggles above). The helpers below
// (validate, enable_all, disable_all, cpu_runtime_warnings) consume this
// table, so no other edit is required.
// ─────────────────────────────────────────────────────────────────────
struct ToggleSpec {
    const char* name;                  // canonical key (matches struct field name)
    bool TermToggles::*field;          // pointer-to-member (typed)
    bool default_value;                // also: value applied by enable_all()
    bool bulk_managed;                 // included in enable_all() / disable_all()
    const char* requires_;             // empty or comma-separated dependency names
    const char* conflicts;             // empty or single conflict name
    const char* gpu_only_warning;      // empty or warning string for cpu_runtime_warnings()
    uint8_t backends;                  // bitmask: CPU=1, GPU=2, JS=4
    const char* description;
};

inline constexpr ToggleSpec TOGGLE_SPECS[] = {
    // {name, field, default, bulk_managed, requires, conflicts, gpu_only_warning, backends, description}
    {"wave_propagation",   &TermToggles::wave_propagation,   true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-read 18-pt Laplacian wave equation"},
    {"coupling",           &TermToggles::coupling,           true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-read state-flux coupling g_c*grad(s)"},
    {"damping",            &TermToggles::damping,            true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-write exponential flux decay at rate alpha"},
    {"genesis",            &TermToggles::genesis,            true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-write manifestation + evaporation (master)"},
    {"evaporation",        &TermToggles::evaporation,        false, true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-write evaporation alone (OR'd with genesis; test isolation)"},
    {"gauss_projection",   &TermToggles::gauss_projection,   true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Enforce div(J) = s constraint via SOR Poisson"},
    {"forces",             &TermToggles::forces,             true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-forces master toggle (EM + gravity)"},
    {"gravity",            &TermToggles::gravity,            true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Gravitational force F = G_N*grad(rho)"},
    {"poisson_coulomb",    &TermToggles::poisson_coulomb,    true,  true,  "",                 "emergent_forces",  "", ToggleBackend::ANY, "Solve Poisson for Coulomb potential phi"},
    {"movement",           &TermToggles::movement,           true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Particle position integration"},
    {"lorentz_force",      &TermToggles::lorentz_force,      true,  true,  "forces",           "",                 "", ToggleBackend::ANY, "Magnetic Lorentz force F = alpha*s*(v x B)"},
    {"selective_damping",  &TermToggles::selective_damping,  true,  true,  "damping",          "",                 "", ToggleBackend::ANY, "Damp only near manifested particles"},
    {"larmor_radiation",   &TermToggles::larmor_radiation,   false, true,  "damping",          "langevin",         "", ToggleBackend::ANY, "Acceleration-squared radiation damping"},
    {"dual_substrate",     &TermToggles::dual_substrate,     true,  true,  "",                 "",                 "", ToggleBackend::ANY, "J_L / J_R chirality split"},
    {"color_forces",       &TermToggles::color_forces,       false, true,  "",                 "",                 "", ToggleBackend::ANY, "SU(3)-inspired color coupling"},
    {"weak_transmutation", &TermToggles::weak_transmutation, true,  true,  "dual_substrate",   "",                 "", ToggleBackend::ANY, "Chirality flip flavor-changing weak"},
    {"strong_force",       &TermToggles::strong_force,       false, true,  "",                 "",                 "strong_force has no CPU implementation — toggle is a no-op on CPU builds\n", ToggleBackend::ANY, "Yukawa short-range nuclear force"},
    {"triad_binding",      &TermToggles::triad_binding,      false, true,  "color_forces",     "",                 "", ToggleBackend::ANY, "Color-singlet triad binding (locked=true)"},
    {"pair_production",    &TermToggles::pair_production,    false, true,  "",                 "",                 "", ToggleBackend::ANY, "Correlated +1/-1 pair manifestation (independent code path; F11.A-5 audit removed artificial 'requires genesis' — pair_production_cpu / GPU pair-production kernel are SEPARATE phases from phase_write::genesis and operate on their own state==0 + jmag>K_GENESIS check)"},
    {"exchange_force",     &TermToggles::exchange_force,     false, true,  "poisson_coulomb",  "",                 "exchange_force has no CPU implementation — toggle is a no-op on CPU builds\n", ToggleBackend::ANY, "Pauli exclusion repulsion (same-spin)"},
    {"latency_field",      &TermToggles::latency_field,      false, true,  "gravity",          "",                 "", ToggleBackend::ANY, "Poisson-based latency field (gravity proxy)"},
    {"exact_dual_gauss",   &TermToggles::exact_dual_gauss,   false, false, "",                 "",                 "", ToggleBackend::ANY, "Exact dual-cell face-flux Gauss projection"},
    {"emergent_forces",    &TermToggles::emergent_forces,    false, false, "",                 "poisson_coulomb",  "", ToggleBackend::ANY, "EFT mode: force from flux gradient (no Poisson)"},
    {"langevin",           &TermToggles::langevin,           false, false, "",                 "larmor_radiation", "", ToggleBackend::ANY, "Stochastic OU thermostat (CPU only at runtime)"},
    {"symplectic_leapfrog", &TermToggles::symplectic_leapfrog, false, true,  "wave_propagation", "",                 "", ToggleBackend::ANY, "Symplectic leapfrog wave integration"},
    {"verlet_wave_integrator", &TermToggles::verlet_wave_integrator, false, false, "wave_propagation", "symplectic_leapfrog", "", ToggleBackend::CPU, "[E1/FTD-0337] Velocity-Verlet (KDK) bare-wave integrator: half-kick + drift in phase_write, second half-kick after a post-drift phase_read. CPU-only; honors dt<1. Default OFF => golden-neutral"},
    {"su2_gauge",           &TermToggles::su2_gauge,           false, true,  "",                 "",                 "", ToggleBackend::ANY, "SU(2) link staple relaxation each tick ([IMPOSED] lattice-gauge import; links are observables only — no feedback into the substrate)"},
    {"su3_gauge",           &TermToggles::su3_gauge,           false, true,  "",                 "",                 "", ToggleBackend::ANY, "SU(3) link staple relaxation each tick ([IMPOSED] lattice-gauge import; links are observables only — no feedback into the substrate)"},
    {"symmetric_movement_order", &TermToggles::symmetric_movement_order, false, true,  "movement",         "",                 "", ToggleBackend::ANY, "Coordinate-independent update traversal & axis ordering"},
    {"absorbing_boundary", &TermToggles::absorbing_boundary, false, true,  "wave_propagation", "",                 "", ToggleBackend::ANY, "Sponge boundary: outgoing waves disperse into the void at lattice faces"},
    {"reflective_boundary", &TermToggles::reflective_boundary, false, true, "movement",         "",                 "", ToggleBackend::ANY, "Mirror-bounce particles at lattice faces; when off they exhaust into the void (no toroidal wrap)"},
    {"field_energy_gravity", &TermToggles::field_energy_gravity, false, true, "latency_field",    "",                 "", ToggleBackend::ANY, "[IMPOSED] Latency Poisson sources from field-energy density (½|J|²) so flux configs gravitate"},
    {"cluster_inertia",    &TermToggles::cluster_inertia,    false, false, "forces",           "",                 "", ToggleBackend::ANY, "[IMPOSED] Rigid-body cluster inertia: locked clusters integrate a_COM = F_cluster/(N*M_REST)"},
    {"de_broglie_clock",   &TermToggles::de_broglie_clock,   false, false, "",                 "",                 "", ToggleBackend::ANY, "[IMPOSED] de Broglie internal clock: Klein-Gordon mass term -omega0^2*J at manifested voxels (FTD-0271). GPU-ported 2026-06-20: the CUDA phase_read kernel applies the same -omega0^2*J KG term, gated by the toggle (default OFF => golden-neutral). Independent of wave_propagation: with the wave term the full KG dispersion omega^2=c^2 k^2 + omega0^2 acts; alone, each manifested voxel is the k=0 rest-frame clock oscillating at omega0."},
    {"db_clock_coulomb",   &TermToggles::db_clock_coulomb,   false, false, "wave_propagation,de_broglie_clock,poisson_coulomb", "forces", "", ToggleBackend::ANY, "[IMPOSED diagnostic] FTD-0281 live Coulomb clock: pre-read phi_C solve plus all-site KG potential omega_eff^2=omega0^2+2*omega0*V, V=-phi_C. GPU-ported 2026-06-20 (CUDA gpu_phase_read pre-solves d_phi_coulomb via FFT then applies the same all-site KG term). forces must stay off to avoid a second same-tick Coulomb solve."},
    {"confinement",        &TermToggles::confinement,        false, false, "",                 "",                 "", ToggleBackend::ANY, "Linear confinement intent flag (no C++ branch yet)"},
    {"knot_tracking",      &TermToggles::knot_tracking,      false, false, "",                 "",                 "", ToggleBackend::ANY, "[OBSERVATION-ONLY] Record per-knot telemetry at end of tick (golden-neutral)"},
    {"strict_validation",  &TermToggles::strict_validation,  false, false, "",                 "",                 "", ToggleBackend::ANY, "Throw on validate() failure (vs. stderr warn)"},
    {"ew_background_sweep",&TermToggles::ew_background_sweep,false, false, "",                 "",                 "", ToggleBackend::ANY, "Sinusoidal uniform +x flux drive for EW phase-transition hysteresis (D=(sin(tick*0.01)+1)/2*0.05 per tick before phase_read)"},
};

// ─────────────────────────────────────────────────────────────────────
// Internal helpers (header-only). C++17, no <algorithm>/<vector> needed.
// ─────────────────────────────────────────────────────────────────────
namespace term_toggles_detail {

// Lookup a spec by name. Returns nullptr if not found.
inline const ToggleSpec* find_spec(std::string_view name) {
    for (const auto& s : TOGGLE_SPECS) {
        if (name == s.name) return &s;
    }
    return nullptr;
}

// Iterate comma-separated dep names, calling `cb` with each trimmed view.
// Skips empty entries. Returns true if `cb` returned true for all entries.
template <typename Cb>
inline void for_each_csv(const char* csv, Cb cb) {
    if (!csv || !*csv) return;
    std::string_view view(csv);
    while (!view.empty()) {
        auto comma = view.find(',');
        std::string_view tok = (comma == std::string_view::npos) ? view : view.substr(0, comma);
        // Trim whitespace
        while (!tok.empty() && (tok.front() == ' ' || tok.front() == '\t')) tok.remove_prefix(1);
        while (!tok.empty() && (tok.back()  == ' ' || tok.back()  == '\t')) tok.remove_suffix(1);
        if (!tok.empty()) cb(tok);
        if (comma == std::string_view::npos) break;
        view.remove_prefix(comma + 1);
    }
}

}  // namespace term_toggles_detail

// ─────────────────────────────────────────────────────────────────────
// Generated helper implementations.
// ─────────────────────────────────────────────────────────────────────
inline bool TermToggles::validate(std::string* err) const {
    using namespace term_toggles_detail;
    std::string msg;

    // Pass 1: table-driven requires_/conflicts checks.
    for (const auto& spec : TOGGLE_SPECS) {
        if (!(this->*(spec.field))) continue;  // only check enabled toggles

        // requires_: every listed dep must also be true.
        for_each_csv(spec.requires_, [&](std::string_view dep) {
            const ToggleSpec* dep_spec = find_spec(dep);
            if (!dep_spec) return;
            if (!(this->*(dep_spec->field))) {
                msg += spec.name;
                msg += " requires ";
                msg += dep_spec->name;
                msg += "\n";
            }
        });

        // conflicts: listed conflict must be false. We emit the message only
        // from the FIRST-defined side of the pair to avoid duplicate output
        // (e.g. emergent_forces lists poisson_coulomb; poisson_coulomb does
        // not list emergent_forces — by convention only the OFF-by-default
        // toggle of a mutex pair declares the conflict).
        if (spec.conflicts && *spec.conflicts) {
            const ToggleSpec* conf_spec = find_spec(spec.conflicts);
            if (conf_spec && (this->*(conf_spec->field))) {
                msg += spec.name;
                msg += " and ";
                msg += conf_spec->name;
                msg += " are mutually exclusive\n";
            }
        }
    }

    // Pass 2: cross-cutting rules that don't fit the per-spec model.
    // (Kept hand-rolled because they involve non-bool fields, multi-target
    // dependencies, or custom phrasing the test suite pins on.)

    // Cluster A: BCC sub-stencil currently uses single-substrate path only.
    if (bcc_stencil != BccStencilMode::FULL && dual_substrate)
        msg += "bcc_stencil != FULL requires dual_substrate=false (single-substrate path; dual-substrate BCC is OPEN-7)\n";
    // Cluster A: a non-default Langevin site filter requires Langevin to be on.
    if (langevin_site_filter != SiteClass::ALL_SITES && !langevin)
        msg += "langevin_site_filter != ALL_SITES requires langevin=true\n";
    // Note: selective_damping->damping requirement is encoded in TOGGLE_SPECS;
    // legacy phrasing was "has no effect with damping=false" but the table
    // generates "selective_damping requires damping" (semantically equivalent;
    // no test pins on the wording).
    if (bcc_stencil != BccStencilMode::FULL && !wave_propagation)
        msg += "bcc_stencil != FULL requires wave_propagation=true (sublattice projection requires the wave path)\n";
    // Census correction (EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING §5.3,
    // AUDIT_EFFECTIVE_TOGGLES_2026-07): triad detection reads states,
    // positions, and locked flags only — no J_L/J_R — on both backends
    // (transmutation_phases.cpp triad_binding_cpu; kernels_forces.cu GPU
    // triad detection). The requirement is retained as declared; only the
    // old "(operates on J_L/J_R)" rationale was drift.
    if (triad_binding && !dual_substrate)
        msg += "triad_binding requires dual_substrate (requirement of record; triad detection itself is geometric — states + distances, no flux-field read)\n";
    if (db_clock_coulomb && dual_substrate)
        msg += "db_clock_coulomb requires dual_substrate=false (FTD-0281 v1 is a single-substrate spectroscopy diagnostic)\n";

    if (err) *err = msg;
    return msg.empty();
}

// F2 (callstack audit 2026-04-17): warn about toggles whose
// implementation lives only on the GPU path. Generated from
// ToggleSpec::gpu_only_warning entries in the table.
inline std::string TermToggles::cpu_runtime_warnings() const {
    std::string msg;
    for (const auto& spec : TOGGLE_SPECS) {
        if (!(this->*(spec.field))) continue;
        if (spec.gpu_only_warning && *spec.gpu_only_warning) {
            msg += spec.gpu_only_warning;
        }
    }
    return msg;
}

// enable_all() — restore the recommended profile (= each toggle's
// `default_value`) for every bulk_managed toggle. Non-bulk toggles
// (langevin, emergent_forces, exact_dual_gauss, confinement,
// strict_validation) are left untouched, matching legacy semantics.
// Non-bool config fields (bcc_stencil, langevin_site_filter) are reset
// to their canonical default mode.
inline void TermToggles::enable_all() {
    for (const auto& spec : TOGGLE_SPECS) {
        if (!spec.bulk_managed) continue;
        this->*(spec.field) = spec.default_value;
    }
    bcc_stencil = BccStencilMode::FULL;
    langevin_site_filter = SiteClass::ALL_SITES;
}

// disable_all() — clear every bulk_managed toggle to false. Non-bulk
// toggles untouched; non-bool config fields reset to canonical defaults.
inline void TermToggles::disable_all() {
    for (const auto& spec : TOGGLE_SPECS) {
        if (!spec.bulk_managed) continue;
        this->*(spec.field) = false;
    }
    bcc_stencil = BccStencilMode::FULL;
    langevin_site_filter = SiteClass::ALL_SITES;
}

}  // namespace ftd
