/**
 * Multi-seed × multi-L verification of R/G/B string lengths.
 *
 * Hypothesis (FTD-sympathetic): pure +x flux produces an R-string of length
 * 4 = mult(A_{1g}); pure +y produces G-string of length 2 = mult(E_g); pure
 * +z produces B-string of length 3 = mult(T_{1u}). The integers (4, 2, 3)
 * are O_h irrep multiplicities in the 27-block decomposition — an emergent
 * realization of FTD-0110's [THEOREM] character-table content.
 *
 * Falsifiers:
 *   - String lengths vary by seed → not seed-independent → not structural
 *   - String lengths shift at L=64 → finite-size effect, not bulk O_h
 *
 * If both falsifiers fail (lengths hold across seeds AND L), the irrep
 * interpretation is structurally tight.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct VoxData {
    int x, y, z;
    int8_t state;
    int8_t color;
};

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

struct RunResult {
    int L;
    int seed;
    char axis;            // 'x', 'y', or 'z'
    int n_total;
    int n_matter;         // s == +1
    int n_antimatter;     // s == -1
    int color_R, color_G, color_B, color_none;
    std::vector<VoxData> coords;
};

static RunResult run_string(int L, int seed, char axis, int n_ticks = 300) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    const int c = L / 2;
    const double A = 5.0 * ftd::K_GENESIS;
    double fx = 0.0, fy = 0.0, fz = 0.0;
    if (axis == 'x') fx = A;
    else if (axis == 'y') fy = A;
    else if (axis == 'z') fz = A;
    rb.inject_flux(c, c, c, {fx, fy, fz});
    for (int t = 0; t < n_ticks; ++t) rb.tick();

    auto coords = get_manifested(rb);
    RunResult r;
    r.L = L; r.seed = seed; r.axis = axis;
    r.n_total = static_cast<int>(coords.size());
    r.n_matter = 0; r.n_antimatter = 0;
    r.color_R = r.color_G = r.color_B = r.color_none = 0;
    for (const auto& v : coords) {
        if (v.state > 0) ++r.n_matter;
        else if (v.state < 0) ++r.n_antimatter;
        if (v.color == 1) ++r.color_R;
        else if (v.color == 2) ++r.color_G;
        else if (v.color == 3) ++r.color_B;
        else ++r.color_none;
    }
    r.coords = std::move(coords);
    return r;
}

static void dump_run_json(const RunResult& r) {
    std::cout << "    {\n"
              << "      \"L\": " << r.L
              << ", \"seed\": " << r.seed
              << ", \"axis\": \"" << r.axis << "\",\n"
              << "      \"n_total\": " << r.n_total
              << ", \"n_matter\": " << r.n_matter
              << ", \"n_antimatter\": " << r.n_antimatter << ",\n"
              << "      \"color_R\": " << r.color_R
              << ", \"color_G\": " << r.color_G
              << ", \"color_B\": " << r.color_B
              << ", \"color_none\": " << r.color_none << ",\n"
              << "      \"coords\": [\n";
    for (size_t i = 0; i < r.coords.size(); ++i) {
        const auto& v = r.coords[i];
        std::cout << "        {\"x\":" << v.x << ",\"y\":" << v.y << ",\"z\":" << v.z
                  << ",\"s\":" << static_cast<int>(v.state)
                  << ",\"c\":" << static_cast<int>(v.color) << "}";
        if (i + 1 < r.coords.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "      ]\n"
              << "    }";
}

int main() {
    std::cerr << "[string-verify] Running 3 axes × 3 seeds × 3 L values = 27 runs ..." << std::endl;

    std::vector<int> L_vals = {32, 48, 64};
    std::vector<int> seeds = {1, 2, 3};
    std::vector<char> axes = {'x', 'y', 'z'};

    std::cout << "{\n  \"runs\": [\n";
    bool first = true;
    for (int L : L_vals) {
        for (char axis : axes) {
            for (int seed : seeds) {
                std::cerr << "  L=" << L << " axis=" << axis << " seed=" << seed << " ..." << std::endl;
                RunResult r = run_string(L, seed, axis);
                if (!first) std::cout << ",\n";
                dump_run_json(r);
                first = false;
                std::cerr << "    n=" << r.n_total << " (R=" << r.color_R
                          << ", G=" << r.color_G << ", B=" << r.color_B << ")" << std::endl;
            }
        }
    }
    std::cout << "\n  ]\n}\n";
    std::cerr << "[string-verify] DONE" << std::endl;
    return 0;
}
