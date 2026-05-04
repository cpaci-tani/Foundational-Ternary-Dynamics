/**
 * Phase B.3 Test 6: systematic τ_bind scan across (toggle, A) configurations.
 *
 * The deterministic flood-onset finding (§5.6.8) identified τ_bind = 210
 * ticks for (defaults+color+triad, A=7, L=32). This test characterizes
 * τ_bind across multiple toggle configurations and amplitudes to identify
 * patterns:
 *
 *   - Which (toggle, A) gives the longest τ_bind?
 *   - Does τ_bind correlate with cluster size n_init?
 *   - Are there any (toggle, A) with τ_bind > 1000 ticks (effectively stable)?
 *
 * 1 seed per (toggle, A) — exploratory scan; multi-seed verification needed
 * for any flagged candidates.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <string>
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

enum class TogCfg {
    Defaults, ColorTriad, ExchangeForce, ExchangeColorTriad,
    PairProd, PairProdColorTriad, ConfPairProd
};

static const char* cfg_name(TogCfg c) {
    switch (c) {
        case TogCfg::Defaults: return "defaults";
        case TogCfg::ColorTriad: return "+color+triad";
        case TogCfg::ExchangeForce: return "+exchange";
        case TogCfg::ExchangeColorTriad: return "+exch+color+triad";
        case TogCfg::PairProd: return "+pair_prod";
        case TogCfg::PairProdColorTriad: return "+pp+color+triad";
        case TogCfg::ConfPairProd: return "+conf+pair_prod";
    }
    return "?";
}

static void apply_cfg(ftd::RenderBridge& rb, TogCfg c) {
    switch (c) {
        case TogCfg::Defaults: break;
        case TogCfg::ColorTriad:
            rb.toggles.color_forces = true;
            rb.toggles.triad_binding = true;
            break;
        case TogCfg::ExchangeForce:
            rb.toggles.exchange_force = true;
            break;
        case TogCfg::ExchangeColorTriad:
            rb.toggles.color_forces = true;
            rb.toggles.triad_binding = true;
            rb.toggles.exchange_force = true;
            break;
        case TogCfg::PairProd:
            rb.toggles.pair_production = true;
            break;
        case TogCfg::PairProdColorTriad:
            rb.toggles.color_forces = true;
            rb.toggles.triad_binding = true;
            rb.toggles.pair_production = true;
            break;
        case TogCfg::ConfPairProd:
            rb.toggles.confinement = true;
            rb.toggles.pair_production = true;
            break;
    }
}

struct Result {
    TogCfg cfg;
    double A;
    int n_init;
    int t_first_growth;          // -1 if not flooded yet at end of run
    int t_full_flood;
    int n_final;
    bool stable_at_end;
};

static Result run_one(TogCfg cfg, double A, int N_TICKS) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int SAMPLE = 10;

    ftd::RenderBridge rb(L);
    apply_cfg(rb, cfg);
    std::string err;
    if (!rb.toggles.validate(&err)) {
        Result r{cfg, A, -1, -1, -1, -1, false};
        return r;
    }
    rb.toggles.langevin_seed = 1;
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    Result r;
    r.cfg = cfg; r.A = A;
    r.n_init = count_manifested(rb);
    r.t_first_growth = -1;
    r.t_full_flood = -1;
    r.stable_at_end = false;

    for (int t = 1; t <= N_TICKS; ++t) {
        rb.tick();
        if (t % SAMPLE == 0) {
            int n = count_manifested(rb);
            if (r.t_first_growth < 0 && n > r.n_init * 1.5 && n > 5) r.t_first_growth = t;
            if (r.t_full_flood < 0 && n > 1000) r.t_full_flood = t;
        }
    }
    r.n_final = count_manifested(rb);
    r.stable_at_end = (r.t_full_flood < 0);
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 Test 6: τ_bind systematic scan (toggle × amplitude)\n";
    std::cout << "================================================================\n\n";

    const int N_TICKS = 600;
    std::vector<TogCfg> configs = {
        TogCfg::Defaults, TogCfg::ColorTriad, TogCfg::ExchangeForce,
        TogCfg::ExchangeColorTriad, TogCfg::PairProd,
        TogCfg::PairProdColorTriad, TogCfg::ConfPairProd
    };
    std::vector<double> A_vals = {4.0, 5.0, 6.0, 7.0, 8.0, 10.0};

    std::cout << "Configuration: L=32, N_TICKS=" << N_TICKS
              << ", 1 seed per (toggle, A)\n\n";

    std::cout << std::left << std::setw(20) << "toggle config" << std::right
              << std::setw(7) << "A/K_G"
              << std::setw(8) << "n_init"
              << std::setw(13) << "t_growth"
              << std::setw(11) << "t_flood"
              << std::setw(10) << "n_final"
              << "  verdict\n";
    std::cout << "------------------- ------ ------- ------------ ---------- ---------  -------\n";

    std::vector<Result> all;
    for (TogCfg cfg : configs) {
        for (double A : A_vals) {
            Result r = run_one(cfg, A, N_TICKS);
            all.push_back(r);
            if (r.n_init < 0) {
                std::cout << std::left << std::setw(20) << cfg_name(cfg) << std::right
                          << std::setw(7) << std::fixed << std::setprecision(1) << A
                          << "  [INVALID TOGGLE]\n";
                continue;
            }
            const char* verdict;
            if (r.t_full_flood > 0) verdict = "FLOODED";
            else if (r.t_first_growth > 0) verdict = "GROWING";
            else if (r.n_final == 0) verdict = "DECAY";
            else verdict = "STABLE";
            std::cout << std::left << std::setw(20) << cfg_name(cfg) << std::right
                      << std::setw(7) << std::fixed << std::setprecision(1) << A
                      << std::setw(8) << r.n_init
                      << std::setw(13) << (r.t_first_growth < 0 ? std::string(">") + std::to_string(N_TICKS) : std::to_string(r.t_first_growth))
                      << std::setw(11) << (r.t_full_flood < 0 ? std::string(">") + std::to_string(N_TICKS) : std::to_string(r.t_full_flood))
                      << std::setw(10) << r.n_final
                      << "  " << verdict << "\n";
        }
    }

    // Summary tables
    std::cout << "\n--- τ_bind table (t_first_growth, '>N' if no growth) ---\n";
    std::cout << std::left << std::setw(20) << "toggle config" << std::right;
    for (double A : A_vals) std::cout << std::setw(8) << ("A=" + std::to_string(static_cast<int>(A)));
    std::cout << "\n";
    for (TogCfg cfg : configs) {
        std::cout << std::left << std::setw(20) << cfg_name(cfg) << std::right;
        for (double A : A_vals) {
            for (const auto& r : all) {
                if (r.cfg == cfg && std::abs(r.A - A) < 0.01) {
                    if (r.n_init < 0) std::cout << std::setw(8) << "INV";
                    else if (r.t_first_growth > 0) std::cout << std::setw(8) << r.t_first_growth;
                    else std::cout << std::setw(8) << ">" + std::to_string(N_TICKS);
                    break;
                }
            }
        }
        std::cout << "\n";
    }

    // Identify longest τ_bind
    std::cout << "\n--- Longest binding lifetimes ---\n";
    std::vector<Result> stable_or_long;
    for (const auto& r : all) {
        if (r.n_init < 0) continue;
        if (r.t_first_growth < 0 && r.n_final > 0) stable_or_long.push_back(r);
        else if (r.t_first_growth > 300) stable_or_long.push_back(r);
    }
    if (stable_or_long.empty()) {
        std::cout << "  No (toggle, A) configuration achieves τ_bind > 300 ticks at single seed.\n";
    } else {
        for (const auto& r : stable_or_long) {
            std::cout << "  " << cfg_name(r.cfg) << " A=" << std::fixed << std::setprecision(1) << r.A
                      << ": n_init=" << r.n_init
                      << ", t_growth=" << (r.t_first_growth < 0 ? std::string(">") + std::to_string(N_TICKS) : std::to_string(r.t_first_growth))
                      << ", n_final=" << r.n_final
                      << "\n";
        }
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}
