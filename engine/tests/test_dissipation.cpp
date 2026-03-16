// Test: Rayleigh Dissipation Function
//
// Verifies R = (DAMPING/2) * |wave_vel|^2
// where DAMPING = alpha (derived from vacuum drag / geometric friction).
//
// Sections:
//   1. DAMPING = ALPHA (identity)
//   2. R = 0 when wave_vel = 0
//   3. R > 0 when wave_vel != 0 (energy sink)
//   4. R scales as |wave_vel|^2

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
    std::cout << "  TEST: Rayleigh Dissipation\n";
    std::cout << "================================================================\n";

    // Section 1: DAMPING = ALPHA
    std::cout << "\n--- Section 1: DAMPING = ALPHA ---\n";
    {
        check_close("DAMPING = ALPHA", ftd::DAMPING, ftd::ALPHA, 1e-15);
        std::cout << "    DAMPING = " << ftd::DAMPING << "\n";
        std::cout << "    ALPHA   = " << ftd::ALPHA << "\n";
    }

    // Section 2: R = 0 when stationary
    std::cout << "\n--- Section 2: Zero for Stationary ---\n";
    {
        ftd::Voxel v;
        v.wave_vel = ftd::Vec3(0, 0, 0);
        double R = ftd::rayleigh_dissipation(v);
        check_close("R = 0 for wave_vel = 0", R, 0.0, 1e-15);
    }

    // Section 3: R > 0 for nonzero wave velocity
    std::cout << "\n--- Section 3: Positive for Moving ---\n";
    {
        ftd::Voxel v;
        v.wave_vel = ftd::Vec3(1.0, 0, 0);
        double R = ftd::rayleigh_dissipation(v);
        std::cout << "    R(|wv|=1) = " << R << "\n";
        check("R > 0 for nonzero wave_vel", R > 0.0);
        check_close("R = DAMPING/2 * |wv|^2", R, 0.5 * ftd::DAMPING * 1.0, 1e-15);
    }

    // Section 4: Quadratic scaling
    std::cout << "\n--- Section 4: Quadratic Scaling ---\n";
    {
        ftd::Voxel v1, v2;
        v1.wave_vel = ftd::Vec3(1.0, 0, 0);
        v2.wave_vel = ftd::Vec3(2.0, 0, 0);

        double R1 = ftd::rayleigh_dissipation(v1);
        double R2 = ftd::rayleigh_dissipation(v2);

        // R2/R1 should be 4 (quadratic scaling)
        double ratio = R2 / R1;
        check_close("R(2v)/R(v) = 4 (quadratic)", ratio, 4.0, 1e-10);

        // 3D case
        ftd::Voxel v3;
        v3.wave_vel = ftd::Vec3(1.0, 1.0, 1.0);
        double R3 = ftd::rayleigh_dissipation(v3);
        double expected = 0.5 * ftd::DAMPING * 3.0;  // |wv|^2 = 3
        check_close("R(1,1,1) = DAMPING/2 * 3", R3, expected, 1e-15);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All dissipation tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
