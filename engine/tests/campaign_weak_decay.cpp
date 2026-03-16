/**
 * Campaign: Weak Decay Rate (Phase 6 — Weak Sector & SU(2))
 *
 * Measures how transmutation rate depends on stress level,
 * verifying the exponential probability formula.
 *
 * Theory: Transmutation probability per tick:
 *   p(stress) = 1 - exp(-(stress - WEAK_THRESHOLD) / K_B)   for stress > WEAK_THRESHOLD
 *   p(stress) = 0                                             for stress <= WEAK_THRESHOLD
 *
 * This gives:
 *   - Sharp threshold at WEAK_THRESHOLD = K_GENESIS = 1.533
 *   - Exponential rise with scale K_B = 0.511
 *   - Saturation p → 1 when stress >> WEAK_THRESHOLD + K_B
 *
 * The functional form is [IMPOSED] from analogy with Fermi's golden rule.
 * What [EMERGES] is the stress-dependent transition rate landscape.
 *
 * Protocol:
 *   1. Create particles with varying injected flux (varying stress)
 *   2. Run ensemble at each stress level
 *   3. Measure transmutation fraction vs stress
 *   4. Verify threshold behavior and monotonic increase
 *
 * Checks:
 *   WD1: Transmutation rate = 0 when stress < WEAK_THRESHOLD
 *   WD2: Transmutation rate > 0 when stress > WEAK_THRESHOLD
 *   WD3: Rate increases monotonically with stress
 *   WD4: Manifested count is conserved (flip, not evaporation)
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

struct StressResult {
    double injection_amp;
    double measured_stress;
    int flips;
    int trials;
    double rate() const { return (double)flips / trials; }
};

StressResult measure_rate(int L, double injection_amp, int trials, int warmup) {
    int mid = L / 2;
    int flips = 0;

    for (int trial = 0; trial < trials; ++trial) {
        ftd::RenderBridge rb(L);
        rb.seed_rng(3000 + trial);
        rb.toggles.genesis = false;
        rb.toggles.weak_transmutation = true;

        rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(warmup);

        // Inject flux to create stress
        if (injection_amp > 0) {
            rb.inject_flux(mid+1, mid, mid, {injection_amp, 0, 0});
            rb.inject_flux(mid-1, mid, mid, {-injection_amp, 0, 0});
        }
        rb.tick();  // propagate stress

        int8_t before = rb.voxels()[rb.lattice().index(mid, mid, mid)].state;
        rb.tick();  // attempt transmutation
        int8_t after = rb.voxels()[rb.lattice().index(mid, mid, mid)].state;

        if (after != before) ++flips;
    }

    // Measure stress at one instance for reporting
    ftd::RenderBridge rb_ref(L);
    rb_ref.toggles.genesis = false;
    rb_ref.toggles.weak_transmutation = false;
    rb_ref.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
    rb_ref.voxels()[rb_ref.lattice().index(mid, mid, mid)].locked = true;
    rb_ref.run(warmup);
    if (injection_amp > 0) {
        rb_ref.inject_flux(mid+1, mid, mid, {injection_amp, 0, 0});
        rb_ref.inject_flux(mid-1, mid, mid, {-injection_amp, 0, 0});
    }
    rb_ref.tick();
    double stress = rb_ref.compute_stress(rb_ref.lattice().index(mid, mid, mid));

    return {injection_amp, stress, flips, trials};
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Weak Decay Rate (Phase 6) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int WARMUP = 200;
    const int TRIALS = 30;

    // Measure at various injection amplitudes
    const int N_AMP = 5;
    double amps[N_AMP] = {0.0, ftd::K_B * 2.0, ftd::K_B * 5.0, ftd::K_B * 10.0, ftd::K_B * 20.0};
    StressResult results[N_AMP];

    std::cout << "\n--- Transmutation Rate vs Stress ---\n";
    std::cout << "  Amp/K_B  | Stress       | Flips/Trials | Rate\n";

    for (int i = 0; i < N_AMP; ++i) {
        results[i] = measure_rate(L, amps[i], TRIALS, WARMUP);
        std::cout << "  " << std::setw(7) << amps[i] / ftd::K_B
                  << " | " << std::setw(12) << results[i].measured_stress
                  << " | " << std::setw(5) << results[i].flips << "/" << results[i].trials
                  << "     | " << std::setw(8) << results[i].rate() << "\n";
    }

    // ================================================================
    // Part 2: Verify manifested count conservation
    // ================================================================
    bool count_conserved = true;
    {
        ftd::RenderBridge rb(L);
        rb.seed_rng(7777);
        rb.toggles.genesis = false;
        rb.toggles.weak_transmutation = true;

        // Create 5 particles spread out
        int positions[5][3] = {{8,16,16}, {12,16,16}, {16,16,16}, {20,16,16}, {24,16,16}};
        for (int i = 0; i < 5; ++i) {
            rb.inject_particle(positions[i][0], positions[i][1], positions[i][2],
                               (i % 2 == 0) ? +1 : -1, {ftd::K_B, 0, 0});
            rb.voxels()[rb.lattice().index(positions[i][0], positions[i][1], positions[i][2])].locked = true;
        }

        rb.run(WARMUP);

        auto audit_before = rb.energy_audit();
        int count_before = audit_before.manifested_count;

        // Inject stress near all particles
        double big_amp = ftd::K_B * 20.0;
        for (int i = 0; i < 5; ++i) {
            rb.inject_flux(positions[i][0]+1, positions[i][1], positions[i][2], {big_amp, 0, 0});
        }

        rb.run(10);  // Several transmutation opportunities

        auto audit_after = rb.energy_audit();
        int count_after = audit_after.manifested_count;

        std::cout << "\n--- Manifested Count Conservation ---\n";
        std::cout << "  Before: " << count_before << "\n";
        std::cout << "  After:  " << count_after << "\n";

        count_conserved = (count_before == count_after);
    }

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // WD1: No transmutation at zero injection (stress below threshold)
    check("WD1: No transmutation when stress < WEAK_THRESHOLD",
          results[0].flips == 0);

    // WD2: Transmutation occurs at high injection (stress above threshold)
    // At least one of the higher amplitudes should produce flips
    bool any_flips = false;
    for (int i = 1; i < N_AMP; ++i)
        if (results[i].flips > 0) any_flips = true;
    check("WD2: Transmutation occurs when stress > WEAK_THRESHOLD",
          any_flips);

    // WD3: Rate increases monotonically with stress (at least weakly)
    // Allow ties but no decreases between successive high-stress levels
    bool monotonic = true;
    for (int i = 2; i < N_AMP; ++i) {
        if (results[i].rate() < results[i-1].rate() - 0.15) {
            monotonic = false;  // Allow statistical fluctuation of 15%
        }
    }
    check("WD3: Transmutation rate increases with stress (monotonic ± noise)",
          monotonic);

    // WD4: Manifested count is conserved (transmutation is flip, not evaporation)
    check("WD4: Manifested count conserved during transmutation",
          count_conserved);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  NOTE: The transmutation probability formula is [IMPOSED]:\n";
    std::cout << "  p = 1 - exp(-(stress - K_GENESIS) / K_B).\n";
    std::cout << "  The threshold K_GENESIS = 3*K_B = " << ftd::K_GENESIS << " is [DERIVED].\n";
    std::cout << "  What [EMERGES] is the stress landscape around particles\n";
    std::cout << "  and the statistical decay rate behavior.\n";
    std::cout << "================================================================\n";
    return failures;
}
