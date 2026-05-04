/**
 * Phase B.3 (δ''') Test 4: long-time evolution + flood-onset characterization.
 *
 * Multi-seed test (test_color_triad_a7_multiseed.cpp) showed that
 * +color+triad A=7 is QUASI-BOUND at 200 ticks but floods to ~30000 voxels
 * by 300 ticks, all 10/10 seeds. The "bound state" is actually a delayed
 * flood. This test characterizes WHEN the flood onset happens with high
 * temporal resolution.
 *
 * Per-seed: track n_total every 10 ticks for 1000 ticks. Identify the
 * tick at which n_total exceeds 100 (flood onset; >7× initial size).
 *
 * Multi-seed: is the flood-onset timing deterministic, or is there
 * stochastic variation across seeds? Stochastic timing → Poisson-like
 * decay; deterministic timing → fixed transient.
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

struct Trace {
    int seed;
    std::vector<int> ticks;
    std::vector<int> n_total;
    int t_first_growth;          // first tick where n > 1.5 * n_init
    int t_flood_onset;           // first tick where n > 100
    int t_full_flood;            // first tick where n > 1000
    int n_init;
};

static Trace run_one(int seed) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 1000;
    const int SAMPLE = 10;
    const double A = 7.0 * ftd::K_GENESIS;

    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    Trace tr;
    tr.seed = seed;
    tr.t_first_growth = -1;
    tr.t_flood_onset = -1;
    tr.t_full_flood = -1;
    tr.n_init = count_manifested(rb);
    tr.ticks.push_back(0);
    tr.n_total.push_back(tr.n_init);

    for (int t = 1; t <= N_TRACE; ++t) {
        rb.tick();
        if (t % SAMPLE == 0) {
            int n = count_manifested(rb);
            tr.ticks.push_back(t);
            tr.n_total.push_back(n);
            if (tr.t_first_growth < 0 && n > tr.n_init * 1.5)
                tr.t_first_growth = t;
            if (tr.t_flood_onset < 0 && n > 100)
                tr.t_flood_onset = t;
            if (tr.t_full_flood < 0 && n > 1000)
                tr.t_full_flood = t;
        }
    }
    return tr;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (δ''') Test 4: +color+triad A=7 flood-onset timing\n";
    std::cout << "================================================================\n\n";

    std::vector<int> seeds = {1, 2, 3, 4, 5};
    std::cout << "Configuration: L=32, A=7·K_GENESIS, +color+triad, 1000 ticks, "
              << seeds.size() << " seeds\n\n";

    std::vector<Trace> traces;
    for (int seed : seeds) {
        std::cout << "Seed " << seed << " ... " << std::flush;
        Trace tr = run_one(seed);
        traces.push_back(tr);
        std::cout << "n_init=" << tr.n_init
                  << ", t_first_growth=" << (tr.t_first_growth < 0 ? std::string("none") : std::to_string(tr.t_first_growth))
                  << ", t_flood_onset=" << (tr.t_flood_onset < 0 ? std::string("none") : std::to_string(tr.t_flood_onset))
                  << ", t_full_flood=" << (tr.t_full_flood < 0 ? std::string("none") : std::to_string(tr.t_full_flood))
                  << "\n";
    }

    // Combined timing summary
    std::cout << "\n--- Per-seed timing summary ---\n";
    std::cout << "  seed   n_init   t_first_growth   t_flood_onset   t_full_flood\n";
    std::cout << "  ----   ------   --------------   -------------   ------------\n";
    for (const auto& tr : traces) {
        std::cout << "  " << std::setw(4) << tr.seed << "   "
                  << std::setw(6) << tr.n_init << "   "
                  << std::setw(14) << (tr.t_first_growth < 0 ? std::string("none") : std::to_string(tr.t_first_growth))
                  << "   "
                  << std::setw(13) << (tr.t_flood_onset < 0 ? std::string("none") : std::to_string(tr.t_flood_onset))
                  << "   "
                  << std::setw(12) << (tr.t_full_flood < 0 ? std::string("none") : std::to_string(tr.t_full_flood))
                  << "\n";
    }

    // Stochasticity analysis
    std::vector<int> growth_ticks, flood_ticks;
    for (const auto& tr : traces) {
        if (tr.t_first_growth > 0) growth_ticks.push_back(tr.t_first_growth);
        if (tr.t_flood_onset > 0) flood_ticks.push_back(tr.t_flood_onset);
    }
    if (!growth_ticks.empty()) {
        int gmin = *std::min_element(growth_ticks.begin(), growth_ticks.end());
        int gmax = *std::max_element(growth_ticks.begin(), growth_ticks.end());
        double gmean = 0; for (int g : growth_ticks) gmean += g; gmean /= growth_ticks.size();
        std::cout << "\n  First-growth tick: min=" << gmin << ", max=" << gmax
                  << ", mean=" << std::fixed << std::setprecision(1) << gmean
                  << ", spread=" << (gmax - gmin) << " ticks\n";
    }
    if (!flood_ticks.empty()) {
        int fmin = *std::min_element(flood_ticks.begin(), flood_ticks.end());
        int fmax = *std::max_element(flood_ticks.begin(), flood_ticks.end());
        double fmean = 0; for (int g : flood_ticks) fmean += g; fmean /= flood_ticks.size();
        std::cout << "  Flood-onset tick: min=" << fmin << ", max=" << fmax
                  << ", mean=" << std::fixed << std::setprecision(1) << fmean
                  << ", spread=" << (fmax - fmin) << " ticks\n";
    }

    // Print one full trace for inspection
    std::cout << "\n--- Full trace for seed " << traces[0].seed << " ---\n";
    std::cout << "  tick    n_total\n";
    std::cout << "  ----    -------\n";
    for (size_t i = 0; i < traces[0].ticks.size(); i += 5) {
        std::cout << "  " << std::setw(4) << traces[0].ticks[i] << "    "
                  << std::setw(7) << traces[0].n_total[i] << "\n";
    }

    // Verdict
    std::cout << "\n--- Verdict ---\n";
    int n_flooded = 0;
    for (const auto& tr : traces) if (tr.t_full_flood > 0) ++n_flooded;
    if (n_flooded == static_cast<int>(seeds.size())) {
        std::cout << "  [VERDICT] All " << seeds.size()
                  << " seeds eventually flood. The +color+triad A=7 'bound state' is a\n";
        std::cout << "  TRANSIENT — quasi-bound for ~150-250 ticks then floods. NOT a true\n";
        std::cout << "  bound state. Phase B observable: 'binding lifetime' = time-to-flood.\n";
    } else {
        std::cout << "  [VERDICT] " << n_flooded << "/" << seeds.size()
                  << " seeds flood within 1000 ticks. Some seeds remain quasi-bound longer.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (5-seed long-time evolution)\n";
    std::cout << "================================================================\n";
    return 0;
}
