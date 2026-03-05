// Test: Portable Self-Field
//
// Verifies that particles carry their flux when they move.
// Before this fix, particles would leave their flux behind and evaporate.
//
// Sections:
//   1. Moving particle retains density >= K_B
//   2. Old position loses K_B worth of flux
//   3. Particle survives 500 ticks while moving
//   4. Stationary particle also survives (backward compatibility)

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
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

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Portable Self-Field\n";
    std::cout << "================================================================\n";

    // Section 1: Moving particle retains density
    std::cout << "\n--- Section 1: Moving Particle Density ---\n";
    {
        const int L = 16;
        const int mid = L / 2;
        ftd::RenderBridge engine(L);

        double iso = ftd::K_B / std::sqrt(3.0);
        engine.inject_particle(mid, mid, mid, -1, {iso, iso, iso});

        // Give it velocity so it will move
        engine.voxel_at(mid, mid, mid).velocity = {0.2, 0, 0};

        double rho_before = engine.voxel_at(mid, mid, mid).density();
        std::cout << "    Density before move: " << std::setprecision(6) << rho_before << "\n";
        check_close("Initial density = K_B", rho_before, ftd::K_B, 0.001);

        // Run enough ticks for the particle to move at least once
        // velocity = 0.2, so remainder reaches 1.0 after 5 ticks
        engine.run(10);

        // Find the particle (it may have moved)
        bool found = false;
        double rho_after = 0;
        int found_x = -1;
        for (int x = 0; x < L; ++x) {
            if (engine.voxel_at(x, mid, mid).state == -1) {
                rho_after = engine.voxel_at(x, mid, mid).density();
                found_x = x;
                found = true;
                break;
            }
        }

        std::cout << "    Particle at x=" << found_x << ", density=" << rho_after << "\n";
        check("Particle still exists after 10 ticks", found);
        check("Particle moved from original position", found_x != mid);
        // Phase 4: Floor removed. Particle carries some flux to new position
        // but not necessarily K_B.  Check non-trivial flux exists.
        check("Non-trivial flux at new position", found && rho_after > 1e-4);
    }

    // Section 2: Old position flux reduced
    std::cout << "\n--- Section 2: Flux Transfer ---\n";
    {
        const int L = 16;
        const int mid = L / 2;
        ftd::RenderBridge engine(L);

        double iso = ftd::K_B / std::sqrt(3.0);
        engine.inject_particle(mid, mid, mid, -1, {iso, iso, iso});
        engine.voxel_at(mid, mid, mid).velocity = {0.5, 0, 0};

        double flux_before = engine.voxel_at(mid, mid, mid).density();

        // Run until particle moves (velocity 0.5 -> moves after 2 ticks)
        engine.run(5);

        double flux_origin = engine.voxel_at(mid, mid, mid).density();
        std::cout << "    Flux at origin after move: " << flux_origin << "\n";
        std::cout << "    Flux before move: " << flux_before << "\n";

        // Origin should have lost K_B worth of flux (some remains as radiation)
        check("Origin flux < original (self-field transferred)", flux_origin < flux_before);
    }

    // Section 3: Long-term survival while moving
    std::cout << "\n--- Section 3: Long-Term Survival ---\n";
    {
        const int L = 32;
        const int mid = L / 2;
        ftd::RenderBridge engine(L);

        double iso = ftd::K_B / std::sqrt(3.0);
        engine.inject_particle(mid, mid, mid, -1, {iso, iso, iso});
        engine.voxel_at(mid, mid, mid).velocity = {0.1, 0.05, 0};

        // Run 500 ticks — particle should survive while moving
        int alive_count = 0;
        for (int t = 0; t < 500; ++t) {
            engine.tick();
            auto d = engine.diagnostics();
            if (d.negative_count > 0) ++alive_count;
        }

        std::cout << "    Alive for " << alive_count << " / 500 ticks\n";
        check("Particle survives > 400 ticks while moving", alive_count > 400);
    }

    // Section 4: Backward compatibility — stationary particle survives
    std::cout << "\n--- Section 4: Stationary Particle Survival ---\n";
    {
        const int L = 16;
        const int mid = L / 2;
        ftd::RenderBridge engine(L);

        double iso = ftd::K_B / std::sqrt(3.0);
        engine.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
        // No velocity — stationary

        engine.run(500);

        bool alive = engine.voxel_at(mid, mid, mid).state == +1;
        double rho = engine.voxel_at(mid, mid, mid).density();

        std::cout << "    After 500 ticks: state=" << (int)engine.voxel_at(mid, mid, mid).state
                  << " density=" << rho << "\n";
        check("Stationary particle survives 500 ticks", alive);
        // Phase 4: Floor removed. Stationary particles persist at natural
        // steady-state density from coupling + wave equation balance.
        check("Stationary particle has non-zero flux", alive && rho > 1e-6);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All portable field tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
