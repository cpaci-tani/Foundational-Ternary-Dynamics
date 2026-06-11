/**
 * Campaign: Weak Transmutation (Phase 6 — Weak Sector & SU(2))
 *
 * Tests stress-threshold polarity flipping (+1 <-> -1) as the FTD
 * analog of weak interactions (beta decay).
 *
 * Theory: In the Standard Model, weak interactions mediate quark flavor
 * transitions (d -> u + W^-). In FTD, the analog is a polarity flip
 * when field stress exceeds WEAK_THRESHOLD = K_GENESIS = 3*K_B.
 *
 * Stress = |div(J)| + |curl(J)| + |grad(rho)|
 *
 * When stress > WEAK_THRESHOLD, transmutation probability is:
 *   p = 1 - exp(-(stress - WEAK_THRESHOLD) / K_B)     [IMPOSED]
 *
 * Protocol:
 *   1. Create isolated particle, warmup -> measure stress (expect < threshold)
 *   2. Create particle with large injected flux nearby -> measure stress (expect > threshold)
 *   3. Enable weak_transmutation, tick, check for polarity flip
 *   4. Verify toggle backward compatibility
 *
 * Checks:
 *   WT1: Isolated warm particle has stress < WEAK_THRESHOLD (no transmutation)
 *   WT2: High-flux injection creates stress > WEAK_THRESHOLD
 *   WT3: Transmutation occurs when stress > threshold and toggle is ON
 *   WT4: No transmutation when toggle is OFF (backward compatibility)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"



int main() {
    ftd::test::init("campaign_weak_transmutation");
    ftd::test::section("weak_sector");

    const int L = 32;
    const int mid = L / 2;
    const int WARMUP = 200;

    // ================================================================
    // Part 1: Isolated particle — stress should be below threshold
    // ================================================================
    double stress_isolated = 0.0;
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.weak_transmutation = false;  // OFF for measurement

        rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(WARMUP);

        stress_isolated = rb.compute_stress(rb.lattice().index(mid, mid, mid));

        std::cout << "\n--- Isolated Particle (warmup " << WARMUP << " ticks) ---\n";
        std::cout << "  Stress at particle:  " << stress_isolated << "\n";
        std::cout << "  WEAK_THRESHOLD:      " << ftd::WEAK_THRESHOLD << "\n";
    }

    // ================================================================
    // Part 2: High-flux injection — create stress above threshold
    // ================================================================
    double stress_high = 0.0;
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.weak_transmutation = false;  // OFF for measurement

        rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(WARMUP);

        // Inject strong flux nearby to create high stress
        // Use amplitude >> K_B to create large gradients
        double amp = ftd::K_B * 10.0;
        rb.inject_flux(mid+1, mid, mid, {amp, 0, 0});
        rb.inject_flux(mid-1, mid, mid, {-amp, 0, 0});
        rb.inject_flux(mid, mid+1, mid, {0, amp, 0});

        rb.tick();  // Let flux propagate one step

        stress_high = rb.compute_stress(rb.lattice().index(mid, mid, mid));

        std::cout << "\n--- High-Flux Injection ---\n";
        std::cout << "  Stress at particle:  " << stress_high << "\n";
        std::cout << "  WEAK_THRESHOLD:      " << ftd::WEAK_THRESHOLD << "\n";
    }

    // ================================================================
    // Part 3: Transmutation with toggle ON + high stress
    // ================================================================
    int flipped_count = 0;
    int total_trials = 20;
    {
        // Run multiple trials (probabilistic transmutation)
        for (int trial = 0; trial < total_trials; ++trial) {
            std::cout << "  Trial " << (trial+1) << "/" << total_trials << "...\n";
            ftd::RenderBridge rb(L);
            rb.seed_rng(1000 + trial);
            rb.toggles.genesis = false;
            rb.toggles.weak_transmutation = true;  // ON

            rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

            rb.run(WARMUP);

            // Inject strong flux to create high stress
            double amp = ftd::K_B * 10.0;
            rb.inject_flux(mid+1, mid, mid, {amp, 0, 0});
            rb.inject_flux(mid-1, mid, mid, {-amp, 0, 0});
            rb.inject_flux(mid, mid+1, mid, {0, amp, 0});

            rb.tick();  // Propagate flux to create stress

            // Now the next tick should attempt transmutation
            int8_t state_before = rb.voxels()[rb.lattice().index(mid, mid, mid)].state;
            rb.tick();
            int8_t state_after = rb.voxels()[rb.lattice().index(mid, mid, mid)].state;

            if (state_after != state_before) ++flipped_count;
        }

        std::cout << "\n--- Transmutation Test (toggle ON, high stress) ---\n";
        std::cout << "  Trials: " << total_trials << "\n";
        std::cout << "  Flipped: " << flipped_count << "\n";
        std::cout << "  Rate: " << (double)flipped_count / total_trials << "\n";
    }

    // ================================================================
    // Part 4: No transmutation with toggle OFF
    // ================================================================
    int flipped_off = 0;
    {
        for (int trial = 0; trial < total_trials; ++trial) {
            std::cout << "  Trial " << (trial+1) << "/" << total_trials << "...\n";
            ftd::RenderBridge rb(L);
            rb.seed_rng(1000 + trial);
            rb.toggles.genesis = false;
            rb.toggles.weak_transmutation = false;  // OFF

            rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

            rb.run(WARMUP);

            double amp = ftd::K_B * 10.0;
            rb.inject_flux(mid+1, mid, mid, {amp, 0, 0});
            rb.inject_flux(mid-1, mid, mid, {-amp, 0, 0});
            rb.inject_flux(mid, mid+1, mid, {0, amp, 0});

            rb.tick();

            int8_t state_before = rb.voxels()[rb.lattice().index(mid, mid, mid)].state;
            rb.tick();
            int8_t state_after = rb.voxels()[rb.lattice().index(mid, mid, mid)].state;

            if (state_after != state_before) ++flipped_off;
        }

        std::cout << "\n--- No Transmutation Test (toggle OFF) ---\n";
        std::cout << "  Flipped: " << flipped_off << " / " << total_trials << "\n";
    }

    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // WT1: Isolated particle stress below threshold
    ftd::test::check("WT1: Isolated warm particle stress < WEAK_THRESHOLD",
          stress_isolated < ftd::WEAK_THRESHOLD);

    // WT2: High-flux injection creates stress above threshold
    ftd::test::check("WT2: High-flux injection stress > WEAK_THRESHOLD",
          stress_high > ftd::WEAK_THRESHOLD);

    // WT3: At least some transmutations occurred with toggle ON + high stress
    ftd::test::check("WT3: Transmutation occurs when stress > threshold (toggle ON)",
          flipped_count > 0);

    // WT4: No transmutations when toggle OFF
    ftd::test::check("WT4: No transmutation when toggle OFF (backward compatibility)",
          flipped_off == 0);

    std::cout << "  NOTE: Transmutation probability and threshold are [IMPOSED]\n";
    std::cout << "  from electroweak theory. The stress formula is [IMPOSED]\n";
    std::cout << "  (|div J| + |curl J| + |grad rho|). What [EMERGES] is that\n";
    std::cout << "  high field stress environments trigger state transitions,\n";
    std::cout << "  analogous to weak decay in high-density nuclear matter.\n";

    return ftd::test::finalize();
}
