/**
 * Phase B.3 protocol candidate: position-fixed mask persistence.
 *
 * Background. The exploratory Γ(T) scan
 * (test_cluster_gamma_t_exploratory.cpp) found that simple identity-tracking
 * fails at high T due to lattice flooding — spontaneous nucleation produces
 * hundreds of new clusters that obscure the original cluster's identity.
 *
 * This test sidesteps the identity-tracking problem entirely. After
 * injection and equilibration, it records the set of voxels manifested by
 * the original cluster (the "mask" M_0). For subsequent ticks at varying
 * T, it measures
 *
 *   persistence(t, T) = |voxels still manifested at tick t in M_0| / |M_0|
 *
 * If the original cluster's matter persists, persistence stays near 1.
 * If it dissolves, persistence drops. The exponential 1/e decay time
 * defines tau_persist(T) without requiring identity matching.
 *
 * Critical caveat: this measurement assumes the cluster does NOT translate
 * substantially during the run (which would bias persistence downward
 * for stationary masks). The FTD-0107 baseline showed clusters are largely
 * stationary at L=32 over 200 ticks, so this assumption holds for the
 * baseline regime; it must be re-checked at higher T.
 *
 * Pre-registration discipline: this is EXPLORATORY (single seed per T,
 * no statistical claim). A pre-registered campaign would seed-sample.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

struct DecayResult {
    double T;
    int initial_mask_size;
    std::vector<int> tick_samples;
    std::vector<double> persistence_at_tick;
    int tau_e_tick;             // tick at which persistence falls below 1/e
    double persistence_at_end;
};

static std::unordered_set<int> snapshot_mask(const ftd::RenderBridge& rb) {
    std::unordered_set<int> mask;
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state != 0) mask.insert(static_cast<int>(i));
    }
    return mask;
}

static double compute_persistence(const ftd::RenderBridge& rb,
                                   const std::unordered_set<int>& mask) {
    if (mask.empty()) return 0.0;
    const auto& vox = rb.voxels();
    int still_manifested = 0;
    for (int idx : mask) {
        if (vox[idx].state != 0) ++still_manifested;
    }
    return static_cast<double>(still_manifested) / mask.size();
}

static DecayResult run_one(double T, int L, int n_warmup, int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = 0.02;

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = 10.0 * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    // Warm up to allow cluster to nucleate and equilibrate.
    for (int t = 0; t < n_warmup; ++t) rb.tick();

    // Snapshot the position-fixed mask AFTER warmup.
    auto mask = snapshot_mask(rb);

    DecayResult r;
    r.T = T;
    r.initial_mask_size = static_cast<int>(mask.size());
    r.tau_e_tick = -1;
    r.persistence_at_end = 1.0;

    // Measure persistence over n_measure additional ticks.
    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample_interval == 0) {
            double p = compute_persistence(rb, mask);
            r.tick_samples.push_back(t);
            r.persistence_at_tick.push_back(p);
            if (r.tau_e_tick < 0 && p < std::exp(-1.0)) {
                r.tau_e_tick = t;
            }
        }
    }

    if (!r.persistence_at_tick.empty()) {
        r.persistence_at_end = r.persistence_at_tick.back();
    }
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 PROTOCOL: position-fixed mask persistence\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_MEASURE = 250;
    const int SAMPLE_INTERVAL = 10;
    std::vector<double> T_values = {0.005, 0.02, 0.05, 0.10, 0.20, 0.50, 1.00};

    std::cout << "Configuration: L=" << L
              << ", warmup=" << N_WARMUP
              << ", measure=" << N_MEASURE
              << ", sample every " << SAMPLE_INTERVAL << " ticks\n";
    std::cout << "Injection: A = 10*K_GENESIS = "
              << 10.0 * ftd::K_GENESIS << " at center\n";
    std::cout << "Mask: snapshot of all manifested voxels at end of warmup\n\n";

    std::vector<DecayResult> results;
    std::cout << "  T       mask_size   tau_e (tick)   persistence_at_end\n";
    std::cout << "  -----   ---------   ------------   ------------------\n";
    for (double T : T_values) {
        DecayResult r = run_one(T, L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL);
        results.push_back(r);
        std::cout << std::fixed << std::setprecision(3)
                  << "  " << std::setw(5) << r.T << "   "
                  << std::setw(9) << r.initial_mask_size << "   "
                  << std::setw(12) << (r.tau_e_tick < 0 ? std::string("> ") + std::to_string(N_MEASURE)
                                                         : std::to_string(r.tau_e_tick)) << "   "
                  << std::setw(18) << std::setprecision(4) << r.persistence_at_end << "\n";
    }

    // Detailed decay curves for representative T values
    std::cout << "\n--- Detailed decay curves ---\n";
    std::cout << "tick       ";
    for (const auto& r : results) {
        std::cout << "T=" << std::fixed << std::setprecision(3) << r.T << "    ";
    }
    std::cout << "\n";

    if (!results.empty()) {
        size_t n_samples = results[0].tick_samples.size();
        for (size_t i = 0; i < n_samples; ++i) {
            std::cout << std::setw(5) << results[0].tick_samples[i] << "      ";
            for (const auto& r : results) {
                std::cout << std::fixed << std::setprecision(3) << std::setw(7)
                          << r.persistence_at_tick[i] << "  ";
            }
            std::cout << "\n";
        }
    }

    // ----- Verdict -----
    std::cout << "\n--- Phase B.3 protocol verdict ---\n";

    // Did we observe a clean exponential decay regime?
    bool low_T_persists = (results.front().persistence_at_end > 0.5);
    bool high_T_decays = (results.back().tau_e_tick > 0 &&
                          results.back().tau_e_tick < N_MEASURE);
    bool monotone_tau_decrease = true;
    int prev_tau = 999999;
    for (const auto& r : results) {
        if (r.tau_e_tick < 0) continue;     // not yet decayed
        if (r.tau_e_tick > prev_tau + 5) { monotone_tau_decrease = false; break; }
        prev_tau = r.tau_e_tick;
    }

    std::cout << "  Low T persistence > 0.5 at end:        "
              << (low_T_persists ? "YES" : "no") << "\n";
    std::cout << "  High T 1/e decay within measurement:   "
              << (high_T_decays ? "YES" : "no") << "\n";
    std::cout << "  tau_e monotone decreasing with T:      "
              << (monotone_tau_decrease ? "YES" : "no") << "\n";

    std::cout << "\n  ";
    if (low_T_persists && high_T_decays && monotone_tau_decrease) {
        std::cout << "[VERDICT] Position-fixed mask protocol WORKS — clean tau_e(T) regime\n";
        std::cout << "  observed. Phase B.3 full pre-registered campaign can launch using\n";
        std::cout << "  this protocol. Recommended: multi-amplitude sweep (electron-id A=10,\n";
        std::cout << "  muon-id A=14, etc.) at T grid spanning low-T persistence to high-T\n";
        std::cout << "  decay, M=100 seeds per (T, A), pre-register protocol hash.\n";
    } else if (low_T_persists && !high_T_decays) {
        std::cout << "[VERDICT] Mask persistence stable up to T=" << T_values.back()
                  << " — need higher T or longer\n";
        std::cout << "  measurement window. Cluster matter is robust under tested perturbation.\n";
    } else {
        std::cout << "[VERDICT] Protocol indicates lattice flooding or measurement artifact —\n";
        std::cout << "  mask_size at warmup may be growing rather than fixed cluster.\n";
        std::cout << "  Inspect mask sizes vs T (high T mask >> low T mask suggests flooding\n";
        std::cout << "  even at warmup). Localized-Langevin engine modification may be needed.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (exploratory; no pre-registration; protocol-design test)\n";
    std::cout << "================================================================\n";
    return 0;
}
