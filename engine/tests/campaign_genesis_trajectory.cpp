/**
 * @file campaign_genesis_trajectory.cpp
 * @brief FTD-0267: genesis-vs-survival per-tick trajectory in the canonical engine.
 *
 * The beta derivation arc (FTD-0265 envelope, FTD-0266 dwell-time kinetics)
 * concluded the sub-knee onset suppression is 100% POST-GENESIS SURVIVAL:
 * at A=10 ~23 voxels cross the K_GENESIS threshold but only ~4 survive to the
 * steady cluster. The direct engine measurement of genesis/evaporation EVENTS
 * (below) falsifies that premise: the engine fires ~5 genesis events at A=10
 * (one-shot burst), evaporation is near-zero, and the cluster size simply
 * tracks the genesis-firing count -- the suppression is at the GENESIS stage
 * (nonlinear flux consumption + coupling + Gauss + damping), not survival.
 *
 * This runner logs the per-tick trajectory from injection (t=0) through steady
 * state: total manifested count, largest 26-connected cluster, +/- counts, and
 * the FTD-0267 observation-only genesis/evaporation EVENT counters (CPU path
 * only -- the GPU kernel path does not populate the host-side counters, so the
 * canonical event-count measurement runs with --cpu).
 *
 * FROZEN PRE-REGISTERED PREDICTION (stated before compute; beta-arc priors
 * CONFIRMED 55 / PARTIAL 30 / NULL 15 -- NULL landed):
 *   P1 peak manifested-count at A=10 in [12, 30]
 *   P2 steady cluster at A=10 (last-third mean) in [2, 8]
 *   P3 peak-manifested / steady-cluster >= 2.0
 *   P4 cumulative genesis events at A=10 in [15, 60]
 *   S1 manifested rises to a peak within ~40 ticks, then DECAYS
 *   S2 survival efficiency (steady cluster / peak manifested) at A=14 > at A=9
 *   S3 steady-window genesis ~ evaporation (within 2x), cluster ~flat
 *   Verdict: CONFIRMED = P1^P2^P3^S1 ; PARTIAL = P3^S1 w/ P1|P2 out of band ;
 *            NULL = !P3 | !S1 (refutes the beta arc's post-genesis conclusion)
 * (Adjudication: scripts/exploration/analyze_genesis_trajectory.py.)
 *
 * Protocol is the canonical ic1 stack (faithful to campaign_thermostat_off_sweep):
 *   toggles wave_propagation + gauss_projection + genesis [+ langevin] [+ coupling],
 *   x-axial point injection A*K_GENESIS at the lattice center, per-tick logging.
 *
 * CLI:
 *   --L=N   --A=X   --ticks=N   --seed=0xHEX|N
 *   --thermostat=on|off  --coupling=on|off  --dir=axial|diag  --gamma=X  --T=X
 *   --cpu   (REQUIRED for genesis/evap event counts)   --tag=S   --output-dir=PATH
 *
 * Output: {output-dir}/traj_{tag}_A{A}.csv with one row per tick:
 *   tick,manifested_count,cluster_size,positive_count,negative_count,genesis_events,evap_events
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

namespace {

// Largest 26-connected cluster of nonzero-state voxels
// (identical logic to campaign_thermostat_off_sweep.cpp).
int largest_cluster_size(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    const int N = L * L * L;
    const auto& voxels = rb.voxels();

    auto idx = [L](int x, int y, int z) {
        x = ((x % L) + L) % L;
        y = ((y % L) + L) % L;
        z = ((z % L) + L) % L;
        return x * L * L + y * L + z;
    };

    std::vector<bool> visited(N, false);
    int best = 0;

    for (int z0 = 0; z0 < L; ++z0)
    for (int y0 = 0; y0 < L; ++y0)
    for (int x0 = 0; x0 < L; ++x0) {
        const int i0 = idx(x0, y0, z0);
        if (visited[i0]) continue;
        if (voxels[i0].state == 0) continue;

        int count = 0;
        std::queue<std::tuple<int, int, int>> q;
        q.push({x0, y0, z0});
        visited[i0] = true;

        while (!q.empty()) {
            auto [cx, cy, cz] = q.front(); q.pop();
            ++count;
            for (int dz = -1; dz <= 1; ++dz)
            for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                const int ni = idx(cx + dx, cy + dy, cz + dz);
                if (visited[ni]) continue;
                if (voxels[ni].state == 0) continue;
                visited[ni] = true;
                q.push({cx + dx, cy + dy, cz + dz});
            }
        }
        if (count > best) best = count;
    }
    return best;
}

// Total manifested voxels (state != 0) and +/- split.
void manifested_counts(const ftd::RenderBridge& rb, int& total, int& pos, int& neg) {
    const auto& voxels = rb.voxels();
    total = pos = neg = 0;
    for (const auto& v : voxels) {
        if (v.state > 0) { ++total; ++pos; }
        else if (v.state < 0) { ++total; ++neg; }
    }
}

} // namespace

int main(int argc, char** argv) {
    int L = 64;
    double A = 10.0;
    int ticks = 300;
    std::uint32_t seed = 0xE0102000u;
    bool thermostat_on = true;
    bool coupling_on = true;   // canonical FTD-0261/0263 stack default
    bool diag = false;
    double gamma = 0.02;
    double T = 0.005;
    bool force_cpu = false;
    std::string tag = "traj";
    std::string output_dir = "engine/results/genesis_trajectory_default/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--A=", 0) == 0)          A = std::atof(a.c_str() + 4);
        else if (a.rfind("--ticks=", 0) == 0)      ticks = std::atoi(a.c_str() + 8);
        else if (a.rfind("--seed=", 0) == 0)       seed = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a == "--thermostat=off")          thermostat_on = false;
        else if (a == "--thermostat=on")           thermostat_on = true;
        else if (a == "--coupling=off")            coupling_on = false;
        else if (a == "--coupling=on")             coupling_on = true;
        else if (a == "--dir=diag")                diag = true;
        else if (a == "--dir=axial")               diag = false;
        else if (a == "--cpu")                     force_cpu = true;
        else if (a.rfind("--gamma=", 0) == 0)      gamma = std::atof(a.c_str() + 8);
        else if (a.rfind("--T=", 0) == 0)          T = std::atof(a.c_str() + 4);
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    ftd::RenderBridge rb(L);
    if (force_cpu) {
        rb.force_cpu();
        rb.set_sor_iterations(150);
    }
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.coupling         = coupling_on;
    rb.toggles.dual_substrate   = false;
    if (thermostat_on) {
        rb.toggles.langevin       = true;
        rb.toggles.langevin_T     = T;
        rb.toggles.langevin_gamma = gamma;
    } else {
        rb.toggles.langevin = false;
    }
    rb.seed_rng(seed);

    if (diag) {
        const double c3 = A * ftd::K_GENESIS / std::sqrt(3.0);
        rb.inject_flux(L / 2, L / 2, L / 2, {c3, c3, c3});
    } else {
        rb.inject_flux(L / 2, L / 2, L / 2, {A * ftd::K_GENESIS, 0, 0});
    }

    fs::create_directories(output_dir);
    char abuf[32];
    std::snprintf(abuf, sizeof(abuf), "%.2f", A);
    const fs::path out_csv = fs::path(output_dir)
                           / ("traj_" + tag + "_A" + abuf + ".csv");

    std::printf("genesis_trajectory: tag=%s L=%d A=%.2f ticks=%d dir=%s thermostat=%s coupling=%s "
                "gamma=%.4f T=%.5f seed=0x%X backend=%s\n",
                tag.c_str(), L, A, ticks, diag ? "diag" : "axial",
                thermostat_on ? "on" : "off", coupling_on ? "on" : "off",
                thermostat_on ? gamma : 0.0, thermostat_on ? T : 0.0,
                seed, force_cpu ? "cpu" : "default");
    if (!force_cpu)
        std::printf("  NOTE: genesis/evap event counters are CPU-only; pass --cpu for P4/S3.\n");
    std::fflush(stdout);

    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", out_csv.string().c_str());
        return 1;
    }
    std::fprintf(f, "# L=%d A=%.4f dir=%s thermostat=%s coupling=%s gamma=%.4f T=%.5f seed=0x%X backend=%s\n",
                 L, A, diag ? "diag" : "axial", thermostat_on ? "on" : "off",
                 coupling_on ? "on" : "off", thermostat_on ? gamma : 0.0,
                 thermostat_on ? T : 0.0, seed, force_cpu ? "cpu" : "default");
    std::fprintf(f, "tick,manifested_count,cluster_size,positive_count,negative_count,genesis_events,evap_events\n");

    int peak_manifested = 0;
    long long cumulative_genesis = 0;
    for (int t = 0; t < ticks; ++t) {
        rb.run(1);
        int total, pos, neg;
        manifested_counts(rb, total, pos, neg);
        const int cluster = largest_cluster_size(rb);
        const long long gev = rb.genesis_events_this_tick();
        const long long eev = rb.evaporation_events_this_tick();
        cumulative_genesis += gev;
        if (total > peak_manifested) peak_manifested = total;
        std::fprintf(f, "%d,%d,%d,%d,%d,%lld,%lld\n",
                     t, total, cluster, pos, neg, gev, eev);
        if (t % 25 == 0 || t == ticks - 1) {
            std::printf("  t=%3d: manifested=%d cluster=%d (+%d/-%d) gen=%lld evap=%lld\n",
                        t, total, cluster, pos, neg, gev, eev);
            std::fflush(stdout);
        }
    }
    std::fclose(f);
    std::printf("DONE A=%.2f: peak_manifested=%d cumulative_genesis=%lld -> %s\n",
                A, peak_manifested, cumulative_genesis, out_csv.string().c_str());
    return 0;
}
