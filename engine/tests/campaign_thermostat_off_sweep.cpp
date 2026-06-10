/**
 * @file campaign_thermostat_off_sweep.cpp
 * @brief FTD-0260 discriminator: is the FTD-0110 k(A) drift thermostat physics?
 *
 * Pre-registration: docs/theory/03_derivations/foundational_mechanics/
 *   PREREG_THERMOSTAT_OFF_AMPLITUDE_SWEEP_v1.md
 * Context: FTD-0259 closed Mechanism alpha (multi-block irrep leakage) as the
 * drift mechanism and elevated Mechanism gamma (Langevin): the drift onset
 * matches the untuned thermal crossover A* = sqrt(L^3 * T_L) = 12.8, and the
 * historical k(A) campaign provably ran thermostat-active
 * (campaign_amplitude_time_series.cpp: langevin=true, gamma=0.02, T=0.005).
 *
 * Protocol is a faithful clone of campaign_amplitude_time_series.cpp
 * (the surviving FTD-0110 rig): canonical ic1 toggles
 * (wave_propagation + gauss_projection + genesis [+ langevin per arm]),
 * x-axial point injection of A*K_GENESIS at the lattice center, largest
 * 26-connected nonzero-state cluster, k = N_mean / A^2.
 *
 * CLI:
 *   --L=N              lattice side (default 32, the historical value)
 *   --A=X              injection amplitude in K_GENESIS units (default 10)
 *   --seeds=N          number of seeds (default 1)
 *   --burn=N           burn-in ticks (default 200, historical)
 *   --window=N         sampling window ticks after burn-in (default 500)
 *   --stride=K         sample every K ticks within the window (default 10)
 *   --thermostat=on|off  arm switch (default on). off => langevin toggle FALSE
 *                        (no friction, no noise: fluctuation-dissipation ties
 *                        sigma = sqrt(2*gamma*T), so gamma=0 kills both).
 *   --dir=axial|diag   injection direction (default axial = {A*K_GENESIS,0,0};
 *                        diag = body-diagonal, components A*K_GENESIS/sqrt(3),
 *                        same magnitude -- the FTD-0263 direction-invariance arm).
 *   --coupling=on|off  g_c*grad(s) state->flux source term (default off =
 *                        v1-compatible). The v2 re-characterization campaign
 *                        passes --coupling=on, aligning with the canonical
 *                        test_emergent_ic1_topology protocol (PREREG_NA_LAW_
 *                        CURRENT_STACK_v1.md).
 *   --gamma=X          Langevin friction when on (default 0.02, historical)
 *   --T=X              Langevin temperature when on (default 0.005, historical)
 *   --tag=S            arm label used in the output filename (default "run")
 *   --output-dir=PATH  output directory
 *
 * Output: {output-dir}/sweep_{tag}_A{A}.csv with one row per seed:
 *   tag,L,A,gamma,T,thermostat,coupling,seed,n_samples,n_mean,n_min,n_max,k_mean
 *
 * Returns 0 if every seed produced at least one sample.
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
// (identical logic to campaign_amplitude_time_series.cpp).
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

struct SeedResult {
    int n_samples = 0;
    double n_mean = 0.0;
    int n_min = 0, n_max = 0;
};

SeedResult run_seed(int L, std::uint32_t seed, double A,
                    int burn, int window, int stride,
                    bool thermostat_on, double gamma, double T,
                    bool coupling_on, bool diag) {
    ftd::RenderBridge rb(L);
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
        rb.toggles.langevin = false;  // no friction, no noise
    }
    rb.seed_rng(seed);

    if (diag) {
        const double c3 = A * ftd::K_GENESIS / std::sqrt(3.0);
        rb.inject_flux(L / 2, L / 2, L / 2, {c3, c3, c3});
    } else {
        rb.inject_flux(L / 2, L / 2, L / 2, {A * ftd::K_GENESIS, 0, 0});
    }
    rb.run(burn);

    SeedResult r;
    long long acc = 0;
    const int n_steps = window / stride;
    for (int s = 0; s < n_steps; ++s) {
        rb.run(stride);
        const int n = largest_cluster_size(rb);
        acc += n;
        if (r.n_samples == 0) { r.n_min = n; r.n_max = n; }
        if (n < r.n_min) r.n_min = n;
        if (n > r.n_max) r.n_max = n;
        ++r.n_samples;
    }
    r.n_mean = (r.n_samples > 0) ? static_cast<double>(acc) / r.n_samples : 0.0;
    return r;
}

} // namespace

int main(int argc, char** argv) {
    int L = 32;
    double A = 10.0;
    int n_seeds = 1;
    int burn = 200;
    int window = 500;
    int stride = 10;
    bool thermostat_on = true;
    bool coupling_on = false;  // v1-compatible default; v2 passes --coupling=on
    bool diag = false;         // FTD-0263 direction arm
    double gamma = 0.02;
    double T = 0.005;
    std::string tag = "run";
    std::string output_dir = "engine/results/thermostat_off_sweep_default/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--A=", 0) == 0)          A = std::atof(a.c_str() + 4);
        else if (a.rfind("--seeds=", 0) == 0)      n_seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--burn=", 0) == 0)       burn = std::atoi(a.c_str() + 7);
        else if (a.rfind("--window=", 0) == 0)     window = std::atoi(a.c_str() + 9);
        else if (a.rfind("--stride=", 0) == 0)     stride = std::atoi(a.c_str() + 9);
        else if (a == "--thermostat=off")          thermostat_on = false;
        else if (a == "--thermostat=on")           thermostat_on = true;
        else if (a == "--coupling=off")            coupling_on = false;
        else if (a == "--coupling=on")             coupling_on = true;
        else if (a == "--dir=diag")                diag = true;
        else if (a == "--dir=axial")               diag = false;
        else if (a.rfind("--gamma=", 0) == 0)      gamma = std::atof(a.c_str() + 8);
        else if (a.rfind("--T=", 0) == 0)          T = std::atof(a.c_str() + 4);
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    fs::create_directories(output_dir);
    char abuf[32];
    std::snprintf(abuf, sizeof(abuf), "%.2f", A);
    const fs::path out_csv = fs::path(output_dir)
                           / ("sweep_" + tag + "_A" + abuf + ".csv");

    std::printf("thermostat_off_sweep: tag=%s L=%d A=%.2f thermostat=%s coupling=%s gamma=%.4f T=%.5f "
                "seeds=%d burn=%d window=%d stride=%d\n",
                tag.c_str(), L, A, thermostat_on ? "on" : "off",
                coupling_on ? "on" : "off",
                thermostat_on ? gamma : 0.0, thermostat_on ? T : 0.0,
                n_seeds, burn, window, stride);
    std::fflush(stdout);

    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", out_csv.string().c_str());
        return 1;
    }
    std::fprintf(f, "tag,L,A,gamma,T,thermostat,coupling,seed,n_samples,n_mean,n_min,n_max,k_mean\n");

    bool all_ok = true;
    for (int s = 0; s < n_seeds; ++s) {
        const std::uint32_t seed = 0xE0102000u + static_cast<std::uint32_t>(s);
        const SeedResult r = run_seed(L, seed, A, burn, window, stride,
                                      thermostat_on, gamma, T, coupling_on, diag);
        const double k = r.n_mean / (A * A);
        std::fprintf(f, "%s,%d,%.4f,%.4f,%.5f,%s,%s,0x%X,%d,%.4f,%d,%d,%.6f\n",
                     tag.c_str(), L, A,
                     thermostat_on ? gamma : 0.0, thermostat_on ? T : 0.0,
                     thermostat_on ? "on" : "off",
                     coupling_on ? "on" : "off",
                     seed, r.n_samples, r.n_mean, r.n_min, r.n_max, k);
        std::printf("  seed 0x%X: n_mean=%.2f n_range=[%d,%d] k=%.4f\n",
                    seed, r.n_mean, r.n_min, r.n_max, k);
        std::fflush(stdout);
        if (r.n_samples == 0) all_ok = false;
    }
    std::fclose(f);
    return all_ok ? 0 : 1;
}
