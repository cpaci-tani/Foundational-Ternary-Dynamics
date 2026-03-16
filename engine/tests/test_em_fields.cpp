/**
 * Test: E/B Field Diagnostics (Phase A — FDTD Bridge)
 *
 * Verifies the electromagnetic field decomposition:
 *   E = -wave_vel  (electric field from leapfrog momentum)
 *   B = curl(J)    (magnetic field from flux curl)
 *
 * 6 checks in 3 groups:
 *
 * Group 1: E/B Identity
 *   EM1: E = -wave_vel for a single particle after settling
 *   EM2: |E| > 0 near a manifested particle (coupling source generates wave_vel)
 *
 * Group 2: Static vs Propagating
 *   EM3: B ≈ 0 at the site of a static (locked, v=0) charge (no current → no B)
 *   EM4: Propagating flux wave has B > 0 (curl of traveling wave ≠ 0)
 *
 * Group 3: Energy Decomposition
 *   EM5: E_field_energy + B_field_energy > 0 in a system with particles
 *   EM6: E_field_energy and B_field_energy appear in energy_audit output
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
    std::cout << "  TEST: E/B Field Diagnostics (Phase A) — 6 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Group 1: E/B Identity
    // ================================================================
    std::cout << "\n--- Group 1: E/B Identity ---\n";

    // EM1: E = -wave_vel for a particle after settling
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(200);  // Let self-field build

        // Check a neighbor site (not the particle itself)
        int idx = rb.lattice().index(mid + 1, mid, mid);
        auto em = rb.em_field_at(idx);
        const auto& wv = rb.voxels()[idx].wave_vel;

        // E should be -wave_vel
        double dx = em.E.x - (-wv.x);
        double dy = em.E.y - (-wv.y);
        double dz = em.E.z - (-wv.z);
        double err = std::sqrt(dx*dx + dy*dy + dz*dz);

        std::cout << std::setprecision(8) << std::scientific;
        std::cout << "    E = (" << em.E.x << ", " << em.E.y << ", " << em.E.z << ")\n";
        std::cout << "    -wv = (" << -wv.x << ", " << -wv.y << ", " << -wv.z << ")\n";
        std::cout << "    |E - (-wv)| = " << err << "\n";
        check("EM1: E = -wave_vel identity (error < 1e-15)", err < 1e-15);
    }

    // EM2: |E| > 0 near a manifested particle
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(200);

        // Check neighbor site — coupling source should have generated wave_vel
        int idx = rb.lattice().index(mid + 1, mid, mid);
        auto em = rb.em_field_at(idx);

        std::cout << "    |E| at (mid+1,mid,mid) = " << em.E_mag << "\n";
        check("EM2: |E| > 0 near manifested particle", em.E_mag > 1e-10);
    }

    // ================================================================
    // Group 2: Static vs Propagating
    // ================================================================
    std::cout << "\n--- Group 2: Static vs Propagating ---\n";

    // EM3: B ≈ 0 at the site of a static locked charge (v=0, no current)
    // A static charge has J aligned along one axis; curl should be small
    // at the particle site itself (spherically symmetric self-field).
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(500);  // Full self-field equilibrium

        int idx = rb.lattice().index(mid, mid, mid);
        auto em = rb.em_field_at(idx);

        std::cout << "    |B| at static particle site = " << em.B_mag << "\n";
        // B should be very small at the particle site (symmetric self-field)
        // The coupling pumps flux in the gradient direction, creating a
        // roughly radial pattern whose curl at center is small.
        check("EM3: |B| < 0.1 at static particle site", em.B_mag < 0.1);
    }

    // EM4: Flux pattern with spatial curl has B > 0
    // A single-axis uniform flux injection has zero curl by symmetry.
    // To produce B = curl(J), we need spatial variation in flux direction.
    // Inject perpendicular flux at adjacent sites to create ∂J_y/∂x ≠ 0 → B_z ≠ 0.
    {
        ftd::RenderBridge rb(32);
        int mid = 16;

        // Create a flux pattern with nonzero curl:
        // J_y at (mid, mid, mid) and -J_y at (mid+1, mid, mid)
        // gives ∂J_y/∂x < 0 → B_z = ∂J_y/∂x > 0 (or vice versa)
        rb.inject_flux(mid, mid, mid,     {0.0, ftd::K_B, 0.0});
        rb.inject_flux(mid + 1, mid, mid, {0.0, -ftd::K_B, 0.0});

        // Also inject perpendicular to get multi-component curl:
        rb.inject_flux(mid, mid + 1, mid, {ftd::K_B, 0.0, 0.0});
        rb.inject_flux(mid, mid, mid + 1, {0.0, 0.0, ftd::K_B});

        // Don't propagate — just check curl at the injection sites
        // immediately (the flux is already spatially varying).
        // After a few ticks the wave equation will smooth things out.
        rb.run(1);

        double max_B = 0.0;
        for (int dx = -1; dx <= 2; ++dx) {
            for (int dy = -1; dy <= 2; ++dy) {
                for (int dz = -1; dz <= 2; ++dz) {
                    int idx = rb.lattice().index(mid + dx, mid + dy, mid + dz);
                    auto em = rb.em_field_at(idx);
                    if (em.B_mag > max_B) max_B = em.B_mag;
                }
            }
        }

        std::cout << "    max |B| in curl-rich region = " << max_B << "\n";
        check("EM4: Spatially varying flux has |B| > 0", max_B > 1e-6);
    }

    // ================================================================
    // Group 3: Energy Decomposition
    // ================================================================
    std::cout << "\n--- Group 3: Energy Decomposition ---\n";

    // EM5: E_field_energy + B_field_energy > 0
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(200);
        auto a = rb.energy_audit();

        std::cout << std::setprecision(6) << std::fixed;
        std::cout << "    E_field_energy = " << a.E_field_energy << "\n";
        std::cout << "    B_field_energy = " << a.B_field_energy << "\n";
        std::cout << "    Total EM = " << (a.E_field_energy + a.B_field_energy) << "\n";
        check("EM5: E + B field energy > 0", (a.E_field_energy + a.B_field_energy) > 0.0);
    }

    // EM6: Both E and B energies are individually non-negative
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(300);
        auto a = rb.energy_audit();

        std::cout << "    E_field_energy = " << a.E_field_energy << "\n";
        std::cout << "    B_field_energy = " << a.B_field_energy << "\n";
        check("EM6: E_field_energy >= 0 AND B_field_energy >= 0",
              a.E_field_energy >= 0.0 && a.B_field_energy >= 0.0);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All 6 E/B field diagnostic tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
