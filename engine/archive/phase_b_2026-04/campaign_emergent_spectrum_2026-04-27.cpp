/**
 * @file campaign_emergent_spectrum_2026-04-27.cpp
 * @brief FTD-0102: Emergent particle spectrum from generic initial conditions.
 *
 * Pre-registration: docs/theory/10_eft_program/PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md
 * Plan: ~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md (Campaign B)
 *
 * Engine-as-instrument flagship campaign per the user's 2026-04-26
 * reorientation. NOT SM-targeting. The output is "what does the engine
 * produce when run from generic initial conditions" — a histogram of
 * stable-bound-state energies, no comparison to SM particle masses.
 *
 * Five IC classes (PROTOCOL §2):
 *   IC-1 high-energy point injection
 *   IC-2 random thermal initialization
 *   IC-3 two-injection collision
 *   IC-4 pair-creation seed
 *   IC-5 pre-thermalized cosmic-baryogenesis-style
 *
 * Per IC class × 5 seeds = 25 ensembles at L=32. Each runs 5000 ticks
 * with sample stride 50 → 100 snapshots per run.
 *
 * Per snapshot:
 *   - Diagnostics (counts)
 *   - EnergyAudit (energies)
 *   - Cluster manifested voxels by spatial connectivity (BFS)
 *   - Track cluster IDs across snapshots (greedy nearest-centroid)
 *   - Stable cluster: age ≥ 100 ticks
 *
 * Output: meta.json + per_snapshot_census.csv + cluster_history.csv +
 *         stable_clusters_terminal.csv + mass_histogram.csv per IC class
 *
 * GPU: REQUIRED (per CLAUDE.md). No force_cpu().
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

namespace {

// ── Cluster tracking ────────────────────────────────────────────────

struct Cluster {
    int id = -1;          // persistent ID across snapshots
    int voxel_count = 0;
    double total_density = 0.0;   // sum of |J| over cluster voxels
    double total_energy = 0.0;    // sum of |J|² over cluster voxels
    double cx = 0.0, cy = 0.0, cz = 0.0;  // centroid
    int charge_sum = 0;   // sum of states (s ∈ {-1, 0, +1})
    int birth_tick = 0;
    int last_seen_tick = 0;
    int age_ticks = 0;
    std::vector<int> voxel_indices;  // for spatial overlap matching
};

// BFS to find connected components of manifested voxels.
// Two voxels are connected if their Moore-neighbor distance is ≤1 (face/edge/corner).
std::vector<Cluster> detect_clusters(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    const auto& voxels = rb.voxels();
    const int N = L * L * L;

    std::vector<bool> visited(N, false);
    std::vector<Cluster> clusters;

    auto idx = [L](int x, int y, int z) {
        x = (x % L + L) % L;
        y = (y % L + L) % L;
        z = (z % L + L) % L;
        return x * L * L + y * L + z;
    };

    for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x) {
                const int i0 = idx(x, y, z);
                if (visited[i0]) continue;
                if (voxels[i0].state == 0) continue;
                // BFS over Moore-neighbors of same-sign manifestation
                Cluster c;
                c.birth_tick = -1;  // will be set by tracker
                std::queue<std::tuple<int,int,int>> q;
                q.push({x, y, z});
                visited[i0] = true;
                while (!q.empty()) {
                    auto [cx, cy, cz] = q.front(); q.pop();
                    const int ci = idx(cx, cy, cz);
                    const auto& v = voxels[ci];
                    c.voxel_count++;
                    const double d = std::sqrt(v.flux.x * v.flux.x +
                                               v.flux.y * v.flux.y +
                                               v.flux.z * v.flux.z);
                    c.total_density += d;
                    c.total_energy += v.flux.x * v.flux.x +
                                      v.flux.y * v.flux.y +
                                      v.flux.z * v.flux.z;
                    c.cx += cx; c.cy += cy; c.cz += cz;
                    c.charge_sum += v.state;
                    c.voxel_indices.push_back(ci);
                    for (int dz = -1; dz <= 1; ++dz)
                        for (int dy = -1; dy <= 1; ++dy)
                            for (int dx = -1; dx <= 1; ++dx) {
                                if (dx == 0 && dy == 0 && dz == 0) continue;
                                const int nx = cx + dx, ny = cy + dy, nz = cz + dz;
                                const int ni = idx(nx, ny, nz);
                                if (visited[ni]) continue;
                                if (voxels[ni].state == 0) continue;
                                visited[ni] = true;
                                q.push({nx, ny, nz});
                            }
                }
                if (c.voxel_count > 0) {
                    c.cx /= c.voxel_count;
                    c.cy /= c.voxel_count;
                    c.cz /= c.voxel_count;
                    clusters.push_back(std::move(c));
                }
            }
    return clusters;
}

// Greedy nearest-centroid matching: assign IDs from `prev` to current
// clusters. New IDs allocated for unmatched current. `next_id` is
// updated.
void match_clusters(std::vector<Cluster>& current,
                    const std::vector<Cluster>& prev,
                    int& next_id, int tick) {
    std::vector<bool> prev_used(prev.size(), false);
    for (auto& c : current) {
        // Find nearest prev within Moore-radius 4
        double best_dist = 1e30;
        int best_prev = -1;
        for (size_t j = 0; j < prev.size(); ++j) {
            if (prev_used[j]) continue;
            const double dx = c.cx - prev[j].cx;
            const double dy = c.cy - prev[j].cy;
            const double dz = c.cz - prev[j].cz;
            const double d = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (d < best_dist) { best_dist = d; best_prev = static_cast<int>(j); }
        }
        if (best_prev >= 0 && best_dist < 4.0) {
            c.id = prev[best_prev].id;
            c.birth_tick = prev[best_prev].birth_tick;
            c.age_ticks = tick - c.birth_tick;
            prev_used[best_prev] = true;
        } else {
            c.id = next_id++;
            c.birth_tick = tick;
            c.age_ticks = 0;
        }
        c.last_seen_tick = tick;
    }
}

// ── IC class setup ──────────────────────────────────────────────────

void setup_baseline_toggles(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = 0.005;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.dual_substrate   = false;
}

void apply_ic_class(ftd::RenderBridge& rb, int L, const std::string& ic, std::uint32_t seed) {
    setup_baseline_toggles(rb);
    rb.seed_rng(seed);

    if (ic == "ic1_inject") {
        // High-energy point injection at lattice center
        rb.inject_flux(L / 2, L / 2, L / 2,
                       {10.0 * ftd::K_GENESIS, 0, 0});
    } else if (ic == "ic2_thermal") {
        // Random thermal: just elevated Langevin T, no point injection
        rb.toggles.langevin_T = 0.05;
    } else if (ic == "ic3_collision") {
        // Two-injection collision
        const int q = L / 4;
        rb.inject_flux(L/2 - q, L/2, L/2, {+5.0 * ftd::K_GENESIS, 0, 0});
        rb.inject_flux(L/2 + q, L/2, L/2, {-5.0 * ftd::K_GENESIS, 0, 0});
    } else if (ic == "ic4_paircreate") {
        // Pair-creation seed: minimal flux + small perturbation at center
        rb.inject_flux(L / 2, L / 2, L / 2,
                       {0.5 * ftd::K_GENESIS, 0, 0});
    } else if (ic == "ic5_baryogenesis") {
        // Pre-thermalized cosmic-style: stronger Langevin + center seed
        rb.toggles.langevin_T = 0.1;
        rb.inject_flux(L / 2, L / 2, L / 2,
                       {3.0 * ftd::K_GENESIS, 0, 0});
    }
}

// ── Per-IC-class run ────────────────────────────────────────────────

struct RunSummary {
    std::string ic_class;
    int seed;
    int total_snapshots = 0;
    int total_clusters_observed = 0;
    int max_concurrent_clusters = 0;
    int stable_clusters = 0;     // alive ≥ 100 ticks
    double mean_total_energy = 0.0;
    double max_total_energy = 0.0;
    int q_cons_violations = 0;
    int initial_charge = 0;
    int final_charge = 0;
};

RunSummary run_ic_seed(const std::string& ic_class, std::uint32_t seed,
                       int L, int N_BURN, int N_SAMPLES, int SAMPLE_STRIDE,
                       const fs::path& out_dir, int stable_threshold)
{
    RunSummary sum;
    sum.ic_class = ic_class;
    sum.seed = static_cast<int>(seed);

    ftd::RenderBridge rb(L);
    apply_ic_class(rb, L, ic_class, seed);
    sum.initial_charge = rb.energy_audit().charge_total;
    rb.run(N_BURN);

    // Open per-seed CSV files
    std::error_code ec;
    fs::create_directories(out_dir, ec);
    std::ofstream census(out_dir / ("per_snapshot_census_seed" + std::to_string(seed) + ".csv"));
    std::ofstream chist(out_dir / ("cluster_history_seed" + std::to_string(seed) + ".csv"));
    if (census) census << "tick,total_energy,manifested,positive,negative,n_clusters,max_cluster_voxels,total_cluster_energy\n";
    if (chist)  chist << "tick,cluster_id,voxel_count,total_density,total_energy,cx,cy,cz,charge_sum,age_ticks,birth_tick\n";

    std::vector<Cluster> prev_clusters;
    std::unordered_set<int> seen_cluster_ids;
    std::unordered_map<int, int> cluster_max_age;
    std::unordered_map<int, double> cluster_max_energy;
    int next_cluster_id = 0;

    for (int s = 0; s < N_SAMPLES; ++s) {
        rb.run(SAMPLE_STRIDE);
        const int tick = N_BURN + (s + 1) * SAMPLE_STRIDE;
        const auto diag = rb.diagnostics();
        const auto eaud = rb.energy_audit();

        std::vector<Cluster> current = detect_clusters(rb);
        match_clusters(current, prev_clusters, next_cluster_id, tick);

        int max_voxels = 0;
        double total_cluster_energy = 0.0;
        for (const auto& c : current) {
            max_voxels = std::max(max_voxels, c.voxel_count);
            total_cluster_energy += c.total_energy;
            seen_cluster_ids.insert(c.id);
            cluster_max_age[c.id] = std::max(cluster_max_age[c.id], c.age_ticks);
            cluster_max_energy[c.id] = std::max(cluster_max_energy[c.id], c.total_energy);
            if (chist) {
                chist << tick << "," << c.id << "," << c.voxel_count << ","
                      << c.total_density << "," << c.total_energy << ","
                      << c.cx << "," << c.cy << "," << c.cz << ","
                      << c.charge_sum << "," << c.age_ticks << "," << c.birth_tick << "\n";
            }
        }

        sum.total_snapshots++;
        sum.mean_total_energy += eaud.total_energy;
        sum.max_total_energy = std::max(sum.max_total_energy, eaud.total_energy);
        sum.max_concurrent_clusters = std::max(sum.max_concurrent_clusters,
                                               static_cast<int>(current.size()));

        if (census) {
            census << tick << "," << eaud.total_energy << ","
                   << diag.manifested_count << ","
                   << diag.positive_count << "," << diag.negative_count << ","
                   << current.size() << "," << max_voxels << "," << total_cluster_energy << "\n";
        }
        prev_clusters = std::move(current);
    }

    sum.total_clusters_observed = static_cast<int>(seen_cluster_ids.size());
    if (sum.total_snapshots > 0) sum.mean_total_energy /= sum.total_snapshots;
    sum.final_charge = rb.energy_audit().charge_total;
    if (sum.final_charge != sum.initial_charge) sum.q_cons_violations++;

    // Stable clusters: max_age ≥ stable_threshold
    int stable_count = 0;
    std::ofstream stable(out_dir / ("stable_clusters_terminal_seed" + std::to_string(seed) + ".csv"));
    if (stable) stable << "cluster_id,max_age_ticks,max_total_energy\n";
    for (const auto& [id, age] : cluster_max_age) {
        if (age >= stable_threshold) {
            stable_count++;
            if (stable) {
                stable << id << "," << age << "," << cluster_max_energy[id] << "\n";
            }
        }
    }
    sum.stable_clusters = stable_count;
    return sum;
}

}  // namespace

int main(int argc, char** argv) {
    int L = 32;
    int N_BURN = 200;
    int N_SAMPLES = 100;
    int SAMPLE_STRIDE = 50;
    int N_SEEDS = 5;
    int stable_threshold = 100;
    std::string ic_filter;   // run only this IC class if specified
    std::string output_dir_override;  // FTD-0107: --output-dir flag for G1 follow-up
    bool smoke = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--smoke") smoke = true;
        else if (a.rfind("--L=", 0) == 0) L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--seeds=", 0) == 0) N_SEEDS = std::atoi(a.c_str() + 8);
        else if (a.rfind("--samples=", 0) == 0) N_SAMPLES = std::atoi(a.c_str() + 10);
        else if (a.rfind("--burn=", 0) == 0) N_BURN = std::atoi(a.c_str() + 7);
        else if (a.rfind("--stride=", 0) == 0) SAMPLE_STRIDE = std::atoi(a.c_str() + 9);
        else if (a.rfind("--stable=", 0) == 0) stable_threshold = std::atoi(a.c_str() + 9);
        else if (a.rfind("--ic=", 0) == 0) ic_filter = a.substr(5);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir_override = a.substr(13);
    }
    if (smoke) {
        L = 16; N_SAMPLES = 10; N_SEEDS = 1; N_BURN = 50;
    }

    const std::vector<std::string> ic_classes = {
        "ic1_inject", "ic2_thermal", "ic3_collision", "ic4_paircreate", "ic5_baryogenesis"
    };

    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN B (FTD-0102): Emergent particle spectrum\n";
    std::cout << "  Pre-reg: PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md\n";
    std::cout << "================================================================\n";
    std::cout << "  L=" << L << "  N_BURN=" << N_BURN
              << "  N_SAMPLES=" << N_SAMPLES << "  STRIDE=" << SAMPLE_STRIDE
              << "  N_SEEDS=" << N_SEEDS
              << "  stable_threshold=" << stable_threshold << " ticks\n";

    fs::path out_root = output_dir_override.empty()
        ? fs::path("engine/results/emergent_spectrum_2026-04-27")
        : fs::path(output_dir_override);
    std::error_code ec;
    fs::create_directories(out_root, ec);

    std::vector<RunSummary> all_summaries;
    int total_q_violations = 0;

    for (const auto& ic : ic_classes) {
        if (!ic_filter.empty() && ic != ic_filter) continue;
        std::cout << "\n  --- IC class: " << ic << " ---\n";
        fs::path ic_dir = out_root / ic;
        fs::create_directories(ic_dir, ec);
        for (int s = 0; s < N_SEEDS; ++s) {
            const std::uint32_t seed = 0xE0102000u + static_cast<std::uint32_t>(s);
            std::cout << "    seed " << s << "/" << N_SEEDS << " (rng=0x"
                      << std::hex << seed << std::dec << ")\n";
            auto sum = run_ic_seed(ic, seed, L, N_BURN, N_SAMPLES, SAMPLE_STRIDE,
                                    ic_dir, stable_threshold);
            std::cout << "      total_clusters_observed=" << sum.total_clusters_observed
                      << ", max_concurrent=" << sum.max_concurrent_clusters
                      << ", stable=" << sum.stable_clusters
                      << ", Q init→final: " << sum.initial_charge << "→" << sum.final_charge
                      << "\n";
            all_summaries.push_back(sum);
            total_q_violations += sum.q_cons_violations;
        }
    }

    // Top-level meta
    std::ofstream meta(out_root / "meta.json");
    if (meta) {
        meta << "{\n";
        meta << "  \"campaign\": \"emergent_spectrum_2026-04-27\",\n";
        meta << "  \"ledger_row\": \"FTD-0102\",\n";
        meta << "  \"protocol\": \"docs/theory/10_eft_program/PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md\",\n";
        meta << "  \"L\": " << L << ",\n";
        meta << "  \"N_BURN\": " << N_BURN << ",\n";
        meta << "  \"N_SAMPLES\": " << N_SAMPLES << ",\n";
        meta << "  \"SAMPLE_STRIDE\": " << SAMPLE_STRIDE << ",\n";
        meta << "  \"N_SEEDS\": " << N_SEEDS << ",\n";
        meta << "  \"stable_threshold_ticks\": " << stable_threshold << ",\n";
        meta << "  \"total_runs\": " << all_summaries.size() << ",\n";
        meta << "  \"total_q_violations\": " << total_q_violations << ",\n";
        meta << "  \"by_ic_class\": {\n";
        std::map<std::string, std::vector<RunSummary>> by_ic;
        for (const auto& s : all_summaries) by_ic[s.ic_class].push_back(s);
        size_t i = 0;
        for (const auto& [ic, runs] : by_ic) {
            int total_observed = 0, total_stable = 0, max_concurrent = 0;
            double mean_e = 0.0;
            for (const auto& r : runs) {
                total_observed += r.total_clusters_observed;
                total_stable += r.stable_clusters;
                max_concurrent = std::max(max_concurrent, r.max_concurrent_clusters);
                mean_e += r.mean_total_energy;
            }
            if (!runs.empty()) mean_e /= runs.size();
            meta << "    \"" << ic << "\": {\n";
            meta << "      \"n_runs\": " << runs.size() << ",\n";
            meta << "      \"total_clusters_observed\": " << total_observed << ",\n";
            meta << "      \"max_concurrent_clusters\": " << max_concurrent << ",\n";
            meta << "      \"total_stable_clusters\": " << total_stable << ",\n";
            meta << "      \"mean_total_energy\": " << mean_e << "\n";
            meta << "    }" << (++i < by_ic.size() ? "," : "") << "\n";
        }
        meta << "  }\n";
        meta << "}\n";
    }

    std::cout << "\n  artifacts → " << out_root.string() << "\n";
    std::cout << "  total Q conservation violations: " << total_q_violations << "\n";
    return 0;
}
