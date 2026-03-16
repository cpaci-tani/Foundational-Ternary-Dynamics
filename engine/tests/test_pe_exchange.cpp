/**
 * Test: PE Exchange (Pauli) Force
 *
 * Verifies that same-spin, same-charge particles experience repulsive
 * exchange force, and that the force is zero for different quantum numbers.
 */

#include <cmath>
#include <iostream>
#include "ftd/particle_engine.h"
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
    std::cout << "  TEST: PE Exchange (Pauli) Force\n";
    std::cout << "================================================================\n";

    // ---- EX1: Same spin, same charge → nonzero repulsive force ----
    std::cout << "\n--- EX1: Same spin, same charge → repulsion ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Two electrons: charge -1, spin +1, at separation 3
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        // Only exchange on
        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("EX1a: nonzero exchange force", f.mag() > 1e-20);
        check("EX1b: repulsive (away from j, f.x < 0)", f.x < 0);
    }

    // ---- EX2: Different spin → zero exchange force ----
    std::cout << "\n--- EX2: Different spin → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, -1, 0);  // opposite spin

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("EX2: zero force for different spin", f.mag() < 1e-30);
    }

    // ---- EX3: Same spin, different charge → zero ----
    std::cout << "\n--- EX3: Same spin, different charge → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);  // different charge

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("EX3: zero force for different charge", f.mag() < 1e-30);
    }

    // ---- EX4: Toggle OFF → zero ----
    std::cout << "\n--- EX4: Toggle OFF → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = false;  // OFF

        ftd::Vec3 f = pe.compute_force(0);
        check("EX4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- EX5: Exponential decay with distance ----
    std::cout << "\n--- EX5: Exponential decay ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {2, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;
        double f_near = pe.compute_force(0).mag();

        // Move particle further
        pe.particles()[1].position = {6, 0, 0};
        double f_far = pe.compute_force(0).mag();

        check("EX5: force decreases with distance", f_near > f_far * 5.0);
    }

    // ---- EX6: Diagnostic component matches ----
    std::cout << "\n--- EX6: Diagnostic decomposition ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        // Run tick to populate force_diag
        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            check("EX6: exchange diag nonzero", fd[0].f_exchange.mag() > 1e-20);
        } else {
            check("EX6: exchange diag nonzero", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All exchange force tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
