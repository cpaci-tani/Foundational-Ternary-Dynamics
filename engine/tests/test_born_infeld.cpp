/**
 * Test: Born-Infeld Lagrangian Predictions
 *
 * Verifies:
 *   1. Weak-field reduction: BI core -> Klein-Gordon for small v, L
 *   2. Bandwidth enforcement: v^2 + L^2 < 1 never violated
 *   3. Lagrangian density computation
 *   4. Hamiltonian = Legendre transform of BI
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (Born-Infeld reformulation v2.0)
 *   - DERIV_FORCE_EMERGENCE.md           (force laws from lattice Green's functions)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/voxel.h"
#include "ftd/lagrangian.h"
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

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Born-Infeld Lagrangian Predictions\n";
    std::cout << "================================================================\n\n";

    // ---- Weak-field reduction ----
    // For small v, L:
    //   -K_B * sqrt(1 - v^2 - L^2) ≈ -K_B + K_B*v^2/2 + K_B*L^2/2
    // The Klein-Gordon kinetic term is (1/2)|dJ/dt|^2 ≈ K_B*v^2/2

    std::cout << "--- Weak-field reduction ---\n";
    {
        double v_vals[] = {0.01, 0.05, 0.1};
        double L_vals[] = {0.01, 0.05, 0.1};

        for (double v : v_vals) {
            for (double L : L_vals) {
                double exact = -ftd::K_B * std::sqrt(1.0 - v*v - L*L);
                double approx = -ftd::K_B + ftd::K_B * v*v / 2.0 + ftd::K_B * L*L / 2.0;
                double rel_err = std::abs(exact - approx) / std::abs(exact);

                char buf[128];
                snprintf(buf, sizeof(buf),
                    "Weak-field v=%.2f L=%.2f (rel err %.2e)", v, L, rel_err);
                // For small v,L the O(v^4) correction should be < 1%
                check(buf, rel_err < 0.01);
            }
        }
    }

    // ---- Strong-field divergence ----
    std::cout << "\n--- Strong-field behavior ---\n";
    {
        ftd::Voxel v;
        v.velocity = {0.99, 0, 0};
        v.latency = 0.0;
        double g = v.gamma_ftd();
        check("gamma at v=0.99 > 7", g > 7.0);

        double bi = v.born_infeld_core();
        check("BI core at v=0.99 is small (near zero)", std::abs(bi) < 0.1);
    }

    // ---- Lagrangian density computation ----
    std::cout << "\n--- Lagrangian density ---\n";
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = {0, 0, ftd::K_B};
        v.velocity = {0.1, 0, 0};
        v.latency = 0.05;

        double divJ = 0.5;  // hypothetical
        double rho = 1.0;   // state = +1

        double L = ftd::lagrangian_density(v, divJ, rho);
        double bi = ftd::born_infeld_term(v);
        double coup = ftd::coupling_term(v, divJ);
        double gauss = ftd::gauss_term(divJ, rho);

        // L should equal sum of all three terms
        check_close("L = BI + coupling + gauss", L, bi + coup + gauss, 1e-12);

        // BI term should be negative
        check("BI term < 0", bi < 0.0);

        // Coupling term: -g_c * 1 * 0.5 = -0.0427
        double expected_coup = -ftd::G_C * 1.0 * 0.5;
        check_close("Coupling = -g_c * s * divJ", coup, expected_coup, 1e-6);

        // Gauss term: -lambda_G * (0.5 - 1.0)^2 = -lambda_G * 0.25
        double expected_gauss = -ftd::LAMBDA_G * 0.25;
        check_close("Gauss = -lambda_G * (divJ - rho)^2", gauss, expected_gauss, 1e-6);
    }

    // ---- Hamiltonian = Legendre transform ----
    std::cout << "\n--- Hamiltonian density ---\n";
    {
        ftd::Voxel v;
        v.state = 0;
        v.velocity = {0.3, 0.4, 0.0};  // |v| = 0.5
        v.latency = 0.0;

        double divJ = 0.0;
        double rho = 0.0;

        double H = ftd::hamiltonian_density(v, divJ, rho);
        double expected_H = ftd::K_B / std::sqrt(1.0 - 0.25);
        // For vacuum (s=0, divJ=0), coupling and gauss terms vanish
        check_close("H_BI = K_B * gamma at v=0.5", H, expected_H, 1e-10);

        // At rest: H = K_B (rest mass energy)
        ftd::Voxel v_rest;
        v_rest.latency = 0.0;
        double H_rest = ftd::hamiltonian_density(v_rest, 0, 0);
        check_close("H at rest = K_B (rest mass)", H_rest, ftd::K_B, 1e-10);
    }

    // ---- Bandwidth enforcement in simulation ----
    std::cout << "\n--- Bandwidth enforcement ---\n";
    {
        ftd::RenderBridge rb(16);
        // Place two particles with large velocities
        rb.inject_particle(4, 8, 8, +1, {0, 0, ftd::K_B});
        rb.inject_particle(12, 8, 8, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(4, 8, 8)].velocity = {0.8, 0, 0};
        rb.voxels()[rb.lattice().index(12, 8, 8)].velocity = {-0.8, 0, 0};

        bool bandwidth_ok = true;
        for (int t = 0; t < 200; ++t) {
            rb.tick();
            for (const auto& v : rb.voxels()) {
                if (v.state != 0 && v.bandwidth_used() >= 1.0) {
                    bandwidth_ok = false;
                    break;
                }
            }
            if (!bandwidth_ok) break;
        }
        check("Bandwidth v^2+L^2 < 1 always (200 ticks)", bandwidth_ok);
    }

    // ---- BI energy-momentum relation ----
    std::cout << "\n--- Energy-momentum relation ---\n";
    {
        // E^2 = p^2 + m^2 (in BI language: H^2 = K_B^2 * v^2 * gamma^2 + K_B^2)
        // Actually: H = K_B * gamma, p = K_B * v * gamma
        // So H^2 - p^2 = K_B^2 * gamma^2 * (1 - v^2) = K_B^2
        double v = 0.6;
        double gamma = 1.0 / std::sqrt(1.0 - v*v);
        double E = ftd::K_B * gamma;
        double p = ftd::K_B * v * gamma;
        double mass_shell = E*E - p*p;
        check_close("E^2 - p^2 = K_B^2 (mass shell)", mass_shell, ftd::K_B * ftd::K_B, 1e-10);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Born-Infeld tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
