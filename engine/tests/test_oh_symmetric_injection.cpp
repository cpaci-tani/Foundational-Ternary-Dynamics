/**
 * Phase B.3 RESONANCE follow-up: O_h-symmetric injection at BCC corner positions.
 *
 * Per polymath agent recommendation (path-3): instead of injecting at a single
 * lattice point and waiting for the engine to spontaneously find an O_h-orbit
 * cluster (which both retractions §5.6.11 + §5.6.17 showed it does NOT do),
 * INJECT directly at 8 BCC corner positions simultaneously with O_h-symmetric
 * flux. Then test whether the engine PRESERVES the O_h symmetry and sustains
 * a localized cluster.
 *
 * Injection geometry: 8 corners of size-1 sub-cube centered at lattice center.
 * For L=32: (15,15,15), (15,15,17), (15,17,15), (17,15,15), (15,17,17),
 *           (17,15,17), (17,17,15), (17,17,17). Each receives radial-outward
 * flux of magnitude A_per_voxel·K_GENESIS, where the direction is the
 * normalized displacement from center.
 *
 * For O_h symmetry preservation:
 *   - All 8 corners equivalent under O_h
 *   - Radial outward flux respects O_h (and is consistent with cluster
 *     "expansion" energy)
 *   - Test if engine maintains 8-voxel cluster + symmetric configuration
 *
 * This is the FIRST test that constructs the O_h orbit BY INITIAL CONDITION
 * rather than waiting for nucleation. Critical for distinguishing "engine
 * supports O_h-symmetric cluster as stable bound state" from "engine never
 * spontaneously produces O_h-symmetric configurations".
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <unordered_set>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static const double TWOPI = 2.0 * 3.14159265358979323846;

struct Voxel3D { int x, y, z; };

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

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

static double compute_rms(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int64_t total = lat.total_sites();
    int n = 0;
    double sx_x=0, cx_x=0, sx_y=0, cx_y=0, sx_z=0, cx_z=0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        ++n;
        auto c = lat.coord(static_cast<int>(i));
        sx_x += std::sin(TWOPI*c.x/L); cx_x += std::cos(TWOPI*c.x/L);
        sx_y += std::sin(TWOPI*c.y/L); cx_y += std::cos(TWOPI*c.y/L);
        sx_z += std::sin(TWOPI*c.z/L); cx_z += std::cos(TWOPI*c.z/L);
    }
    if (n == 0) return 0.0;
    double cx = std::atan2(sx_x, cx_x) * L / TWOPI; if (cx < 0) cx += L;
    double cy = std::atan2(sx_y, cx_y) * L / TWOPI; if (cy < 0) cy += L;
    double cz = std::atan2(sx_z, cx_z) * L / TWOPI; if (cz < 0) cz += L;
    auto wrap = [L](double d) { if (d>L/2.0) d-=L; if (d<-L/2.0) d+=L; return d; };
    double rms = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        auto c = lat.coord(static_cast<int>(i));
        double dx = wrap(c.x - cx), dy = wrap(c.y - cy), dz = wrap(c.z - cz);
        rms += dx*dx + dy*dy + dz*dz;
    }
    return std::sqrt(rms / n);
}

struct R {
    int L;
    double A_per_voxel;
    int n_init;
    int n_t100;
    int n_t300;
    int n_final;
    double rms_final;
    bool symmetric_at_end;     // does cluster still occupy 8 corner positions?
    int original_corners_still_lit;
};

// Inject at 8 BCC-corner positions of size-1 sub-cube, with radial-outward flux
static std::vector<Voxel3D> inject_oh_corners(ftd::RenderBridge& rb, double A_per_voxel) {
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int c = L / 2;
    const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
    std::vector<Voxel3D> corners;
    for (int dx : {-1, 1}) for (int dy : {-1, 1}) for (int dz : {-1, 1}) {
        int x = c + dx, y = c + dy, z = c + dz;
        // Radial outward flux (normalized direction × magnitude)
        double fx = dx * inv_sqrt3 * A_per_voxel * ftd::K_GENESIS;
        double fy = dy * inv_sqrt3 * A_per_voxel * ftd::K_GENESIS;
        double fz = dz * inv_sqrt3 * A_per_voxel * ftd::K_GENESIS;
        rb.inject_flux(x, y, z, {fx, fy, fz});
        corners.push_back({x, y, z});
    }
    return corners;
}

static R run_one(int L, double A_per_voxel, int N_TICKS) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;

    auto corners = inject_oh_corners(rb, A_per_voxel);

    R r;
    r.L = L;
    r.A_per_voxel = A_per_voxel;
    // Tick 0: post-injection, pre-evolution
    r.n_init = count_manifested(rb);

    for (int t = 1; t <= 100; ++t) rb.tick();
    r.n_t100 = count_manifested(rb);

    for (int t = 101; t <= 300; ++t) rb.tick();
    r.n_t300 = count_manifested(rb);

    for (int t = 301; t <= N_TICKS; ++t) rb.tick();
    r.n_final = count_manifested(rb);
    r.rms_final = compute_rms(rb);

    // Check if original 8 corners are still manifested
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    int still_lit = 0;
    for (const auto& corner : corners) {
        int idx = lat.index(corner.x, corner.y, corner.z);
        if (vox[idx].state != 0) ++still_lit;
    }
    r.original_corners_still_lit = still_lit;

    // Check symmetry: do final manifested voxels still form an O_h-symmetric set?
    // Quick proxy: are exactly 8 voxels manifested AND all 8 originals still lit?
    r.symmetric_at_end = (r.n_final == 8 && still_lit == 8);
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  O_h-SYMMETRIC INJECTION at 8 BCC corner positions\n";
    std::cout << "================================================================\n\n";
    std::cout << "Geometry: corners of size-1 sub-cube centered at lattice center\n";
    std::cout << "Flux: radial outward, magnitude A_per_voxel · K_GENESIS\n";
    std::cout << "Toggle: engine defaults + color_forces + triad_binding\n";
    std::cout << "Test: does engine PRESERVE O_h symmetry under direct construction?\n\n";

    const int N_TICKS = 600;

    std::vector<int> L_vals = {32, 48, 64};
    std::vector<double> A_per_voxel_vals = {0.5, 1.0, 2.0, 3.0, 5.0};

    std::cout << "  L    A/voxel   n_init   n_t100   n_t300   n_final   8-orig?   rms_f    sym?\n";
    std::cout << "  ---  -------   ------   ------   ------   -------   -------   -----    ----\n";

    std::vector<R> all;
    for (int L : L_vals) {
        for (double A : A_per_voxel_vals) {
            R r = run_one(L, A, N_TICKS);
            all.push_back(r);
            std::cout << "  " << std::setw(3) << r.L << "   "
                      << std::fixed << std::setprecision(2) << std::setw(5) << r.A_per_voxel << "    "
                      << std::setw(6) << r.n_init << "   "
                      << std::setw(6) << r.n_t100 << "   "
                      << std::setw(6) << r.n_t300 << "   "
                      << std::setw(7) << r.n_final << "   "
                      << std::setw(7) << r.original_corners_still_lit << "   "
                      << std::setw(5) << std::setprecision(2) << r.rms_final << "    "
                      << (r.symmetric_at_end ? "YES" : "no") << "\n";
        }
    }

    // Tally
    std::cout << "\n--- Verdict ---\n";
    int n_symmetric = 0;
    int n_8_original = 0;
    for (const auto& r : all) {
        if (r.symmetric_at_end) ++n_symmetric;
        if (r.original_corners_still_lit == 8) ++n_8_original;
    }
    std::cout << "  Configurations preserving full O_h-orbit symmetry: " << n_symmetric << " / " << all.size() << "\n";
    std::cout << "  Configurations keeping all 8 original corners lit: " << n_8_original << " / " << all.size() << "\n";

    if (n_symmetric > 0) {
        std::cout << "\n  [VERDICT] Direct O_h-symmetric injection PRESERVED in some configurations.\n";
        std::cout << "  This is the first evidence that the engine SUPPORTS an O_h-symmetric stable\n";
        std::cout << "  cluster — but only when constructed by initial condition, not by nucleation.\n";
        std::cout << "  Phase B.4 candidate: this configuration is a candidate 'BCC corner soliton'.\n";
    } else if (n_8_original > 0) {
        std::cout << "\n  [VERDICT] Original corners stay lit but symmetry partially broken (extras).\n";
        std::cout << "  Engine partially preserves the structure but allows growth.\n";
    } else {
        std::cout << "\n  [VERDICT] Direct O_h-symmetric injection NOT preserved at any tested config.\n";
        std::cout << "  The engine actively breaks O_h symmetry under +color+triad dynamics.\n";
        std::cout << "  Confirms that the discrete substrate does not support classical bound states\n";
        std::cout << "  even when initial conditions explicitly construct them.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}
