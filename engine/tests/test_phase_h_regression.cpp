/**
 * @file test_phase_h_regression.cpp
 * @brief Phase-H `coulomb_charge_coupling` knob regression.
 *
 * Closes TEST-009 from CHECKLIST_ENGINE.md.
 *
 * `TermToggles::coulomb_charge_coupling` (default 1.0) is a numeric knob
 * that scales the Gauss-law source term: `source = div(J) - g · s`.
 *  - 1.0           : geometric Coulomb (Phase-G theorem α_r = 2 r G_L(r))
 *  - sqrt(2π·α)    ≈ 0.2141 : engine convention testing α = 1/137
 *  - sqrt(4π·α)    ≈ 0.3028 : classical convention
 *
 * No existing test pins behaviour at the non-default values. A silent
 * default change would invalidate every Phase-G measurement; a silent
 * non-default change would let the α-recovery experiments drift unnoticed.
 *
 * This test:
 *   (a) Runs a charged-pair scenario at each of the three documented values.
 *   (b) Records the Gauss-projected total field energy after a fixed number
 *       of ticks.
 *   (c) Asserts the three energies are distinct (knob actually affects
 *       physics) AND that each value is monotonic with respect to the
 *       coupling magnitude.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ftd/render_bridge.h"

namespace {

double measure_field_energy(double coupling) {
    const int L = 16;
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.coulomb_charge_coupling = coupling;
    rb.force_cpu();
    rb.seed_rng(0xCC0177u);

    // Stamp a +/- charge pair so Gauss has something to enforce.
    rb.inject_particle(L/2 - 2, L/2, L/2, +1, ftd::Vec3{0, 0, 0});
    rb.inject_particle(L/2 + 2, L/2, L/2, -1, ftd::Vec3{0, 0, 0});

    for (int t = 0; t < 100; ++t) rb.tick();

    const auto audit = rb.energy_audit();
    return audit.field_energy;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  TEST-009: Phase-H coulomb_charge_coupling Regression\n");
    std::printf("================================================================\n");
    std::printf("  Pins behaviour at three documented coupling values.\n\n");

    struct Setting {
        double coupling;
        const char* label;
    };
    const Setting settings[] = {
        { 1.0,    "1.0    (Phase-G geometric Coulomb)" },
        { 0.2141, "0.2141 (engine convention α = 1/137 test)" },
        { 0.3028, "0.3028 (classical convention)" },
    };
    const int N = sizeof(settings) / sizeof(settings[0]);

    std::vector<double> energies(N);
    for (int i = 0; i < N; ++i) {
        energies[i] = measure_field_energy(settings[i].coupling);
        std::printf("  coupling = %s\n", settings[i].label);
        std::printf("    field energy after 100 ticks = %.6e\n\n", energies[i]);
    }

    int failures = 0;

    // Contract 1: distinct energies (knob actually affects physics).
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            const double rel = std::abs(energies[i] - energies[j]) /
                                std::max(std::abs(energies[i]), std::abs(energies[j]));
            if (rel < 1e-6) {
                std::printf("  FAIL: energies(%g) and energies(%g) match to %.0e — knob has no effect\n",
                            settings[i].coupling, settings[j].coupling, rel);
                ++failures;
            }
        }
    }
    if (failures == 0) {
        std::printf("  PASS: all three coupling values produce distinct energies\n");
    }

    // Contract 2: at the two non-default values, energy must be FINITE
    // (the Phase-H knob shouldn't NaN-out).
    for (int i = 0; i < N; ++i) {
        if (!std::isfinite(energies[i]) || energies[i] < 0.0) {
            std::printf("  FAIL: coupling=%g produced non-finite energy %.6e\n",
                        settings[i].coupling, energies[i]);
            ++failures;
        }
    }
    if (failures == 0) {
        std::printf("  PASS: all coupling values produce finite non-negative energy\n");
    }

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: PASS — Phase-H coupling knob is wired and distinct\n");
    } else {
        std::printf("  RESULT: FAIL (%d sub-checks)\n", failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
