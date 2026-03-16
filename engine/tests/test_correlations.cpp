/**
 * Test: Correlation Functions (Phase 1 — Measurement Infrastructure)
 *
 * Verifies that spatial and temporal correlation functions work correctly
 * on known field configurations.
 *
 *   CR1: Uniform flux → C(r) = const for all r
 *   CR2: Single point source → C(0) >> C(r>0)
 *   CR3: Charge correlator on empty lattice → G(r) = 0
 *   CR4: Charge correlator with particle pair → G(0) > 0
 *   CR5: Structure factor S(k=0) = sum of C(r)
 *   CR6: Temporal autocorrelation of constant series → C(tau) = 0
 *   CR7: Temporal autocorrelation of sine wave → peaked at period
 *   CR8: Density correlation on empty lattice → D(r) ≈ 0
 */

#include <cmath>
#include <iostream>
#include <vector>
#include "ftd/correlations.h"
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
    std::cout << "  TEST: Correlation Functions (Phase 1) — 8 Checks\n";
    std::cout << "================================================================\n";

    const int L = 16;

    // ----------------------------------------------------------------
    // CR1: Uniform flux → C(r) ≈ constant
    // ----------------------------------------------------------------
    std::cout << "\n--- CR1: Uniform Flux Correlation ---\n";
    {
        ftd::RenderBridge rb(L);
        // Set all voxels to uniform flux
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0.1, 0.2, 0.3});

        auto C = ftd::spatial_flux_correlation(rb, L / 2);

        // C(0) and C(r) should be identical for uniform field
        double C0 = C[0];
        bool all_equal = true;
        for (int r = 1; r < static_cast<int>(C.size()); ++r) {
            if (std::abs(C[r] - C0) > 1e-10 * std::abs(C0)) {
                all_equal = false;
                break;
            }
        }
        check("CR1: Uniform flux gives constant C(r)", all_equal);
    }

    // ----------------------------------------------------------------
    // CR2: Point source → C(0) >> C(r>0)
    // ----------------------------------------------------------------
    std::cout << "\n--- CR2: Point Source Correlation ---\n";
    {
        ftd::RenderBridge rb(L);
        rb.inject_flux(L/2, L/2, L/2, {1.0, 0.0, 0.0});

        auto C = ftd::spatial_flux_correlation(rb, L / 2);
        std::cout << "  C(0)=" << C[0] << " C(1)=" << C[1] << " C(2)=" << C[2] << "\n";

        // C(0) should be much larger than C(r>0) since flux is localized
        check("CR2: C(0) > 100 * |C(1)|", C[0] > 100.0 * std::abs(C[1]));
    }

    // ----------------------------------------------------------------
    // CR3: Empty lattice → charge correlation = 0
    // ----------------------------------------------------------------
    std::cout << "\n--- CR3: Empty Charge Correlation ---\n";
    {
        ftd::RenderBridge rb(L);
        auto G = ftd::charge_correlation(rb, L / 2);

        bool all_zero = true;
        for (double g : G) {
            if (std::abs(g) > 1e-15) { all_zero = false; break; }
        }
        check("CR3: Empty lattice G(r) = 0", all_zero);
    }

    // ----------------------------------------------------------------
    // CR4: Particle pair → G(0) > 0
    // ----------------------------------------------------------------
    std::cout << "\n--- CR4: Particle Pair Charge Correlation ---\n";
    {
        ftd::RenderBridge rb(L);
        int mid = L / 2;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});

        auto G = ftd::charge_correlation(rb, L / 2);
        std::cout << "  G(0)=" << G[0] << " G(1)=" << G[1] << " G(3)=" << G[3] << "\n";

        // G(0) = <s²> > 0 since there are manifested particles
        check("CR4: G(0) > 0 with particles", G[0] > 0.0);
    }

    // ----------------------------------------------------------------
    // CR5: Structure factor S(k=0) = sum of C(r)
    // ----------------------------------------------------------------
    std::cout << "\n--- CR5: Structure Factor Sum Rule ---\n";
    {
        ftd::RenderBridge rb(L);
        rb.inject_flux(L/2, L/2, L/2, {1.0, 0.0, 0.0});
        rb.inject_flux(L/4, L/4, L/4, {0.5, 0.5, 0.0});

        auto C = ftd::spatial_flux_correlation(rb, L / 2);
        auto S = ftd::structure_factor(C);

        // S(0) = C(0) + 2*sum(C(r), r=1..R-1)
        double C_sum = C[0];
        for (size_t r = 1; r < C.size(); ++r) C_sum += 2.0 * C[r];

        std::cout << "  S(0)=" << S[0] << " C_sum=" << C_sum << "\n";
        check("CR5: S(k=0) ≈ sum rule", std::abs(S[0] - C_sum) < 1e-8 * std::abs(C_sum + 1e-30));
    }

    // ----------------------------------------------------------------
    // CR6: Temporal autocorrelation of constant → C(tau) = 0
    // ----------------------------------------------------------------
    std::cout << "\n--- CR6: Constant Time Series ---\n";
    {
        std::vector<double> series(100, 5.0);  // constant
        auto C = ftd::temporal_autocorrelation(series, 20);

        bool all_near_zero = true;
        for (double c : C) {
            if (std::abs(c) > 1e-10) { all_near_zero = false; break; }
        }
        check("CR6: Constant series → C(tau) = 0", all_near_zero);
    }

    // ----------------------------------------------------------------
    // CR7: Sine wave → autocorrelation peaked at period
    // ----------------------------------------------------------------
    std::cout << "\n--- CR7: Sine Wave Autocorrelation ---\n";
    {
        int period = 20;
        std::vector<double> series(200);
        for (int i = 0; i < 200; ++i) {
            series[i] = std::sin(2.0 * ftd::PI * i / period);
        }
        auto C = ftd::temporal_autocorrelation(series, 40);

        // C(0) should be max (variance), C(period/2) should be negative,
        // C(period) should be positive again
        check("CR7: C(0) > 0", C[0] > 0.0);
        check("CR7b: C(period/2) < 0", C[period / 2] < 0.0);
        check("CR7c: C(period) > 0.5 * C(0)", C[period] > 0.5 * C[0]);
    }

    // ----------------------------------------------------------------
    // CR8: Empty lattice → density correlation ≈ 0
    // ----------------------------------------------------------------
    std::cout << "\n--- CR8: Empty Density Correlation ---\n";
    {
        ftd::RenderBridge rb(L);
        auto D = ftd::density_correlation(rb, L / 2);

        bool all_near_zero = true;
        for (double d : D) {
            if (std::abs(d) > 1e-15) { all_near_zero = false; break; }
        }
        check("CR8: Empty lattice D(r) ≈ 0", all_near_zero);
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
