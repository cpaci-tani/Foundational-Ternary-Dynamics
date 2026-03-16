/**
 * Campaign: Poisson Binding (Phase 3)
 *
 * Tests bound-state behavior with proper 1/r² Coulomb force.
 * Phase 2 showed that the legacy ∇∇J force was too short-ranged
 * to bind at r=6. The Poisson-based force should fix this.
 *
 * 4 checks:
 *   PB1: Opposite charges at r=2 attract
 *   PB2: Opposite charges at r=6 attract (was FAILING with legacy)
 *   PB3: Same-sign at r=6 repel
 *   PB4: Opposite charges at r=10 attract (NEW capability)
 *
 * Theory references:
 *   - SPEC_ENGINE.md Phase 3: Poisson Coulomb
 *   - Plan: Deliverable 6 — campaign_poisson_binding
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

using ftd::Vec3;

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

// Measure radial force component (positive = attractive toward center)
// Returns F.x on the right particle (at mid+r), which should be negative
// (pointing left, toward the source at mid) for attraction.
double measure_radial_force(int lattice_size, int separation, int8_t source_sign,
                            int8_t probe_sign, int settle_ticks = 200) {
    ftd::RenderBridge rb(lattice_size);
    int mid = lattice_size / 2;
    rb.inject_particle(mid, mid, mid, source_sign, {0, 0, ftd::K_B * source_sign});
    rb.inject_particle(mid + separation, mid, mid, probe_sign,
                       {0, 0, ftd::K_B * probe_sign});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.voxels()[rb.lattice().index(mid + separation, mid, mid)].locked = true;
    rb.run(settle_ticks);
    return rb.force_diag_at(mid + separation, mid, mid).f_coulomb.x;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Poisson Binding (Phase 3) — 4 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // PB1: Opposite charges at r=2 attract
    // ================================================================
    std::cout << "\n--- PB1: Opposite at r=2 ---\n";
    {
        double fx = measure_radial_force(32, 2, +1, -1);
        std::cout << "    F.x on -1 probe = " << fx << "\n";
        // Attractive: force on right -1 particle should point left (F.x < 0)
        check("PB1: Opposite charges at r=2 attract (F.x < 0)", fx < 0);
    }

    // ================================================================
    // PB2: Opposite charges at r=6 attract (was FAILING)
    // ================================================================
    std::cout << "\n--- PB2: Opposite at r=6 ---\n";
    {
        double fx = measure_radial_force(32, 6, +1, -1);
        std::cout << "    F.x on -1 probe = " << fx << "\n";
        check("PB2: Opposite charges at r=6 attract (F.x < 0)", fx < 0);
    }

    // ================================================================
    // PB3: Same-sign at r=6 repel
    // ================================================================
    std::cout << "\n--- PB3: Same-sign at r=6 ---\n";
    {
        double fx = measure_radial_force(32, 6, +1, +1);
        std::cout << "    F.x on +1 probe = " << fx << "\n";
        // Repulsive: force on right +1 particle should point right (F.x > 0)
        check("PB3: Same-sign at r=6 repel (F.x > 0)", fx > 0);
    }

    // ================================================================
    // PB4: Opposite charges at r=10 attract (NEW capability)
    // ================================================================
    std::cout << "\n--- PB4: Opposite at r=10 ---\n";
    {
        double fx = measure_radial_force(48, 10, +1, -1, 300);
        std::cout << "    F.x on -1 probe = " << fx << "\n";
        check("PB4: Opposite charges at r=10 attract (F.x < 0)", fx < 0);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Poisson binding tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
