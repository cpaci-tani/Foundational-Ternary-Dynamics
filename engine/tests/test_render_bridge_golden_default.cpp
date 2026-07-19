// ============================================================================
// test_render_bridge_golden_default.cpp — DEFAULT-PROFILE golden gate
// (revision 0.5b; ADR-0012 amendment).
//
// The original golden (test_render_bridge_golden.cpp) freezes a MINIMAL
// profile: ~14 subsystems explicitly toggled OFF, including the four
// promoted-to-default-ON extensions (dual_substrate, selective_damping,
// weak_transmutation, damping). That leaves the SHIPPING DEFAULTS — the
// configuration every `RenderBridge rb(L)` user actually gets — unprotected
// by any bit-exact gate.
//
// This test closes that gap:
//   - identical harness geometry (L=17, seed 42, same 3 particles + pulse,
//     100 ticks, CPU-pinned),
//   - ZERO toggle writes: the profile is a pure default-constructed
//     TermToggles{}. If someone changes a toggle default in TOGGLE_SPECS /
//     term_toggles.h, THIS hash moves — making default changes a conscious,
//     stated decision (the toggle-parity web lint fires on the JS side for
//     the whitelisted subset; this gate covers the physics).
//   - EXTENDED fold (compute_state_hash_ext): the default profile exercises
//     dual_substrate, so flux_L/R + wave_vel_L/R + latency are folded in
//     addition to the original field set.
//
// Pinned constant below. Capture protocol: 3 consecutive identical runs
// (plus OMP determinism inherited from the engine's partition-independent
// parallel loops — see parallel.h). To change intentionally: state the
// rationale in the commit and update the constant (ADR-0012 policy).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdint>
#include <cstdio>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Deterministic initial state — identical to the minimal-profile golden.
// ---------------------------------------------------------------------------
static void inject_initial_state(RenderBridge& rb) {
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

// ---------------------------------------------------------------------------
// FROZEN DEFAULT-PROFILE GOLDEN HASH.
//   - 2026-07-02: initial capture (revision 0.5b). Shipping defaults at the
//     time of capture: wave_propagation, coupling, damping, genesis,
//     gauss_projection, forces, gravity, poisson_coulomb, movement,
//     lorentz_force, selective_damping, dual_substrate, weak_transmutation
//     ON; everything else at its TOGGLE_SPECS default (see term_toggles.h).
//     Verified stable across 3 consecutive runs.
// If this changes WITHOUT a stated rationale, either engine physics or a
// TOGGLE DEFAULT changed unexpectedly. To change it intentionally: (1) state
// which default/physics changed and why in the commit, (2) update the
// constant to the new captured value.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_HASH_DEFAULT = 0x54fe2f9ab5c0a255ULL;  // L=17, shipping defaults, ext fold (re-pinned 2026-07-18, Term-2 coupling sign amendment — see test_render_bridge_golden.cpp changelog; was 0x115a6350fcbe39a0)

void test_golden_default_profile() {
    section("100-tick byte-hash regression (shipping-default toggles)");

    RenderBridge rb(17);     // odd lattice — true center voxel at (17-1)/2 = 8
    rb.force_cpu();          // CPU-only — bit-exact reference
    rb.seed_rng(42);
    // NO toggle writes — the profile under test is TermToggles{} defaults.
    inject_initial_state(rb);
    rb.seed_rng(42);         // re-seed after injection (same rationale as minimal golden)

    for (int t = 0; t < 100; ++t) {
        rb.tick();
    }

    const std::uint64_t hash = compute_state_hash_ext(rb);

    std::printf("[golden-default] computed hash = 0x%016llx\n",
                static_cast<unsigned long long>(hash));
    std::printf("[golden-default] expected hash = 0x%016llx\n",
                static_cast<unsigned long long>(GOLDEN_HASH_DEFAULT));

    check("hash matches frozen GOLDEN_HASH_DEFAULT",
          hash == GOLDEN_HASH_DEFAULT,
          "Engine physics under SHIPPING-DEFAULT toggles has changed (or a "
          "toggle default itself changed). If intentional, state the rationale "
          "and update GOLDEN_HASH_DEFAULT.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_render_bridge_golden_default");
    ftd::test::test_golden_default_profile();
    return ftd::test::finalize();
}
