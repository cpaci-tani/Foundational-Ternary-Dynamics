/**
 * @file test_master_quadratic_identities.cpp
 * @brief Numerical verification of the bare algebraic content of the master quadratic.
 *
 * The master quadratic is x^2 - 16G*^2 x + 16G*^3 = 0 with roots x_+ = 137.036..., x_- = 3.024...
 *
 * This test verifies the algebraic identities that follow from Vieta's formulas,
 * specifically the cleanest single-line statement:
 *
 *   α + 1/N_c = 1/G*   (when x_+ = 1/α and x_- = N_c)
 *
 * plus the three-means (AM, GM, HM) geometric-progression structure and the
 * normalized-form small-parameter content.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // High-precision G* from Gamma ratio
    // Using tabulated Gamma(1/4) and Gamma(3/4) to double precision
    const double GAMMA_1_4 = 3.62560990822190831194;
    const double GAMMA_3_4 = 1.22541670246517764512;
    const double G_star = GAMMA_1_4 / GAMMA_3_4;   // 2.9586769...

    std::printf("================================================================\n");
    std::printf("  Master Quadratic — Bare Algebraic Identities\n");
    std::printf("================================================================\n\n");
    std::printf("  G* = Γ(1/4)/Γ(3/4) = %.15f\n", G_star);
    std::printf("  1/G* = %.15f\n\n", 1.0 / G_star);

    // Master quadratic coefficients
    const double A = 16.0 * G_star * G_star;        // linear coefficient  (sum of roots)
    const double B = 16.0 * G_star * G_star * G_star; // constant term     (product of roots)

    std::printf("  Coefficients:\n");
    std::printf("    A = 16 G*^2 = %.12f\n", A);
    std::printf("    B = 16 G*^3 = %.12f\n", B);

    // Solve the quadratic
    const double disc = A * A - 4.0 * B;
    const double sqrt_disc = std::sqrt(disc);
    const double x_plus  = 0.5 * (A + sqrt_disc);
    const double x_minus = 0.5 * (A - sqrt_disc);

    std::printf("\n  Roots (exact solution):\n");
    std::printf("    x+ = %.12f    (identified with 1/α under SP4)\n", x_plus);
    std::printf("    x- = %.12f    (identified with N_c under SP4)\n", x_minus);

    // --- Vieta identities (should be exact to machine precision) ---
    std::printf("\n--- Vieta identities (all exact by construction) ---\n");

    const double sum   = x_plus + x_minus;
    const double prod  = x_plus * x_minus;
    const double recip = 1.0/x_plus + 1.0/x_minus;

    std::printf("  V1: x+ + x-           = %.12f   vs 16G*^2 = %.12f  (err %.2e)\n",
                sum, A, std::abs(sum - A));
    std::printf("  V2: x+ · x-           = %.12f   vs 16G*^3 = %.12f  (err %.2e)\n",
                prod, B, std::abs(prod - B));
    std::printf("  V3: 1/x+ + 1/x-       = %.15f\n", recip);
    std::printf("                   1/G* = %.15f  (err %.2e)\n",
                1.0/G_star, std::abs(recip - 1.0/G_star));

    // --- The central physical identity (under SP4) ---
    std::printf("\n--- The α + 1/N_c = 1/G* identity ---\n");

    const double alpha    = 1.0 / x_plus;          // under SP4
    const double N_c_root = x_minus;               // under SP4 strict
    const double lhs = alpha + 1.0 / N_c_root;
    const double rhs = 1.0 / G_star;

    std::printf("  α = 1/x+     = %.15f\n", alpha);
    std::printf("  1/x-         = %.15f  (= 1/N_c under SP4 strict)\n", 1.0/N_c_root);
    std::printf("  α + 1/x-     = %.15f\n", lhs);
    std::printf("  1/G*         = %.15f\n", rhs);
    std::printf("  |diff|       = %.2e   <-- EXACT by Vieta\n", std::abs(lhs - rhs));

    // --- Three means and their geometric progression ---
    std::printf("\n--- Three means of (x+, x-) ---\n");

    const double AM = 0.5 * sum;                       // arithmetic mean
    const double GM = std::sqrt(prod);                 // geometric mean
    const double HM = 2.0 * prod / sum;                // harmonic mean

    std::printf("  AM = (x+ + x-)/2 = %.8f   vs 8G*^2 = %.8f\n",
                AM, 8.0 * G_star * G_star);
    std::printf("  GM = sqrt(x+·x-) = %.8f   vs 4G*^(3/2) = %.8f\n",
                GM, 4.0 * std::pow(G_star, 1.5));
    std::printf("  HM = 2x+·x-/(x++x-) = %.8f  vs 2G* = %.8f\n",
                HM, 2.0 * G_star);

    const double ratio_AM_GM = AM / GM;
    const double ratio_GM_HM = GM / HM;
    const double expected_ratio = 2.0 * std::sqrt(G_star);

    std::printf("  AM/GM = %.8f,  GM/HM = %.8f,  2√G* = %.8f\n",
                ratio_AM_GM, ratio_GM_HM, expected_ratio);
    std::printf("  (All three equal → AM, GM, HM in geometric progression)\n");

    // --- Normalized form: w^2 - w + ε = 0 where w = x/(16G*²) ---
    std::printf("\n--- Normalized form (w = x / 16G*²) ---\n");

    const double eps = 1.0 / (16.0 * G_star);   // small parameter
    const double w_plus  = x_plus / A;
    const double w_minus = x_minus / A;

    std::printf("  ε = 1/(16G*) = %.10f  (the small parameter)\n", eps);
    std::printf("  w+ = x+ / 16G*² = %.10f   (≈ 1 − ε − ε² − ... ; actual 1−ε = %.10f)\n",
                w_plus, 1.0 - eps);
    std::printf("  w- = x- / 16G*² = %.10f   (≈ ε + ε² + ... ; actual = %.10f)\n",
                w_minus, eps + eps * eps);
    std::printf("  w+ + w-           = %.12f  (should be 1 by construction)\n",
                w_plus + w_minus);
    std::printf("  w+ · w-           = %.12f  vs ε = %.12f (exact)\n",
                w_plus * w_minus, eps);

    // --- The hierarchy x+/x- ---
    std::printf("\n--- Hierarchy content ---\n");

    const double hierarchy = x_plus / x_minus;
    const double pred_hier = 1.0/eps - 1.0;  // w+/w- ≈ (1-ε)/ε = 1/ε − 1 at leading order

    std::printf("  x+ / x-          = %.8f\n", hierarchy);
    std::printf("  1/ε − 1 = 16G* − 1 = %.8f  (leading-order prediction)\n", pred_hier);
    std::printf("  (= EM/QCD-like scale separation)\n");

    std::printf("\n================================================================\n");
    std::printf("  SUMMARY — what the master quadratic actually says:\n");
    std::printf("================================================================\n");
    std::printf("  (1) Two numbers (x+, x-) with sum 16G*^2 and product 16G*^3.\n");
    std::printf("  (2) Reciprocal sum 1/x+ + 1/x- = 1/G* exact (Vieta).\n");
    std::printf("  (3) In normalized units w = x/(16G*²): w² - w + ε = 0 with ε = 1/(16G*).\n");
    std::printf("  (4) Leading-order roots: w+ ≈ 1, w- ≈ ε.\n");
    std::printf("  (5) THE central identity under SP4: α + 1/N_c = 1/G*.\n");
    std::printf("\n  The whole polynomial is a Vieta-encoding of:\n");
    std::printf("      α + 1/N_c = 1/G*    (reciprocal sum)\n");
    std::printf("      α · N_c = 1/(16 G*³) (reciprocal product)\n");
    std::printf("================================================================\n");

    return 0;
}
