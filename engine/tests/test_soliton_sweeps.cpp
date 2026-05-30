/**
 * Test: Soliton Emergence and Sweeps Campaign (Class B Track 2)
 *
 * Performs automated sweeps over amplitudes, seeds, and toggle configs
 * using the triplet metric (n_total, centroid_drift, rms_radius)
 * to map stable solitons, flooding, and decay channels in the CA engine.
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <queue>
#include <tuple>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

struct TripletMetric {
    int n_total = 0;
    double centroid_drift = 0.0;
    double rms_radius = 0.0;
    int tau_bind = 0;
    const char* regime = "DECAY";
};

int wrap_diff(int a, int b, int L) {
    int d = a - b;
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

double wrap_diff_d(double a, double b, double L) {
    double d = a - b;
    if (d > L / 2.0) d -= L;
    if (d < -L / 2.0) d += L;
    return d;
}

// Connected component analysis with periodic-wrapping awareness.
TripletMetric analyze_largest_cluster(const ftd::RenderBridge& rb, int cx, int cy, int cz, int max_ticks, int start_tick) {
    const int L = rb.lattice().size();
    const int N_total = L * L * L;
    const auto& voxels = rb.voxels();

    auto idx = [L](int x, int y, int z) {
        x = ((x % L) + L) % L;
        y = ((y % L) + L) % L;
        z = ((z % L) + L) % L;
        return x * L * L + y * L + z;
    };

    std::vector<bool> visited(N_total, false);
    std::vector<std::vector<int>> clusters;

    for (int z0 = 0; z0 < L; ++z0)
    for (int y0 = 0; y0 < L; ++y0)
    for (int x0 = 0; x0 < L; ++x0) {
        int i0 = idx(x0, y0, z0);
        if (visited[i0] || voxels[i0].state == 0) continue;

        std::vector<int> comp;
        std::queue<int> q;
        q.push(i0);
        visited[i0] = true;

        while (!q.empty()) {
            int curr = q.front(); q.pop();
            comp.push_back(curr);

            auto neighbors = rb.lattice().neighbors_26(curr);
            for (int n : neighbors) {
                if (!visited[n] && voxels[n].state != 0) {
                    visited[n] = true;
                    q.push(n);
                }
            }
        }
        clusters.push_back(std::move(comp));
    }

    TripletMetric metric;
    metric.tau_bind = rb.current_tick();

    if (clusters.empty()) {
        metric.regime = "DECAY";
        return metric;
    }

    // Find largest cluster
    auto it_max = std::max_element(clusters.begin(), clusters.end(),
        [](const auto& a, const auto& b){ return a.size() < b.size(); });
    const auto& pos = *it_max;
    metric.n_total = static_cast<int>(pos.size());

    if (metric.n_total < 4) {
        metric.regime = "DECAY";
        return metric;
    }
    if (metric.n_total > N_total / 100) {
        metric.regime = "FLOODING";
        return metric;
    }

    // Periodic-wrap-aware centroid
    int r_idx = pos[0];
    ftd::Coord r_coord = rb.lattice().coord(r_idx);
    double sx = 0, sy = 0, sz = 0;
    for (int v : pos) {
        ftd::Coord c = rb.lattice().coord(v);
        sx += r_coord.x + wrap_diff(c.x, r_coord.x, L);
        sy += r_coord.y + wrap_diff(c.y, r_coord.y, L);
        sz += r_coord.z + wrap_diff(c.z, r_coord.z, L);
    }
    double ccx = sx / metric.n_total;
    double ccy = sy / metric.n_total;
    double ccz = sz / metric.n_total;

    // Centroid drift
    double dx = wrap_diff_d(ccx, cx, L);
    double dy = wrap_diff_d(ccy, cy, L);
    double dz = wrap_diff_d(ccz, cz, L);
    metric.centroid_drift = std::sqrt(dx*dx + dy*dy + dz*dz);

    // RMS radius
    double sum_r2 = 0.0;
    for (int v : pos) {
        ftd::Coord c = rb.lattice().coord(v);
        double wx = r_coord.x + wrap_diff(c.x, r_coord.x, L) - ccx;
        double wy = r_coord.y + wrap_diff(c.y, r_coord.y, L) - ccy;
        double wz = r_coord.z + wrap_diff(c.z, r_coord.z, L) - ccz;
        sum_r2 += wx*wx + wy*wy + wz*wz;
    }
    metric.rms_radius = std::sqrt(sum_r2 / metric.n_total);

    // Classification
    if (metric.centroid_drift > 3.0 && metric.rms_radius < L / 3.0) {
        metric.regime = "SOLITON";
    } else {
        metric.regime = "BOUND";
    }

    return metric;
}

enum class ToggleConfig { Default, ColorTriad, FullPhysics };

void apply_config(ftd::RenderBridge& rb, ToggleConfig cfg) {
    rb.toggles.disable_all();
    switch (cfg) {
        case ToggleConfig::Default:
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            break;
        case ToggleConfig::ColorTriad:
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.color_forces     = true;
            rb.toggles.dual_substrate   = true; // Required by triad_binding
            rb.toggles.triad_binding    = true;
            rb.toggles.langevin         = true; // Added Langevin baseline
            rb.toggles.langevin_T       = 0.005;
            rb.toggles.langevin_gamma   = 0.02;
            break;
        case ToggleConfig::FullPhysics:
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.color_forces     = true;
            rb.toggles.strong_force     = true;
            rb.toggles.dual_substrate   = true; // Required by triad_binding
            rb.toggles.triad_binding    = true;
            rb.toggles.pair_production  = true;
            rb.toggles.poisson_coulomb  = true; // Required by exchange_force
            rb.toggles.exchange_force   = true;
            rb.toggles.gravity          = true; // Required by latency_field
            rb.toggles.latency_field    = true;
            rb.toggles.langevin         = true;
            rb.toggles.langevin_T       = 0.005;
            rb.toggles.langevin_gamma   = 0.02;
            break;
    }
}

const char* config_name(ToggleConfig cfg) {
    switch (cfg) {
        case ToggleConfig::Default:     return "Default";
        case ToggleConfig::ColorTriad:  return "Color+Triad";
        case ToggleConfig::FullPhysics: return "Full Physics";
    }
    return "";
}

} // namespace

int main() {
    std::printf("================================================================\n");
    std::printf("  TRACK 2: Emergent Soliton Sweeps Campaign (WSL2/CUDA)\n");
    std::printf("  Goal: sweep amplitudes and configs to map soliton attractors\n");
    std::printf("================================================================\n\n");

    const int L = 32;
    const int N_TICKS = 300;
    const int N_SEEDS = 5;
    const int cx = L / 2, cy = L / 2, cz = L / 2;

    std::vector<double> amplitudes = {5.0, 10.0, 15.0, 20.0, 30.0};
    std::vector<ToggleConfig> configs = {ToggleConfig::Default, ToggleConfig::ColorTriad, ToggleConfig::FullPhysics};

    // Open CSV output file
    std::ofstream csv("engine/results/soliton_sweeps.csv");
    csv << "config,amplitude,seed,n_total,centroid_drift,rms_radius,tau_bind,regime\n";

    std::printf("%-13s  %-7s  %-5s  %-8s  %-10s  %-10s  %-8s  %-9s\n",
                "Config", "Amp", "Seed", "n_total", "drift", "rms_rad", "tau_bind", "Regime");
    std::printf("%-13s  %-7s  %-5s  %-8s  %-10s  %-10s  %-8s  %-9s\n",
                "------", "---", "----", "-------", "-----", "-------", "--------", "------");

    for (ToggleConfig cfg : configs) {
        for (double A : amplitudes) {
            double n_sum = 0, drift_sum = 0, rms_sum = 0, tau_sum = 0;
            int soliton_count = 0, flood_count = 0, decay_count = 0, bound_count = 0;

            for (int s = 0; s < N_SEEDS; ++s) {
                std::uint32_t seed = 0xE0102000u + static_cast<std::uint32_t>(s);
                ftd::RenderBridge rb(L);
                apply_config(rb, cfg);
                rb.seed_rng(seed);

                // Inject axial pulse
                rb.inject_flux(cx, cy, cz, {A * ftd::K_GENESIS, 0, 0});

                TripletMetric last_metric;
                bool dissolved_or_flooded = false;

                for (int t = 1; t <= N_TICKS; ++t) {
                    rb.tick();
                    // Diagnostic check every 10 ticks
                    if (t % 10 == 0) {
                        TripletMetric m = analyze_largest_cluster(rb, cx, cy, cz, N_TICKS, 0);
                        if (m.regime == std::string("DECAY") || m.regime == std::string("FLOODING")) {
                            last_metric = m;
                            dissolved_or_flooded = true;
                            break;
                        }
                        last_metric = m;
                    }
                }

                if (!dissolved_or_flooded) {
                    last_metric = analyze_largest_cluster(rb, cx, cy, cz, N_TICKS, 0);
                }

                // Accumulate stats
                n_sum += last_metric.n_total;
                drift_sum += last_metric.centroid_drift;
                rms_sum += last_metric.rms_radius;
                tau_sum += last_metric.tau_bind;

                if (last_metric.regime == std::string("SOLITON")) soliton_count++;
                else if (last_metric.regime == std::string("FLOODING")) flood_count++;
                else if (last_metric.regime == std::string("BOUND")) bound_count++;
                else decay_count++;

                // Write to CSV
                csv << config_name(cfg) << ","
                    << A << ","
                    << seed << ","
                    << last_metric.n_total << ","
                    << last_metric.centroid_drift << ","
                    << last_metric.rms_radius << ","
                    << last_metric.tau_bind << ","
                    << last_metric.regime << "\n";
            }

            // Print aggregated row
            double n_avg = n_sum / N_SEEDS;
            double drift_avg = drift_sum / N_SEEDS;
            double rms_avg = rms_sum / N_SEEDS;
            double tau_avg = tau_sum / N_SEEDS;

            const char* major_regime = "DECAY";
            if (soliton_count > N_SEEDS / 2) major_regime = "SOLITON";
            else if (flood_count > N_SEEDS / 2) major_regime = "FLOODING";
            else if (bound_count > N_SEEDS / 2) major_regime = "BOUND";

            std::printf("%-13s  %-7.1f  %-5s  %7.1f  %9.2f  %9.2f  %8.1f  %-9s\n",
                        config_name(cfg), A, "mean", n_avg, drift_avg, rms_avg, tau_avg, major_regime);
        }
        std::printf("\n");
    }

    csv.close();
    std::printf("Sweeps complete. Raw results exported to engine/results/soliton_sweeps.csv\n");
    return 0;
}
