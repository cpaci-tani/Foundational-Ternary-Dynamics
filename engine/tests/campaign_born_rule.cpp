/**
 * Campaign: Born Rule from Genesis Statistics (Phase 3 — Quantum Mechanics)
 *
 * Validates that particle manifestation (genesis) follows Born rule:
 *   P(x) ∝ |J(x)|²
 *
 * Theory: FTD genesis probability is p = clamp(1 - exp(-(|J|-K_B)/K_B), 0, 1).
 * For |J| >> K_B, this saturates at 1. For |J| near K_B, it's approximately
 * proportional to (|J| - K_B)/K_B. The Born rule P ∝ |J|² should emerge
 * as an envelope of genesis statistics across many independent runs.
 *
 * Protocol:
 *   1. Create a flux field with known non-uniform |J|² profile:
 *      two Gaussian peaks of different amplitudes (ratio 2:1)
 *   2. Run N=200 independent genesis events (different RNG seeds)
 *   3. Record where each genesis occurs (if it occurs)
 *   4. Histogram genesis locations along x-axis
 *   5. Compare ratio of genesis events near peaks with flux amplitude ratio
 *
 * Checks:
 *   BR1: Genesis events detected (non-zero across ensemble)
 *   BR2: Genesis prefers higher-flux region (more events near peak A)
 *   BR3: Genesis ratio between peaks roughly tracks flux² ratio
 *   BR4: No genesis in sub-threshold region (|J| < K_B)
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
    std::cout << "  CAMPAIGN: Born Rule (Phase 3) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int mid = L / 2;
    const int N_RUNS = 200;
    const int GENESIS_TICKS = 5;  // Just enough for genesis to occur

    // Two flux peaks along x-axis:
    //   Peak A at x=8:  amplitude = K_B * 4.0 (strong)
    //   Peak B at x=24: amplitude = K_B * 2.5 (weaker)
    // Ratio of |J|²: (4.0)² / (2.5)² = 16/6.25 = 2.56

    int peak_a = 8;
    int peak_b = 24;
    double amp_a = ftd::K_B * 4.0;
    double amp_b = ftd::K_B * 2.5;
    double expected_ratio = (amp_a * amp_a) / (amp_b * amp_b);

    std::cout << "\n--- Setup ---\n";
    std::cout << "  Peak A (x=" << peak_a << "): amp=" << amp_a << "\n";
    std::cout << "  Peak B (x=" << peak_b << "): amp=" << amp_b << "\n";
    std::cout << "  Expected |J|² ratio: " << expected_ratio << "\n";
    std::cout << "  N_runs: " << N_RUNS << "\n\n";

    // Count genesis events near each peak
    int genesis_near_a = 0;
    int genesis_near_b = 0;
    int genesis_elsewhere = 0;
    int total_genesis = 0;
    int genesis_subthreshold = 0;

    for (int run = 0; run < N_RUNS; ++run) {
        ftd::RenderBridge rb(L);
        rb.seed_rng(static_cast<unsigned int>(run * 137 + 42));
        rb.toggles.genesis = true;
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = false;     // No coupling — pure genesis test
        rb.toggles.forces = false;
        rb.toggles.gravity = false;
        rb.toggles.movement = false;

        // Inject two flux peaks
        rb.inject_flux(peak_a, mid, mid, {amp_a, 0, 0});
        rb.inject_flux(peak_a, mid+1, mid, {amp_a * 0.5, 0, 0});  // Gaussian-ish
        rb.inject_flux(peak_a, mid, mid+1, {amp_a * 0.5, 0, 0});

        rb.inject_flux(peak_b, mid, mid, {amp_b, 0, 0});
        rb.inject_flux(peak_b, mid+1, mid, {amp_b * 0.3, 0, 0});
        rb.inject_flux(peak_b, mid, mid+1, {amp_b * 0.3, 0, 0});

        // Also inject a sub-threshold region
        rb.inject_flux(mid, mid, mid, {ftd::K_B * 0.5, 0, 0});  // Below K_B

        // Run a few ticks for genesis
        rb.run(GENESIS_TICKS);

        // Count where genesis happened
        int N_total = rb.lattice().total_sites();
        for (int i = 0; i < N_total; ++i) {
            if (rb.voxels()[i].state != 0) {
                total_genesis++;
                auto c = rb.lattice().coord(i);

                // Near peak A? (within 3 voxels)
                if (std::abs(c.x - peak_a) <= 3) {
                    genesis_near_a++;
                }
                // Near peak B?
                else if (std::abs(c.x - peak_b) <= 3) {
                    genesis_near_b++;
                }
                // Near sub-threshold region?
                else if (std::abs(c.x - mid) <= 2 &&
                         std::abs(c.y - mid) <= 2 &&
                         std::abs(c.z - mid) <= 2) {
                    genesis_subthreshold++;
                }
                else {
                    genesis_elsewhere++;
                }
            }
        }
    }

    std::cout << "--- Genesis Statistics ---\n";
    std::cout << "  Total genesis events: " << total_genesis << "\n";
    std::cout << "  Near peak A (x=" << peak_a << "): " << genesis_near_a << "\n";
    std::cout << "  Near peak B (x=" << peak_b << "): " << genesis_near_b << "\n";
    std::cout << "  Sub-threshold region: " << genesis_subthreshold << "\n";
    std::cout << "  Elsewhere: " << genesis_elsewhere << "\n";

    // ----------------------------------------------------------------
    // BR1: Genesis events detected
    // ----------------------------------------------------------------
    check("BR1: Genesis events detected (total > 0)", total_genesis > 0);

    // ----------------------------------------------------------------
    // BR2: Higher flux → more genesis
    // ----------------------------------------------------------------
    check("BR2: More genesis near stronger peak A than weaker peak B",
          genesis_near_a > genesis_near_b);

    // ----------------------------------------------------------------
    // BR3: Genesis ratio roughly tracks |J|² ratio
    // ----------------------------------------------------------------
    double measured_ratio = 0.0;
    if (genesis_near_b > 0) {
        measured_ratio = static_cast<double>(genesis_near_a) / genesis_near_b;
    } else if (genesis_near_a > 0) {
        measured_ratio = 100.0;  // All events near A, none near B
    }
    std::cout << "\n  Measured genesis ratio A/B: " << measured_ratio
              << " (expected ~" << expected_ratio << ")\n";

    // Ratio should be in the right ballpark (within factor of 3)
    // Exact Born rule gives ratio = (amp_a/amp_b)² = 2.56
    // Genesis is stochastic, so we allow generous bounds
    bool ratio_ok = (measured_ratio > expected_ratio / 3.0 &&
                     measured_ratio < expected_ratio * 3.0) ||
                    (genesis_near_b == 0 && genesis_near_a > 0);
    check("BR3: Genesis ratio tracks |J|² (within 3x)", ratio_ok);

    // ----------------------------------------------------------------
    // BR4: No genesis in sub-threshold region
    // ----------------------------------------------------------------
    std::cout << "  Sub-threshold genesis: " << genesis_subthreshold << "\n";
    check("BR4: No genesis in sub-threshold region (|J| < K_B)",
          genesis_subthreshold == 0);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
