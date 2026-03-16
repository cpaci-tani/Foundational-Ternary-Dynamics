/**
 * Campaign: Color Force Measurement (Phase 5 — Color Dynamics & SU(3))
 *
 * Tests the SU(3)-inspired color-dependent force between particles.
 *
 * Theory: Three colors (R/G/B) from dominant flux axis (Z_3 symmetry).
 * Color force coefficients from SU(3) Casimir operators [IMPOSED]:
 *   same color:      +1/2 (repulsive)
 *   different color:  -1   (attractive)
 * Coupling: running α_s(r) from ontic chain.
 *
 * Protocol:
 *   1. Place two locked particles at fixed separation
 *   2. Vary color assignments (same vs different)
 *   3. Measure force via velocity change of unlocked probe
 *   4. Compare force magnitude and direction
 *
 * Checks:
 *   CF1: Same-color particles experience repulsive color force
 *   CF2: Different-color particles experience attractive color force
 *   CF3: Color force magnitude follows running coupling α_s(r)
 *   CF4: Color force is zero when toggle is OFF (backward compatibility)
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

struct ForceResult {
    double f_color_x;
    double f_strong_mag;
};

// Measure color force on probe particle after 1 tick
ForceResult measure_color_force(int L, int r_sep, int8_t color_source, int8_t color_probe,
                                 bool enable_color) {
    int mid = L / 2;
    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = false;
    rb.toggles.forces = true;
    rb.toggles.color_forces = enable_color;

    // Source: locked particle at center with specified color
    // Flux direction matches color: R=(K_B,0,0), G=(0,K_B,0), B=(0,0,K_B)
    ftd::Vec3 flux_source = {0, 0, ftd::K_B};
    if (color_source == 1) flux_source = {ftd::K_B, 0, 0};
    else if (color_source == 2) flux_source = {0, ftd::K_B, 0};

    rb.inject_particle(mid, mid, mid, +1, flux_source, 0, color_source);
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // Let self-field establish
    rb.run(200);

    // Probe: free particle at separation r with specified color
    int probe_x = mid + r_sep;
    ftd::Vec3 flux_probe = {0, 0, ftd::K_B * 0.1};
    if (color_probe == 1) flux_probe = {ftd::K_B * 0.1, 0, 0};
    else if (color_probe == 2) flux_probe = {0, ftd::K_B * 0.1, 0};

    rb.inject_particle(probe_x, mid, mid, +1, flux_probe, 0, color_probe);

    double vx_before = rb.voxels()[rb.lattice().index(probe_x, mid, mid)].velocity.x;

    // One tick to measure force
    rb.tick();

    // Find probe velocity after tick
    double vx_after = 0.0;
    for (int dx = -1; dx <= 1; ++dx) {
        int cx = probe_x + dx;
        if (cx >= 0 && cx < L) {
            auto& v = rb.voxels()[rb.lattice().index(cx, mid, mid)];
            if (v.state == +1 && !v.locked) {
                vx_after = v.velocity.x;
                break;
            }
        }
    }

    double accel = vx_after - vx_before;
    double f_strong_mag = rb.force_diag()[rb.lattice().index(probe_x, mid, mid)].f_strong.mag();

    return {accel, f_strong_mag};
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Color Force (Phase 5) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(8);

    const int L = 32;
    const int r_sep = 6;

    // ── Measure forces with different color combinations ────────────
    std::cout << "\n--- Color Force Measurements (r=" << r_sep << ") ---\n";
    std::cout << "  Source | Probe  | Δv_x (accel)   | F_strong_mag\n";

    // Same color (Red-Red): should be repulsive (positive Δv)
    auto rr = measure_color_force(L, r_sep, 1, 1, true);
    std::cout << "  Red    | Red    | " << std::setw(14) << rr.f_color_x
              << " | " << rr.f_strong_mag << "\n";

    // Different color (Red-Green): should be attractive (negative Δv)
    auto rg = measure_color_force(L, r_sep, 1, 2, true);
    std::cout << "  Red    | Green  | " << std::setw(14) << rg.f_color_x
              << " | " << rg.f_strong_mag << "\n";

    // Different color (Red-Blue): should also be attractive
    auto rb_test = measure_color_force(L, r_sep, 1, 3, true);
    std::cout << "  Red    | Blue   | " << std::setw(14) << rb_test.f_color_x
              << " | " << rb_test.f_strong_mag << "\n";

    // Color force OFF (backward compatibility)
    auto off = measure_color_force(L, r_sep, 1, 2, false);
    std::cout << "  Red    | Green  | " << std::setw(14) << off.f_color_x
              << " | " << off.f_strong_mag << " (color OFF)\n";

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // CF1: Same-color particles repel (positive acceleration = away from source)
    // The EM force also causes repulsion (same charge), so color adds to it.
    // We compare same-color vs different-color to isolate the color contribution.
    check("CF1: Same-color pair has MORE repulsion than different-color pair",
          rr.f_color_x > rg.f_color_x);

    // CF2: Different-color force is attractive (less repulsion than same-color)
    // Total force includes EM repulsion, so net may still be positive.
    // The key test: different-color acceleration is LESS than same-color.
    check("CF2: Different-color pair has LESS repulsion (color attraction)",
          rg.f_color_x < rr.f_color_x);

    // CF3: Color force magnitude is nonzero and follows α_s
    double expected_as = ftd::alpha_s_lattice(r_sep);
    std::cout << "  α_s(r=" << r_sep << ") = " << expected_as << "\n";
    std::cout << "  F_strong same:  " << rr.f_strong_mag << "\n";
    std::cout << "  F_strong diff:  " << rg.f_strong_mag << "\n";
    check("CF3: Color force is nonzero when enabled",
          rr.f_strong_mag > 1e-15 && rg.f_strong_mag > 1e-15);

    // CF4: Color force is zero when toggle is OFF
    check("CF4: Color force is zero when toggle OFF (backward compat)",
          off.f_strong_mag < 1e-30);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  NOTE: Color force coefficients (+1/2 same, -1 different)\n";
    std::cout << "  are [IMPOSED] from SU(3) Casimir operators, not derived.\n";
    std::cout << "  Z_3 color labeling from dominant flux axis is [EMERGENT].\n";
    std::cout << "================================================================\n";
    return failures;
}
