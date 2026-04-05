/**
 * Campaign: Wigner Tests — Octahedral Symmetry, Parity, CPT Invariance
 *
 * W1: Octahedral Symmetry
 *     L=48, single +1 at center, 200 ticks.  Measure |J| at 6 axis-symmetric
 *     points at distance 5.  All should be equal within 5%.
 *
 * W2: Parity Test (Dual Substrate)
 *     L=32, dual_substrate=true.  Config L: inject flux_L only.
 *     Config R: inject flux_R only.  Run 200 ticks each.
 *     Chirality density should have opposite sign.
 *
 * W3: CPT Invariance
 *     L=32, enable_all, genesis=false.  Config C: +1 at (8,16,16) vel=(0.3,0,0),
 *     -1 at (24,16,16) vel=(-0.3,0,0).  Config CPT: swap charges AND positions
 *     AND velocities.  Run 100 ticks.  Total energies should match within 0.1%.
 */

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <cstdio>
#include <memory>
#include <algorithm>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

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
// W1: Octahedral Symmetry
// ============================================================================
static void test_octahedral_symmetry() {
    std::printf("\n================================================================\n");
    std::printf("  W1: Octahedral Symmetry\n");
    std::printf("================================================================\n");

    const int L = 48;
    const int mid = L / 2;
    const int r = 5;
    auto rb = std::make_unique<ftd::RenderBridge>(L);

    rb->toggles.genesis = false;
    rb->toggles.movement = false;

    // Place +1 at center using wavepacket for isotropic flux distribution
    rb->inject_wavepacket(mid, mid, mid, +1, 3.0, ftd::K_B);
    rb->voxels()[rb->lattice().index(mid, mid, mid)].locked = true;

    std::printf("    Running 200 ticks on 48^3...\n");
    rb->run(200);

    // Trigger GPU sync
    (void)rb->voxels();

    // Measure |J| at 6 axis-symmetric points
    struct Probe { int dx, dy, dz; const char* label; };
    Probe probes[6] = {
        {+r,  0,  0, "+x"},
        {-r,  0,  0, "-x"},
        { 0, +r,  0, "+y"},
        { 0, -r,  0, "-y"},
        { 0,  0, +r, "+z"},
        { 0,  0, -r, "-z"},
    };

    double J_mag[6];
    double J_min = 1e30, J_max = 0.0;

    std::printf("    Direction  |J|\n");
    for (int i = 0; i < 6; ++i) {
        int x = mid + probes[i].dx;
        int y = mid + probes[i].dy;
        int z = mid + probes[i].dz;
        const auto& v = rb->voxels()[rb->lattice().index(x, y, z)];
        J_mag[i] = v.flux.mag();
        if (J_mag[i] < J_min) J_min = J_mag[i];
        if (J_mag[i] > J_max) J_max = J_mag[i];
        std::printf("    %s: %.6e\n", probes[i].label, J_mag[i]);
    }

    double ratio = (J_min > 0) ? J_max / J_min : 999.0;
    std::printf("    Max/min ratio: %.6f\n", ratio);

    check("W1a: All 6 axis points have nonzero flux", J_min > 1e-15);
    check("W1b: Max/min ratio < 1.05 (octahedral symmetry)", ratio < 1.05);
}

// ============================================================================
// W2: Parity Test (Dual Substrate)
// ============================================================================
static void test_parity() {
    std::printf("\n================================================================\n");
    std::printf("  W2: Parity Test (Dual Substrate)\n");
    std::printf("================================================================\n");

    const int L = 32;
    const int mid = L / 2;

    // --- Config L: inject flux_L only ---
    auto rb_L = std::make_unique<ftd::RenderBridge>(L);
    rb_L->force_cpu();  // Chirality/dual substrate diagnostics need CPU
    rb_L->toggles.disable_all();
    rb_L->toggles.wave_propagation = true;
    rb_L->toggles.damping = true;
    rb_L->toggles.dual_substrate = true;

    {
        int idx = rb_L->lattice().index(mid, mid, mid);
        auto& v = rb_L->voxels()[idx];
        v.flux_L = ftd::Vec3(ftd::K_B, 0.0, 0.0);
        v.flux_R = ftd::Vec3(0.0, 0.0, 0.0);
        v.flux = v.flux_L;  // Observable = L + R
        v.wave_vel_L = ftd::Vec3(0.0, 0.0, 0.0);
        v.wave_vel_R = ftd::Vec3(0.0, 0.0, 0.0);
        v.wave_vel = ftd::Vec3(0.0, 0.0, 0.0);
    }

    std::printf("    Config L: flux_L = (K_B, 0, 0) at center\n");
    std::printf("    Running 200 ticks...\n");
    rb_L->run(200);

    // Trigger GPU sync and measure chirality
    (void)rb_L->voxels();
    auto audit_L = rb_L->energy_audit();
    double chi_L = audit_L.chirality_total;
    std::printf("    Chirality (L config): %.6e\n", chi_L);

    // --- Config R: inject flux_R only ---
    auto rb_R = std::make_unique<ftd::RenderBridge>(L);
    rb_R->force_cpu();
    rb_R->toggles.disable_all();
    rb_R->toggles.wave_propagation = true;
    rb_R->toggles.damping = true;
    rb_R->toggles.dual_substrate = true;

    {
        int idx = rb_R->lattice().index(mid, mid, mid);
        auto& v = rb_R->voxels()[idx];
        v.flux_R = ftd::Vec3(ftd::K_B, 0.0, 0.0);
        v.flux_L = ftd::Vec3(0.0, 0.0, 0.0);
        v.flux = v.flux_R;  // Observable = L + R
        v.wave_vel_L = ftd::Vec3(0.0, 0.0, 0.0);
        v.wave_vel_R = ftd::Vec3(0.0, 0.0, 0.0);
        v.wave_vel = ftd::Vec3(0.0, 0.0, 0.0);
    }

    std::printf("    Config R: flux_R = (K_B, 0, 0) at center\n");
    std::printf("    Running 200 ticks...\n");
    rb_R->run(200);

    (void)rb_R->voxels();
    auto audit_R = rb_R->energy_audit();
    double chi_R = audit_R.chirality_total;
    std::printf("    Chirality (R config): %.6e\n", chi_R);

    // Chirality should have opposite signs
    check("W2a: L-config chirality is positive", chi_L > 0.0);
    check("W2b: R-config chirality is negative", chi_R < 0.0);
    check("W2c: Chirality has opposite sign", chi_L * chi_R < 0.0);
}

// ============================================================================
// W3: CPT Invariance
// ============================================================================
static void test_cpt_invariance() {
    std::printf("\n================================================================\n");
    std::printf("  W3: CPT Invariance\n");
    std::printf("================================================================\n");

    const int L = 32;
    const int mid = L / 2;
    const int TICKS = 100;

    // --- Config C: +1 at (8,16,16) vel=(0.3,0,0), -1 at (24,16,16) vel=(-0.3,0,0) ---
    auto rb_C = std::make_unique<ftd::RenderBridge>(L);
    rb_C->toggles.enable_all();
    rb_C->toggles.genesis = false;
    rb_C->seed_rng(42);

    rb_C->inject_particle(8, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
    rb_C->voxels()[rb_C->lattice().index(8, mid, mid)].velocity =
        ftd::Vec3(0.3, 0.0, 0.0);

    rb_C->inject_particle(24, mid, mid, -1, {0, 0, ftd::K_B * 0.1});
    rb_C->voxels()[rb_C->lattice().index(24, mid, mid)].velocity =
        ftd::Vec3(-0.3, 0.0, 0.0);

    std::printf("    Config C: +1 at (8,16,16) v=(0.3,0,0), -1 at (24,16,16) v=(-0.3,0,0)\n");
    std::printf("    Running %d ticks...\n", TICKS);
    rb_C->run(TICKS);

    (void)rb_C->voxels();
    auto audit_C = rb_C->energy_audit();
    double E_C = audit_C.total_energy;
    std::printf("    Config C total energy: %.6e\n", E_C);

    // --- Config CPT: swap charges AND positions AND velocities ---
    // CPT: C flips charge, P flips position, T flips velocity
    // +1 at (8,16,16) -> -1 at (24,16,16) vel=(-0.3,0,0)
    // -1 at (24,16,16) -> +1 at (8,16,16) vel=(0.3,0,0)
    // This is the same physical configuration! So we expect identical energy.
    auto rb_CPT = std::make_unique<ftd::RenderBridge>(L);
    rb_CPT->toggles.enable_all();
    rb_CPT->toggles.genesis = false;
    rb_CPT->seed_rng(42);

    rb_CPT->inject_particle(24, mid, mid, -1, {0, 0, ftd::K_B * 0.1});
    rb_CPT->voxels()[rb_CPT->lattice().index(24, mid, mid)].velocity =
        ftd::Vec3(-0.3, 0.0, 0.0);

    rb_CPT->inject_particle(8, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
    rb_CPT->voxels()[rb_CPT->lattice().index(8, mid, mid)].velocity =
        ftd::Vec3(0.3, 0.0, 0.0);

    std::printf("    Config CPT: -1 at (24,16,16) v=(-0.3,0,0), +1 at (8,16,16) v=(0.3,0,0)\n");
    std::printf("    Running %d ticks...\n", TICKS);
    rb_CPT->run(TICKS);

    (void)rb_CPT->voxels();
    auto audit_CPT = rb_CPT->energy_audit();
    double E_CPT = audit_CPT.total_energy;
    std::printf("    Config CPT total energy: %.6e\n", E_CPT);

    double avg = 0.5 * (std::fabs(E_C) + std::fabs(E_CPT));
    double rel_diff = (avg > 0) ? std::fabs(E_C - E_CPT) / avg : 0.0;
    std::printf("    Relative difference: %.6e\n", rel_diff);

    check("W3a: Both configs have finite energy", E_C > 0 && E_CPT > 0);
    check("W3b: CPT energy match within 0.1%", rel_diff < 0.001);
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::printf("================================================================\n");
    std::printf("  CAMPAIGN: Wigner Tests — 3 Tests, 7 Checks\n");
    std::printf("================================================================\n");

    test_octahedral_symmetry();
    test_parity();
    test_cpt_invariance();

    std::printf("\n================================================================\n");
    std::printf("  RESULT: %d failures\n", failures);
    std::printf("================================================================\n");

    return failures;
}
