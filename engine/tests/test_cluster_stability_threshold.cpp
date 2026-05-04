/**
 * Phase B.3 follow-up: identify A_min(stable) — the cluster stability threshold.
 *
 * The first τ_e(A) sweep (test_cluster_tau_amplitude_sweep.cpp) found three
 * regimes:
 *   - Sub-critical (A ≤ 10·K_GENESIS): full mask dissolution
 *   - Equilibration (A ∈ [14, 30]·K_GENESIS): dip then stabilize at ~85-90%
 *   - Robust (A ≥ 42·K_GENESIS): brief dip then stable ~85-95%
 *
 * The transition between sub-critical and equilibration regimes lies in
 * A ∈ (10, 14)·K_GENESIS. This test refines the grid in that range plus
 * a sanity check at A=8 (should be sub-critical), and runs THREE seeds per
 * amplitude near the transition to check whether the threshold is sharp
 * (deterministic boundary) or stochastic (probabilistic boundary depending
 * on Langevin/RNG noise).
 *
 * Output: mask trajectory + persistence_at_end + verdict per (A, seed).
 * The threshold A_min(stable) is the smallest A where most seeds produce
 * persistence_at_end > 0.5 at end of run.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

struct ThresholdResult {
    double A_over_KG;
    int seed;
    int initial_mask_size;
    double persistence_at_end;
    int t_at_decay_completion;     // tick at which persistence first hits 0; -1 if never
    int t_at_e_inv;                // tick at e^-1 first crossing; -1 if never
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

static ThresholdResult run_one(double A_over_KG, int seed, int L,
                                int n_warmup, int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    // Engine defaults — Phase B.3 canonical config.
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = A_over_KG * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    ThresholdResult r;
    r.A_over_KG = A_over_KG;
    r.seed = seed;
    r.initial_mask_size = static_cast<int>(mask.size());
    r.persistence_at_end = 1.0;
    r.t_at_decay_completion = -1;
    r.t_at_e_inv = -1;

    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample_interval == 0) {
            double p = compute_persistence(rb, mask);
            if (r.t_at_e_inv < 0 && p < std::exp(-1.0)) r.t_at_e_inv = t;
            if (r.t_at_decay_completion < 0 && p == 0.0) r.t_at_decay_completion = t;
            r.persistence_at_end = p;
        }
    }
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3: stability threshold A_min(stable)\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_MEASURE = 500;
    const int SAMPLE_INTERVAL = 5;

    // Coarse sanity (1 seed) + fine transition grid (3 seeds) + sanity high (1 seed)
    std::vector<double> A_values = {8.0, 10.0, 10.5, 11.0, 11.5, 12.0, 12.5, 13.0, 13.5, 14.0, 16.0};
    std::vector<int> seeds_to_try = {1, 2, 3};

    std::cout << "Configuration: L=" << L
              << ", warmup=" << N_WARMUP
              << ", measure=" << N_MEASURE << " ticks, sample every "
              << SAMPLE_INTERVAL << "\n";
    std::cout << "Toggle config: engine defaults (Phase B.3 canonical)\n";
    std::cout << "Seeds per A: " << seeds_to_try.size() << " (stochastic transition check)\n\n";

    std::cout << "  A/K_G    seed   N_obs    t@e^-1   t@p=0      p_at_end   verdict\n";
    std::cout << "  -----    ----   -----    ------   -------    --------   -------\n";

    std::vector<ThresholdResult> all;
    for (double A : A_values) {
        for (int seed : seeds_to_try) {
            ThresholdResult r = run_one(A, seed, L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL);
            all.push_back(r);

            const char* verdict;
            if (r.persistence_at_end == 0.0) verdict = "DEAD";
            else if (r.persistence_at_end < 0.3) verdict = "DYING";
            else if (r.persistence_at_end < 0.6) verdict = "WEAK";
            else if (r.persistence_at_end < 0.85) verdict = "EQUILIB";
            else verdict = "STABLE";

            std::cout << std::fixed << std::setprecision(1)
                      << "  " << std::setw(5) << r.A_over_KG << "    "
                      << std::setw(4) << r.seed << "   "
                      << std::setw(5) << r.initial_mask_size << "    "
                      << std::setw(6) << (r.t_at_e_inv < 0
                           ? std::string("--")
                           : std::to_string(r.t_at_e_inv)) << "   "
                      << std::setw(7) << (r.t_at_decay_completion < 0
                           ? std::string("--")
                           : std::to_string(r.t_at_decay_completion)) << "    "
                      << std::fixed << std::setprecision(4) << std::setw(8)
                      << r.persistence_at_end << "   " << verdict << "\n";
        }
    }

    // ----- Determine stability pattern (not just threshold; can be non-monotonic) -----
    std::cout << "\n--- Stability pattern analysis ---\n";

    // Categorize each amplitude as STABLE / DEAD / MIXED
    std::vector<std::pair<double, std::string>> patterns;
    for (double A : A_values) {
        int n_dead = 0, n_alive = 0;
        for (const auto& r : all) {
            if (std::abs(r.A_over_KG - A) < 0.01) {
                if (r.persistence_at_end < 0.3) ++n_dead;
                else ++n_alive;
            }
        }
        std::string p;
        if (n_dead == static_cast<int>(seeds_to_try.size())) p = "DEAD (all seeds)";
        else if (n_alive == static_cast<int>(seeds_to_try.size())) p = "STABLE (all seeds)";
        else p = "MIXED (" + std::to_string(n_dead) + " dead, " + std::to_string(n_alive) + " alive)";
        patterns.push_back({A, p});
    }
    for (const auto& [A, p] : patterns) {
        std::cout << "  A = " << std::fixed << std::setprecision(1) << A
                  << " · K_GENESIS  → " << p << "\n";
    }

    // Find death amplitudes (all-seed dead)
    std::vector<double> death_amps;
    for (double A : A_values) {
        int n_dead = 0, n_total = 0;
        for (const auto& r : all) {
            if (std::abs(r.A_over_KG - A) < 0.01) {
                ++n_total;
                if (r.persistence_at_end < 0.3) ++n_dead;
            }
        }
        if (n_dead == n_total && n_total > 0) death_amps.push_back(A);
    }

    bool monotonic_threshold = (death_amps.size() <= 1);
    double A_min_stable = -1.0;
    if (monotonic_threshold) {
        // True threshold case
        for (double A : A_values) {
            int n_dead = 0;
            for (const auto& r : all) {
                if (std::abs(r.A_over_KG - A) < 0.01 && r.persistence_at_end < 0.3) ++n_dead;
            }
            if (n_dead == 0) { A_min_stable = A; break; }
        }
    }

    std::cout << "\n--- Verdict ---\n";
    if (monotonic_threshold && A_min_stable > 0) {
        std::cout << "  [VERDICT] Monotonic stability threshold IDENTIFIED at A = "
                  << std::fixed << std::setprecision(1) << A_min_stable
                  << " · K_GENESIS.\n";
    } else if (!death_amps.empty()) {
        std::cout << "  [VERDICT] NON-MONOTONIC stability — multiple death amplitudes found:\n";
        std::cout << "    Death amplitudes: ";
        for (size_t i = 0; i < death_amps.size(); ++i) {
            std::cout << std::fixed << std::setprecision(1) << death_amps[i];
            if (i + 1 < death_amps.size()) std::cout << ", ";
        }
        std::cout << " · K_GENESIS\n";
        std::cout << "    These amplitudes produce DETERMINISTIC cluster death across all seeds,\n";
        std::cout << "    while neighboring (non-listed) amplitudes produce stable clusters.\n";
        std::cout << "    Pattern reminiscent of nuclear-physics 'magic number' instabilities.\n";
        std::cout << "    Mechanism candidates: (i) genesis-rule resonance at integer K_GENESIS\n";
        std::cout << "    multiples; (ii) cluster-geometry quantization at L=32; (iii) something\n";
        std::cout << "    structural about the engine's discretized injection. Disambiguation\n";
        std::cout << "    requires L-invariance check (run at L=64 to see if death-amps shift).\n";
    } else {
        std::cout << "  [VERDICT] No clear stability pattern — extend grid or revise criterion.\n";
    }

    // Stochasticity check (independent of pattern type)
    std::cout << "\n--- Stochasticity check ---\n";
    for (double A : A_values) {
        std::vector<double> p_vals;
        for (const auto& r : all) {
            if (std::abs(r.A_over_KG - A) < 0.01) p_vals.push_back(r.persistence_at_end);
        }
        if (p_vals.empty()) continue;
        double pmin = *std::min_element(p_vals.begin(), p_vals.end());
        double pmax = *std::max_element(p_vals.begin(), p_vals.end());
        if (pmax - pmin < 0.001) {
            std::cout << "  A=" << std::fixed << std::setprecision(1) << A
                      << ": all seeds identical p_end = " << std::setprecision(4) << pmin
                      << " → DETERMINISTIC at this amplitude\n";
        }
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (3-seed; pre-registration deferred)\n";
    std::cout << "================================================================\n";
    return 0;
}
