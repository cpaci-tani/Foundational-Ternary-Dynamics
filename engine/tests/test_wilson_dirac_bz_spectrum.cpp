/**
 * Wilson-Dirac full-BZ spectrum sweep (Phase II.2-B milestone).
 *
 * Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
 * Spec:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md  section 8.2
 *
 * VALIDATION (committed pre-measurement):
 *   For every momentum p_mu = 2 pi k_mu / L with k_mu in {0,...,L-1}, mu in {0,1,2},
 *   and for both spins s in {0, 1}, applying D_W to the plane wave
 *       psi_p(n) = u(p, s) exp(i p . n)
 *   must yield norm^2(D_W psi_p) = (M_eff^2 + K^2) * norm^2(psi_p) where
 *       M_eff(p) = m + (r/a) sum_mu (1 - cos p_mu)
 *       K^2(p)   = (1/a^2) sum_mu sin^2(p_mu)
 *
 *   Also check doubler-lifting at the BZ corner p = (pi, pi, pi):
 *   M_eff_corner = m + 6 r/a, which is >> m for r ~ 1, a ~ 1.
 *
 * Test parameters:
 *   - L = 8  -> 512 momenta * 2 spins = 1024 modes (cheap, exhaustive)
 *   - tolerance: 1e-10 relative error per mode
 *   - reports worst-case rel_err and the momentum where it occurred
 *
 * Outcomes:
 *   PASS  -> Phase II.2-B milestone CLOSED
 *   FAIL  -> investigation required
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cstdio>
#include <iomanip>
#include <iostream>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

#include "ftd/lattice.h"
#include "ftd/wilson_dirac.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

constexpr double TOL = 1e-10;

struct ModeResult {
    int kx, ky, kz, spin;
    double rel_err;
    double M_eff_squared;
    double K_squared;
};

}  // namespace

int main() {
    std::cout << "Wilson-Dirac full-BZ spectrum sweep (Phase II.2-B)\n";
    std::cout << "Spec: SPEC_WILSON_DIRAC_FTD.md section 8.2\n\n";

    const int L = 8;
    Lattice lattice(L);
    GaugeLinks links(L);
    links.set_identity();

    WilsonDiracParams params;
    params.m = 0.5;   // moderate mass for clear signal
    params.r = 1.0;
    params.a = 1.0;

    SpinorField psi(L);
    SpinorField out(L);

    int total = 0;
    int passed = 0;
    int failed = 0;

    ModeResult worst = {0, 0, 0, 0, 0.0, 0.0, 0.0};
    ModeResult corner_record = {0, 0, 0, 0, 0.0, 0.0, 0.0};

    const double k0 = 2.0 * M_PI / static_cast<double>(L);

    for (int kx = 0; kx < L; ++kx) {
        for (int ky = 0; ky < L; ++ky) {
            for (int kz = 0; kz < L; ++kz) {
                for (int spin = 0; spin < 2; ++spin) {
                    const std::array<double, 3> p = {kx * k0, ky * k0, kz * k0};

                    initialize_plane_wave(psi, lattice, p, spin);
                    const double norm_psi = psi.total_norm_squared();
                    if (norm_psi < 1e-300) {
                        // degenerate spinor (shouldn't happen for plane wave init); skip
                        continue;
                    }

                    apply_wilson_dirac(out, psi, links, lattice, params);
                    const double norm_out = out.total_norm_squared();

                    const double M_eff = params.m + (params.r / params.a) *
                        ((1.0 - std::cos(p[0])) + (1.0 - std::cos(p[1])) + (1.0 - std::cos(p[2])));
                    const double K_sq = (1.0 / (params.a * params.a)) *
                        (std::sin(p[0]) * std::sin(p[0]) +
                         std::sin(p[1]) * std::sin(p[1]) +
                         std::sin(p[2]) * std::sin(p[2]));
                    const double norm_predicted = (M_eff * M_eff + K_sq) * norm_psi;
                    const double rel_err = std::abs(norm_out - norm_predicted) /
                                           std::max(norm_predicted, 1e-300);

                    ++total;
                    if (rel_err < TOL) ++passed; else ++failed;

                    if (rel_err > worst.rel_err) {
                        worst = {kx, ky, kz, spin, rel_err, M_eff * M_eff, K_sq};
                    }

                    // Record the BZ corner mode: kx = ky = kz = L/2 -> p = (pi, pi, pi).
                    if (kx == L / 2 && ky == L / 2 && kz == L / 2 && spin == 0) {
                        corner_record = {kx, ky, kz, spin, rel_err, M_eff * M_eff, K_sq};
                    }
                }
            }
        }
    }

    std::cout << "Swept " << total << " modes (" << L << "^3 momenta * 2 spins).\n";
    std::cout << "  passed (rel_err < " << TOL << "): " << passed << "\n";
    std::cout << "  failed:                            " << failed << "\n\n";

    std::cout << "Worst-case relative error:\n";
    std::cout << "  mode (kx,ky,kz,s) = (" << worst.kx << "," << worst.ky << ","
              << worst.kz << "," << worst.spin << ")\n";
    std::cout << "  rel_err = " << std::scientific << std::setprecision(6) << worst.rel_err << "\n";
    std::cout << "  M_eff^2 = " << std::fixed << std::setprecision(6) << worst.M_eff_squared
              << "  K^2 = " << worst.K_squared << "\n\n";

    std::cout << "BZ-corner mode p = (pi, pi, pi):\n";
    std::cout << "  M_eff^2 = " << std::fixed << std::setprecision(6) << corner_record.M_eff_squared
              << "  (predicted m + 6r/a = " << params.m + 6.0 * params.r / params.a
              << "; squared = " << (params.m + 6.0 * params.r / params.a) * (params.m + 6.0 * params.r / params.a)
              << ")\n";
    std::cout << "  K^2     = " << corner_record.K_squared << "  (predicted 0)\n";
    std::cout << "  rel_err = " << std::scientific << std::setprecision(6) << corner_record.rel_err << "\n\n";

    if (failed == 0) {
        std::cout << "Phase II.2-B milestone: CLOSED. Wilson dispersion verified across full BZ; doublers lifted.\n";
        return 0;
    } else {
        std::cout << "Phase II.2-B milestone: INVESTIGATION REQUIRED.\n";
        return 1;
    }
}
