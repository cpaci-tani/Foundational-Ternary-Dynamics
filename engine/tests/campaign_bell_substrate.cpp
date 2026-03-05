/**
 * Campaign: Bell Inequality — Substrate Level (Phase 3 — Quantum Mechanics)
 *
 * Validates that the FTD lattice (local deterministic substrate) produces
 * S ≤ 2 in a CHSH-type Bell test. This is a THEOREM for any local hidden
 * variable model, and FTD is explicitly local deterministic (POSTULATE 4+5).
 *
 * Theory: For pair-produced particles with anti-correlated flux J_A = -J_B,
 * the "measurement outcome" along direction â is defined as sign(J · â).
 * With uniformly random hidden variable (flux direction in 2D), the
 * correlation E(θ) = -(1 - 2θ/π) [linear sawtooth], giving S = 2 exactly.
 * (The -(2/π)cos(θ) formula applies to 3D hidden variables.)
 *
 * Protocol:
 *   1. Create N EPR pairs with varied flux orientations (uniform over sphere)
 *   2. For each pair, compute "measurement outcomes" along 4 detector bases
 *   3. Compute correlations E(a,b), E(a,b'), E(a',b), E(a',b')
 *   4. Compute CHSH parameter S = |E(a,b) - E(a,b') + E(a',b) + E(a',b')|
 *
 * Detector angles (standard CHSH-optimal): a=0°, a'=45°, b=22.5°, b'=67.5°
 *
 * Checks:
 *   BS1: S ≤ 2.0 (Bell-CHSH bound for local hidden variables)
 *   BS2: |E(a,b)| ≤ 1 for all angle pairs (valid correlations)
 *   BS3: E(a,a) ≈ -1 (perfect anti-correlation when same basis)
 *   BS4: Correlation follows 2D classical prediction -(1-2θ/π)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
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

// "Measurement" of particle flux along direction (cos θ, sin θ, 0)
// Returns +1 or -1 based on sign of projection
int measure(const ftd::Vec3& flux, double angle_rad) {
    double projection = flux.x * std::cos(angle_rad) + flux.y * std::sin(angle_rad);
    return (projection >= 0) ? +1 : -1;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Bell Substrate (Phase 3) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int N_PAIRS = 10000;

    // CHSH-optimal detector angles (in x-y plane)
    double a  = 0.0;                    // 0°
    double a_ = ftd::PI / 4.0;         // 45°
    double b  = ftd::PI / 8.0;         // 22.5°
    double b_ = 3.0 * ftd::PI / 8.0;   // 67.5°

    std::cout << "\n--- Setup ---\n";
    std::cout << "  N_pairs: " << N_PAIRS << "\n";
    std::cout << "  Detector a: " << (a * 180 / ftd::PI) << "°\n";
    std::cout << "  Detector a': " << (a_ * 180 / ftd::PI) << "°\n";
    std::cout << "  Detector b: " << (b * 180 / ftd::PI) << "°\n";
    std::cout << "  Detector b': " << (b_ * 180 / ftd::PI) << "°\n\n";

    // Generate pairs with uniformly random flux direction in x-y plane
    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * ftd::PI);

    // Accumulate correlation products
    double sum_ab  = 0.0;
    double sum_ab_ = 0.0;
    double sum_a_b = 0.0;
    double sum_a_b_= 0.0;
    double sum_aa  = 0.0;  // Same-basis test

    for (int i = 0; i < N_PAIRS; ++i) {
        // Random flux direction for this pair (the "hidden variable")
        double phi = angle_dist(rng);
        double amp = ftd::K_B;

        ftd::Vec3 flux_A = {amp * std::cos(phi), amp * std::sin(phi), 0.0};
        ftd::Vec3 flux_B = {-flux_A.x, -flux_A.y, -flux_A.z};  // Anti-correlated

        // Measure particle A along directions a and a'
        int A_a  = measure(flux_A, a);
        int A_a_ = measure(flux_A, a_);

        // Measure particle B along directions b and b'
        int B_b  = measure(flux_B, b);
        int B_b_ = measure(flux_B, b_);
        int B_a  = measure(flux_B, a);  // Same basis as A for anti-correlation test

        // Accumulate products
        sum_ab   += A_a  * B_b;
        sum_ab_  += A_a  * B_b_;
        sum_a_b  += A_a_ * B_b;
        sum_a_b_ += A_a_ * B_b_;
        sum_aa   += A_a  * B_a;
    }

    // Compute correlations
    double E_ab  = sum_ab  / N_PAIRS;
    double E_ab_ = sum_ab_ / N_PAIRS;
    double E_a_b = sum_a_b / N_PAIRS;
    double E_a_b_= sum_a_b_/ N_PAIRS;
    double E_aa  = sum_aa  / N_PAIRS;

    // CHSH parameter
    double S = std::abs(E_ab - E_ab_ + E_a_b + E_a_b_);

    std::cout << "--- Correlations ---\n";
    std::cout << "  E(a,b)   = " << E_ab << "\n";
    std::cout << "  E(a,b')  = " << E_ab_ << "\n";
    std::cout << "  E(a',b)  = " << E_a_b << "\n";
    std::cout << "  E(a',b') = " << E_a_b_ << "\n";
    std::cout << "  E(a,a)   = " << E_aa << " (theory: -1.0)\n";
    std::cout << "\n  CHSH S = " << S << " (classical bound: 2.0)\n";

    // 2D classical predictions: E(a,b) = -(1 - 2|θ|/π) [linear sawtooth]
    // This is the correct formula for uniformly random hidden variable in 2D.
    // (The -(2/π)cos(θ) formula applies to 3D hidden variables.)
    auto E_2d = [](double theta) {
        double t = std::fmod(std::abs(theta), 2.0 * ftd::PI);
        if (t > ftd::PI) t = 2.0 * ftd::PI - t;
        return -(1.0 - 2.0 * t / ftd::PI);
    };
    double E_ab_theory  = E_2d(b - a);
    double E_ab_theory_ = E_2d(b_ - a);
    double E_a_b_theory = E_2d(b - a_);
    double E_a_b_theory_= E_2d(b_ - a_);
    double S_theory = std::abs(E_ab_theory - E_ab_theory_ + E_a_b_theory + E_a_b_theory_);

    std::cout << "\n--- Classical Theory ---\n";
    std::cout << "  E(a,b)   theory = " << E_ab_theory << "\n";
    std::cout << "  E(a,b')  theory = " << E_ab_theory_ << "\n";
    std::cout << "  E(a',b)  theory = " << E_a_b_theory << "\n";
    std::cout << "  E(a',b') theory = " << E_a_b_theory_ << "\n";
    std::cout << "  S_theory = " << S_theory << "\n";

    // ----------------------------------------------------------------
    // BS1: S ≤ 2.0 (Bell-CHSH bound)
    // ----------------------------------------------------------------
    check("BS1: CHSH S <= 2.0 (local hidden variable bound)", S <= 2.0 + 1e-6);

    // ----------------------------------------------------------------
    // BS2: All correlations bounded by ±1
    // ----------------------------------------------------------------
    bool all_bounded = (std::abs(E_ab) <= 1.0 + 1e-6) &&
                       (std::abs(E_ab_) <= 1.0 + 1e-6) &&
                       (std::abs(E_a_b) <= 1.0 + 1e-6) &&
                       (std::abs(E_a_b_) <= 1.0 + 1e-6);
    check("BS2: |E(a,b)| <= 1 for all angle pairs", all_bounded);

    // ----------------------------------------------------------------
    // BS3: Perfect anti-correlation when same basis
    // ----------------------------------------------------------------
    std::cout << "\n  E(a,a) = " << E_aa << " (expect -1.0)\n";
    check("BS3: E(a,a) = -1.0 (perfect anti-correlation)", std::abs(E_aa + 1.0) < 0.01);

    // ----------------------------------------------------------------
    // BS4: Correlations match classical prediction
    // ----------------------------------------------------------------
    double max_err = std::max({
        std::abs(E_ab - E_ab_theory),
        std::abs(E_ab_ - E_ab_theory_),
        std::abs(E_a_b - E_a_b_theory),
        std::abs(E_a_b_ - E_a_b_theory_)
    });
    std::cout << "  Max correlation error vs classical theory: " << max_err << "\n";
    check("BS4: Correlations match -(1-2|theta|/pi) within 5%",
          max_err < 0.05);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  NOTE: S <= 2 is the EXPECTED result. FTD substrate is local\n";
    std::cout << "  deterministic. QM's S > 2 is an aggregate property — the\n";
    std::cout << "  substrate-to-aggregate transition is documented in\n";
    std::cout << "  DERIV_OBSERVER_BELL_MECHANISM.md but not yet implemented\n";
    std::cout << "  in engine dynamics.\n";
    std::cout << "================================================================\n";
    return failures;
}
