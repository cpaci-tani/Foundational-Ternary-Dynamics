/**
 * @file test_color_binding_and_structure.cpp
 * @brief Phase-4i: Combined tests for (1) RGB triad binding and (2) FTD "color"
 *        transformation structure vs SU(3).
 *
 * Task 1 (binding):
 *   Stamp 3 same-charge quarks in three color configurations:
 *     (A) (R, G, B)  — color-singlet candidate
 *     (B) (R, R, R)  — single-color, should be color-forbidden if SU(3)
 *     (C) (R, G, G)  — partial mixed, not a singlet
 *   Enable forces + strong + movement + triad_binding + exchange.
 *   Run for N ticks, measure separation spread over time.
 *   If (A) binds but (B) does not → color has SU(3)-like physical content.
 *   If all three bind equally → color is just a label.
 *
 * Task 2 (color structure):
 *   Take (R, G, B) configuration, measure some observable (final separation
 *   spread). Rotate the lattice 120° about the body diagonal (x→y→z→x),
 *   which cyclically permutes the color labels (R→G→B→R). Measure the
 *   observable again.
 *   If unchanged: color transforms as O_h axis permutation, a finite
 *   subgroup of SU(3).
 *   If changed: color is orientation-dependent, not even O_h-symmetric.
 *
 *   Continuous SU(3) would require the color field to support arbitrary
 *   mixings of R, G, B, not just the 3! = 6 discrete axis permutations.
 *   FTD stores color as int8_t ∈ {0,1,2,3}, so continuous SU(3) is
 *   structurally impossible at the engine level.  O_h is the best case.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/constructors.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

struct QuarkSpec {
    int charge;    // ±1
    int color;     // 1=R, 2=G, 3=B
    int spin;      // ±1
    ftd::Vec3 pos; // position offset from center
};

// Stamp three quarks and return their final RMS separation after N_TICKS.
double stamp_and_run(const std::array<QuarkSpec, 3>& spec,
                     int L,
                     int N_TICKS,
                     unsigned int seed) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces           = true;
    rb.toggles.poisson_coulomb  = true;
    rb.toggles.movement         = true;
    rb.toggles.color_forces     = true;
    rb.toggles.strong_force     = true;
    rb.toggles.exchange_force   = true;
    rb.toggles.triad_binding    = true;
    rb.seed_rng(seed);

    const int c = L / 2;
    std::vector<std::array<int, 3>> quark_sites;
    for (int q = 0; q < 3; ++q) {
        ftd::Coord pos{c + static_cast<int>(spec[q].pos.x),
                       c + static_cast<int>(spec[q].pos.y),
                       c + static_cast<int>(spec[q].pos.z)};
        ftd::ctor::quark(rb, pos,
                         static_cast<int8_t>(spec[q].charge),
                         static_cast<int8_t>(spec[q].color),
                         static_cast<int8_t>(spec[q].spin));
        quark_sites.push_back({pos.x, pos.y, pos.z});
    }

    rb.run(N_TICKS);

    // Find the three manifested particles closest to the starting positions
    // and compute pairwise RMS separation.
    const auto& vox = rb.voxels();
    std::vector<std::array<int, 3>> found;
    for (const auto& start : quark_sites) {
        // Search in a radius-5 neighborhood of the start position
        int best[3] = {start[0], start[1], start[2]};
        double best_d2 = 1e30;
        bool any = false;
        for (int dx = -5; dx <= 5; ++dx)
        for (int dy = -5; dy <= 5; ++dy)
        for (int dz = -5; dz <= 5; ++dz) {
            const int xx = (start[0] + dx + L) % L;
            const int yy = (start[1] + dy + L) % L;
            const int zz = (start[2] + dz + L) % L;
            const int i = xx * L * L + yy * L + zz;
            if (vox[i].state == 0) continue;
            const double d2 = dx * dx + dy * dy + dz * dz;
            if (d2 < best_d2) { best_d2 = d2; best[0] = xx; best[1] = yy; best[2] = zz; any = true; }
        }
        if (any) found.push_back({best[0], best[1], best[2]});
    }

    if (found.size() < 3) return 999.0;  // particles dispersed or annihilated

    // Compute RMS of the three pairwise distances
    double s2 = 0.0;
    int n_pairs = 0;
    for (size_t a = 0; a < found.size(); ++a)
    for (size_t b = a + 1; b < found.size(); ++b) {
        int dx = found[a][0] - found[b][0];
        int dy = found[a][1] - found[b][1];
        int dz = found[a][2] - found[b][2];
        // Periodic wrap
        if (dx > L / 2) dx -= L; if (dx < -L / 2) dx += L;
        if (dy > L / 2) dy -= L; if (dy < -L / 2) dy += L;
        if (dz > L / 2) dz -= L; if (dz < -L / 2) dz += L;
        s2 += dx * dx + dy * dy + dz * dz;
        ++n_pairs;
    }
    return std::sqrt(s2 / n_pairs);
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Phase-4i: 3-Quark Binding + Color Transformation Structure\n");
    std::printf("================================================================\n");

    const int L = 16;
    const int N_TICKS = 60;
    const int r = 2;  // triangle radius

    // Equilateral triangle in xy-plane (approximate on discrete lattice)
    // Vertex 1: (+r, 0, 0)
    // Vertex 2: (-r/2, +r, 0)    approximated to (-1, +2, 0)
    // Vertex 3: (-r/2, -r, 0)    approximated to (-1, -2, 0)
    const std::array<ftd::Vec3, 3> tri = {
        ftd::Vec3{double(r), 0, 0},
        ftd::Vec3{-1, 2, 0},
        ftd::Vec3{-1, -2, 0}
    };

    // Initial RMS separation (reference)
    double init_sep = 0;
    {
        int dx[3][2] = {{-2, 0}, {1, -2}, {1, 2}};  // pairs: (v1-v2, v2-v3, v1-v3)
        double s2 = 0;
        for (int p = 0; p < 3; ++p) {
            double a = tri[(p + 0) % 3].x - tri[(p + 1) % 3].x;
            double b = tri[(p + 0) % 3].y - tri[(p + 1) % 3].y;
            double c = tri[(p + 0) % 3].z - tri[(p + 1) % 3].z;
            s2 += a * a + b * b + c * c;
        }
        init_sep = std::sqrt(s2 / 3);
    }

    std::printf("  L=%d, N_TICKS=%d, initial RMS separation = %.3f\n\n",
                L, N_TICKS, init_sep);

    // Task 1: binding
    std::printf("--- Task 1: Color configuration vs binding ---\n");
    std::printf("  (smaller RMS separation = tighter binding)\n\n");

    std::array<std::pair<std::string, std::array<QuarkSpec, 3>>, 3> configs = {{
        {"(A) RGB  [SU(3) singlet candidate]",
         std::array<QuarkSpec, 3>{{
             {+1, 1, +1, tri[0]},   // R
             {+1, 2, +1, tri[1]},   // G
             {+1, 3, +1, tri[2]},   // B
         }}},
        {"(B) RRR  [single color, not singlet]",
         std::array<QuarkSpec, 3>{{
             {+1, 1, +1, tri[0]},   // R
             {+1, 1, +1, tri[1]},   // R
             {+1, 1, +1, tri[2]},   // R
         }}},
        {"(C) RGG  [mixed non-singlet]",
         std::array<QuarkSpec, 3>{{
             {+1, 1, +1, tri[0]},   // R
             {+1, 2, +1, tri[1]},   // G
             {+1, 2, +1, tri[2]},   // G
         }}},
    }};

    std::array<double, 3> final_seps;
    for (int i = 0; i < 3; ++i) {
        final_seps[i] = stamp_and_run(configs[i].second, L, N_TICKS, 0xB01A2001 + i);
        std::printf("  %-45s  final RMS sep = %.3f\n",
                    configs[i].first.c_str(), final_seps[i]);
    }

    std::printf("\n  Binding ratio (init / final):\n");
    for (int i = 0; i < 3; ++i) {
        const double ratio = (final_seps[i] > 1e-6) ? init_sep / final_seps[i] : 0.0;
        std::printf("    %-45s  ratio = %.3f  (>1 = tighter, <1 = dispersed)\n",
                    configs[i].first.c_str(), ratio);
    }

    // Task 2: color transformation under lattice rotation (120° about body diagonal)
    // xyz → yzx maps color R(1)→G(2)→B(3)→R(1), which IS the identical role-permutation
    // on the three color labels. We test this by measuring the RGB binding
    // AFTER permuting the color assignments by the 120° rotation:
    //   original (R, G, B) at (v1, v2, v3)
    //   permuted (G, B, R) at (v1, v2, v3)  — same positions, permuted color labels
    std::printf("\n--- Task 2: Color transformation under 120° body-diagonal rotation ---\n");
    std::printf("  Compare (R,G,B)-bound system to (G,B,R)-bound system. If FTD color is\n");
    std::printf("  O_h-symmetric, the two should give identical binding; if not, they\n");
    std::printf("  differ. (Continuous SU(3) would also require arbitrary mixings to\n");
    std::printf("  preserve observables — impossible given int8_t storage.)\n\n");

    const std::array<QuarkSpec, 3> permuted = {{
        {+1, 2, +1, tri[0]},   // G
        {+1, 3, +1, tri[1]},   // B
        {+1, 1, +1, tri[2]},   // R
    }};

    const double sep_rgb = final_seps[0];
    const double sep_gbr = stamp_and_run(permuted, L, N_TICKS, 0xB01A2001);
    // Use the same seed as config A above so RNG draws match exactly.

    std::printf("  (R,G,B) configuration final RMS sep = %.3f\n", sep_rgb);
    std::printf("  (G,B,R) configuration final RMS sep = %.3f\n", sep_gbr);
    const double delta = std::abs(sep_rgb - sep_gbr);
    std::printf("  Absolute difference: %.4f\n", delta);

    if (delta < 0.1) {
        std::printf("\n  O_h-permutation symmetry: OBSERVED (color labels interchangeable).\n");
        std::printf("  Compatible with cyclic color subgroup of SU(3), but does NOT confirm\n");
        std::printf("  full continuous SU(3) — the discrete color alphabet structurally\n");
        std::printf("  forbids continuous color mixing.\n");
    } else {
        std::printf("\n  O_h-permutation symmetry: BROKEN (|delta| = %.3f > 0.1).\n", delta);
        std::printf("  Color is asymmetric under lattice rotation — even the finite\n");
        std::printf("  subgroup of SU(3) fails at the dynamical level.\n");
    }

    // Summary verdict
    std::printf("\n================================================================\n");
    std::printf("  PHASE-4i SUMMARY\n");
    std::printf("================================================================\n");

    const bool rgb_bound = final_seps[0] < init_sep * 1.1;
    const bool rrr_disp  = final_seps[1] > init_sep * 1.1;
    if (rgb_bound && rrr_disp) {
        std::printf("  (A) RGB BOUND, (B) RRR DISPERSED  — color has SU(3)-like\n");
        std::printf("      physical content: singlet states bind, non-singlets don't.\n");
    } else if (!rgb_bound && !rrr_disp) {
        std::printf("  Neither RGB nor RRR shows clear differential binding — color\n");
        std::printf("      label has no measurable dynamical effect at this scale.\n");
    } else {
        std::printf("  Binding pattern: RGB %s, RRR %s, RGG %s.\n",
                    rgb_bound ? "bound" : "dispersed",
                    rrr_disp ? "dispersed" : "bound",
                    final_seps[2] < init_sep * 1.1 ? "bound" : "dispersed");
        std::printf("      Color dynamics partially active but not cleanly SU(3)-like.\n");
    }
    std::printf("================================================================\n");

    return 0;
}
