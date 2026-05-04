/**
 * Dump voxel coordinates + trajectory data for visualization.
 *
 * Outputs JSON to stdout with:
 *   - Spatial configurations at multiple (A, L, tick) points
 *   - Cluster size trajectories n(t) for representative amplitudes
 *   - Resonance landscape: cluster size at end-of-run for fine A grid at each L
 *
 * Designed for the Python visualization script (scripts/exploration/visualize_phase_b3.py).
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct Coord3D { int x, y, z; int8_t state; };

static std::vector<Coord3D> get_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<Coord3D> coords;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state != 0) {
            auto c = lat.coord(static_cast<int>(i));
            coords.push_back({c.x, c.y, c.z, vox[i].state});
        }
    }
    return coords;
}

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

static void dump_coords_json(const std::vector<Coord3D>& coords, const char* indent) {
    std::cout << indent << "[\n";
    for (size_t i = 0; i < coords.size(); ++i) {
        std::cout << indent << "  {\"x\":" << coords[i].x << ",\"y\":" << coords[i].y
                  << ",\"z\":" << coords[i].z << ",\"s\":" << static_cast<int>(coords[i].state) << "}";
        if (i + 1 < coords.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

// Run a single (A, L, +color+triad) configuration; sample voxel positions at given ticks
static void dump_spatial_run(double A, int L, std::vector<int> sample_ticks, const char* label) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;
    const int c = L / 2;
    rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});

    std::cout << "    \"" << label << "\": {\n";
    std::cout << "      \"L\": " << L << ", \"A\": " << A
              << ", \"injection\": \"single_point_x_flux\",\n";
    std::cout << "      \"snapshots\": [\n";

    int max_tick = sample_ticks.empty() ? 0 : *std::max_element(sample_ticks.begin(), sample_ticks.end());
    int prev = 0;
    auto coords = get_manifested(rb);
    bool first = true;
    for (int target : sample_ticks) {
        while (rb.current_tick() < target) rb.tick();
        coords = get_manifested(rb);
        if (!first) std::cout << ",\n";
        std::cout << "        { \"tick\": " << target
                  << ", \"n_manifested\": " << coords.size() << ",\n"
                  << "          \"coords\":";
        dump_coords_json(coords, "          ");
        std::cout << "\n        }";
        first = false;
    }
    std::cout << "\n      ]\n";
    std::cout << "    }";
}

// Run a configuration and dump cluster size trajectory
static void dump_trajectory(double A, int L, int n_ticks, int sample, const char* label) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;
    const int c = L / 2;
    rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});

    std::cout << "    \"" << label << "\": {\n";
    std::cout << "      \"L\": " << L << ", \"A\": " << A << ", \"trajectory\": [\n";
    std::cout << "        [0, " << count_manifested(rb) << "]";
    for (int t = 1; t <= n_ticks; ++t) {
        rb.tick();
        if (t % sample == 0) {
            std::cout << ",\n        [" << t << ", " << count_manifested(rb) << "]";
        }
    }
    std::cout << "\n      ]\n";
    std::cout << "    }";
}

// Resonance landscape: cluster size at end of run for fine A scan
static void dump_resonance_row(int L, std::vector<double> A_vals, int n_ticks) {
    std::cout << "    \"L_" << L << "\": [\n";
    bool first = true;
    for (double A : A_vals) {
        ftd::RenderBridge rb(L);
        rb.toggles.color_forces = true;
        rb.toggles.triad_binding = true;
        rb.toggles.langevin_seed = 1;
        const int c = L / 2;
        rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});

        int n_init = -1, n_t100 = -1, n_final = -1;
        // Skip warmup, then sample
        for (int t = 0; t < 50; ++t) rb.tick();
        n_init = count_manifested(rb);
        for (int t = 51; t <= 150; ++t) rb.tick();
        n_t100 = count_manifested(rb);
        for (int t = 151; t <= n_ticks; ++t) rb.tick();
        n_final = count_manifested(rb);

        if (!first) std::cout << ",\n";
        std::cout << "      {\"A\":" << A << ", \"n_init\":" << n_init
                  << ", \"n_mid\":" << n_t100 << ", \"n_final\":" << n_final << "}";
        first = false;
    }
    std::cout << "\n    ]";
}

int main() {
    std::cout << "{\n";
    std::cout << "  \"meta\": {\n";
    std::cout << "    \"K_GENESIS\": " << ftd::K_GENESIS << ",\n";
    std::cout << "    \"toggle_config\": \"engine_defaults + color_forces + triad_binding\"\n";
    std::cout << "  },\n";

    // ===== SPATIAL: snapshots at key configurations =====
    std::cerr << "[viz] Generating spatial snapshots ..." << std::endl;
    std::cout << "  \"spatial\": {\n";

    std::vector<int> sample_ticks_short = {50, 150, 250};
    std::vector<int> sample_ticks_long = {50, 200, 400, 600};

    // L=32 A=5: stable n=4 (the partial-SC face-axis config)
    dump_spatial_run(5.0, 32, sample_ticks_short, "L32_A5_stable_n4");
    std::cout << ",\n";
    // L=32 A=5.75: the "n=8" multi-cluster
    dump_spatial_run(5.75, 32, sample_ticks_short, "L32_A5p75_n8_artifact");
    std::cout << ",\n";
    // L=48 A=5.75: same A different L
    dump_spatial_run(5.75, 48, sample_ticks_short, "L48_A5p75");
    std::cout << ",\n";
    // L=64 A=5.75: same A different L
    dump_spatial_run(5.75, 64, sample_ticks_short, "L64_A5p75");
    std::cout << ",\n";
    // L=32 A=7: the deterministic flood-onset cluster (210-tick binding lifetime)
    dump_spatial_run(7.0, 32, std::vector<int>{50, 150, 200, 220, 250, 400}, "L32_A7_flood_cascade");
    std::cout << ",\n";
    // L=32 A=10: the "soliton"
    dump_spatial_run(10.0, 32, std::vector<int>{50, 100, 200, 300}, "L32_A10_soliton");
    std::cout << "\n  },\n";

    // ===== TRAJECTORIES =====
    std::cerr << "[viz] Generating trajectories ..." << std::endl;
    std::cout << "  \"trajectories\": {\n";
    dump_trajectory(5.0, 32, 600, 10, "L32_A5_stable");
    std::cout << ",\n";
    dump_trajectory(5.75, 32, 600, 10, "L32_A5p75");
    std::cout << ",\n";
    dump_trajectory(7.0, 32, 600, 10, "L32_A7_floods");
    std::cout << ",\n";
    dump_trajectory(10.0, 32, 600, 10, "L32_A10_soliton");
    std::cout << ",\n";
    dump_trajectory(5.75, 48, 600, 10, "L48_A5p75");
    std::cout << ",\n";
    dump_trajectory(5.75, 64, 600, 10, "L64_A5p75");
    std::cout << "\n  },\n";

    // ===== RESONANCE LANDSCAPE =====
    std::cerr << "[viz] Generating resonance landscape ..." << std::endl;
    std::vector<double> A_vals;
    for (double A = 3.0; A <= 8.0001; A += 0.25) A_vals.push_back(A);
    std::cout << "  \"resonance\": {\n";
    dump_resonance_row(32, A_vals, 300);
    std::cout << ",\n";
    dump_resonance_row(48, A_vals, 300);
    std::cout << ",\n";
    dump_resonance_row(64, A_vals, 300);
    std::cout << "\n  }\n";

    std::cout << "}\n";

    std::cerr << "[viz] DONE" << std::endl;
    return 0;
}
