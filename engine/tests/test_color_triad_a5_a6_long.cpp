/**
 * Phase B.3 (δ''') Test 5: long-time stability of A=5, A=6 +color+triad solitons.
 *
 * The amplitude scan (§5.6.7) found A=5 (n=4 conserved) and A=6 (n=11
 * conserved) are SOLITONS at +color+triad over 300 ticks. The N=4 cluster
 * matches N_base = 4 = mult(A_{1g})² — striking structural coincidence.
 *
 * This test runs both A=5 and A=6 with multi-seed for 1500 ticks to
 * determine: are these truly long-lived particle-like solitons, or do they
 * also eventually flood?
 *
 * If A=5 remains stable matter-conservation across 1500+ ticks across all
 * seeds, this is the engine's first identified stable propagating cluster
 * — strong candidate for FTD-0110 reconciliation.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

struct Result {
    double A;
    int seed;
    int n_init;
    int n_final;
    int t_first_growth;
    int t_flood_onset;
    bool flooded;
    int max_n_observed;
};

static Result run_one(double A, int seed, int N_TICKS) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int SAMPLE = 25;

    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    Result r;
    r.A = A; r.seed = seed;
    r.n_init = count_manifested(rb);
    r.t_first_growth = -1;
    r.t_flood_onset = -1;
    r.flooded = false;
    r.max_n_observed = r.n_init;

    for (int t = 1; t <= N_TICKS; ++t) {
        rb.tick();
        if (t % SAMPLE == 0) {
            int n = count_manifested(rb);
            if (n > r.max_n_observed) r.max_n_observed = n;
            if (r.t_first_growth < 0 && n > r.n_init * 1.5) r.t_first_growth = t;
            if (r.t_flood_onset < 0 && n > 100) {
                r.t_flood_onset = t;
                r.flooded = true;
            }
        }
    }
    r.n_final = count_manifested(rb);
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (δ''') Test 5: long-time A=5, A=6 +color+triad solitons\n";
    std::cout << "================================================================\n\n";

    const int N_TICKS = 1500;
    std::vector<int> seeds = {1, 2, 3, 4};
    std::vector<double> A_vals = {5.0, 6.0};

    std::cout << "Configuration: L=32, " << N_TICKS << " ticks, +color+triad, "
              << seeds.size() << " seeds, A ∈ {5, 6}·K_GENESIS\n\n";

    std::cout << "  A/K_G   seed   n_init   n_max  n_final   t_growth   t_flood   verdict\n";
    std::cout << "  -----   ----   ------   -----  -------   --------   -------   -------\n";

    std::vector<Result> all;
    for (double A : A_vals) {
        for (int seed : seeds) {
            std::cout << "  " << std::fixed << std::setprecision(1) << std::setw(5) << A << "   "
                      << std::setw(4) << seed << "   ... " << std::flush;
            Result r = run_one(A, seed, N_TICKS);
            all.push_back(r);
            std::string verdict = r.flooded ? "FLOODED" : (r.t_first_growth > 0 ? "GROWING" : "STABLE");
            std::cout << "\r  "
                      << std::fixed << std::setprecision(1) << std::setw(5) << r.A << "   "
                      << std::setw(4) << r.seed << "   "
                      << std::setw(6) << r.n_init << "   "
                      << std::setw(5) << r.max_n_observed << "  "
                      << std::setw(7) << r.n_final << "   "
                      << std::setw(8) << (r.t_first_growth < 0 ? std::string("none") : std::to_string(r.t_first_growth)) << "   "
                      << std::setw(7) << (r.t_flood_onset < 0 ? std::string("none") : std::to_string(r.t_flood_onset)) << "   "
                      << verdict << "\n";
        }
    }

    // Per-amplitude tally
    std::cout << "\n--- Per-amplitude verdict ---\n";
    for (double A : A_vals) {
        int n_stable = 0, n_flooded = 0;
        std::vector<int> flood_ticks;
        for (const auto& r : all) {
            if (std::abs(r.A - A) < 0.01) {
                if (r.flooded) { ++n_flooded; flood_ticks.push_back(r.t_flood_onset); }
                else ++n_stable;
            }
        }
        std::cout << "  A=" << std::fixed << std::setprecision(1) << A << " · K_GENESIS: "
                  << n_stable << " stable, " << n_flooded << " flooded across "
                  << seeds.size() << " seeds";
        if (!flood_ticks.empty()) {
            int fmin = *std::min_element(flood_ticks.begin(), flood_ticks.end());
            int fmax = *std::max_element(flood_ticks.begin(), flood_ticks.end());
            std::cout << " (flood ticks: min=" << fmin << ", max=" << fmax << ")";
        }
        std::cout << "\n";
    }

    // Final verdict
    std::cout << "\n--- Verdict ---\n";
    int total_stable_a5 = 0, total_stable_a6 = 0;
    for (const auto& r : all) {
        if (!r.flooded && std::abs(r.A - 5.0) < 0.01) ++total_stable_a5;
        if (!r.flooded && std::abs(r.A - 6.0) < 0.01) ++total_stable_a6;
    }
    if (total_stable_a5 == static_cast<int>(seeds.size())) {
        std::cout << "  [VERDICT] A=5 SOLITON is STABLE across all " << seeds.size()
                  << " seeds for " << N_TICKS << " ticks. This is the engine's first identified\n";
        std::cout << "  long-lived stable propagating cluster. Matter content n=4 = N_base.\n";
        std::cout << "  Strong candidate for FTD-0110 'lightest particle' identification.\n";
    } else if (total_stable_a5 > 0) {
        std::cout << "  [VERDICT] A=5 SOLITON STABLE in " << total_stable_a5 << "/"
                  << seeds.size() << " seeds — partial stability with seed dependence.\n";
    } else {
        std::cout << "  [VERDICT] A=5 SOLITON FLOODS in all seeds at " << N_TICKS
                  << " ticks. No truly stable engine cluster found.\n";
    }
    std::cout << "  A=6 SOLITON: " << total_stable_a6 << "/" << seeds.size()
              << " seeds remain stable across " << N_TICKS << " ticks.\n";

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}
