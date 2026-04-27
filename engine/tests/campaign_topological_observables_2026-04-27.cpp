/**
 * @file campaign_topological_observables_2026-04-27.cpp
 * @brief FTD-0104: Topological observable mapping (engine-native exploration)
 *
 * Pre-registration: docs/theory/10_eft_program/PROTOCOL_TOPOLOGICAL_OBSERVABLES.md
 * Plan: ~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md (Campaign D)
 *
 * Engine-as-instrument exploration. Four sub-experiments, all on L=32
 * Langevin lattice (T=0.005, γ=0.02, gauss-projection ON), 5 seeds each.
 *
 *   D1 — Wilson loop area-law      (s0-seed-wilson-loop)
 *   D2 — Flux-tube tension         (s0-seed-flux-tube)
 *   D3 — Monopole stability        (s0-seed-monopole)
 *   D4 — Vacuum instanton density  (langevin-only or s0-seed-instanton)
 *
 * Each writes:
 *   per_snapshot.csv with columns:
 *     tick,total_density,total_charge,manifested,Q_top,
 *     W_R,E_tube,tube_length,monopole_core,centroid_x,centroid_y,centroid_z
 *   meta.json with experiment metadata + per-seed summaries
 *
 * GPU: REQUIRED (per CLAUDE.md). Constructed with default backend.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// ── Lattice index helper ────────────────────────────────────────────
inline int idx(int L, int x, int y, int z) {
    auto wrap = [L](int v) { return ((v % L) + L) % L; };
    return wrap(x) * L * L + wrap(y) * L + wrap(z);
}

// ── Pontryagin-density approximation Q_top = Σ J·(∇×J) ──────────────
// On a discrete lattice with periodic BC, central differences.
double compute_qtop(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    const auto& v = rb.voxels();
    double Q = 0.0;
    for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x) {
                const int i = idx(L, x, y, z);
                const auto& J = v[i].flux;
                const auto& Jxp = v[idx(L, x+1, y, z)].flux;
                const auto& Jxm = v[idx(L, x-1, y, z)].flux;
                const auto& Jyp = v[idx(L, x, y+1, z)].flux;
                const auto& Jym = v[idx(L, x, y-1, z)].flux;
                const auto& Jzp = v[idx(L, x, y, z+1)].flux;
                const auto& Jzm = v[idx(L, x, y, z-1)].flux;
                // (∇×J)_x = ∂y Jz - ∂z Jy ; etc.
                const double curl_x = 0.5 * ((Jyp.z - Jym.z) - (Jzp.y - Jzm.y));
                const double curl_y = 0.5 * ((Jzp.x - Jzm.x) - (Jxp.z - Jxm.z));
                const double curl_z = 0.5 * ((Jxp.y - Jxm.y) - (Jyp.x - Jym.x));
                Q += J.x * curl_x + J.y * curl_y + J.z * curl_z;
            }
    return Q;
}

// ── D1 Wilson loop trace ───────────────────────────────────────────
// Approximate ⟨exp(i ∮ J·dl)⟩ on the seeded square loop of radius R
// in the z = mc plane. Returns the line-integrated J·dl (real-valued
// proxy for the trace; we report magnitude and sign).
double compute_wilson_trace(const ftd::RenderBridge& rb, int R) {
    const int L = rb.lattice().size();
    const int mc = L / 2;
    const auto& v = rb.voxels();
    double sum = 0.0;
    // Bottom edge: y = mc - R, traverse x from mc-R to mc+R, dl = +x
    for (int x = mc - R; x <= mc + R; ++x) sum += v[idx(L, x, mc - R, mc)].flux.x;
    // Right edge: x = mc + R, traverse y from mc-R to mc+R, dl = +y
    for (int y = mc - R; y <= mc + R; ++y) sum += v[idx(L, mc + R, y, mc)].flux.y;
    // Top edge: y = mc + R, traverse x from mc+R to mc-R, dl = -x
    for (int x = mc + R; x >= mc - R; --x) sum -= v[idx(L, x, mc + R, mc)].flux.x;
    // Left edge: x = mc - R, traverse y from mc+R to mc-R, dl = -y
    for (int y = mc + R; y >= mc - R; --y) sum -= v[idx(L, mc - R, y, mc)].flux.y;
    return sum;
}

// ── D2 Flux tube observables ───────────────────────────────────────
// Tube along x-axis through center; integrate energy density along
// the tube, measure length above threshold.
struct TubeMetrics { double E_tube = 0.0; int length = 0; };
TubeMetrics compute_tube_metrics(const ftd::RenderBridge& rb, double threshold) {
    const int L = rb.lattice().size();
    const int mc = L / 2;
    const auto& v = rb.voxels();
    TubeMetrics m;
    int last_x_above = mc;
    int first_x_above = mc;
    bool found = false;
    for (int x = 0; x < L; ++x) {
        const auto& J = v[idx(L, x, mc, mc)].flux;
        const double e = J.x * J.x + J.y * J.y + J.z * J.z;
        m.E_tube += e;
        if (e > threshold) {
            if (!found) { first_x_above = x; found = true; }
            last_x_above = x;
        }
    }
    m.length = found ? (last_x_above - first_x_above + 1) : 0;
    return m;
}

// ── D3 Monopole core / centroid ────────────────────────────────────
struct MonopoleMetrics {
    int core_voxels = 0;
    double centroid_x = 0.0, centroid_y = 0.0, centroid_z = 0.0;
};
MonopoleMetrics compute_monopole_metrics(const ftd::RenderBridge& rb,
                                         double core_threshold,
                                         int seed_x, int seed_y, int seed_z,
                                         int search_radius) {
    const int L = rb.lattice().size();
    const auto& v = rb.voxels();
    MonopoleMetrics m;
    double weight_sum = 0.0;
    for (int dz = -search_radius; dz <= search_radius; ++dz)
        for (int dy = -search_radius; dy <= search_radius; ++dy)
            for (int dx = -search_radius; dx <= search_radius; ++dx) {
                const int x = seed_x + dx, y = seed_y + dy, z = seed_z + dz;
                const auto& J = v[idx(L, x, y, z)].flux;
                const double e = J.x * J.x + J.y * J.y + J.z * J.z;
                if (e > core_threshold) {
                    m.core_voxels++;
                    m.centroid_x += x * e;
                    m.centroid_y += y * e;
                    m.centroid_z += z * e;
                    weight_sum += e;
                }
            }
    if (weight_sum > 0.0) {
        m.centroid_x /= weight_sum;
        m.centroid_y /= weight_sum;
        m.centroid_z /= weight_sum;
    } else {
        m.centroid_x = seed_x; m.centroid_y = seed_y; m.centroid_z = seed_z;
    }
    return m;
}

// ── Common toggle setup ─────────────────────────────────────────────
void setup_baseline_toggles(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = 0.005;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.dual_substrate   = false;
}

// ── Per-experiment run ──────────────────────────────────────────────

struct RunSummary {
    std::string experiment;
    int seed = 0;
    int param_idx = 0;
    int total_snapshots = 0;
    double mean_qtop = 0.0;
    double mean_W_R = 0.0;
    double mean_E_tube = 0.0;
    int    final_tube_length = 0;
    int    final_core_voxels = 0;
    int    initial_core_voxels = 0;
};

RunSummary run_experiment(const std::string& exp, int param_idx, int param_value,
                          std::uint32_t seed, int L, int N_BURN, int N_SAMPLES,
                          int SAMPLE_STRIDE, const fs::path& out_dir)
{
    RunSummary sum;
    sum.experiment = exp;
    sum.seed = static_cast<int>(seed);
    sum.param_idx = param_idx;

    ftd::RenderBridge rb(L);
    setup_baseline_toggles(rb);
    rb.seed_rng(seed);

    const int mc = L / 2;
    int wilson_R = 0;
    double tube_threshold = 0.001;
    int monopole_search_r = 6;
    double monopole_core_threshold = 0.01;

    // Apply scenario seeding per experiment
    if (exp == "D1_wilson") {
        ftd::setup_s0_seed_scenario(rb, "s0-seed-wilson-loop");
        wilson_R = std::max(3, L / 8);  // matches scenario's auto-radius
    } else if (exp == "D2_flux_tube") {
        ftd::setup_s0_seed_scenario(rb, "s0-seed-flux-tube");
    } else if (exp == "D3_monopole") {
        ftd::setup_s0_seed_scenario(rb, "s0-seed-monopole");
    } else if (exp == "D4_vacuum_instanton") {
        if (param_value == 1) {
            // arm 1: pure vacuum (Langevin-only)
        } else {
            ftd::setup_s0_seed_scenario(rb, "s0-seed-instanton");
        }
    }

    // Capture initial monopole core size BEFORE burn
    if (exp == "D3_monopole") {
        sum.initial_core_voxels = compute_monopole_metrics(
            rb, monopole_core_threshold, mc, mc, mc, monopole_search_r).core_voxels;
    }

    rb.run(N_BURN);

    std::error_code ec_local;
    fs::create_directories(out_dir, ec_local);
    const std::string suffix = "_p" + std::to_string(param_idx) + "_seed" + std::to_string(seed);
    std::ofstream snap(out_dir / ("per_snapshot" + suffix + ".csv"));
    if (snap) snap << "tick,total_density,total_charge,manifested,Q_top,"
                      "W_R,E_tube,tube_length,monopole_core,"
                      "centroid_x,centroid_y,centroid_z\n";

    for (int s = 0; s < N_SAMPLES; ++s) {
        rb.run(SAMPLE_STRIDE);
        const int tick = N_BURN + (s + 1) * SAMPLE_STRIDE;
        const auto diag = rb.diagnostics();
        const auto eaud = rb.energy_audit();

        double Q = compute_qtop(rb);
        double W = (exp == "D1_wilson") ? compute_wilson_trace(rb, wilson_R) : 0.0;
        TubeMetrics tm{};
        if (exp == "D2_flux_tube") tm = compute_tube_metrics(rb, tube_threshold);
        MonopoleMetrics mm{};
        if (exp == "D3_monopole") {
            mm = compute_monopole_metrics(rb, monopole_core_threshold,
                                          mc, mc, mc, monopole_search_r);
        }

        sum.mean_qtop  += Q;
        sum.mean_W_R   += W;
        sum.mean_E_tube += tm.E_tube;
        sum.total_snapshots++;
        sum.final_tube_length = tm.length;
        sum.final_core_voxels = mm.core_voxels;

        if (snap) {
            snap << tick << "," << eaud.total_energy << ","
                 << eaud.charge_total << "," << diag.manifested_count << ","
                 << Q << "," << W << ","
                 << tm.E_tube << "," << tm.length << ","
                 << mm.core_voxels << ","
                 << mm.centroid_x << "," << mm.centroid_y << "," << mm.centroid_z << "\n";
        }
    }

    if (sum.total_snapshots > 0) {
        sum.mean_qtop  /= sum.total_snapshots;
        sum.mean_W_R   /= sum.total_snapshots;
        sum.mean_E_tube /= sum.total_snapshots;
    }
    return sum;
}

}  // namespace

int main(int argc, char** argv) {
    int L = 32;
    int N_BURN = 200;
    int N_SAMPLES = 40;
    int SAMPLE_STRIDE = 50;
    int N_SEEDS = 5;
    std::string exp_filter;
    bool smoke = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--smoke") smoke = true;
        else if (a.rfind("--L=", 0) == 0) L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--seeds=", 0) == 0) N_SEEDS = std::atoi(a.c_str() + 8);
        else if (a.rfind("--samples=", 0) == 0) N_SAMPLES = std::atoi(a.c_str() + 10);
        else if (a.rfind("--burn=", 0) == 0) N_BURN = std::atoi(a.c_str() + 7);
        else if (a.rfind("--stride=", 0) == 0) SAMPLE_STRIDE = std::atoi(a.c_str() + 9);
        else if (a.rfind("--exp=", 0) == 0) exp_filter = a.substr(6);
    }
    if (smoke) {
        L = 16; N_SAMPLES = 4; N_SEEDS = 1; N_BURN = 50;
    }

    const std::vector<std::string> experiments = {
        "D1_wilson", "D2_flux_tube", "D3_monopole", "D4_vacuum_instanton"
    };

    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN D (FTD-0104): Topological observable mapping\n";
    std::cout << "  Pre-reg: PROTOCOL_TOPOLOGICAL_OBSERVABLES.md\n";
    std::cout << "================================================================\n";
    std::cout << "  L=" << L << "  N_BURN=" << N_BURN
              << "  N_SAMPLES=" << N_SAMPLES << "  STRIDE=" << SAMPLE_STRIDE
              << "  N_SEEDS=" << N_SEEDS << "\n";

    fs::path out_root = fs::path("engine/results/topological_observables_2026-04-27");
    std::error_code ec_root;
    fs::create_directories(out_root, ec_root);

    std::vector<RunSummary> all_summaries;

    for (const auto& exp : experiments) {
        if (!exp_filter.empty() && exp != exp_filter) continue;
        std::cout << "\n  --- " << exp << " ---\n";
        fs::path exp_dir = out_root / exp;
        std::error_code ec_exp;
        fs::create_directories(exp_dir, ec_exp);

        // For D4 we have two arms (vacuum-only=1, instanton-seeded=2)
        std::vector<int> param_values = {0};
        if (exp == "D4_vacuum_instanton") param_values = {1, 2};

        for (size_t p = 0; p < param_values.size(); ++p) {
            for (int s = 0; s < N_SEEDS; ++s) {
                const std::uint32_t seed = 0xE0104000u
                                          + static_cast<std::uint32_t>(p) * 100
                                          + static_cast<std::uint32_t>(s);
                std::cout << "    p" << p << " seed " << s << "/" << N_SEEDS
                          << " (rng=0x" << std::hex << seed << std::dec << ")\n";
                auto sum = run_experiment(exp, static_cast<int>(p), param_values[p],
                                          seed, L, N_BURN, N_SAMPLES, SAMPLE_STRIDE,
                                          exp_dir);
                std::cout << "      mean_Q_top=" << sum.mean_qtop
                          << " mean_W_R=" << sum.mean_W_R
                          << " mean_E_tube=" << sum.mean_E_tube
                          << " final_tube_len=" << sum.final_tube_length
                          << " final_core=" << sum.final_core_voxels
                          << " (init=" << sum.initial_core_voxels << ")\n";
                all_summaries.push_back(sum);
            }
        }
    }

    // Top-level meta.json
    std::ofstream meta(out_root / "meta.json");
    if (meta) {
        meta << "{\n";
        meta << "  \"campaign\": \"topological_observables_2026-04-27\",\n";
        meta << "  \"ledger_row\": \"FTD-0104\",\n";
        meta << "  \"protocol\": \"docs/theory/10_eft_program/PROTOCOL_TOPOLOGICAL_OBSERVABLES.md\",\n";
        meta << "  \"L\": " << L << ",\n";
        meta << "  \"N_BURN\": " << N_BURN << ",\n";
        meta << "  \"N_SAMPLES\": " << N_SAMPLES << ",\n";
        meta << "  \"SAMPLE_STRIDE\": " << SAMPLE_STRIDE << ",\n";
        meta << "  \"N_SEEDS\": " << N_SEEDS << ",\n";
        meta << "  \"total_runs\": " << all_summaries.size() << ",\n";
        meta << "  \"summaries\": [\n";
        for (size_t i = 0; i < all_summaries.size(); ++i) {
            const auto& s = all_summaries[i];
            meta << "    { \"experiment\": \"" << s.experiment << "\","
                 << " \"seed\": " << s.seed << ","
                 << " \"param_idx\": " << s.param_idx << ","
                 << " \"mean_Q_top\": " << s.mean_qtop << ","
                 << " \"mean_W_R\": " << s.mean_W_R << ","
                 << " \"mean_E_tube\": " << s.mean_E_tube << ","
                 << " \"final_tube_length\": " << s.final_tube_length << ","
                 << " \"final_core_voxels\": " << s.final_core_voxels << ","
                 << " \"initial_core_voxels\": " << s.initial_core_voxels << " }";
            if (i + 1 < all_summaries.size()) meta << ",";
            meta << "\n";
        }
        meta << "  ]\n";
        meta << "}\n";
    }

    std::cout << "\n  Wrote " << (out_root / "meta.json").string() << "\n";
    return 0;
}
