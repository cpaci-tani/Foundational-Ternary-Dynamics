/**
 * Test: PE Relativistic Corrections
 *
 * Verifies that high-speed particles experience reduced acceleration
 * due to relativistic mass increase (gamma factor).
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
    std::cout << "  TEST: PE Relativistic Corrections\n";
    std::cout << "================================================================\n";

    // ---- RE1: High-speed → reduced acceleration ----
    std::cout << "\n--- RE1: Relativistic mass increase ---\n";
    {
        // Slow particle
        ftd::ParticleEngine pe_slow;
        pe_slow.set_damping_enabled(false);
        pe_slow.add_particle(+1, {0, 0, 0});
        pe_slow.add_particle(-1, {10, 0, 0});

        pe_slow.toggles.coulomb = true;
        pe_slow.toggles.gravity = false;
        pe_slow.toggles.relativistic = true;

        ftd::Vec3 f_slow = pe_slow.compute_force(0);

        // Fast particle (v = 0.5c)
        ftd::ParticleEngine pe_fast;
        pe_fast.set_damping_enabled(false);
        pe_fast.add_particle(+1, {0, 0, 0}, {0.5 * ftd::C_SPEED, 0, 0});
        pe_fast.add_particle(-1, {10, 0, 0});

        pe_fast.toggles.coulomb = true;
        pe_fast.toggles.gravity = false;
        pe_fast.toggles.relativistic = true;

        ftd::Vec3 f_fast = pe_fast.compute_force(0);

        // Fast particle should have smaller net force due to gamma correction
        check("RE1: fast particle has reduced net force", f_fast.mag() < f_slow.mag());
    }

    // ---- RE2: v=0 → no relativistic correction ----
    std::cout << "\n--- RE2: Stationary → no correction ---\n";
    {
        ftd::ParticleEngine pe_on;
        pe_on.set_damping_enabled(false);
        pe_on.add_particle(+1, {0, 0, 0});  // v=0
        pe_on.add_particle(-1, {10, 0, 0});

        pe_on.toggles.coulomb = true;
        pe_on.toggles.gravity = false;
        pe_on.toggles.relativistic = true;

        ftd::Vec3 f_on = pe_on.compute_force(0);

        ftd::ParticleEngine pe_off;
        pe_off.set_damping_enabled(false);
        pe_off.add_particle(+1, {0, 0, 0});
        pe_off.add_particle(-1, {10, 0, 0});

        pe_off.toggles.coulomb = true;
        pe_off.toggles.gravity = false;
        pe_off.toggles.relativistic = false;

        ftd::Vec3 f_off = pe_off.compute_force(0);

        double diff = (f_on - f_off).mag();
        check("RE2: no correction at v=0", diff < 1e-20);
    }

    // ---- RE3: Toggle OFF → same as non-relativistic ----
    std::cout << "\n--- RE3: Toggle OFF → no correction ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {0.3 * ftd::C_SPEED, 0, 0});
        pe.add_particle(-1, {10, 0, 0});

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;

        pe.toggles.relativistic = false;
        ftd::Vec3 f_off = pe.compute_force(0);

        pe.toggles.relativistic = true;
        ftd::Vec3 f_on = pe.compute_force(0);

        check("RE3: relativistic ON changes force at v>0", (f_on - f_off).mag() > 1e-15);
    }

    // ---- RE4: Gamma factor matches expected ----
    std::cout << "\n--- RE4: Gamma factor check ---\n";
    {
        double v = 0.5 * ftd::C_SPEED;
        double beta2 = (v * v) / (ftd::C_SPEED * ftd::C_SPEED);
        double gamma = 1.0 / std::sqrt(1.0 - beta2);

        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {v, 0, 0});
        pe.add_particle(-1, {10, 0, 0});

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;

        pe.toggles.relativistic = false;
        double f_nr = pe.compute_force(0).mag();

        pe.toggles.relativistic = true;
        double f_r = pe.compute_force(0).mag();

        // f_r should equal f_nr / gamma
        double expected_ratio = 1.0 / gamma;
        double actual_ratio = f_r / f_nr;
        double err = std::abs(actual_ratio - expected_ratio) / expected_ratio;
        std::cout << "  gamma=" << gamma << " expected_ratio=" << expected_ratio
                  << " actual=" << actual_ratio << " err=" << err << "\n";
        check("RE4: force ratio matches 1/gamma within 1%", err < 0.01);
    }

    // ---- RE5: Speed limit still enforced ----
    std::cout << "\n--- RE5: Speed limit enforced ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.1);
        // Fast particle near speed limit
        pe.add_particle(+1, {0, 0, 0}, {0.9 * ftd::C_SPEED, 0, 0});
        pe.add_particle(-1, {10, 0, 0});

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.relativistic = true;

        pe.run(100);

        double v_final = pe.particles()[0].velocity.mag();
        check("RE5: speed <= C_SPEED", v_final <= ftd::C_SPEED * 1.001);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All relativistic correction tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
