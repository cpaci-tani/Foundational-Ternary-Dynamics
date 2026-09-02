// ==========================================================================
//  engine/src/scenarios.cpp
//
//  Thin router + shared RNG for the Scale-0 scenario library. Scenario
//  bodies moved out to engine/src/scenarios/{flux,light,quantum,s0_seed,
//  s0_field}.cpp as part of ticket S1. See engine/include/ftd/scenarios.h
//  for the public contract and engine/src/scenarios/_helpers.h for the
//  shared inline primitives.
//
//  What's left in this file:
//    1. The thread_local mt19937 state + SCN_RNG_SEED constant.
//    2. ftd::detail::urand() / ftd::detail::reset_scenario_rng() — the
//       external-linkage bridge that lets the 5 stochastic scenarios in
//       flux/quantum reach the shared RNG across TU boundaries.
//    3. dispatch_scenario() — resets the RNG, then walks the 7 group
//       functions in prefix order (first match wins).
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "scenarios/_helpers.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>

namespace ftd {

namespace {

// Used by scenarios that call Math.random() in JS (flux-random-genesis,
// flux-thermalization, flux-vacuum-foam, flux-zero-point, quantum-born-rule,
// quantum-casimir).
// Thread-local so each worker thread gets its own state; reset_scenario_rng()
// is invoked at the top of dispatch_scenario() below so repeated
// setupScenario calls produce a reproducible sequence within a single
// process run.
//
// NOTE: JS Math.random() is not seedable, so JS↔C++ parity for the 5
// stochastic scenarios is statistical (same distribution), not bit-exact.
// The fixed seed ensures repeatability within one WASM process run —
// important for snapshot tests.
constexpr std::uint_fast32_t SCN_RNG_SEED = 0xC0DEFACE;
thread_local std::mt19937 g_rng{SCN_RNG_SEED};
thread_local std::uniform_real_distribution<double> g_uniform01{0.0, 1.0};

}  // namespace

namespace detail {

// External-linkage bridges declared in engine/src/scenarios/_helpers.h.
// Defined here so every stochastic scenario (wherever split) shares the
// same RNG state without exposing g_rng / g_uniform01 publicly.
double urand() { return g_uniform01(g_rng); }

void reset_scenario_rng() {
    g_rng.seed(SCN_RNG_SEED);
    g_uniform01.reset();
}

}  // namespace detail

const std::vector<std::string_view>& scale0_scenario_ids() {
    // Mirrors SCALE0_SCENARIO_CATALOG in the web registry.  Keeping the list
    // here makes native dispatch fail closed and gives CTest a finite surface
    // over which to validate every final physics profile.
    static const std::vector<std::string_view> ids = {
        "empty",
        "s0-seed-dynamical-flux-dressing",
        "s0-seed-moving-source-reciprocity",
        "flux-pulse",
        "flux-dipole",
        "flux-standing",
        "flux-nested-standing",
        "flux-soliton",
        "flux-interference",
        "flux-vortex",
        "flux-dual-substrate",
        "flux-cascade",
        "flux-random-genesis",
        "flux-genesis-between-gates",
        "s0-seed-ew-phase-transition",
        "flux-pair-production",
        "flux-annihilation",
        "flux-vacuum-foam",
        "flux-meson",
        "flux-string-breaking",
        "flux-baryon",
        "flux-cyclotron",
        "flux-screening",
        "flux-thermalization",
        "flux-triad",
        "flux-zero-point",
        "light-rainbow",
        "light-dipole",
        "light-two-slit",
        "light-photon-race",
        "quantum-born-rule",
        "quantum-double-slit",
        "quantum-eraser",
        "quantum-tunnel",
        "quantum-well",
        "quantum-entangle",
        "quantum-aharonov-bohm",
        "quantum-casimir",
        "quantum-zeno",
        "s0-seed-up-quark",
        "s0-seed-down-quark",
        "s0-seed-strange-quark",
        "s0-seed-charm-quark",
        "s0-seed-bottom-quark",
        "s0-seed-top-quark",
        "s0-seed-anti-up-quark",
        "s0-seed-anti-down-quark",
        "s0-seed-anti-strange-quark",
        "s0-seed-anti-charm-quark",
        "s0-seed-anti-bottom-quark",
        "s0-seed-anti-top-quark",
        "s0-seed-higgs-field",
        "s0-seed-gluon",
        "s0-seed-beta-decay",
        "s0-seed-ee-annihilation",
        "s0-seed-quark-gluon-plasma",
        "s0-seed-hydrogen",
        "s0-seed-helium",
        "s0-seed-h2-bond-formation",
        "s0-seed-spark-of-life",
        "s0-seed-wilson-loop",
        "s0-seed-flux-tube",
        "s0-seed-monopole",
        "s0-seed-instanton",
        "s0-seed-schwarzschild",
        "s0-seed-gravitational-lensing",
        "s0-seed-gravitational-wave",
        "s0-seed-massive-body",
        "s0-seed-time-gravity-well",
        "s0-seed-time-twin-clocks",
        "s0-seed-time-horizon",
        "s0-seed-sloop",
        "s0-seed-observer-cell",
        "s0-field-plane-wave",
        "s0-field-standing-wave",
        "s0-field-uniform-e",
        "s0-field-uniform-b",
        "s0-field-photon-pulse",
        "s0-field-rf-lattice-wave",
        "s0-field-light-lattice-wave",
        "s0-field-sound-lattice-wave",
        "s0-field-sound-collision",
        "s0-field-thomson-scattering",
        "s0-field-thomson-unlocked-recoil",
        "s0-field-spacetime-forcing-boundary",
        "s0-field-electric-dipole",
        "s0-field-magnetic-dipole",
        "s0-field-vortex-line",
        "s0-seed-octahedron",
        "s0-seed-cuboctahedron",
        "s0-seed-stella-octangula",
        "s0-seed-moore-cell",
        "s0-seed-moore-decomposition",
        "s0-seed-emergent-ic1",
        "s0-seed-emergent-ic3-collision",
        "s0-seed-emergent-ic4-subthreshold",
        "s0-seed-emergent-ic2-thermal-runaway",
        "s0-seed-emergent-ic1-diagonal",
        "s0-seed-emergent-ic1-isotropic",
        "s0-seed-emergent-ic1-viz",
        "s0-seed-emergent-ic1-diagonal-viz",
        "s0-seed-emergent-ic1-isotropic-viz",
        "s0-seed-cluster-law",
        "s0-seed-cluster-law-subknee",
        "s0-seed-cluster-law-knee",
        "s0-seed-cluster-law-superknee",
        "s0-vacuum-electron",
        "s0-vacuum-muon",
        "s0-vacuum-tau",
        "s0-vacuum-positron",
        "s0-vacuum-antimuon",
        "s0-vacuum-antitau",
        "s0-vacuum-electron-neutrino",
        "s0-vacuum-muon-neutrino",
        "s0-vacuum-tau-neutrino",
        "s0-vacuum-electron-antineutrino",
        "s0-vacuum-muon-antineutrino",
        "s0-vacuum-tau-antineutrino",
        "s0-vacuum-photon",
        "s0-vacuum-w-boson",
        "s0-vacuum-w-minus-boson",
        "s0-vacuum-z-boson",
        "s0-vacuum-higgs",
        "s0-vacuum-proton",
        "s0-vacuum-neutron",
        "s0-vacuum-pion-charged",
        "s0-vacuum-pion-neutral",
        "s0-vacuum-kaon-charged",
        "s0-seed-de-broglie-clock",
        "s0-seed-thermal-ignition",
        // s0-cell-* flux cells (src/scenarios/cell.cpp, 2026-09-02)
        "s0-cell-capacitor",
        "s0-cell-torus",
        "s0-cell-torus-reverse",
        "s0-cell-torus-scrambled",
        "s0-cell-torus-open",
        "s0-cell-torus-walled",
        "s0-cell-triad",
        "s0-cell-torus-membrane",
        "s0-cell-torus-membrane-gated",
        "s0-cell-membrane-pumped",
        "s0-cell-membrane-transfer",
        "s0-cell-membrane-pumped-resonant",
    };
    return ids;
}

// ==========================================================================
//  Dispatcher — matches JS runSetupScenario contract.
// ==========================================================================
bool dispatch_scenario(RenderBridge& rb, const std::string& name) {
    // Reset the stochastic RNG so each setupScenario call produces a
    // reproducible sequence. Without this, the thread_local distribution
    // state from a previous scenario (e.g. flux-random-genesis) would leak
    // into the next stochastic scenario called in the same process.
    detail::reset_scenario_rng();

    // Flux-cell mechanisms are per-bridge state, not toggles: a body that
    // wants them re-registers them; nothing leaks between scenarios.
    rb.clear_flux_cell_region();
    rb.clear_flux_pump();
    rb.clear_flux_cell_port();

    const auto& registered = scale0_scenario_ids();
    if (std::find(registered.begin(), registered.end(), std::string_view(name))
        == registered.end())
        return false;

    const auto accept_profile = [&](bool handled) {
        if (!handled) return false;
        std::string validation_error;
        if (!rb.toggles.validate(&validation_error)) {
            std::cerr << "[Scenario] '" << name
                      << "' produced an invalid physics profile: "
                      << validation_error << '\n';
            return false;
        }
        return true;
    };

    if (name == "empty") {
        // Scenario ID: empty
        // Physical Purpose: Serves as the baseline state of the lattice with no initial particles or fields.
        // Initial Condition Parameters: None.
        // Expected Behaviour: The lattice remains completely quiet and empty.
        // Isolate every production phase so the null control is not the
        // full dashboard default stack running on a zero field.
        configure_static_seed_terms(rb);
        return accept_profile(true);
    }

    // Try each group in order; first matching prefix wins.
    if (setup_flux_scenario(rb, name))     return accept_profile(true);
    if (setup_light_scenario(rb, name))    return accept_profile(true);
    if (setup_quantum_scenario(rb, name))  return accept_profile(true);
    if (setup_vacuum_scenario(rb, name))   return accept_profile(true);
    if (setup_s0_seed_scenario(rb, name))  return accept_profile(true);
    if (setup_s0_field_scenario(rb, name)) return accept_profile(true);
    if (setup_cell_scenario(rb, name))     return accept_profile(true);
    return false;
}

}  // namespace ftd
