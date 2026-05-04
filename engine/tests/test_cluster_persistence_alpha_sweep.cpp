/**
 * Test: Cluster Persistence — alpha sensitivity sweep (Phase B.2 diagnostic)
 *
 * FINDING B.2-B from test_cluster_persistence_quiescent: clusters nucleate
 * under FTD-0110-canonical injection but dissolve within ~16 ticks under
 * default-toggle quiescent dynamics with the SPEC-default tracker (alpha=0.5,
 * Moore-26).
 *
 * Diagnosis (a) candidate: tracker identity-criterion too strict for moving
 * clusters — overlap |C ∩ C'| / |C| drops below alpha as cluster translates,
 * even though the cluster persists physically.
 *
 * This test runs the SAME injection at the SAME L=32 lattice for the SAME
 * 200 ticks, sweeping alpha ∈ {0.50, 0.30, 0.10, 0.05}. The expected pattern
 * if diagnosis (a) is correct:
 *
 *   alpha=0.50 -> short mean lifetime (baseline, B.2 finding)
 *   alpha=0.30 -> longer mean lifetime
 *   alpha=0.10 -> much longer mean lifetime
 *   alpha=0.05 -> longest mean lifetime
 *
 * If lifetimes extend monotonically with decreasing alpha, diagnosis (a) is
 * confirmed. If lifetimes stay roughly constant, the cluster is genuinely
 * dissolving (diagnoses (b) or (c) are operative).
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/cluster_tracker.h"
#include "ftd/constants.h"

struct SweepResult {
    double alpha;
    int total_tracked;
    int max_size;
    int max_lifetime;
    double mean_lifetime;
    int alive_at_end;
};

static SweepResult run_one(double alpha, int L, int n_ticks, int record_interval) {
    ftd::ClusterTrackerParams params;
    params.use_moore_neighbors = true;
    params.min_cluster_size = 4;
    params.overlap_threshold = alpha;

    ftd::RenderBridge rb(L);
    ftd::ClusterTracker tracker(params);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = 10.0 * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    tracker.record(rb);
    for (int t = 1; t <= n_ticks; ++t) {
        rb.tick();
        if (t % record_interval == 0) tracker.record(rb);
    }

    SweepResult r;
    r.alpha = alpha;
    r.total_tracked = tracker.total_tracked();
    r.max_size = tracker.max_size_observed();
    auto lt = tracker.lifetime_distribution();
    r.max_lifetime = lt.empty() ? 0 : lt.back();
    r.mean_lifetime = tracker.mean_lifetime();
    r.alive_at_end = tracker.alive_count();
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Cluster Persistence — alpha sweep (B.2 diagnosis (a))\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_TICKS = 200;
    const int RECORD_INTERVAL = 5;          // finer than B.2 baseline (was 10)
    std::vector<double> alpha_values = {0.50, 0.30, 0.10, 0.05};

    std::cout << "Configuration: L=" << L << ", N_TICKS=" << N_TICKS
              << ", record_interval=" << RECORD_INTERVAL
              << ", Moore-26, N_min=4\n";
    std::cout << "Injection: J_x = 10*K_GENESIS = "
              << 10.0 * ftd::K_GENESIS << " at (16,16,16)\n\n";

    std::cout << "alpha   total_tracked   max_size   max_lifetime   mean_lifetime   alive_at_end\n";
    std::cout << "-----   -------------   --------   ------------   -------------   ------------\n";

    std::vector<SweepResult> results;
    for (double a : alpha_values) {
        SweepResult r = run_one(a, L, N_TICKS, RECORD_INTERVAL);
        results.push_back(r);
        std::cout << std::fixed << std::setprecision(2)
                  << std::setw(5) << r.alpha << "   "
                  << std::setw(13) << r.total_tracked << "   "
                  << std::setw(8) << r.max_size << "   "
                  << std::setw(12) << r.max_lifetime << "   "
                  << std::setw(13) << std::setprecision(2) << r.mean_lifetime << "   "
                  << std::setw(12) << r.alive_at_end << "\n";
    }

    // ----- Diagnosis -----
    std::cout << "\n--- Diagnosis (a) verdict ---\n";

    // Check monotonic decrease in total_tracked (fewer "deaths" with smaller alpha
    // → fewer fragmenting transitions counted as new births)
    bool total_monotone_dec = true;
    for (size_t i = 1; i < results.size(); ++i) {
        if (results[i].total_tracked > results[i-1].total_tracked) {
            total_monotone_dec = false;
            break;
        }
    }

    // Check monotonic increase in max_lifetime
    bool max_lt_monotone_inc = true;
    for (size_t i = 1; i < results.size(); ++i) {
        if (results[i].max_lifetime < results[i-1].max_lifetime) {
            max_lt_monotone_inc = false;
            break;
        }
    }

    // Check that smallest-alpha run produces a non-zero alive_at_end (cluster
    // persists to end of run with relaxed criterion)
    bool persists_with_relaxed_alpha = (results.back().alive_at_end >= 1);

    std::cout << "  total_tracked monotone decreasing with alpha: "
              << (total_monotone_dec ? "YES" : "no") << "\n";
    std::cout << "  max_lifetime monotone increasing with alpha decrease: "
              << (max_lt_monotone_inc ? "YES" : "no") << "\n";
    std::cout << "  cluster alive at end of run (alpha=" << alpha_values.back() << "): "
              << (persists_with_relaxed_alpha ? "YES" : "no") << "\n";

    // Verdict
    std::cout << "\n  ";
    if (max_lt_monotone_inc && persists_with_relaxed_alpha) {
        std::cout << "[VERDICT] Diagnosis (a) CONFIRMED — relaxing alpha extends lifetime;\n";
        std::cout << "  cluster persists physically with relaxed criterion. Tracker is the\n";
        std::cout << "  bottleneck, not the engine. Recommended fix: implement centroid-tracking\n";
        std::cout << "  in ClusterTracker (next concrete deliverable).\n";
    } else if (max_lt_monotone_inc && !persists_with_relaxed_alpha) {
        std::cout << "[VERDICT] Diagnosis (a) PARTIALLY confirmed — lifetime extends with\n";
        std::cout << "  smaller alpha, but cluster still dies before end of run. Tracker is\n";
        std::cout << "  partially responsible; engine dissipation may also contribute.\n";
        std::cout << "  Next: try centroid-tracking + investigate engine toggles.\n";
    } else {
        std::cout << "[VERDICT] Diagnosis (a) REJECTED — alpha sensitivity does not extend\n";
        std::cout << "  lifetime monotonically. Cluster is genuinely dissolving in the engine.\n";
        std::cout << "  Move to diagnosis (b): engine toggle configuration, or (c): lattice size.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (no assertions; this is a diagnostic measurement)\n";
    std::cout << "================================================================\n";
    return 0;
}
