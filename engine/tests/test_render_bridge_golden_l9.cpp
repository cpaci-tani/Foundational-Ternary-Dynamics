// ============================================================================
// test_render_bridge_golden_l9.cpp — secondary lattice-size golden (L=9)
// (revision 0.7 CPU half; ADR-0012 amendment / multi-profile policy).
//
// The L=17-only gates are blind to L-dependent indexing/boundary bugs —
// exactly the class of regression that boundary/stencil refactors risk.
// L=9 is the smallest odd lattice with a true center voxel (9-1)/2 = 4
// that still fits the standard 3-particle + pulse harness geometry
// (rescaled), and runs in well under a second.
//
// Shipping-default toggles (no writes), extended fold — same policy as
// test_render_bridge_golden_default.cpp.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdint>
#include <cstdio>

namespace ftd { namespace test {

// Standard harness geometry rescaled to L=9: particles at the analogous
// interior positions, pulse at the true center (4,4,4).
static void inject_initial_state(RenderBridge& rb) {
    rb.inject_particle(2, 2, 2, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(6, 6, 6, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(4, 2, 6, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(4, 4, 4, Vec3{1.0, 0.0, 0.0});
}

// ---------------------------------------------------------------------------
// FROZEN L=9 GOLDEN HASH.
//   - 2026-07-02: initial capture (revision 0.7). Verified stable across 3
//     consecutive runs + OMP_NUM_THREADS=1. Re-baseline policy: ADR-0012.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_HASH_L9 = 0x3480b40d8b801c93ULL;  // re-pinned 2026-07-18, Term-2 coupling sign amendment — see test_render_bridge_golden.cpp changelog; was 0x774ae2ef158a50d6

void test_golden_l9() {
    section("100-tick byte-hash regression (L=9, shipping defaults)");

    RenderBridge rb(9);
    rb.force_cpu();
    rb.seed_rng(42);
    inject_initial_state(rb);
    rb.seed_rng(42);

    for (int t = 0; t < 100; ++t) {
        rb.tick();
    }

    const std::uint64_t hash = compute_state_hash_ext(rb);

    std::printf("[golden-l9] computed hash = 0x%016llx\n",
                static_cast<unsigned long long>(hash));
    std::printf("[golden-l9] expected hash = 0x%016llx\n",
                static_cast<unsigned long long>(GOLDEN_HASH_L9));

    check("hash matches frozen GOLDEN_HASH_L9",
          hash == GOLDEN_HASH_L9,
          "Engine physics at L=9 changed — if the L=17 goldens are green "
          "this is an L-DEPENDENT regression (indexing/boundary). If "
          "intentional, state the rationale and update GOLDEN_HASH_L9.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_render_bridge_golden_l9");
    ftd::test::test_golden_l9();
    return ftd::test::finalize();
}
