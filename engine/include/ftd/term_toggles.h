#pragma once
// Runtime toggles for the logic-first engine.
// 44 boolean toggles plus typed non-bool configuration fields.
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
//   Periodic   — toroidal wrap (closed, translation-invariant finite lattice)
//   Reflective — one copied ghost shell imposing a discrete Neumann condition
//   Dispersal  — outer-shell multiplier keep=1-C_SPEED (an imposed lossy shell,
//                not a Mur/Sommerfeld radiation condition)
enum class FluxBoundaryMode : int { Periodic = 0, Reflective = 1, Dispersal = 2 };

// Backend bitmask used by ToggleSpec::backends. CPU = 0b001, GPU = 0b010,
// JS  = 0b100, ANY = 0b111. The native WebSocket binding enforces this table
// transactionally through validate_backend(); the WASM map also filters by
// `(backends & 0b100)` so unsupported terms cannot be acknowledged silently.
namespace ToggleBackend {
    constexpr uint8_t CPU = 0b001;
    constexpr uint8_t GPU = 0b010;
    constexpr uint8_t JS  = 0b100;
    constexpr uint8_t ANY = CPU | GPU | JS;
}

struct TermToggles {
    // ── Boolean toggles (table-managed; see TOGGLE_SPECS[] below) ─────
    bool wave_propagation = true;   // phase_read: Laplacian wave equation
    bool coupling = true;           // phase_read: -g_c * grad(s) source term (Term 2, 2026-07-18)
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
    bool strong_stress_energy = false; // [OWNER-AUTHORIZED SELECTION, FTD-0406] CPU pair Hamiltonian, energy projection, local string T00/stress, and T00/C_SPEED^2 latency source
    bool weak_transmutation = true; // tick: chirality/stress polarity flip (+1 ↔ -1)
    bool strong_force = false;      // phase_forces: Yukawa short-range nuclear force
    bool triad_binding = false;     // tick: detect 3-particle triads, set locked=true
    bool pair_production = false;   // genesis: correlated +1/-1 pairs from high-flux void
    bool exchange_force = false;    // phase_forces: Pauli exclusion repulsion (same-spin)
    bool latency_field = false;     // Poisson-based latency field ∇²L = 4πGρ (gravity potential)
    bool exact_dual_gauss = false;  // gauss_project: exact dual-cell face-flux projection
    bool matched_gauss_dynamics = false; // [SELECTED ENGINE EXTENSION, FTD-0428] CPU-only oriented-face Maxwell/Gauss evolution; isolated from legacy flux writers and reactions
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
    bool lorentz_period2_floquet = false; // [SELECTED PROTOTYPE, FTD-0408] P4-preserving
                                    // free-wave kick sequence +3/13, -1/13 on
                                    // even/odd ticks. Its exact two-tick pole
                                    // cancels the q^4 preferred-frame term and
                                    // is stable over the complete 18-point band.
                                    // CPU-only; requires the unit-step default
                                    // kick-drift integrator. Default OFF.
    bool lorentz_bcc_time_floquet = false; // [SELECTED IR PROTOTYPE, FTD-0411]
                                    // SC+FCC spatial propagation with a stable
                                    // two-tick localization of the selected
                                    // BCC temporal kernel. Kicks are
                                    // (1+sqrt(2))/7, (1-sqrt(2))/7; c^2=1/7.
                                    // Matches through q^4, not exact at q^6.
                                    // CPU-only, unit-step, default OFF.
    bool su2_gauge = false;         // tick Rule 7b: per-tick SU(2) link staple relaxation ([IMPOSED] Wilson-action import; links are write-only — no substrate feedback, see test_gauge_links G1)
    bool su3_gauge = false;         // tick Rule 7b: per-tick SU(3) link staple relaxation ([IMPOSED] Wilson-action import; links are write-only — no substrate feedback, see test_gauge_links G1)
    bool symmetric_movement_order = false; // CPU/CUDA phase_movement: coordinate-independent update traversal & axis ordering
    bool absorbing_boundary = false; // tick: imposed D-deep quadratic damping sponge; reflection performance is not guaranteed by the operator definition
    bool reflective_boundary = false; // phase_movement: mirror-bounce at faces when on; particles exhaust into the void when off (no periodic wrap)
    bool field_energy_gravity = false; // [IMPOSED] latency Poisson also sources from field-energy density ½(|J|²+|wave_vel|²), not only particle rest mass, so flux-only configs (gravity waves) carry a real potential. Requires latency_field.
    bool cluster_inertia = false;   // [IMPOSED] phase_forces: rigid-body integrate LOCKED clusters at inertial mass N·M_INERTIAL. Additive; needs a force channel.
    bool geometric_gravity = false; // [FTD-1016 SELECTED] phase_forces: replace F=G_N∇|J| with F=M_INERTIAL C² ℒ ∇ℒ from voxel.latency. Native CUDA + CPU; default OFF ⇒ golden-neutral.
    bool de_broglie_clock = false;  // [IMPOSED] phase_read: Klein-Gordon term −ω₀²·J with the frequency calibration tied explicitly to K_B (FTD-0271), not to a unified mass role. Native flux is massless (A0). GPU-ported 2026-06-20.
    bool db_clock_coulomb = false;  // [IMPOSED diagnostic] FTD-0281: pre-solve the live Coulomb Poisson field and apply omega_eff^2 = omega0^2 + 2*omega0*V to the clocked flux field at every site, with V=-phi_coulomb in the engine force convention. CPU + GPU (CUDA gpu_phase_read pre-solves d_phi_coulomb via FFT, then the kernel applies the all-site KG term); default OFF => golden-neutral.
    bool knot_tracking = false;     // [OBSERVATION-ONLY] tick-end: record per-knot telemetry from settled state. Reads voxels()/lattice()/current_tick() only ⇒ golden-neutral by construction.

    // [SELECTION] Linear colour string at r >= COLOR_TRANSITION_RADIUS.
    // Default color_forces keeps the harmonic F∝r shell. When this is on
    // (and color_forces is on), that shell is F = SIGMA_STRING * cf, the
    // ParticleEngine law. Not FTD-0025 (area-law Wilson loops remain a
    // closed negative at substrate level). Default OFF => golden-neutral.
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
    // ω₀∝K_B is [IMPOSED] (native flux is massless); K_B→ω₀ scale is
    // [SELECTION] (no ℏ in the substrate). Stability bound: ω₀·dt < 2.
    double omega0 = 1.0;

    // FTD-0276 (2026-06-12): runtime kinetic-drain knob. Fraction of wave_vel
    // consumed at a genesis manifestation event (selected drain;
    // v.wave_vel *= (1 − kinetic_drain)). FTD-0567 proves it is not an exact
    // common-action latent-heat identity. Default 0.5 reproduces the
    // legacy constexpr K_GENESIS_KINETIC_DRAIN exactly (golden-neutral). Honored
    // on both CPU and GPU single-substrate paths. Exposed to test whether the
    // cluster-efficiency k_eff scales as drain² (Leg A of FTD-0276).
    double kinetic_drain = 0.5;

    // ── Generated helpers — bodies live in this header (header-only,
    // POD struct preserved). Implementations below TOGGLE_SPECS[]. ────
    bool validate(std::string* err = nullptr) const;
    // Validate that every enabled term has a real implementation on the
    // selected backend.  `require_device_resident` is used by the native
    // interactive CUDA server: hybrid extensions that deliberately mirror the
    // whole lattice to the host are valid for campaigns, but are not allowed
    // to masquerade as full-GPU interactive physics.
    bool validate_backend(uint8_t backend,
                          bool require_device_resident = false,
                          std::string* err = nullptr) const;
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
    {"coupling",           &TermToggles::coupling,           true,  true,  "",                 "",                 "", ToggleBackend::ANY, "Phase-read state-flux coupling -g_c*grad(s)"},
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
    {"strong_stress_energy", &TermToggles::strong_stress_energy, false, false, "color_forces", "",                 "", ToggleBackend::ANY, "[FTD-0406 SELECTED] Collision-free strong Hamiltonian projection plus local string stress-energy; native CUDA + CPU"},
    {"weak_transmutation", &TermToggles::weak_transmutation, true,  true,  "dual_substrate",   "",                 "", ToggleBackend::ANY, "Chirality flip flavor-changing weak"},
    {"strong_force",       &TermToggles::strong_force,       false, true,  "",                 "",                 "", ToggleBackend::ANY, "Yukawa short-range nuclear force"},
    {"triad_binding",      &TermToggles::triad_binding,      false, true,  "color_forces",     "",                 "", ToggleBackend::ANY, "Color-singlet triad binding (locked=true)"},
    {"pair_production",    &TermToggles::pair_production,    false, true,  "",                 "",                 "", ToggleBackend::ANY, "Correlated +1/-1 pair manifestation (independent code path; F11.A-5 audit removed artificial 'requires genesis' — pair_production_cpu / GPU pair-production kernel are SEPARATE phases from phase_write::genesis and operate on their own state==0 + jmag>K_GENESIS check)"},
    {"exchange_force",     &TermToggles::exchange_force,     false, true,  "poisson_coulomb",  "",                 "", ToggleBackend::ANY, "Pauli exclusion repulsion (same-spin)"},
    {"latency_field",      &TermToggles::latency_field,      false, true,  "gravity",          "",                 "", ToggleBackend::ANY, "Poisson-based latency field (gravity proxy)"},
    {"exact_dual_gauss",   &TermToggles::exact_dual_gauss,   false, false, "",                 "",                 "", ToggleBackend::ANY, "Exact dual-cell face-flux Gauss projection"},
    {"matched_gauss_dynamics", &TermToggles::matched_gauss_dynamics, false, false, "",          "",                 "", ToggleBackend::ANY, "[FTD-0428 SELECTED ENGINE EXTENSION] Projection-free oriented-face Maxwell/Gauss evolution with event-routed conservative current; isolated native CUDA + CPU"},
    {"emergent_forces",    &TermToggles::emergent_forces,    false, false, "",                 "poisson_coulomb",  "", ToggleBackend::ANY, "EFT mode: force from flux gradient (no Poisson)"},
    {"langevin",           &TermToggles::langevin,           false, false, "",                 "larmor_radiation", "", ToggleBackend::ANY, "Stochastic OU thermostat (native CUDA + CPU; SplitMix64 per-voxel noise, default OFF => golden-neutral)"},
    {"symplectic_leapfrog", &TermToggles::symplectic_leapfrog, false, true,  "wave_propagation", "",                 "", ToggleBackend::ANY, "Symplectic leapfrog wave integration"},
    {"verlet_wave_integrator", &TermToggles::verlet_wave_integrator, false, false, "wave_propagation", "symplectic_leapfrog", "", ToggleBackend::ANY, "[E1/FTD-0337] Velocity-Verlet (KDK) bare-wave integrator: half-kick + drift in phase_write, second half-kick after a post-drift phase_read. Native CUDA + CPU; honors dt<1. Default OFF => golden-neutral"},
    {"lorentz_period2_floquet", &TermToggles::lorentz_period2_floquet, false, false, "wave_propagation", "verlet_wave_integrator", "", ToggleBackend::ANY, "[FTD-0408 SELECTED PROTOTYPE] P4-preserving period-two free-wave kicks +3/13 and -1/13. Cancels the q^4 Floquet-pole term; native CUDA + CPU, unit-step, default OFF"},
    {"lorentz_bcc_time_floquet", &TermToggles::lorentz_bcc_time_floquet, false, false, "wave_propagation", "lorentz_period2_floquet", "", ToggleBackend::ANY, "[FTD-0411 SELECTED IR PROTOTYPE] Stable P4-local period-two surrogate for the BCC temporal kernel, with c^2=1/7 and exact q^4 cancellation; differs from literal BCC time at q^6; native CUDA + CPU, unit-step, default OFF"},
    {"su2_gauge",           &TermToggles::su2_gauge,           false, true,  "",                 "",                 "", ToggleBackend::ANY, "SU(2) link staple relaxation each tick ([IMPOSED] lattice-gauge import; links are observables only — no feedback into the substrate)"},
    {"su3_gauge",           &TermToggles::su3_gauge,           false, true,  "",                 "",                 "", ToggleBackend::ANY, "SU(3) link staple relaxation each tick ([IMPOSED] lattice-gauge import; links are observables only — no feedback into the substrate)"},
    {"symmetric_movement_order", &TermToggles::symmetric_movement_order, false, true,  "movement",         "",                 "", ToggleBackend::ANY, "Coordinate-independent update traversal & axis ordering (native CUDA + CPU; SplitMix64 Fisher-Yates)"},
    {"absorbing_boundary", &TermToggles::absorbing_boundary, false, true,  "wave_propagation", "",                 "", ToggleBackend::ANY, "Imposed D-deep quadratic damping sponge at lattice faces"},
    {"reflective_boundary", &TermToggles::reflective_boundary, false, true, "movement",         "",                 "", ToggleBackend::ANY, "Mirror-bounce particles at lattice faces; when off they exhaust into the void (no toroidal wrap)"},
    {"field_energy_gravity", &TermToggles::field_energy_gravity, false, true, "latency_field",    "",                 "", ToggleBackend::ANY, "[IMPOSED] Latency Poisson sources from field-energy density (½|J|²) so flux configs gravitate"},
    {"cluster_inertia",    &TermToggles::cluster_inertia,    false, false, "",                 "",                 "", ToggleBackend::ANY, "[IMPOSED] Rigid-body cluster inertia: locked clusters integrate a_COM = F_cluster/(N*M_INERTIAL); requires a force channel"},
    {"geometric_gravity",  &TermToggles::geometric_gravity,  false, true,  "gravity,forces",   "",                 "", ToggleBackend::ANY, "[FTD-1016 SELECTED ENGINE EXTENSION] Replace F=G_N∇|J| with F=M_INERTIAL C² ℒ ∇ℒ from voxel.latency; native CUDA + CPU; default OFF => golden-neutral"},
    {"de_broglie_clock",   &TermToggles::de_broglie_clock,   false, false, "",                 "",                 "", ToggleBackend::ANY, "[IMPOSED] de Broglie internal clock: Klein-Gordon mass term -omega0^2*J at manifested voxels (FTD-0271). GPU-ported 2026-06-20: the CUDA phase_read kernel applies the same -omega0^2*J KG term, gated by the toggle (default OFF => golden-neutral). Independent of wave_propagation: with the wave term the full KG dispersion omega^2=c^2 k^2 + omega0^2 acts; alone, each manifested voxel is the k=0 rest-frame clock oscillating at omega0."},
    {"db_clock_coulomb",   &TermToggles::db_clock_coulomb,   false, false, "wave_propagation,de_broglie_clock,poisson_coulomb", "forces", "", ToggleBackend::ANY, "[IMPOSED diagnostic] FTD-0281 live Coulomb clock: pre-read phi_C solve plus all-site KG potential omega_eff^2=omega0^2+2*omega0*V, V=-phi_C. GPU-ported 2026-06-20 (CUDA gpu_phase_read pre-solves d_phi_coulomb via FFT then applies the same all-site KG term). forces must stay off to avoid a second same-tick Coulomb solve."},
    {"confinement",        &TermToggles::confinement,        false, false, "color_forces",     "",                 "", ToggleBackend::ANY, "[SELECTION] Linear colour string F=SIGMA_STRING·cf at r>=8; not FTD-0025. Native CUDA + CPU; default OFF"},
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
    if (lorentz_period2_floquet && symplectic_leapfrog)
        msg += "lorentz_period2_floquet and symplectic_leapfrog are mutually exclusive (FTD-0408 requires the unit-step default kick-drift map)\n";
    if (lorentz_bcc_time_floquet && verlet_wave_integrator)
        msg += "lorentz_bcc_time_floquet and verlet_wave_integrator are mutually exclusive (FTD-0411 requires the unit-step default kick-drift map)\n";
    if (lorentz_bcc_time_floquet && symplectic_leapfrog)
        msg += "lorentz_bcc_time_floquet and symplectic_leapfrog are mutually exclusive (FTD-0411 requires the unit-step default kick-drift map)\n";
    // Both CPU and CUDA implement the OU thermostat only on the canonical
    // single-substrate register.  Accepting Langevin with dual_substrate used
    // to acknowledge a profile whose stochastic phase was silently skipped.
    if (langevin && dual_substrate)
        msg += "langevin requires dual_substrate=false (OU thermostat is single-substrate only)\n";
    if (cluster_inertia
        && !forces && !color_forces && !strong_force && !exchange_force)
        msg += "cluster_inertia requires a force channel (forces, color_forces, strong_force, or exchange_force)\n";

    // FTD-0428: the matched face/edge complex owns all field evolution in its
    // selected branch.  Only conservative particle movement and read-only
    // observers may coexist.  This prevents an apparent Gauss failure from
    // actually being an unjournaled legacy writer or reaction.
    if (matched_gauss_dynamics) {
        if (flux_boundary != FluxBoundaryMode::Periodic)
            msg += "matched_gauss_dynamics requires flux_boundary=Periodic\n";
        if (wave_propagation || coupling || damping || genesis || evaporation
            || gauss_projection || forces || gravity || poisson_coulomb
            || lorentz_force || selective_damping || larmor_radiation
            || dual_substrate || color_forces || strong_stress_energy
            || weak_transmutation || strong_force || triad_binding
            || pair_production || exchange_force || latency_field
            || exact_dual_gauss || emergent_forces || langevin
            || symplectic_leapfrog || verlet_wave_integrator
            || lorentz_period2_floquet || lorentz_bcc_time_floquet
            || su2_gauge || su3_gauge || absorbing_boundary
            || reflective_boundary || field_energy_gravity || cluster_inertia
            || de_broglie_clock || db_clock_coulomb || ew_background_sweep
            || geometric_gravity) {
            msg += "matched_gauss_dynamics requires the isolated conservative movement sector\n";
        }
    }

    // FTD-0406 v1: exact energy projection is intentionally scoped to the
    // isolated flat colour sector. Static (movement=false) configurations are
    // allowed to exercise the selected local T00/C_SPEED^2 latency source.
    if (strong_stress_energy && movement) {
        if (!forces)
            msg += "strong_stress_energy with movement requires forces=true\n";
        if (damping || genesis || evaporation || pair_production
            || poisson_coulomb || emergent_forces || gravity || latency_field
            || lorentz_force || strong_force || exchange_force
            || weak_transmutation || triad_binding || absorbing_boundary
            || reflective_boundary) {
            msg += "strong_stress_energy projected movement requires the isolated flat collision-free colour sector\n";
        }
    }

    if (err) *err = msg;
    return msg.empty();
}

inline bool TermToggles::validate_backend(uint8_t backend,
                                          bool require_device_resident,
                                          std::string* err) const {
    std::string msg;
    for (const auto& spec : TOGGLE_SPECS) {
        if (!(this->*(spec.field))) continue;
        if ((spec.backends & backend) == 0) {
            msg += spec.name;
            msg += " is not implemented on the selected backend\n";
        }
    }

    if (backend == ToggleBackend::GPU && require_device_resident) {
        // knot_tracking still materializes the canonical AoS every tick.
        if (knot_tracking)
            msg += "knot_tracking requires a full host mirror and is unavailable in full-GPU interactive mode\n";
    }

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
