/**
 * Falsifiability Tests: Negative-Result Validation
 *
 * PURPOSE: Demonstrate that FTD is CONSTRAINED, not arbitrary.
 * These tests verify that WRONG parameter choices produce WRONG physics.
 * A framework that accepts any input is unfalsifiable; these tests show
 * that specific inputs are required for physically meaningful outputs.
 *
 * This is the single most important test for scientific credibility:
 * it transforms "we chose parameters that work" into "only these
 * parameters CAN work."
 *
 * 12 checks:
 *   F1:  Wrong coefficient (k=15) gives wrong alpha
 *   F2:  Wrong coefficient (k=17) gives wrong alpha
 *   F3:  Wrong G* (3.0 instead of 2.9587) gives wrong alpha
 *   F4:  Wrong G* (2.9 instead of 2.9587) gives wrong alpha
 *   F5:  N_c=4 gives wrong Weinberg angle (not within 1% of exp)
 *   F6:  N_c=2 gives wrong Weinberg angle (not within 1% of exp)
 *   F7:  b_3=8 gives wrong strong coupling (not within 2% of exp)
 *   F8:  N_eff=12 gives wrong strong coupling (not within 2% of exp)
 *   F9:  Wrong integers break precision formula (> 100 ppm)
 *   F10: Master quadratic discriminant requires G* > 1/4 for real roots
 *   F11: 3 generations requires x- in [3, 4) — wrong G* breaks this
 *   F12: Correct parameters DO produce correct physics (control)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/constants.h"
#include "ftd/ontic.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

// Compute 1/alpha from master quadratic with arbitrary coefficient and G*
double compute_alpha_inv(double k, double gstar) {
    double disc = k * gstar * gstar * gstar * (k * gstar - 4.0);
    if (disc < 0) return -1.0;  // complex roots — no physical alpha
    double sqrt_disc = std::sqrt(disc);
    double x_plus = (k * gstar * gstar + sqrt_disc) / 2.0;
    return x_plus;
}

// Compute x- (color root) from master quadratic
double compute_x_minus(double k, double gstar) {
    double disc = k * gstar * gstar * gstar * (k * gstar - 4.0);
    if (disc < 0) return -1.0;
    double sqrt_disc = std::sqrt(disc);
    double x_minus = (k * gstar * gstar - sqrt_disc) / 2.0;
    return x_minus;
}

// Compute Weinberg angle from N_c and N_eff: sin^2(theta_W) = N_c / N_eff
double compute_sin2_weinberg(int nc, int neff) {
    return static_cast<double>(nc) / neff;
}

// Compute strong coupling from b_3 and N_eff: alpha_s = b_3 / (b_3 + 4*N_eff)
double compute_alpha_s(int b3, int neff) {
    return static_cast<double>(b3) / (b3 + 4 * neff);
}

// Compute 4-term precision formula with arbitrary integers
double compute_precision_alpha(double xp, int nc, int nbase, int b3, int neff) {
    int d = nc * nbase * nbase - 1;  // constraint dimension

    double c1 = static_cast<double>(nc * nc) / d;
    double c2 = static_cast<double>(neff - 2 * nbase) / (nbase * nbase * nbase);
    double c3 = static_cast<double>(nbase) / (nc * d);
    double c4 = static_cast<double>(nc * d) / (b3 + nbase);

    double e_pi = std::exp(M_PI);
    double eps = e_pi - M_PI - (b3 + neff);
    double ea = std::abs(eps);

    return xp - c1 * ea + c2 * ea * ea - c3 * ea * ea * ea - c4 * ea * ea * ea * ea;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  FALSIFIABILITY TESTS: Negative-Result Validation — 12 Checks\n";
    std::cout << "================================================================\n";

    double codata_alpha_inv = 137.035999177;  // CODATA 2022
    double exp_sin2_w = 0.23122;              // Experimental Weinberg angle
    double exp_alpha_s = 0.1179;              // Experimental alpha_s(M_Z)

    // ================================================================
    // Part 1: Wrong coefficient in master quadratic
    // ================================================================
    std::cout << "\n--- Wrong Coefficient Tests ---\n";

    {
        double gstar = ftd::G_STAR;

        // k=15 (wrong): should NOT give 1/alpha ~ 137
        double alpha_inv_k15 = compute_alpha_inv(15, gstar);
        double err_k15 = std::abs(alpha_inv_k15 - codata_alpha_inv) / codata_alpha_inv;
        std::cout << "  k=15: 1/alpha = " << std::setprecision(6) << alpha_inv_k15
                  << " (error: " << err_k15 * 100 << "%)\n";
        check("F1: k=15 gives WRONG alpha (error > 1%)", err_k15 > 0.01);

        // k=17 (wrong): should NOT give 1/alpha ~ 137
        double alpha_inv_k17 = compute_alpha_inv(17, gstar);
        double err_k17 = std::abs(alpha_inv_k17 - codata_alpha_inv) / codata_alpha_inv;
        std::cout << "  k=17: 1/alpha = " << std::setprecision(6) << alpha_inv_k17
                  << " (error: " << err_k17 * 100 << "%)\n";
        check("F2: k=17 gives WRONG alpha (error > 1%)", err_k17 > 0.01);
    }

    // ================================================================
    // Part 2: Wrong G* value
    // ================================================================
    std::cout << "\n--- Wrong G* Tests ---\n";

    {
        int k = ftd::COEFFICIENT;  // 16

        // G*=3.0 (wrong, 1.4% off):
        double alpha_inv_g30 = compute_alpha_inv(k, 3.0);
        double err_g30 = std::abs(alpha_inv_g30 - codata_alpha_inv) / codata_alpha_inv;
        std::cout << "  G*=3.0: 1/alpha = " << std::setprecision(6) << alpha_inv_g30
                  << " (error: " << err_g30 * 100 << "%)\n";
        check("F3: G*=3.0 gives WRONG alpha (error > 1%)", err_g30 > 0.01);

        // G*=2.9 (wrong, 2% off):
        double alpha_inv_g29 = compute_alpha_inv(k, 2.9);
        double err_g29 = std::abs(alpha_inv_g29 - codata_alpha_inv) / codata_alpha_inv;
        std::cout << "  G*=2.9: 1/alpha = " << std::setprecision(6) << alpha_inv_g29
                  << " (error: " << err_g29 * 100 << "%)\n";
        check("F4: G*=2.9 gives WRONG alpha (error > 1%)", err_g29 > 0.01);
    }

    // ================================================================
    // Part 3: Wrong integers for Weinberg angle
    // ================================================================
    std::cout << "\n--- Wrong Integer Tests (Weinberg) ---\n";

    {
        // N_c=4, N_eff=13: sin^2 = 4/13 = 0.3077 (way off from 0.23122)
        double sw_nc4 = compute_sin2_weinberg(4, 13);
        double err_nc4 = std::abs(sw_nc4 - exp_sin2_w) / exp_sin2_w;
        std::cout << "  N_c=4, N_eff=13: sin^2(theta_W) = " << std::setprecision(5)
                  << sw_nc4 << " (error: " << err_nc4 * 100 << "%)\n";
        check("F5: N_c=4 gives WRONG Weinberg angle (error > 1%)", err_nc4 > 0.01);

        // N_c=2, N_eff=13: sin^2 = 2/13 = 0.1538 (way off)
        double sw_nc2 = compute_sin2_weinberg(2, 13);
        double err_nc2 = std::abs(sw_nc2 - exp_sin2_w) / exp_sin2_w;
        std::cout << "  N_c=2, N_eff=13: sin^2(theta_W) = " << std::setprecision(5)
                  << sw_nc2 << " (error: " << err_nc2 * 100 << "%)\n";
        check("F6: N_c=2 gives WRONG Weinberg angle (error > 1%)", err_nc2 > 0.01);
    }

    // ================================================================
    // Part 4: Wrong integers for strong coupling
    // ================================================================
    std::cout << "\n--- Wrong Integer Tests (Strong Coupling) ---\n";

    {
        // b_3=8, N_eff=13: alpha_s = 8/(8+52) = 0.1333 (13% off from 0.1179)
        double as_b8 = compute_alpha_s(8, 13);
        double err_b8 = std::abs(as_b8 - exp_alpha_s) / exp_alpha_s;
        std::cout << "  b_3=8, N_eff=13: alpha_s = " << std::setprecision(5)
                  << as_b8 << " (error: " << err_b8 * 100 << "%)\n";
        check("F7: b_3=8 gives WRONG alpha_s (error > 2%)", err_b8 > 0.02);

        // b_3=7, N_eff=12: alpha_s = 7/(7+48) = 0.1273 (8% off from 0.1179)
        double as_n12 = compute_alpha_s(7, 12);
        double err_n12 = std::abs(as_n12 - exp_alpha_s) / exp_alpha_s;
        std::cout << "  b_3=7, N_eff=12: alpha_s = " << std::setprecision(5)
                  << as_n12 << " (error: " << err_n12 * 100 << "%)\n";
        check("F8: N_eff=12 gives WRONG alpha_s (error > 2%)", err_n12 > 0.02);
    }

    // ================================================================
    // Part 5: Wrong integers break precision formula
    // ================================================================
    std::cout << "\n--- Wrong Integer Precision Formula ---\n";

    {
        // Correct x+ but wrong integers: {4, 5, 8, 14}
        double alpha_inv_wrong = compute_precision_alpha(ftd::X_PLUS, 4, 5, 8, 14);
        double ppm_wrong = std::abs(alpha_inv_wrong - codata_alpha_inv) / codata_alpha_inv * 1e6;
        std::cout << "  {4,5,8,14}: 1/alpha = " << std::setprecision(10) << alpha_inv_wrong
                  << " (" << ppm_wrong << " ppm)\n";
        check("F9: Wrong integers {4,5,8,14} break precision (> 100 ppm)", ppm_wrong > 100.0);
    }

    // ================================================================
    // Part 6: Discriminant requires G* > 1/4 for real roots
    // ================================================================
    std::cout << "\n--- Discriminant Constraint ---\n";

    {
        int k = ftd::COEFFICIENT;
        // G* must satisfy k*G* > 4, i.e., G* > 4/16 = 0.25
        // G* = 0.2 → discriminant < 0 → no real physics roots
        double disc_low = k * 0.2 * 0.2 * 0.2 * (k * 0.2 - 4.0);
        std::cout << "  G*=0.2: discriminant = " << disc_low << " (must be < 0)\n";
        check("F10: G* < 0.25 gives complex roots (no real physics)", disc_low < 0);
    }

    // ================================================================
    // Part 7: 3 generations requires x- in [3, 4)
    // ================================================================
    std::cout << "\n--- Generation Count Constraint ---\n";

    {
        // G*=3.5 → x- should be far enough from [3,4) to give wrong gen count
        double xm_g35 = compute_x_minus(ftd::COEFFICIENT, 3.5);
        int ngen_g35 = (xm_g35 > 0) ? static_cast<int>(std::floor(xm_g35)) : -1;
        std::cout << "  G*=3.5: x- = " << std::setprecision(4) << xm_g35
                  << ", N_gen = " << ngen_g35 << "\n";

        // G*=2.0 → should give different gen count
        double xm_g20 = compute_x_minus(ftd::COEFFICIENT, 2.0);
        int ngen_g20 = (xm_g20 > 0) ? static_cast<int>(std::floor(xm_g20)) : -1;
        std::cout << "  G*=2.0: x- = " << std::setprecision(4) << xm_g20
                  << ", N_gen = " << ngen_g20 << "\n";

        // At least one of these must give wrong gen count
        bool either_wrong = (ngen_g35 != 3) || (ngen_g20 != 3);
        check("F11: Sufficiently wrong G* breaks generation count", either_wrong);
    }

    // ================================================================
    // Part 8: Control — correct parameters DO work
    // ================================================================
    std::cout << "\n--- Control: Correct Parameters ---\n";

    {
        // Master quadratic with correct k=16, G*=2.9587
        double alpha_inv_correct = compute_alpha_inv(ftd::COEFFICIENT, ftd::G_STAR);
        double err_correct = std::abs(alpha_inv_correct - codata_alpha_inv) / codata_alpha_inv;

        // Weinberg with correct N_c=3, N_eff=13
        double sw_correct = compute_sin2_weinberg(ftd::N_C, ftd::N_EFF);
        double err_sw = std::abs(sw_correct - exp_sin2_w) / exp_sin2_w;

        // Strong coupling with correct b_3=7, N_eff=13
        double as_correct = compute_alpha_s(ftd::B_3, ftd::N_EFF);
        double err_as = std::abs(as_correct - exp_alpha_s) / exp_alpha_s;

        // Generation count from x-
        double xm = compute_x_minus(ftd::COEFFICIENT, ftd::G_STAR);
        int ngen = static_cast<int>(std::floor(xm));

        std::cout << "  Correct 1/alpha = " << std::setprecision(8) << alpha_inv_correct
                  << " (error: " << err_correct * 100 << "%)\n";
        std::cout << "  Correct sin^2(theta_W) = " << std::setprecision(5) << sw_correct
                  << " (error: " << err_sw * 100 << "%)\n";
        std::cout << "  Correct alpha_s = " << std::setprecision(5) << as_correct
                  << " (error: " << err_as * 100 << "%)\n";
        std::cout << "  Correct N_gen = " << ngen << "\n";

        bool all_correct = (err_correct < 0.001) &&   // < 0.1% for alpha
                           (err_sw < 0.003) &&          // < 0.3% for Weinberg
                           (err_as < 0.01) &&            // < 1% for alpha_s
                           (ngen == 3);                   // exactly 3 generations

        check("F12: Correct {16, G*, 3, 4, 7, 13} gives correct physics", all_correct);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  NOTE: These tests demonstrate that FTD parameters are\n";
    std::cout << "  CONSTRAINED — wrong inputs produce wrong physics.\n";
    std::cout << "  This is necessary (but not sufficient) for falsifiability.\n";
    std::cout << "  The framework is not arbitrary: it can fail.\n";
    std::cout << "================================================================\n";
    return failures;
}
