/**
 * Wilson-Dirac gauge-link verification (Phase II.2-C milestone).
 *
 * Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
 * Spec:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md  section 8.3
 *
 * Two checks committed pre-measurement:
 *
 *   1. Gauge covariance.
 *      Pick an arbitrary lattice scalar chi(n). Define
 *           psi'(n)        = exp(i chi(n)) psi(n)
 *           U'_mu(n)       = exp(i chi(n)) U_mu(n) exp(-i chi(n+mu_hat))
 *      Then D_W' psi' must equal exp(i chi(n)) (D_W psi)(n) at every site.
 *      Equivalently the per-site norm-squared is invariant:
 *           |D_W' psi'(n)|^2 == |D_W psi(n)|^2     for all n.
 *      This is the deepest correctness check on the gauge-link integration:
 *      it would fail if (a) U_mu(n-mu) is replaced by U_mu(n) anywhere in
 *      the dagger term, (b) the conjugation is dropped, or (c) phase-attach
 *      direction is wrong. Tolerance: 1e-12 on the worst per-site rel_err.
 *
 *   2. Plaquette flux for properly-quantized uniform B in z.
 *      Set alpha = 2 pi * n_flux / L^2 with n_flux integer. Use Landau gauge
 *      with a twist at the y-boundary so the configuration is single-valued
 *      on the torus:
 *           U_x(x, y, z) = exp(-i alpha y)
 *           U_y(x, y, z) = 1                        (y != L-1)
 *           U_y(x, L-1, z) = exp(+i alpha x L)      (twist at y boundary)
 *           U_z(x, y, z) = 1
 *      Verify: every xy-plaquette has phase exp(+i alpha); xz and yz
 *      plaquettes are 1. Tolerance: 1e-12.
 *
 * Outcomes:
 *   PASS  -> Phase II.2-C milestone CLOSED
 *   FAIL  -> investigation required
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

#include "ftd/lattice.h"
#include "ftd/wilson_dirac.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

constexpr double TOL_GAUGE = 1e-12;
constexpr double TOL_PLAQ = 1e-12;

// Given U_mu(n), apply gauge transformation:
//   U'_mu(n) = exp(i chi(n)) U_mu(n) exp(-i chi(n+mu_hat))
GaugeLinks gauge_transform_links(const GaugeLinks& links,
                                 const std::vector<double>& chi,
                                 const Lattice& lattice) {
    const int L = lattice.size();
    GaugeLinks out(L);
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int idx = lattice.index(x, y, z);
                const double chi_n = chi[idx];

                const int idx_xp = lattice.index((x + 1) % L, y, z);
                const int idx_yp = lattice.index(x, (y + 1) % L, z);
                const int idx_zp = lattice.index(x, y, (z + 1) % L);

                const cdouble phase_xp = std::exp(cdouble{0, chi_n - chi[idx_xp]});
                const cdouble phase_yp = std::exp(cdouble{0, chi_n - chi[idx_yp]});
                const cdouble phase_zp = std::exp(cdouble{0, chi_n - chi[idx_zp]});

                out.U[0][static_cast<std::size_t>(idx)] = phase_xp * links.U[0][static_cast<std::size_t>(idx)];
                out.U[1][static_cast<std::size_t>(idx)] = phase_yp * links.U[1][static_cast<std::size_t>(idx)];
                out.U[2][static_cast<std::size_t>(idx)] = phase_zp * links.U[2][static_cast<std::size_t>(idx)];
            }
        }
    }
    return out;
}

SpinorField gauge_transform_spinor(const SpinorField& psi,
                                   const std::vector<double>& chi,
                                   const Lattice& lattice) {
    const int L = lattice.size();
    SpinorField out(L);
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int idx = lattice.index(x, y, z);
                const cdouble phase = std::exp(cdouble{0, chi[idx]});
                const auto& s = psi.at(idx);
                out.at(idx) = {phase * s[0], phase * s[1], phase * s[2], phase * s[3]};
            }
        }
    }
    return out;
}

bool check_gauge_covariance(int L) {
    Lattice lattice(L);
    std::mt19937 rng(0xfeedface);
    std::uniform_real_distribution<double> phase_dist(-M_PI, M_PI);
    std::normal_distribution<double> spinor_dist(0.0, 1.0);

    // Random gauge field (NOT identity, to exercise the U_mu(n) and U_mu^dag(n-mu) paths).
    GaugeLinks links(L);
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    for (int mu = 0; mu < 3; ++mu) {
        for (std::size_t i = 0; i < N; ++i) {
            links.U[mu][i] = std::exp(cdouble{0, phase_dist(rng)});
        }
    }

    // Random spinor field.
    SpinorField psi(L);
    for (std::size_t i = 0; i < N; ++i) {
        psi.data[i] = {cdouble{spinor_dist(rng), spinor_dist(rng)},
                       cdouble{spinor_dist(rng), spinor_dist(rng)},
                       cdouble{spinor_dist(rng), spinor_dist(rng)},
                       cdouble{spinor_dist(rng), spinor_dist(rng)}};
    }

    // Random gauge function chi(n).
    std::vector<double> chi(N);
    for (std::size_t i = 0; i < N; ++i) chi[i] = phase_dist(rng);

    WilsonDiracParams params;
    params.m = 0.5;
    params.r = 1.0;
    params.a = 1.0;

    // Compute D_W psi.
    SpinorField Dpsi(L);
    apply_wilson_dirac(Dpsi, psi, links, lattice, params);

    // Apply gauge transform.
    GaugeLinks links_g = gauge_transform_links(links, chi, lattice);
    SpinorField psi_g = gauge_transform_spinor(psi, chi, lattice);

    // Compute D_W' psi'.
    SpinorField Dpsi_g(L);
    apply_wilson_dirac(Dpsi_g, psi_g, links_g, lattice, params);

    // Predicted: Dpsi_g(n) = exp(i chi(n)) Dpsi(n).
    double worst = 0.0;
    int worst_idx = 0;
    for (std::size_t i = 0; i < N; ++i) {
        const cdouble phase = std::exp(cdouble{0, chi[i]});
        const auto& A = Dpsi_g.data[i];
        const auto& B = Dpsi.data[i];
        Spinor predicted = {phase * B[0], phase * B[1], phase * B[2], phase * B[3]};
        double diff_norm_sq = 0.0;
        double pred_norm_sq = 0.0;
        for (int k = 0; k < 4; ++k) {
            const cdouble d = A[k] - predicted[k];
            diff_norm_sq += std::norm(d);
            pred_norm_sq += std::norm(predicted[k]);
        }
        const double rel = std::sqrt(diff_norm_sq) / std::max(std::sqrt(pred_norm_sq), 1e-300);
        if (rel > worst) {
            worst = rel;
            worst_idx = static_cast<int>(i);
        }
    }

    std::cout << "  gauge cov  L=" << L
              << "  worst rel_err = " << std::scientific << std::setprecision(6) << worst
              << "  (site idx " << worst_idx << ")"
              << "  " << (worst < TOL_GAUGE ? "PASS" : "FAIL") << "\n";
    return worst < TOL_GAUGE;
}

bool check_plaquette_flux(int L, int n_flux) {
    Lattice lattice(L);
    GaugeLinks links(L);
    const double alpha = 2.0 * M_PI * static_cast<double>(n_flux) / (static_cast<double>(L) * L);

    // Build properly-twisted Landau gauge for uniform B in z. Index via lattice.index.
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const std::size_t idx = static_cast<std::size_t>(lattice.index(x, y, z));
                links.U[0][idx] = std::exp(cdouble{0, -alpha * static_cast<double>(y)});
                links.U[1][idx] = (y == L - 1)
                    ? std::exp(cdouble{0, +alpha * static_cast<double>(x) * static_cast<double>(L)})
                    : cdouble{1, 0};
                links.U[2][idx] = cdouble{1, 0};
            }
        }
    }

    auto plaquette_phase = [&](int x, int y, int z, int mu, int nu) {
        // P_{mu,nu}(n) = U_mu(n) U_nu(n+mu_hat) U_mu^dag(n+nu_hat) U_nu^dag(n)
        const int idx = lattice.index(x, y, z);
        const int dx_mu = (mu == 0) ? 1 : 0;
        const int dy_mu = (mu == 1) ? 1 : 0;
        const int dz_mu = (mu == 2) ? 1 : 0;
        const int dx_nu = (nu == 0) ? 1 : 0;
        const int dy_nu = (nu == 1) ? 1 : 0;
        const int dz_nu = (nu == 2) ? 1 : 0;
        const int idx_pmu = lattice.index((x + dx_mu) % L, (y + dy_mu) % L, (z + dz_mu) % L);
        const int idx_pnu = lattice.index((x + dx_nu) % L, (y + dy_nu) % L, (z + dz_nu) % L);

        const cdouble U1 = links.U[mu][static_cast<std::size_t>(idx)];
        const cdouble U2 = links.U[nu][static_cast<std::size_t>(idx_pmu)];
        const cdouble U3 = std::conj(links.U[mu][static_cast<std::size_t>(idx_pnu)]);
        const cdouble U4 = std::conj(links.U[nu][static_cast<std::size_t>(idx)]);
        return U1 * U2 * U3 * U4;
    };

    const cdouble target_xy = std::exp(cdouble{0, alpha});
    double worst_xy = 0.0;
    double worst_xz = 0.0;
    double worst_yz = 0.0;
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const cdouble pxy = plaquette_phase(x, y, z, 0, 1);
                const cdouble pxz = plaquette_phase(x, y, z, 0, 2);
                const cdouble pyz = plaquette_phase(x, y, z, 1, 2);
                worst_xy = std::max(worst_xy, std::abs(pxy - target_xy));
                worst_xz = std::max(worst_xz, std::abs(pxz - cdouble{1, 0}));
                worst_yz = std::max(worst_yz, std::abs(pyz - cdouble{1, 0}));
            }
        }
    }

    const bool ok_xy = worst_xy < TOL_PLAQ;
    const bool ok_xz = worst_xz < TOL_PLAQ;
    const bool ok_yz = worst_yz < TOL_PLAQ;

    std::cout << "  plaquette  L=" << L << "  n_flux=" << n_flux
              << "  alpha=" << std::fixed << std::setprecision(8) << alpha << "\n";
    std::cout << "    xy plaquettes (target exp(+i alpha)):  worst |P - target| = "
              << std::scientific << std::setprecision(6) << worst_xy
              << "  " << (ok_xy ? "PASS" : "FAIL") << "\n";
    std::cout << "    xz plaquettes (target 1):              worst |P - 1|       = "
              << worst_xz << "  " << (ok_xz ? "PASS" : "FAIL") << "\n";
    std::cout << "    yz plaquettes (target 1):              worst |P - 1|       = "
              << worst_yz << "  " << (ok_yz ? "PASS" : "FAIL") << "\n";

    return ok_xy && ok_xz && ok_yz;
}

}  // namespace

int main() {
    std::cout << "Wilson-Dirac gauge-link verification (Phase II.2-C)\n";
    std::cout << "Spec: SPEC_WILSON_DIRAC_FTD.md section 8.3\n\n";

    int passed = 0;
    int failed = 0;

    std::cout << "Check 1: gauge covariance D_W' psi' = exp(i chi) D_W psi\n";
    if (check_gauge_covariance(8)) ++passed; else ++failed;
    if (check_gauge_covariance(12)) ++passed; else ++failed;

    std::cout << "\nCheck 2: plaquette-flux for twisted Landau-gauge uniform B in z\n";
    if (check_plaquette_flux(8, 1)) ++passed; else ++failed;
    if (check_plaquette_flux(8, 2)) ++passed; else ++failed;
    if (check_plaquette_flux(12, 3)) ++passed; else ++failed;

    std::cout << "\nAggregate: " << passed << " passed, " << failed << " failed\n";

    if (failed == 0) {
        std::cout << "Phase II.2-C milestone: CLOSED. Gauge-link integration verified.\n";
        return 0;
    } else {
        std::cout << "Phase II.2-C milestone: INVESTIGATION REQUIRED.\n";
        return 1;
    }
}
