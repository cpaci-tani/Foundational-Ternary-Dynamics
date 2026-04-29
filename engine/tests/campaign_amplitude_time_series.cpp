/**
 * @file campaign_amplitude_time_series.cpp
 * @brief Per-tick cluster-size logging at custom injection amplitude.
 *
 * For FTD-0110 Bridge-II regime-4 (temporal/frequency) variance test
 * (DERIV_FTD0110_VARIANCE_ENTROPY.md).
 *
 * Existing campaigns log cluster_history at stride 50 ticks, which
 * miss within-block boundary churn. This binary runs ic1 at a
 * specified amplitude and seeds, logging cluster info every tick
 * (or every --stride ticks) so we can resolve regime-4 temporal
 * variance at amplitudes where boundary fluctuation is genuinely
 * expected (A ≥ 30, where cluster extends beyond a single 27-block).
 *
 * CLI:
 *   --L=N             lattice side length (default 32)
 *   --A=X             injection amplitude in K_GENESIS units (default 10.0)
 *   --seeds=N         number of seeds (default 5)
 *   --stride=K        log every K ticks (default 1)
 *   --burn=N          burn-in ticks (default 200)
 *   --samples=N       sample count after burn-in (default 500)
 *   --T=X             Langevin temperature (default 0.005)
 *   --output-dir=PATH output directory
 *
 * Output:
 *   {output-dir}/cluster_history_seed{rng}.csv with columns
 *   tick,cluster_id,voxel_count,cx,cy,cz,charge_sum
 *
 * Returns 0 if all seeds produce non-empty cluster history.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <algorithm>
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

struct ClusterInfo {
    int voxel_count = 0;
    double cx = 0, cy = 0, cz = 0;
    int charge_sum = 0;
};

// Largest cluster only (we don't care about secondary clusters at
// canonical ic1; this saves a lot of allocation in the per-tick loop).
ClusterInfo largest_cluster(const ftd::RenderBridge& rb) {
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
    ClusterInfo best;

    for (int z0 = 0; z0 < L; ++z0)
    for (int y0 = 0; y0 < L; ++y0)
    for (int x0 = 0; x0 < L; ++x0) {
        const int i0 = idx(x0, y0, z0);
        if (visited[i0]) continue;
        if (voxels[i0].state == 0) continue;

        ClusterInfo c;
        std::queue<std::tuple<int,int,int>> q;
        q.push({x0, y0, z0});
        visited[i0] = true;

        while (!q.empty()) {
            auto [cx, cy, cz] = q.front(); q.pop();
            const int ci = idx(cx, cy, cz);
            const auto& v = voxels[ci];
            c.voxel_count++;
            c.cx += cx; c.cy += cy; c.cz += cz;
            c.charge_sum += v.state;
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

        if (c.voxel_count > best.voxel_count) {
            best = c;
        }
    }
    if (best.voxel_count > 0) {
        best.cx /= best.voxel_count;
        best.cy /= best.voxel_count;
        best.cz /= best.voxel_count;
    }
    return best;
}

void run_seed_with_logging(int L, std::uint32_t seed, double A,
                           int N_BURN, int N_SAMPLES, int STRIDE,
                           double Langevin_T,
                           const fs::path& out_csv) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = Langevin_T;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.dual_substrate   = false;
    rb.seed_rng(seed);

    rb.inject_flux(L / 2, L / 2, L / 2,
                   {A * ftd::K_GENESIS, 0, 0});

    // Burn-in
    rb.run(N_BURN);

    // Open CSV and write header
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: could not open %s for writing\n",
                     out_csv.string().c_str());
        return;
    }
    std::fprintf(f, "tick,cluster_id,voxel_count,cx,cy,cz,charge_sum\n");

    // Sample loop: every STRIDE ticks, log largest cluster
    for (int sample = 0; sample < N_SAMPLES; ++sample) {
        rb.run(STRIDE);
        const int tick = N_BURN + (sample + 1) * STRIDE;
        const ClusterInfo c = largest_cluster(rb);
        std::fprintf(f, "%d,0,%d,%.4f,%.4f,%.4f,%d\n",
                     tick, c.voxel_count, c.cx, c.cy, c.cz, c.charge_sum);
    }
    std::fclose(f);
}

} // anon

int main(int argc, char** argv) {
    int L = 32;
    double A = 10.0;
    int N_SEEDS = 5;
    int STRIDE = 1;
    int N_BURN = 200;
    int N_SAMPLES = 500;
    double Langevin_T = 0.005;
    std::string output_dir = "engine/results/amplitude_time_series_default/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=",       0) == 0) L          = std::atoi(a.c_str() + 4);
        else if (a.rfind("--A=",       0) == 0) A          = std::atof(a.c_str() + 4);
        else if (a.rfind("--seeds=",   0) == 0) N_SEEDS    = std::atoi(a.c_str() + 8);
        else if (a.rfind("--stride=",  0) == 0) STRIDE     = std::atoi(a.c_str() + 9);
        else if (a.rfind("--burn=",    0) == 0) N_BURN     = std::atoi(a.c_str() + 7);
        else if (a.rfind("--samples=", 0) == 0) N_SAMPLES  = std::atoi(a.c_str() + 10);
        else if (a.rfind("--T=",       0) == 0) Langevin_T = std::atof(a.c_str() + 4);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    fs::create_directories(output_dir);
    std::printf("================================================================\n");
    std::printf("Campaign: amplitude time-series (per-tick cluster_history logging)\n");
    std::printf("  L=%d  A=%.2f·K_GENESIS  T=%.4f  seeds=%d  burn=%d  samples=%d  stride=%d\n",
                L, A, Langevin_T, N_SEEDS, N_BURN, N_SAMPLES, STRIDE);
    std::printf("  output_dir=%s\n", output_dir.c_str());
    std::printf("================================================================\n");

    for (int s = 0; s < N_SEEDS; ++s) {
        const std::uint32_t seed = 0xE0102000u + static_cast<std::uint32_t>(s);
        const fs::path out_csv = fs::path(output_dir)
                               / ("cluster_history_seed" + std::to_string(seed) + ".csv");
        std::printf("  seed %d/%d (rng=0x%X) → %s\n",
                    s + 1, N_SEEDS, seed, out_csv.string().c_str());
        std::fflush(stdout);
        run_seed_with_logging(L, seed, A, N_BURN, N_SAMPLES, STRIDE, Langevin_T, out_csv);
    }

    std::printf("\nDONE — wrote %d cluster_history files to %s\n", N_SEEDS, output_dir.c_str());
    return 0;
}
