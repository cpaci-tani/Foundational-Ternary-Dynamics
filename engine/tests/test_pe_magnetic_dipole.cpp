/**
 * Test: PE Magnetic Dipole-Dipole Force
 *
 * Verifies that particles with nonzero spin_axis experience
 * the classical magnetic dipole-dipole interaction.
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
    std::cout << "  TEST: PE Magnetic Dipole-Dipole Force\n";
    std::cout << "================================================================\n";

    // ---- MD1: Aligned dipoles along separation axis → attractive ----
    std::cout << "\n--- MD1: Aligned dipoles along axis → attract ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Both spin along x (parallel to separation axis)
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].spin_axis = {1, 0, 0};
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[1].spin_axis = {1, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        ftd::Vec3 f = pe.compute_force(0);
        // Head-to-tail along axis → attractive (f.x > 0)
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        check("MD1: aligned along axis → attractive (f.x > 0)", f.x > 0);
    }

    // ---- MD2: Anti-aligned dipoles along axis → repulsive ----
    std::cout << "\n--- MD2: Anti-aligned along axis → repel ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].spin_axis = {1, 0, 0};
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[1].spin_axis = {-1, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        ftd::Vec3 f = pe.compute_force(0);
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        check("MD2: anti-aligned along axis → repulsive (f.x < 0)", f.x < 0);
    }

    // ---- MD3: Zero spin_axis → zero ----
    std::cout << "\n--- MD3: Zero spin_axis → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0});   // no spin
        pe.add_particle(+1, {10, 0, 0});   // no spin

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("MD3: zero force when spin_axis=0", f.mag() < 1e-30);
    }

    // ---- MD4: Toggle OFF → zero ----
    std::cout << "\n--- MD4: Toggle OFF → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = false;

        ftd::Vec3 f = pe.compute_force(0);
        check("MD4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- MD5: Force decays as 1/r^4 ----
    std::cout << "\n--- MD5: 1/r^4 decay ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].spin_axis = {0, 0, 1};
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[1].spin_axis = {0, 0, 1};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        double f_near = pe.compute_force(0).mag();

        pe.particles()[1].position = {10, 0, 0};
        double f_far = pe.compute_force(0).mag();

        // r doubled → force should decrease by ~2^4 = 16
        double ratio = f_near / f_far;
        std::cout << "  f_near=" << f_near << " f_far=" << f_far
                  << " ratio=" << ratio << " (expect ~16)\n";
        check("MD5: force ratio ~16 when r doubles (1/r^4)", ratio > 10.0 && ratio < 25.0);
    }

    // ---- MD6: Diagnostic component ----
    std::cout << "\n--- MD6: Diagnostic ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            check("MD6: magnetic_dipole diag nonzero", fd[0].f_magnetic_dipole.mag() > 1e-30);
        } else {
            check("MD6: magnetic_dipole diag nonzero", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All magnetic dipole-dipole tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
