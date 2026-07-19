// ============================================================================
// test_boundary_modes_golden.cpp — boundary-mode characterization goldens
// (revision 0.6; ADR-0012 amendment / multi-profile policy).
//
// The default golden profiles run with the PERIODIC (toroidal wrap) boundary,
// so every non-default boundary path is bit-exact-unprotected:
//   - FluxBoundaryMode::Reflective  -> apply_reflective_flux_boundary
//   - FluxBoundaryMode::Dispersal   -> apply_dispersal_flux_boundary
//   - absorbing_boundary (sponge)   -> apply_absorbing_boundary
//   - reflective_boundary (movement mirror-bounce at lattice faces)
// (engine/src/render_bridge_phases/phase_write.cpp:389-464; tick-cycle gating
// in render_bridge.cpp Rule 5b/5c.)
//
// This test pins one hash per mode over the standard golden harness
// (L=17, seed 42, 3 particles + center pulse, 100 ticks, CPU, shipping-
// default toggles + the boundary setting). These are CHARACTERIZATION
// hashes: they freeze today's behavior so the planned boundary-shell
// dedup (revision ticket 2.3) and any later boundary work is provably
// bit-exact. Re-baseline policy is per-profile (ADR-0012 amendment).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdint>
#include <cstdio>

namespace ftd { namespace test {

static void inject_initial_state(RenderBridge& rb) {
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

// Runs the standard harness with a mode-configurator applied after
// construction, returns the extended state hash. extra_inject (nullable)
// adds mode-specific initial state AFTER the standard injection.
static std::uint64_t run_profile(void (*configure)(RenderBridge&),
                                 void (*extra_inject)(RenderBridge&)) {
    RenderBridge rb(17);
    rb.force_cpu();
    rb.seed_rng(42);
    configure(rb);
    inject_initial_state(rb);
    if (extra_inject) extra_inject(rb);
    rb.seed_rng(42);
    for (int t = 0; t < 100; ++t) rb.tick();
    return compute_state_hash_ext(rb);
}

// The movement mirror-bounce only fires when a particle actually reaches a
// lattice face: the standard 3 particles start well inside and never get
// there in 100 ticks (verified: without this crosser the reflective_boundary
// hash equals the default-profile golden, i.e. the toggle was inert on the
// harness). Inject a boundary-bound particle so the bounce path is genuinely
// exercised. NOTE inject_particle's Vec3 parameter is FLUX, not velocity —
// the crosser's motion comes from writing velocity directly (remainder
// accumulation then drives integer jumps toward the x=0 face).
static void inject_boundary_crosser(RenderBridge& rb) {
    // NOTE inject_particle's Vec3 parameter is FLUX, not velocity, and flux
    // at a charge site is rewritten by the Gauss projection each tick (the
    // longitudinal point-flux is projected away — measured: any injected
    // magnitude collapses to |J|~0.099 by end of tick 0), so flux cannot be
    // used to keep the crosser alive. The direct velocity write (-0.5/tick)
    // drives remainder to the x=0 face: jump to x=0 during tick 1, crossing
    // attempt (mirror-bounce vs void-exhaust) during tick 2.
    rb.inject_particle(1, 8, 8, +1, Vec3{0.0, 0.0, 0.0});
    const int idx = rb.lattice().index(1, 8, 8);
    rb.voxels()[idx].velocity = Vec3{-0.5, 0.0, 0.0};
}

// The movement-bounce profile must also disable manifestation kinetics:
// with shipping defaults the stochastic evaporation draw (evap_prob ~
// exp(-E/K_MANIFEST^2) * K_EVAP_RATE, ~9%/tick at Gauss-normalized charge
// self-field energy) removes the crosser at tick 1 with the seeded RNG —
// identically on both boundary settings, silently making the pin inert.
// A movement-boundary profile legitimately pins kinematics, not genesis.
static void configure_reflective_move(RenderBridge& rb) {
    rb.toggles.reflective_boundary = true;
    rb.toggles.genesis     = false;
    rb.toggles.evaporation = false;
}

// ---------------------------------------------------------------------------
// FROZEN BOUNDARY-MODE HASHES (captured 2026-07-02, revision 0.6; each
// verified stable across 3 consecutive runs + OMP_NUM_THREADS=1).
// To change intentionally: state the rationale and update the constant.
//   - 2026-07-18: all four RE-PINNED — Term-2 electric coupling sign amendment
//     (lagrangian.h; see test_render_bridge_golden.cpp changelog for the full
//     rationale). Profiles contain injected particles, so trajectories
//     legitimately change. Old values: 0xbe736c3006d4fed0 / 0x9778edf520396c54
//     / 0x208c18ce2f75082c / 0x285566e618111ead.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_REFLECTIVE_FLUX  = 0x251df4627c88a705ULL;
static constexpr std::uint64_t GOLDEN_DISPERSAL_FLUX   = 0x6b13e15becc31a1aULL;
static constexpr std::uint64_t GOLDEN_ABSORBING        = 0x2bf1ae7920cfc7c5ULL;
static constexpr std::uint64_t GOLDEN_REFLECTIVE_MOVE  = 0x942950ab797ab662ULL;

struct Mode {
    const char* name;
    void (*configure)(RenderBridge&);
    void (*extra_inject)(RenderBridge&);
    std::uint64_t expected;
};

void test_boundary_mode_goldens() {
    const Mode modes[] = {
        {"flux_boundary=Reflective",
         [](RenderBridge& rb) { rb.toggles.flux_boundary = FluxBoundaryMode::Reflective; },
         nullptr,
         GOLDEN_REFLECTIVE_FLUX},
        {"flux_boundary=Dispersal",
         [](RenderBridge& rb) { rb.toggles.flux_boundary = FluxBoundaryMode::Dispersal; },
         nullptr,
         GOLDEN_DISPERSAL_FLUX},
        {"absorbing_boundary=true",
         [](RenderBridge& rb) { rb.toggles.absorbing_boundary = true; },
         nullptr,
         GOLDEN_ABSORBING},
        {"reflective_boundary=true (movement bounce; genesis/evap off)",
         configure_reflective_move,
         inject_boundary_crosser,
         GOLDEN_REFLECTIVE_MOVE},
    };

    for (const auto& m : modes) {
        section(m.name);
        const std::uint64_t hash = run_profile(m.configure, m.extra_inject);
        std::printf("[golden-boundary] %-42s computed = 0x%016llx expected = 0x%016llx\n",
                    m.name,
                    static_cast<unsigned long long>(hash),
                    static_cast<unsigned long long>(m.expected));
        check("hash matches frozen boundary-mode golden",
              hash == m.expected,
              "Boundary-mode physics changed. If intentional, state the "
              "rationale and update the pinned constant (ADR-0012 policy).");
    }

    // Negative control: the movement mirror-bounce must actually FIRE on its
    // harness — the same crosser under default (periodic) boundary must give
    // a DIFFERENT hash than under reflective_boundary=true. Guards against
    // the pin silently degenerating into an inert-toggle characterization.
    section("negative control: bounce path is live");
    const std::uint64_t bounce_on  = run_profile(configure_reflective_move,
                                                 inject_boundary_crosser);
    const std::uint64_t bounce_off = run_profile(
        [](RenderBridge& rb) {
            rb.toggles.genesis     = false;
            rb.toggles.evaporation = false;
        },
        inject_boundary_crosser);
    std::printf("[golden-boundary] crosser bounce-on  = 0x%016llx\n",
                static_cast<unsigned long long>(bounce_on));
    std::printf("[golden-boundary] crosser bounce-off = 0x%016llx\n",
                static_cast<unsigned long long>(bounce_off));
    check("reflective_boundary changes the crosser's trajectory",
          bounce_on != bounce_off,
          "The boundary-crossing particle no longer reaches a face within "
          "100 ticks — the reflective_boundary pin has gone inert. Adjust "
          "inject_boundary_crosser so the bounce path executes.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_boundary_modes_golden");
    ftd::test::test_boundary_mode_goldens();
    return ftd::test::finalize();
}
