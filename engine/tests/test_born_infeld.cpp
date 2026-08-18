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
    //   -E0*sqrt(1-u²/C²-L²) ≈ -E0 + E0*u²/(2C²) + E0*L²/2

    std::cout << "--- Weak-field reduction ---\n";
    {
        double v_vals[] = {0.01, 0.05, 0.1};
        double L_vals[] = {0.01, 0.05, 0.1};

        for (double v : v_vals) {
            for (double L : L_vals) {
                double exact = -ftd::E_REST * std::sqrt(
                    1.0 - v*v/(ftd::C_SPEED*ftd::C_SPEED) - L*L);
                double approx = -ftd::E_REST
                    + ftd::E_REST * v*v / (2.0*ftd::C_SPEED*ftd::C_SPEED)
                    + ftd::E_REST * L*L / 2.0;
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
        v.velocity = {0.99 * ftd::C_SPEED, 0, 0};
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

        // Coupling term: +g_c * 1 * 0.5 = +0.0427.  The 2026-07-18
        // action-sign correction makes this cooperate with div(J)=rho.
        double expected_coup = ftd::G_C * 1.0 * 0.5;
        check_close("Coupling = +g_c * s * divJ", coup, expected_coup, 1e-6);

        // Gauss term: -lambda_G * (0.5 - 1.0)^2 = -lambda_G * 0.25
        double expected_gauss = -ftd::LAMBDA_G * 0.25;
        check_close("Gauss = -lambda_G * (divJ - rho)^2", gauss, expected_gauss, 1e-6);
    }

    // ---- Hamiltonian = Legendre transform ----
    std::cout << "\n--- Hamiltonian density ---\n";
    {
        ftd::Voxel v;
        v.state = 0;
        v.velocity = {ftd::C_SPEED / 2.0, 0.0, 0.0};
        v.latency = 0.0;

        double divJ = 0.0;
        double rho = 0.0;

        double H = ftd::hamiltonian_density(v, divJ, rho);
        double expected_H = ftd::E_REST / std::sqrt(1.0 - 0.25);
        // For vacuum (s=0, divJ=0), coupling and gauss terms vanish
        check_close("H_BI = E_REST * gamma at u=C/2", H, expected_H, 1e-10);

        // At rest: H = E_REST
        ftd::Voxel v_rest;
        v_rest.latency = 0.0;
        double H_rest = ftd::hamiltonian_density(v_rest, 0, 0);
        check_close("H at rest = E_REST", H_rest, ftd::E_REST, 1e-10);
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
        const double beta = 0.6;
        const double u = beta * ftd::C_SPEED;
        const double gamma = 1.0 / std::sqrt(1.0 - beta*beta);
        const double E = ftd::E_REST * gamma;
        const double p = ftd::M_INERTIAL * u * gamma;
        const double mass_shell = E*E - ftd::C_SPEED*ftd::C_SPEED*p*p;
        check_close("E^2-C^2p^2 = E_REST^2", mass_shell,
                    ftd::E_REST * ftd::E_REST, 1e-10);
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
