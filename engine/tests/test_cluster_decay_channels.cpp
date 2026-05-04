/**
 * Phase B.3 protocol candidate: active decay-channel sweep.
 *
 * Two prior tests established:
 *   - Energy-based protocols (Langevin heating, damping cooling) do NOT
 *     produce cluster mask decay; manifested voxels remain manifested.
 *   - Engine evaporation rule is energy-suppressed: exp(-E/K_B²); high E
 *     suppresses decay, so adding energy makes clusters MORE stable.
 *
 * This test sweeps ACTIVE decay-channel toggles, which are SM-decay-like
 * mechanisms (weak transmutation, pair production, color/strong forces).
 * The hypothesis: real SM decay rates correspond to matrix-element-driven
 * channels, not Boltzmann evaporation. So enabling these toggles should
 * produce cluster decay; varying their parameters should produce a
 * Γ(channel-strength) curve usable for ratio comparison to PDG.
 *
 * Configs (all keep default damping/genesis/wave/gauss/movement; build up):
 *   E: defaults only (weak_transmutation default ON, pair_production OFF)
 *   F: defaults + pair_production
 *   G: defaults + dual_substrate explicitly + larmor
 *   H: defaults + color_forces + strong_force (requires dual_substrate)
 *
 * Reference baseline:
 *   C: FTD-0107 baseline (langevin T=0.005, narrow toggle set)
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

enum class Config { C_baseline, E_defaults, F_defaults_plus_pp, G_defaults_plus_larmor,
                     H_color_strong };

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

static void apply_config(ftd::RenderBridge& rb, Config cfg) {
    // For decay-channel sweep, do NOT call disable_all — keep all
    // default-ON toggles (weak_transmutation, dual_substrate, etc.).
    switch (cfg) {
        case Config::C_baseline:
            // Strip back to FTD-0107 baseline for comparison
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.langevin         = true;
            rb.toggles.langevin_T       = 0.005;
            rb.toggles.langevin_gamma   = 0.02;
            break;
        case Config::E_defaults:
            // Keep all engine defaults (weak_transmutation ON, dual_substrate ON, etc.)
            break;
        case Config::F_defaults_plus_pp:
            rb.toggles.pair_production = true;
            break;
        case Config::G_defaults_plus_larmor:
            rb.toggles.larmor_radiation = true;
            break;
        case Config::H_color_strong:
            rb.toggles.color_forces  = true;
            rb.toggles.strong_force  = true;
            // dual_substrate already ON by default
            break;
    }
}

struct ConfigResult {
    const char* name;
    int initial_mask_size;
    std::vector<double> persistence_at_tick;
    std::vector<int> tick_samples;
    int tau_e_tick;
    double persistence_at_end;
    bool error;
    std::string error_msg;
};

static ConfigResult run_one(const char* name, Config cfg, int L,
                             int n_warmup, int n_measure, int sample_interval) {
    ftd::RenderBridge rb(L);
    apply_config(rb, cfg);
    std::string err;
    if (!rb.toggles.validate(&err)) {
        ConfigResult r;
        r.name = name;
        r.error = true;
        r.error_msg = err;
        return r;
    }

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const double A = 10.0 * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {A, 0.0, 0.0});

    for (int t = 0; t < n_warmup; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    ConfigResult r;
    r.name = name;
    r.error = false;
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
    std::cout << "  PHASE B.3: active decay-channel sweep\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_MEASURE = 300;
    const int SAMPLE_INTERVAL = 10;

    std::vector<ConfigResult> results = {
        run_one("C: FTD-0107 baseline (reference)",     Config::C_baseline,            L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("E: engine defaults (weak ON)",          Config::E_defaults,            L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("F: defaults + pair_production",         Config::F_defaults_plus_pp,    L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("G: defaults + larmor radiation",        Config::G_defaults_plus_larmor,L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
        run_one("H: defaults + color + strong force",    Config::H_color_strong,        L, N_WARMUP, N_MEASURE, SAMPLE_INTERVAL),
    };

    std::cout << "config                                  mask_size   tau_e   persistence_at_end\n";
    std::cout << "------                                  ---------   -----   ------------------\n";
    for (const auto& r : results) {
        if (r.error) {
            std::cout << std::left << std::setw(40) << r.name << " [TOGGLE INVALID: " << r.error_msg << "]\n";
            continue;
        }
        std::cout << std::left << std::setw(40) << r.name << std::right
                  << std::setw(9) << r.initial_mask_size << "   "
                  << std::setw(5) << (r.tau_e_tick < 0
                       ? std::string("> ") + std::to_string(N_MEASURE)
                       : std::to_string(r.tau_e_tick)) << "   "
                  << std::fixed << std::setprecision(4) << std::setw(18)
                  << r.persistence_at_end << "\n";
    }

    // Detailed curves
    std::cout << "\n--- Detailed decay curves (persistence vs tick) ---\n";
    std::cout << std::left << std::setw(7) << "tick";
    for (const auto& r : results) {
        if (!r.error) std::cout << std::setw(40) << r.name;
    }
    std::cout << "\n";
    if (!results.empty() && !results[0].error) {
        size_t n = results[0].tick_samples.size();
        for (size_t i = 0; i < n; ++i) {
            std::cout << std::setw(7) << results[0].tick_samples[i];
            for (const auto& r : results) {
                if (r.error) continue;
                std::cout << std::setw(40) << std::fixed << std::setprecision(3)
                          << r.persistence_at_tick[i];
            }
            std::cout << "\n";
        }
    }

    // Verdict
    int decayed = 0;
    for (const auto& r : results) if (!r.error && r.tau_e_tick > 0) ++decayed;
    std::cout << "\n--- Verdict ---\n";
    std::cout << "  Configurations producing observable mask decay: " << decayed << "\n";
    std::cout << "\n  ";
    if (decayed >= 1) {
        std::cout << "[VERDICT] At least one decay-channel config produces cluster decay.\n";
        std::cout << "  Phase B.3 protocol can use the active-decay-channel approach. The\n";
        std::cout << "  decay rate is matrix-element-driven (not Boltzmann); ratios across\n";
        std::cout << "  channel configurations should be compared to PDG branching ratios,\n";
        std::cout << "  not lifetime ratios in the simple Arrhenius sense.\n";
    } else {
        std::cout << "[VERDICT] No tested decay-channel config produces measurable decay\n";
        std::cout << "  within " << N_MEASURE << " ticks. Cluster stability is structurally\n";
        std::cout << "  extreme — even with weak/strong/color/larmor channels enabled, the\n";
        std::cout << "  cluster's manifested matter persists. Either: (i) decay timescales\n";
        std::cout << "  exceed " << N_MEASURE << " ticks (need much longer runs), (ii) cluster\n";
        std::cout << "  needs targeted perturbation (collision with antiparticle), or\n";
        std::cout << "  (iii) decay observables are not in the mask but in adjacent\n";
        std::cout << "  manifestation patterns (need new observable, not mask persistence).\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (exploratory protocol design)\n";
    std::cout << "================================================================\n";
    return 0;
}
