/**
 * Phase B.3 protocol candidate: cooling-induced evaporation.
 *
 * Critical finding from test_cluster_mask_persistence (2026-05-04): the
 * engine's evaporation rule is `evap_prob ~ exp(-local_energy / K_B²)`,
 * which is HIGH when local energy is LOW. Langevin thermostat ADDS energy,
 * SUPPRESSING evaporation — opposite of the SPEC's assumption that "thermal
 * regime causes decay." Cluster decay is *cooling-induced*, not thermal.
 *
 * This test sweeps four damping/Langevin configurations to identify
 * which produces clean cluster decay (mask persistence drops monotonically):
 *
 *   A: damping=ON, langevin=OFF (default damping, no thermal injection)
 *      Energy decays via damping; cluster cools; evaporation kicks in.
 *      Expected: cluster mask shrinks monotonically.
 *
 *   B: damping=ON, larmor=ON, langevin=OFF (enhanced cooling at particle sites)
 *      Larmor radiation drains energy specifically at manifested voxels.
 *      Expected: faster mask decay than A.
 *
 *   C: damping=ON, langevin=ON @ T=0.005 (FTD-0107 baseline)
 *      Langevin replenishes energy; evaporation suppressed; cluster persists.
 *      Reference baseline (mask persistence ≈ 1.0 throughout).
 *
 *   D: damping=ON, selective_damping=OFF, langevin=OFF
 *      Global damping (not just near particles); whole lattice cools fast.
 *      Expected: fastest mask decay (and cluster background).
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

enum class Config { A_default_no_lang, B_larmor_no_lang, C_baseline_lang, D_global_damp };

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
    int n = 0;
    for (int idx : mask) if (vox[idx].state != 0) ++n;
    return static_cast<double>(n) / mask.size();
}

static void apply_config(ftd::RenderBridge& rb, Config cfg) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.damping          = true;
    rb.toggles.selective_damping = true;
    switch (cfg) {
        case Config::A_default_no_lang:
            // damping ON, langevin OFF, larmor OFF, selective ON
            break;
        case Config::B_larmor_no_lang:
            rb.toggles.larmor_radiation = true;
            break;
        case Config::C_baseline_lang:
            rb.toggles.langevin       = true;
            rb.toggles.langevin_T     = 0.005;
            rb.toggles.langevin_gamma = 0.02;
            break;
        case Config::D_global_damp:
            rb.toggles.selective_damping = false;
            break;
    }
}

struct ConfigResult {
    const char* name;
    int initial_mask_size;
    std::vector<int> tick_samples;
    std::vector<double> persistence_at_tick;
    int tau_e_tick;
    double persistence_at_end;
};

static ConfigResult run_one(const char* name, Config cfg, int L, int n_warmup,
                             int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    apply_config(rb, cfg);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = 10.0 * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    ConfigResult r;
    r.name = name;
    r.initial_mask_size = static_cast<int>(mask.size());
    r.tau_e_tick = -1;
    r.persistence_at_end = 1.0;

    for (int t = 1; t <= n_measure; ++t) {
        rb.tick();
        if (t % sample_interval == 0) {
            double p = compute_persistence(rb, mask);
            r.tick_samples.push_back(t);
            r.persistence_at_tick.push_back(p);
            if (r.tau_e_tick < 0 && p < std::exp(-1.0)) r.tau_e_tick = t;
        }
    }
    if (!r.persistence_at_tick.empty())
        r.persistence_at_end = r.persistence_at_tick.back();
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3: cooling-induced evaporation sweep\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_MEASURE = 300;
    const int SAMPLE_INTERVAL = 10;

    std::cout << "Configuration: L=" << L
              << ", warmup=" << N_WARMUP
              << ", measure=" << N_MEASURE << " ticks\n\n";

    std::vector<ConfigResult> results = {
        run_one("A: default+damp, no Langevin",       Config::A_default_no_lang, L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("B: larmor+damp, no Langevin",        Config::B_larmor_no_lang,  L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("C: FTD-0107 baseline (Lang T=.005)", Config::C_baseline_lang,   L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("D: global damping, no Langevin",     Config::D_global_damp,     L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
    };

    std::cout << "config                              mask_size   tau_e (tick)   persistence_at_end\n";
    std::cout << "------                              ---------   ------------   ------------------\n";
    for (const auto& r : results) {
        std::cout << std::left << std::setw(36) << r.name << std::right
                  << std::setw(9) << r.initial_mask_size << "   "
                  << std::setw(12) << (r.tau_e_tick < 0
                       ? std::string("> ") + std::to_string(N_MEASURE)
                       : std::to_string(r.tau_e_tick)) << "   "
                  << std::fixed << std::setprecision(4) << std::setw(18)
                  << r.persistence_at_end << "\n";
    }

    // Detailed decay curves
    std::cout << "\n--- Detailed decay curves ---\n";
    std::cout << std::left << std::setw(7) << "tick";
    for (const auto& r : results) {
        std::cout << std::setw(36) << r.name;
    }
    std::cout << "\n";
    if (!results.empty()) {
        size_t n = results[0].tick_samples.size();
        for (size_t i = 0; i < n; ++i) {
            std::cout << std::setw(7) << results[0].tick_samples[i];
            for (const auto& r : results) {
                std::cout << std::setw(36) << std::fixed << std::setprecision(3)
                          << r.persistence_at_tick[i];
            }
            std::cout << "\n";
        }
    }

    std::cout << "\n--- Verdict ---\n";
    int n_decayed = 0;
    for (const auto& r : results) {
        if (r.tau_e_tick > 0) ++n_decayed;
    }
    std::cout << "  Configurations producing observable mask decay: "
              << n_decayed << " / " << results.size() << "\n";

    if (n_decayed >= 1) {
        std::cout << "\n  [VERDICT] Cooling-induced evaporation regime EXISTS — at least one\n";
        std::cout << "  configuration produces clean mask decay within " << N_MEASURE << " ticks.\n";
        std::cout << "  Phase B.3 protocol: tune damping/Langevin balance to produce decay\n";
        std::cout << "  curves; sweep across cluster types (different injection amplitudes)\n";
        std::cout << "  for ratio-based PDG comparison. The right Phase B regime is COOLING,\n";
        std::cout << "  not thermal heating — opposite of the original SPEC §4.2 assumption.\n";
    } else {
        std::cout << "\n  [VERDICT] No tested config produced cluster mask decay within "
                  << N_MEASURE << "\n";
        std::cout << "  ticks. Cluster stability is structurally robust under cooling. Real\n";
        std::cout << "  cluster decay may require active fragmentation (collision, weak\n";
        std::cout << "  transmutation, pair_production) rather than energy-based evaporation.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (exploratory protocol design)\n";
    std::cout << "================================================================\n";
    return 0;
}
