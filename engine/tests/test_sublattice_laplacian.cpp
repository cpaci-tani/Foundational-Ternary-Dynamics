/**
 * @file test_sublattice_laplacian.cpp
 * @brief Validate sublattice-projected Laplacians (laplacian_sc, _fcc, _bcc).
 *
 * Three tests:
 *
 *   T1 — Taylor consistency on f(x,y,z) = x²+y²+z².
 *        Continuum ∇²f = 6. Discrete result for each stencil:
 *          SC:  Σ((x±1)²+y²+z²)/6 + perm − (x²+y²+z²) = 1     (per-stencil scaling)
 *          FCC: 12 edge sum / 12 − center = 2 (factor 2 from √2 distance)
 *          BCC: 8 corner sum / 8 − center = 3 (factor 3 from √3 distance)
 *        We verify these per-stencil scaling factors so spectrum extractors
 *        can divide them out for calibration-invariant ratios.
 *
 *   T2 — Plane-wave eigenvalue. f(x,y,z) = cos(k·x) for k = (π/4, 0, 0).
 *        Each stencil has an analytic eigenvalue; check it.
 *
 *   T3 — Sum-of-weights = 0 (consistency: constant fields produce zero Laplacian).
 *        Verifies the stencils are valid discrete Laplacians.
 */

#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/lattice.h"
#include "ftd/sublattice.h"
#include "ftd/voxel.h"
#include "ftd/ontic/master_quadratic.h"   // D_SPATIAL

using namespace ftd;

static int failures = 0;
static const double TOL = 1e-10;

#define CHECK(cond, msg) do { \
    if (!(cond)) { std::printf("[FAIL] %s\n", msg); ++failures; } \
    else { std::printf("[ ok ] %s\n", msg); } \
} while (0)

#define CHECK_NEAR(actual, expected, tol, msg) do { \
    double err = std::abs((actual) - (expected)); \
    if (err > (tol)) { \
        std::printf("[FAIL] %s  (got %.6g, expected %.6g, err %.3g)\n", \
                    msg, (double)(actual), (double)(expected), err); \
        ++failures; \
    } else { \
        std::printf("[ ok ] %s  (= %.6g)\n", msg, (double)(actual)); \
    } \
} while (0)

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Sublattice Laplacian tests (SC / FCC / BCC)\n");
    std::printf("================================================================\n");

    const int L = 16;
    Lattice lat(L);
    std::vector<Voxel> voxels(lat.total_sites());

    // Choose a test site away from the boundary so wrap doesn't bite.
    const int xc = 8, yc = 8, zc = 8;
    const int idx = lat.index(xc, yc, zc);

    // ===== T1: Taylor consistency on f = x²+y²+z² =====
    // Use Vec3 storage: pack the scalar f into Vec3.x. Set y, z = 0.
    auto fill_quadratic = [&](double base) {
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    int i = lat.index(x, y, z);
                    // Use centered coords (x-xc, y-yc, z-zc) so the test point is the origin.
                    double dx = x - xc, dy = y - yc, dz = z - zc;
                    voxels[i].flux = Vec3(base + dx*dx + dy*dy + dz*dz, 0.0, 0.0);
                }
    };
    fill_quadratic(0.0);

    // Continuum ∇²(x²+y²+z²) = 6. Each per-stencil result has predictable scaling.
    Vec3 lap_sc  = laplacian_sc <&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_fcc = laplacian_fcc<&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_bcc = laplacian_bcc<&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_full = laplacian_sublattice<&Voxel::flux>(BccStencilMode::FULL, voxels, lat, idx);

    // For f(r) = |r|² = x²+y²+z², continuum ∇²f = 2D = 2·D_SPATIAL.
    // The discrete Laplacians equal the squared-distance to a representative
    // neighbor (averaging cancels parity, so each per-stencil contribution
    // equals one nbr's squared distance):
    //   SC face nbr at (±1,0,0)            → |Δr|² = 1
    //   FCC edge nbr at (±1,±1,0)          → |Δr|² = 2 = D_SPATIAL − 1
    //   BCC corner nbr at (±1,±1,±1)       → |Δr|² = 3 = D_SPATIAL
    constexpr int D = ftd::ontic::D_SPATIAL;
    constexpr double EXPECT_SC  = 1.0;            // by construction (one axis squared)
    constexpr double EXPECT_FCC = static_cast<double>(D - 1);   // 2
    constexpr double EXPECT_BCC = static_cast<double>(D);       // 3
    constexpr double EXPECT_FULL = 2.0 * static_cast<double>(D); // 6 = continuum ∇²f

    CHECK_NEAR(lap_sc.x,   EXPECT_SC,   TOL, "SC Laplacian on f=|r|² → 1 (= 1)");
    CHECK_NEAR(lap_fcc.x,  EXPECT_FCC,  TOL, "FCC Laplacian on f=|r|² → 2 (= D−1)");
    CHECK_NEAR(lap_bcc.x,  EXPECT_BCC,  TOL, "BCC Laplacian on f=|r|² → 3 (= D)");
    CHECK_NEAR(lap_full.x, EXPECT_FULL, TOL, "FULL Laplacian on f=|r|² → 6 (= 2D = continuum ∇²f)");

    // ===== T2: Plane-wave eigenvalue on cos(k·x) =====
    // For each stencil, eigenvalue formula:
    //   SC:  λ_SC  = (1/6) · Σ_{6 face} cos(k·δ) − 1
    //              = (cos kx + cos ky + cos kz)/3 − 1
    //   FCC: λ_FCC = (1/12) · Σ_{12 edge} cos(k·δ) − 1
    //              = (cos kx cos ky + cos ky cos kz + cos kz cos kx)·(2/12)·... ;
    //              expand: at k = (kx, 0, 0), edge nbrs decompose to 4 with δ=(1,1,0),
    //              4 with (1,0,1), 4 with (0,1,1). cos(k·δ) for k=(kx,0,0):
    //              4·cos(kx) + 4·cos(kx) + 4·1 = 8 cos(kx) + 4 → divide by 12: (2cos kx + 1)/3.
    //              Subtract 1: (2cos kx − 2)/3.
    //   BCC: λ_BCC = cos(kx)·cos(ky)·cos(kz) − 1 = cos(kx) − 1 at k=(kx,0,0).
    const double kx = 3.141592653589793 / 4.0;
    auto fill_plane_wave = [&]() {
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    int i = lat.index(x, y, z);
                    double dx = x - xc;
                    voxels[i].flux = Vec3(std::cos(kx * dx), 0.0, 0.0);
                }
    };
    fill_plane_wave();
    // f at center = cos(0) = 1.
    Vec3 lap_sc_pw  = laplacian_sc <&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_fcc_pw = laplacian_fcc<&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_bcc_pw = laplacian_bcc<&Voxel::flux>(voxels, lat, idx);

    const double c = std::cos(kx);
    // SC: 6 face nbrs at (±1,0,0)(0,±1,0)(0,0,±1). For k=(kx,0,0):
    //   2 nbrs at ±x: cos(±kx) = cos(kx) → 2c
    //   4 nbrs at ±y, ±z: cos(0) = 1     → 4
    // Sum = 2c+4. Divide by 6: (2c+4)/6. Subtract f(0)=1 → (2c+4)/6 − 1 = (c−1)/3.
    const double expect_sc  = (c - 1.0) / 3.0;

    // FCC: 12 edge nbrs. For k=(kx,0,0):
    //   4 nbrs at (±1,±1,0): cos(±kx) = c → 4c
    //   4 nbrs at (±1,0,±1): cos(±kx) = c → 4c
    //   4 nbrs at (0,±1,±1): cos(0) = 1   → 4
    // Sum = 8c+4. Divide by 12: (8c+4)/12 = (2c+1)/3. Subtract 1 → (2c−2)/3.
    const double expect_fcc = (2.0*c - 2.0) / 3.0;

    // BCC: 8 corners at (±1,±1,±1). k·δ = ±kx for k=(kx,0,0). cos(±kx) = c.
    // Sum = 8c. Divide by 8 → c. Subtract f(0)=1 → c − 1.
    const double expect_bcc = c - 1.0;

    CHECK_NEAR(lap_sc_pw.x,  expect_sc,  1e-9, "SC Laplacian plane-wave eigenvalue");
    CHECK_NEAR(lap_fcc_pw.x, expect_fcc, 1e-9, "FCC Laplacian plane-wave eigenvalue");
    CHECK_NEAR(lap_bcc_pw.x, expect_bcc, 1e-9, "BCC Laplacian plane-wave eigenvalue");

    // ===== T3: Constant field → zero Laplacian (sum-of-weights = 0) =====
    auto fill_constant = [&](double v) {
        for (auto& vox : voxels) vox.flux = Vec3(v, v, v);
    };
    fill_constant(3.7);
    Vec3 lap_sc_const  = laplacian_sc <&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_fcc_const = laplacian_fcc<&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_bcc_const = laplacian_bcc<&Voxel::flux>(voxels, lat, idx);
    Vec3 lap_full_const = laplacian_sublattice<&Voxel::flux>(BccStencilMode::FULL, voxels, lat, idx);
    CHECK_NEAR(lap_sc_const.x,   0.0, TOL, "SC  Lap on constant field = 0");
    CHECK_NEAR(lap_fcc_const.x,  0.0, TOL, "FCC Lap on constant field = 0");
    CHECK_NEAR(lap_bcc_const.x,  0.0, TOL, "BCC Lap on constant field = 0");
    CHECK_NEAR(lap_full_const.x, 0.0, TOL, "FULL Lap on constant field = 0");

    // ===== T4: Dispatch wrapper agrees with direct kernels =====
    fill_quadratic(0.0);
    Vec3 disp_sc  = laplacian_sublattice<&Voxel::flux>(BccStencilMode::SC,  voxels, lat, idx);
    Vec3 disp_fcc = laplacian_sublattice<&Voxel::flux>(BccStencilMode::FCC, voxels, lat, idx);
    Vec3 disp_bcc = laplacian_sublattice<&Voxel::flux>(BccStencilMode::BCC, voxels, lat, idx);
    CHECK_NEAR(disp_sc.x,  EXPECT_SC,  TOL, "dispatch wrapper: SC mode");
    CHECK_NEAR(disp_fcc.x, EXPECT_FCC, TOL, "dispatch wrapper: FCC mode");
    CHECK_NEAR(disp_bcc.x, EXPECT_BCC, TOL, "dispatch wrapper: BCC mode");

    std::printf("================================================================\n");
    std::printf("  Result: %s (%d failure(s))\n",
                failures == 0 ? "PASS" : "FAIL", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
