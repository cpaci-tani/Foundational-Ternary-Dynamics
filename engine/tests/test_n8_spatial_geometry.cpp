/**
 * Phase B.3 RESONANCE: spatial geometry of the n=8 BCC-corner candidate.
 *
 * Both ontological-polymath + ftd-lead-physicist savant agents converged on
 * a critical test: does the n=8 stable cluster at A=5.75·K_GENESIS actually
 * occupy BCC corner positions (8 corners of a sub-cube), or is it just
 * cardinality-8 with arbitrary spatial arrangement?
 *
 * Hypothesis A (BCC corner orbit): the 8 voxels sit on the corners of a
 * sub-cube. Pairwise distance multiset = {edge×12, face-diag×12, body-diag×4}.
 *
 * Hypothesis B (geometric coincidence): the 8 voxels are arranged in some
 * other configuration (chain, planar, irregular).
 *
 * Test: extract the 8 voxel coordinates at the stable A=5.75 cluster at
 * each L ∈ {32, 48, 64}, compute pairwise distance matrix, check for cube
 * vertex signature.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct Voxel3D { int x, y, z; };

static std::vector<Voxel3D> get_manifested_coords(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<Voxel3D> coords;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state != 0) {
            auto c = lat.coord(static_cast<int>(i));
            coords.push_back({c.x, c.y, c.z});
        }
    }
    return coords;
}

static std::vector<double> pairwise_distances_squared(const std::vector<Voxel3D>& coords, int L) {
    std::vector<double> dists;
    auto wrap = [L](int d) { if (d > L/2) d -= L; if (d < -L/2) d += L; return d; };
    for (size_t i = 0; i < coords.size(); ++i) {
        for (size_t j = i+1; j < coords.size(); ++j) {
            int dx = wrap(coords[i].x - coords[j].x);
            int dy = wrap(coords[i].y - coords[j].y);
            int dz = wrap(coords[i].z - coords[j].z);
            dists.push_back(dx*dx + dy*dy + dz*dz);
        }
    }
    std::sort(dists.begin(), dists.end());
    return dists;
}

static void analyze_one(int L, double A_over_KG) {
    const int N_WARMUP = 200;  // long enough to reach stable state

    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A_over_KG * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    auto coords = get_manifested_coords(rb);

    std::cout << "\n--- L=" << L << ", A=" << std::fixed << std::setprecision(2)
              << A_over_KG << "·K_GENESIS, after " << N_WARMUP << " ticks ---\n";
    std::cout << "  N manifested voxels: " << coords.size() << "\n";

    if (coords.empty()) {
        std::cout << "  No manifested voxels.\n";
        return;
    }

    // Print all coordinates
    std::cout << "  Voxel coordinates:\n";
    for (const auto& c : coords) {
        std::cout << "    (" << c.x << ", " << c.y << ", " << c.z << ")\n";
    }

    // Print pairwise distance squared multiset
    auto dists = pairwise_distances_squared(coords, L);
    std::cout << "  Pairwise distances² (sorted):\n  ";
    for (size_t i = 0; i < dists.size(); ++i) {
        std::cout << dists[i] << " ";
        if ((i+1) % 12 == 0) std::cout << "\n  ";
    }
    std::cout << "\n";

    // Cube-vertex signature check
    if (coords.size() == 8) {
        // For an 8-corner cube of side d:
        //   12 edges of length² = d²
        //   12 face diagonals of length² = 2·d²
        //   4 body diagonals of length² = 3·d²
        // Total 28 pairs.
        std::cout << "  Total pairs: " << dists.size() << " (expected 28 for n=8)\n";
        if (dists.size() == 28) {
            int n_d2 = 0, n_2d2 = 0, n_3d2 = 0;
            // Look for edge lengths d² in {1, 4, 9, 16, ...}
            // Just count how many of each unique value appear
            std::vector<int> uniq;
            for (double d : dists) {
                if (uniq.empty() || uniq.back() != static_cast<int>(d)) uniq.push_back(static_cast<int>(d));
            }
            std::cout << "  Unique distance² values: ";
            for (int u : uniq) std::cout << u << " ";
            std::cout << "\n";

            if (uniq.size() == 3) {
                int d2 = uniq[0], d2_face = uniq[1], d2_body = uniq[2];
                int n_edge = 0, n_face = 0, n_body = 0;
                for (double d : dists) {
                    if (static_cast<int>(d) == d2) ++n_edge;
                    else if (static_cast<int>(d) == d2_face) ++n_face;
                    else if (static_cast<int>(d) == d2_body) ++n_body;
                }
                std::cout << "  Distance breakdown:\n";
                std::cout << "    " << n_edge << " × d²=" << d2
                          << "  (expected 12 for cube edges)\n";
                std::cout << "    " << n_face << " × d²=" << d2_face
                          << "  (expected 12 for face diagonals)\n";
                std::cout << "    " << n_body << " × d²=" << d2_body
                          << "  (expected 4 for body diagonals)\n";
                bool is_cube = (n_edge == 12 && n_face == 12 && n_body == 4
                                && d2_face == 2*d2 && d2_body == 3*d2);
                std::cout << "  CUBE-CORNER SIGNATURE: " << (is_cube ? "YES" : "no") << "\n";
                if (is_cube) {
                    int side = static_cast<int>(std::round(std::sqrt(static_cast<double>(d2))));
                    std::cout << "  -> Cube edge length = " << side << " voxels\n";
                    std::cout << "  -> Confirms BCC corner orbit (Hypothesis A)\n";
                } else {
                    std::cout << "  -> NOT a cube. Configuration is some other 8-voxel shape.\n";
                    std::cout << "  -> Falsifies BCC corner orbit (Hypothesis B)\n";
                }
            } else if (uniq.size() == 1) {
                std::cout << "  All 28 pairs have same distance — degenerate (impossible for n=8).\n";
            } else {
                std::cout << "  " << uniq.size() << " unique distances — not a cube (expected 3).\n";
            }
        }
    } else {
        std::cout << "  N != 8 — cannot apply cube-corner test.\n";
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  Spatial geometry of n=8 cluster at A=5.75·K_GENESIS\n";
    std::cout << "================================================================\n";
    std::cout << "Per ontological-polymath + ftd-lead-physicist agent recommendation\n";
    std::cout << "Test: do the 8 voxels occupy BCC corner positions of a sub-cube?\n";

    analyze_one(32, 5.75);
    analyze_one(48, 5.75);
    analyze_one(64, 5.75);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}
