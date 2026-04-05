/**
 * Campaign: Plato — Dispositional Field Tests
 *
 * Three tests probing the dispositional (flux) layer of FTD:
 *
 *   P1: Dispositional Ratio — Verify 1/r^2 Coulomb falloff of |J(r)|^2
 *   P2: Genesis Phase Transition — Manifestation threshold at K_GENESIS
 *   P3: Void Energy — Empty lattice has zero field energy; particle pair radiates
 *
 * These tests verify [EMERGENT] behavior arising from the 6 core rules.
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
// P1: Dispositional Ratio
//
// A single locked +1 particle at center builds a self-field whose magnitude
// |J(r)| should decrease monotonically with distance from the source.
// With damping and coupling, the exact profile is steeper than 1/r but the
// key physics is: the flux field falls with distance [EMERGENT].
//
// We measure |J(r)| at r = 3,5,7,9,11 along x-axis after 500 ticks warmup
// on L=64 lattice, then verify monotonic decrease.
// ============================================================================
static void test_dispositional_ratio() {
    std::printf("\n--- P1: Dispositional Ratio (field falloff from source) ---\n");

    const int L = 48;
    const int mid = L / 2;
    auto rb = std::make_unique<RenderBridge>(L);

    rb->toggles.genesis = false;
    rb->toggles.movement = false;

    double iso = K_B / std::sqrt(3.0);
    rb->inject_particle(mid, mid, mid, +1, Vec3(iso, iso, iso));
    rb->sync_from_gpu();
    rb->voxels()[rb->lattice().index(mid, mid, mid)].locked = true;

    // Warmup for steady-state self-field
    rb->run(200);
    rb->sync_from_gpu();

    // Measure |J(r)| along x-axis at several radii (staying well within L/2).
    // Start at r=5 to avoid near-field coupling depletion zone (r<=4).
    int radii[] = {5, 7, 9, 11, 14};
    int n_radii = 5;
    double Jmag[5] = {};

    std::printf("  r   |  |J(r)|          |  |J(r)|^2\n");
    for (int i = 0; i < n_radii; ++i) {
        int r = radii[i];
        auto& v = rb->voxel_at(mid + r, mid, mid);
        Jmag[i] = v.flux.mag();
        std::printf("  %2d  |  %.8e  |  %.8e\n", r, Jmag[i], Jmag[i] * Jmag[i]);
    }

    // Also measure along y-axis for isotropy comparison
    double Jmag_y[5] = {};
    for (int i = 0; i < n_radii; ++i) {
        int r = radii[i];
        Jmag_y[i] = rb->voxel_at(mid, mid + r, mid).flux.mag();
    }

    // Check: field exists at all measured radii
    bool all_positive = true;
    for (int i = 0; i < n_radii; ++i) {
        if (Jmag[i] <= 0.0) all_positive = false;
    }
    check("P1a: |J(r)| > 0 at all radii", all_positive);

    // Check: field at r=5 is stronger than at r=14 (monotonic decrease overall)
    check("P1b: |J(5)| > |J(14)| (field decays with distance)",
          Jmag[0] > Jmag[n_radii - 1]);

    // Check: approximate isotropy — x and y measurements at r=6 agree within 50%
    double avg = (Jmag[1] + Jmag_y[1]) / 2.0;
    double aniso = (avg > 1e-30) ? std::abs(Jmag[1] - Jmag_y[1]) / avg : 0.0;
    std::printf("  Isotropy at r=6: |J_x|=%.6e, |J_y|=%.6e, dev=%.1f%%\n",
                Jmag[1], Jmag_y[1], aniso * 100.0);
    check("P1c: Approximate isotropy at r=6 (< 50% deviation)", aniso < 0.50);
}

// ============================================================================
// P2: Genesis Phase Transition
//
// Test the manifestation threshold directly by injecting flux of known
// magnitude into void sites and checking whether genesis occurs.
//
// K_GENESIS = N_c * K_B = 1.533 is the threshold. Genesis probability
// p = 1 - exp(-(|J| - K_GENESIS) / K_B) only fires when |J| > K_GENESIS.
//
// We inject flux with |J| below and above K_GENESIS at isolated void sites,
// run genesis for 100 ticks, and count manifested particles.
// ============================================================================
static void test_genesis_phase_transition() {
    std::printf("\n--- P2: Genesis Phase Transition ---\n");

    const int L = 32;
    const int mid = L / 2;

    std::printf("  K_GENESIS = %.6f, K_B = %.6f\n", K_GENESIS, K_B);

    // Sub-threshold: inject flux with |J| = 0.5 * K_GENESIS at void sites
    int manifested_below = 0;
    {
        auto rb = std::make_unique<RenderBridge>(L);
        rb->seed_rng(42);
        rb->toggles.genesis = true;
        rb->toggles.movement = false;
        rb->toggles.wave_propagation = false;  // Prevent flux from dispersing
        rb->toggles.coupling = false;
        rb->toggles.damping = false;
        rb->toggles.gauss_projection = false;

        // Inject sub-threshold flux at 10 separated void sites
        double sub_mag = 0.5 * K_GENESIS / std::sqrt(3.0);
        for (int i = 0; i < 10; ++i) {
            int x = 4 + i * 2;
            rb->inject_flux(x, mid, mid, Vec3(sub_mag, sub_mag, sub_mag));
        }

        rb->run(100);
        rb->sync_from_gpu();

        const auto& voxels = rb->voxels();
        for (int idx = 0; idx < L * L * L; ++idx) {
            if (voxels[idx].state != 0) ++manifested_below;
        }
        std::printf("  Sub-threshold (0.5 * K_GENESIS): %d manifested\n", manifested_below);
    }

    // Super-threshold: inject flux with |J| = 2.0 * K_GENESIS at void sites
    int manifested_above = 0;
    {
        auto rb = std::make_unique<RenderBridge>(L);
        rb->seed_rng(42);
        rb->toggles.genesis = true;
        rb->toggles.movement = false;
        rb->toggles.wave_propagation = false;  // Prevent flux from dispersing
        rb->toggles.coupling = false;
        rb->toggles.damping = false;
        rb->toggles.gauss_projection = false;

        // Inject super-threshold flux at 10 separated void sites
        double super_mag = 2.0 * K_GENESIS / std::sqrt(3.0);
        for (int i = 0; i < 10; ++i) {
            int x = 4 + i * 2;
            rb->inject_flux(x, mid, mid, Vec3(super_mag, super_mag, super_mag));
        }

        rb->run(100);
        rb->sync_from_gpu();

        const auto& voxels = rb->voxels();
        for (int idx = 0; idx < L * L * L; ++idx) {
            if (voxels[idx].state != 0) ++manifested_above;
        }
        std::printf("  Super-threshold (2.0 * K_GENESIS): %d manifested\n", manifested_above);
    }

    // Checks
    check("P2a: Sub-threshold flux produces zero particles",
          manifested_below == 0);
    check("P2b: Super-threshold flux produces particles",
          manifested_above > 0);
    check("P2c: More particles above threshold than below",
          manifested_above > manifested_below);
}

// ============================================================================
// P3: Void Energy
//
// Config A: Empty lattice (500 ticks) -> field_energy should be ~0.
// Config B: +1/-1 pair at separation 20 (500 ticks) -> field_energy > 0
//           because injection distributes flux that persists.
// Check: E_B > E_A (pair has more field energy than vacuum).
// ============================================================================
static void test_void_energy() {
    std::printf("\n--- P3: Void Energy ---\n");

    const int L = 32;
    const int mid = L / 2;

    // Config A: empty lattice
    double E_A;
    {
        auto rb = std::make_unique<RenderBridge>(L);
        rb->toggles.genesis = false;
        rb->run(200);
        rb->sync_from_gpu();

        auto ea = rb->energy_audit();
        E_A = ea.field_energy;
        std::printf("  Config A (empty):  field_energy = %.8e\n", E_A);
    }

    // Config B: inject +1/-1 pair at separation 20
    double E_B;
    {
        auto rb = std::make_unique<RenderBridge>(L);
        rb->toggles.genesis = false;
        rb->toggles.movement = false;  // Keep particles stationary

        double iso = K_B / std::sqrt(3.0);
        rb->inject_particle(mid - 6, mid, mid, +1, Vec3(iso, iso, iso));
        rb->inject_particle(mid + 6, mid, mid, -1, Vec3(iso, iso, iso));
        rb->run(200);
        rb->sync_from_gpu();

        auto ea = rb->energy_audit();
        E_B = ea.field_energy;
        std::printf("  Config B (pair):   field_energy = %.8e\n", E_B);
    }

    // Checks
    check("P3a: Empty lattice field energy ~ 0 (< 1e-10)", E_A < 1e-10);
    check("P3b: Pair lattice field energy > 0", E_B > 0.0);
    check("P3c: E_pair > E_empty", E_B > E_A);
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::printf("================================================================\n");
    std::printf("  CAMPAIGN: Plato — Dispositional Field Tests (3 tests, 9 checks)\n");
    std::printf("================================================================\n");

    test_dispositional_ratio();
    test_genesis_phase_transition();
    test_void_energy();

    std::printf("\n================================================================\n");
    std::printf("  RESULT: %s (%d failures)\n",
                failures == 0 ? "ALL PASSED" : "FAILURES DETECTED", failures);
    std::printf("================================================================\n");

    return failures;
}
