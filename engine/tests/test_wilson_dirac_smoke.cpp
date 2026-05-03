/**
 * Wilson-Dirac smoke test (Phase II.2-A milestone).
 *
 * Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md (tag preregister-phase-ii-wilson-dirac-g2-v1)
 * Spec:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md  section 8 milestones
 *
 * VALIDATION CHECKS (committed pre-measurement):
 *   1. Free-fermion plane-wave: with B = 0 and identity gauge links,
 *      apply D_W to a plane wave psi_p(n) = u(p,s) exp(i p . n).
 *      The result should be lambda(p) * psi_p(n) where
 *           lambda(p) = (m + 4r/a) - (1/a) sum_mu [ r cos(p_mu) - i sin(p_mu) gamma^mu ]
 *      Specifically, for low momentum p << pi/a:
 *           lambda(p) approx m + sum_mu (r/2) p_mu^2 - i sum_mu sin(p_mu) gamma^mu
 *      The dispersion is the standard lattice Wilson-Dirac one.
 *
 *      Assertion: norm of D_W psi_p (which is |lambda(p)| * norm of psi_p) matches
 *      the analytical prediction within 1e-10 relative error.
 *
 *   2. Norm conservation under RK4 evolution: i d/dt psi = D_W psi is unitary
 *      (D_W is hermitian for r=0; with Wilson term r != 0, the effective
 *      Hamiltonian is still hermitian because Wilson term is real-symmetric in
 *      position basis). Total norm should be conserved over time.
 *
 *      Assertion: after 100 RK4 steps with dt = 0.01, total spinor norm is
 *      conserved to within 1e-6 relative error.
 *
 *   3. Identity gauge links: U_mu = 1 everywhere reproduces free Wilson-Dirac
 *      (no gauge effect). Verified by setting U.set_identity() and confirming
 *      checks 1 and 2 still pass.
 *
 * Outcomes:
 *   - All 3 checks PASS  -> Phase II.2-A milestone CLOSED
 *   - Any check FAILS    -> investigation required; do not proceed to II.3
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cstdio>
#include <iostream>
#include <iomanip>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

#include "ftd/lattice.h"
#include "ftd/wilson_dirac.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

constexpr double TOL_DISPERSION = 1e-10;
constexpr double TOL_NORM = 1e-6;

bool check_plane_wave_dispersion(int L, const std::array<double, 3>& momentum, int spin) {
    Lattice lattice(L);
    GaugeLinks links(L);
    links.set_identity();

    SpinorField psi(L);
    initialize_plane_wave(psi, lattice, momentum, spin);
    const double norm_psi = psi.total_norm_squared();

    SpinorField out(L);
    WilsonDiracParams params;
    params.m = 0.5;   // moderate mass for clear signal in test
    params.r = 1.0;
    params.a = 1.0;
    apply_wilson_dirac(out, psi, links, lattice, params);

    const double norm_out = out.total_norm_squared();

    // Analytical eigenvalue magnitude for plane wave on free Wilson-Dirac:
    //   D_W u_p exp(i p.n) = E(p) u_p exp(i p.n)
    //   where E(p)^2 = (m + (r/a) sum_mu (1 - cos p_mu))^2 + (1/a^2) sum_mu sin^2(p_mu)
    // (this is the Wilson-Dirac dispersion in the spinor norm; see Montvay-Munster eq 4.2)
    const double M_eff = params.m + (params.r / params.a) * ((1.0 - std::cos(momentum[0]))
                                                              + (1.0 - std::cos(momentum[1]))
                                                              + (1.0 - std::cos(momentum[2])));
    const double K_sq = (1.0 / (params.a * params.a)) * (std::sin(momentum[0]) * std::sin(momentum[0])
                                                          + std::sin(momentum[1]) * std::sin(momentum[1])
                                                          + std::sin(momentum[2]) * std::sin(momentum[2]));
    const double E_pred_sq = M_eff * M_eff + K_sq;

    // Predicted norm of D_W psi = E(p) * norm(psi)
    const double norm_out_predicted = E_pred_sq * norm_psi;
    const double rel_err = std::abs(norm_out - norm_out_predicted) / norm_out_predicted;

    std::cout << "  plane-wave  L=" << L
              << "  p=(" << std::fixed << std::setprecision(4) << momentum[0] << ","
              << momentum[1] << "," << momentum[2] << ")"
              << "  norm(psi)=" << std::scientific << std::setprecision(6) << norm_psi
              << "  norm(D psi)=" << norm_out
              << "  predicted=" << norm_out_predicted
              << "  rel_err=" << rel_err
              << "  " << (rel_err < TOL_DISPERSION ? "PASS" : "FAIL")
              << "\n";

    return rel_err < TOL_DISPERSION;
}

bool check_norm_conservation(int L) {
    Lattice lattice(L);
    GaugeLinks links(L);
    links.set_identity();

    SpinorField psi(L);
    SpinorField k1(L);
    SpinorField k_temp(L);

    // Localised Gaussian wave packet for richer initial state than plane wave.
    const int mid = L / 2;
    const double sigma = 2.0;
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const double dx = x - mid, dy = y - mid, dz = z - mid;
                const double r2 = dx * dx + dy * dy + dz * dz;
                const double envelope = std::exp(-r2 / (2.0 * sigma * sigma));
                const int idx = lattice.index(x, y, z);
                psi.at(idx) = {cdouble{envelope, 0}, cdouble{0, 0},
                               cdouble{0, 0}, cdouble{0, 0}};
            }
        }
    }
    const double norm_initial = psi.total_norm_squared();

    WilsonDiracParams params;
    params.m = 0.1;  // small mass: a typical electron-ish parameter
    params.r = 1.0;
    params.a = 1.0;

    const double dt = 0.01;
    const int n_steps = 100;
    for (int step = 0; step < n_steps; ++step) {
        evolve_rk4_step(psi, k1, k_temp, links, lattice, params, dt);
    }

    const double norm_final = psi.total_norm_squared();
    const double rel_err = std::abs(norm_final - norm_initial) / norm_initial;

    std::cout << "  norm-cons   L=" << L
              << "  initial=" << std::scientific << std::setprecision(8) << norm_initial
              << "  after_" << n_steps << "_steps=" << norm_final
              << "  rel_err=" << rel_err
              << "  " << (rel_err < TOL_NORM ? "PASS" : "FAIL")
              << "\n";

    return rel_err < TOL_NORM;
}

}  // namespace

int main() {
    std::cout << "Wilson-Dirac smoke test (Phase II.2-A)\n";
    std::cout << "Spec: SPEC_WILSON_DIRAC_FTD.md section 8\n\n";

    int passed = 0;
    int failed = 0;

    // Check 1: free-fermion plane-wave dispersion at three momenta.
    std::cout << "Check 1: free-fermion plane-wave dispersion\n";
    const int L = 16;
    const double k0 = 2.0 * M_PI / static_cast<double>(L);
    if (check_plane_wave_dispersion(L, {0.0, 0.0, 0.0}, 0)) ++passed; else ++failed;
    if (check_plane_wave_dispersion(L, {k0, 0.0, 0.0}, 0)) ++passed; else ++failed;
    if (check_plane_wave_dispersion(L, {2.0 * k0, k0, 0.0}, 0)) ++passed; else ++failed;
    if (check_plane_wave_dispersion(L, {3.0 * k0, 2.0 * k0, k0}, 1)) ++passed; else ++failed;

    // Check 2: norm conservation under RK4.
    std::cout << "\nCheck 2: norm conservation under RK4 evolution\n";
    if (check_norm_conservation(L)) ++passed; else ++failed;

    std::cout << "\nAggregate: " << passed << " passed, " << failed << " failed\n";

    if (failed == 0) {
        std::cout << "Phase II.2-A milestone: CLOSED. Free Wilson-Dirac operator and RK4 evolution validated.\n";
        return 0;
    } else {
        std::cout << "Phase II.2-A milestone: INVESTIGATION REQUIRED.\n";
        return 1;
    }
}
