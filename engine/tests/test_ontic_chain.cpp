/**
 * Test: Ontic Derivation Chain — γ → Γ(1/4) → ϖ → G* → α → all physics
 *
 * Pure mathematics. No simulation. Verifies that the complete derivation
 * chain from transcendental seeds to measurable physics is self-consistent.
 * Every constant computable from {D=3, ϖ} alone.
 *
 * Theory references:
 *   - FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md (complete constant chain, PRIMARY)
 *   - DERIV_DISCRETE_CONTINUOUS_BRIDGE.md (master quadratic bridge)
 *   - SPEC_FTD_LAGRANGIAN.md             (from axioms through G* to physics)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/ontic.h"

int main() {
    // Run the built-in ontic audit (Layers 0-7)
    int failures = ftd::ontic::ontic_audit();

    // ================================================================
    // Additional self-consistency checks beyond the audit
    // ================================================================
    std::cout << "\n--- Additional Self-Consistency ---\n";

    auto check = [&](const char* name, bool ok) {
        if (ok) { std::cout << "  PASS  " << name << "\n"; }
        else    { std::cout << "  FAIL  " << name << "\n"; ++failures; }
    };

    auto check_close = [&](const char* name, double a, double b, double tol) {
        bool ok = std::abs(a - b) < tol;
        if (ok) { std::cout << "  PASS  " << name << "\n"; }
        else {
            std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                      << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
            ++failures;
        }
    };

    // 1. Everything from D=3 alone
    // N_BASE = 2^((D+1)/2) = 2^2 = 4
    int n_base_from_d = 1;
    for (int i = 0; i < (ftd::ontic::D_SPATIAL + 1) / 2; ++i) n_base_from_d *= 2;
    check("N_BASE = 2^((D+1)/2) from D=3", n_base_from_d == ftd::ontic::N_BASE);

    // COEFFICIENT = N_BASE^2 = 16
    check("COEFFICIENT = N_BASE^2", ftd::ontic::N_BASE * ftd::ontic::N_BASE == ftd::ontic::COEFFICIENT);

    // 2. The integer cascade: x_- → N_c → N_gen → N_f → b_3 → N_eff → D
    check("N_gen = N_c", ftd::ontic::N_GEN == ftd::ontic::N_C);
    check("N_f = 2*N_gen", ftd::ontic::N_F == 2 * ftd::ontic::N_GEN);
    int b3 = (11 * ftd::ontic::N_C - 2 * ftd::ontic::N_F) / 3;
    check("b_3 = (11*N_c - 2*N_f)/3", b3 == ftd::ontic::B_3);
    check("N_eff = b_3 + 2*N_c", ftd::ontic::B_3 + 2 * ftd::ontic::N_C == ftd::ontic::N_EFF);
    check("D = N_c*N_base^2 - 1", ftd::ontic::N_C * ftd::ontic::N_BASE * ftd::ontic::N_BASE - 1 == ftd::ontic::D_CONSTRAINT);

    // 3. Precision coefficient integer formulas match their names
    check("c1 num: N_c^2 = 9", ftd::ontic::N_C * ftd::ontic::N_C == 9);
    check("c1 den: D = 47", ftd::ontic::D_CONSTRAINT == 47);
    check("c2 num: N_eff - 2*N_base = 5", ftd::ontic::N_EFF - 2 * ftd::ontic::N_BASE == 5);
    check("c2 den: N_base^3 = 64", ftd::ontic::N_BASE * ftd::ontic::N_BASE * ftd::ontic::N_BASE == 64);
    check("c3 num: N_base = 4", ftd::ontic::N_BASE == 4);
    check("c3 den: N_c*D = 141", ftd::ontic::N_C * ftd::ontic::D_CONSTRAINT == 141);
    check("c4 num: N_c*D = 141", ftd::ontic::N_C * ftd::ontic::D_CONSTRAINT == 141);
    check("c4 den: b_3+N_base = 11", ftd::ontic::B_3 + ftd::ontic::N_BASE == 11);

    // 4. Cross-layer consistency: G* connects layers 1 and 5
    // G*^2 = x_+ * x_- / 16  (from Vieta: x_+*x_- = 16*G*^3, so G* = x_+*x_-/16/G*^2... use direct)
    double product = ftd::ontic::X_PLUS * ftd::ontic::X_MINUS;
    double gstar_cubed = product / 16.0;
    double gstar_from_roots = std::cbrt(gstar_cubed);
    check_close("G* from root product", gstar_from_roots, ftd::ontic::G_STAR, 1e-6);

    // 5. The hierarchy: alpha^20 factor
    double alpha_20 = std::pow(ftd::ontic::ALPHA, 20);
    std::cout << "    alpha^20 = " << std::setprecision(6) << alpha_20 << "\n";
    // This should be O(10^-43) — explaining why gravity is 10^39 times weaker than EM
    check("alpha^20 ~ 10^-43 (hierarchy explanation)", alpha_20 > 1e-44 && alpha_20 < 1e-42);

    // 6. The damping identification: DAMPING = alpha [IMPOSED — ASSUMP.6]
    // See: EXPLR_VACUUM_DRAG_DERIVATION.md, SPEC_SIX_ALGORITHMS.md (Algorithm 5)
    check("DAMPING = ALPHA exactly", ftd::ontic::DAMPING == ftd::ontic::ALPHA);
    std::cout << "    DAMPING = " << std::setprecision(10) << ftd::ontic::DAMPING
              << " = alpha (vacuum drag / geometric friction)\n";

    // 7. CFL bound check — C_WAVE = 1/sqrt(3) saturates the bound exactly
    check("C_WAVE^2 <= 1/3 (CFL stability)", ftd::ontic::C_WAVE * ftd::ontic::C_WAVE <= 1.0 / 3.0 + 1e-15);
    check("C_WAVE^2 * 12 <= 4 (eigenvalue bound)", ftd::ontic::C_WAVE * ftd::ontic::C_WAVE * 12.0 <= 4.0 + 1e-12);

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All ontic chain tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
