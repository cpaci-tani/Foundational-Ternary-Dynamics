/**
 * Phase B.3 RESONANCE final test: state-only injection at 8 BCC corner positions.
 *
 * Prior tests showed radial-outward FLUX injection at 8 corners triggers
 * cascade flooding (the flux drives outward expansion). This test removes
 * the flux entirely and sets s=+1 at the 8 corners by direct state-injection
 * (via inject_particle, which sets state without driving flux expansion).
 *
 * If the engine's dynamics support an 8-corner BCC bound state as a STATIC
 * configuration, this test will show it: the 8 voxels stay manifested
 * indefinitely without flux-driven cascade.
 *
 * If the 8 corners decay (no flux to maintain manifestation), the engine
 * cannot support a flux-free bound state — the 8-corner configuration is
 * a flooding seed under flux and a decay seed under no flux. There is no
 * "between" regime.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct Voxel3D { int x, y, z; };

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

struct R {
    int L;
    int n_init;
    int n_t100;
    int n_t300;
    int n_final;
    int original_corners_lit;
    bool floods;
    bool decays;
};

static R run_one(int L, int N_TICKS) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;

    // State-only injection: set s=+1 at 8 BCC corner positions with NO flux
    const int c = L / 2;
    std::vector<Voxel3D> corners;
    for (int dx : {-1, 1}) for (int dy : {-1, 1}) for (int dz : {-1, 1}) {
        int x = c + dx, y = c + dy, z = c + dz;
        // inject_particle sets state directly with given flux (we use {0,0,0} for state-only)
        rb.inject_particle(x, y, z, +1, {0.0, 0.0, 0.0});
        corners.push_back({x, y, z});
    }

    R r;
    r.L = L;
    r.n_init = count_manifested(rb);

    for (int t = 1; t <= 100; ++t) rb.tick();
    r.n_t100 = count_manifested(rb);

    for (int t = 101; t <= 300; ++t) rb.tick();
    r.n_t300 = count_manifested(rb);

    for (int t = 301; t <= N_TICKS; ++t) rb.tick();
    r.n_final = count_manifested(rb);

    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    int still_lit = 0;
    for (const auto& corner : corners) {
        int idx = lat.index(corner.x, corner.y, corner.z);
        if (vox[idx].state != 0) ++still_lit;
    }
    r.original_corners_lit = still_lit;
    r.floods = (r.n_final > 100);
    r.decays = (r.n_final < 4);
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  STATE-ONLY injection at 8 BCC corner positions (no flux)\n";
    std::cout << "================================================================\n\n";
    std::cout << "Test: set s=+1 at 8 corners of size-1 sub-cube (zero flux)\n";
    std::cout << "Question: does engine sustain bare 8-corner state as static configuration?\n";
    std::cout << "Toggle: engine defaults + color_forces + triad_binding\n\n";

    const int N_TICKS = 600;
    std::vector<int> L_vals = {32, 48, 64};

    std::cout << "  L     n_init   n_t100   n_t300   n_final   8_lit   verdict\n";
    std::cout << "  ----  ------   ------   ------   -------   -----   -------\n";

    std::vector<R> all;
    for (int L : L_vals) {
        R r = run_one(L, N_TICKS);
        all.push_back(r);
        std::string verdict;
        if (r.floods) verdict = "FLOODED";
        else if (r.decays) verdict = "DECAYED";
        else if (r.n_final == 8 && r.original_corners_lit == 8) verdict = "BOUND";
        else if (r.original_corners_lit == 8) verdict = "EXPANDING_KEPT_8";
        else verdict = "OTHER";
        std::cout << "  " << std::setw(4) << r.L << "  "
                  << std::setw(6) << r.n_init << "   "
                  << std::setw(6) << r.n_t100 << "   "
                  << std::setw(6) << r.n_t300 << "   "
                  << std::setw(7) << r.n_final << "   "
                  << std::setw(5) << r.original_corners_lit << "   "
                  << verdict << "\n";
    }

    int n_bound = 0, n_decay = 0, n_flood = 0;
    for (const auto& r : all) {
        if (r.floods) ++n_flood;
        else if (r.decays) ++n_decay;
        else if (r.n_final == 8 && r.original_corners_lit == 8) ++n_bound;
    }

    std::cout << "\n--- Verdict ---\n";
    std::cout << "  BOUND (exactly 8 voxels, all originals): " << n_bound << " / 3\n";
    std::cout << "  DECAYED: " << n_decay << " / 3\n";
    std::cout << "  FLOODED: " << n_flood << " / 3\n\n  ";

    if (n_bound == 3) {
        std::cout << "[VERDICT] BOUND state DISCOVERED. State-only 8-corner BCC injection\n";
        std::cout << "  produces an L-invariant bound cluster. The flux-free O_h-symmetric\n";
        std::cout << "  configuration is the engine's first identified true bound state!\n";
    } else if (n_decay == 3) {
        std::cout << "[VERDICT] State-only 8-corner injection decays at all L. The bare\n";
        std::cout << "  s=+1 configuration without flux cannot sustain itself. The engine\n";
        std::cout << "  needs flux to maintain manifestation, but flux drives expansion.\n";
        std::cout << "  Definitive negative: NO bound state for 8-corner BCC orbit under\n";
        std::cout << "  any tested injection geometry.\n";
    } else {
        std::cout << "[VERDICT] Mixed outcomes — see per-L data.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}
