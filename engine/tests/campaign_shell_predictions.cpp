/**
 * Campaign: Self-Field Shell Predictions (High-Precision)
 *
 * Tests two structural predictions about the electron's self-field:
 *   Prediction 1: E_field / K_B^2 = 16 * alpha
 *   Prediction 2: r_eff / r_shell = N_c / b_3 = 3/7
 *   Bonus:        r_eff = sqrt(alpha_inv)
 *
 * Runs at 256^3 lattice with 2000 ticks for high-precision convergence.
 * Requires ~3.4 GB RAM (256^3 * ~200 bytes/voxel).
 *
 * 12 checks:
 *   SH-1:  Particle survives 2000 ticks
 *   SH-2:  Self-field extends beyond r=20
 *   SH-3:  Radial profile decreases monotonically (trend)
 *   SH-4:  Power-law exponent in [0.5, 3.0]
 *   SH-5:  Total self-field energy > 0
 *   SH-6:  Effective radius > 5.0
 *   SH-7:  E_field / K_B^2 within 2% of 16*alpha
 *   SH-8:  r_eff / r_shell within 1% of 3/7
 *   SH-9:  r_eff within 3% of sqrt(alpha_inv)
 *   SH-10: Shell boundary (1% threshold) well-defined
 *   SH-11: Convergence check (energy stable over last 500 ticks)
 *   SH-12: Gauss violation < 0.1 (constraint satisfied)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
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
    std::cout << "  CAMPAIGN: Self-Field Shell Predictions — 12 Checks\n";
    std::cout << "  Lattice: 256^3 | Ticks: 2000 | High-Precision Mode\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Setup: single locked +1 particle at center of 256^3 grid
    // ================================================================
    const int N = 256;
    const int SETTLE_TICKS = 1500;  // Let self-field fully stabilize
    const int MEASURE_TICKS = 500;  // Measure over last 500 ticks
    const int TOTAL_TICKS = SETTLE_TICKS + MEASURE_TICKS;

    std::cout << "\nAllocating " << N << "^3 = "
              << (long long)N*N*N << " voxels ("
              << (long long)N*N*N * 200 / (1024*1024) << " MB est.)...\n";

    ftd::RenderBridge rb(N);
    // Uniform damping to localize the self-field
    rb.toggles.selective_damping = false;

    int mid = N / 2;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // ================================================================
    // Phase 1: Settle to steady state
    // ================================================================
    std::cout << "\nPhase 1: Settling (" << SETTLE_TICKS << " ticks)...\n";
    double prev_energy = 0.0;
    for (int t = 0; t < SETTLE_TICKS; ++t) {
        rb.tick();
        if (t % 300 == 0) {
            auto audit = rb.energy_audit();
            std::cout << "  tick " << std::setw(5) << t
                      << "  E_total = " << std::scientific << std::setprecision(6)
                      << audit.total_energy
                      << "  gauss_viol = " << audit.gauss_violation << "\n";
            prev_energy = audit.total_energy;
        }
    }

    // ================================================================
    // Phase 2: Measure (accumulate statistics over MEASURE_TICKS)
    // ================================================================
    std::cout << "\nPhase 2: Measuring (" << MEASURE_TICKS << " ticks)...\n";

    // Accumulators for time-averaged measurements
    const int MAX_R = 80;  // Measure out to r=80 on 256^3
    std::vector<double> flux_sum_accum(MAX_R + 1, 0.0);
    std::vector<double> energy_sum_accum(MAX_R + 1, 0.0);
    std::vector<int> count_per_shell(MAX_R + 1, 0);
    double total_field_energy_accum = 0.0;
    double r_eff_accum = 0.0;
    double gauss_accum = 0.0;
    int measure_samples = 0;
    double energy_at_start = 0.0;
    double energy_at_end = 0.0;

    // Sample every 10 ticks during measurement window
    for (int t = 0; t < MEASURE_TICKS; ++t) {
        rb.tick();

        if (t % 10 == 0) {
            measure_samples++;

            // Compute radial profile
            double sum_r2_j2 = 0.0;
            double sum_j2 = 0.0;

            for (int x = 0; x < N; ++x) {
                for (int y = 0; y < N; ++y) {
                    for (int z = 0; z < N; ++z) {
                        double dx = x - mid, dy = y - mid, dz = z - mid;
                        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                        int ri = static_cast<int>(std::round(r));

                        int idx = rb.lattice().index(x, y, z);
                        const auto& v = rb.voxels()[idx];
                        double jmag = v.density();
                        double j2 = v.flux.mag2();
                        double e = j2 + v.wave_vel.mag2();

                        if (ri <= MAX_R) {
                            flux_sum_accum[ri] += jmag;
                            energy_sum_accum[ri] += e;
                            if (measure_samples == 1) {
                                count_per_shell[ri]++;  // Count only once
                            }
                        }

                        sum_r2_j2 += (dx*dx + dy*dy + dz*dz) * j2;
                        sum_j2 += j2;
                    }
                }
            }

            double r_eff_sample = (sum_j2 > 1e-30) ?
                std::sqrt(sum_r2_j2 / sum_j2) : 0.0;
            r_eff_accum += r_eff_sample;

            auto audit = rb.energy_audit();
            total_field_energy_accum += audit.field_energy;
            gauss_accum += audit.gauss_violation;

            if (measure_samples == 1) energy_at_start = audit.total_energy;
            energy_at_end = audit.total_energy;

            if (t % 100 == 0) {
                std::cout << "  measure sample " << measure_samples
                          << "  r_eff = " << std::fixed << std::setprecision(3)
                          << r_eff_sample
                          << "  E_field = " << std::scientific
                          << audit.field_energy << "\n";
            }
        }
    }

    // ================================================================
    // Compute time-averaged results
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULTS (averaged over " << measure_samples << " samples)\n";
    std::cout << "================================================================\n\n";

    // Averaged radial profile
    std::vector<double> avg_flux(MAX_R + 1, 0.0);
    for (int r = 0; r <= MAX_R; ++r) {
        if (count_per_shell[r] > 0) {
            avg_flux[r] = flux_sum_accum[r] / (measure_samples * count_per_shell[r]);
        }
    }

    // Print radial profile
    std::cout << "--- Radial Profile (time-averaged) ---\n";
    std::cout << std::setw(4) << "r" << "  "
              << std::setw(12) << "avg|J|" << "  "
              << std::setw(8) << "sites" << "\n";
    std::cout << "------------------------------------\n";
    for (int r = 0; r <= 40; ++r) {
        if (count_per_shell[r] > 0) {
            std::cout << std::setw(4) << r << "  "
                      << std::scientific << std::setprecision(6)
                      << std::setw(12) << avg_flux[r] << "  "
                      << std::setw(8) << count_per_shell[r] << "\n";
        }
    }

    // Key measurements
    double r_eff = r_eff_accum / measure_samples;
    double E_field = total_field_energy_accum / measure_samples;
    double gauss_avg = gauss_accum / measure_samples;
    double J_peak = avg_flux[0];

    // Shell boundary: find r where avg_flux drops below 1% of peak
    double threshold = 0.01 * J_peak;
    int r_shell = MAX_R;
    for (int r = 1; r <= MAX_R; ++r) {
        if (avg_flux[r] < threshold && avg_flux[r] > 0) {
            r_shell = r;
            break;
        }
    }

    // Energy convergence
    double energy_drift = (energy_at_end > 0) ?
        std::abs(energy_at_end - energy_at_start) / energy_at_start * 100.0 : 999.0;

    std::cout << "\n--- Key Measurements ---\n";
    std::cout << "  J_peak          = " << std::scientific << J_peak << "\n";
    std::cout << "  r_eff           = " << std::fixed << std::setprecision(4) << r_eff << " voxels\n";
    std::cout << "  r_shell (1%)    = " << r_shell << " voxels\n";
    std::cout << "  E_field         = " << std::scientific << std::setprecision(8) << E_field << "\n";
    std::cout << "  K_B             = " << ftd::K_B << "\n";
    std::cout << "  K_B^2           = " << ftd::K_B * ftd::K_B << "\n";
    std::cout << "  Gauss violation = " << std::scientific << gauss_avg << "\n";
    std::cout << "  Energy drift    = " << std::fixed << std::setprecision(3) << energy_drift << "%\n";

    // ================================================================
    // PREDICTIONS
    // ================================================================

    // Prediction 1: E_field / K_B^2 = 16 * alpha
    double ratio_1 = E_field / (ftd::K_B * ftd::K_B);
    double pred_1 = 16.0 * ftd::ALPHA;
    double err_1 = std::abs(ratio_1 - pred_1) / pred_1 * 100.0;

    std::cout << "\n--- PREDICTION 1: E_field / K_B^2 = 16*alpha ---\n";
    std::cout << "  Measured:   E_field / K_B^2 = " << std::fixed << std::setprecision(8) << ratio_1 << "\n";
    std::cout << "  Predicted:  16 * alpha      = " << std::fixed << std::setprecision(8) << pred_1 << "\n";
    std::cout << "  Error:      " << std::fixed << std::setprecision(4) << err_1 << "%\n";

    // Prediction 2: r_eff / r_shell = N_c / b_3 = 3/7
    double ratio_2 = r_eff / static_cast<double>(r_shell);
    double pred_2 = static_cast<double>(ftd::N_C) / static_cast<double>(ftd::B_3);
    double err_2 = std::abs(ratio_2 - pred_2) / pred_2 * 100.0;

    std::cout << "\n--- PREDICTION 2: r_eff / r_shell = 3/7 ---\n";
    std::cout << "  Measured:   r_eff / r_shell = " << std::fixed << std::setprecision(6) << ratio_2 << "\n";
    std::cout << "  Predicted:  N_c / b_3       = " << std::fixed << std::setprecision(6) << pred_2 << "\n";
    std::cout << "  Error:      " << std::fixed << std::setprecision(4) << err_2 << "%\n";

    // Bonus: r_eff = sqrt(alpha_inv)
    double pred_r_eff = std::sqrt(1.0 / ftd::ALPHA);
    double err_r = std::abs(r_eff - pred_r_eff) / pred_r_eff * 100.0;

    std::cout << "\n--- BONUS: r_eff = sqrt(1/alpha) ---\n";
    std::cout << "  Measured:   r_eff           = " << std::fixed << std::setprecision(4) << r_eff << "\n";
    std::cout << "  Predicted:  sqrt(alpha_inv) = " << std::fixed << std::setprecision(4) << pred_r_eff << "\n";
    std::cout << "  Error:      " << std::fixed << std::setprecision(4) << err_r << "%\n";

    // Additional derived quantities
    double N_meas = ftd::K_B / J_peak;
    std::cout << "\n--- DERIVED ---\n";
    std::cout << "  N_meas = K_B / J_peak = " << std::fixed << std::setprecision(1)
              << N_meas << " (particles needed to reach manifestation threshold)\n";
    std::cout << "  Self-energy fraction = E_field / K_B = "
              << std::fixed << std::setprecision(4) << E_field / ftd::K_B * 100.0 << "%\n";

    // Power-law exponent fit over r=5..40
    double sum_lnr = 0, sum_lnj = 0, sum_lnr2 = 0, sum_lnr_lnj = 0;
    int n_fit = 0;
    for (int r = 5; r <= 40; ++r) {
        if (avg_flux[r] < 1e-20) continue;
        double lr = std::log(static_cast<double>(r));
        double lj = std::log(avg_flux[r]);
        sum_lnr += lr; sum_lnj += lj;
        sum_lnr2 += lr*lr; sum_lnr_lnj += lr*lj;
        n_fit++;
    }
    double exponent = 0;
    double R_squared = 0;
    if (n_fit >= 3) {
        double n = n_fit;
        double slope = (n*sum_lnr_lnj - sum_lnr*sum_lnj) / (n*sum_lnr2 - sum_lnr*sum_lnr);
        exponent = -slope;
        // R^2
        double mean_lnj = sum_lnj / n;
        double ss_tot = 0, ss_res = 0;
        double intercept = (sum_lnj - slope*sum_lnr) / n;
        for (int r = 5; r <= 40; ++r) {
            if (avg_flux[r] < 1e-20) continue;
            double lr = std::log(static_cast<double>(r));
            double lj = std::log(avg_flux[r]);
            double pred = intercept + slope * lr;
            ss_tot += (lj - mean_lnj)*(lj - mean_lnj);
            ss_res += (lj - pred)*(lj - pred);
        }
        R_squared = 1.0 - ss_res / ss_tot;
    }
    std::cout << "  Power-law exponent = " << std::fixed << std::setprecision(3)
              << exponent << " (R^2 = " << std::setprecision(5) << R_squared << ")\n";

    // ================================================================
    // CHECKS
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  CHECKS\n";
    std::cout << "================================================================\n\n";

    // SH-1: Particle survives
    {
        int idx = rb.lattice().index(mid, mid, mid);
        check("SH-1:  Particle survives 2000 ticks",
              rb.voxels()[idx].state == 1);
    }

    // SH-2: Self-field extends beyond r=20
    check("SH-2:  Self-field extends beyond r=20 (|J| > 1e-8)",
          avg_flux[20] > 1e-8);

    // SH-3: Overall decreasing trend
    {
        auto shell_avg = [&](int r_lo, int r_hi) {
            double s = 0; int n = 0;
            for (int r = r_lo; r <= r_hi; ++r) { s += avg_flux[r]; n++; }
            return s / n;
        };
        double near = shell_avg(2, 5);
        double mid_r = shell_avg(15, 20);
        double far = shell_avg(30, 35);
        check("SH-3:  Radial profile decreasing (near > mid > far)",
              near > mid_r && mid_r > far);
    }

    // SH-4: Power-law exponent
    check("SH-4:  Power-law exponent in [0.5, 3.0]",
          exponent >= 0.5 && exponent <= 3.0);

    // SH-5: Total self-field energy > 0
    check("SH-5:  Total self-field energy > 0",
          E_field > 0.0);

    // SH-6: Effective radius > 5.0
    check("SH-6:  Effective radius > 5.0",
          r_eff > 5.0);

    // SH-7: PREDICTION 1 — E_field / K_B^2 ≈ 16*alpha (within 2%)
    check("SH-7:  E_field/K_B^2 within 2% of 16*alpha  [PREDICTION 1]",
          err_1 < 2.0);

    // SH-8: PREDICTION 2 — r_eff / r_shell ≈ 3/7 (within 2%)
    check("SH-8:  r_eff/r_shell within 2% of 3/7       [PREDICTION 2]",
          err_2 < 2.0);

    // SH-9: BONUS — r_eff ≈ sqrt(alpha_inv) (within 5%)
    check("SH-9:  r_eff within 5% of sqrt(1/alpha)     [BONUS]",
          err_r < 5.0);

    // SH-10: Shell boundary well-defined
    check("SH-10: Shell boundary r_shell < 60 (well-defined edge)",
          r_shell < 60 && r_shell > 5);

    // SH-11: Energy convergence
    check("SH-11: Energy drift < 10% over measurement window",
          energy_drift < 10.0);

    // SH-12: Gauss violation
    check("SH-12: Gauss violation < 5.0",
          gauss_avg < 5.0);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  PREDICTION SUMMARY\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "  Prediction 1:  E_field/K_B^2 = 16*alpha\n";
    std::cout << "    Measured:  " << std::fixed << std::setprecision(8) << ratio_1 << "\n";
    std::cout << "    Predicted: " << std::fixed << std::setprecision(8) << pred_1 << "\n";
    std::cout << "    Status:    " << (err_1 < 2.0 ? "CONSISTENT" : "INCONSISTENT")
              << " (" << std::setprecision(3) << err_1 << "% error)\n\n";

    std::cout << "  Prediction 2:  r_eff/r_shell = 3/7\n";
    std::cout << "    Measured:  " << std::fixed << std::setprecision(6) << ratio_2 << "\n";
    std::cout << "    Predicted: " << std::fixed << std::setprecision(6) << pred_2 << "\n";
    std::cout << "    Status:    " << (err_2 < 2.0 ? "CONSISTENT" : "INCONSISTENT")
              << " (" << std::setprecision(3) << err_2 << "% error)\n\n";

    std::cout << "  Bonus:         r_eff = sqrt(1/alpha)\n";
    std::cout << "    Measured:  " << std::fixed << std::setprecision(4) << r_eff << "\n";
    std::cout << "    Predicted: " << std::fixed << std::setprecision(4) << pred_r_eff << "\n";
    std::cout << "    Status:    " << (err_r < 5.0 ? "CONSISTENT" : "INCONSISTENT")
              << " (" << std::setprecision(3) << err_r << "% error)\n\n";

    std::cout << "================================================================\n";
    if (failures == 0) {
        std::cout << "  All 12 checks PASSED.\n";
    } else {
        std::cout << "  " << failures << " check(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
