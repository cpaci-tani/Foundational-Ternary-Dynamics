/**
 * Test: Symmetric Movement and Coordinate-Independent Chirality Density
 *
 * Verifies that:
 * 1. Enabling symmetric_movement_order shuffles loop traversal and axis resolution.
 * 2. Voxel::chirality_density() is coordinate-independent when moving and projects
 *    perpendicular to velocity, while falling back to legacy z-axis projection when stationary.
 */

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>

namespace ftd { namespace test {

void test_chirality_coordinate_independence() {
    section("chirality coordinate independence verification");

    Voxel v;
    v.flux_L = Vec3{1.0, 2.0, 3.0};
    v.flux_R = Vec3{0.5, 1.0, 1.5};

    // 1. Stationary voxel -> projects on z-axis (uses x, y)
    v.velocity = Vec3{0.0, 0.0, 0.0};
    double chi_stat = v.chirality_density();
    double expected_stat = (1.0*1.0 + 2.0*2.0) - (0.5*0.5 + 1.0*1.0);
    check_close("stationary chirality matches legacy z-axis", chi_stat, expected_stat, 1e-9);

    // 2. Moving along z-axis -> projects on z-axis (uses x, y)
    v.velocity = Vec3{0.0, 0.0, 2.0};
    double chi_z = v.chirality_density();
    check_close("moving along z chirality matches legacy", chi_z, chi_stat, 1e-9);

    // 3. Moving along x-axis -> projects on x-axis (uses y, z)
    v.velocity = Vec3{1.5, 0.0, 0.0};
    double chi_x = v.chirality_density();
    double expected_x = (2.0*2.0 + 3.0*3.0) - (1.0*1.0 + 1.5*1.5);
    check_close("moving along x projects on y-z plane", chi_x, expected_x, 1e-9);

    // 4. Moving along y-axis -> projects on y-axis (uses x, z)
    v.velocity = Vec3{0.0, 1.5, 0.0};
    double chi_y = v.chirality_density();
    double expected_y = (1.0*1.0 + 3.0*3.0) - (0.5*0.5 + 1.5*1.5);
    check_close("moving along y projects on x-z plane", chi_y, expected_y, 1e-9);
}

void test_symmetric_movement_order() {
    section("symmetric_movement_order toggle verification");

    // Initialize two identical setups with colliding/blocking chains of particles
    // to expose update-order dependency.
    RenderBridge rb_asym(16);
    RenderBridge rb_sym(16);

    rb_asym.force_cpu();
    rb_sym.force_cpu();

    rb_asym.seed_rng(42);
    rb_sym.seed_rng(42);

    auto set_toggles = [](RenderBridge& rb, bool sym_enabled) {
        rb.toggles.disable_all();
        rb.toggles.movement = true;
        rb.toggles.symmetric_movement_order = sym_enabled;
    };

    set_toggles(rb_asym, false);
    set_toggles(rb_sym, true);

    // Inject three adjacent positive particles in a row.
    rb_asym.inject_particle(5, 5, 5, +1, Vec3{0.0, 0.0, 0.0});
    rb_sym.inject_particle(5, 5, 5, +1, Vec3{0.0, 0.0, 0.0});

    rb_asym.inject_particle(6, 5, 5, +1, Vec3{0.0, 0.0, 0.0});
    rb_sym.inject_particle(6, 5, 5, +1, Vec3{0.0, 0.0, 0.0});

    rb_asym.inject_particle(7, 5, 5, +1, Vec3{0.0, 0.0, 0.0});
    rb_sym.inject_particle(7, 5, 5, +1, Vec3{0.0, 0.0, 0.0});

    // Manually set their velocities to 5.0 on the x axis so that they jump multiple times.
    int idx1 = rb_asym.lattice().index(5, 5, 5);
    int idx2 = rb_asym.lattice().index(6, 5, 5);
    int idx3 = rb_asym.lattice().index(7, 5, 5);

    rb_asym.voxels()[idx1].velocity = Vec3{5.0, 0.0, 0.0};
    rb_sym.voxels()[idx1].velocity = Vec3{5.0, 0.0, 0.0};

    rb_asym.voxels()[idx2].velocity = Vec3{5.0, 0.0, 0.0};
    rb_sym.voxels()[idx2].velocity = Vec3{5.0, 0.0, 0.0};

    rb_asym.voxels()[idx3].velocity = Vec3{5.0, 0.0, 0.0};
    rb_sym.voxels()[idx3].velocity = Vec3{5.0, 0.0, 0.0};

    rb_asym.seed_rng(42);
    rb_sym.seed_rng(42);

    // Run for 10 ticks (enough for multiple jumps to occur)
    int N = rb_asym.lattice().total_sites();
    for (int t = 0; t < 10; ++t) {
        rb_asym.tick();
        rb_sym.tick();
    }

    auto audit_asym = rb_asym.energy_audit();
    auto audit_sym = rb_sym.energy_audit();

    // Verify charge is conserved (still 3 particles)
    check("Charge conservation (asymmetric)", audit_asym.charge_total == 3);
    check("Charge conservation (symmetric)", audit_sym.charge_total == 3);

    // Verify that the final positions/states are DIFFERENT due to symmetric movement order shuffling
    bool identical = true;
    for (int idx = 0; idx < N; ++idx) {
        if (rb_asym.voxels()[idx].state != rb_sym.voxels()[idx].state) {
            identical = false;
            break;
        }
    }

    check("Symmetric movement order diverges from asymmetric movement order due to shuffling", !identical);
}

}} // namespace ftd::test

int main() {
    ftd::test::init("test_symmetric_movement");
    ftd::test::test_chirality_coordinate_independence();
    ftd::test::test_symmetric_movement_order();
    return ftd::test::finalize();
}
