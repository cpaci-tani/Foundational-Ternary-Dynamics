/**
 * Campaign: Coulomb force law (consolidated suite)
 *
 * Merges 5 legacy test/campaign_*.cpp files into a single ftd::test-instrumented
 * suite using the Phase 2a NDJSON telemetry API:
 *
 *   test_poisson_coulomb           -> section "poisson_coulomb"
 *   campaign_coulomb_convergence   -> section "coulomb_convergence"
 *   campaign_force_law             -> section "force_law"
 *   campaign_poisson_force_law     -> section "poisson_force_law"
 *   campaign_poisson_binding       -> section "poisson_binding"
 *
 * Every check(...) from the legacy files is preserved verbatim (same condition,
 * same label) and routed through ftd::test::check for uniform telemetry.
 *
 * Wave 4c.8 consolidation (2026-04-14). Structural parity only — no physics
 * fixes. File-scope helpers collide across sources and are renamed with
 * _pc/_cc/_fl/_pfl/_pb suffixes; all section helpers are static to avoid
 * linker collisions.
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: poisson_coulomb  (from test_poisson_coulomb.cpp)
// ============================================================================

static void section_poisson_coulomb() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Poisson-Based Coulomb Force (Phase 3) — 8 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // PC1: Force decreases with distance
    // ================================================================
    std::cout << "\n--- PC1: Force monotonically decreases ---\n";
    {
        ftd::RenderBridge rb(48);
        int mid = 24;
        // Locked +1 source at center
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Locked -1 probe particles at r=4 and r=8
        rb.inject_particle(mid + 4, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 4, mid, mid)].locked = true;
        rb.inject_particle(mid + 8, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 8, mid, mid)].locked = true;

        rb.run(200);

        double f4 = rb.force_diag_at(mid + 4, mid, mid).f_coulomb.mag();
        double f8 = rb.force_diag_at(mid + 8, mid, mid).f_coulomb.mag();
        std::cout << "    F(r=4) = " << f4 << ", F(r=8) = " << f8 << "\n";
        ftd::test::check("PC1: F(r=4) > F(r=8)", f4 > f8);
    }

    // ================================================================
    // PC2: Power law exponent — steeper than -1 (improved over legacy -3.8)
    // ================================================================
    // Each measurement uses a SEPARATE simulation with only one source and
    // one probe, avoiding multi-charge contamination of the Poisson solution.
    std::cout << "\n--- PC2: Power law exponent ---\n";
    {
        std::vector<int> radii = {3, 5, 7, 9, 11};
        std::vector<double> log_r, log_f;

        for (int r : radii) {
            ftd::RenderBridge rb(48);
            int mid = 24;
            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
            rb.run(200);
            double f = rb.force_diag_at(mid + r, mid, mid).f_coulomb.mag();
            if (f > 1e-20) {
                log_r.push_back(std::log(static_cast<double>(r)));
                log_f.push_back(std::log(f));
                std::cout << "    r=" << r << " F=" << f << "\n";
            }
        }

        double exponent = 0.0;
        if (log_r.size() >= 2) {
            double xbar = 0, ybar = 0;
            for (size_t i = 0; i < log_r.size(); ++i) {
                xbar += log_r[i];
                ybar += log_f[i];
            }
            xbar /= log_r.size();
            ybar /= log_f.size();
            double num = 0, den = 0;
            for (size_t i = 0; i < log_r.size(); ++i) {
                num += (log_r[i] - xbar) * (log_f[i] - ybar);
                den += (log_r[i] - xbar) * (log_r[i] - xbar);
            }
            exponent = (den > 1e-30) ? num / den : 0.0;
        }
        std::cout << "    Power law exponent = " << std::setprecision(3) << exponent << "\n";
        // Poisson solver on periodic lattice with warm-started SOR may not give
        // exact -2.0 (periodic images, finite iterations), but should be much
        // steeper than -1 and better than legacy -3.8. Accept [-3.5, -1.0].
        ftd::test::check("PC2: Exponent in [-3.5, -1.0] (improved over legacy -3.8)",
              exponent >= -3.5 && exponent <= -1.0);
    }

    // ================================================================
    // PC3: Long-range detection
    // ================================================================
    std::cout << "\n--- PC3: Long-range force ---\n";
    {
        ftd::RenderBridge rb(48);
        int mid = 24;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.inject_particle(mid + 2, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 2, mid, mid)].locked = true;
        rb.inject_particle(mid + 12, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 12, mid, mid)].locked = true;

        rb.run(300);

        double f2 = rb.force_diag_at(mid + 2, mid, mid).f_coulomb.mag();
        double f12 = rb.force_diag_at(mid + 12, mid, mid).f_coulomb.mag();
        double ratio = (f2 > 1e-30) ? f12 / f2 : 0.0;
        std::cout << "    F(r=2) = " << f2 << ", F(r=12) = " << f12
                  << ", ratio = " << ratio << "\n";
        ftd::test::check("PC3: F(r=12) > 1e-6 * F(r=2) (long range)", ratio > 1e-6);
    }

    // ================================================================
    // PC4: Unlike charges attract
    // ================================================================
    std::cout << "\n--- PC4: Unlike charges attract ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        // +1 at left, -1 at right
        rb.inject_particle(mid - 4, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 4, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 4, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 4, mid, mid)].locked = true;

        rb.run(100);

        // Force on +1 should point rightward (+x), toward the -1
        ftd::Vec3 f_pos = rb.force_diag_at(mid - 4, mid, mid).f_coulomb;
        // Force on -1 should point leftward (-x), toward the +1
        ftd::Vec3 f_neg = rb.force_diag_at(mid + 4, mid, mid).f_coulomb;
        std::cout << "    F(+1).x = " << f_pos.x << ", F(-1).x = " << f_neg.x << "\n";
        ftd::test::check("PC4: +1 attracted toward -1 (F.x > 0)", f_pos.x > 0);
    }

    // ================================================================
    // PC5: Like charges repel
    // ================================================================
    std::cout << "\n--- PC5: Like charges repel ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 4, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 4, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 4, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 4, mid, mid)].locked = true;

        rb.run(100);

        ftd::Vec3 f_left = rb.force_diag_at(mid - 4, mid, mid).f_coulomb;
        std::cout << "    F(left +1).x = " << f_left.x << "\n";
        ftd::test::check("PC5: Like charges repel (F.x < 0, away from right)", f_left.x < 0);
    }

    // ================================================================
    // PC6: Isotropy > 0.5 at r=5
    // ================================================================
    std::cout << "\n--- PC6: Isotropy ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        // Source at center with isotropic flux
        double kb3 = ftd::K_B / std::sqrt(3.0);
        rb.inject_particle(mid, mid, mid, +1, {kb3, kb3, kb3});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Probe charges on 3 axes at r=5
        rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 5, mid, mid)].locked = true;
        rb.inject_particle(mid, mid + 5, mid, -1, {0, -ftd::K_B, 0});
        rb.voxels()[rb.lattice().index(mid, mid + 5, mid)].locked = true;
        rb.inject_particle(mid, mid, mid + 5, -1, {-ftd::K_B, 0, 0});
        rb.voxels()[rb.lattice().index(mid, mid, mid + 5)].locked = true;

        rb.run(200);

        double fx = rb.force_diag_at(mid + 5, mid, mid).f_coulomb.mag();
        double fy = rb.force_diag_at(mid, mid + 5, mid).f_coulomb.mag();
        double fz = rb.force_diag_at(mid, mid, mid + 5).f_coulomb.mag();
        double fmax = std::max({fx, fy, fz});
        double fmin = std::min({fx, fy, fz});
        double isotropy = (fmax > 1e-30) ? fmin / fmax : 0.0;
        std::cout << "    F_x=" << fx << " F_y=" << fy << " F_z=" << fz
                  << " isotropy=" << isotropy << "\n";
        ftd::test::check("PC6: Isotropy ratio > 0.5 at r=5", isotropy > 0.5);
    }

    // ================================================================
    // PC7: Warm-start convergence
    // ================================================================
    std::cout << "\n--- PC7: Warm-start ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // First tick: cold start
        rb.run(1);
        auto a1 = rb.energy_audit();
        double residual1 = a1.gauss_violation;

        // Second tick: warm start (phi_coulomb_ retains previous solution)
        rb.run(1);
        auto a2 = rb.energy_audit();
        double residual2 = a2.gauss_violation;

        std::cout << "    Gauss residual tick 1: " << residual1
                  << ", tick 2: " << residual2 << "\n";
        // Warm-started SOR should converge faster (lower residual or similar)
        // Since the particle is locked, the charge config is identical → warm start helps
        ftd::test::check("PC7: Warm-start produces valid result (residual finite)",
              std::isfinite(residual2));
    }

    // ================================================================
    // PC8: Toggle off → legacy ∇∇J behavior
    // ================================================================
    std::cout << "\n--- PC8: Toggle off ---\n";
    {
        // Run with Poisson
        ftd::RenderBridge rb_poisson(32);
        int mid = 16;
        rb_poisson.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb_poisson.voxels()[rb_poisson.lattice().index(mid, mid, mid)].locked = true;
        rb_poisson.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
        rb_poisson.voxels()[rb_poisson.lattice().index(mid + 5, mid, mid)].locked = true;
        rb_poisson.run(100);
        double f_poisson = rb_poisson.force_diag_at(mid + 5, mid, mid).f_coulomb.mag();

        // Run with legacy
        ftd::RenderBridge rb_legacy(32);
        rb_legacy.toggles.poisson_coulomb = false;
        rb_legacy.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb_legacy.voxels()[rb_legacy.lattice().index(mid, mid, mid)].locked = true;
        rb_legacy.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
        rb_legacy.voxels()[rb_legacy.lattice().index(mid + 5, mid, mid)].locked = true;
        rb_legacy.run(100);
        double f_legacy = rb_legacy.force_diag_at(mid + 5, mid, mid).f_coulomb.mag();

        std::cout << "    Poisson force at r=5: " << f_poisson << "\n";
        std::cout << "    Legacy  force at r=5: " << f_legacy << "\n";
        // Both should be nonzero, and they should differ (different force laws)
        ftd::test::check("PC8: Both methods produce nonzero force",
              f_poisson > 1e-15 && f_legacy > 1e-15);
    }

    std::cout << "\n================================================================\n";
    std::cout << "  Poisson-Coulomb section complete.\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// Section: coulomb_convergence  (from campaign_coulomb_convergence.cpp)
// ============================================================================

static void section_coulomb_convergence() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Coulomb Convergence (Phase 2) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int mid = L / 2;
    const int SETUP_TICKS = 200;  // Let self-field establish

    // Measure force at several distances
    std::vector<double> radii = {4, 6, 8, 10, 12};
    std::vector<double> forces;
    std::vector<double> log_r, log_f;

    std::cout << "\n--- Force vs Distance ---\n";
    std::cout << "  r    | force     | log(r)    | log(|F|)\n";

    for (double r : radii) {
        int rx = static_cast<int>(r);

        // Fresh lattice each measurement
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;  // Isolate EM: G_N=0.01 >> α/(4π) contaminates

        // Source charge at center
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Let self-field establish
        rb.run(SETUP_TICKS);

        // Place probe charge, measure force via 1-tick velocity change
        int probe_x = mid + rx;
        rb.inject_particle(probe_x, mid, mid, +1, {0, 0, ftd::K_B * 0.1});

        auto& probe = rb.voxels()[rb.lattice().index(probe_x, mid, mid)];
        double vx_before = probe.velocity.x;

        rb.tick();

        double vx_after = rb.voxels()[rb.lattice().index(probe_x, mid, mid)].velocity.x;
        double f = vx_after - vx_before;  // Force ≈ Δv (mass=1 in natural units)

        forces.push_back(f);

        if (std::abs(f) > 1e-30) {
            log_r.push_back(std::log(r));
            log_f.push_back(std::log(std::abs(f)));
        }

        std::cout << "  " << std::setw(4) << r
                  << " | " << std::setw(12) << f
                  << " | " << std::setw(8) << std::log(r)
                  << " | " << std::setw(8) << (std::abs(f) > 1e-30 ? std::log(std::abs(f)) : -99.0)
                  << "\n";
    }

    // ----------------------------------------------------------------
    // Linear regression: log(F) = n*log(r) + c
    // ----------------------------------------------------------------
    double exponent = 0.0;
    double r_squared = 0.0;

    if (log_r.size() >= 3) {
        int n = static_cast<int>(log_r.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
        for (int i = 0; i < n; ++i) {
            sx += log_r[i]; sy += log_f[i];
            sxx += log_r[i] * log_r[i];
            sxy += log_r[i] * log_f[i];
            syy += log_f[i] * log_f[i];
        }
        double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            exponent = (n * sxy - sx * sy) / denom;
            double intercept = (sy - exponent * sx) / n;

            // R² calculation
            double ss_res = 0.0, ss_tot = 0.0;
            double mean_y = sy / n;
            for (int i = 0; i < n; ++i) {
                double pred = exponent * log_r[i] + intercept;
                ss_res += (log_f[i] - pred) * (log_f[i] - pred);
                ss_tot += (log_f[i] - mean_y) * (log_f[i] - mean_y);
            }
            r_squared = 1.0 - ss_res / (ss_tot + 1e-30);
        }
    }

    std::cout << "\n--- Power Law Fit ---\n";
    std::cout << "  Exponent = " << exponent << " (theory: -2.0)\n";
    std::cout << "  R² = " << r_squared << "\n";

    // ----------------------------------------------------------------
    // CC1: Exponent in reasonable range
    // ----------------------------------------------------------------
    ftd::test::check("CC1: Force exponent between -1.5 and -3.0",
          exponent < -1.5 && exponent > -3.0);

    // ----------------------------------------------------------------
    // CC2: Force is repulsive (positive for same-sign charges)
    // ----------------------------------------------------------------
    bool all_repulsive = true;
    for (double f : forces) {
        if (f < -1e-10) { all_repulsive = false; break; }
    }
    ftd::test::check("CC2: Force is repulsive for same-sign charges", all_repulsive);

    // ----------------------------------------------------------------
    // CC3: Power law fit is good
    // ----------------------------------------------------------------
    ftd::test::check("CC3: R² > 0.90 (good power-law fit)", r_squared > 0.90);

    // ----------------------------------------------------------------
    // CC4: Force decreases with distance
    // ----------------------------------------------------------------
    bool decreasing = true;
    for (size_t i = 1; i < forces.size(); ++i) {
        if (forces[i] > forces[i-1] + 1e-10) {
            decreasing = false;
            break;
        }
    }
    ftd::test::check("CC4: Force decreases with distance", decreasing);

    std::cout << "\n================================================================\n";
    std::cout << "  Coulomb convergence section complete.\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// Section: force_law  (from campaign_force_law.cpp)
// ============================================================================

static void section_force_law() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Force Law Profile\n";
    std::cout << "================================================================\n\n";

    const int L = 48;
    ftd::RenderBridge engine(L);
    int mid = L / 2;

    // Place a single locked +1 particle at center with isotropic flux
    double iso = ftd::K_B / std::sqrt(3.0);
    engine.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(mid, mid, mid).locked = true;

    // Equilibrate — with C_WAVE=1/√3, self-field extends to r_eff≈6.8
    // and needs more time to fully settle
    std::cout << "  Equilibrating 1000 ticks on 48^3 lattice...\n";
    engine.run(1000);

    // ---- Test 1: Monotonic decrease ----
    std::cout << "\n  --- Force vs Distance ---\n";
    std::cout << "  r, |grad(div(J))|\n";

    std::vector<int> radii = {2, 4, 6, 8, 10, 12, 14, 16};
    std::vector<double> forces;

    for (int r : radii) {
        int px = mid + r;
        int idx = engine.lattice().index(px, mid, mid);
        ftd::Vec3 gdj = engine.gradient_divergence(idx);
        double F = gdj.mag();
        forces.push_back(F);
        std::cout << "  " << std::setw(3) << r << ", "
                  << std::setprecision(8) << std::scientific << F << "\n";
    }

    // F1: Monotonic decrease in the clean far-field zone.
    // The gradient_divergence (legacy force, not default Poisson-based force) has:
    //   - Self-field artifacts at r <= r_eff ≈ 6.8 (wave-bounce near self-field edge)
    //   - Periodic boundary artifacts at r >= L/2 - r_eff ≈ 17 (image charges)
    // The clean zone is roughly r = 8..14. Check that forces decrease monotonically
    // in consecutive pairs where both radii are in this zone.
    int violations = 0;
    for (size_t i = 1; i < forces.size(); ++i) {
        if (radii[i-1] >= 8 && radii[i] <= 14 && forces[i] >= forces[i-1]) {
            ++violations;
        }
    }
    ftd::test::check("F1: Clean-zone force decreases with r (r=8..14)", violations <= 1);

    // F2: Force at r=4 > force at r=8
    ftd::test::check("F2: F(r=4) > F(r=8)", forces[1] > forces[3]);

    // F3: Force at r=2 is non-zero
    ftd::test::check("F3: F(r=2) > 0", forces[0] > 1e-30);

    // F4: Force at r=16 is non-zero (force reaches far)
    ftd::test::check("F4: F(r=16) > 0", forces.back() > 1e-30);

    // ---- Test 2: Power law fit ----
    std::cout << "\n  --- Power Law Fit ---\n";
    double sum_lr = 0, sum_lF = 0, sum_lr2 = 0, sum_lrlF = 0;
    int n = 0;
    for (size_t i = 0; i < radii.size(); ++i) {
        if (forces[i] < 1e-30) continue;
        double lr = std::log(static_cast<double>(radii[i]));
        double lF = std::log(forces[i]);
        sum_lr += lr; sum_lF += lF;
        sum_lr2 += lr * lr; sum_lrlF += lr * lF;
        n++;
    }

    double exponent = 0;
    if (n >= 3) {
        exponent = (n * sum_lrlF - sum_lr * sum_lF) /
                   (n * sum_lr2 - sum_lr * sum_lr);
        std::cout << std::defaultfloat << std::setprecision(4);
        std::cout << "  Measured exponent: " << exponent << "\n";
        std::cout << "  Expected (3D Coulomb): -2.0\n";
    }

    // Measured ~-3.8 (steeper than 3D Coulomb's -2.0 due to double gradient)
    ftd::test::check("F5: Power law exponent in [-5.0, -1.0]",
          n >= 3 && exponent >= -5.0 && exponent <= -1.0);

    // ---- Test 3: Isotropy at r=5 ----
    std::cout << "\n  --- Isotropy at r=5 ---\n";
    auto measure = [&](int dx, int dy, int dz) {
        int idx = engine.lattice().index(mid+dx, mid+dy, mid+dz);
        return engine.gradient_divergence(idx).mag();
    };

    double fx = measure(5, 0, 0);
    double fy = measure(0, 5, 0);
    double fz = measure(0, 0, 5);
    double f_avg = (fx + fy + fz) / 3.0;
    double f_min = std::min({fx, fy, fz});
    double f_max = std::max({fx, fy, fz});
    double isotropy = (f_max > 1e-30) ? f_min / f_max : 0.0;

    std::cout << std::setprecision(6) << std::scientific;
    std::cout << "  +x: " << fx << "\n";
    std::cout << "  +y: " << fy << "\n";
    std::cout << "  +z: " << fz << "\n";
    std::cout << std::defaultfloat << std::setprecision(4);
    std::cout << "  Isotropy (min/max): " << isotropy << "\n";

    // Measured ~0.40 — cubic lattice introduces significant anisotropy
    ftd::test::check("F6: Isotropy > 0.2 (within 5x)", isotropy > 0.2);

    // Body diagonal vs on-axis at similar distance
    // r=5 on-axis vs (3,3,3) = sqrt(27)=5.2 on diagonal
    double f_diag = measure(3, 3, 3);
    double diag_ratio = (f_avg > 1e-30) ? f_diag / f_avg : 0.0;
    std::cout << "  Diagonal (3,3,3) r=5.2: " << std::scientific << f_diag << "\n";
    std::cout << std::defaultfloat << "  Diagonal/axis ratio: " << diag_ratio << "\n";
    ftd::test::check("F7: Diagonal force is non-zero", f_diag > 1e-30);

    // ---- Summary ----
    std::cout << "\n================================================================\n";
    std::cout << "  Force Law section complete.\n";
    if (n >= 3) {
        std::cout << "  MEASURED FORCE LAW: |F| ~ r^(" << std::setprecision(3)
                  << exponent << ")\n";
    }
    std::cout << "================================================================\n";
}

// ============================================================================
// Section: poisson_force_law  (from campaign_poisson_force_law.cpp)
// ============================================================================

// Measure force magnitude at separation r using isolated two-particle simulation
static double measure_force_pfl(int r) {
    ftd::RenderBridge rb(48);
    int mid = 24;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
    rb.run(200);
    return rb.force_diag_at(mid + r, mid, mid).f_coulomb.mag();
}

static void section_poisson_force_law() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Poisson Force Law Profile (Phase 3)\n";
    std::cout << "================================================================\n";

    // Measure force at multiple distances
    std::vector<int> radii = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16};
    std::vector<double> forces;
    std::vector<double> log_r, log_f;

    std::cout << "\n--- Force Profile ---\n";
    std::cout << std::setw(6) << "r" << std::setw(15) << "F(r)" << "\n";
    for (int r : radii) {
        double f = measure_force_pfl(r);
        forces.push_back(f);
        std::cout << std::setw(6) << r << std::setw(15) << std::scientific
                  << std::setprecision(4) << f << "\n";
        if (f > 1e-20) {
            log_r.push_back(std::log(static_cast<double>(r)));
            log_f.push_back(std::log(f));
        }
    }

    // ================================================================
    // PF1: Force monotonically decreases
    // ================================================================
    std::cout << "\n--- Checks ---\n";
    {
        bool monotone = true;
        for (size_t i = 1; i < forces.size(); ++i) {
            if (forces[i] > forces[i - 1] * 1.05) {  // 5% tolerance
                monotone = false;
                std::cout << "    Non-monotone at r=" << radii[i] << "\n";
            }
        }
        ftd::test::check("PF1: Force monotonically decreases", monotone);
    }

    // ================================================================
    // PF2: F(r=4) > F(r=8)
    // ================================================================
    {
        double f4 = forces[2];  // index for r=4
        double f8 = forces[7];  // index for r=8 (radii[7]=9, actually need r=8)
        // Find the actual indices
        double f_r4 = 0, f_r8 = 0;
        for (size_t i = 0; i < radii.size(); ++i) {
            if (radii[i] == 4) f_r4 = forces[i];
            if (radii[i] == 8) f_r8 = forces[i];
        }
        ftd::test::check("PF2: F(r=4) > F(r=8)", f_r4 > f_r8);
    }

    // ================================================================
    // PF3: F(r=2) non-zero
    // ================================================================
    ftd::test::check("PF3: F(r=2) non-zero", forces[0] > 1e-15);

    // ================================================================
    // PF4: Long-range: F(r=16) > 1e-6 × F(r=2)
    // ================================================================
    {
        double f2 = forces[0];
        double f16 = forces.back();
        double ratio = (f2 > 1e-30) ? f16 / f2 : 0.0;
        std::cout << "    F(r=2)/F(r=16) ratio = " << ratio << "\n";
        ftd::test::check("PF4: F(r=16) > 1e-6 * F(r=2) (long range)", ratio > 1e-6);
    }

    // ================================================================
    // PF5: Power law exponent
    // ================================================================
    double exponent = 0.0;
    double r_squared = 0.0;
    {
        if (log_r.size() >= 3) {
            double xbar = 0, ybar = 0;
            for (size_t i = 0; i < log_r.size(); ++i) {
                xbar += log_r[i];
                ybar += log_f[i];
            }
            xbar /= log_r.size();
            ybar /= log_f.size();
            double num = 0, den = 0, ss_res = 0, ss_tot = 0;
            for (size_t i = 0; i < log_r.size(); ++i) {
                num += (log_r[i] - xbar) * (log_f[i] - ybar);
                den += (log_r[i] - xbar) * (log_r[i] - xbar);
            }
            exponent = (den > 1e-30) ? num / den : 0.0;
            double intercept = ybar - exponent * xbar;

            // Compute R²
            for (size_t i = 0; i < log_r.size(); ++i) {
                double y_pred = exponent * log_r[i] + intercept;
                ss_res += (log_f[i] - y_pred) * (log_f[i] - y_pred);
                ss_tot += (log_f[i] - ybar) * (log_f[i] - ybar);
            }
            r_squared = (ss_tot > 1e-30) ? 1.0 - ss_res / ss_tot : 0.0;
        }
        std::cout << "    Power law exponent = " << std::setprecision(3) << std::fixed
                  << exponent << "\n";
        std::cout << "    R² = " << std::setprecision(4) << r_squared << "\n";
        ftd::test::check("PF5: Exponent in [-2.8, -1.5]", exponent >= -2.8 && exponent <= -1.5);
    }

    // ================================================================
    // PF6: Isotropy at r=5
    // ================================================================
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        double kb3 = ftd::K_B / std::sqrt(3.0);
        rb.inject_particle(mid, mid, mid, +1, {kb3, kb3, kb3});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Probe on 3 axes
        rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 5, mid, mid)].locked = true;
        rb.inject_particle(mid, mid + 5, mid, -1, {0, -ftd::K_B, 0});
        rb.voxels()[rb.lattice().index(mid, mid + 5, mid)].locked = true;
        rb.inject_particle(mid, mid, mid + 5, -1, {-ftd::K_B, 0, 0});
        rb.voxels()[rb.lattice().index(mid, mid, mid + 5)].locked = true;

        rb.run(200);

        double fx = rb.force_diag_at(mid + 5, mid, mid).f_coulomb.mag();
        double fy = rb.force_diag_at(mid, mid + 5, mid).f_coulomb.mag();
        double fz = rb.force_diag_at(mid, mid, mid + 5).f_coulomb.mag();
        double fmax = std::max({fx, fy, fz});
        double fmin = std::min({fx, fy, fz});
        double isotropy = (fmax > 1e-30) ? fmin / fmax : 0.0;
        std::cout << "    Isotropy at r=5: " << std::setprecision(3) << isotropy << "\n";
        ftd::test::check("PF6: Isotropy > 0.5 at r=5", isotropy > 0.5);
    }

    // ================================================================
    // PF7: R² > 0.80
    // ================================================================
    ftd::test::check("PF7: Log-log fit R² > 0.80", r_squared > 0.80);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  Poisson force law section complete.\n";
    std::cout << "  Force law exponent: " << std::setprecision(3) << exponent
              << " (ideal: -2.0, legacy: -3.8)\n";
    std::cout << "  R²: " << std::setprecision(4) << r_squared << "\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// Section: poisson_binding  (from campaign_poisson_binding.cpp)
// ============================================================================

// Measure radial force component (positive = attractive toward center)
// Returns F.x on the right particle (at mid+r), which should be negative
// (pointing left, toward the source at mid) for attraction.
static double measure_radial_force_pb(int lattice_size, int separation, int8_t source_sign,
                                      int8_t probe_sign, int settle_ticks = 200) {
    ftd::RenderBridge rb(lattice_size);
    int mid = lattice_size / 2;
    rb.inject_particle(mid, mid, mid, source_sign, {0, 0, ftd::K_B * source_sign});
    rb.inject_particle(mid + separation, mid, mid, probe_sign,
                       {0, 0, ftd::K_B * probe_sign});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.voxels()[rb.lattice().index(mid + separation, mid, mid)].locked = true;
    rb.run(settle_ticks);
    return rb.force_diag_at(mid + separation, mid, mid).f_coulomb.x;
}

static void section_poisson_binding() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Poisson Binding (Phase 3) — 4 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // PB1: Opposite charges at r=2 attract
    // ================================================================
    std::cout << "\n--- PB1: Opposite at r=2 ---\n";
    {
        double fx = measure_radial_force_pb(32, 2, +1, -1);
        std::cout << "    F.x on -1 probe = " << fx << "\n";
        // Attractive: force on right -1 particle should point left (F.x < 0)
        ftd::test::check("PB1: Opposite charges at r=2 attract (F.x < 0)", fx < 0);
    }

    // ================================================================
    // PB2: Opposite charges at r=6 attract (was FAILING)
    // ================================================================
    std::cout << "\n--- PB2: Opposite at r=6 ---\n";
    {
        double fx = measure_radial_force_pb(32, 6, +1, -1);
        std::cout << "    F.x on -1 probe = " << fx << "\n";
        ftd::test::check("PB2: Opposite charges at r=6 attract (F.x < 0)", fx < 0);
    }

    // ================================================================
    // PB3: Same-sign at r=6 repel
    // ================================================================
    std::cout << "\n--- PB3: Same-sign at r=6 ---\n";
    {
        double fx = measure_radial_force_pb(32, 6, +1, +1);
        std::cout << "    F.x on +1 probe = " << fx << "\n";
        // Repulsive: force on right +1 particle should point right (F.x > 0)
        ftd::test::check("PB3: Same-sign at r=6 repel (F.x > 0)", fx > 0);
    }

    // ================================================================
    // PB4: Opposite charges at r=10 attract (NEW capability)
    // ================================================================
    std::cout << "\n--- PB4: Opposite at r=10 ---\n";
    {
        double fx = measure_radial_force_pb(48, 10, +1, -1, 300);
        std::cout << "    F.x on -1 probe = " << fx << "\n";
        ftd::test::check("PB4: Opposite charges at r=10 attract (F.x < 0)", fx < 0);
    }

    std::cout << "\n================================================================\n";
    std::cout << "  Poisson binding section complete.\n";
    std::cout << "================================================================\n";
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("campaign_coulomb_force_law");
    ftd::test::section("poisson_coulomb"); section_poisson_coulomb();
    ftd::test::section("coulomb_convergence"); section_coulomb_convergence();
    ftd::test::section("force_law"); section_force_law();
    ftd::test::section("poisson_force_law"); section_poisson_force_law();
    ftd::test::section("poisson_binding"); section_poisson_binding();
    return ftd::test::finalize();
}
