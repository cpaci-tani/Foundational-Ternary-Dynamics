/**
 * L=128 string verification — the critical falsifier test.
 *
 * Hypothesis Prediction B': string lengths at L=128 are integers from the
 * FTD framework integer set {1, 2, 3, 4, 7, 13, 27} or simple combinations.
 *
 * Falsifier: if any axis at L=128 produces a non-framework integer length
 * (e.g., 5, 9, 11), the L=32 irrep-match was coincidental.
 *
 * Confirmer: if all 3 axes give deterministic integer lengths in the
 * framework set or simple sums, the engine's string quantization is
 * structurally tight to FTD-0110's algebraic spine.
 *
 * Cost: L=128 is 8x slower per tick than L=64. At 200 ticks × 6 runs,
 * estimated ~60-90 min wall time.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct VoxData { int x, y, z; int8_t state; int8_t color; };

static std::vector<VoxData> get_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<VoxData> out;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state != 0) {
            auto c = lat.coord(static_cast<int>(i));
            out.push_back({c.x, c.y, c.z, vox[i].state, vox[i].color});
        }
    }
    return out;
}

int main() {
    const int L = 128;
    const int n_ticks = 200;
    std::vector<int> seeds = {1, 2};
    std::vector<char> axes = {'x', 'y', 'z'};

    std::cerr << "[L128-verify] Running 3 axes x 2 seeds at L=128, " << n_ticks << " ticks ..." << std::endl;

    std::cout << "{\n  \"runs\": [\n";
    bool first = true;
    for (char axis : axes) {
        for (int seed : seeds) {
            std::cerr << "  axis=" << axis << " seed=" << seed << " ..." << std::flush;

            ftd::RenderBridge rb(L);
            rb.toggles.color_forces = true;
            rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
            const int c = L / 2;
            const double A = 5.0 * ftd::K_GENESIS;
            double fx = 0.0, fy = 0.0, fz = 0.0;
            if (axis == 'x') fx = A;
            else if (axis == 'y') fy = A;
            else fz = A;
            rb.inject_flux(c, c, c, {fx, fy, fz});
            for (int t = 0; t < n_ticks; ++t) rb.tick();

            auto coords = get_manifested(rb);
            int n_R = 0, n_G = 0, n_B = 0, n_none = 0;
            int n_matter = 0, n_anti = 0;
            for (const auto& v : coords) {
                if (v.color == 1) ++n_R;
                else if (v.color == 2) ++n_G;
                else if (v.color == 3) ++n_B;
                else ++n_none;
                if (v.state > 0) ++n_matter;
                else if (v.state < 0) ++n_anti;
            }

            std::cerr << " n=" << coords.size() << " (R=" << n_R << ",G=" << n_G
                      << ",B=" << n_B << ",none=" << n_none << ", matter=" << n_matter
                      << ", anti=" << n_anti << ")" << std::endl;

            if (!first) std::cout << ",\n";
            std::cout << "    {\"L\":128,\"axis\":\"" << axis << "\",\"seed\":" << seed
                      << ",\"n_total\":" << coords.size()
                      << ",\"n_matter\":" << n_matter << ",\"n_antimatter\":" << n_anti
                      << ",\"color_R\":" << n_R << ",\"color_G\":" << n_G
                      << ",\"color_B\":" << n_B << ",\"color_none\":" << n_none
                      << ",\"coords\":[";
            for (size_t i = 0; i < coords.size(); ++i) {
                if (i) std::cout << ",";
                std::cout << "{\"x\":" << coords[i].x << ",\"y\":" << coords[i].y
                          << ",\"z\":" << coords[i].z
                          << ",\"s\":" << static_cast<int>(coords[i].state)
                          << ",\"c\":" << static_cast<int>(coords[i].color) << "}";
            }
            std::cout << "]}";
            first = false;
        }
    }
    std::cout << "\n  ]\n}\n";
    std::cerr << "[L128-verify] DONE" << std::endl;
    return 0;
}
