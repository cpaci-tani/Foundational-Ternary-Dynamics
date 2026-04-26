/**
 * @file test_toggle_matrix.cpp
 * @brief Pairwise toggle-combination smoke test.
 *
 * Closes TEST-005 from CHECKLIST_ENGINE.md. The audit found that virtually
 * all existing tests use enable_all() / disable_all() ± a few flips. The
 * 13 OFF-default toggles produce 2¹³ = 8192 combinations, of which the
 * existing tests probe maybe 30–50 directly.
 *
 * Pairwise (orthogonal-array) coverage of N binary parameters needs
 * roughly O(log N) test runs but enumerating ALL pairs is N(N-1)/2 ≈ 78
 * for N=13. We do all 78 pairs explicitly: each pair (i, j) with both ON,
 * everything else OFF (plus the always-on base toggles wave_propagation +
 * gauss_projection).
 *
 * Pass criteria for each combination:
 *   (a) `validate()` either accepts the combo or rejects it cleanly (no
 *       crash); rejected combos are SKIPPED.
 *   (b) Engine ticks 5 times without throwing, NaN, or signal.
 *   (c) Energy ledger stays finite.
 *
 * Catches bugs of the form "toggle X + toggle Y silently misbehave when
 * combined" — the most common failure mode for new engine extensions.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <utility>
#include <string>

#include "ftd/render_bridge.h"

namespace {

// Setter for each OFF-default toggle. Functions take a TermToggles& and
// flip the named toggle ON while leaving others untouched.
struct ToggleSpec {
    const char* name;
    void (*set_on)(ftd::TermToggles&);
};

// 13 OFF-default toggles per term_toggles.h.
const ToggleSpec specs[] = {
    {"larmor_radiation",  [](ftd::TermToggles& t){ t.larmor_radiation  = true; t.damping = true; }},  // requires damping
    {"color_forces",      [](ftd::TermToggles& t){ t.color_forces      = true; }},
    {"strong_force",      [](ftd::TermToggles& t){ t.strong_force      = true; }},
    {"triad_binding",     [](ftd::TermToggles& t){ t.triad_binding     = true; t.color_forces = true; }}, // requires color_forces
    {"pair_production",   [](ftd::TermToggles& t){ t.pair_production   = true; }},
    {"exchange_force",    [](ftd::TermToggles& t){ t.exchange_force    = true; t.poisson_coulomb = true; }}, // requires poisson_coulomb
    {"latency_field",     [](ftd::TermToggles& t){ t.latency_field     = true; t.gravity = true; }}, // requires gravity
    {"exact_dual_gauss",  [](ftd::TermToggles& t){ t.exact_dual_gauss  = true; }},
    {"emergent_forces",   [](ftd::TermToggles& t){ t.emergent_forces   = true; t.poisson_coulomb = false; }}, // mutex with poisson
    {"langevin",          [](ftd::TermToggles& t){ t.langevin          = true; t.langevin_T = 0.005; t.langevin_gamma = 0.02; }},
    {"weak_transmutation",[](ftd::TermToggles& t){ t.weak_transmutation= true; t.dual_substrate = true; }}, // requires dual_substrate
    {"dual_substrate",    [](ftd::TermToggles& t){ t.dual_substrate    = true; }},
    {"selective_damping", [](ftd::TermToggles& t){ t.selective_damping = true; t.damping = true; }},
};
const int N_TOGGLES = static_cast<int>(sizeof(specs) / sizeof(specs[0]));

void enable_base(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.movement         = true;
}

bool finite_audit(const ftd::EnergyLedger& l) {
    return std::isfinite(l.E_curr) && std::isfinite(l.dE_dt) && std::isfinite(l.residual);
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  TEST-005: Toggle Pairwise Smoke Matrix (%d toggles, %d pairs)\n",
                N_TOGGLES, N_TOGGLES * (N_TOGGLES - 1) / 2);
    std::printf("================================================================\n\n");

    int failures = 0;
    int rejected_by_validate = 0;
    int passed = 0;

    auto run_pair = [&](int i, int j) -> bool {
        ftd::RenderBridge rb(8);
        enable_base(rb.toggles);
        specs[i].set_on(rb.toggles);
        specs[j].set_on(rb.toggles);
        rb.force_cpu();
        rb.seed_rng(0xBADC0FFEEu + i * 100 + j);

        // Skip combinations that the validator rejects. (Not a failure —
        // the validator's job is exactly this.)
        std::string err;
        if (!rb.toggles.validate(&err)) {
            ++rejected_by_validate;
            return true;
        }

        // Inject a small charged pair so something interesting can happen
        // for force/exchange/triad/pair_production-on combos.
        rb.inject_particle(3, 4, 4, +1, ftd::Vec3{0, 0, 0}, 0, 1);
        rb.inject_particle(5, 4, 4, -1, ftd::Vec3{0, 0, 0}, 0, 2);

        try {
            for (int t = 0; t < 5; ++t) rb.tick();
        } catch (const std::exception& e) {
            std::printf("  FAIL  pair (%s, %s): exception: %s\n",
                        specs[i].name, specs[j].name, e.what());
            return false;
        }

        if (!finite_audit(rb.energy_ledger())) {
            std::printf("  FAIL  pair (%s, %s): non-finite energy ledger\n",
                        specs[i].name, specs[j].name);
            return false;
        }
        return true;
    };

    for (int i = 0; i < N_TOGGLES; ++i) {
        for (int j = i + 1; j < N_TOGGLES; ++j) {
            if (run_pair(i, j)) ++passed;
            else                ++failures;
        }
    }

    std::printf("\n  Combinations tested: %d (passed) + %d (validator-rejected) + %d (failed)\n",
                passed, rejected_by_validate, failures);
    std::printf("================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: PASS — all %d toggle pairs survive 5 ticks without crash/NaN\n",
                    passed + rejected_by_validate);
    } else {
        std::printf("  RESULT: FAIL — %d pairs trigger crash, exception, or non-finite state\n",
                    failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
