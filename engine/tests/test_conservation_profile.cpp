// ============================================================================
// test_conservation_profile.cpp  (engine-flawless audit, 2026-06-01)
// ----------------------------------------------------------------------------
// PINS the engine's REAL energy-conservation profile so the honest
// characterization cannot silently regress — and so the numbers live in a
// runnable artifact instead of a doc comment.
//
// This test is NOT a quality gate. Each assertion's PURPOSE is
// documentation-of-reality: it encodes a measured fact about how the
// production tick cycle conserves (or fails to conserve) energy, and it
// prints the live measurement to stdout so the file doubles as a standing
// measurement. A future *physics* fix (e.g. a variational projection
// operator) would VISIBLY flip assertion CP-2, which is exactly the signal we
// want — the test would then need updating, and that update would be the
// honest record that the defect was repaired.
//
// ── ENERGY METRIC (read this first) ─────────────────────────────────────────
// The audited energy is compute_energy_audit().total_energy =
//   1/2 Σ|J|² + 1/2 Σ|wave_vel|² + Σ_state≠0 1/2|v|²   (canonical 1/2 factor).
// This naive sum is NOT the exact invariant of the leapfrog scheme (the
// discrete leapfrog conserves a *shadow* Hamiltonian that differs from the
// naive sum by an O(dt²) cross term). So for a freely-evolving wave the naive
// total energy OSCILLATES within a fixed envelope as the field trades between
// the |J|² ("potential") and |wave_vel|² ("kinetic") channels. The signature
// of a WELL-POSED symplectic integrator is therefore NOT "the naive energy is
// flat" — it is "the naive energy stays BOUNDED, with no secular growth." That
// boundedness is exactly what CP-1 certifies, and exactly what the
// non-variational Gauss projection destroys in CP-2.
//
// ── The three facts pinned here (live measurements at L=24) ─────────────────
//
//   CP-1  BARE LEAPFROG is well-posed: its energy is BOUNDED.
//         Wave propagation with a pair of locked opposite charges, gauss
//         projection OFF and damping OFF. The naive total energy oscillates
//         (a sharp blob start sits at an envelope minimum, so it rises ~2.5x
//         transiently) but does NOT grow secularly: the max over the LATE
//         window [100,300] is <= the max over the EARLY window [0,100]
//         (measured late/early ~ 0.79). No energy is injected without bound.
//
//   CP-2  GAUSS PROJECTION is the conservation-limiting operator: it breaks
//         boundedness. Turn ON gauss_projection in the SAME scenario and the
//         energy grows SECULARLY: 100-tick drift ~ 11-18x (> 100%), and the
//         late-window max [100,300] EXCEEDS the early-window max [0,100]
//         (late/early ~ 1.8 and climbing). The projection step
//         J -= grad(phi) is a hard constraint NOT derived from the action; it
//         is non-variational and pumps energy every tick. This is a KNOWN
//         LIMITATION, asserted on purpose. Machine-precision conservation
//         requires a variational/energy-aware projection (a separate physics
//         task), NOT a tighter or longer linear solve (see CP-3).
//
//   CP-3  The central-difference gauss_violation saturates to a structural
//         FLOOR that more SOR iterations do not reduce. gauss_violation =
//         Σ_i (div6(J_i) - state_i)² with the 6-point central-difference
//         divergence (EnergyAudit.gauss_violation via divergence_flux), while
//         gauss_project solves phi with an 18-point Laplacian. The stencil
//         mismatch (engine/include/ftd/eft/matched_poisson.h:7-19) means the
//         solved phi cannot zero the 6-pt divergence: the residual saturates
//         to ~0.342 sum-sq (~5.0e-3 RMS over N=13824), is within ~4e-8 of the
//         floor by 50 iterations, and is BIT-IDENTICAL from 100 through 1000.
//         (6 iters sits ~7.6% ABOVE that floor — close, but not yet pinned.)
//         The key fact: the floor is a fixed point of the wrong stencil, so
//         raising iterations never drives it toward zero.
//
// All three are CPU-only (force_cpu), deterministic (seed_rng fixed; no path
// here depends on the RNG once genesis is off, but we seed for reproducibility),
// and fast (<3s at L=24).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/diagnostics_compute.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Shared scenario builder for CP-1 / CP-2.
//
// "Wave + locked opposite charges": a flux blob seeds genuine wave dynamics
// (so the leapfrog integrator is actually exercised), and two opposite,
// LOCKED charges sit a few voxels apart. The charges are `locked` so
// phase_write's evaporation cannot remove them; movement is OFF so they never
// relocate. The ONLY difference between CP-1 and CP-2 is one toggle
// (gauss_projection).
//
// Toggle baseline (only the wave path active):
//   wave_propagation ON  -> phase_read produces ΔJ = C²∇²J each tick
//   coupling OFF         -> NO g_c*grad(s) source. With static locked charges
//                           the coupling term would be a constant forcing on
//                           the leapfrog (a driven oscillator) and would itself
//                           inject energy without bound — masking the question
//                           we are asking ("does the bare integrator conserve?").
//                           OFF isolates the pure leapfrog. (The locked charges
//                           still matter to CP-2 via the Gauss constraint.)
//   damping OFF          -> no dissipation channel, so any drift is integrator
//                           or projection, never damping.
//   genesis/movement/forces/gravity/lorentz/dual_substrate/weak_transmutation
//   /selective_damping all OFF.
//
// dual_substrate OFF selects the single-substrate leapfrog
// (phase_write.cpp:229-237): wave_vel += ΔJ; flux += wave_vel — the bare
// integrator whose boundedness CP-1 certifies.
// ---------------------------------------------------------------------------
static void configure_conservation_baseline(RenderBridge& rb, bool gauss_on) {
    rb.toggles.disable_all();            // all bulk toggles -> false (valid config)
    rb.toggles.strict_validation = true; // fail loudly if a config slips invalid
    rb.toggles.wave_propagation = true;  // exercise the leapfrog wave update
    rb.toggles.coupling         = false; // NO constant source term (see note above)
    rb.toggles.gauss_projection = gauss_on;  // THE single variable under test
    // Everything else stays OFF (damping included) so the ONLY operators
    // touching the field are the leapfrog (always) and, when gauss_on, the
    // non-variational projection.
}

// Build the fixed initial condition: a flux blob + two locked opposite charges.
// Identical for CP-1 and CP-2 so the two runs differ only by the toggle.
static void inject_conservation_ic(RenderBridge& rb) {
    const int c = rb.lattice().size() / 2;

    // A compact, asymmetric flux blob to seed wave propagation. (Genesis is
    // OFF so the amplitude need not stay sub-threshold.) The blob starts as
    // pure |J| with wave_vel == 0, i.e. at the MINIMUM of the naive-energy
    // oscillation envelope, which is why CP-1's energy rises transiently
    // before settling into its bounded band.
    rb.voxel_at(c,     c, c).flux = Vec3{0.8, 0.0, 0.0};
    rb.voxel_at(c + 1, c, c).flux = Vec3{0.0, 0.6, 0.0};
    rb.voxel_at(c,     c + 1, c).flux = Vec3{0.0, 0.0, 0.5};

    // Two opposite, LOCKED charges 4 voxels apart along x. Locked => not
    // evaporated; movement OFF => stationary. They give the Gauss constraint
    // (CP-2) a real charge density to chase.
    rb.inject_particle(c - 2, c, c, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(c + 2, c, c, -1, Vec3{0.0, 0.0, 0.0});
    rb.voxels()[rb.lattice().index(c - 2, c, c)].locked = true;
    rb.voxels()[rb.lattice().index(c + 2, c, c)].locked = true;
}

// Run the scenario and report: E0, endpoint E_end, and the max naive energy
// over an EARLY window [1, split] and a LATE window [split+1, total]. The
// early/late max ratio is the boundedness discriminator: <=1 means bounded
// (no secular growth), >1 means the energy is still climbing (runaway).
struct ConsProfile {
    double e0 = 0.0;
    double e_end = 0.0;
    double early_max = 0.0;
    double late_max = 0.0;
};
static ConsProfile measure_conservation(bool gauss_on, int split, int total) {
    const int L = 24;
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1234);                   // fixed seed -> reproducible
    configure_conservation_baseline(rb, gauss_on);
    inject_conservation_ic(rb);

    ConsProfile p;
    p.e0 = compute_energy_audit(rb).total_energy;
    for (int t = 1; t <= split; ++t) {
        rb.tick();
        p.early_max = std::max(p.early_max, compute_energy_audit(rb).total_energy);
    }
    for (int t = split + 1; t <= total; ++t) {
        rb.tick();
        const double e = compute_energy_audit(rb).total_energy;
        p.late_max = std::max(p.late_max, e);
        p.e_end = e;
    }
    return p;
}

// ===========================================================================
// CP-1: BARE LEAPFROG well-posedness — energy is BOUNDED (gauss OFF).
//
// The leapfrog integrator alone (no projection, no damping) does NOT inject
// energy without bound: the naive total energy oscillates within a fixed
// envelope, so the late-window [100,300] maximum is <= the early-window
// [0,100] maximum (measured late/early ~ 0.79). We assert late_max <=
// 1.2*early_max (a small slack above the measured 0.79 to absorb run-to-run
// envelope sampling). A FAILURE here means the core integrator started
// injecting energy secularly in isolation — a genuine regression. (We do NOT
// assert "drift < 5%": the naive energy is not the leapfrog invariant and
// legitimately swings within its envelope; the well-posedness claim is about
// boundedness, not flatness.)
// ===========================================================================
void test_bare_leapfrog_well_posed() {
    section("CP-1: bare leapfrog energy is BOUNDED (gauss OFF; no secular growth)");

    const ConsProfile p = measure_conservation(/*gauss_on=*/false,
                                               /*split=*/100, /*total=*/300);
    const double late_over_early = p.late_max / std::max(p.early_max, 1e-300);

    std::printf("    [CP-1] gauss OFF: E0=%.6f  max[0,100]=%.6f (%.2fx)  "
                "max[100,300]=%.6f (%.2fx)  late/early=%.3f\n",
                p.e0, p.early_max, p.early_max / p.e0,
                p.late_max, p.late_max / p.e0, late_over_early);

    // Non-vacuity: there must actually be energy in the system.
    check("CP-1: initial energy is non-trivial (test is non-vacuous)",
          p.e0 > 1e-6,
          "Injected field carried ~0 energy; the conservation premise is vacuous.");

    // The load-bearing fact: the bare integrator is well-posed — its energy is
    // BOUNDED (late-window max does not exceed the early-window max).
    check("CP-1: bare-leapfrog energy is bounded (late_max <= 1.2*early_max; measured ~0.79)",
          late_over_early <= 1.2,
          "The bare leapfrog (gauss OFF, damping OFF) is injecting energy "
          "SECULARLY: the late-window [100,300] maximum exceeds the early-window "
          "[0,100] maximum. With no projection and no damping the naive energy "
          "must stay within a bounded oscillation envelope. This is a "
          "core-integrator regression.");
}

// ===========================================================================
// CP-2: GAUSS PROJECTION is the conservation-limiting operator (gauss ON).
//
// DOCUMENTS A KNOWN LIMITATION. The SAME scenario as CP-1, with the single
// change gauss_projection ON, breaks boundedness: the energy grows SECULARLY
// (100-tick endpoint drift ~ 11-18x; late-window [100,300] max ~ 1.8x the
// early-window [0,100] max, and climbing). The projection J -= grad(phi)
// enforces div(J) = state as a HARD constraint NOT derived from a term in the
// action — it is therefore non-variational and injects energy every tick.
//
// We ASSERT the defect exists (endpoint drift > 100% AND late_max >
// early_max). This is deliberate:
//   - It pins the current honest characterization (the engine does NOT
//     conserve under live projected dynamics).
//   - A future variational / energy-aware projection that actually conserves
//     would make these assertions FAIL, surfacing the fix loudly and forcing
//     this test to be updated — the honest record that the limitation was
//     repaired.
//
// Machine-precision energy conservation requires that variational projection
// (a separate physics task). It is NOT obtainable by raising the SOR solver
// iteration count — see CP-3, which shows the solver is already on its
// structural floor.
// ===========================================================================
void test_gauss_projection_is_limiting() {
    section("CP-2: gauss projection breaks boundedness (KNOWN LIMITATION, ~12x runaway)");

    const ConsProfile pg = measure_conservation(/*gauss_on=*/true,
                                               /*split=*/100, /*total=*/300);
    const double endpoint_drift = std::abs(pg.e_end - pg.e0) / std::max(pg.e0, 1e-300);
    const double late_over_early = pg.late_max / std::max(pg.early_max, 1e-300);

    std::printf("    [CP-2] gauss ON: E0=%.6f  E300=%.6f  endpoint_drift=%.4f rel  "
                "max[0,100]=%.4f (%.2fx)  max[100,300]=%.4f (%.2fx)  late/early=%.3f\n",
                pg.e0, pg.e_end, endpoint_drift,
                pg.early_max, pg.early_max / pg.e0,
                pg.late_max, pg.late_max / pg.e0, late_over_early);

    check("CP-2: initial energy is non-trivial (test is non-vacuous)",
          pg.e0 > 1e-6,
          "Injected field carried ~0 energy; the runaway premise is vacuous.");

    // The load-bearing fact (documenting a KNOWN LIMITATION, NOT a quality
    // pass): the non-variational J-=grad(phi) projection is not
    // energy-conserving — total energy runs away by well over 100%.
    check("CP-2: gauss-projected energy runs away (>100% endpoint drift)",
          endpoint_drift > 1.0,
          "The gauss-projected run no longer shows the expected >100% energy "
          "runaway. If drift is now SMALL, a variational/energy-aware projection "
          "has replaced the non-variational J-=grad(phi) step — update CP-2 to "
          "pin the new conservation profile (this is the success signal, not a "
          "bug). Machine-precision conservation requires that variational fix, "
          "NOT a tighter/longer SOR solve.");

    // And it is SECULAR (still climbing), not a one-shot transient: the
    // late-window maximum exceeds the early-window maximum. This is the exact
    // boundedness property CP-1 has and the projection destroys.
    check("CP-2: runaway is secular (late_max > early_max; bare leapfrog had <=)",
          late_over_early > 1.05,
          "The gauss-projected energy is not still growing in the late window; "
          "the secular-injection characterization of the projection operator no "
          "longer holds.");
}

// ===========================================================================
// CP-3: gauss_violation saturates to a structural FLOOR (stencil mismatch).
//
// See the file header and matched_poisson.h:7-19 for the mechanism. Method:
// build independent, freshly-constructed bridges with an IDENTICAL fixed
// non-Gauss-satisfying flux config (each fresh bridge has phi_
// zero-initialized, so the warm-start state is identical). Set SOR iterations
// per run. Run exactly ONE tick with ONLY gauss_projection ON — so the sole
// flux-mutating operator is gauss_project (phase_read is skipped because
// wave/coupling are OFF, and phase_write is a no-op on flux because delta_j_
// and wave_vel are both zero on a fresh bridge with genesis/damping OFF). Then
// read gauss_violation from compute_energy_audit.
//
// Measured curve (L=24 dipole, single tick):
//   iters     gauss_violation
//      6       3.681915347673e-01     (~7.6% above the floor)
//     30       3.421126673557e-01     (saturating)
//     50       3.421153841364e-01     (within ~4e-8 of the floor)
//    100       3.421153709442e-01  ┐  BIT-IDENTICAL (diff ~5e-15 = machine eps)
//   1000       3.421153709442e-01  ┘  -> the floor
// So the residual SATURATES: it is within ~4e-8 of the floor by 50 iters and
// BIT-IDENTICAL from 100 through 1000. It never trends toward zero — it is a
// fixed point of the wrong stencil. 6 iters sits ~7.6% above the floor (close,
// but not pinned). The assertions below pin (a) the floor is bit-identical
// across 100 vs 1000, (b) it has saturated to ~1e-7 by 50, and (c) it does NOT
// collapse toward zero with more iterations.
// ===========================================================================

// Fixed, deliberately non-Gauss-satisfying flux config: a flux dipole with NO
// matching charge. div(J) is non-zero where state is 0, so the Gauss residual
// is large pre-projection and the floor is well-exercised.
static void inject_floor_ic(RenderBridge& rb) {
    const int c = rb.lattice().size() / 2;
    rb.voxel_at(c,     c, c).flux = Vec3{1.0,  0.0, 0.0};
    rb.voxel_at(c + 1, c, c).flux = Vec3{-1.0, 0.0, 0.0};
    rb.voxel_at(c,     c + 1, c).flux = Vec3{0.0, 0.7, 0.0};
    rb.voxel_at(c,     c, c + 1).flux = Vec3{0.0, 0.0, 0.4};
}

// Run a single projected tick at `sor_iters` and return gauss_violation.
static double gauss_violation_at_iters(int sor_iters) {
    const int L = 24;
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1234);
    rb.toggles.disable_all();            // only gauss_projection below
    rb.toggles.strict_validation = true;
    rb.toggles.gauss_projection = true;  // the ONLY active phase
    // wave/coupling OFF  -> phase_read skipped -> delta_j_ stays zero
    // genesis/damping OFF -> phase_write is a no-op on flux (wave_vel==0,
    //                        delta_j_==0 on this fresh bridge), so the ONLY
    //                        operator touching flux this tick is gauss_project.
    rb.set_sor_iterations(sor_iters);
    inject_floor_ic(rb);

    rb.tick();                           // single projection sweep at sor_iters
    return compute_energy_audit(rb).gauss_violation;
}

void test_gauss_violation_floor_is_structural() {
    section("CP-3: gauss_violation saturates to a structural floor (stencil mismatch)");

    const double gv6    = gauss_violation_at_iters(6);
    const double gv30   = gauss_violation_at_iters(30);
    const double gv50   = gauss_violation_at_iters(50);
    const double gv100  = gauss_violation_at_iters(100);
    const double gv1000 = gauss_violation_at_iters(1000);

    // sum-of-squares vs RMS: gauss_violation is the SUMMED squared residual
    // over all N voxels; the RMS the docs quote is sqrt(gauss_violation / N).
    const int N = 24 * 24 * 24;
    const double rms_floor = std::sqrt(gv100 / N);

    std::printf("    [CP-3] gauss_violation:  6=%.12e  30=%.12e  50=%.12e  "
                "100=%.12e  1000=%.12e\n", gv6, gv30, gv50, gv100, gv1000);
    std::printf("    [CP-3] -> floor RMS=%.6e (sum-sq over N=%d voxels); "
                "100 vs 1000 bit-identical (diff=%.2e); 6 iters is %.2f%% above the floor.\n",
                rms_floor, N, std::abs(gv100 - gv1000), 100.0 * (gv6 - gv100) / gv100);

    // Non-vacuity: the config genuinely violates Gauss (floor is non-zero).
    check("CP-3: residual is non-trivial (floor is real, test non-vacuous)",
          gv100 > 1e-9,
          "gauss_violation ~ 0 — the chosen config already satisfies the 6-pt "
          "Gauss law, so there is no floor to exercise.");

    // The load-bearing fact: in the fully-saturated regime the floor is exactly
    // iteration-independent — 100 and 1000 iterations are BIT-IDENTICAL (the
    // residual difference is at machine epsilon). More SOR iterations buy
    // NOTHING. This is the structural floor (the 18-pt-Laplacian /
    // 6-pt-divergence stencil mismatch documented in matched_poisson.h:7-19),
    // not under-convergence.
    check_close("CP-3: gauss_violation(100) == gauss_violation(1000) (BIT-IDENTICAL floor)",
                gv100, gv1000, 1e-12);

    // The floor has already SATURATED by 50 iters: 50 agrees with the
    // bit-identical 100/1000 value to ~1e-7. (Not bit-identical at 50 — the SOR
    // sweep is still settling the last few ulps — but converged for all
    // practical purposes; beyond ~100 nothing changes at all.)
    check_close("CP-3: gauss_violation(50) ~= floor to ~1e-6 (already saturated)",
                gv50, gv100, 1e-6);

    // The floor is a HARD wall well above zero. Going from 6 -> 1000 iters moves
    // gauss_violation by only ~7.6% and then it PINS — it never trends toward
    // zero. (If the residual instead kept shrinking with iterations, the floor
    // would be a convergence artifact, contradicting the stencil-mismatch
    // analysis.) We bound the total 6->floor improvement well under 10%.
    const double improvement_6_to_floor = (gv6 - gv100) / gv100;
    check("CP-3: 6 iters is already within ~10% of the saturated floor (measured ~7.6%)",
          improvement_6_to_floor >= 0.0 && improvement_6_to_floor < 0.10,
          "The gauss_violation at 6 iters differs from the saturated floor by "
          "more than 10%; the residual is still converging substantially, "
          "weakening the structural-floor characterization. Re-examine the "
          "solver/measure stencils.");
    check("CP-3: the floor does NOT trend to zero (it is the wrong-stencil fixed point)",
          gv1000 > 0.5 * gv100,
          "gauss_violation at 1000 iters collapsed far below the 100-iter value; "
          "the floor is then not a fixed point and more iterations DO help — "
          "contradicting the documented stencil mismatch.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_conservation_profile");

    ftd::test::test_bare_leapfrog_well_posed();            // CP-1
    ftd::test::test_gauss_projection_is_limiting();         // CP-2
    ftd::test::test_gauss_violation_floor_is_structural();  // CP-3

    return ftd::test::finalize();
}
