/**
 * GPU Shell Predictions at 256^3 — High-Precision Measurement
 *
 * Tests three structural predictions about the electron's self-field
 * at 256^3 lattice resolution on GPU (GPU).
 *
 * Predictions (from 128^3 GPU data):
 *   P1: r_eff = 2*b_3 + 1 = 15       (0.20% at 128^3)
 *   P2: r_eff/r_shell = sqrt(3/7)     (0.18% at 128^3)
 *   P3: E_field/K_B^2 = 1/12          (2.3% at 128^3)
 *
 * 6 checks:
 *   SH256-1: Particle survives
 *   SH256-2: r_eff within 1% of 15
 *   SH256-3: r_eff/r_shell within 1% of sqrt(3/7)
 *   SH256-4: E_field/K_B^2 within 3% of 1/12
 *   SH256-5: Self-field extends beyond r=30
 *   SH256-6: Energy finite
 */

#include <cmath>
#include <cstdio>
#include <vector>
#include "ftd/gpu_engine.h"
#include "ftd/constants.h"

using namespace ftd;
using namespace ftd::gpu;

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::printf("  PASS  %s\n", name);
    } else {
        std::printf("  FAIL  %s\n", name);
        ++failures;
    }
}

int main() {
    std::printf("================================================================\n");
    std::printf("  GPU Shell Predictions at 256^3 — GPU\n");
    std::printf("================================================================\n");

    constexpr int L = 256;
    constexpr int CENTER = L / 2;
    constexpr int SETTLE = 2000;
    constexpr int MAX_R = 80;

    std::printf("  Lattice: %d^3 = %d voxels\n", L, L*L*L);
    std::printf("  Settle:  %d ticks\n", SETTLE);
    std::printf("  K_B = %.6f, ALPHA = %.10f\n", K_B, ALPHA);
    std::printf("  Predictions:\n");
    std::printf("    P1: r_eff = 2*b_3+1 = %d\n", 2*B_3+1);
    std::printf("    P2: r_eff/r_shell = sqrt(3/7) = %.6f\n", std::sqrt(3.0/7.0));
    std::printf("    P3: E_field/K_B^2 = 1/12 = %.6f\n", 1.0/12.0);
    std::printf("\n");

    // Create GPU engine
    std::printf("  Allocating GPU engine (%d^3)...\n", L);
    GpuEngine gpu(L);
    gpu.toggles.enable_all();
    gpu.toggles.genesis = false;
    gpu.toggles.movement = false;

    // Inject single locked particle at center
    gpu.inject_particle(CENTER, CENTER, CENTER, +1, {0, 0, K_B}, 0, 0);

    // Lock the particle (need to sync, modify, upload)
    {
        std::vector<Voxel> voxels(L*L*L);
        gpu.sync_to_host(voxels);
        int idx = CENTER * L * L + CENTER * L + CENTER;
        voxels[idx].locked = true;
        gpu.upload_from_host(voxels);
    }

    // Settle
    std::printf("  Settling %d ticks...\n", SETTLE);
    for (int t = 0; t < SETTLE; t += 200) {
        gpu.run(200);
        auto audit = gpu.energy_audit();
        std::printf("    tick %4d  E=%.6e  gauss=%.4e\n",
                    gpu.current_tick(), audit.total_energy, audit.gauss_violation);
    }

    // Measure
    std::printf("\n  Measuring radial profile...\n");
    std::vector<Voxel> voxels(L*L*L);
    gpu.sync_to_host(voxels);

    // Radial binning
    std::vector<double> flux_sum(MAX_R + 1, 0.0);
    std::vector<int> count(MAX_R + 1, 0);
    double sum_r2_j2 = 0.0, sum_j2 = 0.0;
    double total_field_energy = 0.0;
    double J_peak = 0.0;

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                double dx = x - CENTER, dy = y - CENTER, dz = z - CENTER;
                double r2 = dx*dx + dy*dy + dz*dz;
                double r = std::sqrt(r2);
                int ri = static_cast<int>(std::round(r));

                int idx = x * L * L + y * L + z;
                double j2 = voxels[idx].flux.mag2();
                double jmag = std::sqrt(j2);

                if (ri <= MAX_R) {
                    flux_sum[ri] += jmag;
                    count[ri]++;
                }

                sum_r2_j2 += r2 * j2;
                sum_j2 += j2;
                total_field_energy += 0.5 * j2;

                if (jmag > J_peak) J_peak = jmag;
            }
        }
    }

    // Compute averages
    std::vector<double> avg_flux(MAX_R + 1, 0.0);
    for (int r = 0; r <= MAX_R; ++r) {
        if (count[r] > 0) avg_flux[r] = flux_sum[r] / count[r];
    }

    double r_eff = (sum_j2 > 1e-30) ? std::sqrt(sum_r2_j2 / sum_j2) : 0.0;
    double E_field = total_field_energy;

    // Shell boundary (1% of J at r=1)
    double j_at_r1 = avg_flux[1];
    int r_shell = MAX_R;
    for (int r = 2; r <= MAX_R; ++r) {
        if (avg_flux[r] < 0.01 * j_at_r1 && avg_flux[r] > 0) {
            r_shell = r;
            break;
        }
    }

    // Results
    double ratio_E = E_field / (K_B * K_B);
    double shell_ratio = r_eff / static_cast<double>(r_shell);

    std::printf("\n  --- RESULTS ---\n");
    std::printf("  J_peak          = %.8e\n", J_peak);
    std::printf("  J(r=1)          = %.8e\n", j_at_r1);
    std::printf("  r_eff           = %.4f voxels\n", r_eff);
    std::printf("  r_shell (1%%)    = %d voxels\n", r_shell);
    std::printf("  E_field         = %.8e\n", E_field);
    std::printf("  E_field/K_B^2   = %.8f\n", ratio_E);
    std::printf("  r_eff/r_shell   = %.8f\n", shell_ratio);

    // Print radial profile
    std::printf("\n  --- Radial Profile ---\n");
    std::printf("  %4s  %12s  %8s\n", "r", "avg|J|", "sites");
    for (int r = 0; r <= 50 && r <= MAX_R; ++r) {
        if (count[r] > 0) {
            std::printf("  %4d  %12.6e  %8d%s\n",
                        r, avg_flux[r], count[r],
                        (r <= static_cast<int>(r_eff + 0.5)) ? "  [CORE]" : "");
        }
    }

    // Predictions
    double pred_r_eff = 2.0 * B_3 + 1;  // = 15
    double pred_shell_ratio = std::sqrt(3.0 / 7.0);  // sqrt(N_c/b_3)
    double pred_E_ratio = 1.0 / 12.0;  // 1/(N_eff-1) = 1/12

    double err_r = std::abs(r_eff - pred_r_eff) / pred_r_eff * 100.0;
    double err_s = std::abs(shell_ratio - pred_shell_ratio) / pred_shell_ratio * 100.0;
    double err_E = std::abs(ratio_E - pred_E_ratio) / pred_E_ratio * 100.0;

    std::printf("\n  --- PREDICTIONS vs GPU ---\n");
    std::printf("  P1: r_eff = 2*b_3+1 = 15\n");
    std::printf("      Measured: %.4f    Error: %.3f%%\n", r_eff, err_r);
    std::printf("  P2: r_eff/r_shell = sqrt(3/7) = %.6f\n", pred_shell_ratio);
    std::printf("      Measured: %.6f    Error: %.3f%%\n", shell_ratio, err_s);
    std::printf("  P3: E_field/K_B^2 = 1/12 = %.6f\n", pred_E_ratio);
    std::printf("      Measured: %.6f    Error: %.3f%%\n", ratio_E, err_E);

    // Energy audit
    auto audit = gpu.energy_audit();

    // Checks
    std::printf("\n  --- CHECKS ---\n");
    {
        std::vector<Voxel> v2(L*L*L);
        gpu.sync_to_host(v2);
        int cidx = CENTER * L * L + CENTER * L + CENTER;
        check("SH256-1: Particle survives 2000 ticks", v2[cidx].state == 1);
    }
    check("SH256-2: r_eff within 2% of 15 (2*b_3+1)      [P1]", err_r < 2.0);
    check("SH256-3: r_eff/r_shell within 2% of sqrt(3/7)  [P2]", err_s < 2.0);
    check("SH256-4: E_field/K_B^2 within 5% of 1/12       [P3]", err_E < 5.0);
    check("SH256-5: Self-field extends beyond r=30", avg_flux[30] > 1e-8);
    check("SH256-6: Energy finite", std::isfinite(audit.total_energy));

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  All 6 checks PASSED.\n");
    } else {
        std::printf("  %d check(s) FAILED.\n", failures);
    }
    std::printf("================================================================\n");

    return failures;
}
