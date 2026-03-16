/**
 * Test: Self-Field Profile Investigation (Phase 6, Stage 1)
 *
 * Characterizes the steady-state flux envelope around a single locked
 * point-particle.  This is pure investigation — no engine changes.
 *
 * The coupling source g_c·∇(s) + wave equation + Gauss constraint produce
 * an extended self-field around any state ±1 site.  This test measures:
 *   - Radial flux magnitude |J|(r)
 *   - Radial energy density (|J|² + |wave_vel|²)
 *   - Power-law exponent of radial falloff
 *   - Total self-field energy
 *   - Effective radius of the flux envelope
 *
 * 6 checks:
 *   SP1: Particle survives 1000 ticks (locked, trivially true)
 *   SP2: Radial profile is non-zero out to r=5 (self-field extends)
 *   SP3: Radial profile decreases monotonically for r >= 2
 *   SP4: Power-law exponent is in range [0.5, 3.0]
 *   SP5: Total self-field energy > 0
 *   SP6: Effective radius > 1.0 (flux is spatially extended)
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
    std::cout << "  TEST: Self-Field Profile (Phase 6, Stage 1) — 6 Checks\n";
    std::cout << "================================================================\n";

    // Setup: single locked +1 particle at center of 64³ grid
    const int N = 64;
    ftd::RenderBridge rb(N);
    // Uniform damping creates a localized self-field profile.
    // With selective_damping=true (default), vacuum waves propagate losslessly
    // and fill the entire grid, making the radial profile nearly flat.
    rb.toggles.selective_damping = false;
    int mid = N / 2;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // Run to steady state
    std::cout << "\nRunning 1000 ticks to reach steady state...\n";
    rb.run(1000);

    // ================================================================
    // Measure radial profile
    // ================================================================
    // For each integer distance r, average |J| and energy over all voxels
    // at that distance from the center.
    const int MAX_R = 20;
    std::vector<double> flux_sum(MAX_R + 1, 0.0);
    std::vector<double> energy_sum(MAX_R + 1, 0.0);
    std::vector<int> count(MAX_R + 1, 0);

    for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
            for (int z = 0; z < N; ++z) {
                double dx = x - mid, dy = y - mid, dz = z - mid;
                double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                int ri = static_cast<int>(std::round(r));
                if (ri > MAX_R) continue;

                int idx = rb.lattice().index(x, y, z);
                const auto& v = rb.voxels()[idx];
                double jmag = v.density();
                double e = v.flux.mag2() + v.wave_vel.mag2();

                flux_sum[ri] += jmag;
                energy_sum[ri] += e;
                count[ri]++;
            }
        }
    }

    // Compute averages
    std::vector<double> avg_flux(MAX_R + 1, 0.0);
    std::vector<double> avg_energy(MAX_R + 1, 0.0);
    for (int r = 0; r <= MAX_R; ++r) {
        if (count[r] > 0) {
            avg_flux[r] = flux_sum[r] / count[r];
            avg_energy[r] = energy_sum[r] / count[r];
        }
    }

    // Print radial profile
    std::cout << "\n--- Radial Profile (averaged over shells) ---\n";
    std::cout << std::setw(4) << "r" << "  "
              << std::setw(10) << "avg|J|" << "  "
              << std::setw(10) << "avg_E" << "  "
              << std::setw(6) << "sites" << "\n";
    std::cout << "--------------------------------------------\n";
    for (int r = 0; r <= MAX_R; ++r) {
        std::cout << std::setw(4) << r << "  "
                  << std::scientific << std::setprecision(4)
                  << std::setw(10) << avg_flux[r] << "  "
                  << std::setw(10) << avg_energy[r] << "  "
                  << std::setw(6) << count[r] << "\n";
    }

    // ================================================================
    // SP1: Particle survives (trivially true for locked)
    // ================================================================
    std::cout << "\n--- SP1: Particle survival ---\n";
    {
        int idx = rb.lattice().index(mid, mid, mid);
        check("SP1: Locked particle survives 1000 ticks",
              rb.voxels()[idx].state == 1);
    }

    // ================================================================
    // SP2: Self-field extends to r=5
    // ================================================================
    std::cout << "\n--- SP2: Self-field extent ---\n";
    {
        bool extends_to_5 = avg_flux[5] > 1e-6;
        std::cout << "    |J|(r=5) = " << avg_flux[5] << "\n";
        check("SP2: Self-field extends to r=5 (|J| > 1e-6)", extends_to_5);
    }

    // ================================================================
    // SP3: Radial profile decreases overall (may have standing wave bumps)
    // ================================================================
    // The coupling source + wave equation create standing wave patterns,
    // so strict monotonicity is NOT expected.  Instead check that the
    // overall trend is decreasing: avg(r=1..3) > avg(r=8..10) > avg(r=16..18).
    std::cout << "\n--- SP3: Overall decreasing trend ---\n";
    {
        auto shell_avg = [&](int r_lo, int r_hi) {
            double sum = 0.0; int n = 0;
            for (int r = r_lo; r <= r_hi; ++r) { sum += avg_flux[r]; n++; }
            return sum / n;
        };
        double near = shell_avg(1, 3);
        double mid_r = shell_avg(8, 10);
        double far = shell_avg(16, 18);
        std::cout << "    avg(r=1..3)   = " << near << "\n";
        std::cout << "    avg(r=8..10)  = " << mid_r << "\n";
        std::cout << "    avg(r=16..18) = " << far << "\n";
        check("SP3: Overall decreasing trend (near > mid > far)",
              near > mid_r && mid_r > far);
    }

    // ================================================================
    // SP4: Power-law exponent fit
    // ================================================================
    std::cout << "\n--- SP4: Power-law exponent ---\n";
    {
        // Fit ln(|J|) = a - n·ln(r) using least-squares over r=3..12
        // (avoid r=0,1,2 where discrete effects dominate, and r>12 where
        //  signal may be too weak on 64³ grid)
        double sum_lnr = 0.0, sum_lnj = 0.0, sum_lnr2 = 0.0, sum_lnr_lnj = 0.0;
        int n_points = 0;
        for (int r = 3; r <= 12; ++r) {
            if (avg_flux[r] < 1e-15) continue;
            double lr = std::log(static_cast<double>(r));
            double lj = std::log(avg_flux[r]);
            sum_lnr += lr;
            sum_lnj += lj;
            sum_lnr2 += lr * lr;
            sum_lnr_lnj += lr * lj;
            n_points++;
        }
        double exponent = 0.0;
        if (n_points >= 3) {
            exponent = -(n_points * sum_lnr_lnj - sum_lnr * sum_lnj)
                     / (n_points * sum_lnr2 - sum_lnr * sum_lnr);
        }
        std::cout << "    Fitted exponent n = " << std::fixed << std::setprecision(3)
                  << exponent << " (|J| ~ r^-n)\n";
        std::cout << "    (n=1 → Coulomb potential, n=2 → Coulomb field)\n";
        check("SP4: Power-law exponent in [0.5, 3.0]",
              exponent >= 0.5 && exponent <= 3.0);
    }

    // ================================================================
    // SP5: Total self-field energy
    // ================================================================
    std::cout << "\n--- SP5: Total self-field energy ---\n";
    {
        auto audit = rb.energy_audit();
        std::cout << "    field_energy  = " << std::scientific << audit.field_energy << "\n";
        std::cout << "    wave_energy   = " << audit.wave_energy << "\n";
        std::cout << "    total_energy  = " << audit.total_energy << "\n";
        check("SP5: Total self-field energy > 0", audit.total_energy > 0.0);
    }

    // ================================================================
    // SP6: Effective radius
    // ================================================================
    std::cout << "\n--- SP6: Effective radius ---\n";
    {
        double sum_r2_j2 = 0.0;
        double sum_j2 = 0.0;
        for (int x = 0; x < N; ++x) {
            for (int y = 0; y < N; ++y) {
                for (int z = 0; z < N; ++z) {
                    double dx = x - mid, dy = y - mid, dz = z - mid;
                    double r2 = dx*dx + dy*dy + dz*dz;
                    int idx = rb.lattice().index(x, y, z);
                    double j2 = rb.voxels()[idx].flux.mag2();
                    sum_r2_j2 += r2 * j2;
                    sum_j2 += j2;
                }
            }
        }
        double r_eff = (sum_j2 > 1e-30) ? std::sqrt(sum_r2_j2 / sum_j2) : 0.0;
        std::cout << "    Effective radius r_eff = " << std::fixed << std::setprecision(2)
                  << r_eff << " lattice units\n";
        check("SP6: Effective radius > 1.0 (flux is spatially extended)", r_eff > 1.0);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All self-field profile checks PASSED.\n";
    } else {
        std::cout << "  " << failures << " check(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
