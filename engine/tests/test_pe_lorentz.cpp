/**
 * Test: PE Lorentz Force
 *
 * Verifies that moving charges in magnetic fields from spinning particles
 * experience a force perpendicular to their velocity (v × B).
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
    std::cout << "  TEST: PE Lorentz Force\n";
    std::cout << "================================================================\n";

    // ---- LZ1: Moving charge near spinning particle → nonzero force ----
    std::cout << "\n--- LZ1: Moving charge near dipole ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Particle 0: moving in +x, charge -1
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        // Particle 1: stationary spinning particle producing B-field
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("LZ1: nonzero Lorentz force", f.mag() > 1e-30);
    }

    // ---- LZ2: Stationary charge → zero Lorentz ----
    std::cout << "\n--- LZ2: Stationary → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);  // v=0
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("LZ2: zero force when v=0", f.mag() < 1e-30);
    }

    // ---- LZ3: Force perpendicular to velocity ----
    std::cout << "\n--- LZ3: F ⊥ v ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Moving in +x
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        // Dipole along z-axis at (0, 10, 0)
        pe.add_particle(+1, {0, 10, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::Vec3 v = {0.1, 0, 0};
        // v · F should be ~0 (Lorentz does no work)
        double v_dot_f = v.x * f.x + v.y * f.y + v.z * f.z;
        double relative = (f.mag() > 1e-30) ? std::abs(v_dot_f) / (v.mag() * f.mag()) : 0.0;
        std::cout << "  v.F=" << v_dot_f << " |v|=" << v.mag() << " |F|=" << f.mag()
                  << " cos=" << relative << "\n";
        check("LZ3: F perpendicular to v (cos < 0.1)", relative < 0.1);
    }

    // ---- LZ4: Toggle OFF → zero ----
    std::cout << "\n--- LZ4: Toggle OFF → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = false;

        ftd::Vec3 f = pe.compute_force(0);
        check("LZ4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- LZ5: No dipole source → zero ----
    std::cout << "\n--- LZ5: No dipole → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});  // no spin
        pe.add_particle(+1, {10, 0, 0});                // no spin

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("LZ5: zero when no dipole sources", f.mag() < 1e-30);
    }

    // ---- LZ6: Diagnostic component ----
    std::cout << "\n--- LZ6: Diagnostic ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            check("LZ6: lorentz diag nonzero", fd[0].f_lorentz.mag() > 1e-30);
        } else {
            check("LZ6: lorentz diag nonzero", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Lorentz force tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
