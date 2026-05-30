/**
 * Test: Reference frame context Constants and Quadratic Structure
 *
 * Verifies that the reference frame context-sector constants from ontic.h Layer 8
 * are correctly derived and internally consistent.
 *
 * The sLoop detection, attention field, and noetic mass implementations
 * were removed in v2.11 (they were stubs returning zero since v2.0).
 * When a reference frame context implementation is added, this test should be
 * extended with runtime checks.
 *
 * Checklist items #71, #72, #73.
 *
 * Theory references:
 *   - ontic.h Layer 8 (K_C, COS2_THETA_C, reference frame context quadratic)
 *   - FOUND_THE_EXISTENCE_FILTER.md (reference frame context threshold)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
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
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Reference frame context Constants and Quadratic Structure\n";
    std::cout << "================================================================\n\n";

    // SLOOP-1: Reference frame context constants are positive and finite
    {
        std::cout << "--- SLOOP-1: Reference frame context constants well-defined ---\n";
        check("SLOOP-1: K_C_SQUARED > 0", ftd::K_C_SQUARED > 0.0);
        check("SLOOP-1: K_NOETIC > 0", ftd::K_NOETIC > 0.0);
        check("SLOOP-1: SIN2_THETA_C in (0,1)", ftd::SIN2_THETA_C > 0.0 && ftd::SIN2_THETA_C < 1.0);
        check("SLOOP-1: COS2_THETA_C in (0,1)", ftd::COS2_THETA_C > 0.0 && ftd::COS2_THETA_C < 1.0);
        check_close("SLOOP-1: sin^2 + cos^2 = 1", ftd::SIN2_THETA_C + ftd::COS2_THETA_C, 1.0, 1e-12);
    }

    // SLOOP-2: Golden ratio fixed point
    {
        std::cout << "\n--- SLOOP-2: Golden ratio self-referential fixed point ---\n";
        check_close("SLOOP-2: PHI^2 = PHI + 1", ftd::PHI * ftd::PHI, ftd::PHI + 1.0, 1e-12);
        check_close("SLOOP-2: PHI * PHI_INV = 1", ftd::PHI * ftd::PHI_INV, 1.0, 1e-12);
        check("SLOOP-2: N_CONSCIOUSNESS_MIN >= 1", ftd::N_CONSCIOUSNESS_MIN >= 1);
    }

    // SLOOP-3: Mandelbrot connection (c = 1/G* from ontic chain)
    {
        std::cout << "\n--- SLOOP-3: Mandelbrot critical point ---\n";
        check_close("SLOOP-3: C_MANDELBROT = 1/G*", ftd::C_MANDELBROT, 1.0 / ftd::G_STAR, 1e-12);
    }

    // SLOOP-4: Engine runs without reference frame context fields
    {
        std::cout << "\n--- SLOOP-4: Engine operates without reference frame context fields ---\n";
        const int L = 8;
        ftd::RenderBridge engine(L);
        engine.toggles.enable_all();
        engine.toggles.genesis = false;

        int cx = L / 2, cy = L / 2, cz = L / 2;
        engine.inject_wavepacket(cx, cy, cz, +1);
        engine.run(50);

        int idx = engine.lattice().index(cx, cy, cz);
        const auto& v = engine.voxels()[idx];
        check("SLOOP-4: Particle still manifested after 50 ticks", v.state != 0);

        auto diag = engine.diagnostics();
        check("SLOOP-4: Diagnostics report positive energy", diag.total_energy > 0.0);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All reference frame context constant tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
