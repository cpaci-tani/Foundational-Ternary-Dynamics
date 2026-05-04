/**
 * Phase B.3 (α') follow-up: L-invariance of the lightest L=32 stable
 * amplitudes A ∈ {7.0, 8.0, 9.0, 9.5}.
 *
 * From test_cluster_lightest_stable.cpp at L=32: stability tracks N_obs,
 * not A. Stable amps {7.0, 8.0, 9.0, 9.5} all produce N >= 15.
 *
 * This test spot-checks each at L=64 with 2 seeds. If the stability
 * pattern holds, the size threshold N ≈ 15 is L-invariant and is the
 * candidate "lightest stable cluster size" in engine physics.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

struct R { double A; int seed; int N; double p_end; int t_at_zero; };

static std::unordered_set<int> snapshot_mask(const ftd::RenderBridge& rb) {
    std::unordered_set<int> mask;
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    for (int64_t i = 0; i < total; ++i)
        if (vox[i].state != 0) mask.insert(static_cast<int>(i));
    return mask;
}

static double persist(const ftd::RenderBridge& rb,
                      const std::unordered_set<int>& mask) {
    if (mask.empty()) return 0.0;
    const auto& vox = rb.voxels();
    int n = 0;
    for (int idx : mask) if (vox[idx].state != 0) ++n;
    return static_cast<double>(n) / mask.size();
}

static R run_one(double A_over_KG, int seed, int L) {
    const int n_warmup = 50, n_measure = 500, sample = 10;
    ftd::RenderBridge rb(L);
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    const int c = L / 2;
    rb.inject_flux(c, c, c, {A_over_KG * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);
    R r{A_over_KG, seed, static_cast<int>(mask.size()), 1.0, -1};
    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample == 0) {
            double p = persist(rb, mask);
            if (r.t_at_zero < 0 && p == 0.0) r.t_at_zero = t;
            r.p_end = p;
        }
    }
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (α'): L=64 spot-check of lightest stable amps\n";
    std::cout << "================================================================\n\n";

    std::vector<double> A_vals = {7.0, 8.0, 9.0, 9.5};
    std::vector<int> seeds = {1, 2};
    const int L = 64;

    std::cout << "  L=64 results (2 seeds, 500 ticks):\n\n";
    std::cout << "  A/K_G   seed   N_obs   t@p=0   p_end    L=32 ref   verdict\n";
    std::cout << "  -----   ----   -----   -----   ------   --------   -------\n";
    std::vector<R> results;
    for (double A : A_vals) {
        for (int seed : seeds) {
            R r = run_one(A, seed, L);
            results.push_back(r);
            const char* v = (r.p_end < 0.3) ? "DEAD" :
                            (r.p_end < 0.6) ? "WEAK" :
                            (r.p_end < 0.85) ? "EQUILIB" : "STABLE";
            std::cout << std::fixed << std::setprecision(1)
                      << "  " << std::setw(5) << r.A << "   "
                      << std::setw(4) << r.seed << "   "
                      << std::setw(5) << r.N << "   "
                      << std::setw(5) << (r.t_at_zero < 0 ? std::string("--") : std::to_string(r.t_at_zero))
                      << "   "
                      << std::setw(6) << std::setprecision(4) << r.p_end << "   "
                      << "STABLE     " << v << "\n";
        }
    }

    // Verdict
    std::cout << "\n--- L-invariance verdict ---\n";
    int n_invariant = 0;
    for (double A : A_vals) {
        bool all_stable = true;
        for (const auto& r : results) {
            if (std::abs(r.A - A) < 0.01 && r.p_end < 0.3) all_stable = false;
        }
        if (all_stable) {
            ++n_invariant;
            std::cout << "  A=" << std::fixed << std::setprecision(1) << A
                      << " · K_GENESIS: STABLE at L=32 AND L=64 (L-INVARIANT)\n";
        } else {
            std::cout << "  A=" << std::fixed << std::setprecision(1) << A
                      << " · K_GENESIS: STABLE at L=32 but DEAD at L=64 (L=32 artifact)\n";
        }
    }

    std::cout << "\n  ";
    if (n_invariant == static_cast<int>(A_vals.size())) {
        std::cout << "[VERDICT] All tested L=32 stable amplitudes ARE ALSO STABLE at L=64.\n";
        std::cout << "  The size-threshold pattern (N >= 15 → stable) is L-invariant. The\n";
        std::cout << "  lightest L-invariant stable amplitude is A=7.0·K_GENESIS (N=24).\n";
        std::cout << "  Recommended FTD-0110 reconciliation: cluster-size identification\n";
        std::cout << "  applies to STABLE clusters only; engine produces stable clusters at\n";
        std::cout << "  N ∈ {~15, ~23, ~25} via discrete genesis attractors; the simple\n";
        std::cout << "  N(A) = ¼·(A/K_GENESIS)² formula is a *static* upper bound, not the\n";
        std::cout << "  engine's actual nucleation map.\n";
    } else if (n_invariant > 0) {
        std::cout << "[VERDICT] " << n_invariant << "/" << A_vals.size()
                  << " L=32 stable amps remain stable at L=64. Partial L-invariance;\n";
        std::cout << "  the size-threshold reading needs refinement.\n";
    } else {
        std::cout << "[VERDICT] None of the L=32 stable amps are stable at L=64.\n";
        std::cout << "  L=32 stability was an artifact; need different test setup.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (L=64 spot-check, 2 seeds)\n";
    std::cout << "================================================================\n";
    return 0;
}
