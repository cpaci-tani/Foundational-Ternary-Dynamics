/**
 * Test: PE Strong (Yukawa + Confinement) Force
 *
 * Verifies color-dependent strong force: different colors attract,
 * same colors repel, colorless particles unaffected.
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
    std::cout << "  TEST: PE Strong (Yukawa + Confinement) Force\n";
    std::cout << "================================================================\n";

    // ---- ST1: Different colors attract ----
    std::cout << "\n--- ST1: Different colors attract ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);   // color=red
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 2);   // color=green

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("ST1: different colors → attractive (f.x > 0)", f.x > 0);
    }

    // ---- ST2: Same colors repel ----
    std::cout << "\n--- ST2: Same colors repel ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);   // color=red
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);   // color=red

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("ST2: same colors → repulsive (f.x < 0)", f.x < 0);
    }

    // ---- ST3: Colorless → zero ----
    std::cout << "\n--- ST3: Colorless → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 0);   // colorless
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);   // colored

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("ST3: colorless → zero force", f.mag() < 1e-30);
    }

    // ---- ST4: Toggle OFF → zero ----
    std::cout << "\n--- ST4: Toggle OFF → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 2);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = false;

        ftd::Vec3 f = pe.compute_force(0);
        check("ST4: toggle off → zero", f.mag() < 1e-30);
    }

    // ---- ST5: Color factor ratio ----
    std::cout << "\n--- ST5: Color factor ratio ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);

        // Same color pair
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        double f_same = pe.compute_force(0).mag();

        // Different color pair
        pe.particles()[1].color = 2;
        double f_diff = pe.compute_force(0).mag();

        // Ratio should be 0.5 (same cf=0.5 vs diff cf=-1.0)
        double ratio = f_same / f_diff;
        check("ST5: |F_same|/|F_diff| ~ 0.5", std::abs(ratio - 0.5) < 0.1);
    }

    // ---- ST6: Force diagnostic ----
    std::cout << "\n--- ST6: Force diagnostic ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 2);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            check("ST6: strong diag nonzero", fd[0].f_strong.mag() > 1e-20);
        } else {
            check("ST6: strong diag nonzero", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All strong force tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
