// ============================================================================
// test_observable_commutativity.cpp
// ----------------------------------------------------------------------------
// Part C of "certify the physics" of the commutativity no-go
// (PREREG_COMMUTATIVITY_INDEPENDENCE_v1 / PREREG_COMMUTATIVITY_DERIVATION_v1;
//  Lean: lean/FtdNoGo/, lean/Standalone.lean; model-level: FTD-0226).
//
// ENGINE-LEVEL empirical evidence for the two halves of the no-go:
//
//   HALF 1 (substrate reads COMMUTE) — the floor, expected to pass.
//     On a FIXED lattice state, reading observable A then B gives the same
//     pair of values as reading B then A, and reads are idempotent (a read
//     does not mutate state). This is the engine-level face of "observables
//     are functions of the configuration J → they commute" (Lean Claim A/C,
//     FTD-0226 Step 1-2). It is the floor, not the result.
//
//   HALF 2 (the measurement map M does NOT commute) — the real result.
//     The state-mutating operations bundled in tick() — the Gauss projection
//     (∇·J = ρ back-reaction) and the genesis/manifestation threshold — change
//     the value a subsequent read returns. So "read ∘ evolve" ≠ "evolve ∘ read":
//     the operation that fails to commute with reads is the empirical
//     measurement map M, exactly the candidate 6th-postulate locus the no-go
//     isolates. This is non-vacuous: these operations genuinely mutate state.
//
// HONEST SCOPE: this is engine-level EMPIRICAL corroboration (Level 2/3), not
// a proof. It does NOT certify that nature is commutative beneath measurement
// (Level 4, empirically undecidable). It localizes M; it does not derive that
// M must be non-commutative.
//
// Determinism anchor: the substrate update is deterministic (golden-tick hash
// gate, test_render_bridge_golden.cpp, hash 0xcd957b601d47868a). Determinism
// is the precondition for HALF 1 (a fixed state yields read-order-independent,
// repeatable values).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>

using namespace ftd;

// A scalar "read" observable: total |∇·J|² over the lattice (a Gauss-sector
// observable that the projection directly affects). Pure function of state;
// marked by taking `const RenderBridge&`.
static double read_div2(const RenderBridge& rb) {
    double s = 0.0;
    const int n = (int)rb.voxels().size();
    for (int i = 0; i < n; ++i) {
        const double d = rb.divergence_flux(i);
        s += d * d;
    }
    return s;
}

// A second independent scalar "read": total flux density Σ|J|.
static double read_density(const RenderBridge& rb) {
    double s = 0.0;
    for (const auto& v : rb.voxels()) s += v.density();
    return s;
}

// Seed a fixed, reproducible non-trivial flux configuration.
static void seed_state(RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(42);
    const int L = rb.lattice().size();
    // Two opposed flux pulses (a configuration with nonzero divergence that
    // the Gauss projection will act on), plus an off-axis component.
    rb.inject_flux(L / 2 - 2, L / 2, L / 2, Vec3{ 0.8, 0.2, 0.0});
    rb.inject_flux(L / 2 + 2, L / 2, L / 2, Vec3{-0.8, 0.0, 0.3});
    rb.inject_flux(L / 2, L / 2 + 2, L / 2, Vec3{ 0.0, 0.5, 0.1});
}

// ---------------------------------------------------------------------------
// HALF 1 — substrate reads commute (and are idempotent) on a fixed state.
// ---------------------------------------------------------------------------
static void test_reads_commute() {
    test::section("HALF 1: substrate reads commute on a fixed state (the floor)");

    RenderBridge rb(16);
    seed_state(rb);

    // Order A→B
    const double a1 = read_div2(rb);
    const double b1 = read_density(rb);
    // Order B→A (state untouched by reads)
    const double b2 = read_density(rb);
    const double a2 = read_div2(rb);

    // Reads are order-independent: [read_A, read_B] = 0 at the substrate.
    test::check_close("read order A->B vs B->A: div2 equal", a1, a2, 1e-12);
    test::check_close("read order A->B vs B->A: density equal", b1, b2, 1e-12);

    // Reads are idempotent (a read does not mutate state): repeat A.
    const double a3 = read_div2(rb);
    test::check_close("read is idempotent (no state mutation)", a1, a3, 1e-12);
}

// ---------------------------------------------------------------------------
// HALF 2 — the measurement map M (tick = gauss_project + genesis + ...) does
// NOT commute with reads: read∘evolve ≠ evolve∘read. This LOCALIZES M.
// ---------------------------------------------------------------------------
static void test_measurement_map_noncommuting() {
    test::section("HALF 2: the measurement map M (gauss/genesis via tick) does NOT commute with reads");

    // Branch 1: read BEFORE the map.
    RenderBridge rb1(16);
    seed_state(rb1);
    const double read_before = read_div2(rb1);   // read, then evolve
    rb1.tick();                                   // M: gauss_project + genesis + ...
    const double after_then_value = read_div2(rb1);

    // Branch 2: identical seed, evolve FIRST, then read.
    RenderBridge rb2(16);
    seed_state(rb2);
    rb2.tick();                                   // M first
    const double read_after = read_div2(rb2);

    // Determinism cross-check: the two branches share the same seed+state, so
    // the post-tick read MUST agree (this is the determinism anchor, not the
    // result). If this fails, the update is nondeterministic and HALF 1's
    // premise is void.
    test::check_close("determinism: post-tick read agrees across branches",
                      after_then_value, read_after, 1e-9);

    // THE RESULT: the value read BEFORE the map differs from the value read
    // AFTER the map. The map M changed what the observable returns, so M does
    // not commute with the read. M (the Gauss projection / genesis state
    // mutation) is the locus where ordering-dependence enters — the candidate
    // 6th-postulate measurement map the no-go isolates.
    const double delta = std::abs(read_before - read_after);
    test::metric("read_before_M (div2)", read_before);
    test::metric("read_after_M  (div2)", read_after);
    test::metric("|delta| (M-induced change)", delta);
    test::check("M does not commute with reads: read_before != read_after (|delta| > 1e-6)",
                delta > 1e-6);
}

int main() {
    test::init("observable_commutativity");
    test_reads_commute();
    test_measurement_map_noncommuting();
    return test::finalize();
}
