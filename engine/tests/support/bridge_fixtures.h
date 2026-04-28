// ============================================================================
// tests/support/bridge_fixtures.h
// ----------------------------------------------------------------------------
// Phase 7 (2026-04-27): shared test helpers that wrap the recurring
// `RenderBridge rb(L); rb.toggles.foo = true; ... loop ticks; check;`
// pattern. Adopt incrementally — every existing test continues to compile
// without using these helpers.
//
// These are simple delegators to existing RenderBridge methods. They do NOT
// add new physics, change tick semantics, or modify default toggles in any
// way that changes test outcomes — they only save tests from repeating the
// same construction boilerplate.
//
// USAGE:
//
//   #include "tests/support/bridge_fixtures.h"
//
//   void my_test() {
//       auto rb = ftd::test::make_bridge(/*L=*/16,
//                                        ftd::test::ToggleProfile::Logic6,
//                                        /*seed=*/42);
//       ftd::test::inject_particle_at_center(rb, /*state=*/+1);
//       ftd::test::run_for(rb, /*ticks=*/100);
//       (void)ftd::test::assert_energy_conserved(rb, /*n_ticks=*/50,
//                                                /*eps_rel=*/1e-6);
//   }
//
// IMPLEMENTATION NOTE: bodies live in tests/support/bridge_fixtures.cpp,
// linked via the `ftd_test_support` static library. The fixtures need
// ftd_core (RenderBridge / Vec3), so a test that uses them must link both.
// ============================================================================

#pragma once

#include <memory>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"  // Vec3

namespace ftd {
namespace test {

// Toggle profiles for common scenarios. Match the patterns already used in
// existing tests — these helpers do NOT invent new physics defaults.
enum class ToggleProfile {
    // The 6 canonical physics phases ON (matches RenderBridge default
    // TermToggles for the core Logic-6 set), all extension toggles OFF.
    // Use this for general-purpose RenderBridge tests.
    Logic6,

    // Stencil-only: wave_propagation + coupling, everything else OFF.
    // Use this for pure wave/Laplacian/dispersion experiments.
    LogicOnly,

    // Logic6 + lorentz_force ON. Magnetic-force tests.
    FullEM,

    // Logic6 + lorentz_force + color_forces + strong_force +
    // dual_substrate. Standard-Model-style tests.
    FullSM,

    // Caller mutates rb.toggles after the call. make_bridge() leaves the
    // RenderBridge default (Logic6 + dual_substrate + weak_transmutation)
    // alone — see term_toggles.h for the full default set.
    Custom,
};

// Construct a deterministic RenderBridge with the given lattice size,
// toggle profile, RNG seed, and (optionally) backend.
//
// Defaults:
//   profile   = Logic6
//   seed      = 42 (matches the golden-tick test convention)
//   force_cpu = true (deterministic; no GPU contention in unit tests)
//
// Returns std::unique_ptr<RenderBridge> because RenderBridge has a user-
// declared destructor (which suppresses the implicit move ctor) — so it
// can't be returned by value without explicit move declarations on the
// engine type, and Phase 7 must not modify engine headers. Callers
// dereference normally:
//
//   auto rb = ftd::test::make_bridge(16);
//   rb->tick();
//   ftd::test::run_for(*rb, 100);
std::unique_ptr<RenderBridge>
make_bridge(int L,
            ToggleProfile profile = ToggleProfile::Logic6,
            unsigned seed = 42,
            bool force_cpu = true);

// Run a fixed number of ticks. Equivalent to `for (i=0; i<ticks; ++i) rb.tick()`,
// but documented and easier to grep for.
void run_for(RenderBridge& rb, int ticks);

// Inject a single particle at the lattice center with optional velocity.
// state must be ±1 (0 = void; the call would be a no-op).
// flux is set to {state, 0, 0}; spin/color = 0 (default).
//
// Velocity v is written to the same voxel after injection so movement-phase
// tests can drive the particle. Pass Vec3{0,0,0} for a stationary particle.
void inject_particle_at_center(RenderBridge& rb,
                               int8_t state,
                               Vec3 v = Vec3{0.0, 0.0, 0.0});

// Assert energy is conserved within a relative epsilon over n_ticks. Calls
// `ftd::test::check()` once with name "energy conservation"; returns true
// iff the check passed (so tests can branch on the result).
//
// Convention: energy is the EnergyAudit `total_energy` field. eps_rel is
// the maximum allowed |E(t) - E(0)| / max(|E(0)|, eps_abs) over the run.
// Damping toggle should be OFF for a meaningful conservation check; this
// helper does NOT enforce that — the caller is responsible for the toggle
// configuration that makes the assertion physically valid.
bool assert_energy_conserved(RenderBridge& rb, int n_ticks,
                             double eps_rel = 1e-6);

}  // namespace test
}  // namespace ftd
