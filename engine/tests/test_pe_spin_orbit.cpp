/**
 * Test: PE Spin-Orbit Coupling Force
 *
 * Verifies that particles with nonzero spin experience a force
 * proportional to L·S (orbital angular momentum dot spin).
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
    std::cout << "  TEST: PE Spin-Orbit Coupling Force\n";
    std::cout << "================================================================\n";

    // ---- SO1: Orbiting particle with spin → nonzero force ----
    std::cout << "\n--- SO1: L·S nonzero → force ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Electron orbiting: at (10,0,0) moving in +y → L = r×p in +z
        // Spin in +z → L·S > 0
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        // Nucleus at origin
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        ftd::Vec3 f = pe.compute_force(0);
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        check("SO1: nonzero spin-orbit force", f.mag() > 1e-30);
    }

    // ---- SO2: Zero spin → zero force ----
    std::cout << "\n--- SO2: Zero spin → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0});  // no spin
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("SO2: zero force when spin=0", f.mag() < 1e-30);
    }

    // ---- SO3: Zero velocity → zero orbital angular momentum → zero ----
    std::cout << "\n--- SO3: Zero velocity → zero L → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);  // v=0
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("SO3: zero force when v=0 (no L)", f.mag() < 1e-30);
    }

    // ---- SO4: Toggle OFF → zero ----
    std::cout << "\n--- SO4: Toggle OFF → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = false;

        ftd::Vec3 f = pe.compute_force(0);
        check("SO4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- SO5: Opposite spin → opposite force direction ----
    std::cout << "\n--- SO5: Opposite spin → opposite force ---\n";
    {
        // Spin up
        ftd::ParticleEngine pe_up;
        pe_up.set_damping_enabled(false);
        pe_up.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe_up.add_particle(+1, {0, 0, 0});
        pe_up.toggles.coulomb = false;
        pe_up.toggles.gravity = false;
        pe_up.toggles.spin_orbit = true;
        ftd::Vec3 f_up = pe_up.compute_force(0);

        // Spin down
        ftd::ParticleEngine pe_dn;
        pe_dn.set_damping_enabled(false);
        pe_dn.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, -1, 0);
        pe_dn.add_particle(+1, {0, 0, 0});
        pe_dn.toggles.coulomb = false;
        pe_dn.toggles.gravity = false;
        pe_dn.toggles.spin_orbit = true;
        ftd::Vec3 f_dn = pe_dn.compute_force(0);

        std::cout << "  f_up = (" << f_up.x << ", " << f_up.y << ", " << f_up.z << ")\n";
        std::cout << "  f_dn = (" << f_dn.x << ", " << f_dn.y << ", " << f_dn.z << ")\n";

        // Forces should be opposite
        double sum = (f_up + f_dn).mag();
        double diff = (f_up - f_dn).mag();
        check("SO5: opposite spin → opposite force (|f_up+f_dn| << |f_up-f_dn|)",
              sum < diff * 0.01);
    }

    // ---- SO6: Diagnostic component ----
    std::cout << "\n--- SO6: Diagnostic ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            check("SO6: spin_orbit diag nonzero", fd[0].f_spin_orbit.mag() > 1e-30);
        } else {
            check("SO6: spin_orbit diag nonzero", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All spin-orbit coupling tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
