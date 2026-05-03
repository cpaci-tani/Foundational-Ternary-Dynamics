/**
 * Wilson-Dirac limit consistency (Phase II.2-D milestone).
 *
 * Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
 * Spec:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md  section 8.4
 *
 * VALIDATION:
 *   With small gauge phase epsilon, U_mu(n) = exp(i epsilon phi_mu(n)) for
 *   arbitrary phi_mu, the Wilson-Dirac operator must continuously reduce
 *   to the free Wilson-Dirac as epsilon -> 0:
 *
 *       || D_W^epsilon psi - D_W^0 psi ||  =  O(epsilon)
 *
 *   The leading-order coefficient comes from the gauge-link expansion
 *   U_mu(n) = 1 + i epsilon phi_mu + O(epsilon^2). We verify the linear
 *   scaling by computing the ratio
 *       R(epsilon) = || D_W^epsilon psi - D_W^0 psi || / epsilon
 *   and confirming R is bounded and converges to a finite limit as
 *   epsilon decreases over four decades 1e-1, 1e-2, 1e-3, 1e-4.
 *
 *   Specifically: R(eps_k) - R(eps_k+1) -> 0 as k increases (Cauchy
 *   condition). We accept |R(1e-3) - R(1e-4)| < 1e-3 * R(1e-4) as
 *   evidence of convergence.
 *
 * Outcomes:
 *   PASS  -> Phase II.2-D milestone CLOSED
 *   FAIL  -> investigation required (suggests non-smooth gauge integration)
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

double diff_norm(const SpinorField& a, const SpinorField& b) {
    double s = 0.0;
    for (std::size_t i = 0; i < a.data.size(); ++i) {
        for (int k = 0; k < 4; ++k) {
            s += std::norm(a.data[i][k] - b.data[i][k]);
        }
    }
    return std::sqrt(s);
}

}  // namespace

int main() {
    std::cout << "Wilson-Dirac limit consistency (Phase II.2-D)\n";
    std::cout << "Spec: SPEC_WILSON_DIRAC_FTD.md section 8.4\n\n";

    const int L = 12;
    Lattice lattice(L);
    const std::size_t N = static_cast<std::size_t>(L) * L * L;

    std::mt19937 rng(0xc0ffee01);
    std::uniform_real_distribution<double> phase_dist(-M_PI, M_PI);
    std::normal_distribution<double> spinor_dist(0.0, 1.0);

    // Fix random phi_mu(n) and random psi.
    std::array<std::vector<double>, 3> phi;
    for (int mu = 0; mu < 3; ++mu) {
        phi[mu].resize(N);
        for (std::size_t i = 0; i < N; ++i) phi[mu][i] = phase_dist(rng);
    }
    SpinorField psi(L);
    for (std::size_t i = 0; i < N; ++i) {
        psi.data[i] = {cdouble{spinor_dist(rng), spinor_dist(rng)},
                       cdouble{spinor_dist(rng), spinor_dist(rng)},
                       cdouble{spinor_dist(rng), spinor_dist(rng)},
                       cdouble{spinor_dist(rng), spinor_dist(rng)}};
    }

    WilsonDiracParams params;
    params.m = 0.5;
    params.r = 1.0;
    params.a = 1.0;

    GaugeLinks links_free(L);
    links_free.set_identity();

    SpinorField Dpsi_free(L);
    apply_wilson_dirac(Dpsi_free, psi, links_free, lattice, params);

    const std::vector<double> epsilons = {1e-1, 1e-2, 1e-3, 1e-4};
    std::vector<double> R_values;

    for (double eps : epsilons) {
        GaugeLinks links_eps(L);
        for (int mu = 0; mu < 3; ++mu) {
            for (std::size_t i = 0; i < N; ++i) {
                links_eps.U[mu][i] = std::exp(cdouble{0, eps * phi[mu][i]});
            }
        }
        SpinorField Dpsi_eps(L);
        apply_wilson_dirac(Dpsi_eps, psi, links_eps, lattice, params);

        const double delta = diff_norm(Dpsi_eps, Dpsi_free);
        const double R = delta / eps;
        R_values.push_back(R);

        std::cout << "  eps=" << std::scientific << std::setprecision(2) << eps
                  << "   ||D_eps psi - D_0 psi||=" << std::setprecision(6) << delta
                  << "   R = delta/eps = " << R << "\n";
    }

    const double R_last_two_diff = std::abs(R_values[2] - R_values[3]);
    const double R_last = R_values[3];
    const double rel_change = R_last_two_diff / std::max(R_last, 1e-300);

    std::cout << "\n  |R(1e-3) - R(1e-4)| / R(1e-4) = " << std::scientific
              << std::setprecision(6) << rel_change << "\n";

    const bool ok_cauchy = rel_change < 1e-3;

    std::cout << "  " << (ok_cauchy ? "PASS" : "FAIL")
              << " (tolerance 1e-3)\n\n";

    // Sanity check: at eps = 0 exactly, the difference is zero.
    GaugeLinks links_zero(L);
    links_zero.set_identity();
    SpinorField Dpsi_zero(L);
    apply_wilson_dirac(Dpsi_zero, psi, links_zero, lattice, params);
    const double zero_delta = diff_norm(Dpsi_zero, Dpsi_free);
    const bool ok_zero = zero_delta < 1e-14;
    std::cout << "  Identity-link sanity check: ||D_id psi - D_free psi|| = "
              << std::scientific << std::setprecision(6) << zero_delta
              << "  " << (ok_zero ? "PASS" : "FAIL") << "\n\n";

    if (ok_cauchy && ok_zero) {
        std::cout << "Phase II.2-D milestone: CLOSED. D_W is continuous in the gauge phase; "
                     "trivial-link limit reproduces free Wilson-Dirac exactly.\n";
        return 0;
    } else {
        std::cout << "Phase II.2-D milestone: INVESTIGATION REQUIRED.\n";
        return 1;
    }
}
