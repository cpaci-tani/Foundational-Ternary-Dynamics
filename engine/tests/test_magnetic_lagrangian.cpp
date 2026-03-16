// Test: Magnetic Lagrangian -- Velocity Coupling Term
//
// Verifies L_VELOCITY = -g_c * s * (v . J)
// The Euler-Lagrange equation yields the Lorentz force:
//   F_mag = g_c * q * v x curl(J)
//
// Sections:
//   1. L_VELOCITY is zero for stationary particles (v=0)
//   2. L_VELOCITY is nonzero for moving charged particles
//   3. L_VELOCITY sign depends on charge and velocity direction
//   4. Lorentz force is perpendicular to velocity

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
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
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Magnetic Lagrangian (Velocity Coupling)\n";
    std::cout << "================================================================\n";

    // Section 1: Zero for stationary particles
    std::cout << "\n--- Section 1: Zero for Stationary ---\n";
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = ftd::Vec3(0, 0, ftd::K_B);
        v.velocity = ftd::Vec3(0, 0, 0);  // stationary

        double L_vel = ftd::velocity_coupling_term(v);
        check_close("L_VELOCITY = 0 for v=0", L_vel, 0.0, 1e-15);
    }

    // Section 2: Nonzero for moving particles
    std::cout << "\n--- Section 2: Nonzero for Moving ---\n";
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = ftd::Vec3(0, 0, ftd::K_B);
        v.velocity = ftd::Vec3(0, 0, 0.5);  // moving along z

        double L_vel = ftd::velocity_coupling_term(v);
        // L = -g_c * s * (v . J) = -g_c * 1 * (0.5 * K_B) = -g_c * K_B * 0.5
        double expected = -ftd::G_C * 1 * (0.5 * ftd::K_B);
        std::cout << "    L_VELOCITY = " << L_vel << "\n";
        std::cout << "    Expected   = " << expected << "\n";
        check_close("L_VELOCITY correct value", L_vel, expected, 1e-10);
        check("L_VELOCITY is nonzero", std::abs(L_vel) > 1e-10);
    }

    // Section 3: Sign depends on charge
    std::cout << "\n--- Section 3: Charge-Dependent Sign ---\n";
    {
        ftd::Voxel v_pos, v_neg;
        v_pos.state = +1;
        v_neg.state = -1;
        v_pos.flux = v_neg.flux = ftd::Vec3(1, 0, 0);
        v_pos.velocity = v_neg.velocity = ftd::Vec3(0.3, 0, 0);

        double L_pos = ftd::velocity_coupling_term(v_pos);
        double L_neg = ftd::velocity_coupling_term(v_neg);

        std::cout << "    L_vel(+1) = " << L_pos << "\n";
        std::cout << "    L_vel(-1) = " << L_neg << "\n";
        check_close("Opposite signs for opposite charges", L_pos + L_neg, 0.0, 1e-15);
    }

    // Section 4: Perpendicular velocity has no contribution
    std::cout << "\n--- Section 4: Perpendicular Velocity ---\n";
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = ftd::Vec3(0, 0, ftd::K_B);
        v.velocity = ftd::Vec3(0.5, 0, 0);  // perpendicular to flux

        double L_vel = ftd::velocity_coupling_term(v);
        check_close("L_VELOCITY = 0 when v perp J", L_vel, 0.0, 1e-15);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All magnetic Lagrangian tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
