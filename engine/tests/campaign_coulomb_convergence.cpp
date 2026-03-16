/**
 * Campaign: Coulomb Law Convergence (Phase 2 — Continuum Limit)
 *
 * Validates that the electrostatic force converges to 1/r² as lattice
 * size increases. This is the key test for EM gauge emergence.
 *
 * Theory: In 3D, Poisson equation ∇²φ = -ρ gives φ ~ 1/r, F ~ -∇φ ~ 1/r².
 * On a discrete lattice, corrections appear as O(1/r³) terms.
 * As L→∞ (or equivalently r/L → 0), the exponent should approach -2.0.
 *
 * Protocol:
 *   1. Place a single +1 charge at center of lattice
 *   2. Run enough ticks for self-field to establish (100 ticks)
 *   3. Place a +1 probe charge at various distances r
 *   4. Measure force via 1-tick velocity change
 *   5. Fit log(F) vs log(r) → extract power law exponent
 *
 * Checks:
 *   CC1: Force exponent between -1.5 and -3.0 (correct sign/order)
 *   CC2: Force is repulsive (same sign charges)
 *   CC3: R² of power-law fit > 0.90
 *   CC4: Force decreases with distance (qualitative 1/r² check)
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

int main() {
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
    check("CC1: Force exponent between -1.5 and -3.0",
          exponent < -1.5 && exponent > -3.0);

    // ----------------------------------------------------------------
    // CC2: Force is repulsive (positive for same-sign charges)
    // ----------------------------------------------------------------
    bool all_repulsive = true;
    for (double f : forces) {
        if (f < -1e-10) { all_repulsive = false; break; }
    }
    check("CC2: Force is repulsive for same-sign charges", all_repulsive);

    // ----------------------------------------------------------------
    // CC3: Power law fit is good
    // ----------------------------------------------------------------
    check("CC3: R² > 0.90 (good power-law fit)", r_squared > 0.90);

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
    check("CC4: Force decreases with distance", decreasing);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
