/**
 * Campaign: Poisson Force Law Profile (Phase 3)
 *
 * Re-runs Phase 2 force law characterization with Poisson-based Coulomb.
 * THE key experiment: does the Poisson solver produce proper 1/r² force?
 *
 * Setup: Single +1 source, single -1 probe at various distances.
 *        Each distance run independently (no multi-charge contamination).
 *        48³ lattice, 200 ticks settling time.
 *
 * 7 checks:
 *   PF1: Force monotonically decreases
 *   PF2: F(r=4) > F(r=8)
 *   PF3: F(r=2) non-zero
 *   PF4: Long-range: F(r=16) > 1e-6 × F(r=2)
 *   PF5: Power law exponent in [-2.8, -1.5]
 *   PF6: Isotropy at r=5 > 0.5
 *   PF7: Log-log fit R² > 0.80 (clean power law)
 *
 * Theory references:
 *   - SPEC_ENGINE.md Phase 3: Poisson Coulomb force
 *   - Plan: Deliverable 6 — campaign_poisson_force_law
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

// Measure force magnitude at separation r using isolated two-particle simulation
double measure_force(int r) {
    ftd::RenderBridge rb(48);
    int mid = 24;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
    rb.run(200);
    return rb.force_diag_at(mid + r, mid, mid).f_coulomb.mag();
}

int main() {
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
        double f = measure_force(r);
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
        check("PF1: Force monotonically decreases", monotone);
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
        check("PF2: F(r=4) > F(r=8)", f_r4 > f_r8);
    }

    // ================================================================
    // PF3: F(r=2) non-zero
    // ================================================================
    check("PF3: F(r=2) non-zero", forces[0] > 1e-15);

    // ================================================================
    // PF4: Long-range: F(r=16) > 1e-6 × F(r=2)
    // ================================================================
    {
        double f2 = forces[0];
        double f16 = forces.back();
        double ratio = (f2 > 1e-30) ? f16 / f2 : 0.0;
        std::cout << "    F(r=2)/F(r=16) ratio = " << ratio << "\n";
        check("PF4: F(r=16) > 1e-6 * F(r=2) (long range)", ratio > 1e-6);
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
        check("PF5: Exponent in [-2.8, -1.5]", exponent >= -2.8 && exponent <= -1.5);
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
        check("PF6: Isotropy > 0.5 at r=5", isotropy > 0.5);
    }

    // ================================================================
    // PF7: R² > 0.80
    // ================================================================
    check("PF7: Log-log fit R² > 0.80", r_squared > 0.80);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULTS: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED") << "\n";
    std::cout << "  Force law exponent: " << std::setprecision(3) << exponent
              << " (ideal: -2.0, legacy: -3.8)\n";
    std::cout << "  R²: " << std::setprecision(4) << r_squared << "\n";
    std::cout << "  Failures: " << failures << "\n";
    std::cout << "================================================================\n";

    return failures;
}
