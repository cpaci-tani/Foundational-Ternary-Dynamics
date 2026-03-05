/**
 * Test: Voxel derived quantities
 *
 * Verifies density(), speed(), bandwidth_used(), gamma_ftd(),
 * and born_infeld_core() for known inputs.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md  (Born-Infeld core, bandwidth constraint)
 *   - voxel.h                 (Voxel struct and derived methods)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include "ftd/voxel.h"

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
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15)
                  << a << ", expected " << b << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Voxel Properties\n";
    std::cout << "================================================================\n\n";

    // Default voxel: all fields zero
    {
        ftd::Voxel v;
        check("Default state = 0", v.state == 0);
        check_close("Default density = 0", v.density(), 0.0, 1e-15);
        check_close("Default speed = 0", v.speed(), 0.0, 1e-15);
        check_close("Default latency = 0", v.latency, 0.0, 1e-15);
        check_close("Default tau = 0", v.tau, 0.0, 1e-15);
        check_close("Default drag = 0", v.drag, 0.0, 1e-15);
        check("Default locked = false", v.locked == false);
        check("Default pair_id = -1", v.pair_id == -1);
        check_close("Default attention = 0", v.attention, 0.0, 1e-15);
        check("Default sloop_depth = 0", v.sloop_depth == 0);
        check("Default is_sloop = false", v.is_sloop == false);
    }

    // density() = |flux|
    {
        ftd::Voxel v;
        v.flux = {3.0, 4.0, 0.0};
        check_close("density (3,4,0) = 5", v.density(), 5.0, 1e-12);

        v.flux = {1.0, 1.0, 1.0};
        check_close("density (1,1,1) = sqrt(3)", v.density(), std::sqrt(3.0), 1e-12);

        v.flux = {0.0, 0.0, 0.0};
        check_close("density (0,0,0) = 0", v.density(), 0.0, 1e-15);
    }

    // speed() = |velocity|
    {
        ftd::Voxel v;
        v.velocity = {0.6, 0.8, 0.0};
        check_close("speed (0.6,0.8,0) = 1.0", v.speed(), 1.0, 1e-12);

        v.velocity = {0.3, 0.0, 0.0};
        check_close("speed (0.3,0,0) = 0.3", v.speed(), 0.3, 1e-12);
    }

    // bandwidth_used() = speed^2 + latency^2
    {
        ftd::Voxel v;
        v.velocity = {0.5, 0.0, 0.0};
        v.latency = 0.3;
        // speed = 0.5, speed^2 = 0.25, latency^2 = 0.09
        check_close("bandwidth (v=0.5, L=0.3) = 0.34", v.bandwidth_used(), 0.34, 1e-12);

        v.velocity = {0.0, 0.0, 0.0};
        v.latency = 0.0;
        check_close("bandwidth (0,0) = 0", v.bandwidth_used(), 0.0, 1e-15);
    }

    // gamma_ftd() = 1/sqrt(1 - bw)
    {
        ftd::Voxel v;
        v.velocity = {0.0, 0.0, 0.0};
        v.latency = 0.0;
        check_close("gamma at rest = 1.0", v.gamma_ftd(), 1.0, 1e-12);

        v.velocity = {0.5, 0.0, 0.0};
        v.latency = 0.0;
        // bw = 0.25, gamma = 1/sqrt(0.75) = 2/sqrt(3)
        check_close("gamma (v=0.5) = 2/sqrt(3)", v.gamma_ftd(), 2.0/std::sqrt(3.0), 1e-12);

        // Bandwidth overflow
        v.velocity = {1.0, 0.0, 0.0};
        v.latency = 0.0;
        check("gamma at bw=1 is very large", v.gamma_ftd() > 1e20);
    }

    // born_infeld_core() = -K_B * sqrt(1 - bw)
    {
        ftd::Voxel v;
        v.velocity = {0.0, 0.0, 0.0};
        v.latency = 0.0;
        check_close("BI core at rest = -K_B", v.born_infeld_core(), -ftd::K_B, 1e-12);

        v.velocity = {0.5, 0.0, 0.0};
        v.latency = 0.0;
        // bw = 0.25, core = -K_B * sqrt(0.75)
        check_close("BI core (v=0.5)", v.born_infeld_core(), -ftd::K_B * std::sqrt(0.75), 1e-12);

        // Bandwidth overflow
        v.velocity = {1.0, 0.0, 0.0};
        v.latency = 0.0;
        check_close("BI core at bw=1 = 0", v.born_infeld_core(), 0.0, 1e-12);
    }

    // Vec3 operations
    {
        ftd::Vec3 a(1.0, 2.0, 3.0);
        ftd::Vec3 b(4.0, 5.0, 6.0);
        auto c = a + b;
        check_close("Vec3 add x", c.x, 5.0, 1e-15);
        check_close("Vec3 add y", c.y, 7.0, 1e-15);
        check_close("Vec3 add z", c.z, 9.0, 1e-15);

        auto d = a - b;
        check_close("Vec3 sub x", d.x, -3.0, 1e-15);

        auto e = a * 2.0;
        check_close("Vec3 scale x", e.x, 2.0, 1e-15);
        check_close("Vec3 scale y", e.y, 4.0, 1e-15);

        check_close("Vec3 mag2", a.mag2(), 14.0, 1e-12);
        check_close("Vec3 mag", a.mag(), std::sqrt(14.0), 1e-12);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All voxel property tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
