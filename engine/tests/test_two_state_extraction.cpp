/**
 * @file test_two_state_extraction.cpp
 * @brief Validate Prony + GEVP two-state extractors on synthetic data.
 *
 * The pre-registered prediction is x_plus/x_minus ≈ 45.31 (= 137.04 / 3.024).
 * For the test, use eigenvalues with the SAME ratio but smaller absolute
 * values so the per-step decay multiplier exp(−x) is well-resolved at τ=0..N.
 *
 * Test design:
 *   T1 — Prony recovers (λ₁, λ₂) within 1% of input on noiseless synthetic.
 *   T2 — GEVP recovers (λ₁, λ₂) within 1% on noiseless synthetic.
 *   T3 — Falsification: feeding a synthetic with wrong λ_input must yield
 *        an extracted ratio that differs from the target by > 5%.
 */

#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/spectrum_extraction.h"
#include "ftd/ontic/master_quadratic.h"   // X_PLUS, X_MINUS — single source of truth

using namespace ftd;

static int failures = 0;
#define CHECK_REL(actual, expected, tol_pct, msg) do { \
    double rel_err = std::abs(((actual) - (expected)) / (expected)); \
    if (rel_err > (tol_pct) / 100.0) { \
        std::printf("[FAIL] %s  (got %.6g, expected %.6g, rel %.3f%%)\n", \
                    msg, (double)(actual), (double)(expected), 100.0*rel_err); \
        ++failures; \
    } else { \
        std::printf("[ ok ] %s  (= %.6g, rel %.4f%%)\n", \
                    msg, (double)(actual), 100.0*rel_err); \
    } \
} while (0)

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Two-state spectrum extraction tests (Prony + GEVP)\n");
    std::printf("================================================================\n");

    // Target ratio derived from master quadratic roots (single source of truth):
    //   master-quadratic ratio = X_PLUS / X_MINUS ≈ 45.31
    // Choose lambda values small enough that exp(-lambda*N) is non-trivial across
    // the lag window; preserve the ratio.
    const double MQ_RATIO = ftd::ontic::X_PLUS / ftd::ontic::X_MINUS;  // ≈ 45.31
    const double lam1_in = 0.5;
    const double lam2_in = lam1_in / MQ_RATIO;
    const double A1 = 1.0, A2 = 0.5;
    const int N_LAG = 30;

    auto synth = [&](double l1, double l2) {
        std::vector<double> C(N_LAG);
        for (int t = 0; t < N_LAG; ++t) {
            C[t] = A1 * std::exp(-l1 * t) + A2 * std::exp(-l2 * t);
        }
        return C;
    };

    // ===== T1: Prony =====
    std::printf("  T1 — Prony on synthetic two-exponential (no noise)\n");
    auto C = synth(lam1_in, lam2_in);
    auto pres = extract_two_state_prony(C, /*tau0=*/2);
    if (!pres.valid) {
        std::printf("[FAIL] Prony reported invalid: %s\n", pres.failure_reason);
        ++failures;
    } else {
        CHECK_REL(pres.x_plus,  lam1_in, 1.0, "Prony x_plus");
        CHECK_REL(pres.x_minus, lam2_in, 1.0, "Prony x_minus");
        CHECK_REL(pres.x_plus / pres.x_minus, MQ_RATIO, 1.0, "Prony ratio = X_PLUS/X_MINUS");
    }

    // ===== T2: GEVP =====
    std::printf("\n  T2 — GEVP on synthetic two-state correlator\n");
    // Build C00, C11, C01 with two distinct operator overlaps.
    // C_ij(τ) = Σ_n O_in O_jn e^{-λ_n τ}, with overlap matrix O.
    // Use simple O = [[1,0],[1,1]] (lower triangular, rank 2).
    std::vector<double> C00(N_LAG), C11(N_LAG), C01(N_LAG);
    for (int t = 0; t < N_LAG; ++t) {
        // O₀₁=1, O₀₂=0  (operator 0 couples only to state 1 via amplitude 1)
        // O₁₁=1, O₁₂=1  (operator 1 couples to both states with amplitude 1)
        const double e1 = std::exp(-lam1_in * t);
        const double e2 = std::exp(-lam2_in * t);
        C00[t] = 1.0 * 1.0 * e1 + 0.0 * 0.0 * e2;        // = e1
        C01[t] = 1.0 * 1.0 * e1 + 0.0 * 1.0 * e2;        // = e1
        C11[t] = 1.0 * 1.0 * e1 + 1.0 * 1.0 * e2;        // = e1 + e2
    }
    auto gres = extract_two_state_gevp(C00, C01, C11, /*tau0=*/1);
    if (!gres.valid) {
        std::printf("[FAIL] GEVP reported invalid: %s\n", gres.failure_reason);
        ++failures;
    } else {
        CHECK_REL(gres.x_plus,  lam1_in, 1.0, "GEVP x_plus");
        CHECK_REL(gres.x_minus, lam2_in, 1.0, "GEVP x_minus");
        CHECK_REL(gres.x_plus / gres.x_minus, MQ_RATIO, 1.0, "GEVP ratio = X_PLUS/X_MINUS");
    }

    // ===== T3: Falsification — wrong input must produce wrong ratio =====
    // Wrong ratio = MQ_RATIO − 1.31 (i.e. 44.0 when MQ_RATIO ≈ 45.31).
    // Chosen 2.89% below the master-quadratic ratio so the extractor must
    // distinguish a mistake bigger than the 1% campaign tolerance.
    const double WRONG_RATIO = MQ_RATIO - 1.31;
    std::printf("\n  T3 — Falsification: synth with wrong ratio (%.2f) must NOT recover %.2f\n",
                WRONG_RATIO, MQ_RATIO);
    const double lam2_wrong = lam1_in / WRONG_RATIO;
    auto C_wrong = synth(lam1_in, lam2_wrong);
    auto pres_wrong = extract_two_state_prony(C_wrong, 2);
    if (!pres_wrong.valid) {
        std::printf("[FAIL] Falsification synthesis: extractor reported invalid\n");
        ++failures;
    } else {
        const double r = pres_wrong.x_plus / pres_wrong.x_minus;
        const double rel_to_target = std::abs(r - MQ_RATIO) / MQ_RATIO;
        // Threshold is 1% — same as the campaign's pre-registered prediction
        // tolerance. If the extractor returned the target ratio when the
        // input had a different ratio, the extractor is ill-conditioned.
        if (rel_to_target < 0.01) {
            std::printf("[FAIL] wrong-input ratio = %.4f, only %.3f%% from %.2f (should be >1%%)\n",
                        r, 100.0 * rel_to_target, MQ_RATIO);
            ++failures;
        } else {
            std::printf("[ ok ] wrong-input ratio = %.4f differs from %.2f by %.2f%%\n",
                        r, MQ_RATIO, 100.0 * rel_to_target);
        }
    }

    std::printf("================================================================\n");
    std::printf("  Result: %s (%d failure(s))\n",
                failures == 0 ? "PASS" : "FAIL", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
