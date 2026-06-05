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
//    3. dispatch_scenario() — resets the RNG, then walks the 5 group
//       functions in prefix order (first match wins).
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"

#include <cstdint>
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

// ==========================================================================
//  Dispatcher — matches JS runSetupScenario contract.
// ==========================================================================
bool dispatch_scenario(RenderBridge& rb, const std::string& name) {
    // Reset the stochastic RNG so each setupScenario call produces a
    // reproducible sequence. Without this, the thread_local distribution
    // state from a previous scenario (e.g. flux-random-genesis) would leak
    // into the next stochastic scenario called in the same process.
    detail::reset_scenario_rng();

    // Try each group in order; first matching prefix wins.
    if (setup_flux_scenario(rb, name))     return true;
    if (setup_light_scenario(rb, name))    return true;
    if (setup_quantum_scenario(rb, name))  return true;
    if (setup_vacuum_scenario(rb, name))   return true;
    if (setup_s0_seed_scenario(rb, name))  return true;
    if (setup_s0_field_scenario(rb, name)) return true;
    return false;
}

}  // namespace ftd
