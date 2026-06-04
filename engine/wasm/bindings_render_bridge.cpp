/**
 * @file bindings_render_bridge.cpp
 * @brief Embind bindings for RenderBridge (Scale 0 — voxel lattice engine).
 *
 * Extracted from ftd_wasm.cpp as part of W1-W3. Contains:
 *   - rb_toggle_map (pointer-to-member TermToggles lookup)
 *   - get_toggle / set_toggle
 *   - Injection wrappers (inject_particle_simple, inject_wavepacket_simple,
 *     inject_flux, create_entangled_pair)
 *   - Time step control
 *   - setup_scenario (legacy backward-compat + dispatch to ftd::dispatch_scenario)
 *   - The RenderBridge class_<> binding itself
 *
 * Shared typed-array helpers live in ftd_wasm.cpp and are declared in
 * bindings_internal.h.
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <unordered_map>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/scenarios.h"  // ftd::dispatch_scenario — ported JS scenario library
#include "bindings_internal.h"

using namespace emscripten;

// ── Toggle wrapper ───────────────────────────────────────────────────
// Pointer-to-member map for TermToggles, populated from the canonical
// TOGGLE_SPECS[] table in term_toggles.h. Adding a new toggle requires
// no edit here — the table is the single source of truth. Filtered to
// JS-visible toggles via the `backends` bitmask.
using RbBoolPTM = bool ftd::TermToggles::*;
static const std::unordered_map<std::string, RbBoolPTM>& rb_toggle_map() {
    static const std::unordered_map<std::string, RbBoolPTM> kMap = [] {
        std::unordered_map<std::string, RbBoolPTM> m;
        for (const auto& spec : ftd::TOGGLE_SPECS) {
            if (spec.backends & ftd::ToggleBackend::JS) {
                m.emplace(spec.name, spec.field);
            }
        }
        return m;
    }();
    return kMap;
}

static void set_toggle(ftd::RenderBridge& rb, const std::string& name, bool value) {
    auto it = rb_toggle_map().find(name);
    if (it != rb_toggle_map().end()) rb.toggles.*(it->second) = value;
}

static bool get_toggle(ftd::RenderBridge& rb, const std::string& name) {
    auto it = rb_toggle_map().find(name);
    if (it != rb_toggle_map().end()) return rb.toggles.*(it->second);
    return false;
}

// ── Inject wrappers ──────────────────────────────────────────────────
static void inject_particle_simple(ftd::RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_particle(x, y, z, static_cast<int8_t>(state), ftd::Vec3(0, 0, 0));
}

static void inject_wavepacket_simple(ftd::RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_wavepacket(x, y, z, static_cast<int8_t>(state));
}

static void inject_flux(ftd::RenderBridge& rb, int x, int y, int z,
                         double fx, double fy, double fz) {
    rb.inject_flux(x, y, z, ftd::Vec3(fx, fy, fz));
}

// NOTE: the original ftd_wasm.cpp defined static wrappers inject_flux_add /
// inject_wave_vel_add for RenderBridge::inject_flux_add and ::inject_wave_vel_add,
// but never registered them with EMSCRIPTEN_BINDINGS and never called them
// from within the TU. The additive injectors are reached from C++ scenario
// helpers (src/scenarios/_helpers.h) directly on the RenderBridge instance,
// so the WASM wrappers were dead code. Dropped during the W1-W3 extraction.
// If a future binding needs to expose them to JS, add them here.

static void create_entangled_pair(ftd::RenderBridge& rb, int x, int y, int z,
                                   double fx, double fy, double fz) {
    rb.create_entangled_pair(x, y, z, ftd::Vec3(fx, fy, fz));
}

// ── Energy Ledger (D-1, M2: per-tick conservation bookkeeping) ──────
// Mirrors the EnergyLedger struct in include/ftd/render_bridge.h. Tests
// and dashboards can poll this every tick to see drift_frac / residual /
// the running cumulative_injection / cumulative_dissipation totals.
// Distinct from getEnergyAudit (one-shot snapshot of current totals).
static val get_energy_ledger(ftd::RenderBridge& rb) {
    const ftd::EnergyLedger& el = rb.energy_ledger();
    val result = val::object();
    result.set("tickPrev",             el.tick_prev);
    result.set("EPrev",                el.E_prev);
    result.set("ECurr",                el.E_curr);
    result.set("dEdt",                 el.dE_dt);
    result.set("driftFrac",            el.drift_frac);
    result.set("expectedRate",         el.expected_rate);
    result.set("residual",             el.residual);
    result.set("cumulativeInjection",  el.cumulative_injection);
    result.set("cumulativeDissipation",el.cumulative_dissipation);
    result.set("maxResidualSeen",      el.max_residual_seen);
    return result;
}

// ── Time step control ────────────────────────────────────────────────
static void set_dt(ftd::RenderBridge& rb, double dt) { rb.set_dt(dt); }
static double get_dt(ftd::RenderBridge& rb) { return rb.dt(); }
static double get_physical_time(ftd::RenderBridge& rb) { return rb.physical_time(); }

// ── Scenario setup ───────────────────────────────────────────────────
// NOTE: Primary scenario dispatch path is ftd::dispatch_scenario(rb, name)
// (src/scenarios.cpp + include/ftd/scenarios.h) which owns every flux-*,
// light-*, quantum-*, s0-seed-*, s0-field-* scenario in the UI registry.
// The legacy switch below handles ONLY prefixless backward-compat names
// (pair, cluster, wave, hydrogen, scattering, annihilation, dipole,
// entangled, force, interference, vacuum, triad, production, empty) that
// the dispatcher does not prefix-match. These remain for older tests and
// saved dashboards.
//
// Removed 2026-04-18: 21 dead flux-*/light-* branches that were either
// (a) dispatcher-duplicates (pulse/dipole/standing/soliton/cascade/
// interference/vortex/pair-production/random-genesis/dual-substrate,
// light-rainbow/dipole/two-slit/photon-race) or (b) unreachable orphans
// whose prefix the dispatcher swallows with an unconditional return-true
// (light-prism, flux-collision/damping/dispersion/gravity-cluster/
// hydrogen/ring). None had tests or JS callers. See git log for the
// deleted bodies if you need to restore any as proper dispatcher entries.
//
// JS<->WASM parity: any scenario name in the UI registry (engine/web/js/
// scales/scale0/scenario-registry.js) produces identical physics on both
// backends. Single-source authority: src/scenarios.cpp (C++) + engine/
// web/js/bridge/scenarios/*.js (JS); both are hand-maintained mirrors.
// ── Legacy backward-compat alias map (audit-4 2026-04-28) ──────────
// Older saved-state JSONs may reference these short names. Each is now
// aliased to its closest modern scenario name; the per-name body code
// (~80 LOC) was deleted in favour of a single redispatch through
// dispatch_scenario(). Aliases were chosen by scenario-body similarity
// at the time of removal — see git log for the deleted bodies if you
// need to restore exact pre-2026-04-28 behavior for any name.
struct LegacyAlias { const char* old_name; const char* modern_name; };
static constexpr LegacyAlias kLegacyAliases[] = {
    {"pair",         "flux-pair-production"},
    {"production",   "flux-pair-production"},
    {"interference", "flux-interference"},
    {"force",        "flux-pulse"},
    {"hydrogen",     "s0-seed-hydrogen"},
    {"entangled",    "quantum-entangle"},
    {"annihilation", "s0-seed-ee-annihilation"},
    {"triad",        "flux-triad"},
    {"dipole",       "flux-dipole"},
    {"scattering",   "s0-seed-ee-annihilation"},
    {"wave",         "flux-pulse"},
    {"cluster",      "s0-seed-stella-octangula"},
    {"vacuum",       "flux-vacuum-foam"},
};

static void setup_scenario(ftd::RenderBridge& rb, const std::string& name) {
    // Primary path: ported JS scenario library (flux-/light-/quantum-/
    // s0-seed-/s0-vacuum-/s0-field-). Dispatcher returns true on prefix match.
    if (ftd::dispatch_scenario(rb, name)) return;

    // Backward-compat: 'empty' is also handled by the JS dispatcher (index.js)
    // as an early-return; the C++ dispatcher does not match it via prefix, so
    // keep this explicit no-op so legacy callers see consistent behavior.
    if (name == "empty") return;

    // Resolve any legacy short name to its modern alias and redispatch.
    for (const auto& alias : kLegacyAliases) {
        if (name == alias.old_name) {
            ftd::dispatch_scenario(rb, alias.modern_name);
            return;
        }
    }

    // Unknown name — silently no-op (matches pre-2026-04-28 behavior; the
    // primary dispatcher already returned false above).
}

// ── Embind Registration ──────────────────────────────────────────────
// All RB-related helpers (data extraction, inspection, scenario setup,
// toggles, injection, time step) register here. The RenderBridge class_<>
// itself lives in ftd_wasm.cpp so the core tick/run/currentTick surface
// stays next to its constructor.
EMSCRIPTEN_BINDINGS(ftd_module_render_bridge) {
    using namespace ftd_wasm_internal;

    // Data extraction
    function("getParticleData",    &get_particle_data);
    function("getDiagnostics",     &get_diagnostics);
    function("getEnergyAudit",     &get_energy_audit);
    function("getEnergyLedger",    &get_energy_ledger);
    function("getLagrangian",      &get_lagrangian);
    function("getConstants",       &get_constants);
    function("getLatticeSize",     &get_lattice_size);

    // Voxel inspection
    function("inspectVoxel",       &inspect_voxel);
    function("getForceAt",         &get_force_at);

    // Bulk flux extraction (for flux volume visualization)
    function("getFluxSlice",       &get_flux_slice);
    function("getFluxVolume",      &get_flux_volume);

    // Bulk sampled vector field exports (for field line / arrow visualization)
    function("getEFieldSampled",      &get_e_field_sampled);
    function("getBFieldSampled",      &get_b_field_sampled);
    function("getPoyntingSampled",    &get_poynting_sampled);
    function("getDivJSampled",        &get_divj_sampled);
    function("getFluxVectorSampled",  &get_flux_vector_sampled);
    function("getForceFieldSampled",  &get_force_field_sampled);

    // Force-field decomposition samplers (2026-04-19) — per-voxel force vectors
    // decomposed by physical interaction, for force-arrow overlay visualization.
    function("getGravityFieldSampled", &get_gravity_field_sampled);
    function("getEMForceField",        &get_em_force_field);
    function("getStrongForceField",    &get_strong_force_field);

    // Scalar / derived field samplers (2026-06-03) — vorticity, helicity,
    // curl, coherence, Fisher, latency, Kretschmann, and the ternary state
    // field. Light up the topology + phenomena overlays on WASM-owned
    // scenarios (empty/light/quantum) that previously rendered nothing.
    function("getVorticitySampled",   &get_vorticity_sampled);
    function("getHelicitySampled",    &get_helicity_sampled);
    function("getCurlJSampled",       &get_curlj_sampled);
    function("getCoherenceSampled",   &get_coherence_sampled);
    function("getFisherSampled",      &get_fisher_sampled);
    function("getLatencySampled",     &get_latency_sampled);
    function("getKretschmannSampled", &get_kretschmann_sampled);
    function("getStateFieldSampled",  &get_state_field_sampled);
    function("getGaussResidualSampled", &get_gauss_residual_sampled);

    // Direct Coulomb-potential ray sampling (2026-04-27) — engine-side
    // trilinear interpolation of phi_coulomb_ for the P1 Coulomb panel.
    function("sampleVAtRay",           &sample_v_at_ray);

    // Controls
    function("setToggle",          &set_toggle);
    function("getToggle",          &get_toggle);

    // Injection
    function("injectParticle",     &inject_particle_simple);
    function("injectWavepacket",   &inject_wavepacket_simple);
    function("injectFlux",         &inject_flux);
    function("createEntangledPair", &create_entangled_pair);

    // Time step control
    function("setDt",              &set_dt);
    function("getDt",              &get_dt);
    function("getPhysicalTime",    &get_physical_time);

    // Scenarios
    function("setupScenario",      &setup_scenario);
}
