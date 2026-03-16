/**
 * Campaign: Dispersion Relation Convergence (Phase 2 — Continuum Limit)
 *
 * Validates that the lattice wave equation converges to Maxwell's equations
 * by measuring the dispersion relation ω(k) at multiple lattice sizes.
 *
 * Theory: On a cubic lattice with c² = 1/3, the exact dispersion relation is
 *   ω² = (4/3) Σ_i sin²(k_i/2)
 * For a 1D mode along x: ω = (2/√3)·|sin(k/2)|
 * In the continuum limit (k→0): ω → c·|k| with c = 1/√3 ≈ 0.577
 *
 * Protocol:
 *   1. Measure ω(k) for modes 1..4 on L=16 and L=32
 *   2. Compare with exact lattice dispersion (not continuum!)
 *   3. Verify c_eff → C_WAVE in low-k limit
 *   4. Verify convergence: larger L gives smaller deviation from continuum
 *
 * Checks:
 *   DC1: Mode 1 c_eff within 15% of C_WAVE (L=16)
 *   DC2: Mode 1 c_eff within 10% of C_WAVE (L=32)
 *   DC3: L=32 c_eff closer to C_WAVE than L=16 (convergence)
 *   DC4: ω matches exact lattice formula within 25% (mode 1, L=32)
 *   DC5: ω(k) is monotonically increasing for modes 1..4 (L=32)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/spectral.h"
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
    std::cout << "  CAMPAIGN: Dispersion Convergence (Phase 2) — 5 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    // ----------------------------------------------------------------
    // Measure dispersion at L=16 and L=32
    // ----------------------------------------------------------------
    struct SizeResult {
        int L;
        std::vector<ftd::DispersionPoint> points;
    };

    int sizes[] = {16, 32};
    SizeResult results[2];

    for (int s = 0; s < 2; ++s) {
        int L = sizes[s];
        std::cout << "\n--- L=" << L << " Dispersion Relation ---\n";

        auto pts = ftd::dispersion_relation(L, 4, 512);
        results[s] = {L, pts};

        std::cout << "  Mode | k        | omega    | c_eff    | c_theory\n";
        for (int m = 0; m < static_cast<int>(pts.size()); ++m) {
            double k = pts[m].k;
            // Exact lattice dispersion: ω = (2/√3)·sin(k/2)
            double omega_exact = (2.0 / std::sqrt(3.0)) * std::sin(k / 2.0);
            std::cout << "  " << (m+1)
                      << "    | " << std::setw(8) << k
                      << " | " << std::setw(8) << pts[m].omega
                      << " | " << std::setw(8) << pts[m].c_eff
                      << " | " << std::setw(8) << (omega_exact / k)
                      << "\n";
        }
    }

    // ----------------------------------------------------------------
    // DC1: Mode 1 c_eff within 15% of C_WAVE (L=16)
    // ----------------------------------------------------------------
    double c_eff_16 = results[0].points[0].c_eff;
    double c_theory = ftd::C_WAVE;  // 1/sqrt(3) ≈ 0.577
    double err_16 = std::abs(c_eff_16 - c_theory) / c_theory;
    std::cout << "\n--- Convergence Analysis ---\n";
    std::cout << "  L=16: c_eff=" << c_eff_16 << " C_WAVE=" << c_theory
              << " err=" << (err_16 * 100) << "%\n";
    check("DC1: L=16 mode-1 c_eff within 15% of C_WAVE", err_16 < 0.15);

    // ----------------------------------------------------------------
    // DC2: Mode 1 c_eff within 10% of C_WAVE (L=32)
    // ----------------------------------------------------------------
    double c_eff_32 = results[1].points[0].c_eff;
    double err_32 = std::abs(c_eff_32 - c_theory) / c_theory;
    std::cout << "  L=32: c_eff=" << c_eff_32 << " C_WAVE=" << c_theory
              << " err=" << (err_32 * 100) << "%\n";
    check("DC2: L=32 mode-1 c_eff within 10% of C_WAVE", err_32 < 0.10);

    // ----------------------------------------------------------------
    // DC3: Convergence — L=32 closer to theory than L=16
    // ----------------------------------------------------------------
    check("DC3: L=32 error < L=16 error (convergence)", err_32 < err_16 + 1e-6);

    // ----------------------------------------------------------------
    // DC4: ω matches exact lattice formula within 25%
    // ----------------------------------------------------------------
    double k1 = results[1].points[0].k;
    double omega_measured = results[1].points[0].omega;
    double omega_exact = (2.0 / std::sqrt(3.0)) * std::sin(k1 / 2.0);
    double omega_err = std::abs(omega_measured - omega_exact) / omega_exact;
    std::cout << "  omega_measured=" << omega_measured
              << " omega_exact=" << omega_exact
              << " err=" << (omega_err * 100) << "%\n";
    check("DC4: omega within 25% of lattice theory (mode 1, L=32)", omega_err < 0.25);

    // ----------------------------------------------------------------
    // DC5: ω(k) monotonically increasing (L=32)
    // ----------------------------------------------------------------
    bool monotonic = true;
    for (size_t i = 1; i < results[1].points.size(); ++i) {
        if (results[1].points[i].omega < results[1].points[i-1].omega - 1e-6) {
            monotonic = false;
            break;
        }
    }
    check("DC5: omega(k) monotonically increasing (L=32)", monotonic);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
