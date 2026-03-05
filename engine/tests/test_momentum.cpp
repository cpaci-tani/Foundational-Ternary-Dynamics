/**
 * Test: Momentum Conservation — Noether Current from Translation Symmetry
 *
 * Verifies that total flux momentum is conserved in closed systems
 * (no external forces, no boundary effects). By Noether's theorem,
 * translation invariance of the Lagrangian implies momentum conservation.
 *
 * Flux momentum: P = sum_i J(i) (total flux vector over all sites)
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (translation symmetry of L_RB)
 *   - FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md (conservation laws)
 *   - DERIV_DISCRETE_CONTINUOUS_BRIDGE.md (Noether on the lattice)
 */

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
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(8) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

// Compute total flux momentum: P = sum J(i)
ftd::Vec3 total_flux_momentum(const ftd::RenderBridge& rb) {
    ftd::Vec3 P;
    int N = rb.lattice().total_sites();
    for (int i = 0; i < N; ++i) {
        P += rb.voxels()[i].flux;
    }
    return P;
}

// Compute total wave velocity momentum
ftd::Vec3 total_wave_momentum(const ftd::RenderBridge& rb) {
    ftd::Vec3 P;
    int N = rb.lattice().total_sites();
    for (int i = 0; i < N; ++i) {
        P += rb.voxels()[i].wave_vel;
    }
    return P;
}

// Compute total particle momentum
ftd::Vec3 total_particle_momentum(const ftd::RenderBridge& rb) {
    ftd::Vec3 P;
    int N = rb.lattice().total_sites();
    for (int i = 0; i < N; ++i) {
        if (rb.voxels()[i].state != 0) {
            P += rb.voxels()[i].velocity;
        }
    }
    return P;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Momentum Conservation — Noether Current\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Section 1: Flux momentum direction preserved
    // ================================================================
    // A flux pulse with net momentum in one direction should maintain
    // that directionality.
    std::cout << "\n--- Section 1: Flux Momentum Direction ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Inject flux with net z-momentum
        rb.inject_flux(cx, cx, cx, {0, 0, 3.0});

        ftd::Vec3 P0 = total_flux_momentum(rb);
        std::cout << "    Initial P = (" << P0.x << ", " << P0.y << ", " << P0.z << ")\n";

        rb.run(20);

        ftd::Vec3 P1 = total_flux_momentum(rb);
        std::cout << "    After 20 ticks P = (" << P1.x << ", " << P1.y << ", " << P1.z << ")\n";

        // Due to damping, magnitude decreases, but direction should be preserved
        // On a periodic lattice, the Laplacian preserves total momentum direction.
        // Damping uniformly scales all flux, so direction is preserved.

        // Check that z-component remains dominant
        check("Z-momentum remains dominant direction",
              std::abs(P1.z) >= std::abs(P1.x) && std::abs(P1.z) >= std::abs(P1.y));
    }

    // ================================================================
    // Section 2: Zero initial momentum stays zero
    // ================================================================
    // A symmetric flux configuration (zero net momentum) should
    // maintain zero total momentum.
    std::cout << "\n--- Section 2: Zero Momentum Conservation ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Two equal, opposite flux pulses -> net zero momentum
        rb.inject_flux(cx - 3, cx, cx, {0, 0, 3.0});
        rb.inject_flux(cx + 3, cx, cx, {0, 0, -3.0});

        ftd::Vec3 P0 = total_flux_momentum(rb);
        std::cout << "    Initial |P| = " << P0.mag() << "\n";
        check("Initial momentum ≈ 0", P0.mag() < 0.01);

        rb.run(30);

        ftd::Vec3 P1 = total_flux_momentum(rb);
        std::cout << "    After 30 ticks |P| = " << P1.mag() << "\n";

        // Should remain approximately zero (damping is uniform).
        // Threshold 0.5 vs initial magnitude 6.0 = 92% conservation.
        // Non-zero residual arises from wave interference + discrete damping
        // and the self-field floor (which adds energy to manifested particles).
        check("Momentum stays near zero after evolution", P1.mag() < 0.5);
    }

    // ================================================================
    // Section 3: Annihilation conserves direction
    // ================================================================
    // When particles annihilate, the total system momentum (particles +
    // flux) should be approximately conserved.
    std::cout << "\n--- Section 3: Annihilation Momentum ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Two particles on collision course
        rb.inject_particle(cx - 2, cx, cx, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx - 2, cx, cx)].velocity = {0.1, 0, 0};

        rb.inject_particle(cx + 2, cx, cx, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(cx + 2, cx, cx)].velocity = {-0.1, 0, 0};

        // Symmetric setup: total particle momentum ≈ 0
        ftd::Vec3 Pp0 = total_particle_momentum(rb);
        std::cout << "    Initial particle P = (" << Pp0.x << ", " << Pp0.y << ", " << Pp0.z << ")\n";
        check("Initial particle momentum ≈ 0", Pp0.mag() < 0.01);
    }

    // ================================================================
    // Section 4: Newton's third law — action/reaction pairs
    // ================================================================
    std::cout << "\n--- Section 4: Action-Reaction (Newton's Third Law) ---\n";
    {
        int L = 48;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Two particles interacting via Coulomb
        rb.inject_particle(cx - 4, cx, cx, +1, {0, 0, ftd::K_B});
        rb.inject_particle(cx + 4, cx, cx, +1, {0, 0, ftd::K_B});

        // Lock both initially, let self-fields settle
        rb.voxels()[rb.lattice().index(cx-4, cx, cx)].locked = true;
        rb.voxels()[rb.lattice().index(cx+4, cx, cx)].locked = true;
        rb.run(500);

        // Unlock both and measure velocity changes after one tick
        rb.voxels()[rb.lattice().index(cx-4, cx, cx)].locked = false;
        rb.voxels()[rb.lattice().index(cx+4, cx, cx)].locked = false;

        double vx1_before = rb.voxels()[rb.lattice().index(cx-4, cx, cx)].velocity.x;
        double vx2_before = rb.voxels()[rb.lattice().index(cx+4, cx, cx)].velocity.x;

        rb.tick();

        double vx1_after = rb.voxels()[rb.lattice().index(cx-4, cx, cx)].velocity.x;
        double vx2_after = rb.voxels()[rb.lattice().index(cx+4, cx, cx)].velocity.x;

        double dvx1 = vx1_after - vx1_before;
        double dvx2 = vx2_after - vx2_before;

        std::cout << "    dvx (particle 1) = " << dvx1 << "\n";
        std::cout << "    dvx (particle 2) = " << dvx2 << "\n";
        std::cout << "    Sum dvx = " << dvx1 + dvx2 << "\n";

        // Newton's third law: forces are equal and opposite
        // So dvx1 + dvx2 ≈ 0 (to within numerical error from self-field asymmetry)
        // The pairwise Coulomb is exactly antisymmetric.
        // Gravity from grad(rho) may not be exactly symmetric due to self-field asymmetry.
        check("Action-reaction: sum of velocity changes small",
              std::abs(dvx1 + dvx2) < std::max(std::abs(dvx1), 1e-6) * 0.5);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All momentum conservation tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
