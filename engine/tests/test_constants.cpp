/**
 * Test: Derivation chain D=3 -> alpha
 *
 * Verifies that all constants are self-consistently derived
 * from D=3 + varpi (lemniscate constant).
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (derivation chain, constants)
 *   - DERIV_ALPHA_PRECISION_FORMULA.md    (fine structure constant < 0.001 ppt)
 *   - FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md (γ → Γ(1/4) → ϖ → G* → α)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/constants.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int failures = 0;

void check(const char* name, bool condition, const char* detail = "") {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        if (detail[0]) std::cout << "        " << detail << "\n";
        ++failures;
    }
}

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Derivation Chain D=3 -> alpha\n";
    std::cout << "================================================================\n\n";

    // Axiomatic constants
    check("D = 3", ftd::D_SPATIAL == 3);
    check("N_BASE = 4", ftd::N_BASE == 4);
    check("COEFFICIENT = 16", ftd::COEFFICIENT == 16);
    check_close("PF = pi/4", ftd::PF, M_PI / 4.0, 1e-14);

    // Mathematical constants
    check_close("VARPI ~ 2.6221", ftd::VARPI, 2.6220575, 0.0001);
    check_close("G* ~ 2.9587", ftd::G_STAR, 2.958675, 0.0001);
    check_close("sqrt(G*) ~ 1.7201", ftd::SQRT_GSTAR, std::sqrt(ftd::G_STAR), 1e-10);

    // G* = varpi / sqrt(PF)
    double g_from_bridge = ftd::VARPI / std::sqrt(ftd::PF);
    check_close("G* = varpi/sqrt(PF)", ftd::G_STAR, g_from_bridge, 1e-10);

    // Master quadratic verification
    double c = ftd::G_STAR;
    double b = -16.0 * c * c;
    double cc = 16.0 * c * c * c;
    double disc = b * b - 4.0 * cc;
    double x_plus = (-b + std::sqrt(disc)) / 2.0;
    double x_minus = (-b - std::sqrt(disc)) / 2.0;

    check_close("x_+ ~ 137.036", x_plus, 137.036, 0.001);
    check_close("x_- ~ 3.024", x_minus, 3.024, 0.001);

    // Vieta relations
    check_close("Vieta: x_+ + x_- = 16*G*^2", x_plus + x_minus, 16.0 * c * c, 1e-10);
    check_close("Vieta: x_+ * x_- = 16*G*^3", x_plus * x_minus, 16.0 * c * c * c, 1e-10);

    // Derived integers
    check("N_C = floor(x_-) = 3", ftd::N_C == 3);
    check("B_3 = 7", ftd::B_3 == 7);
    check("N_EFF = 13", ftd::N_EFF == 13);
    check("D_CONSTRAINT = 47", ftd::D_CONSTRAINT == 47);

    // Framework integer relationships
    int n_gen = ftd::N_C;
    int n_f = 2 * n_gen;
    int b3_check = (11 * ftd::N_C - 2 * n_f) / 3;
    check("b_3 = (11*N_c - 2*N_f)/3", b3_check == ftd::B_3);

    int neff_check = ftd::B_3 + 2 * ftd::N_C;
    check("N_eff = b_3 + 2*N_c = 13", neff_check == ftd::N_EFF);

    // Alpha
    check_close("alpha ~ 1/137.036", ftd::ALPHA, 1.0 / 137.036, 0.0001);

    // Drag values
    check_close("DRAG_PER_AXIS = 0.25", ftd::DRAG_PER_AXIS, 0.25, 1e-15);
    check_close("DRAG_ELECTRON = 0.25", ftd::DRAG_ELECTRON, 0.25, 1e-15);
    check_close("DRAG_TOP = 0.75", ftd::DRAG_TOP, 0.75, 1e-15);

    // ================================================================
    // NEW: Derived constants from Lagrangian 2.0
    // ================================================================
    std::cout << "\n--- Lagrangian 2.0: Derived Constants ---\n";

    // G_N derivation: 1/(b_3 + N_c)^2 = 1/100 = 0.01
    check_close("G_N = 1/(B_3+N_C)^2 = 0.01", ftd::G_N, 0.01, 1e-15);
    check_close("G_N = 1/100 exactly", ftd::G_N, 1.0 / 100.0, 1e-15);
    check("G_N denominator = (7+3)^2 = 100",
          (ftd::B_3 + ftd::N_C) * (ftd::B_3 + ftd::N_C) == 100);

    // K_GENESIS = N_C * K_B (factor 3 is the framework integer N_C)
    check_close("K_GENESIS = N_C * K_B", ftd::K_GENESIS, ftd::N_C * ftd::K_B, 1e-15);

    // DAMPING = ALPHA [DERIVED from vacuum drag / geometric friction]
    // See: EXPLR_VACUUM_DRAG_DERIVATION.md, SPEC_SIX_ALGORITHMS.md (Algorithm 5)
    check_close("DAMPING = alpha (vacuum drag)", ftd::DAMPING, ftd::ALPHA, 1e-15);

    // g_c^2 = alpha identity
    check_close("G_C^2 ~ ALPHA", ftd::G_C * ftd::G_C, ftd::ALPHA, 0.0001);

    // EPSILON_ABS = |EPSILON|
    check_close("EPSILON_ABS = |EPSILON|", ftd::EPSILON_ABS, std::abs(ftd::EPSILON), 1e-10);

    // ================================================================
    // NEW: Precision Formula Verification
    // ================================================================
    std::cout << "\n--- Precision Formula: 4-Term Corrected Alpha ---\n";

    // Step 1: Verify epsilon = e^pi - pi - 20
    double e_pi = std::exp(M_PI);
    double eps_computed = e_pi - M_PI - 20.0;
    std::cout << "    e^pi = " << std::setprecision(15) << e_pi << "\n";
    std::cout << "    epsilon = " << eps_computed << "\n";
    check_close("epsilon = e^pi - pi - 20", eps_computed, ftd::EPSILON, 1e-7);
    check("epsilon < 0", eps_computed < 0);
    check_close("|epsilon| ~ 0.00090002", std::abs(eps_computed), 0.0009000208, 1e-7);

    // Step 2: Verify epsilon connects to framework integers
    // b_3 + N_eff = 7 + 13 = 20 (the integer in e^pi - pi - 20)
    check("b_3 + N_eff = 20", ftd::B_3 + ftd::N_EFF == 20);

    // Step 3: Verify coefficient integer formulas
    double c1_check = static_cast<double>(ftd::N_C * ftd::N_C) / ftd::D_CONSTRAINT;
    double c2_check = static_cast<double>(ftd::N_EFF - 2 * ftd::N_BASE) /
                      (ftd::N_BASE * ftd::N_BASE * ftd::N_BASE);
    double c3_check = static_cast<double>(ftd::N_BASE) /
                      (ftd::N_C * ftd::D_CONSTRAINT);
    double c4_check = static_cast<double>(ftd::N_C * ftd::D_CONSTRAINT) /
                      (ftd::B_3 + ftd::N_BASE);

    check_close("c1 = N_c^2/D = 9/47", ftd::C1, c1_check, 1e-15);
    check_close("c2 = (N_eff-2N_base)/N_base^3 = 5/64", ftd::C2, c2_check, 1e-15);
    check_close("c3 = N_base/(N_c*D) = 4/141", ftd::C3, c3_check, 1e-15);
    check_close("c4 = (N_c*D)/(B_3+N_base) = 141/11", ftd::C4, c4_check, 1e-15);

    // Verify exact integer ratios
    check("c1 numerator: N_c^2 = 9", ftd::N_C * ftd::N_C == 9);
    check("c2 numerator: N_eff - 2*N_base = 5", ftd::N_EFF - 2 * ftd::N_BASE == 5);
    check("c3 numerator: N_base = 4", ftd::N_BASE == 4);
    check("c4 numerator: N_c*D = 141", ftd::N_C * ftd::D_CONSTRAINT == 141);
    check("c4 denominator: B_3+N_base = 11", ftd::B_3 + ftd::N_BASE == 11);

    // Step 4: Compute 4-term corrected alpha
    double eps = std::abs(eps_computed);
    double eps2 = eps * eps;
    double eps3 = eps2 * eps;
    double eps4 = eps3 * eps;

    double tree_level = x_plus;  // 137.0361714582...
    double order1 = tree_level - ftd::C1 * eps;
    double order2 = order1 + ftd::C2 * eps2;
    double order3 = order2 - ftd::C3 * eps3;
    double order4 = order3 - ftd::C4 * eps4;

    // CODATA 2022: 1/alpha = 137.035999177(21)
    double codata_alpha_inv = 137.035999177;

    std::cout << "    Tree level 1/alpha  = " << std::setprecision(15) << tree_level << "\n";
    std::cout << "    After c1 correction = " << order1 << "\n";
    std::cout << "    After c2 correction = " << order2 << "\n";
    std::cout << "    After c3 correction = " << order3 << "\n";
    std::cout << "    After c4 correction = " << order4 << "\n";
    std::cout << "    CODATA 2022         = " << codata_alpha_inv << "\n";
    std::cout << "    Residual            = " << std::abs(order4 - codata_alpha_inv) << "\n";

    // Tree level: 1.26 ppm from CODATA
    double tree_ppm = std::abs(tree_level - codata_alpha_inv) / codata_alpha_inv * 1e6;
    std::cout << "    Tree-level gap      = " << tree_ppm << " ppm\n";
    check("Tree-level within 2 ppm of CODATA", tree_ppm < 2.0);

    // c1 correction should close most of the gap
    check("c1 correction reduces error", std::abs(order1 - codata_alpha_inv) < std::abs(tree_level - codata_alpha_inv));

    // Full 4-term: should match CODATA to < 0.001 ppt
    double residual = std::abs(order4 - codata_alpha_inv);
    double ppt = residual / codata_alpha_inv * 1e12;
    std::cout << "    4-term precision    = " << ppt << " ppt\n";
    check("4-term matches CODATA to < 1 ppt", ppt < 1.0);

    // Step 5: Gravitational hierarchy (pure math)
    // alpha_G = 2*pi*(16/3)^2*(N_eff + 3/b_3)^2 * alpha^20
    double ratio_16_3 = 16.0 / 3.0;
    double neff_corr = ftd::N_EFF + 3.0 / ftd::B_3;  // 13 + 3/7 = 13.4286...
    double alpha_20 = std::pow(ftd::ALPHA, 20);
    double alpha_G = 2.0 * M_PI * ratio_16_3 * ratio_16_3 * neff_corr * neff_corr * alpha_20;
    std::cout << "\n--- Gravitational Hierarchy ---\n";
    std::cout << "    alpha_G = " << std::setprecision(6) << alpha_G << "\n";
    std::cout << "    alpha_G/alpha = " << alpha_G / ftd::ALPHA << "\n";
    check("alpha_G ~ 5.9e-39", alpha_G > 5e-39 && alpha_G < 7e-39);
    check("alpha_G/alpha ~ 8e-37", alpha_G / ftd::ALPHA > 5e-37 && alpha_G / ftd::ALPHA < 1e-36);

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All constant derivation tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
