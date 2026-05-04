/**
 * Phase B.3 follow-up (α'): find the lightest stable cluster amplitude.
 *
 * Background: A=10·K_GENESIS dies (L-invariant); A=8 was stable at L=32 (1
 * seed, exploratory). This test refines the A < 10 range with 3 seeds per
 * amplitude at L=32 to identify the smallest A that produces consistent
 * stability across all seeds. The candidate "lightest stable engine
 * cluster" is then the basis for a candidate identification with the
 * lightest stable particle in the SM (electron, by mass scale).
 *
 * If the lightest stable A produces a cluster that — when scaled by
 * FTD-0110's N(A) ≈ ¼·(A/K_GENESIS)² — corresponds to a cluster size that
 * matches the electron's mass identification per FTD-0110, then the
 * "size-mass identification" and the "lightest-stable identification"
 * point at the same particle, restoring consistency in the FTD-0110
 * extension.
 *
 * If the lightest stable A is significantly different from FTD-0110's
 * electron amplitude (A=10·K_GENESIS), then the size-mass identification
 * and the dynamical-stability identification refer to different particles
 * — a substantive finding.
 *
 * L-invariance follow-up will spot-check the candidate at L=64.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

struct R {
    double A_over_KG;
    int seed;
    int N_obs;
    double p_end;
    int t_at_p_zero;
};

static std::unordered_set<int> snapshot_mask(const ftd::RenderBridge& rb) {
    std::unordered_set<int> mask;
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    for (int64_t i = 0; i < total; ++i)
        if (vox[i].state != 0) mask.insert(static_cast<int>(i));
    return mask;
}

static double compute_persistence(const ftd::RenderBridge& rb,
                                   const std::unordered_set<int>& mask) {
    if (mask.empty()) return 0.0;
    const auto& vox = rb.voxels();
    int n = 0;
    for (int idx : mask) if (vox[idx].state != 0) ++n;
    return static_cast<double>(n) / mask.size();
}

static R run_one(double A_over_KG, int seed, int L,
                  int n_warmup, int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    const int cx = L / 2, cy = L / 2, cz = L / 2;
    rb.inject_flux(cx, cy, cz, {A_over_KG * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    R r;
    r.A_over_KG = A_over_KG;
    r.seed = seed;
    r.N_obs = static_cast<int>(mask.size());
    r.p_end = 1.0;
    r.t_at_p_zero = -1;

    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample_interval == 0) {
            double p = compute_persistence(rb, mask);
            if (r.t_at_p_zero < 0 && p == 0.0) r.t_at_p_zero = t;
            r.p_end = p;
        }
    }
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (α'): lightest stable cluster amplitude (L=32, 3 seeds)\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_MEASURE = 500;
    const int SAMPLE_INTERVAL = 10;
    std::vector<double> A_values = {6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0};
    std::vector<int> seeds = {1, 2, 3};

    std::cout << "Configuration: L=" << L << ", warmup=" << N_WARMUP
              << ", measure=" << N_MEASURE << " ticks\n";
    std::cout << "Toggle: engine defaults\n\n";

    std::cout << "  A/K_G   seed   N_obs   t@p=0   p_end    verdict\n";
    std::cout << "  -----   ----   -----   -----   ------   -------\n";
    std::vector<R> all;
    for (double A : A_values) {
        for (int seed : seeds) {
            R r = run_one(A, seed, L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL);
            all.push_back(r);
            const char* v = (r.p_end < 0.3) ? "DEAD" :
                            (r.p_end < 0.6) ? "WEAK" :
                            (r.p_end < 0.85) ? "EQUILIB" : "STABLE";
            std::cout << std::fixed << std::setprecision(1)
                      << "  " << std::setw(5) << r.A_over_KG << "   "
                      << std::setw(4) << r.seed << "   "
                      << std::setw(5) << r.N_obs << "   "
                      << std::setw(5) << (r.t_at_p_zero < 0 ? std::string("--") : std::to_string(r.t_at_p_zero))
                      << "   "
                      << std::fixed << std::setprecision(4) << std::setw(6)
                      << r.p_end << "   " << v << "\n";
        }
    }

    // Aggregate by amplitude
    std::cout << "\n--- Per-amplitude summary ---\n";
    std::cout << "  A/K_G    n_dead   n_alive   pattern\n";
    std::cout << "  -----    ------   -------   -------\n";
    for (double A : A_values) {
        int n_dead = 0, n_alive = 0;
        for (const auto& r : all) {
            if (std::abs(r.A_over_KG - A) < 0.01) {
                if (r.p_end < 0.3) ++n_dead;
                else ++n_alive;
            }
        }
        std::string pattern;
        if (n_dead == static_cast<int>(seeds.size())) pattern = "ALL DEAD";
        else if (n_alive == static_cast<int>(seeds.size())) pattern = "ALL STABLE";
        else pattern = "MIXED";
        std::cout << std::fixed << std::setprecision(1)
                  << "  " << std::setw(5) << A << "    "
                  << std::setw(6) << n_dead << "   "
                  << std::setw(7) << n_alive << "   "
                  << pattern << "\n";
    }

    // Find lightest A where ALL seeds are stable
    double lightest_stable = -1.0;
    for (double A : A_values) {
        int n_dead = 0;
        for (const auto& r : all) {
            if (std::abs(r.A_over_KG - A) < 0.01 && r.p_end < 0.3) ++n_dead;
        }
        if (n_dead == 0) {
            lightest_stable = A;
            break;
        }
    }

    std::cout << "\n--- Verdict ---\n";
    if (lightest_stable > 0) {
        int N_pred = static_cast<int>(0.25 * lightest_stable * lightest_stable);
        std::cout << "  Lightest L=32 stable amplitude: A = "
                  << std::fixed << std::setprecision(1) << lightest_stable
                  << " · K_GENESIS\n";
        std::cout << "  FTD-0110 predicted cluster size at this A: " << N_pred << " voxels\n";
        std::cout << "  vs FTD-0110 'electron-identified' size (A=10): 25 voxels\n";
        std::cout << "  vs FTD-0110 'electron-identified' amplitude:    10 · K_GENESIS\n";
        std::cout << "\n  [VERDICT] The lightest L=32 stable amplitude is A = "
                  << std::fixed << std::setprecision(1) << lightest_stable
                  << " · K_GENESIS\n";
        std::cout << "  (NOT A=10 — the FTD-0110 'electron-identified' amplitude is unstable\n";
        std::cout << "  per the L-invariance check). NEXT: spot-check this candidate at L=64\n";
        std::cout << "  to confirm it's a real lightest-stable, not an L=32 artifact.\n";
    } else {
        std::cout << "  No amplitude in the tested range is stable at all 3 seeds.\n";
        std::cout << "  Either A < 6 also is too low, or the death valley extends below A=6.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (3-seed L=32 sweep)\n";
    std::cout << "================================================================\n";
    return 0;
}
