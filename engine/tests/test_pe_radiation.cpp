/**
 * Test: PE Radiation Reaction Force
 *
 * Verifies Abraham-Lorentz radiation damping: accelerating charges
 * experience a force opposing their motion proportional to a².
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
    std::cout << "  TEST: PE Radiation Reaction Force\n";
    std::cout << "================================================================\n";

    // ---- RD1: Accelerating charge → force opposes motion ----
    std::cout << "\n--- RD1: Radiation opposes motion ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Electron moving in +x, with prev_acceleration in +x
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});

        // Manually set prev_acceleration (simulates previous tick)
        pe.particles()[0].prev_acceleration = {0.05, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("RD1: radiation force opposes motion (f.x < 0)", f.x < 0);
    }

    // ---- RD2: Zero acceleration → zero radiation ----
    std::cout << "\n--- RD2: No acceleration → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});
        // prev_acceleration = {0,0,0} (default)

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        ftd::Vec3 f = pe.compute_force(0);
        check("RD2: zero force when a_prev=0", f.mag() < 1e-30);
    }

    // ---- RD3: Toggle OFF → zero ----
    std::cout << "\n--- RD3: Toggle OFF → zero ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});
        pe.particles()[0].prev_acceleration = {0.05, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = false;

        ftd::Vec3 f = pe.compute_force(0);
        check("RD3: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- RD4: Higher acceleration → larger radiation force ----
    std::cout << "\n--- RD4: |F_rad| scales with a² ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        pe.particles()[0].prev_acceleration = {0.01, 0, 0};
        double f_small = pe.compute_force(0).mag();

        pe.particles()[0].prev_acceleration = {0.1, 0, 0};
        double f_large = pe.compute_force(0).mag();

        // Should scale as a², so 10x acceleration → 100x force
        double ratio = f_large / f_small;
        check("RD4: 10x acceleration → ~100x force", ratio > 50.0 && ratio < 200.0);
    }

    // ---- RD5: System loses energy over time ----
    std::cout << "\n--- RD5: Energy loss over time ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.1);

        // Two opposite charges → orbiting system
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].locked = true;
        pe.add_particle(-1, {20, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, -1, 0);

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        // Run a few ticks to build up prev_acceleration
        pe.run(10);
        auto d0 = pe.diagnostics();

        pe.run(200);
        auto d1 = pe.diagnostics();

        // With radiation on, energy should decrease (radiation removes energy)
        check("RD5: energy decreases with radiation", d1.total_energy < d0.total_energy);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All radiation reaction tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
