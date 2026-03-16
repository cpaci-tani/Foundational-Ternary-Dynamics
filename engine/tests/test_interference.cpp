/**
 * Test: Interference — Flux Superposition and Pattern Formation
 *
 * Verifies that the linear flux field produces constructive and
 * destructive interference from two coherent sources:
 *   J_total = J_1 + J_2  (linear superposition)
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md               (linear wave equation)
 *   - DERIV_QUANTUM_MECHANICS_RESOLVED.md   (wave behavior pre-manifestation)
 *   - DERIV_DISCRETE_CONTINUOUS_BRIDGE.md   (lattice wave properties)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
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

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Interference — Flux Superposition\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Section 1: Two-source constructive interference
    // ================================================================
    // Two identical flux sources separated by distance d.
    // At the midpoint: constructive interference (both waves arrive in phase).
    // The two-source midpoint density should exceed a single source measured
    // at the same distance from a source.
    std::cout << "\n--- Section 1: Two-Source Interference ---\n";
    {
        int L = 48;
        int cy = L / 2;
        int cz = L / 2;
        int sep = 10;  // separation between sources
        int x1 = L/2 - sep/2;
        int x2 = L/2 + sep/2;
        int ticks = 20;

        // Two coherent sources (same polarization, same amplitude)
        double amp = 3.0;
        ftd::RenderBridge rb(L);
        rb.inject_flux(x1, cy, cz, {0, 0, amp});
        rb.inject_flux(x2, cy, cz, {0, 0, amp});
        rb.run(ticks);

        // Single-source reference: source at x1, measure at midpoint (distance = sep/2)
        ftd::RenderBridge rb_single(L);
        rb_single.inject_flux(x1, cy, cz, {0, 0, amp});
        rb_single.run(ticks);

        // Midpoint between sources vs single-source at same distance
        int mx = L / 2;
        double rho_mid = rb.voxels()[rb.lattice().index(mx, cy, cz)].density();
        double rho_single_mid = rb_single.voxels()[rb_single.lattice().index(mx, cy, cz)].density();

        std::cout << "    Two-source density at midpoint: " << rho_mid << "\n";
        std::cout << "    Single-source density at same distance: " << rho_single_mid << "\n";

        // At midpoint, both waves arrive in phase — constructive interference
        // Two sources should produce more than one source at same distance
        check("Constructive interference: two sources > single source",
              rho_mid > rho_single_mid);
    }

    // ================================================================
    // Section 2: Superposition linearity
    // ================================================================
    // J(source_A + source_B) should equal J(source_A) + J(source_B)
    // in the linear regime (before manifestation).
    //
    // The wave equation itself is linear: ∂²J/∂t² = c²∇²J.
    // However, the Gauss projection (SOR solver) introduces path-dependent
    // nonlinearity: proj(A+B) ≠ proj(A) + proj(B) after finite iterations.
    // To test pure wave equation linearity, we disable Gauss projection.
    std::cout << "\n--- Section 2: Superposition Linearity ---\n";
    {
        int L = 32;
        int ticks = 10;

        // Run with both sources (Gauss + genesis OFF for pure wave linearity)
        ftd::RenderBridge rb_both(L);
        rb_both.toggles.gauss_projection = false;
        rb_both.toggles.genesis = false;
        rb_both.inject_flux(10, 16, 16, {0, 0, 2.0});
        rb_both.inject_flux(22, 16, 16, {0, 0, 2.0});
        rb_both.run(ticks);

        // Run source A alone
        ftd::RenderBridge rb_a(L);
        rb_a.toggles.gauss_projection = false;
        rb_a.toggles.genesis = false;
        rb_a.inject_flux(10, 16, 16, {0, 0, 2.0});
        rb_a.run(ticks);

        // Run source B alone
        ftd::RenderBridge rb_b(L);
        rb_b.toggles.gauss_projection = false;
        rb_b.toggles.genesis = false;
        rb_b.inject_flux(22, 16, 16, {0, 0, 2.0});
        rb_b.run(ticks);

        // Check that J_both ≈ J_a + J_b at several test points
        double max_err = 0.0;
        int test_points[] = {12, 14, 16, 18, 20};
        for (int tx : test_points) {
            int idx = rb_both.lattice().index(tx, 16, 16);
            ftd::Vec3 j_both = rb_both.voxels()[idx].flux;
            ftd::Vec3 j_a = rb_a.voxels()[idx].flux;
            ftd::Vec3 j_b = rb_b.voxels()[idx].flux;

            // Linear superposition: j_both should ≈ j_a + j_b
            ftd::Vec3 j_sum = j_a + j_b;
            double err = (j_both - j_sum).mag();
            double scale = std::max(j_both.mag(), 1e-10);
            double rel_err = err / scale;
            if (rel_err > max_err) max_err = rel_err;
        }

        std::cout << "    Max relative error |J_both - (J_a + J_b)| / |J_both|: "
                  << max_err << "\n";

        // Without Gauss projection, the wave equation + damping are strictly linear.
        // Superposition should hold to floating-point precision.
        check("Superposition holds: relative error < 1%", max_err < 0.01);
    }

    // ================================================================
    // Section 3: Destructive interference
    // ================================================================
    // Two anti-phase sources should cancel at midpoint
    std::cout << "\n--- Section 3: Destructive Interference ---\n";
    {
        int L = 48;
        ftd::RenderBridge rb(L);
        int cy = L / 2;
        int cz = L / 2;

        // Two sources with opposite phase (z vs -z polarization)
        double amp = 3.0;
        rb.inject_flux(L/2 - 5, cy, cz, {0, 0, amp});
        rb.inject_flux(L/2 + 5, cy, cz, {0, 0, -amp});

        // Constructive reference
        ftd::RenderBridge rb_con(L);
        rb_con.inject_flux(L/2 - 5, cy, cz, {0, 0, amp});
        rb_con.inject_flux(L/2 + 5, cy, cz, {0, 0, amp});

        rb.run(15);
        rb_con.run(15);

        int mx = L / 2;
        double rho_destructive = rb.voxels()[rb.lattice().index(mx, cy, cz)].density();
        double rho_constructive = rb_con.voxels()[rb_con.lattice().index(mx, cy, cz)].density();

        std::cout << "    Destructive midpoint density: " << rho_destructive << "\n";
        std::cout << "    Constructive midpoint density: " << rho_constructive << "\n";

        // Destructive interference should be weaker
        check("Destructive < constructive at midpoint",
              rho_destructive < rho_constructive);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All interference tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
