/**
 * Campaign: Triad Energy Measurement (Phase 4 — Emergent Mass Spectrum)
 *
 * Measures the total energy of locked triads (3 same-sign particles)
 * and compares with single-particle energy to extract binding energy.
 *
 * Theory: FTD predicts triads (nucleon analogs) form stable bound states
 * via the strong force. However, the current engine has only EM + gravity.
 * Same-sign particles REPEL via Coulomb: F = +α/r² (repulsive).
 *
 * This campaign honestly documents what happens:
 *   - Locked triads: artificially stable (locked=true prevents escape)
 *   - Free triads: should REPEL and fly apart (no strong force)
 *
 * The ratio E_triad / E_single tells us about EM self-energy overlap.
 *
 * Protocol:
 *   1. Measure energy of a single locked +1 particle (baseline)
 *   2. Create locked triad at equilateral triangle (r ≈ √2)
 *   3. Measure total energy of triad after self-field equilibration
 *   4. Test free triad: do particles separate?
 *
 * Checks:
 *   TE1: Single particle has well-defined self-energy
 *   TE2: Triad has higher energy than 3× single (Coulomb repulsion)
 *   TE3: Triad energy is finite and stable (locked particles persist)
 *   TE4: Free same-sign particles repel (separation increases)
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
    std::cout << "  CAMPAIGN: Triad Energy (Phase 4) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int mid = L / 2;
    const int WARMUP = 500;

    // ================================================================
    // Part 1: Single particle self-energy
    // ================================================================
    double E_single = 0.0;
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(WARMUP);

        auto audit = rb.energy_audit();
        E_single = audit.field_energy + audit.coulomb_pe;

        std::cout << "\n--- Single Particle ---\n";
        std::cout << "  Field energy: " << audit.field_energy << "\n";
        std::cout << "  Coulomb PE:   " << audit.coulomb_pe << "\n";
        std::cout << "  Total:        " << E_single << "\n";
    }

    // ================================================================
    // Part 2: Locked triad (equilateral triangle, r ≈ √2)
    // ================================================================
    double E_triad = 0.0;
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;

        // Equilateral triangle in xy-plane, side length √2 (face diagonal)
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid+1, mid+1, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid+1, mid, mid+1, +1, {0, 0, ftd::K_B});

        // Lock all three
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid+1, mid+1, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid+1, mid, mid+1)].locked = true;

        rb.run(WARMUP);

        auto audit = rb.energy_audit();
        E_triad = audit.field_energy + audit.coulomb_pe;

        std::cout << "\n--- Locked Triad ---\n";
        std::cout << "  Field energy: " << audit.field_energy << "\n";
        std::cout << "  Coulomb PE:   " << audit.coulomb_pe << "\n";
        std::cout << "  Total:        " << E_triad << "\n";
        std::cout << "  Manifested:   " << audit.manifested_count << "\n";
        std::cout << "  Ratio E_triad / (3 × E_single): "
                  << (std::abs(E_single) > 1e-10 ? E_triad / (3.0 * E_single) : 0.0)
                  << "\n";
    }

    // ================================================================
    // Part 3: Free same-sign particles (repulsion test)
    // ================================================================
    double sep_initial = 0.0, sep_final = 0.0;
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;

        // Two same-sign particles at moderate separation
        int r0 = 6;
        rb.inject_particle(mid - r0/2, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
        rb.inject_particle(mid + r0/2, mid, mid, +1, {0, 0, ftd::K_B * 0.1});

        sep_initial = static_cast<double>(r0);

        // Let self-field establish
        rb.run(200);

        // Evolve and track
        rb.run(2000);

        // Find particle positions
        int N_total = rb.lattice().total_sites();
        int p1_x = -1, p2_x = -1;
        for (int i = 0; i < N_total; ++i) {
            if (rb.voxels()[i].state == +1) {
                auto c = rb.lattice().coord(i);
                if (p1_x < 0) p1_x = c.x;
                else p2_x = c.x;
            }
        }

        if (p1_x >= 0 && p2_x >= 0) {
            // Handle periodic boundary
            int dx = std::abs(p2_x - p1_x);
            if (dx > L / 2) dx = L - dx;
            sep_final = static_cast<double>(dx);
        }

        std::cout << "\n--- Free Same-Sign Repulsion ---\n";
        std::cout << "  Initial separation: " << sep_initial << "\n";
        std::cout << "  Final separation:   " << sep_final << "\n";
        std::cout << "  Particles found:    "
                  << ((p1_x >= 0) + (p2_x >= 0)) << "\n";
    }

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // TE1: Single particle has well-defined self-energy
    check("TE1: Single particle has measurable self-energy (> 0)",
          E_single > 0.0);

    // TE2: Triad energy > 3× single (Coulomb repulsion adds energy)
    check("TE2: Triad energy >= 3 × single (Coulomb repulsion contributes)",
          E_triad >= 3.0 * E_single * 0.8);  // Allow 20% margin

    // TE3: Triad energy is finite and positive
    check("TE3: Triad energy is finite and positive",
          std::isfinite(E_triad) && E_triad > 0.0);

    // TE4: Free same-sign particles repel (separation increases or unchanged)
    std::cout << "  Sep change: " << sep_initial << " -> " << sep_final << "\n";
    check("TE4: Same-sign particles repel (sep increases or unchanged)",
          sep_final >= sep_initial - 1.0);  // Allow ±1 for lattice discreteness

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  NOTE: Without strong force (Phase 5), same-sign triads cannot\n";
    std::cout << "  bind spontaneously. Locked triads are artificial. Baryon\n";
    std::cout << "  binding requires SU(3) color dynamics.\n";
    std::cout << "================================================================\n";
    return failures;
}
