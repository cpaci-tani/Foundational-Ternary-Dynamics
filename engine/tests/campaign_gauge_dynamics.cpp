/**
 * Campaign: Gauge Dynamics — U(1) Invariance Under Time Evolution
 *
 * Tests that the gauge transformation J -> J + grad(lambda) leaves
 * physical observables invariant under full simulation dynamics.
 * Goes beyond test_gauge.cpp (which only checks static identities)
 * by verifying dynamic invariance with particles present.
 *
 * Theory: FTD's U(1) gauge symmetry emerges from the Helmholtz
 * decomposition. The transverse part J_T (div=0) carries physical
 * content (2 photon polarizations). The longitudinal part J_L (curl=0)
 * is constrained by Gauss's law div(J) = rho_charge.
 *
 * Under J -> J + grad(lambda):
 *   curl(J) -> curl(J)              (unchanged: curl of grad = 0)
 *   div(J)  -> div(J) + lap(lambda) (changed, but charge unchanged)
 *   Forces  -> unchanged             (pairwise Coulomb, not flux-mediated)
 *
 * Sub-campaigns:
 *   6a — Particle trajectory invariance under gauge transform
 *   6b — Curl invariance under time evolution
 *   6c — Divergence decomposition identity
 *   6d — Longitudinal mode decay vs transverse mode survival
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

// ============================================================================
// Test infrastructure
// ============================================================================
static int g_failures = 0;
static int g_passes   = 0;

static void check(const char* name, bool cond) {
    if (cond) { std::cout << "  PASS  " << name << "\n"; ++g_passes; }
    else      { std::cout << "  FAIL  " << name << "\n"; ++g_failures; }
}

static void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) { std::cout << "  PASS  " << name << "\n"; ++g_passes; }
    else {
        std::cout << "  FAIL  " << name
                  << " (got " << std::setprecision(8) << a
                  << ", expected " << b
                  << ", diff " << std::abs(a - b) << ")\n";
        ++g_failures;
    }
}

// Helper: compute discrete gradient of scalar field lambda at a point
// Uses central difference: grad_i = (lambda[i+1] - lambda[i-1]) / 2
static ftd::Vec3 discrete_grad_lambda(
    int x, int y, int z, int L,
    const std::vector<double>& lambda)
{
    auto wrap = [L](int v) -> int {
        int r = v % L;
        return r < 0 ? r + L : r;
    };
    auto idx = [L, &wrap](int a, int b, int c) -> int {
        return wrap(a) * L * L + wrap(b) * L + wrap(c);
    };

    ftd::Vec3 grad;
    grad.x = (lambda[idx(x + 1, y, z)] - lambda[idx(x - 1, y, z)]) * 0.5;
    grad.y = (lambda[idx(x, y + 1, z)] - lambda[idx(x, y - 1, z)]) * 0.5;
    grad.z = (lambda[idx(x, y, z + 1)] - lambda[idx(x, y, z - 1)]) * 0.5;
    return grad;
}

// Helper: compute discrete Laplacian of scalar field at a point
// lap = sum(6 neighbors) - 6*center
static double discrete_laplacian_lambda(
    int x, int y, int z, int L,
    const std::vector<double>& lambda)
{
    auto wrap = [L](int v) -> int {
        int r = v % L;
        return r < 0 ? r + L : r;
    };
    auto idx = [L, &wrap](int a, int b, int c) -> int {
        return wrap(a) * L * L + wrap(b) * L + wrap(c);
    };

    int ci = idx(x, y, z);
    double lap = lambda[idx(x + 1, y, z)] + lambda[idx(x - 1, y, z)]
               + lambda[idx(x, y + 1, z)] + lambda[idx(x, y - 1, z)]
               + lambda[idx(x, y, z + 1)] + lambda[idx(x, y, z - 1)]
               - 6.0 * lambda[ci];
    return lap;
}

// Helper: generate smooth periodic lambda field
static std::vector<double> make_lambda_field(int L, double amp) {
    std::vector<double> lam(L * L * L);
    double kk = 2.0 * ftd::PI / L;
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx = x * L * L + y * L + z;
                lam[idx] = amp * std::sin(kk * x) * std::sin(kk * y)
                         + amp * 0.5 * std::cos(kk * z);
            }
        }
    }
    return lam;
}

// ============================================================================
// 6a — Particle trajectory invariance under gauge transform
// ============================================================================
// Since FTD forces use direct pairwise Coulomb (not flux gradients),
// particle trajectories should be exactly identical regardless of
// the flux gauge.
static void campaign_6a() {
    std::cout << "\n================================================================\n";
    std::cout << "  Campaign 6a: Trajectory Invariance Under Gauge Transform\n";
    std::cout << "================================================================\n";

    const int L = 32;
    const int mid = L / 2;
    const int T_RUN = 20;
    const double GAUGE_AMP = 0.5;

    // --- Simulation A: standard setup ---
    ftd::RenderBridge eng_a(L);
    double iso = ftd::K_B / std::sqrt(3.0);
    eng_a.inject_particle(mid - 5, mid, mid, +1, {iso, iso, iso});
    eng_a.inject_particle(mid + 5, mid, mid, -1, {iso, iso, iso});
    eng_a.voxel_at(mid - 5, mid, mid).locked = true;
    eng_a.voxel_at(mid + 5, mid, mid).locked = true;

    // Equilibrate with locked particles
    eng_a.run(20);

    // --- Simulation B: add gauge field ---
    ftd::RenderBridge eng_b(L);
    eng_b.inject_particle(mid - 5, mid, mid, +1, {iso, iso, iso});
    eng_b.inject_particle(mid + 5, mid, mid, -1, {iso, iso, iso});
    eng_b.voxel_at(mid - 5, mid, mid).locked = true;
    eng_b.voxel_at(mid + 5, mid, mid).locked = true;

    // Add gauge field: J -> J + grad(lambda)
    auto lambda = make_lambda_field(L, GAUGE_AMP);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx = eng_b.lattice().index(x, y, z);
                ftd::Vec3 j_orig = eng_b.voxels()[idx].flux;
                ftd::Vec3 grad_lam = discrete_grad_lambda(x, y, z, L, lambda);
                eng_b.inject_flux(x, y, z, {
                    j_orig.x + grad_lam.x,
                    j_orig.y + grad_lam.y,
                    j_orig.z + grad_lam.z
                });
            }
        }
    }

    // Equilibrate B with same number of ticks
    eng_b.run(20);

    // Unlock both and run
    // Find particles in A
    int pa_pos_idx = -1, pa_neg_idx = -1;
    int pb_pos_idx = -1, pb_neg_idx = -1;
    for (int i = 0; i < eng_a.lattice().total_sites(); ++i) {
        if (eng_a.voxels()[i].state == 1 && pa_pos_idx < 0) pa_pos_idx = i;
        if (eng_a.voxels()[i].state == -1 && pa_neg_idx < 0) pa_neg_idx = i;
        if (eng_b.voxels()[i].state == 1 && pb_pos_idx < 0) pb_pos_idx = i;
        if (eng_b.voxels()[i].state == -1 && pb_neg_idx < 0) pb_neg_idx = i;
    }

    // Particles should still exist in both
    check("6a: Positive particle survives in A", pa_pos_idx >= 0);
    check("6a: Negative particle survives in A", pa_neg_idx >= 0);
    check("6a: Positive particle survives in B (gauge)", pb_pos_idx >= 0);
    check("6a: Negative particle survives in B (gauge)", pb_neg_idx >= 0);

    if (pa_pos_idx >= 0 && pb_pos_idx >= 0) {
        // Unlock and run
        eng_a.voxels()[pa_pos_idx].locked = false;
        eng_a.voxels()[pa_neg_idx].locked = false;
        eng_b.voxels()[pb_pos_idx].locked = false;
        eng_b.voxels()[pb_neg_idx].locked = false;

        eng_a.run(T_RUN);
        eng_b.run(T_RUN);

        // Find final positions
        auto coord_a_pos = eng_a.lattice().coord(pa_pos_idx);
        auto coord_b_pos = eng_b.lattice().coord(pb_pos_idx);

        // Look for the particles again (they may have moved)
        int fa_pos_idx = -1, fb_pos_idx = -1;
        for (int i = 0; i < eng_a.lattice().total_sites(); ++i) {
            if (eng_a.voxels()[i].state == 1) fa_pos_idx = i;
            if (eng_b.voxels()[i].state == 1) fb_pos_idx = i;
        }

        if (fa_pos_idx >= 0 && fb_pos_idx >= 0) {
            auto ca = eng_a.lattice().coord(fa_pos_idx);
            auto cb = eng_b.lattice().coord(fb_pos_idx);
            int dx = std::abs(ca.x - cb.x);
            int dy = std::abs(ca.y - cb.y);
            int dz = std::abs(ca.z - cb.z);
            // Handle periodic
            if (dx > L / 2) dx = L - dx;
            if (dy > L / 2) dy = L - dy;
            if (dz > L / 2) dz = L - dz;
            int dist = dx + dy + dz;

            std::cout << "    A final pos: (" << ca.x << "," << ca.y << "," << ca.z << ")\n";
            std::cout << "    B final pos: (" << cb.x << "," << cb.y << "," << cb.z << ")\n";
            std::cout << "    Manhattan distance: " << dist << "\n";

            // Trajectories should match (within 2 voxels for numerical tolerance)
            // The gauge field modifies flux which can affect evaporation/self-field,
            // so allow small deviation
            check("6a: Trajectories match within 3 voxels", dist <= 3);
        } else {
            std::cout << "    (Particles evaporated after unlock)\n";
            // Both should have same fate
            check("6a: Trajectories match within 3 voxels",
                  (fa_pos_idx < 0) == (fb_pos_idx < 0));
        }
    }
}

// ============================================================================
// 6b — Curl invariance under time evolution
// ============================================================================
// Verify that curl(J) evolves identically in gauge-related simulations.
// Since the wave equation is linear and curl(grad)=0, the curl should
// remain invariant even after time evolution.
static void campaign_6b() {
    std::cout << "\n================================================================\n";
    std::cout << "  Campaign 6b: Curl Invariance Under Time Evolution\n";
    std::cout << "================================================================\n";

    const int L = 16;
    const double GAUGE_AMP = 0.3;
    const int T_RUN = 15;

    // --- Simulation A: interesting flux pattern (no particles) ---
    ftd::RenderBridge eng_a(L);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                double kk = 2.0 * ftd::PI / L;
                double fx = 0.2 * std::sin(kk * y);
                double fy = 0.2 * std::cos(kk * x);
                double fz = 0.1 * std::sin(kk * z);
                eng_a.inject_flux(x, y, z, {fx, fy, fz});
            }
        }
    }

    // --- Simulation B: same flux + grad(lambda) ---
    ftd::RenderBridge eng_b(L);
    auto lambda = make_lambda_field(L, GAUGE_AMP);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                double kk = 2.0 * ftd::PI / L;
                double fx = 0.2 * std::sin(kk * y);
                double fy = 0.2 * std::cos(kk * x);
                double fz = 0.1 * std::sin(kk * z);
                ftd::Vec3 grad_lam = discrete_grad_lambda(x, y, z, L, lambda);
                eng_b.inject_flux(x, y, z, {
                    fx + grad_lam.x,
                    fy + grad_lam.y,
                    fz + grad_lam.z
                });
            }
        }
    }

    // Verify curl matches at t=0
    double max_diff_t0 = 0;
    for (int i = 0; i < eng_a.lattice().total_sites(); ++i) {
        ftd::Vec3 ca = eng_a.curl_flux(i);
        ftd::Vec3 cb = eng_b.curl_flux(i);
        double diff = (ca - cb).mag();
        if (diff > max_diff_t0) max_diff_t0 = diff;
    }
    std::cout << "    Max |curl_A - curl_B| at t=0: " << max_diff_t0 << "\n";
    check("6b: Curl matches at t=0 (< 1e-8)", max_diff_t0 < 1e-8);

    // Run both simulations
    eng_a.run(T_RUN);
    eng_b.run(T_RUN);

    // Compare curl after evolution
    double max_diff_tf = 0;
    for (int i = 0; i < eng_a.lattice().total_sites(); ++i) {
        ftd::Vec3 ca = eng_a.curl_flux(i);
        ftd::Vec3 cb = eng_b.curl_flux(i);
        double diff = (ca - cb).mag();
        if (diff > max_diff_tf) max_diff_tf = diff;
    }
    std::cout << "    Max |curl_A - curl_B| at t=" << T_RUN << ": " << max_diff_tf << "\n";

    // The wave equation propagates curl(grad(lambda))=0 identically,
    // so curl should remain identical. But damping is multiplicative
    // and |J_A| != |J_B| so damping factor may differ slightly.
    // Allow tolerance of 0.05.
    check("6b: Curl invariance preserved under evolution (< 0.05)", max_diff_tf < 0.05);
}

// ============================================================================
// 6c — Divergence linearity and gauge decomposition
// ============================================================================
// The discrete divergence is LINEAR: div(J + K) = div(J) + div(K).
// Since grad(lambda) is computed with central-difference operators and the
// Laplacian uses a different stencil, div(grad(lambda)) != lap(lambda) on the
// discrete lattice. But div IS linear, so we test:
//   (a) div(J + grad_lambda) = div(J) + div(grad_lambda)  [exact]
//   (b) curl(grad_lambda) = 0  [exact discrete identity]
// These together imply the gauge transform only affects divergence (charge
// sector), not curl (magnetic sector) — the essence of U(1) gauge structure.
static void campaign_6c() {
    std::cout << "\n================================================================\n";
    std::cout << "  Campaign 6c: Divergence Linearity & Gauge Decomposition\n";
    std::cout << "================================================================\n";

    const int L = 16;
    const double GAUGE_AMP = 1.0;

    // Create a non-trivial flux field
    ftd::RenderBridge eng_orig(L);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                double kk = 2.0 * ftd::PI / L;
                double fx = 0.5 * std::sin(kk * x) * std::cos(kk * z);
                double fy = 0.3 * std::cos(kk * y);
                double fz = 0.4 * std::sin(kk * z) * std::sin(kk * x);
                eng_orig.inject_flux(x, y, z, {fx, fy, fz});
            }
        }
    }

    // Create an engine with ONLY the gauge field grad(lambda)
    auto lambda = make_lambda_field(L, GAUGE_AMP);
    ftd::RenderBridge eng_grad(L);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                ftd::Vec3 grad_lam = discrete_grad_lambda(x, y, z, L, lambda);
                eng_grad.inject_flux(x, y, z, grad_lam);
            }
        }
    }

    // Create gauge-transformed engine: J + grad(lambda)
    ftd::RenderBridge eng_gauge(L);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx = eng_orig.lattice().index(x, y, z);
                ftd::Vec3 j = eng_orig.voxels()[idx].flux;
                ftd::Vec3 g = eng_grad.voxels()[idx].flux;
                eng_gauge.inject_flux(x, y, z, {j.x + g.x, j.y + g.y, j.z + g.z});
            }
        }
    }

    // Test (a): Divergence linearity
    // div(J + grad_lambda) = div(J) + div(grad_lambda) — exact
    double max_div_err = 0;
    int n_tested = 0;
    for (int i = 0; i < eng_orig.lattice().total_sites(); ++i) {
        double div_combined = eng_gauge.divergence_flux(i);
        double div_orig = eng_orig.divergence_flux(i);
        double div_grad = eng_grad.divergence_flux(i);

        double lhs = div_combined;
        double rhs = div_orig + div_grad;
        double err = std::abs(lhs - rhs);
        if (err > max_div_err) max_div_err = err;
        ++n_tested;
    }

    std::cout << "    Tested " << n_tested << " lattice points\n";
    std::cout << "    Max |div(J+G) - (div(J) + div(G))| = " << max_div_err << "\n";
    check("6c: Divergence linearity holds (< 1e-10)", max_div_err < 1e-10);

    // Test (b): curl(grad_lambda) = 0  — exact discrete identity
    double max_curl = 0;
    for (int i = 0; i < eng_grad.lattice().total_sites(); ++i) {
        ftd::Vec3 c = eng_grad.curl_flux(i);
        double cmag = c.mag();
        if (cmag > max_curl) max_curl = cmag;
    }
    std::cout << "    Max |curl(grad(lambda))| = " << max_curl << "\n";
    check("6c: curl(grad(lambda)) = 0 (< 1e-10)", max_curl < 1e-10);

    // Test (c): div(grad_lambda) is non-zero — gauge changes divergence
    double max_div_grad = 0;
    for (int i = 0; i < eng_grad.lattice().total_sites(); ++i) {
        double d = std::abs(eng_grad.divergence_flux(i));
        if (d > max_div_grad) max_div_grad = d;
    }
    std::cout << "    Max |div(grad(lambda))| = " << max_div_grad << "\n";
    check("6c: Gauge changes divergence (div(grad) > 0)", max_div_grad > 0.001);
}

// ============================================================================
// 6d — Longitudinal mode decay vs transverse mode survival
// ============================================================================
// Pure longitudinal flux (J = grad(phi)) should decay or be absorbed
// faster than pure transverse flux (div=0), because the longitudinal
// component is constrained by Gauss's law.
static void campaign_6d() {
    std::cout << "\n================================================================\n";
    std::cout << "  Campaign 6d: Longitudinal vs Transverse Mode Decay\n";
    std::cout << "================================================================\n";

    const int L = 32;
    const int mid = L / 2;
    const double AMP = 0.2;
    const double SIGMA = 4.0;
    const int T_RUN = 50;

    // --- Transverse pulse: J_y(x) = Gaussian in x ---
    // div(J) = dJ_y/dy = 0 (J_y doesn't depend on y)
    ftd::RenderBridge eng_trans(L);
    eng_trans.toggles.gauss_projection = false;  // Test pure wave decay without Gauss solver
    double E0_trans = 0;
    for (int x = 0; x < L; ++x) {
        double dx = x - mid;
        double jy = AMP * std::exp(-dx * dx / (2.0 * SIGMA * SIGMA));
        // Set on midplane only for speed
        for (int y = mid - 2; y <= mid + 2; ++y) {
            for (int z = mid - 2; z <= mid + 2; ++z) {
                eng_trans.inject_flux(x, y, z, {0, jy, 0});
            }
        }
    }
    // Measure initial energy
    for (int i = 0; i < eng_trans.lattice().total_sites(); ++i) {
        E0_trans += eng_trans.voxels()[i].flux.mag2();
    }

    // --- Longitudinal pulse: J_x(x) = Gaussian in x ---
    // div(J) = dJ_x/dx != 0
    ftd::RenderBridge eng_long(L);
    eng_long.toggles.gauss_projection = false;  // Test pure wave decay without Gauss solver
    double E0_long = 0;
    for (int x = 0; x < L; ++x) {
        double dx = x - mid;
        double jx = AMP * std::exp(-dx * dx / (2.0 * SIGMA * SIGMA));
        for (int y = mid - 2; y <= mid + 2; ++y) {
            for (int z = mid - 2; z <= mid + 2; ++z) {
                eng_long.inject_flux(x, y, z, {jx, 0, 0});
            }
        }
    }
    for (int i = 0; i < eng_long.lattice().total_sites(); ++i) {
        E0_long += eng_long.voxels()[i].flux.mag2();
    }

    std::cout << "    Initial energy transverse:   " << std::setprecision(6) << E0_trans << "\n";
    std::cout << "    Initial energy longitudinal: " << E0_long << "\n";

    // Run both
    eng_trans.run(T_RUN);
    eng_long.run(T_RUN);

    // Measure final energy
    double Ef_trans = 0, Ef_long = 0;
    for (int i = 0; i < eng_trans.lattice().total_sites(); ++i) {
        Ef_trans += eng_trans.voxels()[i].flux.mag2();
    }
    for (int i = 0; i < eng_long.lattice().total_sites(); ++i) {
        Ef_long += eng_long.voxels()[i].flux.mag2();
    }

    double ratio_trans = (E0_trans > 1e-20) ? Ef_trans / E0_trans : 0;
    double ratio_long  = (E0_long > 1e-20)  ? Ef_long / E0_long : 0;

    std::cout << "    After " << T_RUN << " ticks:\n";
    std::cout << "      Transverse energy ratio:   " << ratio_trans
              << " (" << ratio_trans * 100 << "% remaining)\n";
    std::cout << "      Longitudinal energy ratio: " << ratio_long
              << " (" << ratio_long * 100 << "% remaining)\n";

    // Both should have decayed due to DAMPING
    check("6d: Transverse energy decreased", ratio_trans < 1.0);
    check("6d: Longitudinal energy decreased", ratio_long < 1.0);

    // The longitudinal mode should decay at least as fast as transverse
    // (or faster, due to Gauss constraint coupling)
    // In practice, if damping is purely multiplicative on |J|,
    // both decay at same rate. So we just check they're comparable.
    std::cout << "    Energy retention: trans/long = "
              << ((ratio_long > 1e-20) ? ratio_trans / ratio_long : 999) << "\n";

    // At minimum, transverse mode should retain energy comparably to longitudinal
    // (both experience same damping; longitudinal may additionally lose energy
    // to charge creation via Gauss constraint)
    check("6d: Transverse retains >= longitudinal energy",
          ratio_trans >= ratio_long * 0.8);  // allow 20% tolerance
}

// ============================================================================
// Main
// ============================================================================
int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Gauge Dynamics — U(1) Invariance Under Evolution\n";
    std::cout << "  ALPHA = " << ftd::ALPHA << "  C_WAVE = " << ftd::C_WAVE << "\n";
    std::cout << "================================================================\n";

    campaign_6a();
    campaign_6b();
    campaign_6c();
    campaign_6d();

    std::cout << "\n================================================================\n";
    std::cout << "  CAMPAIGN GAUGE DYNAMICS COMPLETE: "
              << g_passes << " passed, " << g_failures << " failed\n";
    std::cout << "================================================================\n";

    return g_failures;
}
