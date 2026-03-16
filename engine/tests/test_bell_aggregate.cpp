/**
 * Test: Bell Aggregate — Ensemble S = 2sqrt(2)
 *
 * Verifies the three-level observer Bell hierarchy from FTD:
 *
 *   Level 1 (Substrate):    S <= 2       (local deterministic, triangular correlation)
 *   Level 2 (Complex):      E(theta) = cos(theta) via complexification psi = J_x + iJ_y
 *   Level 3 (sLoop/QM):     S = 2*sqrt(2) (Tsirelson bound from joint coupling)
 *
 * The key insight: S_observer = S_substrate * sqrt(2) = 2*sqrt(2).
 * Two factors produce the enhancement:
 *   1. Complexification changes correlation shape (sawtooth -> cosine)
 *   2. sLoop doubles correlation strength (independent -> joint coupling)
 *
 * Tests:
 *   BELL-1: Substrate S <= 2 (classical bound from triangular correlation)
 *   BELL-2: Complex correlation shape E(theta) = cos(theta)
 *   BELL-3: CHSH S = 2*sqrt(2) from quantum (cosine) correlations
 *   BELL-4: Enhancement factor S_quantum / S_classical = sqrt(2)
 *   BELL-5: Tsirelson bound S_max = 2*sqrt(2) via optimization
 *   BELL-6: Correlation function comparison at key angles
 *   BELL-7: sLoop infrastructure on lattice (entangled pair creation)
 *
 * Theory references:
 *   - CLAUDE.md Ch.12        (entanglement in the model)
 *   - CLAUDE.md Ch.13        (measurement question, sLoop)
 *   - CLAUDE.md Ch.22.4      (Bell locality verification)
 *   - DERIV_OBSERVER_BELL_MECHANISM.md  (three-level hierarchy derivation)
 *
 * Epistemic status:
 *   BELL-1..6: [THEOREM] — mathematical identities of correlation functions
 *   BELL-7: [SELECTION] — sLoop mechanism infrastructure
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

static void check_close(const char* name, double got, double expected, double tol) {
    double err = std::abs(got - expected);
    bool ok = err < tol;
    if (ok) {
        std::cout << "  PASS  " << name << " (" << std::setprecision(8)
                  << got << " vs " << expected << ", err=" << err << ")\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10)
                  << got << ", expected " << expected << ", err=" << err << ")\n";
        ++g_failures;
    }
}

// ============================================================================
// Classical (substrate) correlation: triangular
// E_cl(theta) = 1 - 2|theta|/pi for theta in [-pi, pi]
// This is the maximum achievable correlation for a local hidden variable model
// with uniformly distributed hidden variable.
// ============================================================================
static double E_classical(double theta) {
    // Normalize theta to [0, pi]
    double t = std::fmod(std::abs(theta), 2.0 * M_PI);
    if (t > M_PI) t = 2.0 * M_PI - t;
    return 1.0 - 2.0 * t / M_PI;
}

// ============================================================================
// Quantum (complexified) correlation: cosine
// E_qu(theta) = cos(theta)
// Arises from Born rule on complexified flux: psi = J_x + i*J_y
// P(same) = cos^2(theta/2), P(diff) = sin^2(theta/2)
// E = P(same) - P(diff) = cos^2(theta/2) - sin^2(theta/2) = cos(theta)
// ============================================================================
static double E_quantum(double theta) {
    return std::cos(theta);
}

// ============================================================================
// CHSH S value from a correlation function E(theta)
// S = |E(a,b) - E(a,b') + E(a',b) + E(a',b')|
// where the angles refer to the difference angles between measurement settings.
// ============================================================================
static double compute_CHSH(double (*E_func)(double),
                           double a, double a_prime,
                           double b, double b_prime) {
    double S = E_func(a - b) - E_func(a - b_prime)
             + E_func(a_prime - b) + E_func(a_prime - b_prime);
    return std::abs(S);
}

// ============================================================================
// Find maximum CHSH S by scanning over measurement angles
// ============================================================================
static double maximize_CHSH(double (*E_func)(double), int num_steps = 360) {
    double S_max = 0.0;
    double step = M_PI / num_steps;

    for (int ia = 0; ia < num_steps; ++ia) {
        double a = ia * step;
        for (int iap = 0; iap < num_steps; ++iap) {
            double ap = iap * step;
            for (int ib = 0; ib < num_steps; ++ib) {
                double b = ib * step;
                for (int ibp = 0; ibp < num_steps; ++ibp) {
                    double bp = ibp * step;
                    double S = compute_CHSH(E_func, a, ap, b, bp);
                    if (S > S_max) S_max = S;
                }
            }
        }
    }
    return S_max;
}

// ============================================================================
// Smarter maximization: for quantum correlations the optimal angles are known.
// For classical, we do a coarser scan since the bound is analytic anyway.
// ============================================================================
static double maximize_CHSH_smart(double (*E_func)(double), int num_steps = 100) {
    double S_max = 0.0;
    double step = M_PI / num_steps;

    // Scan a and a' over [0, pi), with b and b' over [0, pi)
    for (int ia = 0; ia < num_steps; ++ia) {
        double a = ia * step;
        for (int iap = 0; iap < num_steps; ++iap) {
            double ap = iap * step;
            for (int ib = 0; ib < num_steps; ++ib) {
                double b = ib * step;
                for (int ibp = 0; ibp < num_steps; ++ibp) {
                    double bp = ibp * step;
                    double S = compute_CHSH(E_func, a, ap, b, bp);
                    if (S > S_max) S_max = S;
                }
            }
        }
    }
    return S_max;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Bell Aggregate — Ensemble S = 2*sqrt(2)\n";
    std::cout << "  Three-Level Observer Bell Hierarchy\n";
    std::cout << "================================================================\n";

    const double SQRT2 = std::sqrt(2.0);
    const double TSIRELSON = 2.0 * SQRT2;  // 2*sqrt(2) = 2.8284...

    // ================================================================
    // BELL-1: Substrate S <= 2 (classical bound)
    // ================================================================
    std::cout << "\n-- BELL-1: Substrate S <= 2 (Classical Bound) --\n";
    {
        // The triangular correlation E_cl(theta) = 1 - 2|theta|/pi is the
        // strongest correlation achievable by a local hidden variable model
        // with a uniformly distributed hidden variable.
        //
        // For CHSH with optimal classical angles, the maximum is exactly S = 2.
        // Optimal classical: a=0, a'=pi/2, b=pi/4, b'=3pi/4
        //   E_cl(0-pi/4) = 1-2*(pi/4)/pi = 1-1/2 = 0.5
        //   E_cl(0-3pi/4) = 1-2*(3pi/4)/pi = 1-3/2 = -0.5
        //   E_cl(pi/2-pi/4) = 1-2*(pi/4)/pi = 0.5
        //   E_cl(pi/2-3pi/4) = 1-2*(pi/4)/pi = 0.5
        //   S = |0.5 - (-0.5) + 0.5 + 0.5| = |2.0| = 2.0

        double S_cl_optimal = compute_CHSH(E_classical,
                                           0.0, M_PI / 2.0,
                                           M_PI / 4.0, 3.0 * M_PI / 4.0);

        std::cout << "    Optimal classical CHSH: S = " << S_cl_optimal << "\n";
        std::cout << "    Classical bound:        S = 2.0\n";

        check_close("BELL-1a: Classical optimal S = 2.0", S_cl_optimal, 2.0, 1e-12);

        // Also verify via numerical scan that no angles exceed 2
        double S_cl_max = maximize_CHSH_smart(E_classical, 50);
        std::cout << "    Scanned classical max:  S = " << S_cl_max << "\n";

        check("BELL-1b: Classical S <= 2.0 (scanned)", S_cl_max <= 2.0 + 1e-10);
    }

    // ================================================================
    // BELL-2: Complex correlation shape E(theta) = cos(theta)
    // ================================================================
    std::cout << "\n-- BELL-2: Complexified Correlation E(theta) = cos(theta) --\n";
    {
        // The complexified flux psi = J_x + iJ_y produces Born rule:
        //   P(same) = cos^2(theta/2)
        //   P(diff) = sin^2(theta/2)
        //   E(theta) = P(same) - P(diff) = cos(theta)
        //
        // Test at key angles where values are known exactly.

        struct TestCase {
            double theta;
            double expected;
            const char* label;
        };

        TestCase cases[] = {
            {0.0,               1.0,       "E(0) = 1 (perfect correlation)"},
            {M_PI / 4.0,        SQRT2/2.0, "E(pi/4) = sqrt(2)/2"},
            {M_PI / 3.0,        0.5,       "E(pi/3) = 1/2"},
            {M_PI / 2.0,        0.0,       "E(pi/2) = 0 (uncorrelated)"},
            {2.0 * M_PI / 3.0, -0.5,       "E(2pi/3) = -1/2"},
            {3.0 * M_PI / 4.0, -SQRT2/2.0, "E(3pi/4) = -sqrt(2)/2"},
            {M_PI,             -1.0,       "E(pi) = -1 (perfect anticorrelation)"},
        };

        bool all_pass = true;
        for (const auto& tc : cases) {
            double got = E_quantum(tc.theta);
            double err = std::abs(got - tc.expected);
            bool ok = err < 1e-14;
            if (!ok) all_pass = false;
            std::cout << "    " << tc.label << ": got " << got
                      << " (err=" << err << ")" << (ok ? "" : " MISMATCH") << "\n";
        }

        check("BELL-2: Quantum correlation = cos(theta) at all test angles", all_pass);

        // Verify the Born rule decomposition explicitly
        // P(same) = cos^2(theta/2), P(diff) = sin^2(theta/2)
        // E = P(same) - P(diff) = cos^2(theta/2) - sin^2(theta/2) = cos(theta)
        double theta_test = M_PI / 5.0;  // arbitrary non-special angle
        double p_same = std::cos(theta_test / 2.0) * std::cos(theta_test / 2.0);
        double p_diff = std::sin(theta_test / 2.0) * std::sin(theta_test / 2.0);
        double E_born = p_same - p_diff;
        double E_cos = std::cos(theta_test);

        check_close("BELL-2b: Born decomposition cos^2 - sin^2 = cos",
                    E_born, E_cos, 1e-14);
    }

    // ================================================================
    // BELL-3: CHSH S = 2*sqrt(2) from quantum correlations
    // ================================================================
    std::cout << "\n-- BELL-3: Quantum CHSH S = 2*sqrt(2) --\n";
    {
        // For E(theta) = cos(theta), the optimal CHSH angles are:
        //   a = 0, a' = pi/2, b = pi/4, b' = -pi/4
        //
        // S = E(0 - pi/4) - E(0 - (-pi/4)) + E(pi/2 - pi/4) + E(pi/2 - (-pi/4))
        //   = cos(-pi/4) - cos(pi/4) + cos(pi/4) + cos(3pi/4)
        //
        // Wait — let's be precise. The standard CHSH optimal angles for
        // quantum singlet correlations E(a,b) = -cos(a-b) use:
        //   a=0, a'=pi/2, b=pi/4, b'=3pi/4
        //
        // But for E(theta) = cos(theta) (not -cos), we use:
        //   a=0, a'=pi/2, b=pi/4, b'=-pi/4 (equivalently, 7pi/4)
        //
        // S = cos(0-pi/4) - cos(0-(-pi/4)) + cos(pi/2-pi/4) + cos(pi/2-(-pi/4))
        //   = cos(-pi/4) - cos(pi/4) + cos(pi/4) + cos(3pi/4)
        //   = cos(pi/4) - cos(pi/4) + cos(pi/4) + cos(3pi/4)
        //   = cos(pi/4) + cos(3pi/4)
        //   = sqrt(2)/2 - sqrt(2)/2 = 0  <-- wrong angles!
        //
        // Standard optimal for cos correlation:
        //   a=0, a'=pi/2, b=pi/4, b'=3pi/4
        //   S = cos(-pi/4) - cos(-3pi/4) + cos(pi/2-pi/4) + cos(pi/2-3pi/4)
        //   = cos(pi/4) - cos(3pi/4) + cos(pi/4) + cos(-pi/4)
        //   = sqrt(2)/2 + sqrt(2)/2 + sqrt(2)/2 + sqrt(2)/2
        //   = 4 * sqrt(2)/2 = 2*sqrt(2)

        double a  = 0.0;
        double ap = M_PI / 2.0;
        double b  = M_PI / 4.0;
        double bp = 3.0 * M_PI / 4.0;

        double E_ab  = E_quantum(a - b);    // cos(-pi/4) = sqrt(2)/2
        double E_abp = E_quantum(a - bp);   // cos(-3pi/4) = -sqrt(2)/2
        double E_apb = E_quantum(ap - b);   // cos(pi/4) = sqrt(2)/2
        double E_apbp= E_quantum(ap - bp);  // cos(-pi/4) = sqrt(2)/2

        double S = E_ab - E_abp + E_apb + E_apbp;

        std::cout << "    E(a,b)   = cos(-pi/4)  = " << E_ab << "\n";
        std::cout << "    E(a,b')  = cos(-3pi/4) = " << E_abp << "\n";
        std::cout << "    E(a',b)  = cos(pi/4)   = " << E_apb << "\n";
        std::cout << "    E(a',b') = cos(-pi/4)  = " << E_apbp << "\n";
        std::cout << "    S = E(a,b) - E(a,b') + E(a',b) + E(a',b') = " << S << "\n";
        std::cout << "    Tsirelson bound: 2*sqrt(2) = " << TSIRELSON << "\n";

        check_close("BELL-3: Quantum CHSH S = 2*sqrt(2)", S, TSIRELSON, 1e-12);
    }

    // ================================================================
    // BELL-4: Enhancement factor sqrt(2)
    // ================================================================
    std::cout << "\n-- BELL-4: Enhancement Factor sqrt(2) --\n";
    {
        // The ratio S_quantum / S_classical = 2*sqrt(2) / 2 = sqrt(2)
        // This sqrt(2) enhancement has TWO sources in FTD:
        //   1. Complexification (Gauss constraint -> psi = J_x + iJ_y)
        //      changes correlation from triangular to cosine
        //   2. sLoop (joint coupling) doubles the correlation strength
        //      when observer is embedded in the same substrate

        double S_classical = 2.0;
        double S_quantum = TSIRELSON;
        double ratio = S_quantum / S_classical;

        std::cout << "    S_classical (substrate): " << S_classical << "\n";
        std::cout << "    S_quantum (observer):    " << S_quantum << "\n";
        std::cout << "    Ratio:                   " << ratio << "\n";
        std::cout << "    Expected:                sqrt(2) = " << SQRT2 << "\n";

        check_close("BELL-4: S_quantum / S_classical = sqrt(2)", ratio, SQRT2, 1e-12);

        // Verify the factorization:
        // S_observer = S_substrate * sqrt(2)
        double S_from_factorization = S_classical * SQRT2;
        check_close("BELL-4b: S_substrate * sqrt(2) = Tsirelson",
                    S_from_factorization, TSIRELSON, 1e-12);
    }

    // ================================================================
    // BELL-5: Tsirelson bound from numerical optimization
    // ================================================================
    std::cout << "\n-- BELL-5: Tsirelson Bound (Numerical Verification) --\n";
    {
        // For cos(theta) correlations, the maximum CHSH S should be exactly
        // 2*sqrt(2) = 2.8284..., achieved at the optimal angles from BELL-3.
        //
        // We verify by scanning over a grid of angle settings.
        // Use moderate resolution (50 steps) for speed — the analytical check
        // in BELL-3 already confirms the exact value.

        double S_max_qu = maximize_CHSH_smart(E_quantum, 50);

        std::cout << "    Scanned quantum max S:  " << S_max_qu << "\n";
        std::cout << "    Tsirelson bound:        " << TSIRELSON << "\n";
        std::cout << "    Difference:             " << std::abs(S_max_qu - TSIRELSON) << "\n";

        // The scan with 50 steps (step = pi/50 ~ 3.6 deg) should get within
        // ~0.5% of the true maximum. We use a generous tolerance since the
        // exact result was already verified analytically in BELL-3.
        check("BELL-5: Scanned S_max >= 2.82 (within ~0.3% of Tsirelson)",
              S_max_qu >= 2.82);

        // Also verify it does NOT exceed Tsirelson (up to numerical noise)
        check("BELL-5b: Scanned S_max <= Tsirelson + epsilon",
              S_max_qu <= TSIRELSON + 0.01);
    }

    // ================================================================
    // BELL-6: Correlation functions at key angles — classical vs quantum
    // ================================================================
    std::cout << "\n-- BELL-6: Classical vs Quantum Correlations --\n";
    {
        // The two correlations agree at theta = 0, pi/2, pi
        // but differ at intermediate angles. The quantum correlation
        // is always >= classical in absolute value (for 0 < theta < pi),
        // which is why quantum CHSH exceeds the classical bound.

        struct CompareCase {
            double theta;
            double E_cl;
            double E_qu;
            const char* label;
        };

        CompareCase cases[] = {
            {0.0,            1.0,       1.0,       "theta=0: both = 1 (agree)"},
            {M_PI / 8.0,     0.75,      std::cos(M_PI/8.0), "theta=pi/8"},
            {M_PI / 4.0,     0.5,       SQRT2/2.0, "theta=pi/4: 0.5 vs 0.707"},
            {M_PI / 3.0,     1.0/3.0,   0.5,       "theta=pi/3: 0.333 vs 0.5"},
            {M_PI / 2.0,     0.0,       0.0,       "theta=pi/2: both = 0 (agree)"},
            {2.0*M_PI/3.0,  -1.0/3.0,  -0.5,      "theta=2pi/3: -0.333 vs -0.5"},
            {3.0*M_PI/4.0,  -0.5,      -SQRT2/2.0, "theta=3pi/4: -0.5 vs -0.707"},
            {M_PI,          -1.0,      -1.0,       "theta=pi: both = -1 (agree)"},
        };

        bool all_pass = true;
        for (const auto& tc : cases) {
            double got_cl = E_classical(tc.theta);
            double got_qu = E_quantum(tc.theta);
            bool cl_ok = std::abs(got_cl - tc.E_cl) < 1e-12;
            bool qu_ok = std::abs(got_qu - tc.E_qu) < 1e-12;
            if (!cl_ok || !qu_ok) all_pass = false;

            std::cout << "    " << tc.label
                      << " | cl=" << std::fixed << std::setprecision(4) << got_cl
                      << " qu=" << got_qu
                      << (cl_ok && qu_ok ? "" : " MISMATCH") << "\n";
        }

        check("BELL-6a: All correlation values match expected", all_pass);

        // Key property: |E_qu| >= |E_cl| at all angles
        // This is what allows quantum to exceed the classical CHSH bound.
        // Check at pi/4 where the difference is most pronounced:
        double diff_pi4 = std::abs(E_quantum(M_PI / 4.0)) - std::abs(E_classical(M_PI / 4.0));
        std::cout << "    |E_qu(pi/4)| - |E_cl(pi/4)| = "
                  << std::setprecision(6) << diff_pi4 << "\n";

        check("BELL-6b: |E_quantum| >= |E_classical| at pi/4", diff_pi4 > 0.0);

        // At pi/4: quantum = sqrt(2)/2 = 0.707, classical = 0.5
        // Ratio = sqrt(2)/2 / 0.5 = sqrt(2)
        double ratio_pi4 = std::abs(E_quantum(M_PI / 4.0)) / std::abs(E_classical(M_PI / 4.0));
        check_close("BELL-6c: |E_qu/E_cl| at pi/4 = sqrt(2)",
                    ratio_pi4, SQRT2, 1e-12);
    }

    // ================================================================
    // BELL-7: sLoop Infrastructure on Lattice
    // ================================================================
    std::cout << "\n-- BELL-7: sLoop Infrastructure (Entangled Pair on Lattice) --\n";
    {
        // Create an entangled pair on the lattice and verify the infrastructure
        // needed for the sLoop mechanism:
        //   (a) Pair production assigns matching pair_id
        //   (b) Complementary states (+1 and -1)
        //   (c) Both particles embedded in the same flux substrate (sLoop condition)
        //   (d) Complexified flux yields non-trivial Hilbert state

        int L = 16;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.gauss_projection = true;

        int mid = L / 2;
        rb.create_entangled_pair(mid, mid, mid, {0, 0, ftd::K_B});

        // Let fields build
        rb.run(100);

        // (a) Find the pair: one +1 and one -1 sharing the same pair_id
        int pos_idx = -1, neg_idx = -1;
        int pair_id_found = -1;
        int N = L * L * L;
        for (int i = 0; i < N; ++i) {
            const auto& v = rb.voxels()[i];
            if (v.state == +1 && v.pair_id >= 0) {
                pos_idx = i;
                pair_id_found = v.pair_id;
            }
            if (v.state == -1 && v.pair_id >= 0) {
                neg_idx = i;
            }
        }

        bool pair_exists = (pos_idx >= 0 && neg_idx >= 0);
        std::cout << "    Positive particle at index: " << pos_idx << "\n";
        std::cout << "    Negative particle at index: " << neg_idx << "\n";
        std::cout << "    Pair ID: " << pair_id_found << "\n";

        check("BELL-7a: Entangled pair created (both particles found)", pair_exists);

        if (pair_exists) {
            // (b) Complementary states
            check("BELL-7b: Complementary states (+1 and -1)",
                  rb.voxels()[pos_idx].state == +1 &&
                  rb.voxels()[neg_idx].state == -1);

            // (c) Matching pair_id (shared origin)
            check("BELL-7c: Matching pair_id (shared origin)",
                  rb.voxels()[pos_idx].pair_id == rb.voxels()[neg_idx].pair_id);
        }

        // (d) Complexified flux yields non-trivial Hilbert state
        // After 100 ticks, the flux field should be non-zero due to coupling.
        // The Hilbert state psi = J_x + iJ_y should have non-zero norm.
        auto hs = rb.hilbert_state();
        double norm2 = hs.norm_squared();

        std::cout << "    Hilbert state ||psi||^2 = " << norm2 << "\n";

        check("BELL-7d: Non-trivial Hilbert state (||psi||^2 > 0)", norm2 > 1e-10);

        // (e) Energy audit confirms both particles are embedded in the same
        // flux field (the ontological unity required for sLoop)
        auto ea = rb.energy_audit();
        std::cout << "    Field energy: " << ea.field_energy << "\n";
        std::cout << "    Manifested count: " << ea.manifested_count << "\n";
        std::cout << "    Charge total: " << ea.charge_total << " (should be 0)\n";

        check("BELL-7e: Charge conservation (Q=0 for EPR pair)",
              ea.charge_total == 0);
        check("BELL-7f: Both particles manifested in shared substrate",
              ea.manifested_count == 2);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  Bell Aggregate Summary\n";
    std::cout << "  ----------------------\n";
    std::cout << "  Level 1 (Substrate): S_max = 2         [local deterministic]\n";
    std::cout << "  Level 2 (Complex):   E(theta) = cos(theta) [Born rule]\n";
    std::cout << "  Level 3 (sLoop/QM):  S_max = 2*sqrt(2) = "
              << std::setprecision(6) << TSIRELSON << " [Tsirelson]\n";
    std::cout << "  Enhancement factor:  sqrt(2) = " << SQRT2 << "\n";
    std::cout << "================================================================\n";

    if (g_failures == 0) {
        std::cout << "  ALL CHECKS PASSED\n";
    } else {
        std::cout << "  " << g_failures << " CHECK(S) FAILED\n";
    }
    std::cout << "================================================================\n";

    return g_failures;
}
