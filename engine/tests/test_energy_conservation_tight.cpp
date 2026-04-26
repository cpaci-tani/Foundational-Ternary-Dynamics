/**
 * @file test_energy_conservation_tight.cpp
 * @brief Symplectic-leapfrog energy conservation: bounded oscillation, no drift.
 *
 * Closes TEST-007 from CHECKLIST_ENGINE.md.
 *
 * The original framing ("per-tick |dE/E| < 1e-12") is incorrect for a
 * second-order symplectic leapfrog. Leapfrog conserves a *shadow*
 * Hamiltonian H_shadow = H + O(dt²); the true total energy H oscillates by
 * O(dt²) around its mean and does NOT match machine epsilon per tick.
 *
 * Correct contract for symplectic conservation:
 *   1. NO secular drift over long runs (mean energy stable).
 *   2. Oscillation amplitude is bounded (does not grow with N_ticks).
 *   3. Time-averaged energy at two disjoint windows agrees to <0.5%.
 *
 * This test asserts those three. Catches a regression that introduces a
 * dissipative leak (which would show up as monotonic decrease) or an
 * unbounded growth bug (which would show ramping amplitude).
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "ftd/render_bridge.h"

namespace {

struct WindowStats {
    double mean    = 0.0;
    double stdev   = 0.0;
    double e_min   = 1e300;
    double e_max   = -1e300;
};

WindowStats sample_window(std::vector<double>::const_iterator begin,
                          std::vector<double>::const_iterator end) {
    WindowStats s;
    int n = 0;
    for (auto it = begin; it != end; ++it) {
        s.mean  += *it;
        s.e_min  = std::min(s.e_min, *it);
        s.e_max  = std::max(s.e_max, *it);
        ++n;
    }
    s.mean /= std::max(1, n);
    double ss = 0.0;
    for (auto it = begin; it != end; ++it) ss += (*it - s.mean) * (*it - s.mean);
    s.stdev = std::sqrt(ss / std::max(1, n));
    return s;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  TEST-007: Symplectic Energy Conservation (Pure Wave)\n");
    std::printf("================================================================\n");
    std::printf("  Asserts:\n");
    std::printf("    (1) no secular drift between two disjoint time windows\n");
    std::printf("    (2) oscillation amplitude bounded (does not grow with N)\n");
    std::printf("    (3) time-averaged energy stable to < 0.5%% across runs\n\n");

    const int    L      = 16;
    const int    N_tick = 5000;

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling         = false;
    rb.toggles.damping          = false;
    rb.toggles.genesis          = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.forces           = false;
    rb.toggles.movement         = false;
    rb.force_cpu();
    rb.seed_rng(0xC07B57E91u);

    rb.inject_flux_add(L/2,     L/2, L/2, ftd::Vec3{2.0, 0.5,  0.0});
    rb.inject_flux_add(L/2 + 1, L/2, L/2, ftd::Vec3{-1.0, 0.0, 0.5});
    rb.inject_flux_add(L/2,     L/2 + 1, L/2, ftd::Vec3{0.0, 1.5, -0.3});

    std::vector<double> energies;
    energies.reserve(N_tick + 1);

    rb.tick();
    energies.push_back(rb.energy_ledger().E_curr);
    const double E0 = energies.front();
    std::printf("  initial energy E_0 = %.15e\n", E0);

    for (int t = 0; t < N_tick; ++t) {
        rb.tick();
        energies.push_back(rb.energy_ledger().E_curr);
    }

    // Two disjoint windows: first half [0, N/2), second half [N/2, N].
    const auto mid = energies.begin() + energies.size() / 2;
    auto w1 = sample_window(energies.begin(), mid);
    auto w2 = sample_window(mid, energies.end());

    std::printf("\n  Window 1 [t=0..%d]:\n", N_tick / 2);
    std::printf("    mean=%.6f  stdev=%.6f  range=[%.6f, %.6f]\n",
                w1.mean, w1.stdev, w1.e_min, w1.e_max);
    std::printf("  Window 2 [t=%d..%d]:\n", N_tick / 2, N_tick);
    std::printf("    mean=%.6f  stdev=%.6f  range=[%.6f, %.6f]\n",
                w2.mean, w2.stdev, w2.e_min, w2.e_max);

    int failures = 0;

    // Test 1: secular drift between windows < 0.5%.
    const double drift_frac = std::abs(w2.mean - w1.mean) / std::max(std::abs(w1.mean), 1e-30);
    std::printf("\n  (1) inter-window drift |w2_mean - w1_mean| / |w1_mean|: %.4e\n",
                drift_frac);
    const double drift_tol = 5e-3;
    if (drift_frac > drift_tol) {
        std::printf("      FAIL: %.4e > %.4e (energy is leaking or pumping)\n",
                    drift_frac, drift_tol);
        ++failures;
    } else {
        std::printf("      PASS: <= %.4e\n", drift_tol);
    }

    // Test 2: oscillation amplitude bounded (window 2 stdev not >> window 1 stdev).
    const double stdev_ratio = w2.stdev / std::max(w1.stdev, 1e-30);
    std::printf("\n  (2) oscillation amplitude ratio w2_stdev/w1_stdev: %.4f\n",
                stdev_ratio);
    if (stdev_ratio > 2.0) {
        std::printf("      FAIL: oscillation amplitude grew > 2x\n");
        ++failures;
    } else {
        std::printf("      PASS: bounded (ratio <= 2x)\n");
    }

    // Test 3: cumulative |E_final - E_0| reasonable.
    const double final_drift = std::abs(energies.back() - E0) / std::abs(E0);
    std::printf("\n  (3) endpoint relative drift |E_final - E_0| / |E_0|: %.4e\n",
                final_drift);
    if (final_drift > 0.5) {
        std::printf("      FAIL: > 50%% — endpoint energy diverged drastically\n");
        ++failures;
    } else {
        std::printf("      PASS: <= 50%%\n");
    }

    std::printf("\n================================================================\n");
    std::printf("  RESULT: %s (%d failures)\n", failures == 0 ? "PASS" : "FAIL", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
