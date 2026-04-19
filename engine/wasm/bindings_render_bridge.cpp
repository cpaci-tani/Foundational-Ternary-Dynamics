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
// Pointer-to-member map for TermToggles (RenderBridge).
using RbBoolPTM = bool ftd::TermToggles::*;
static const std::unordered_map<std::string, RbBoolPTM>& rb_toggle_map() {
    static const std::unordered_map<std::string, RbBoolPTM> kMap = {
        {"wave_propagation",  &ftd::TermToggles::wave_propagation},
        {"coupling",          &ftd::TermToggles::coupling},
        {"damping",           &ftd::TermToggles::damping},
        {"genesis",           &ftd::TermToggles::genesis},
        {"gauss_projection",  &ftd::TermToggles::gauss_projection},
        {"forces",            &ftd::TermToggles::forces},
        {"gravity",           &ftd::TermToggles::gravity},
        {"poisson_coulomb",   &ftd::TermToggles::poisson_coulomb},
        {"movement",          &ftd::TermToggles::movement},
        {"lorentz_force",     &ftd::TermToggles::lorentz_force},
        {"selective_damping", &ftd::TermToggles::selective_damping},
        {"larmor_radiation",  &ftd::TermToggles::larmor_radiation},
        {"dual_substrate",    &ftd::TermToggles::dual_substrate},
        {"color_forces",      &ftd::TermToggles::color_forces},
        {"weak_transmutation",&ftd::TermToggles::weak_transmutation},
        {"strong_force",      &ftd::TermToggles::strong_force},
        {"triad_binding",     &ftd::TermToggles::triad_binding},
        {"pair_production",   &ftd::TermToggles::pair_production},
        {"exchange_force",    &ftd::TermToggles::exchange_force},
        {"latency_field",     &ftd::TermToggles::latency_field},
        {"emergent_forces",   &ftd::TermToggles::emergent_forces},
    };
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
static void setup_scenario(ftd::RenderBridge& rb, const std::string& name) {
    // Primary path: ported JS scenario library (83 scenarios, flux-/light-/
    // quantum-/s0-seed-/s0-field-). Dispatcher returns true on prefix match.
    if (ftd::dispatch_scenario(rb, name)) return;

    const int N = rb.lattice().size();
    const int mid = static_cast<int>(std::round((N - 1) * 0.5));

    // ── Legacy backward-compat scenarios (not in JS UI registry;
    // preserved for older tests and saved dashboards) ────────────────

    if (name == "empty") {
        // Nothing to inject
    } else if (name == "pair") {
        rb.inject_wavepacket(mid, mid, mid, 1);
        rb.inject_particle(mid + 6, mid, mid, -1, ftd::Vec3(0, 0, 0));
    } else if (name == "production") {
        for (int i = 0; i < 5; i++) {
            rb.inject_particle(4 + i, mid, mid, 1, ftd::Vec3(0, 0, 0));
            rb.inject_particle(N - 5 - i, mid, mid, -1, ftd::Vec3(0, 0, 0));
        }
    } else if (name == "interference") {
        int q = N / 4;
        rb.inject_wavepacket(q, q, mid, 1);
        rb.inject_wavepacket(N - q, q, mid, 1);
        rb.inject_wavepacket(q, N - q, mid, 1);
        rb.inject_wavepacket(N - q, N - q, mid, 1);
    } else if (name == "force") {
        rb.inject_wavepacket(mid, mid, mid, 1);
    } else if (name == "hydrogen") {
        rb.inject_particle(mid, mid, mid, 1, ftd::Vec3(0, 0, 0));
        rb.inject_particle(mid + 8, mid, mid, -1, ftd::Vec3(0, 0, 0));
    } else if (name == "entangled") {
        rb.create_entangled_pair(mid, mid, mid, ftd::Vec3(ftd::K_B, 0, 0));
    } else if (name == "annihilation") {
        rb.inject_particle(mid - 3, mid, mid, 1, ftd::Vec3(0, 0, 0));
        rb.inject_particle(mid + 3, mid, mid, -1, ftd::Vec3(0, 0, 0));
    } else if (name == "triad") {
        rb.inject_wavepacket(mid, mid + 2, mid, 1);
        rb.inject_wavepacket(mid - 2, mid - 1, mid, 1);
        rb.inject_wavepacket(mid + 2, mid - 1, mid, 1);
    } else if (name == "dipole") {
        rb.inject_wavepacket(mid - 2, mid, mid, 1);
        rb.inject_wavepacket(mid + 2, mid, mid, -1);
        rb.voxel_at(mid - 2, mid, mid).locked = true;
        rb.voxel_at(mid + 2, mid, mid).locked = true;
    } else if (name == "scattering") {
        auto& v1 = rb.voxel_at(mid - 8, mid, mid);
        rb.inject_particle(mid - 8, mid, mid, 1, ftd::Vec3(0, 0, 0));
        v1.velocity = ftd::Vec3(0.3, 0.05, 0);
        auto& v2 = rb.voxel_at(mid + 8, mid, mid);
        rb.inject_particle(mid + 8, mid, mid, 1, ftd::Vec3(0, 0, 0));
        v2.velocity = ftd::Vec3(-0.3, -0.05, 0);
    } else if (name == "wave") {
        double amp = ftd::K_B * 0.8;
        rb.inject_flux(mid, mid, mid, ftd::Vec3(amp, 0, 0));
        rb.inject_flux(mid+1, mid, mid, ftd::Vec3(amp*0.6, 0, 0));
        rb.inject_flux(mid-1, mid, mid, ftd::Vec3(amp*0.6, 0, 0));
        rb.inject_flux(mid, mid+1, mid, ftd::Vec3(0, amp*0.6, 0));
        rb.inject_flux(mid, mid-1, mid, ftd::Vec3(0, amp*0.6, 0));
        rb.inject_flux(mid, mid, mid+1, ftd::Vec3(0, 0, amp*0.6));
        rb.inject_flux(mid, mid, mid-1, ftd::Vec3(0, 0, amp*0.6));
    } else if (name == "cluster") {
        int d = 3;
        for (int dx = -1; dx <= 1; dx += 2) {
            for (int dy = -1; dy <= 1; dy += 2) {
                for (int dz = -1; dz <= 1; dz += 2) {
                    int8_t st = ((dx + dy + dz) > 0) ? 1 : -1;
                    rb.inject_wavepacket(mid + dx * d, mid + dy * d, mid + dz * d, st);
                }
            }
        }
    } else if (name == "vacuum") {
        std::mt19937 rng(123);
        std::uniform_real_distribution<double> dist(-0.2, 0.2);
        double seed_amp = ftd::K_B * 0.3;
        for (int x = mid - 4; x <= mid + 4; ++x) {
            for (int y = mid - 4; y <= mid + 4; ++y) {
                for (int z = mid - 4; z <= mid + 4; ++z) {
                    rb.inject_flux(x, y, z, ftd::Vec3(
                        seed_amp * dist(rng),
                        seed_amp * dist(rng),
                        seed_amp * dist(rng)
                    ));
                }
            }
        }
    }
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
