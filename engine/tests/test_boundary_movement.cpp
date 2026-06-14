/**
 * test_boundary_movement.cpp
 *
 * Verifies phase_movement face handling:
 *   reflective_boundary OFF → particle exhausts into the void (no toroidal wrap)
 *   reflective_boundary ON  → mirror bounce at the face
 */

#include <cmath>
#include <iostream>
#include "ftd/render_bridge.h"

int failures = 0;

static void check(const char* name, bool cond) {
    std::cout << (cond ? "  PASS  " : "  FAIL  ") << name << "\n";
    if (!cond) ++failures;
}

static void disable_extras(ftd::RenderBridge& rb) {
    rb.toggles.wave_propagation = false;
    rb.toggles.coupling = false;
    rb.toggles.damping = false;
    rb.toggles.genesis = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.forces = false;
    rb.toggles.gravity = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.selective_damping = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.movement = true;
    rb.set_dt(1.0);
}

int main() {
    std::cout << "=== test_boundary_movement ===\n";

    const int L = 17;
    const int far_x = L - 1;

    // --- Exhaustive boundary (default): no wrap to opposite face ---
    {
        ftd::RenderBridge rb(L);
        disable_extras(rb);
        rb.toggles.reflective_boundary = false;
        rb.force_cpu();

        rb.inject_particle(0, 8, 8, +1, ftd::Vec3{0.0, 0.0, 0.0});
        rb.voxel_at(0, 8, 8).velocity = ftd::Vec3{-1.0, 0.0, 0.0};
        rb.tick();

        check("exhaustive: source voxel void after exit attempt", rb.voxel_at(0, 8, 8).state == 0);
        check("exhaustive: opposite face stays void", rb.voxel_at(far_x, 8, 8).state == 0);
    }

    // --- Reflective boundary: mirror bounce, still no wrap ---
    {
        ftd::RenderBridge rb(L);
        disable_extras(rb);
        rb.toggles.reflective_boundary = true;
        rb.force_cpu();

        rb.inject_particle(0, 8, 8, +1, ftd::Vec3{0.0, 0.0, 0.0});
        rb.voxel_at(0, 8, 8).velocity = ftd::Vec3{-1.0, 0.0, 0.0};
        rb.tick();

        check("reflective: particle remains at source voxel", rb.voxel_at(0, 8, 8).state == 1);
        check("reflective: velocity x flipped", rb.voxel_at(0, 8, 8).velocity.x > 0.0);
        check("reflective: opposite face stays void", rb.voxel_at(far_x, 8, 8).state == 0);
    }

    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
