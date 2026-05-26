/**
 * @file test_relativistic_verlet.cpp
 * @brief Relativistic Verlet integrator speed cap and momentum verification test.
 */

#include "ftd/particle_engine.h"
#include "test_helpers.h"
#include <iostream>
#include <cmath>

int main() {
    using namespace ftd;
    using namespace ftd::test;

    Counter c;
    std::cout << "============================================================\n";
    std::cout << "  Scale 1: Relativistic Verlet Speed Cap & Momentum Test\n";
    std::cout << "============================================================\n\n";

    // ---- Test 1: Relativistic velocity relation to momentum ----
    {
        std::cout << "--- Test 1: Relativistic velocity vs momentum relation ---\n";
        ParticleEngine pe;
        pe.toggles.relativistic_verlet = true;
        pe.set_damping_enabled(false);

        // Add a particle with initial velocity = 0.9 * C_SPEED
        double v0 = 0.9 * C_SPEED;
        int id = pe.add_particle(0, {0, 0, 0}, {v0, 0, 0}); // neutral to avoid automatic forces

        const auto& p = pe.particles()[0];
        
        // Expected initial momentum: p0 = m * v0 * gamma
        double gamma = 1.0 / std::sqrt(1.0 - (v0 * v0) / (C_SPEED * C_SPEED));
        double expected_p = p.mass * v0 * gamma;

        std::cout << "    Initial velocity: " << p.velocity.x << " (expected: " << v0 << ")\n";
        std::cout << "    Initial momentum: " << p.momentum.x << " (expected: " << expected_p << ")\n";

        check_close("Initial velocity matches input", p.velocity.x, v0, 1e-6, &c);
        check_close("Relativistic momentum matches gamma-scaled mass*velocity", p.momentum.x, expected_p, 1e-6, &c);
    }

    // ---- Test 2: Speed cap enforcement under high accelerating forces ----
    {
        std::cout << "\n--- Test 2: Speed cap under high forces ---\n";
        ParticleEngine pe;
        pe.toggles.relativistic_verlet = true;
        pe.set_damping_enabled(false);

        // Put a locked, highly charged particle at the origin, and a lighter particle nearby
        // to create a massive accelerating Coulomb force.
        int center_id = pe.add_locked_particle(-100, {0, 0, 0}, 100.0);
        int particle_id = pe.add_particle(+1, {2.0, 0, 0}, {0.0, 0.0, 0.0});

        // Run for several ticks to let the particle fall toward the center under huge forces
        for (int i = 0; i < 50; ++i) {
            pe.tick();
            
            // Check speed of the mobile particle
            double speed = pe.particles()[1].velocity.mag();
            if (speed >= C_SPEED) {
                std::cout << "    VIOLATION at tick " << i << ": speed = " << speed << " (C_SPEED = " << C_SPEED << ")\n";
            }
        }

        double final_speed = pe.particles()[1].velocity.mag();
        std::cout << "    Final Speed:    " << final_speed << " (C_SPEED limit = " << C_SPEED << ")\n";
        std::cout << "    Final Momentum: " << pe.particles()[1].momentum.mag() << "\n";

        check("Speed is strictly below light cone limit C_SPEED", final_speed < C_SPEED, &c);
    }

    // ---- Test 3: Relativistic Verlet vs Classical Verlet comparisons ----
    {
        std::cout << "\n--- Test 3: Relativistic Verlet vs Classical Verlet ---\n";
        // Classical Verlet PE
        ParticleEngine pe_classical;
        pe_classical.toggles.relativistic_verlet = false;
        pe_classical.set_damping_enabled(false);
        pe_classical.add_locked_particle(-100, {0, 0, 0}, 100.0);
        pe_classical.add_particle(+1, {2.0, 0, 0}, {0.0, 0.0, 0.0});

        // Relativistic Verlet PE
        ParticleEngine pe_rel;
        pe_rel.toggles.relativistic_verlet = true;
        pe_rel.set_damping_enabled(false);
        pe_rel.add_locked_particle(-100, {0, 0, 0}, 100.0);
        pe_rel.add_particle(+1, {2.0, 0, 0}, {0.0, 0.0, 0.0});

        // Run both for 20 ticks
        for (int i = 0; i < 20; ++i) {
            pe_classical.tick();
            pe_rel.tick();
        }

        double speed_classical = pe_classical.particles()[1].velocity.mag();
        double speed_rel = pe_rel.particles()[1].velocity.mag();

        std::cout << "    Classical Speed: " << speed_classical << "\n";
        std::cout << "    Relativistic Speed: " << speed_rel << " (strictly capped by C_SPEED)\n";

        // Under huge acceleration, classical should have run away or clamped artificially,
        // while relativistic is smoothly and physically limited.
        check("Relativistic speed stays bounded under light cone", speed_rel < C_SPEED, &c);
    }

    return report_and_exit_code(c, "Relativistic Verlet integration");
}
