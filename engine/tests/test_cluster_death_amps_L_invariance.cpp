/**
 * Phase B.3 follow-up: L-invariance of death amplitudes A=10, 11.
 *
 * The stability-threshold sweep (test_cluster_stability_threshold) at L=32
 * found DETERMINISTIC cluster death at A ∈ {10, 11}·K_GENESIS, with
 * stable persistence at neighboring amplitudes (A=8, 10.5, 11.5, 12+).
 * The death is seed-independent (zero stochasticity at death amplitudes).
 *
 * Two candidate explanations:
 *   (i)  L=32 lattice resonance — death-amps would shift with L
 *   (ii) Scale-invariant K_GENESIS resonance — death-amps stay at integer
 *        multiples of K_GENESIS regardless of L
 *
 * This test runs the same protocol at L=64 for the death amplitudes (10, 11)
 * plus controls (8, 12) and 2 seeds each. If death persists at A=10, 11 at
 * L=64, the resonance is scale-invariant. If it shifts, it's an L=32 artifact.
 *
 * Cost: 4 amplitudes × 2 seeds × ~1000 ticks at L=64 — moderate compute time.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

struct Result {
    double A_over_KG;
    int L;
    int seed;
    int initial_mask_size;
    double persistence_at_end;
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

static Result run_one(double A_over_KG, int L, int seed,
                       int n_warmup, int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = A_over_KG * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    Result r;
    r.A_over_KG = A_over_KG;
    r.L = L;
    r.seed = seed;
    r.initial_mask_size = static_cast<int>(mask.size());
    r.persistence_at_end = 1.0;
    r.t_at_p_zero = -1;

    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample_interval == 0) {
            double p = compute_persistence(rb, mask);
            if (r.t_at_p_zero < 0 && p == 0.0) r.t_at_p_zero = t;
            r.persistence_at_end = p;
        }
    }
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  Phase B.3: death-amp L-invariance check (L=64 vs L=32)\n";
    std::cout << "================================================================\n\n";

    const int L_BIG = 64;
    const int N_WARMUP = 50;
    const int N_MEASURE = 500;
    const int SAMPLE_INTERVAL = 10;

    std::vector<double> A_values = {8.0, 10.0, 11.0, 12.0};
    std::vector<int> seeds = {1, 2};

    std::cout << "Configuration: L=" << L_BIG
              << ", warmup=" << N_WARMUP
              << ", measure=" << N_MEASURE << " ticks\n";
    std::cout << "Toggle config: engine defaults\n\n";

    std::cout << "  L     A/K_G    seed   N_obs    t@p=0      p_at_end   verdict\n";
    std::cout << "  ---   -----    ----   -----    -------    --------   -------\n";

    std::vector<Result> all;
    for (double A : A_values) {
        for (int seed : seeds) {
            Result r = run_one(A, L_BIG, seed, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL);
            all.push_back(r);
            const char* verdict = (r.persistence_at_end < 0.3) ? "DEAD" :
                                   (r.persistence_at_end < 0.6) ? "WEAK" :
                                   (r.persistence_at_end < 0.85) ? "EQUILIB" : "STABLE";
            std::cout << "  " << r.L << "    "
                      << std::fixed << std::setprecision(1) << std::setw(5) << r.A_over_KG << "    "
                      << std::setw(4) << r.seed << "   "
                      << std::setw(5) << r.initial_mask_size << "    "
                      << std::setw(7) << (r.t_at_p_zero < 0
                           ? std::string("--")
                           : std::to_string(r.t_at_p_zero)) << "    "
                      << std::fixed << std::setprecision(4) << std::setw(8)
                      << r.persistence_at_end << "   " << verdict << "\n";
        }
    }

    // Verdict: do A=10, 11 still die at L=64?
    std::cout << "\n--- L-invariance verdict ---\n";
    bool A10_dead_at_L64 = true;
    bool A11_dead_at_L64 = true;
    for (const auto& r : all) {
        if (std::abs(r.A_over_KG - 10.0) < 0.01 && r.persistence_at_end >= 0.3) A10_dead_at_L64 = false;
        if (std::abs(r.A_over_KG - 11.0) < 0.01 && r.persistence_at_end >= 0.3) A11_dead_at_L64 = false;
    }

    std::cout << "  L=32 reference (from prior test): A=10 DEAD, A=11 DEAD at all seeds\n";
    std::cout << "  L=64: A=10 dead at all seeds: " << (A10_dead_at_L64 ? "YES" : "no") << "\n";
    std::cout << "  L=64: A=11 dead at all seeds: " << (A11_dead_at_L64 ? "YES" : "no") << "\n";

    std::cout << "\n  ";
    if (A10_dead_at_L64 && A11_dead_at_L64) {
        std::cout << "[VERDICT] Death amplitudes A=10, A=11 are L-INVARIANT.\n";
        std::cout << "  Death pattern is scale-invariant: it's a property of the K_GENESIS\n";
        std::cout << "  scale itself, not an L=32 boundary effect. Candidate mechanism:\n";
        std::cout << "  genesis rule has a deterministic resonance at integer multiples of\n";
        std::cout << "  K_GENESIS that produces specific cluster geometries triggering\n";
        std::cout << "  weak_transmutation cascade collapse. This is engine physics, not\n";
        std::cout << "  finite-size artifact.\n";
    } else if (!A10_dead_at_L64 && !A11_dead_at_L64) {
        std::cout << "[VERDICT] Death amplitudes A=10, A=11 are L=32-SPECIFIC.\n";
        std::cout << "  At L=64 these amplitudes are stable. The L=32 finding is a finite-\n";
        std::cout << "  size lattice resonance. The 'three-regime' picture is L-dependent\n";
        std::cout << "  and needs to be rebuilt at L=64+ for canonical Phase B.3 measurement.\n";
    } else {
        std::cout << "[VERDICT] PARTIAL L-invariance — one of A=10/A=11 shifted with L.\n";
        std::cout << "  Death amplitudes are partially scale-invariant; mechanism unclear.\n";
        std::cout << "  Need finer L-scan and/or longer runs to characterize.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (2-seed L=64 spot check)\n";
    std::cout << "================================================================\n";
    return 0;
}
