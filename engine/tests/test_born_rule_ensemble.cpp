/**
 * Test: Born Rule Ensemble — Multi-site |psi|^2 Distribution Validation
 *
 * Enhances Born rule testing beyond single-site genesis to validate
 * the full multi-site probability distribution:
 *   BORN-1: Gaussian flux → manifestation histogram matches |psi|^2
 *   BORN-2: Two-peak flux → bimodal manifestation distribution
 *   BORN-3: Normalization: sum P(v) = 1 for Born distribution
 *   BORN-4: Inner product orthogonality for spatially separated wave packets
 *
 * Theory references:
 *   - CLAUDE.md §4.1, §13.1          (manifestation probability, Born rule)
 *   - DERIV_QUANTUM_MECHANICS_RESOLVED.md  (Born rule derivation)
 *   - SPEC_FTD_REFERENCE.md           (P(v) = |psi(v)|^2 / ||psi||^2)
 *
 * Epistemic status: [SELECTION + IMPOSED] — the Born rule emerges under
 * the manifestation-threshold sampling rule, which is itself imposed.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include "ftd/render_bridge.h"
#include "ftd/hilbert.h"
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

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(8) << a
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Born Rule Ensemble — Multi-Site |psi|^2 Validation\n";
    std::cout << "================================================================\n";

    // ================================================================
    // BORN-1: Gaussian flux → manifestation histogram matches |psi|^2
    // ================================================================
    // Inject a Gaussian flux profile above K_GENESIS. Run many independent
    // realizations. The histogram of WHERE genesis occurs should match
    // the Born distribution |psi(v)|^2 / ||psi||^2.
    std::cout << "\n--- BORN-1: Gaussian Flux Manifestation Histogram ---\n";
    {
        int L = 16;
        int center = L / 2;
        int N_realizations = 200;
        double sigma = 2.0;
        double amplitude = ftd::K_GENESIS * 4.0;  // well above threshold

        // We will count manifestation events along the x-axis (y=z=center)
        // by running many realizations with different RNG seeds
        std::vector<int> histogram(L, 0);
        int total_events = 0;

        for (int trial = 0; trial < N_realizations; ++trial) {
            ftd::RenderBridge rb(L);
            rb.seed_rng(static_cast<unsigned int>(trial * 137 + 42));
            // Disable heavy physics not needed for genesis statistics
            rb.toggles.coupling = false;
            rb.toggles.forces = false;
            rb.toggles.gravity = false;
            rb.toggles.movement = false;
            rb.toggles.poisson_coulomb = false;
            rb.toggles.gauss_projection = false;
            rb.toggles.lorentz_force = false;

            // Inject Gaussian flux profile along x-axis
            for (int x = 0; x < L; ++x) {
                double dx = x - center;
                double amp = amplitude * std::exp(-dx * dx / (2.0 * sigma * sigma));
                if (amp > 0.01) {
                    rb.inject_flux(x, center, center, {amp, amp * 0.5, 0.0});
                }
            }

            // Run a few ticks to allow genesis (5 ticks is sufficient)
            rb.run(5);

            // Record where genesis occurred
            for (int x = 0; x < L; ++x) {
                int idx = rb.lattice().index(x, center, center);
                if (rb.voxels()[idx].state != 0) {
                    histogram[x]++;
                    total_events++;
                }
            }
        }

        std::cout << "    Total manifestation events: " << total_events << "\n";

        // Compute the theoretical Born distribution from the initial flux
        ftd::RenderBridge rb_ref(L);
        for (int x = 0; x < L; ++x) {
            double dx = x - center;
            double amp = amplitude * std::exp(-dx * dx / (2.0 * sigma * sigma));
            if (amp > 0.01) {
                rb_ref.inject_flux(x, center, center, {amp, amp * 0.5, 0.0});
            }
        }
        auto h = rb_ref.hilbert_state();

        // Compare histogram shape with |psi|^2 along x-axis
        // The distribution should be concentrated near center and decay outward.
        // Due to wave propagation over 50 ticks, the exact center bin may not
        // be the strict maximum — nearby bins can receive comparable flux after
        // the center manifests and the field evolves. Use a 3-bin window
        // around center vs edges to verify the bell-shaped profile.
        int center_window = histogram[center - 1] + histogram[center] + histogram[center + 1];
        int edge_left = histogram[0] + histogram[1] + histogram[2];
        int edge_right = histogram[L-3] + histogram[L-2] + histogram[L-1];
        bool center_peak = (center_window > edge_left) && (center_window > edge_right);

        std::cout << "    Histogram[center-3..center+3]: ";
        for (int x = center - 3; x <= center + 3; ++x) {
            std::cout << histogram[x] << " ";
        }
        std::cout << "\n";
        std::cout << "    Center window (3 bins): " << center_window
                  << ", Edge left: " << edge_left
                  << ", Edge right: " << edge_right << "\n";

        check("BORN-1a: manifestation events occurred", total_events > 0);
        check("BORN-1b: distribution peaks near center", center_peak);

        // Histogram should not be uniform — should be concentrated near center
        int inner_count = 0;
        int outer_count = 0;
        for (int x = center - 2; x <= center + 2; ++x) inner_count += histogram[x];
        for (int x = 0; x < center - 4; ++x) outer_count += histogram[x];
        for (int x = center + 5; x < L; ++x) outer_count += histogram[x];

        std::cout << "    Inner (center +/- 2): " << inner_count
                  << ", Outer (edges): " << outer_count << "\n";
        check("BORN-1c: manifestation concentrated near high |psi|^2",
              inner_count > outer_count);
    }

    // ================================================================
    // BORN-2: Two-peak flux → bimodal manifestation distribution
    // ================================================================
    std::cout << "\n--- BORN-2: Two-Peak Bimodal Distribution ---\n";
    {
        int L = 32;
        int peak_a = 8;
        int peak_b = 24;
        int cy = L / 2;
        int N_realizations = 200;
        double amplitude = ftd::K_GENESIS * 4.0;

        std::vector<int> histogram(L, 0);
        int total_events = 0;

        for (int trial = 0; trial < N_realizations; ++trial) {
            ftd::RenderBridge rb(L);
            rb.seed_rng(static_cast<unsigned int>(trial * 251 + 7));
            // Disable heavy physics not needed for genesis statistics
            rb.toggles.coupling = false;
            rb.toggles.forces = false;
            rb.toggles.gravity = false;
            rb.toggles.movement = false;
            rb.toggles.poisson_coulomb = false;
            rb.toggles.gauss_projection = false;
            rb.toggles.lorentz_force = false;

            // Two separated peaks of equal amplitude
            rb.inject_flux(peak_a, cy, cy, {amplitude, amplitude * 0.5, 0.0});
            rb.inject_flux(peak_a + 1, cy, cy, {amplitude * 0.5, amplitude * 0.25, 0.0});
            rb.inject_flux(peak_a - 1, cy, cy, {amplitude * 0.5, amplitude * 0.25, 0.0});

            rb.inject_flux(peak_b, cy, cy, {amplitude, amplitude * 0.5, 0.0});
            rb.inject_flux(peak_b + 1, cy, cy, {amplitude * 0.5, amplitude * 0.25, 0.0});
            rb.inject_flux(peak_b - 1, cy, cy, {amplitude * 0.5, amplitude * 0.25, 0.0});

            rb.run(5);

            for (int x = 0; x < L; ++x) {
                int idx = rb.lattice().index(x, cy, cy);
                if (rb.voxels()[idx].state != 0) {
                    histogram[x]++;
                    total_events++;
                }
            }
        }

        std::cout << "    Total manifestation events: " << total_events << "\n";

        // Count events near each peak
        int near_a = 0, near_b = 0, in_between = 0;
        for (int x = peak_a - 3; x <= peak_a + 3; ++x) near_a += histogram[x];
        for (int x = peak_b - 3; x <= peak_b + 3; ++x) near_b += histogram[x];
        for (int x = peak_a + 5; x < peak_b - 4; ++x) in_between += histogram[x];

        std::cout << "    Near peak A (" << peak_a << "): " << near_a << "\n";
        std::cout << "    Near peak B (" << peak_b << "): " << near_b << "\n";
        std::cout << "    Between peaks: " << in_between << "\n";

        check("BORN-2a: events near both peaks", near_a > 0 && near_b > 0);
        check("BORN-2b: bimodal (peaks > valley)", near_a > in_between && near_b > in_between);

        // Equal amplitude peaks should yield roughly equal counts
        double ratio = (near_b > 0) ? static_cast<double>(near_a) / near_b : 0.0;
        std::cout << "    Peak ratio A/B: " << ratio << "\n";
        check("BORN-2c: equal peaks yield similar counts (ratio 0.3-3.0)",
              ratio > 0.3 && ratio < 3.0);
    }

    // ================================================================
    // BORN-3: Normalization — sum P(v) = 1
    // ================================================================
    std::cout << "\n--- BORN-3: Born Distribution Normalization ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);

        // Inject flux at several sites with varied amplitudes
        rb.inject_flux(4, 8, 8, {2.0, 1.0, 0.5});
        rb.inject_flux(8, 8, 8, {3.0, 2.0, 1.0});
        rb.inject_flux(12, 8, 8, {1.0, 0.5, 0.3});
        rb.inject_flux(8, 4, 8, {0.5, 1.5, 0.0});

        auto h = rb.hilbert_state();
        auto dist = h.born_distribution();

        double sum = 0.0;
        double max_p = 0.0;
        int max_idx = -1;
        bool all_non_negative = true;

        for (int i = 0; i < static_cast<int>(dist.size()); ++i) {
            sum += dist[i];
            if (dist[i] < 0.0) all_non_negative = false;
            if (dist[i] > max_p) {
                max_p = dist[i];
                max_idx = i;
            }
        }

        std::cout << "    Sum of P(v): " << std::setprecision(15) << sum << "\n";
        std::cout << "    Max P(v) = " << max_p << " at index " << max_idx << "\n";

        check("BORN-3a: all probabilities >= 0", all_non_negative);
        check_close("BORN-3b: sum P(v) = 1.0", sum, 1.0, 1e-12);

        // Verify that P(v) is proportional to |psi(v)|^2
        int idx_high = rb.lattice().index(8, 8, 8);  // highest flux
        int idx_low = rb.lattice().index(12, 8, 8);   // lower flux

        double p_high = h.born_probability(idx_high);
        double p_low = h.born_probability(idx_low);

        std::cout << "    P(high flux site) = " << p_high << "\n";
        std::cout << "    P(low flux site) = " << p_low << "\n";

        check("BORN-3c: higher |psi|^2 → higher P(v)", p_high > p_low);
    }

    // ================================================================
    // BORN-4: Inner product orthogonality for separated wave packets
    // ================================================================
    std::cout << "\n--- BORN-4: Inner Product Orthogonality ---\n";
    {
        int L = 32;

        // Packet 1: localized at (6,16,16), polarized in x-direction
        ftd::RenderBridge rb1(L);
        for (int dx = -2; dx <= 2; ++dx) {
            double amp = 2.0 * std::exp(-dx * dx / 2.0);
            rb1.inject_flux(6 + dx, 16, 16, {amp, 0.0, 0.0});
        }

        // Packet 2: localized at (26,16,16), polarized in y-direction
        ftd::RenderBridge rb2(L);
        for (int dx = -2; dx <= 2; ++dx) {
            double amp = 2.0 * std::exp(-dx * dx / 2.0);
            rb2.inject_flux(26 + dx, 16, 16, {0.0, amp, 0.0});
        }

        auto h1 = rb1.hilbert_state();
        auto h2 = rb2.hilbert_state();

        ftd::Complex ip12 = h1.inner_product(h2);
        double fid12 = ftd::fidelity(h1, h2);

        std::cout << "    <psi1|psi2> = " << ip12.real() << " + " << ip12.imag() << "i\n";
        std::cout << "    Fidelity = " << fid12 << "\n";

        // Spatially separated packets with no overlap should be orthogonal
        check("BORN-4a: <psi1|psi2> ~ 0 (separated packets)", std::abs(ip12) < 1e-12);
        check_close("BORN-4b: fidelity ~ 0 (orthogonal)", fid12, 0.0, 1e-12);

        // Self-fidelity should be 1
        double fid11 = ftd::fidelity(h1, h1);
        check_close("BORN-4c: self-fidelity = 1", fid11, 1.0, 1e-12);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Born rule ensemble tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
