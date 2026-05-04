/**
 * Test: Cluster Persistence — toggle configuration sweep (B.2 diagnosis (b))
 *
 * The alpha-sweep diagnostic (test_cluster_persistence_alpha_sweep.cpp)
 * showed cluster lifetimes saturate at ~45 ticks even with relaxed alpha,
 * pointing at engine dynamics (not tracker) as the dominant decay mechanism.
 *
 * This test compares three toggle configurations to identify which is
 * needed for FTD-0107-style cluster persistence:
 *
 *   Config A — engine default (B.2 baseline; langevin OFF)
 *   Config B — FTD-0107 baseline (langevin ON at T=0.005, gamma=0.02)
 *   Config C — FTD-0107 baseline + triad_binding ON (additional stabilization candidate)
 *
 * If Config B persists clusters >> A, langevin IS the missing ingredient
 * for FTD-0107-canonical cluster behavior — diagnosis (b) confirmed.
 * Class B persistence protocol should adopt the FTD-0107 baseline as canonical.
 *
 * If Config C >> B, triad binding adds further stabilization — informative
 * for choosing the canonical Phase B.3 thermal-regime baseline.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/cluster_tracker.h"
#include "ftd/constants.h"

struct ConfigResult {
    const char* name;
    int total_tracked;
    int max_size;
    int max_lifetime;
    double mean_lifetime;
    int alive_at_end;
    int alive_max;
};

enum class Config { Default, Ftd0107Baseline, Ftd0107PlusTriad };

static void apply_config(ftd::RenderBridge& rb, Config cfg) {
    switch (cfg) {
        case Config::Default:
            // Engine defaults — wave, genesis, gauss already true; langevin off
            break;
        case Config::Ftd0107Baseline:
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.langevin         = true;
            rb.toggles.langevin_T       = 0.005;
            rb.toggles.langevin_gamma   = 0.02;
            break;
        case Config::Ftd0107PlusTriad:
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.langevin         = true;
            rb.toggles.langevin_T       = 0.005;
            rb.toggles.langevin_gamma   = 0.02;
            rb.toggles.triad_binding    = true;
            break;
    }
}

static ConfigResult run_one(const char* name, Config cfg, int L, int n_ticks, int record_interval) {
    ftd::ClusterTrackerParams params;
    params.use_moore_neighbors = true;
    params.min_cluster_size = 4;
    params.overlap_threshold = 0.1;   // Use relaxed alpha (per alpha-sweep saturation finding)

    ftd::RenderBridge rb(L);
    apply_config(rb, cfg);
    ftd::ClusterTracker tracker(params);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = 10.0 * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    tracker.record(rb);
    int alive_max = 0;
    for (int t = 1; t <= n_ticks; ++t) {
        rb.tick();
        if (t % record_interval == 0) {
            tracker.record(rb);
            int ac = tracker.alive_count();
            if (ac > alive_max) alive_max = ac;
        }
    }

    ConfigResult r;
    r.name = name;
    r.total_tracked = tracker.total_tracked();
    r.max_size = tracker.max_size_observed();
    auto lt = tracker.lifetime_distribution();
    r.max_lifetime = lt.empty() ? 0 : lt.back();
    r.mean_lifetime = tracker.mean_lifetime();
    r.alive_at_end = tracker.alive_count();
    r.alive_max = alive_max;
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Cluster Persistence — toggle sweep (B.2 diagnosis (b))\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_TICKS = 200;
    const int RECORD_INTERVAL = 5;

    std::cout << "Configuration: L=" << L << ", N_TICKS=" << N_TICKS
              << ", record_interval=" << RECORD_INTERVAL
              << ", alpha=0.10 (relaxed per alpha-sweep finding)\n";
    std::cout << "Injection: J_x = 10*K_GENESIS = "
              << 10.0 * ftd::K_GENESIS << " at (16,16,16)\n\n";

    std::vector<ConfigResult> results = {
        run_one("A: default (no Langevin)",  Config::Default,           L, N_TICKS, RECORD_INTERVAL),
        run_one("B: FTD-0107 baseline",      Config::Ftd0107Baseline,   L, N_TICKS, RECORD_INTERVAL),
        run_one("C: FTD-0107 + triad_bind",  Config::Ftd0107PlusTriad,  L, N_TICKS, RECORD_INTERVAL),
    };

    std::cout << "config                      tracked  max_size  max_lt  mean_lt  alive_max  alive_end\n";
    std::cout << "------                      -------  --------  ------  -------  ---------  ---------\n";
    for (const auto& r : results) {
        std::cout << std::left << std::setw(28) << r.name
                  << std::right
                  << std::setw(7) << r.total_tracked
                  << std::setw(10) << r.max_size
                  << std::setw(8) << r.max_lifetime
                  << std::setw(9) << std::fixed << std::setprecision(1) << r.mean_lifetime
                  << std::setw(11) << r.alive_max
                  << std::setw(11) << r.alive_at_end
                  << "\n";
    }

    // ----- Diagnosis -----
    std::cout << "\n--- Diagnosis (b) verdict ---\n";
    const auto& A = results[0];
    const auto& B = results[1];
    const auto& C = results[2];

    bool B_better_than_A = (B.max_lifetime > A.max_lifetime + 10) ||
                            (B.alive_at_end > A.alive_at_end);
    bool C_better_than_B = (C.max_lifetime > B.max_lifetime + 10) ||
                            (C.alive_at_end > B.alive_at_end);
    bool any_persists_to_end = (A.alive_at_end >= 1 || B.alive_at_end >= 1 ||
                                  C.alive_at_end >= 1);

    std::cout << "  B (FTD-0107 baseline) substantially better than A (default): "
              << (B_better_than_A ? "YES" : "no") << "\n";
    std::cout << "  C (FTD-0107 + triad) substantially better than B: "
              << (C_better_than_B ? "YES" : "no") << "\n";
    std::cout << "  Any config produces cluster alive at tick " << N_TICKS << ": "
              << (any_persists_to_end ? "YES" : "no") << "\n";

    std::cout << "\n  ";
    if (any_persists_to_end) {
        std::cout << "[VERDICT] Diagnosis (b) CONFIRMED — toggle configuration is the\n";
        std::cout << "  determining factor. The minimal config that yields persistent clusters\n";
        std::cout << "  should be adopted as the Class B canonical baseline. Phase B.3 thermal\n";
        std::cout << "  campaign cleared to proceed using the working config.\n";
    } else if (B_better_than_A) {
        std::cout << "[VERDICT] Diagnosis (b) PARTIALLY confirmed — Langevin does extend\n";
        std::cout << "  cluster lifetime but no config persists to N_TICKS=" << N_TICKS << ".\n";
        std::cout << "  Either lifetime is intrinsically finite under tested configs (likely\n";
        std::cout << "  needs longer N_TICKS or larger L), or additional stabilization toggle\n";
        std::cout << "  is needed beyond triad_binding. Move to diagnosis (c) (lattice size)\n";
        std::cout << "  or expand toggle search space.\n";
    } else {
        std::cout << "[VERDICT] Diagnosis (b) REJECTED — toggle config does not substantially\n";
        std::cout << "  affect cluster lifetime. Move to diagnosis (c) (lattice size, L=64+).\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (no assertions; this is a diagnostic measurement)\n";
    std::cout << "================================================================\n";
    return 0;
}
