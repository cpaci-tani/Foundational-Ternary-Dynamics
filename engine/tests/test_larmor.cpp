/**
 * Test: Larmor Radiation (Acceleration-Dependent Damping)
 *
 * When the larmor_radiation toggle is ON, damping at near-particle sites is
 * modulated by the particle's acceleration (ftd/larmor_damping.h):
 *
 *   gain(a)     = min(1 + K_LARMOR * |a|², LARMOR_MAX_GAIN)
 *   eff_damping = damping_factor ^ gain(a)
 *
 * Static charges (a=0) → exactly the baseline damping (the toggle is a no-op)
 * Accelerating charges → strictly MORE damping than the baseline, growing
 *                        monotonically with |a|
 *
 * This implements the classical Larmor formula: P ∝ a² — accelerating
 * charges radiate energy proportional to their acceleration squared, so they
 * must lose energy FASTER than a static charge, not slower.
 *
 * AMENDED 2026-09-16. Until this date the engine computed
 *     larmor_mod  = min(1, LARMOR_FLOOR + K_LARMOR * |a|²)
 *     eff_damping = 1 - DAMPING * larmor_mod
 * — the baseline loss multiplied by a factor CAPPED AT 1 — so enabling the
 * toggle could only ever REDUCE dissipation below the undamped baseline (1% of
 * baseline at a=0, parity at a≈0.171, never above). LAM-1 asserted exactly that
 * inverted behaviour ("static charge decays SLOWER with Larmor ON", damping
 * rate == LARMOR_FLOOR); it is rewritten below to assert the corrected physics.
 * LAM-3 measured the a² scaling of the old `larmor_mod` and now measures the a²
 * scaling of the gain excess. The law's own properties (monotonicity,
 * boundedness, baseline-exactness at a=0) are gated by
 * tests/test_larmor_damping_law.cpp.
 *
 * Tests:
 *   LAM-1: Static charge (a=0) with Larmor ON == the baseline run exactly
 *   LAM-2: Accelerating charge (Coulomb pair) loses energy faster
 *   LAM-3: Larmor gain excess proportional to a²
 *   LAM-4: Toggle OFF = exact match to baseline (no behavior change)
 *   LAM-5: Selective damping + Larmor interaction (void=no damp, particle=Larmor)
 *   LAM-6: Dipole radiation spatial profile (equatorial > axial by sin²θ)
 *
 * Constants (from constants.h / larmor_damping.h):
 *   K_LARMOR = 4*N_EFF/(3*K_B) ≈ 33.9
 *   LARMOR_FLOOR = 0.01   (retained for provenance; no longer in the law —
 *                          gain ≥ 1 makes "dissipation is never off" hold by
 *                          construction)
 *   LARMOR_MAX_GAIN = 256
 *
 * Theory references:
 *   - CLAUDE.md §6.3 (EM-like behavior)
 *   - constants.h: K_LARMOR, LARMOR_FLOOR
 *   - ftd/larmor_damping.h: the law itself, shared CPU/CUDA
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/larmor_damping.h"

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Larmor Radiation — 7 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // LAM-1: Static charge — Larmor is a no-op at zero acceleration
    // ================================================================
    std::cout << "\n-- LAM-1: Static Charge == Baseline --\n";
    {
        // Two-part test:
        // (a) Total energy: with the particle locked and forces off, every
        //     near-particle site has a = 0, so the Larmor run must match the
        //     uniform-damping run EXACTLY.
        // (b) Formula verification: at a=0 the gain is 1, so the effective
        //     damping factor is bit-exactly the baseline factor.
        //
        // Before 2026-09-16 this check asserted the opposite (Larmor retains
        // MORE energy, damping rate == LARMOR_FLOOR = 1% of baseline) because
        // the capped formula turned the toggle into a dissipation reducer.

        // Part (a): Total energy comparison
        double E_larmor = 0.0, E_uniform = 0.0;
        int ticks = 500;
        int mid = 8;

        for (int trial = 0; trial < 2; ++trial) {
            ftd::RenderBridge rb(16);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.selective_damping = true;
            rb.toggles.larmor_radiation = (trial == 0);

            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

            rb.run(100);  // Let self-field build
            double E0 = rb.energy_audit().field_energy;
            rb.run(ticks);
            double Ef = rb.energy_audit().field_energy;

            if (trial == 0) E_larmor = Ef / E0;
            else E_uniform = Ef / E0;
        }

        std::cout << "    Larmor: " << E_larmor * 100 << "% energy remaining\n";
        std::cout << "    Uniform: " << E_uniform * 100 << "% remaining\n";
        std::cout << "    Difference: " << std::abs(E_larmor - E_uniform) << "\n";

        // Part (b): Direct formula verification at a = 0.
        // gain(0) = 1  ⇒  eff_damping = baseline^1 = baseline, bit-exactly.
        const double baseline = 1.0 - ftd::DAMPING;
        const double eff0 = ftd::larmor_effective_damping(baseline, 0.0);
        std::cout << "    Baseline survival factor:   " << std::setprecision(17)
                  << baseline << "\n";
        std::cout << "    Larmor factor at a = 0:     " << eff0 << "\n"
                  << std::setprecision(6);

        // At zero acceleration there is no radiation reaction, so the toggle
        // changes nothing — in the formula and in the run.
        check("LAM-1: Static charge — Larmor at a=0 is exactly the baseline",
              E_larmor == E_uniform && eff0 == baseline);
    }

    // ================================================================
    // LAM-2: Accelerating charge loses energy faster
    // ================================================================
    std::cout << "\n-- LAM-2: Accelerating Charge Enhanced Damping --\n";
    {
        // Two opposite charges attract → accelerate → Larmor enhances damping
        double E_larmor_final = 0.0, E_nolarmor_final = 0.0;
        int ticks = 300;

        for (int trial = 0; trial < 2; ++trial) {
            ftd::RenderBridge rb(32);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.movement = true;
            rb.toggles.selective_damping = true;
            rb.toggles.larmor_radiation = (trial == 0);
            rb.toggles.gravity = false;

            int mid = 16;
            rb.inject_particle(mid - 5, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});

            rb.run(100);  // Let self-fields build
            double E0 = rb.energy_audit().field_energy;
            rb.run(ticks);
            double Ef = rb.energy_audit().field_energy;

            if (trial == 0) E_larmor_final = Ef / E0;
            else E_nolarmor_final = Ef / E0;
        }

        std::cout << "    With Larmor: " << E_larmor_final * 100 << "% remaining\n";
        std::cout << "    Without: " << E_nolarmor_final * 100 << "% remaining\n";

        // Accelerating charges should lose MORE energy with Larmor ON
        check("LAM-2: Accelerating pair loses more energy with Larmor (less remaining)",
              E_larmor_final < E_nolarmor_final);
    }

    // ================================================================
    // LAM-3: Larmor modulation proportional to a²
    // ================================================================
    std::cout << "\n-- LAM-3: Power ∝ a² Verification --\n";
    {
        // The Larmor law: gain(a) = min(1 + K_LARMOR * a², LARMOR_MAX_GAIN).
        // The excess over the baseline gain of 1 is K_LARMOR * a², so doubling
        // a must quadruple it. We verify the law directly with known
        // acceleration values, well below the clamp knee at
        // a = sqrt((LARMOR_MAX_GAIN - 1)/K_LARMOR) ≈ 2.74.
        //
        // Updated 2026-09-16: previously measured the old capped `larmor_mod`;
        // the shape of the a² law is unchanged, only where it enters.

        double a1 = 0.05;
        double a2 = 0.10;

        double active1 = ftd::larmor_damping_gain(a1) - 1.0;
        double active2 = ftd::larmor_damping_gain(a2) - 1.0;
        double ratio = (active1 > 0) ? active2 / active1 : 0;

        std::cout << "    gain(a=0.05) = " << ftd::larmor_damping_gain(a1)
                  << ", gain(a=0.10) = " << ftd::larmor_damping_gain(a2) << "\n";
        std::cout << "    Excess ratio: " << ratio << " (expected 4.0 for a² scaling)\n";
        std::cout << "    K_LARMOR = " << ftd::K_LARMOR
                  << ", LARMOR_MAX_GAIN = " << ftd::LARMOR_MAX_GAIN << "\n";

        check("LAM-3: Larmor gain excess scales as a² (ratio = 4.0 ± 0.01)",
              std::abs(ratio - 4.0) < 0.01);
    }

    // ================================================================
    // LAM-4: Toggle OFF = exact baseline match
    // ================================================================
    std::cout << "\n-- LAM-4: Toggle OFF Baseline Match --\n";
    {
        // Run identical simulations: one with larmor=false, one default
        // Results must be IDENTICAL (bit-exact)
        double E_off = 0.0, E_default = 0.0;
        int ticks = 200;

        for (int trial = 0; trial < 2; ++trial) {
            ftd::RenderBridge rb(16);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.selective_damping = true;

            if (trial == 0) rb.toggles.larmor_radiation = false;
            else rb.toggles.larmor_radiation = false;  // Both OFF

            int mid = 8;
            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});

            rb.run(ticks);
            auto a = rb.energy_audit();
            if (trial == 0) E_off = a.field_energy;
            else E_default = a.field_energy;
        }

        double diff = std::abs(E_off - E_default);
        std::cout << "    Toggle OFF: E = " << E_off << "\n";
        std::cout << "    Default:    E = " << E_default << "\n";
        std::cout << "    Difference: " << diff << "\n";

        check("LAM-4: Toggle OFF = exact baseline (diff = 0)",
              diff == 0.0);
    }

    // ================================================================
    // LAM-5: Selective damping + Larmor interaction
    // ================================================================
    std::cout << "\n-- LAM-5: Selective + Larmor Combined --\n";
    {
        // With selective_damping=true AND larmor_radiation=true:
        // - Void sites: NO damping (selective blocks it)
        // - Particle sites: Larmor-modulated damping
        // - Near-particle void sites: uniform damping (not Larmor, since state=0)

        ftd::RenderBridge rb(16);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.damping = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.selective_damping = true;
        rb.toggles.larmor_radiation = true;

        int mid = 8;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Also inject flux at a distant void site (should NOT be damped)
        rb.inject_flux(1, 1, 1, {0, 0.5, 0});

        rb.run(100);

        // Distant flux should be mostly undamped (selective blocks it)
        double far_flux = rb.voxels()[rb.lattice().index(1, 1, 1)].flux.mag();
        // Particle-site flux should be damped (Larmor-modulated)
        double particle_flux = rb.voxels()[rb.lattice().index(mid, mid, mid)].flux.mag();

        // The far flux propagates via wave equation, so it's spread out.
        // But there should still be energy at (1,1,1) from wave propagation
        // The key check: the system runs without crashing and produces finite values
        std::cout << "    Far flux |J| at (1,1,1) = " << far_flux << "\n";
        std::cout << "    Particle |J| at center = " << particle_flux << "\n";

        check("LAM-5: Selective + Larmor runs correctly (finite values)",
              std::isfinite(far_flux) && std::isfinite(particle_flux));
    }

    // ================================================================
    // LAM-6: Dipole radiation spatial profile
    // ================================================================
    std::cout << "\n-- LAM-6: Dipole Radiation Spatial Profile --\n";
    {
        // Classical dipole radiation has angular dependence P(θ) ∝ sin²(θ)
        // where θ is the angle from the acceleration axis.
        // Equatorial (θ=π/2): maximum radiation
        // Axial (θ=0,π): zero radiation
        //
        // Test: locked dipole pair along z-axis, measure |J|² at distance R
        // in equatorial plane vs along the axis. Equatorial should be larger.
        //
        // Grid: 32³, pair at (16,16,14) and (16,16,18) — z-axis dipole
        // Measure at R=10: equatorial (26,16,16) vs axial (16,16,6)
        //
        // ⚠ REWRITTEN 2026-09-16 — THIS CHECK NEVER MEASURED LARMOR, AND ITS
        // OLD THRESHOLD WAS AN ARTIFACT OF THE DAMPING BUG. Measured A/B on
        // this exact scenario:
        //     old (capped) law, larmor ON : eq/ax = 6.089   → passed > 1.5
        //     corrected law,    larmor ON : eq/ax = 1.364   → would fail > 1.5
        //     corrected law,    larmor OFF: eq/ax = 1.364   → identical
        // The peak acceleration here is |a| = 3.83e-5, so the radiation-reaction
        // gain is 1 + K_LARMOR·a² = 1 + 5e-8 — a null effect, exactly as it
        // should be. The old law's 6.089 came from the fact that it applied only
        // LARMOR_FLOOR = 1% of the baseline damping at near-particle sites, i.e.
        // it effectively switched damping OFF around the dipole; the "dipole
        // pattern" it scored was the pattern of an undamped source, not a Larmor
        // effect. No value of K_LARMOR could produce 6.089 under a law that
        // damps at least as hard as the baseline.
        //
        // The check now (a) asserts the part that is physically true and
        // measurable — equatorial exceeds axial — and (b) asserts the null
        // result explicitly: at these accelerations Larmor ON must be
        // indistinguishable from Larmor OFF. A future scenario with genuinely
        // large |a| is what would give this check Larmor content; that is an
        // open item, not something to recover by loosening a threshold.

        auto dipole_anisotropy = [](bool larmor, double& eq_out, double& ax_out) {
            ftd::RenderBridge rb(32);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.selective_damping = true;
            rb.toggles.larmor_radiation = larmor;

            // Create a z-aligned dipole: +1 at z=14, -1 at z=18
            int mid = 16;
            rb.inject_particle(mid, mid, mid - 2, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid, mid, mid + 2, -1, {0, 0, -ftd::K_B});

            // Lock both so they oscillate in place (bound dipole)
            rb.voxels()[rb.lattice().index(mid, mid, mid - 2)].locked = true;
            rb.voxels()[rb.lattice().index(mid, mid, mid + 2)].locked = true;

            // Let fields build and radiation pattern establish
            rb.run(500);

            // Measure flux energy density at radius R=10 from center
            int R = 10;

            // Equatorial samples: in xy-plane at z=mid (θ=π/2)
            double eq_energy = 0.0;
            int eq_count = 0;
            int eq_offsets[][2] = {{R, 0}, {-R, 0}, {0, R}, {0, -R}};
            for (auto& off : eq_offsets) {
                int x = mid + off[0], y = mid + off[1], z = mid;
                if (x >= 0 && x < 32 && y >= 0 && y < 32) {
                    double rho = rb.voxels()[rb.lattice().index(x, y, z)].density();
                    eq_energy += rho * rho;
                    eq_count++;
                }
            }
            eq_out = eq_energy / eq_count;

            // Axial samples: along z-axis (θ=0,π)
            double ax_energy = 0.0;
            int ax_count = 0;
            int ax_offsets[] = {R, -R};
            for (int dz : ax_offsets) {
                int x = mid, y = mid, z = mid + dz;
                if (z >= 0 && z < 32) {
                    double rho = rb.voxels()[rb.lattice().index(x, y, z)].density();
                    ax_energy += rho * rho;
                    ax_count++;
                }
            }
            ax_out = ax_energy / ax_count;
        };

        double eq_on = 0, ax_on = 0, eq_off = 0, ax_off = 0;
        dipole_anisotropy(true, eq_on, ax_on);
        dipole_anisotropy(false, eq_off, ax_off);

        double aniso_on = (ax_on > 1e-30) ? eq_on / ax_on : 0;
        double aniso_off = (ax_off > 1e-30) ? eq_off / ax_off : 0;

        std::cout << "    Equatorial |J|² (avg): " << std::scientific << eq_on << "\n";
        std::cout << "    Axial |J|² (avg):      " << std::scientific << ax_on << "\n";
        std::cout << "    Anisotropy ratio (eq/ax), Larmor ON:  " << std::fixed
                  << std::setprecision(4) << aniso_on << "\n";
        std::cout << "    Anisotropy ratio (eq/ax), Larmor OFF: "
                  << aniso_off << "\n";
        const double aniso_rel = std::abs(aniso_on - aniso_off)
                               / std::max(1e-30, aniso_off);
        std::cout << "    ON/OFF relative difference: " << std::scientific
                  << aniso_rel << std::fixed << "\n";
        std::cout << "    (Classical dipole predicts ratio → ∞; the lattice"
                     " near field gives ~1.36 at R=10)\n";

        check("LAM-6a: Equatorial radiation > axial (dipole pattern)",
              eq_on > 1e-30 && aniso_on > 1.0);
        // Gain excess here is K_LARMOR·a² ≈ 5e-8, so the ON/OFF split must stay
        // in the noise. The old capped law produced 6.089 vs 1.364 — a 4.5x
        // split — so a 0.1% bound is an ample, non-vacuous discriminator.
        check("LAM-6b: at |a| ~ 4e-5 the radiation-reaction gain is ~1, so "
              "Larmor ON is indistinguishable from OFF",
              aniso_rel < 1e-3);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (g_failures == 0)
        std::cout << "  All 7 Larmor radiation checks PASSED.\n";
    else
        std::cout << "  " << g_failures << " test(s) FAILED.\n";
    std::cout << "================================================================\n";

    return g_failures;
}
