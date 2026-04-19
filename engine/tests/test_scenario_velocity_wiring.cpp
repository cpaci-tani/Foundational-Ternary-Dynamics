/**
 * test_scenario_velocity_wiring.cpp
 *
 * Audit item (physics-orchestrator, 2026-04-18): after porting the
 * JS flux-meson / flux-string-breaking / flux-baryon scenarios to
 * engine/src/scenarios.cpp, verify that the SET_VEL macro (which
 * writes into voxel_at(x,y,z).velocity) actually drives particle
 * kinematics in phase_movement().
 *
 * JS path (MockBridge): scenario writes p.vy on an entry in the
 * this._particles[] array, then MockBridge._phaseMovement() reads it.
 *
 * C++ path (RenderBridge): scenario writes voxel_at(x,y,z).velocity.
 * phase_movement() reads v.velocity from the voxel directly — see
 * render_bridge.cpp line 1255: `v.remainder += v.velocity * dt_`.
 *
 * Equivalence criterion: for flux-meson, the two quark analogs are
 * placed at (mL, mc, mc) with v = (0, +0.05, 0) and (mR, mc, mc) with
 * v = (0, -0.05, 0). With dt ≥ 1, after a single tick each particle's
 * remainder should advance by 0.05·dt on the y-axis and stay below 1
 * (so NO integer jump yet — we verify the remainder bookkeeping is
 * live and the velocity survived the tick).
 *
 * This is a minimal wiring test. Broader physics behavior (string
 * formation, confinement linearity, etc.) is covered by the QCD
 * campaign tests.
 */

#include <cmath>
#include <iostream>
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"

int failures = 0;

static void check(const char* name, bool cond) {
    std::cout << (cond ? "  PASS  " : "  FAIL  ") << name << "\n";
    if (!cond) ++failures;
}

static void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::fabs(a - b) <= tol;
    std::cout << (ok ? "  PASS  " : "  FAIL  ") << name
              << "  got=" << a << " expected=" << b
              << " |diff|=" << std::fabs(a - b) << " tol=" << tol << "\n";
    if (!ok) ++failures;
}

int main() {
    std::cout << "=== test_scenario_velocity_wiring ===\n";

    const int L = 32;
    ftd::RenderBridge rb(L);

    // Disable every extension we can. We are testing ONE thing:
    // does voxel.velocity drive phase_movement. Damping, forces,
    // and coupling must not muddy the remainder arithmetic.
    rb.toggles.wave_propagation = false;
    rb.toggles.coupling         = false;
    rb.toggles.damping          = false;
    rb.toggles.genesis          = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.forces           = false;
    rb.toggles.gravity          = false;
    rb.toggles.poisson_coulomb  = false;
    rb.toggles.lorentz_force    = false;
    rb.toggles.selective_damping = false;
    rb.toggles.dual_substrate   = false;
    rb.toggles.weak_transmutation = false;
    // movement MUST be on — that's the subject of the test
    rb.toggles.movement = true;

    rb.set_dt(1.0);

    // Run the exact setup the physics-orchestrator flagged.
    bool handled = ftd::setup_flux_scenario(rb, "flux-meson");
    check("flux-meson scenario dispatched", handled);

    // Compute the same particle coordinates the scenario code uses.
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mOff = std::max(2, N / 8);
    const int    mL_x = static_cast<int>(std::floor(midF)) - mOff;
    const int    mR_x = static_cast<int>(std::ceil (midF)) + mOff;
    const int    mc   = static_cast<int>(std::round(midF));

    // Before tick: voxels should hold the seeded velocity.
    auto& vL0 = rb.voxel_at(mL_x, mc, mc);
    auto& vR0 = rb.voxel_at(mR_x, mc, mc);
    check("left quark manifested",       vL0.state == 1);
    check("right quark manifested",      vR0.state == -1);
    check_close("left quark vy seeded",  vL0.velocity.y,  0.05, 1e-12);
    check_close("right quark vy seeded", vR0.velocity.y, -0.05, 1e-12);

    // One tick. Expected motion: remainder.y advances by 0.05 each;
    // |0.05| < 1, so no integer jump — particle stays at the same voxel.
    rb.tick();

    auto& vL1 = rb.voxel_at(mL_x, mc, mc);
    auto& vR1 = rb.voxel_at(mR_x, mc, mc);

    check("left quark still present after 1 tick",  vL1.state == 1);
    check("right quark still present after 1 tick", vR1.state == -1);

    // Velocity survives (no damping, no force path): exact.
    check_close("left quark velocity preserved",  vL1.velocity.y,  0.05, 1e-12);
    check_close("right quark velocity preserved", vR1.velocity.y, -0.05, 1e-12);

    // Remainder advanced by v*dt. THIS is the load-bearing assertion:
    // it proves phase_movement actually read voxel.velocity.
    check_close("left quark remainder.y advanced",  vL1.remainder.y,  0.05, 1e-9);
    check_close("right quark remainder.y advanced", vR1.remainder.y, -0.05, 1e-9);

    // Run enough ticks to force an integer jump (|0.05|*20 = 1.0).
    for (int t = 0; t < 20; ++t) rb.tick();

    // After 21 total ticks of v=0.05, left quark should have moved +1 in y.
    auto& vL_moved = rb.voxel_at(mL_x, mc + 1, mc);
    auto& vR_moved = rb.voxel_at(mR_x, mc - 1, mc);
    check("left quark translated +y by 1 lattice unit",  vL_moved.state == 1);
    check("right quark translated -y by 1 lattice unit", vR_moved.state == -1);

    // And the old positions should be void (the transfer in phase_movement
    // resets source state/velocity/remainder).
    auto& vL_old = rb.voxel_at(mL_x, mc, mc);
    auto& vR_old = rb.voxel_at(mR_x, mc, mc);
    check("left quark old position vacated",  vL_old.state == 0);
    check("right quark old position vacated", vR_old.state == 0);

    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
