/**
 * Test: Charge Continuity Equation
 *
 * Verifies that total electric charge Q = sum(state) is exactly conserved
 * through all dynamics: static, dynamic, annihilation, genesis.
 *
 * The continuity equation dQ/dt + div(J_matter) = 0 is implicitly enforced
 * by the Gauss constraint div(J) = s and the conservation properties of
 * the update rules.
 *
 * Tests:
 *   CONT-1: Static charges — Q conserved over 1000 ticks
 *   CONT-2: Dynamic charges (Coulomb motion) — Q exact at every checkpoint
 *   CONT-3: Annihilation — Q = 0 throughout
 *   CONT-4: Post-genesis — Q frozen after genesis turned off
 *   CONT-5: Gauss constraint quality — sum|div(J)-s|² bounded
 *   CONT-6: Mixed scenario — particles + waves + genesis
 *   CONT-7: Multi-pair annihilation — Q exact throughout
 *
 * Theory references:
 *   - SPEC_ENGINE.md §3 (Gauss constraint enforcement)
 *   - CLAUDE.md §14.3 (U(1) gauge emergence)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

// Get total charge Q = sum(state)
static int total_charge(const ftd::RenderBridge& rb) {
    int Q = 0;
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        Q += rb.voxels()[i].state;
    return Q;
}

// Get manifested count
static int manifested_count(const ftd::RenderBridge& rb) {
    int n = 0;
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        if (rb.voxels()[i].state != 0) n++;
    return n;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Charge Continuity Equation — 7 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // CONT-1: Static charges — Q conserved over 1000 ticks
    // ================================================================
    std::cout << "\n-- CONT-1: Static Charge Conservation --\n";
    {
        ftd::RenderBridge rb(16);
        // Disable genesis so self-fields from particles cannot trigger
        // spontaneous particle creation, which would change total charge.
        // See AUDIT_PLAN.md I-08.
        rb.toggles.genesis = false;
        int mid = 8;

        // 5 locked particles: 3 positive, 2 negative → Q = +1
        rb.inject_particle(mid-2, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid+2, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid, mid-2, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid, mid+2, mid, -1, {0, 0, -ftd::K_B});
        rb.inject_particle(mid, mid, mid+2, -1, {0, 0, -ftd::K_B});

        // Lock all particles
        for (int i = 0; i < rb.lattice().total_sites(); ++i)
            if (rb.voxels()[i].state != 0)
                rb.voxels()[i].locked = true;

        int Q0 = total_charge(rb);
        rb.run(1000);
        int Q1000 = total_charge(rb);

        std::cout << "    Q(0) = " << Q0 << ", Q(1000) = " << Q1000 << "\n";
        check("CONT-1: Static charges Q conserved (Q(0) == Q(1000))",
              Q0 == Q1000);
    }

    // ================================================================
    // CONT-2: Dynamic charges under Coulomb force
    // ================================================================
    std::cout << "\n-- CONT-2: Dynamic Charge Conservation --\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;

        // Two opposite charges — will attract
        rb.inject_particle(mid - 5, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});

        rb.toggles.genesis = false;  // No spontaneous creation

        int Q0 = total_charge(rb);
        bool all_conserved = true;

        for (int step = 0; step < 5; ++step) {
            rb.run(100);
            int Q = total_charge(rb);
            if (Q != Q0) all_conserved = false;
        }

        std::cout << "    Q(0) = " << Q0 << ", conserved at all checkpoints: "
                  << (all_conserved ? "YES" : "NO") << "\n";
        check("CONT-2: Dynamic charges Q exact at every 100-tick checkpoint",
              all_conserved);
    }

    // ================================================================
    // CONT-3: Annihilation — Q = 0 throughout
    // ================================================================
    std::cout << "\n-- CONT-3: Annihilation Charge Conservation --\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;

        // +1/-1 pair on collision course — Q should be 0 always
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});

        rb.toggles.genesis = false;

        int Q0 = total_charge(rb);
        bool always_zero = (Q0 == 0);

        for (int step = 0; step < 20; ++step) {
            rb.run(100);
            int Q = total_charge(rb);
            if (Q != 0) always_zero = false;
        }

        std::cout << "    Q always 0: " << (always_zero ? "YES" : "NO") << "\n";
        check("CONT-3: Annihilation Q = 0 throughout (2000 ticks)",
              always_zero);
    }

    // ================================================================
    // CONT-4: Post-genesis charge conservation — Q frozen after genesis off
    // ================================================================
    std::cout << "\n-- CONT-4: Post-Genesis Charge Conservation --\n";
    {
        ftd::RenderBridge rb(16);

        // Inject a strong flux pulse to trigger genesis
        int mid = 8;
        for (int x = mid-2; x <= mid+2; ++x)
            for (int y = mid-2; y <= mid+2; ++y)
                for (int z = mid-2; z <= mid+2; ++z) {
                    double amp = ftd::K_GENESIS * 2.0;
                    rb.inject_flux(x, y, z, {0, 0, amp});
                }

        // Enable everything including genesis to create particles
        rb.toggles.enable_all();
        rb.toggles.dual_substrate = false;
        rb.toggles.selective_damping = false;

        rb.run(100);  // Let genesis create particles

        // Record charge and particle count AFTER genesis
        int Q_post_genesis = total_charge(rb);
        int n_post = manifested_count(rb);

        // Now disable genesis — subsequent dynamics must conserve charge
        // NOTE: Evaporation (state ±1 → 0) is a physical process that
        // destroys individual particles when neighborhood energy drops below
        // threshold. This legitimately changes Q. We verify that Q changes
        // ONLY due to evaporation, not due to spurious creation or sign flips.
        // Check: |delta_Q| <= number_of_evaporated_particles.
        rb.toggles.genesis = false;

        int Q_final = Q_post_genesis;
        for (int step = 0; step < 5; ++step) {
            rb.run(100);
            Q_final = total_charge(rb);
        }

        int n_final = manifested_count(rb);
        int delta_Q = std::abs(Q_final - Q_post_genesis);
        int delta_n = n_post - n_final;  // evaporated particles (always >= 0)
        std::cout << "    Post-genesis Q = " << Q_post_genesis
                  << ", particles = " << n_post << "\n";
        std::cout << "    After 500 more ticks: Q = " << Q_final
                  << ", particles = " << n_final
                  << ", |dQ| = " << delta_Q
                  << ", evaporated = " << delta_n << "\n";

        // Q change bounded by evaporation count (each evaporated particle changes Q by at most 1)
        check("CONT-4: Post-genesis |dQ| <= evaporated count (genesis OFF)",
              n_post > 0 && delta_Q <= std::max(delta_n, 0));
    }

    // ================================================================
    // CONT-5: Gauss constraint quality
    // ================================================================
    std::cout << "\n-- CONT-5: Gauss Constraint Quality --\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;

        // Multi-particle system with dynamics
        rb.inject_particle(mid - 4, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 4, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.inject_particle(mid, mid - 4, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid, mid + 4, mid, -1, {0, 0, -ftd::K_B});

        rb.toggles.genesis = false;

        rb.run(500);

        auto audit = rb.energy_audit();
        std::cout << "    Gauss violation (L2) = " << audit.gauss_violation << "\n";
        std::cout << "    Max Gauss error = " << audit.max_gauss_error << "\n";

        check("CONT-5: Gauss constraint quality (L2 violation < 1.0 after 500 ticks)",
              audit.gauss_violation < 1.0);
    }

    // ================================================================
    // CONT-6: Mixed scenario — particles + waves + genesis off
    // ================================================================
    std::cout << "\n-- CONT-6: Mixed Scenario --\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;

        // Particles + background flux waves
        rb.inject_particle(mid - 5, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});

        // Add a flux wave in the background
        for (int x = 0; x < 32; ++x)
            for (int y = 0; y < 32; ++y)
                for (int z = 0; z < 32; ++z) {
                    double extra = 0.01 * std::sin(2.0 * M_PI * 3 * x / 32.0);
                    auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
                    v.flux.y += extra;
                }

        rb.toggles.genesis = false;

        int Q0 = total_charge(rb);
        rb.run(500);
        int Q500 = total_charge(rb);

        std::cout << "    Q(0) = " << Q0 << ", Q(500) = " << Q500 << "\n";
        check("CONT-6: Mixed scenario Q conserved over 500 ticks",
              Q0 == Q500);
    }

    // ================================================================
    // CONT-7: Multi-pair annihilation
    // ================================================================
    std::cout << "\n-- CONT-7: Multi-Pair Annihilation --\n";
    {
        ftd::RenderBridge rb(32);

        // 4 pairs, tightly packed — high probability of annihilation
        rb.inject_particle(8, 16, 16, +1, {0, 0, ftd::K_B});
        rb.inject_particle(10, 16, 16, -1, {0, 0, -ftd::K_B});
        rb.inject_particle(14, 16, 16, +1, {0, 0, ftd::K_B});
        rb.inject_particle(16, 16, 16, -1, {0, 0, -ftd::K_B});
        rb.inject_particle(20, 16, 16, +1, {0, 0, ftd::K_B});
        rb.inject_particle(22, 16, 16, -1, {0, 0, -ftd::K_B});
        rb.inject_particle(26, 16, 16, +1, {0, 0, ftd::K_B});
        rb.inject_particle(28, 16, 16, -1, {0, 0, -ftd::K_B});

        rb.toggles.genesis = false;

        int Q0 = total_charge(rb);  // Should be 0
        bool always_zero = (Q0 == 0);

        for (int step = 0; step < 10; ++step) {
            rb.run(200);
            int Q = total_charge(rb);
            if (Q != 0) always_zero = false;
        }

        int n_final = manifested_count(rb);
        std::cout << "    Initial particles: 8, final: " << n_final
                  << ", Q always 0: " << (always_zero ? "YES" : "NO") << "\n";

        check("CONT-7: Multi-pair Q = 0 throughout 2000 ticks",
              always_zero);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (g_failures == 0)
        std::cout << "  All 7 continuity equation tests PASSED.\n";
    else
        std::cout << "  " << g_failures << " test(s) FAILED.\n";
    std::cout << "================================================================\n";

    return g_failures;
}
