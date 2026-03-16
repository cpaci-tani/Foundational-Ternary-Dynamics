/**
 * Test: ParticleToggles — Per-force toggle control for ParticleEngine
 *
 * Verifies that each toggle in ParticleToggles enables/disables its force,
 * and that force_diag_ correctly decomposes forces by type.
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
    std::cout << "  TEST: ParticleToggles — Per-Force Control\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Section 1: Default toggles match legacy behavior
    // ================================================================
    std::cout << "\n--- Section 1: Default Toggle State ---\n";
    {
        ftd::ParticleEngine pe;
        check("Default: coulomb ON", pe.toggles.coulomb == true);
        check("Default: gravity ON", pe.toggles.gravity == true);
        check("Default: damping ON", pe.toggles.damping == true);
        check("Default: lorentz OFF", pe.toggles.lorentz == false);
        check("Default: exchange OFF", pe.toggles.exchange == false);
        check("Default: strong OFF", pe.toggles.strong == false);
    }

    // ================================================================
    // Section 2: Backward-compatible setters still work
    // ================================================================
    std::cout << "\n--- Section 2: Backward Compatibility ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        check("set_damping_enabled(false) → toggles.damping=false", pe.toggles.damping == false);
        check("damping_enabled() reflects toggles", pe.damping_enabled() == false);

        pe.set_gravity_enabled(false);
        check("set_gravity_enabled(false) → toggles.gravity=false", pe.toggles.gravity == false);
        check("gravity_enabled() reflects toggles", pe.gravity_enabled() == false);
    }

    // ================================================================
    // Section 3: Coulomb toggle — ON produces force, OFF produces zero
    // ================================================================
    std::cout << "\n--- Section 3: Coulomb Toggle ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0});
        pe.add_particle(+1, {10, 0, 0});

        // Coulomb ON
        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        ftd::Vec3 f_on = pe.compute_force(0);
        double mag_on = f_on.mag();

        // Coulomb OFF
        pe.toggles.coulomb = false;
        ftd::Vec3 f_off = pe.compute_force(0);
        double mag_off = f_off.mag();

        check("Coulomb ON → nonzero force", mag_on > 1e-10);
        check("Coulomb OFF → zero force", mag_off < 1e-30);
    }

    // ================================================================
    // Section 4: Gravity toggle — ON produces force, OFF produces zero
    // ================================================================
    std::cout << "\n--- Section 4: Gravity Toggle ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(0, {0, 0, 0}, {}, 1.0);   // neutral, mass 1
        pe.add_particle(0, {10, 0, 0}, {}, 1.0);   // neutral, mass 1

        // Gravity ON
        pe.toggles.coulomb = false;
        pe.toggles.gravity = true;
        ftd::Vec3 f_on = pe.compute_force(0);
        double mag_on = f_on.mag();

        // Gravity OFF
        pe.toggles.gravity = false;
        ftd::Vec3 f_off = pe.compute_force(0);
        double mag_off = f_off.mag();

        check("Gravity ON → nonzero force", mag_on > 1e-10);
        check("Gravity OFF → zero force", mag_off < 1e-30);
    }

    // ================================================================
    // Section 5: Force diagnostic decomposition
    // ================================================================
    std::cout << "\n--- Section 5: Force Diagnostics ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B);
        pe.add_particle(-1, {10, 0, 0}, {}, ftd::K_B);

        pe.toggles.coulomb = true;
        pe.toggles.gravity = true;

        // Run one tick to populate force_diag_
        pe.tick();

        const auto& fd = pe.force_diag();
        check("force_diag has 2 entries", fd.size() == 2);

        if (fd.size() >= 2) {
            // Particle 0 (+1) with particle 1 (-1): opposite charges → attractive Coulomb
            double fc_mag = fd[0].f_coulomb.mag();
            double fg_mag = fd[0].f_gravity.mag();
            check("Coulomb diag nonzero", fc_mag > 1e-10);
            check("Gravity diag nonzero", fg_mag > 1e-10);

            // Coulomb for opposite signs: attractive → toward +x
            check("Coulomb attractive (toward +x)", fd[0].f_coulomb.x > 0);

            // Gravity: always attractive → toward +x
            check("Gravity attractive (toward +x)", fd[0].f_gravity.x > 0);

            // Total should equal sum
            ftd::Vec3 tot = fd[0].total();
            ftd::Vec3 expected = fd[0].f_coulomb + fd[0].f_gravity;
            double diff = (tot - expected).mag();
            check("total() equals sum of components", diff < 1e-15);
        }
    }

    // ================================================================
    // Section 6: enable_all() and minimal()
    // ================================================================
    std::cout << "\n--- Section 6: enable_all() / minimal() ---\n";
    {
        ftd::ParticleToggles t;
        t.enable_all();
        check("enable_all: lorentz ON", t.lorentz == true);
        check("enable_all: strong ON", t.strong == true);
        check("enable_all: relativistic ON", t.relativistic == true);

        t.minimal();
        check("minimal: coulomb ON", t.coulomb == true);
        check("minimal: lorentz OFF", t.lorentz == false);
        check("minimal: exchange OFF", t.exchange == false);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All particle toggle tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
