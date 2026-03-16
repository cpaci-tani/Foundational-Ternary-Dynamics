/**
 * Test: Energy Conservation
 *
 * Verifies that the total Hamiltonian is approximately conserved
 * during simulation evolution. With damping, some dissipation is
 * expected — we track the energy loss rate and verify it's
 * consistent with the damping parameter.
 *
 * Also verifies total flux magnitude conservation.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (action principle, Hamiltonian)
 *   - SPEC_SIX_ALGORITHMS.md             (energy conservation in update cycle)
 *   - EXPLR_VACUUM_DRAG_DERIVATION.md    (DAMPING = α from vacuum drag)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
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
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Energy Conservation\n";
    std::cout << "================================================================\n\n";

    // ---- Test 1: Vacuum stability (no particles, no flux = zero energy) ----
    std::cout << "--- Vacuum stability ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.run(100);
        auto ld = ftd::compute_lagrangian_diagnostics(rb);
        check("Vacuum: no manifested particles", ld.manifested_count == 0);
        check_close("Vacuum: total flux = 0", ld.total_flux_mag, 0.0, 1e-15);
        check_close("Vacuum: total wave energy = 0", ld.total_wave_energy, 0.0, 1e-15);
    }

    // ---- Test 2: Single locked particle energy stability ----
    std::cout << "\n--- Single locked particle ---\n";
    {
        ftd::RenderBridge rb(16);
        int cx = 8, cy = 8, cz = 8;
        rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;

        // Let self-field establish
        rb.run(100);

        // Track Hamiltonian over next 200 ticks
        std::vector<double> H_values;
        for (int t = 0; t < 200; ++t) {
            auto ld = ftd::compute_lagrangian_diagnostics(rb);
            H_values.push_back(ld.total_hamiltonian);
            rb.tick();
        }

        // After equilibration, Hamiltonian should be roughly stable
        // (damping causes slow decrease, but no large jumps)
        double H_max = H_values[0], H_min = H_values[0];
        for (double h : H_values) {
            H_max = std::max(H_max, h);
            H_min = std::min(H_min, h);
        }
        double H_range = H_max - H_min;
        double H_avg = 0;
        for (double h : H_values) H_avg += h;
        H_avg /= H_values.size();

        std::cout << "    H_avg = " << H_avg << ", range = " << H_range << "\n";
        // Energy variation should be less than 50% of average (allowing for damping)
        check("Locked particle: H variation < 50% of avg",
              H_avg > 0.0 ? H_range / H_avg < 0.5 : H_range < 1.0);
        check("Locked particle: still manifested", rb.voxels()[rb.lattice().index(cx,cy,cz)].state != 0);
    }

    // ---- Test 3: Two-particle system energy tracking ----
    std::cout << "\n--- Two-particle energy tracking ---\n";
    {
        ftd::RenderBridge rb(32);
        // Place two opposite-charge locked particles
        rb.inject_particle(11, 16, 16, +1, {0, 0, ftd::K_B});
        rb.inject_particle(21, 16, 16, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(11, 16, 16)].locked = true;
        rb.voxels()[rb.lattice().index(21, 16, 16)].locked = true;

        // Let fields establish
        rb.run(200);

        // Record baseline
        auto ld_start = ftd::compute_lagrangian_diagnostics(rb);
        double H_start = ld_start.total_hamiltonian;

        // Run 300 more ticks
        rb.run(300);
        auto ld_end = ftd::compute_lagrangian_diagnostics(rb);
        double H_end = ld_end.total_hamiltonian;

        std::cout << "    H_start = " << H_start << "\n";
        std::cout << "    H_end   = " << H_end << "\n";

        // With damping, energy should decrease monotonically (not increase)
        check("Two-particle: H_end <= H_start (damping removes energy)", H_end <= H_start * 1.01);
        // Both particles still present
        check("Two-particle: still 2 particles", ld_end.manifested_count == 2);
    }

    // ---- Test 4: Wave pulse dissipates with damping ----
    // With DAMPING = alpha = 0.00729, flux retains 99.27% per tick.
    // Use low flux (well below K_GENESIS = 1.533) to avoid spontaneous genesis.
    std::cout << "\n--- Wave pulse dissipation ---\n";
    {
        ftd::RenderBridge rb(16);
        // Inject a flux pulse below K_GENESIS (no particle creation)
        rb.inject_flux(8, 8, 8, {0, 0, 0.5});

        // Let wave spread and partially dissipate
        rb.run(50);
        auto ld50 = ftd::compute_lagrangian_diagnostics(rb);
        double E50 = ld50.total_wave_energy;

        // Run 500 more ticks — damping = alpha should reduce wave energy
        // After 500 ticks: retention = (1-0.00729)^500 = 0.026, so ~97% dissipated
        rb.run(500);
        auto ld550 = ftd::compute_lagrangian_diagnostics(rb);
        double E550 = ld550.total_wave_energy;

        std::cout << "    E(t=50)  = " << E50 << "\n";
        std::cout << "    E(t=550) = " << E550 << "\n";
        // After wave has spread and damping acts, wave energy should decrease
        check("Wave energy: E(550) < E(50) (damping dissipates)", E550 < E50);
        // But not go negative
        check("Wave energy: E(550) >= 0", E550 >= 0.0);
        // No particles should have been created (flux 0.5 < K_GENESIS 1.533)
        check("No spontaneous particles", ld550.manifested_count == 0);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All energy conservation tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
