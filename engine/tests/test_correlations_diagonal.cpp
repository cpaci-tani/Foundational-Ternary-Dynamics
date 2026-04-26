/**
 * @file test_correlations_diagonal.cpp
 * @brief Validate sublattice-filtered + diagonal-displacement correlators.
 *
 * T1 — Plane-wave seed: flux = e_x · cos(k · x).
 *      C(r) along AXIS (+x̂) at r=1: <cos(k·x) · cos(k·(x+1))> = ½ cos(k).
 *      C(r) along BODY_DIAG (+1,+1,+1) at r=1: <cos(k·x) · cos(k·(x+1))>
 *      since k=(kx,0,0) → also ½ cos(kx).
 * T2 — Sublattice filter: with BCC_SITES filter, only odd-parity voxels
 *      contribute. On a uniform-magnitude seed the correlator value is unchanged
 *      (averaging is over a different set), but the COUNT must drop to N/8.
 *      This is verified indirectly by checking the value on a filtered uniform field.
 * T3 — sum_flux_energy_sublattice on a uniform field equals N_class · |J|².
 */

#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/correlations.h"
#include "ftd/sublattice.h"

using namespace ftd;

static int failures = 0;

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
    std::printf("  Diagonal + sublattice-filtered correlator tests\n");
    std::printf("================================================================\n");

    const int L = 16;
    RenderBridge rb(L);
    rb.force_cpu();

    // === T1: Plane-wave seed, axis vs body-diagonal correlators ===
    const double k = 3.141592653589793 / 4.0;
    {
        // Set flux.x = cos(k * x). flux.y, flux.z = 0. Other voxel state untouched.
        // Bypass the engine — use voxel access directly via a const_cast on the
        // mutable internal vector (we only modify flux for the seed).
        // The test is read-only otherwise.
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    int i = rb.lattice().index(x, y, z);
                    // Use the public seed helper if exposed; fallback: const_cast voxels.
                    auto& vox_mut = const_cast<std::vector<Voxel>&>(rb.voxels());
                    vox_mut[i].flux = Vec3(std::cos(k * x), 0.0, 0.0);
                }

        // Axis correlator
        auto C_axis = spatial_flux_correlation_sublattice(rb, SiteClass::ALL_SITES,
                                                            DisplacementMode::AXIS, 4);
        // <cos(k x) cos(k(x+1))>_x averaged over a full period = ½ cos(k).
        // The averaging here is over all 3 axes. For y or z direction with f only
        // depending on x: <cos(k x) cos(k x)> = ½ for all r. Mixed sum:
        //   x-direction: ½ cos(k r)
        //   y-direction: ½
        //   z-direction: ½
        // Average of 3: (½ cos(k r) + ½ + ½)/3 = (cos(k r) + 2)/6.
        double expect_axis_r1 = (std::cos(k) + 2.0) / 6.0;
        CHECK_NEAR(C_axis[1], expect_axis_r1, 1e-6, "AXIS C(1) on cos(kx) seed");

        // Body-diagonal correlator. For displacement (±1,±1,±1) at r=1,
        // f at (x±1, y±1, z±1) = cos(k(x±1)) — independent of y,z.
        // <cos(k x) cos(k(x±1))> = ½ cos(k). Same for all 4 representative dirs.
        auto C_body = spatial_flux_correlation_sublattice(rb, SiteClass::ALL_SITES,
                                                            DisplacementMode::BODY_DIAG, 4);
        double expect_body_r1 = 0.5 * std::cos(k);
        CHECK_NEAR(C_body[1], expect_body_r1, 1e-6, "BODY_DIAG C(1) on cos(kx) seed");

        // r=0 must equal <|f|²> = ½ for both modes (cos²-averaged).
        CHECK_NEAR(C_axis[0], 0.5, 1e-6, "AXIS C(0) = <|f|²> = ½");
        CHECK_NEAR(C_body[0], 0.5, 1e-6, "BODY_DIAG C(0) = ½");
    }

    // === T2: Sublattice filter + uniform field ===
    {
        const Vec3 J0(1.0, 0.5, -0.25);
        for (int i = 0; i < (int)rb.voxels().size(); ++i) {
            auto& vox_mut = const_cast<std::vector<Voxel>&>(rb.voxels());
            vox_mut[i].flux = J0;
        }
        // On a uniform field, every C(r) = J0·J0 regardless of filter.
        const double dot = J0.x*J0.x + J0.y*J0.y + J0.z*J0.z;
        auto C_all  = spatial_flux_correlation_sublattice(rb, SiteClass::ALL_SITES,
                                                           DisplacementMode::BODY_DIAG, 3);
        auto C_bcc  = spatial_flux_correlation_sublattice(rb, SiteClass::BCC_SITES,
                                                           DisplacementMode::BODY_DIAG, 3);
        // For body-diagonal displacement at integer r, parity flips: BCC -> BCC iff r is even.
        // r=0 (self): C = |J0|².
        // r=1: BCC → SC (parity flip per coordinate). Filter rejects → counts=0 → C[1]=0 (default).
        // r=2: BCC → BCC again. C = |J0|².
        CHECK_NEAR(C_all[0], dot, 1e-12, "ALL_SITES BODY_DIAG C(0) = |J|²");
        CHECK_NEAR(C_bcc[0], dot, 1e-12, "BCC_SITES BODY_DIAG C(0) = |J|²");
        CHECK_NEAR(C_bcc[2], dot, 1e-12, "BCC_SITES BODY_DIAG C(2) = |J|² (even step preserves parity)");
        // r=1: BCC body-diagonal lands on SC sites; second-leg filter rejects → C=0.
        CHECK_NEAR(C_bcc[1], 0.0, 1e-12, "BCC_SITES BODY_DIAG C(1) = 0 (parity flip rejected by filter)");
    }

    // === T3: sum_flux_energy_sublattice on uniform field ===
    {
        // Field already uniform from T2; |J0|² = 1.0 + 0.25 + 0.0625 = 1.3125.
        const Vec3 J0(1.0, 0.5, -0.25);
        const double e = J0.mag2();
        const long long N = static_cast<long long>(L) * L * L;
        double total_all = sum_flux_energy_sublattice(rb, SiteClass::ALL_SITES);
        double total_bcc = sum_flux_energy_sublattice(rb, SiteClass::BCC_SITES);
        double total_sc  = sum_flux_energy_sublattice(rb, SiteClass::SC_SITES);
        CHECK_NEAR(total_all, N * e, 1e-9, "sum energy ALL = N·|J|²");
        CHECK_NEAR(total_bcc, (N / 8) * e, 1e-9, "sum energy BCC = (N/8)·|J|²");
        CHECK_NEAR(total_sc,  (N / 8) * e, 1e-9, "sum energy SC = (N/8)·|J|²");
    }

    std::printf("================================================================\n");
    std::printf("  Result: %s (%d failure(s))\n",
                failures == 0 ? "PASS" : "FAIL", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
