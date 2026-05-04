/**
 * Exploratory: Cluster Γ(T) scan — does an exponential decay regime exist?
 *
 * Phase B.3 needs to verify that elevated-T runs produce exponential decay
 * before a full pre-registered campaign is worth running. This test is
 * EXPLORATORY (single seed per T, no pre-registration, no statistical claim).
 *
 * Sweeps Langevin T over {0.005 (baseline), 0.02, 0.05, 0.10, 0.20} keeping
 * gamma fixed at the FTD-0107 value (0.02). Reports mean cluster lifetime
 * and alive-at-end for each T.
 *
 * Expected if a Γ(T) regime exists:
 *   T=0.005 -> alive at end (baseline; no decay)
 *   T=0.02  -> some decay; long mean lifetime
 *   T=0.05  -> moderate decay
 *   T=0.10  -> substantial decay
 *   T=0.20  -> rapid decay (cluster dies quickly)
 *
 * If lifetimes scale as ~exp(-Δ/T) for some Δ, the decay is Boltzmann-like
 * (Arrhenius regime). If lifetimes scale as power-law in T, decay is
 * non-Boltzmann. Either is informative.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/cluster_tracker.h"
#include "ftd/constants.h"

struct GammaResult {
    double T;
    int total_tracked;
    int max_size;
    double mean_lifetime;
    int max_lifetime;
    int alive_at_end;
};

static GammaResult run_one(double T, int L, int n_ticks, int record_interval) {
    ftd::ClusterTrackerParams params;
    params.use_moore_neighbors = true;
    params.min_cluster_size = 4;
    params.overlap_threshold = 0.10;

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = 0.02;

    ftd::ClusterTracker tracker(params);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = 10.0 * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    tracker.record(rb);
    for (int t = 1; t <= n_ticks; ++t) {
        rb.tick();
        if (t % record_interval == 0) tracker.record(rb);
    }

    GammaResult r;
    r.T = T;
    r.total_tracked = tracker.total_tracked();
    r.max_size = tracker.max_size_observed();
    r.mean_lifetime = tracker.mean_lifetime();
    auto lt = tracker.lifetime_distribution();
    r.max_lifetime = lt.empty() ? 0 : lt.back();
    r.alive_at_end = tracker.alive_count();
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  EXPLORATORY: Cluster Γ(T) scan — does decay regime exist?\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_TICKS = 200;
    const int RECORD_INTERVAL = 5;
    std::vector<double> T_values = {0.005, 0.02, 0.05, 0.10, 0.20, 0.50};

    std::cout << "Configuration: L=" << L << ", N_TICKS=" << N_TICKS
              << ", FTD-0107 baseline + variable T, alpha=0.10\n";
    std::cout << "Injection: A = 10*K_GENESIS = "
              << 10.0 * ftd::K_GENESIS << "\n\n";

    std::cout << "  T      tracked   max_size   max_lt   mean_lt   alive_end   1/mean (Γ proxy)\n";
    std::cout << "  -----  -------   --------   ------   -------   ---------   ----------------\n";

    std::vector<GammaResult> results;
    for (double T : T_values) {
        GammaResult r = run_one(T, L, N_TICKS, RECORD_INTERVAL);
        results.push_back(r);
        double gamma_proxy = (r.mean_lifetime > 0) ? 1.0 / r.mean_lifetime : 0.0;
        std::cout << std::fixed << std::setprecision(3)
                  << "  " << std::setw(5) << r.T << "  "
                  << std::setw(7) << r.total_tracked << "   "
                  << std::setw(8) << r.max_size << "   "
                  << std::setw(6) << r.max_lifetime << "   "
                  << std::setw(7) << std::setprecision(2) << r.mean_lifetime << "   "
                  << std::setw(9) << r.alive_at_end << "   "
                  << std::setw(16) << std::scientific << std::setprecision(3) << gamma_proxy
                  << std::fixed << "\n";
    }

    // ----- Diagnostic on regime -----
    std::cout << "\n--- Regime diagnostic ---\n";
    bool baseline_persists = (results.front().alive_at_end >= 1);
    bool top_T_decays = (results.back().alive_at_end == 0 && results.back().mean_lifetime > 0);

    std::cout << "  Baseline (T=" << T_values.front() << ") persists to end:    "
              << (baseline_persists ? "YES" : "no") << "\n";
    std::cout << "  Highest-T (T=" << T_values.back() << ") decays:             "
              << (top_T_decays ? "YES" : "no") << "\n";

    // Does mean_lifetime decrease with T (cluster destabilized by thermal noise)?
    bool monotone_decreasing = true;
    for (size_t i = 1; i < results.size(); ++i) {
        // Skip if either is alive-at-end (mean_lifetime undefined for alive clusters)
        if (results[i].mean_lifetime == 0 || results[i-1].mean_lifetime == 0) continue;
        if (results[i].mean_lifetime > results[i-1].mean_lifetime + 5.0) {
            monotone_decreasing = false;
            break;
        }
    }
    std::cout << "  mean_lifetime decreases (mostly) with T:        "
              << (monotone_decreasing ? "YES" : "no") << "\n";

    std::cout << "\n  ";
    if (baseline_persists && top_T_decays) {
        std::cout << "[VERDICT] Γ(T) decay regime EXISTS. Phase B.3 full pre-registered\n";
        std::cout << "  thermal campaign is justified. Recommended T grid: log-spaced from\n";
        std::cout << "  the lowest T that produces decay to the highest T at which clusters\n";
        std::cout << "  still nucleate before dissolving. Multi-amplitude sweep (electron-id,\n";
        std::cout << "  muon-id, ...) needed for ratio-based PDG comparison.\n";
    } else if (baseline_persists && !top_T_decays) {
        std::cout << "[VERDICT] Higher T may be needed — even T=" << T_values.back()
                  << " did not produce cluster decay.\n";
        std::cout << "  Either Langevin coupling at fixed gamma is too weak to destabilize\n";
        std::cout << "  the cluster within tested T range, or N_TICKS=" << N_TICKS << " is\n";
        std::cout << "  insufficient. Try T={1.0, 2.0, 5.0} or longer runs.\n";
    } else if (!baseline_persists) {
        std::cout << "[VERDICT] Baseline does not persist — toggle config or amplitude\n";
        std::cout << "  may be wrong. Re-verify FTD-0107 reproduction first.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (exploratory; no pre-registration; no statistical claim)\n";
    std::cout << "================================================================\n";
    return 0;
}
