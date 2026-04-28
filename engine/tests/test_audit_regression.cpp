// ============================================================================
// test_audit_regression.cpp
// ----------------------------------------------------------------------------
// Focused regression tests for recent audit fixes that previously had no C++
// coverage. Each section below pins down one specific invariant that, if
// broken in the future, would silently corrupt diagnostics or a force path.
//
// Tasks (Wave-3 Agent G):
//   G-6                  ½ factor on field_energy / wave_energy
//   G-1                  locked-particle pair force does NOT skip unlocked partner
//   G-2                  E_L / E_R (flux) vs wv_L / wv_R (wave_vel) split
//   G-4                  18-pt Laplacian sum-rule weights
//   G-Coulomb-PE-pair    coulomb_pe == ½·Σ α·q·φ pair-PE convention
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/diagnostics_compute.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstring>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// G-6: Energy convention ½ factor regression test.
//
// After the 2026-04-27 diagnostics_compute.cpp fix, field_energy /
// wave_energy / E_L_total / E_R_total / wv_L_total / wv_R_total / coulomb_pe
// all carry the canonical ½ factor (see diagnostics_compute.cpp lines 92-99,
// 110-118, 138-148). Pre-fix, MockBridge reported half the WasmBridge value
// for the same scenario, silently 2×-ing the Energy Budget chart whenever
// the user switched bridges.
//
// This test guards against any future "drop the ½" regression by injecting
// a known flux pattern at one voxel and asserting field_energy = ½·|J|² and
// wave_energy = 0 (wave_vel was not set).
// ---------------------------------------------------------------------------
void test_energy_audit_half_factor() {
    section("G-6: Energy audit ½ factor regression");

    RenderBridge rb(8);
    rb.force_cpu();  // Make this a deterministic CPU-only test.

    // Disable everything that might mutate voxel state — we want the audit
    // to see exactly the flux we wrote, with no tick advancement.
    rb.toggles.dual_substrate = false;  // simplifies the audit (skip dual block)

    // Mutate one voxel directly via the public voxels() accessor.
    auto& voxels = rb.voxels();
    const int idx = rb.lattice().index(2, 3, 4);
    voxels[idx].flux = Vec3{1.0, 0.0, 0.0};
    voxels[idx].wave_vel = Vec3{0.0, 0.0, 0.0};

    auto audit = compute_energy_audit(rb);

    // |J|² = 1.0 at one site; with the ½ factor, field_energy = 0.5.
    check_close("field_energy = ½·|J|²",     audit.field_energy, 0.5, 1e-9);
    check_close("wave_energy = 0 (no wv)",   audit.wave_energy,  0.0, 1e-9);

    // Anisotropic check: same magnitude, different axis — must give same E.
    voxels[idx].flux = Vec3{0.0, 0.0, 0.0};
    const int idx2 = rb.lattice().index(5, 5, 5);
    voxels[idx2].flux = Vec3{0.0, std::sqrt(2.0), 0.0};  // |J|² = 2
    auto audit2 = compute_energy_audit(rb);
    check_close("field_energy = ½·|J|² (|J|²=2)", audit2.field_energy, 1.0, 1e-9);
}

// ---------------------------------------------------------------------------
// G-1: Locked-particle pair force does NOT skip the unlocked partner.
//
// JS-side bug from commit dc329d6: when scanning pairs, a `locked` particle
// was skipped, so the partner did not feel its force. The C++ side has the
// same risk in any forces-phase code that early-returns on locked. This test
// asserts that force-on-unlocked-B is delivered whether or not A is locked.
//
// Strategy: identical 2-particle scenarios, run for the same number of
// ticks, only difference is whether A.locked is set on tick 0. Compare B's
// kinetic energy / velocity. If locked-A bug exists, B sits still in the
// locked case and accelerates in the unlocked case → big delta.
// ---------------------------------------------------------------------------
static double run_pair_and_get_b_speed(bool lock_a) {
    RenderBridge rb(16);
    rb.force_cpu();

    // Keep physics minimal but still active for forces:
    //   forces ON, poisson_coulomb ON → Coulomb force computed
    //   movement OFF → so we read the velocity directly without it being
    //                   reset by sub-lattice transitions
    rb.toggles.movement = false;
    rb.toggles.damping = false;
    rb.toggles.selective_damping = false;  // depends on damping; silences toggle warning
    rb.toggles.larmor_radiation = false;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces = true;
    rb.toggles.poisson_coulomb = true;
    rb.toggles.lorentz_force = false;  // pure Coulomb — eliminate B-field cross-talk
    rb.toggles.gravity = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.genesis = false;        // nothing should manifest spontaneously
    rb.toggles.coupling = false;       // no g_c·∇s source — keep flux clean

    // Two static charges, separation 5 along x.
    rb.inject_particle( 5, 8, 8, +1, {0, 0, 0});
    rb.inject_particle(10, 8, 8, -1, {0, 0, 0});

    if (lock_a) {
        // Set locked AFTER injection; both still exist as charges.
        rb.voxels()[rb.lattice().index(5, 8, 8)].locked = true;
    }

    // Few ticks — long enough for Poisson to settle and forces to integrate.
    for (int t = 0; t < 5; ++t) rb.tick();

    // B is the unlocked partner at (10,8,8).
    const auto& vB = rb.voxels()[rb.lattice().index(10, 8, 8)].velocity;
    return vB.mag();
}

void test_locked_partner_still_feels_force() {
    section("G-1: Locked partner still pushes unlocked partner");

    const double speed_unlocked = run_pair_and_get_b_speed(/*lock_a=*/false);
    const double speed_locked   = run_pair_and_get_b_speed(/*lock_a=*/true);

    // B must accelerate in BOTH scenarios — locked-A still produces a force.
    // If the C++ has a "skip locked partner" bug, speed_locked ≈ 0 while
    // speed_unlocked > 0.
    check("B accelerates when A is unlocked",
          speed_unlocked > 1e-6,
          "If this fails, the test setup itself is broken (no Coulomb force at all).");
    check("B accelerates when A is locked",
          speed_locked > 1e-6,
          "FAIL means locked-A is being skipped as a force source.");

    // Both magnitudes should be comparable — locked vs unlocked changes
    // movement integration of A but not the Coulomb force B feels FROM A.
    if (speed_unlocked > 1e-9) {
        const double ratio = speed_locked / speed_unlocked;
        check("locked/unlocked B-speed ratio within 0.5×",
              ratio > 0.5 && ratio < 2.0,
              "Locked partner producing wildly different force on B suggests partial-skip bug.");
    }
}

// ---------------------------------------------------------------------------
// G-2: E_L vs wv_L split test.
//
// Commit d0329f6 fixed a conflation where E_L_total used to also pick up
// wave_vel_L energy. Confirm that flux_L → E_L_total only and wave_vel_L →
// wv_L_total only, both with the ½ factor (G-6 tie-in).
// ---------------------------------------------------------------------------
void test_dual_substrate_e_vs_wv_split() {
    section("G-2: Dual-substrate E_L (flux) vs wv_L (wave_vel) split");

    RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.dual_substrate = true;

    auto& voxels = rb.voxels();

    // Site i: flux_L only.
    const int i = rb.lattice().index(2, 2, 2);
    voxels[i].flux_L     = Vec3{1.0, 0.0, 0.0};
    voxels[i].wave_vel_L = Vec3{0.0, 0.0, 0.0};
    voxels[i].flux_R     = Vec3{0.0, 0.0, 0.0};
    voxels[i].wave_vel_R = Vec3{0.0, 0.0, 0.0};

    // Site j: wave_vel_L only.
    const int j = rb.lattice().index(5, 5, 5);
    voxels[j].flux_L     = Vec3{0.0, 0.0, 0.0};
    voxels[j].wave_vel_L = Vec3{1.0, 0.0, 0.0};
    voxels[j].flux_R     = Vec3{0.0, 0.0, 0.0};
    voxels[j].wave_vel_R = Vec3{0.0, 0.0, 0.0};

    auto audit = compute_energy_audit(rb);

    // After the d0329f6 conflation fix:
    //   E_L_total picks up flux_L only            → 0.5 from site i
    //   wv_L_total picks up wave_vel_L only       → 0.5 from site j
    //   E_R_total / wv_R_total stay zero (R-substrate untouched)
    check_close("E_L_total = ½·|flux_L|² only",   audit.E_L_total,  0.5, 1e-9);
    check_close("wv_L_total = ½·|wave_vel_L|² only", audit.wv_L_total, 0.5, 1e-9);
    check_close("E_R_total = 0  (R untouched)",    audit.E_R_total,  0.0, 1e-9);
    check_close("wv_R_total = 0 (R untouched)",    audit.wv_R_total, 0.0, 1e-9);
}

// ---------------------------------------------------------------------------
// G-4: 18-pt Laplacian sum-rule unit test.
//
// The isotropic 18-point stencil weights must satisfy
//      6·face + 12·edge − 4·center = 0
// for the central voxel coefficient (4) to be the correct
// closure of the sum. If somebody changes face/edge weights without
// retuning the center coefficient, the discrete Laplacian gains a constant
// drift on uniform fields — silent killer of energy conservation tests.
// ---------------------------------------------------------------------------
void test_laplacian_sum_rule() {
    section("G-4: 18-pt Laplacian sum-rule");

    const double sum = LAPLACIAN_FACE_WEIGHT * 6.0
                     + LAPLACIAN_EDGE_WEIGHT * 12.0
                     - 4.0;
    check_close("face·6 + edge·12 − 4 = 0", sum, 0.0, 1e-15);

    // Component checks for clarity if the line above ever fails.
    check_close("LAPLACIAN_FACE_WEIGHT = 1/3", LAPLACIAN_FACE_WEIGHT, 1.0 / 3.0, 1e-15);
    check_close("LAPLACIAN_EDGE_WEIGHT = 1/6", LAPLACIAN_EDGE_WEIGHT, 1.0 / 6.0, 1e-15);
}

// ---------------------------------------------------------------------------
// G-Coulomb-PE-pair: coulomb_pe matches the canonical ½·Σ α·q·φ convention.
//
// After the 2026-04-27 ½-fix in diagnostics_compute.cpp (lines 138-148),
// audit.coulomb_pe should equal exactly  ½·Σ_i α·q_i·φ_i  computed over
// manifested sites. Compute that sum manually from rb.voxels() and
// rb.phi_coulomb() and compare.
//
// We use 2 opposite charges at separation 2 voxels and run enough ticks
// to let the warm-started SOR Poisson solver settle.
// ---------------------------------------------------------------------------
void test_coulomb_pe_pair_convention() {
    section("G-Coulomb-PE-pair: ½·Σ α·q·φ convention");

    RenderBridge rb(16);
    rb.force_cpu();

    rb.toggles.movement = false;
    rb.toggles.damping = false;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces = true;
    rb.toggles.poisson_coulomb = true;
    rb.toggles.lorentz_force = false;
    rb.toggles.gravity = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.genesis = false;
    rb.toggles.coupling = false;
    rb.set_sor_iterations(30);  // tight Poisson convergence

    rb.inject_particle(7, 8, 8, +1, {0, 0, 0});
    rb.inject_particle(9, 8, 8, -1, {0, 0, 0});

    for (int t = 0; t < 10; ++t) rb.tick();

    const auto& voxels      = rb.voxels();
    const auto& phi_coulomb = rb.phi_coulomb();

    // Skip with a recorded reason if the Poisson solver buffer is empty
    // (poisson_coulomb toggle path not active in this build configuration).
    if (phi_coulomb.empty()) {
        check("phi_coulomb available (skip otherwise)", false,
              "Poisson solver did not populate phi_coulomb_; skipping pair-PE check.");
        return;
    }

    // Manual computation of ½·Σ α·q·φ over manifested voxels.
    double manual_pe = 0.0;
    int n_manifested = 0;
    const int N = static_cast<int>(rb.lattice().total_sites());
    for (int idx = 0; idx < N; ++idx) {
        const auto& v = voxels[idx];
        if (v.state != 0) {
            manual_pe += 0.5 * ALPHA * static_cast<double>(v.state) * phi_coulomb[idx];
            ++n_manifested;
        }
    }

    // Sanity: we still have both particles at the end of the run.
    check("two particles still manifested", n_manifested == 2);

    auto audit = compute_energy_audit(rb);
    check_close("audit.coulomb_pe = ½·Σ α·q·φ",
                audit.coulomb_pe, manual_pe, 1e-6);
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_audit_regression");

    ftd::test::test_energy_audit_half_factor();
    ftd::test::test_locked_partner_still_feels_force();
    ftd::test::test_dual_substrate_e_vs_wv_split();
    ftd::test::test_laplacian_sum_rule();
    ftd::test::test_coulomb_pe_pair_convention();

    return ftd::test::finalize();
}
