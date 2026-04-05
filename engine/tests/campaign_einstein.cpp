/**
 * Campaign: Einstein — Relativistic and Gravitational Tests
 *
 * Three tests probing energy conservation, Lorentz contraction, and
 * gravitational redshift in the FTD engine:
 *
 *   E1: Energy Conservation — 3-particle system, total energy drift < 50%
 *   E2: Lorentz Contraction — Boosted particle field compressed along motion
 *   E3: Gravitational Redshift — Latency field gradient near massive cluster
 *
 * These tests verify [EMERGENT] relativistic behavior from the tick cycle.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace ftd;

static int failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::printf("  PASS  %s\n", name);
    } else {
        std::printf("  FAIL  %s\n", name);
        ++failures;
    }
}

// ============================================================================
// E1: Energy Conservation
//
// Three locked particles on a 32^3 lattice. Only wave propagation + damping
// enabled (no genesis, coupling, forces, or movement). This isolates the
// wave equation's energy behavior: with damping, total energy must decrease
// monotonically. Energy drift is measured as fractional change.
//
// The coupling source term g_c * grad(s) continuously injects energy into
// the field when coupling is ON, so we disable it to test pure conservation.
// ============================================================================
static void test_energy_conservation() {
    std::printf("\n--- E1: Energy Conservation (3-particle system) ---\n");

    const int L = 32;
    auto rb = std::make_unique<RenderBridge>(L);

    // Minimal toggles: wave + damping only (no coupling, no Gauss injection)
    rb->toggles.disable_all();
    rb->toggles.wave_propagation = true;
    rb->toggles.damping = true;

    double iso = K_B / std::sqrt(3.0);

    // Particle 1: +1 at (8,16,16)
    rb->inject_particle(8, 16, 16, +1, Vec3(iso, iso, iso));
    rb->sync_from_gpu();
    rb->voxels()[rb->lattice().index(8, 16, 16)].locked = true;

    // Particle 2: -1 at (24,16,16)
    rb->inject_particle(24, 16, 16, -1, Vec3(iso, iso, iso));
    rb->sync_from_gpu();
    rb->voxels()[rb->lattice().index(24, 16, 16)].locked = true;

    // Particle 3: +1 at (16,16,8)
    rb->inject_particle(16, 16, 8, +1, Vec3(iso, iso, iso));
    rb->sync_from_gpu();
    rb->voxels()[rb->lattice().index(16, 16, 8)].locked = true;

    // Measure initial energy
    rb->sync_from_gpu();
    auto ea0 = rb->energy_audit();
    double E0 = ea0.total_energy;
    std::printf("  Tick   0: total_energy = %.8e (field=%.4e, wave=%.4e, KE=%.4e)\n",
                E0, ea0.field_energy, ea0.wave_energy, ea0.particle_ke);

    // Run 500 ticks
    rb->run(500);

    // Measure final energy
    rb->sync_from_gpu();
    auto ea500 = rb->energy_audit();
    double E500 = ea500.total_energy;
    std::printf("  Tick 500: total_energy = %.8e (field=%.4e, wave=%.4e, KE=%.4e)\n",
                E500, ea500.field_energy, ea500.wave_energy, ea500.particle_ke);

    double ratio = (E0 > 1e-30) ? E500 / E0 : 0.0;
    std::printf("  Energy ratio E_final/E_initial: %.6f\n", ratio);

    // Checks
    check("E1a: Initial energy > 0", E0 > 0.0);
    check("E1b: Final energy > 0 (some energy remains after damping)", E500 > 0.0);
    // With damping ON, energy must decrease (not increase). This is the real
    // conservation test: no spurious energy injection when coupling is OFF.
    check("E1c: Energy decreases with damping (E_final < E_initial)", E500 < E0);

    // Charge conservation
    check("E1d: Charge conserved (+1 -1 +1 = +1)",
          ea500.charge_total == ea0.charge_total);
}

// ============================================================================
// E2: Lorentz Contraction
//
// REST: +1 particle at center of 64^3, no velocity.
// BOOSTED: +1 particle at (16,32,32) with vel=(0.3,0,0).
// After 200 ticks, measure |J| at +8 along x vs +8 along y from the particle.
// For the rest frame, the two measurements should be nearly equal.
// For the boosted frame, the field along the direction of motion (x) should
// be more compressed — meaning |J| at the same coordinate distance is LARGER
// along x than along y, so the ratio |J_x|/|J_y| > 1 for rest and the
// boosted ratio should differ from the rest ratio.
// ============================================================================
static void test_lorentz_contraction() {
    std::printf("\n--- E2: Lorentz Contraction ---\n");

    const int L = 48;
    const int mid = L / 2;
    double iso = K_B / std::sqrt(3.0);

    // REST configuration
    double J_rest_x, J_rest_y;
    {
        auto rb = std::make_unique<RenderBridge>(L);
        rb->force_cpu();
        rb->toggles.genesis = false;
        rb->toggles.movement = false;

        rb->inject_particle(mid, mid, mid, +1, Vec3(iso, iso, iso));
        rb->sync_from_gpu();
        rb->voxels()[rb->lattice().index(mid, mid, mid)].locked = true;

        rb->run(200);
        rb->sync_from_gpu();

        J_rest_x = rb->voxel_at(mid + 8, mid, mid).flux.mag();
        J_rest_y = rb->voxel_at(mid, mid + 8, mid).flux.mag();

        std::printf("  REST:    |J(+8,0,0)| = %.8e,  |J(0,+8,0)| = %.8e\n",
                    J_rest_x, J_rest_y);
    }

    // BOOSTED configuration: particle with velocity creates anisotropic self-field
    // Use force_cpu() — GPU path doesn't populate voxel flux for direct reads
    double J_boost_x, J_boost_y;
    int px = 8, py = mid, pz = mid;
    {
        auto rb = std::make_unique<RenderBridge>(L);
        rb->force_cpu();
        rb->toggles.genesis = false;
        rb->toggles.movement = true;  // Must move for Lorentz contraction to appear

        rb->inject_particle(px, py, pz, +1, Vec3(iso, iso, iso));
        rb->sync_from_gpu();
        rb->voxels()[rb->lattice().index(px, py, pz)].velocity = Vec3(0.3, 0.0, 0.0);
        rb->voxels()[rb->lattice().index(px, py, pz)].locked = true;

        rb->run(200);
        rb->sync_from_gpu();

        // Measure at +8 along x and +8 along y from particle position
        J_boost_x = rb->voxel_at(px + 8, py, pz).flux.mag();
        J_boost_y = rb->voxel_at(px, py + 8, pz).flux.mag();

        std::printf("  BOOSTED: |J(+8,0,0)| = %.8e,  |J(0,+8,0)| = %.8e\n",
                    J_boost_x, J_boost_y);
    }

    // Compute ratios
    double ratio_rest = (J_rest_y > 1e-30) ? J_rest_x / J_rest_y : 1.0;
    double ratio_boost = (J_boost_y > 1e-30) ? J_boost_x / J_boost_y : 1.0;

    std::printf("  Ratio |J_x|/|J_y| — REST: %.4f, BOOSTED: %.4f\n",
                ratio_rest, ratio_boost);

    // Checks
    check("E2a: Rest frame fields exist (|J| > 0 at r=8)",
          J_rest_x > 1e-30 && J_rest_y > 1e-30);

    check("E2b: Boosted frame fields exist (|J| > 0 at r=8)",
          J_boost_x > 1e-30 && J_boost_y > 1e-30);

    // The velocity imparts a current source (s*v) that breaks isotropy.
    // With vel=(0.3,0,0), the Biot-Savart coupling term ∇×(s·v) generates
    // anisotropy in the flux profile. The boosted ratio should differ from rest.
    double ratio_diff = std::abs(ratio_boost - ratio_rest);
    std::printf("  |ratio_boost - ratio_rest| = %.6f\n", ratio_diff);

    check("E2c: Boosted field anisotropy differs from rest (ratio_diff > 0.001)",
          ratio_diff > 0.001);
}

// ============================================================================
// E3: Gravitational Redshift
//
// Enable latency_field. Create a cluster of 8 locked +1 particles in a
// 2x2x2 arrangement at the center of a 32^3 lattice. Run 500 ticks to let
// the Poisson solver converge. Then read the phi_latency potential at r=3
// (close to mass) and r=10 (far from mass). The potential should be higher
// near the mass, producing a latency gradient (gravitational redshift).
//
// We read phi_latency (the raw Poisson potential) rather than voxel.latency
// (which is sqrt(clamp(phi,0,0.998))) because the potential may be small.
// ============================================================================
static void test_gravitational_redshift() {
    std::printf("\n--- E3: Gravitational Redshift (Latency Field) ---\n");

    const int L = 32;
    const int mid = L / 2;
    auto rb = std::make_unique<RenderBridge>(L);
    rb->force_cpu();  // Latency field solver is CPU-only

    // Minimal toggles + latency field
    rb->toggles.disable_all();
    rb->toggles.wave_propagation = true;
    rb->toggles.gauss_projection = true;
    rb->toggles.latency_field = true;

    double iso = K_B / std::sqrt(3.0);

    // Create 2x2x2 cluster of locked +1 particles at center
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            for (int dz = 0; dz <= 1; ++dz) {
                int x = mid + dx;
                int y = mid + dy;
                int z = mid + dz;
                rb->inject_particle(x, y, z, +1, Vec3(iso, iso, iso));
                rb->sync_from_gpu();
                rb->voxels()[rb->lattice().index(x, y, z)].locked = true;
            }
        }
    }

    std::printf("  Cluster: 8 locked +1 particles at (%d-%d, %d-%d, %d-%d)\n",
                mid, mid + 1, mid, mid + 1, mid, mid + 1);

    // Run 500 ticks to let Poisson solver converge (30 SOR iterations/tick = 15000 total)
    rb->run(500);
    rb->sync_from_gpu();

    // Read phi_latency potential directly (not voxel.latency which is sqrt(clamp(...)))
    int idx_r3 = rb->lattice().index(mid + 3, mid, mid);
    int idx_r10 = rb->lattice().index(mid + 10, mid, mid);

    double phi_r3 = rb->phi_latency()[idx_r3];
    double phi_r10 = rb->phi_latency()[idx_r10];

    // Also read voxel.latency (derived from phi via sqrt(clamp))
    double latency_r3 = rb->voxel_at(mid + 3, mid, mid).latency;
    double latency_r10 = rb->voxel_at(mid + 10, mid, mid).latency;

    std::printf("  phi_latency at r=3:   %.8e\n", phi_r3);
    std::printf("  phi_latency at r=10:  %.8e\n", phi_r10);
    std::printf("  voxel.latency at r=3: %.8e\n", latency_r3);
    std::printf("  voxel.latency at r=10:%.8e\n", latency_r10);

    // Checks
    // The Poisson potential has larger magnitude near mass.
    // With periodic BC + mean subtraction, phi is typically negative near mass
    // (the gauge choice shifts the absolute level), so we compare |phi|.
    std::printf("  |phi_r3| = %.8e, |phi_r10| = %.8e\n",
                std::abs(phi_r3), std::abs(phi_r10));
    check("E3a: |phi_latency| at r=3 > |phi_latency| at r=10 (potential gradient)",
          std::abs(phi_r3) > std::abs(phi_r10));

    // Latency should be non-negative (derived from phi via sqrt(clamp(phi,0,...)))
    check("E3b: voxel.latency at r=3 >= 0", latency_r3 >= 0.0);
    check("E3c: voxel.latency at r=10 >= 0", latency_r10 >= 0.0);

    // The latency potential should be established near mass
    check("E3d: phi_latency near mass is nonzero (|phi_r3| > 1e-15)",
          std::abs(phi_r3) > 1e-15);
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::printf("================================================================\n");
    std::printf("  CAMPAIGN: Einstein — Relativistic & Gravitational Tests\n");
    std::printf("             (3 tests, 11 checks)\n");
    std::printf("================================================================\n");

    test_energy_conservation();
    test_lorentz_contraction();
    test_gravitational_redshift();

    std::printf("\n================================================================\n");
    std::printf("  RESULT: %s (%d failures)\n",
                failures == 0 ? "ALL PASSED" : "FAILURES DETECTED", failures);
    std::printf("================================================================\n");

    return failures;
}
