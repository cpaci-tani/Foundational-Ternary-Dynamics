/**
 * Phase B.3 RESONANCE refinement: threshold-amplitude scan for O_h-symmetric injection.
 *
 * Prior O_h-injection test (`test_oh_symmetric_injection.cpp`) showed that
 * at A_per_voxel ≥ 2.0 the 8 corners remain lit but the cluster floods due
 * to outward-radial-flux-driven cascade. At A_per_voxel = 1.0 only 1-7 of 8
 * corners stay lit (borderline). At A_per_voxel = 0.5 nothing manifests.
 *
 * This test finely scans A_per_voxel ∈ [0.6, 1.2] to find the regime where:
 *   - All 8 corners stay lit (above threshold for genesis)
 *   - But cascade flooding does NOT trigger (below threshold for runaway)
 *
 * If such a regime exists at any L, the 8-corner BCC bound state IS sustainable
 * under O_h-symmetric initial conditions when the flux is just-suprathreshold.
 *
 * If no such regime exists, the 8-corner configuration is fundamentally
 * unstable — either decays (low A) or floods (high A).
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
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

static std::vector<Voxel3D> inject_oh_corners(ftd::RenderBridge& rb, double A_per_voxel) {
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int c = L / 2;
    const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
    std::vector<Voxel3D> corners;
    for (int dx : {-1, 1}) for (int dy : {-1, 1}) for (int dz : {-1, 1}) {
        int x = c + dx, y = c + dy, z = c + dz;
        double fx = dx * inv_sqrt3 * A_per_voxel * ftd::K_GENESIS;
        double fy = dy * inv_sqrt3 * A_per_voxel * ftd::K_GENESIS;
        double fz = dz * inv_sqrt3 * A_per_voxel * ftd::K_GENESIS;
        rb.inject_flux(x, y, z, {fx, fy, fz});
        corners.push_back({x, y, z});
    }
    return corners;
}

struct R {
    int L;
    double A_per_voxel;
    int n_init;
    int n_t50;
    int n_t150;
    int n_final;
    int orig_corners_lit;
    bool flooded;
};

static R run_one(int L, double A_per_voxel, int N_TICKS) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;

    auto corners = inject_oh_corners(rb, A_per_voxel);

    R r;
    r.L = L;
    r.A_per_voxel = A_per_voxel;
    r.n_init = count_manifested(rb);

    for (int t = 1; t <= 50; ++t) rb.tick();
    r.n_t50 = count_manifested(rb);

    for (int t = 51; t <= 150; ++t) rb.tick();
    r.n_t150 = count_manifested(rb);

    for (int t = 151; t <= N_TICKS; ++t) rb.tick();
    r.n_final = count_manifested(rb);

    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    int still_lit = 0;
    for (const auto& corner : corners) {
        int idx = lat.index(corner.x, corner.y, corner.z);
        if (vox[idx].state != 0) ++still_lit;
    }
    r.orig_corners_lit = still_lit;
    r.flooded = (r.n_final > 100);
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  O_h-symmetric injection: threshold-amplitude scan\n";
    std::cout << "================================================================\n\n";
    std::cout << "Goal: find A_per_voxel that lights all 8 corners WITHOUT cascade flood\n\n";

    const int N_TICKS = 600;

    std::vector<int> L_vals = {32, 48, 64};
    std::vector<double> A_vals;
    for (double A = 0.5; A <= 1.5001; A += 0.05) A_vals.push_back(A);

    std::cout << "  L    A/vox    n_init   n_t50    n_t150   n_final   8_lit   regime\n";
    std::cout << "  ---  ------   ------   ------   ------   -------   -----   -------\n";

    std::vector<R> all;
    for (int L : L_vals) {
        for (double A : A_vals) {
            R r = run_one(L, A, N_TICKS);
            all.push_back(r);
            std::string regime;
            if (r.n_init == 0 && r.n_final == 0) regime = "NO_GENESIS";
            else if (r.flooded) regime = "FLOODED";
            else if (r.orig_corners_lit == 8 && r.n_final >= 8 && r.n_final < 30) regime = "BOUND_8_CORNERS";
            else if (r.orig_corners_lit < 8) regime = "PARTIAL_DECAY";
            else regime = "OTHER";
            std::cout << "  " << std::setw(3) << r.L << "   "
                      << std::fixed << std::setprecision(2) << std::setw(4) << r.A_per_voxel << "    "
                      << std::setw(6) << r.n_init << "   "
                      << std::setw(6) << r.n_t50 << "   "
                      << std::setw(6) << r.n_t150 << "   "
                      << std::setw(7) << r.n_final << "   "
                      << std::setw(5) << r.orig_corners_lit << "   "
                      << regime << "\n";
        }
    }

    // Find any BOUND_8_CORNERS configurations
    std::vector<R> bound_configs;
    for (const auto& r : all) {
        if (r.orig_corners_lit == 8 && !r.flooded && r.n_final >= 8 && r.n_final < 30) {
            bound_configs.push_back(r);
        }
    }

    std::cout << "\n--- Verdict ---\n";
    std::cout << "  Configurations with 8 corners lit + n_final < 30 (no cascade): "
              << bound_configs.size() << " / " << all.size() << "\n";
    if (!bound_configs.empty()) {
        std::cout << "\n  [BOUND CANDIDATES] " << bound_configs.size() << " (toggle, A, L) configs sustain\n";
        std::cout << "  the 8-corner BCC orbit without flooding. List:\n";
        for (const auto& r : bound_configs) {
            std::cout << "    L=" << r.L << "  A_per_voxel=" << std::fixed << std::setprecision(2) << r.A_per_voxel
                      << "  n_final=" << r.n_final << "  8_corners_lit=" << r.orig_corners_lit << "\n";
        }
        std::cout << "\n  This is potential evidence that an O_h-symmetric BCC corner cluster\n";
        std::cout << "  IS sustainable when given the right initial conditions — the engine\n";
        std::cout << "  doesn't spontaneously nucleate it but doesn't actively break it either.\n";
        std::cout << "  Worth multi-seed verification + spatial-configuration check.\n";
    } else {
        std::cout << "\n  [NEGATIVE] No configuration sustains 8-corner BCC orbit without flooding.\n";
        std::cout << "  The radial-outward-flux injection geometry is fundamentally a flooding seed.\n";
        std::cout << "  Try alternate injection geometries (tangential flux, no flux + s-injection).\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}
