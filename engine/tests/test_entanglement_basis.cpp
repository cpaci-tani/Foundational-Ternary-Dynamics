/**
 * Test: Entanglement Basis Dependence — Measurement-Basis Correlations
 *
 * Tests the local hidden-variable model of entangled pairs, verifying that
 * correlations depend on the measurement basis angle and follow the CLASSICAL
 * (triangular) pattern expected from a local deterministic substrate:
 *   ENT-1: Same-basis measurement → perfect anti-correlation
 *   ENT-2: Orthogonal-basis measurement → 50% correlation
 *   ENT-3: 45-degree basis → 0.75 correlation (classical, not quantum 0.854)
 *   ENT-4: Multiple bases → correlation follows 1 - |delta|/pi (triangular)
 *
 * The entangled pair uses a hidden angle phi (uniformly distributed),
 * with deterministic measurement outcomes. This is explicitly a local
 * hidden-variable model. By Bell's theorem, it gives S <= 2 and classical
 * correlations — the CORRECT result for FTD's substrate-level dynamics.
 *
 * Classical (triangular) pattern:
 *   P(anti-correlated) = 1 - |delta|/pi
 * vs Quantum (sinusoidal) pattern:
 *   P(anti-correlated) = cos^2(delta/2)
 * These agree at delta=0 (both 1.0), delta=pi/2 (both 0.5), and delta=pi (both 0.0)
 * but diverge at intermediate angles (e.g., delta=pi/4: classical=0.75, quantum=0.854).
 *
 * Theory references:
 *   - CLAUDE.md §12                   (entanglement in the model)
 *   - DERIV_QUANTUM_MECHANICS_RESOLVED.md  (singlet correlations)
 *   - SPEC_FTD_REFERENCE.md           (Hilbert space tensor product)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
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

/**
 * Model an entangled pair as anti-correlated complex flux states.
 *
 * Particle A at site idx_a has psi_A = cos(phi) + i*sin(phi)
 * Particle B at site idx_b has psi_B = cos(phi+pi) + i*sin(phi+pi) = -psi_A
 *
 * "Measurement in basis theta" means projecting psi onto e^(i*theta):
 *   outcome = Re(conj(e^(i*theta)) * psi) = Re(e^(-i*theta) * psi)
 *   result = sign(outcome): +1 or -1
 *
 * This is a LOCAL HIDDEN-VARIABLE model: each pair carries a definite hidden
 * angle phi, and measurement is a deterministic function of phi and theta.
 * By Bell's theorem, such models produce CLASSICAL correlations:
 *   P_anti(delta) = 1 - |delta|/pi   (triangular, not cos^2)
 *
 * This is the CORRECT result for FTD's local deterministic substrate (S <= 2).
 * Quantum correlations cos^2(delta/2) would require non-local effects.
 */
struct EntangledPair {
    ftd::Complex psi_A;
    ftd::Complex psi_B;

    // Create an entangled pair with flux angle phi
    EntangledPair(double phi) {
        psi_A = std::polar(1.0, phi);
        psi_B = -psi_A;  // anti-correlated
    }

    // "Measure" particle in basis theta: project onto e^(i*theta) direction
    // Returns +1 or -1 based on projection sign
    static int measure(ftd::Complex psi, double theta) {
        ftd::Complex basis = std::polar(1.0, theta);
        double projection = (std::conj(basis) * psi).real();
        return (projection >= 0.0) ? +1 : -1;
    }
};

/**
 * Run an ensemble of entangled pair measurements and compute correlation.
 *
 * Creates N entangled pairs with uniformly distributed flux angles.
 * Measures each pair: A in basis theta_A, B in basis theta_B.
 * Returns the fraction of trials where outcomes are anti-correlated.
 */
double ensemble_anticorrelation(double theta_A, double theta_B, int N_trials) {
    int anti_correlated = 0;

    for (int i = 0; i < N_trials; ++i) {
        // Each pair has a different "hidden" angle phi,
        // uniformly distributed over [0, 2*pi)
        double phi = 2.0 * M_PI * i / N_trials;

        EntangledPair pair(phi);
        int result_A = EntangledPair::measure(pair.psi_A, theta_A);
        int result_B = EntangledPair::measure(pair.psi_B, theta_B);

        if (result_A != result_B) {
            anti_correlated++;
        }
    }

    return static_cast<double>(anti_correlated) / N_trials;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Entanglement Basis Dependence\n";
    std::cout << "================================================================\n";

    int N_trials = 10000;  // large ensemble for statistical convergence

    // ================================================================
    // ENT-1: Same-basis measurement → perfect anti-correlation
    // ================================================================
    std::cout << "\n--- ENT-1: Same-Basis Measurement ---\n";
    {
        double theta_A = 0.0;
        double theta_B = 0.0;  // Same basis
        double p_anti = ensemble_anticorrelation(theta_A, theta_B, N_trials);

        std::cout << "    theta_A = 0, theta_B = 0 (same basis)\n";
        std::cout << "    P(anti-correlated) = " << p_anti << "\n";
        std::cout << "    Expected: cos^2(0/2) = 1.0\n";

        // Same basis: cos^2(0) = 1.0 → perfect anti-correlation
        check_close("ENT-1: same basis → perfect anti-correlation",
                    p_anti, 1.0, 0.02);
    }

    // ================================================================
    // ENT-2: Orthogonal-basis measurement → 50% correlation
    // ================================================================
    std::cout << "\n--- ENT-2: Orthogonal-Basis Measurement ---\n";
    {
        double theta_A = 0.0;
        double theta_B = M_PI / 2.0;  // 90 degrees apart
        double p_anti = ensemble_anticorrelation(theta_A, theta_B, N_trials);

        // Classical: 1 - |pi/2|/pi = 0.5 (same as quantum cos^2(pi/4) = 0.5)
        double expected = 1.0 - (M_PI / 2.0) / M_PI;  // = 0.5

        std::cout << "    theta_A = 0, theta_B = pi/2 (orthogonal)\n";
        std::cout << "    P(anti-correlated) = " << p_anti << "\n";
        std::cout << "    Expected (classical): 1 - (pi/2)/pi = " << expected << "\n";

        // Orthogonal basis: classical and quantum agree at 0.5
        check_close("ENT-2: orthogonal basis → 50% anti-correlation",
                    p_anti, expected, 0.03);
    }

    // ================================================================
    // ENT-3: 45-degree basis → classical 0.75 correlation
    // ================================================================
    std::cout << "\n--- ENT-3: 45-Degree Basis ---\n";
    {
        double theta_A = 0.0;
        double theta_B = M_PI / 4.0;  // 45 degrees apart
        double p_anti = ensemble_anticorrelation(theta_A, theta_B, N_trials);

        // Classical (triangular): 1 - |pi/4|/pi = 1 - 0.25 = 0.75
        // Quantum would give cos^2(pi/8) ~ 0.854 — but this is a local HV model
        double expected = 1.0 - (M_PI / 4.0) / M_PI;  // = 0.75

        std::cout << "    theta_A = 0, theta_B = pi/4 (45 degrees)\n";
        std::cout << "    P(anti-correlated) = " << p_anti << "\n";
        std::cout << "    Expected (classical): 1 - (pi/4)/pi = " << expected << "\n";
        std::cout << "    (Quantum would give cos^2(pi/8) = "
                  << std::cos(M_PI/8.0)*std::cos(M_PI/8.0) << ")\n";

        check_close("ENT-3: 45-degree basis → classical 0.75 (not quantum 0.854)",
                    p_anti, expected, 0.03);
    }

    // ================================================================
    // ENT-4: Multiple bases — correlation follows 1 - |delta|/pi (classical)
    // ================================================================
    std::cout << "\n--- ENT-4: Correlation vs Angle ---\n";
    {
        // Test at several angles and verify the CLASSICAL triangular pattern
        // P_anti(delta) = 1 - |delta|/pi  (local hidden-variable model)
        double angles[] = {0.0, M_PI / 6.0, M_PI / 4.0, M_PI / 3.0,
                           M_PI / 2.0, 2.0 * M_PI / 3.0, M_PI};
        int n_angles = 7;
        double max_error = 0.0;

        std::cout << "    delta(rad)   P_anti(measured)   1-|d|/pi (classical)   error\n";

        for (int i = 0; i < n_angles; ++i) {
            double delta = angles[i];
            double p_anti = ensemble_anticorrelation(0.0, delta, N_trials);
            double expected = 1.0 - std::abs(delta) / M_PI;  // classical triangular
            double error = std::abs(p_anti - expected);
            if (error > max_error) max_error = error;

            std::cout << "    " << std::setw(8) << std::setprecision(4) << delta
                      << "    " << std::setw(12) << p_anti
                      << "         " << std::setw(10) << expected
                      << "        " << std::setw(8) << error << "\n";
        }

        std::cout << "    Maximum error across all angles: " << max_error << "\n";

        check("ENT-4a: all angles fit classical 1-|d|/pi within 5%", max_error < 0.05);

        // Verify monotonicity: P_anti decreases as delta increases from 0 to pi
        double p_0 = ensemble_anticorrelation(0.0, 0.0, N_trials);
        double p_half = ensemble_anticorrelation(0.0, M_PI / 2.0, N_trials);
        double p_pi = ensemble_anticorrelation(0.0, M_PI, N_trials);

        check("ENT-4b: P_anti(0) > P_anti(pi/2) > P_anti(pi)",
              p_0 > p_half && p_half > p_pi);

        // At delta = pi (opposite basis), anti-correlation → 0
        // Classical: 1 - pi/pi = 0 (same as quantum cos^2(pi/2) = 0)
        double expected_pi = 0.0;
        check_close("ENT-4c: opposite basis → 0% anti-correlation",
                    p_pi, expected_pi, 0.03);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All entanglement basis tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
