/**
 * Test: FTD Lorentz Factor special cases
 *
 * Verifies gamma_FTD = 1/sqrt(1 - v^2 - L^2)
 * for all special cases in SPEC_FTD_LAGRANGIAN.md Part V.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (Born-Infeld with built-in speed limit)
 *   - DERIV_RELATIVITY_DERIVATION.md     (complete SR derivation from C=1)
 *   - FOUND_RELATIVITY_GRAVITY_DISTINCTION.md (SR/Gravity/GR trichotomy)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/voxel.h"
#include "ftd/constants.h"

int failures = 0;

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10) << a
                  << ", expected " << b << ")\n";
        ++failures;
    }
}

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

double gamma_ftd(double v, double L) {
    double bw = v * v + L * L;
    if (bw >= 1.0) return 1e30;
    return 1.0 / std::sqrt(1.0 - bw);
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: FTD Lorentz Factor Special Cases\n";
    std::cout << "================================================================\n\n";

    // Case 1: Rest in flat space
    check_close("Rest flat: gamma = 1", gamma_ftd(0, 0), 1.0, 1e-15);

    // Case 2: Standard SR (L=0)
    double v_tests[] = {0.1, 0.3, 0.5, 0.8, 0.9, 0.99};
    for (double v : v_tests) {
        double g_ftd = gamma_ftd(v, 0);
        double g_sr = 1.0 / std::sqrt(1.0 - v*v);
        char buf[128];
        snprintf(buf, sizeof(buf), "SR v=%.2f: gamma_FTD = gamma_SR", v);
        check_close(buf, g_ftd, g_sr, 1e-12);
    }

    // Case 3: Gravitational time dilation (v=0)
    double rs_over_r = 0.1;
    double L = std::sqrt(rs_over_r);
    double g = gamma_ftd(0, L);
    double g_schwarz = 1.0 / std::sqrt(1.0 - rs_over_r);
    check_close("Grav dilation r_s/r=0.1", g, g_schwarz, 1e-12);

    // Case 4: Dark matter (L=0.75)
    double g_dm = gamma_ftd(0, 0.75);
    double expected = 1.0 / std::sqrt(1.0 - 0.75*0.75);
    check_close("Dark matter L=0.75", g_dm, expected, 1e-12);
    check("Dark matter gamma > 1.5", g_dm > 1.5);

    // Case 5: Combined v and L
    double v_combo = 0.6, L_combo = 0.6;
    double budget = v_combo*v_combo + L_combo*L_combo;
    check("v=0.6 L=0.6 budget < 1", budget < 1.0);
    double g_combo = gamma_ftd(v_combo, L_combo);
    double expected_combo = 1.0 / std::sqrt(1.0 - budget);
    check_close("Combined v=0.6 L=0.6", g_combo, expected_combo, 1e-12);

    // Case 6: Near horizon
    double g_horizon = gamma_ftd(0, std::sqrt(0.999));
    check("Near horizon gamma > 30", g_horizon > 30.0);

    // Case 7: Budget exceeded (forbidden)
    double budget_over = 0.8*0.8 + 0.7*0.7;
    check("v=0.8 L=0.7 budget > 1 (forbidden)", budget_over > 1.0);

    // Case 8: Voxel gamma_ftd method
    ftd::Voxel vox;
    vox.velocity = {0.3, 0.4, 0.0};  // |v| = 0.5
    vox.latency = 0.3;
    double g_vox = vox.gamma_ftd();
    double g_expected = 1.0 / std::sqrt(1.0 - 0.25 - 0.09);
    check_close("Voxel gamma_ftd v=0.5 L=0.3", g_vox, g_expected, 1e-12);

    // Case 9: Born-Infeld core
    double bi = vox.born_infeld_core();
    double bi_expected = -ftd::K_B * std::sqrt(1.0 - 0.25 - 0.09);
    check_close("Voxel Born-Infeld core", bi, bi_expected, 1e-12);

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Lorentz factor tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
