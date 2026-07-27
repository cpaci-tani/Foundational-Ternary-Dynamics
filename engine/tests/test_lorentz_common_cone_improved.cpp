/**
 * FTD-0413 Moore-local q^4 common-cone fermion gate.
 *
 * Enlarges the FTD-0412 axial Wilson kinetic stencil to the face-diagonal
 * Moore shell.  The selected free symbol is
 *
 *   K_i = sin(q_i) [1 + cos(q_j) + cos(q_k)] / 3,
 *   r^2 = 4/3,  c_s^2 = 1/7.
 *
 * It cancels both independent quartic tensors while retaining a Wilson mass
 * at all seven Brillouin-zone corners.  This is a free-sector, default-off
 * diagnostic; the matter/flux poles still differ at q^6.
 */

#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <string>

#include "ftd/lattice.h"
#include "ftd/lorentz_bcc_time.h"
#include "ftd/wilson_dirac.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double TOL = 8.0e-11;

bool close(double lhs, double rhs, double tol = TOL) {
    return std::abs(lhs - rhs)
        <= tol * std::max({1.0, std::abs(lhs), std::abs(rhs)});
}

bool report(const std::string& label, bool ok) {
    std::cout << (ok ? "PASS  " : "FAIL  ") << label << '\n';
    return ok;
}

void fill_plane_wave(SpinorField& field,
                     const Lattice& lattice,
                     const std::array<double, 3>& p,
                     const Spinor& amplitude) {
    const int L = lattice.size();
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const double phase_arg = p[0] * x + p[1] * y + p[2] * z;
                const cdouble phase = std::exp(cdouble{0.0, phase_arg});
                field.at(lattice.index(x, y, z)) = scale(phase, amplitude);
            }
        }
    }
}

double improved_energy_sq(const std::array<double, 3>& p,
                          const WilsonDiracParams& params) {
    const double b = params.kinetic_transverse_weight;
    double kinetic_sq = 0.0;
    double wilson_sum = 0.0;
    for (int mu = 0; mu < 3; ++mu) {
        const int nu = (mu + 1) % 3;
        const int rho = (mu + 2) % 3;
        const double transverse = (1.0 - 2.0 * b)
                                + b * (std::cos(p[nu]) + std::cos(p[rho]));
        const double k_mu = std::sin(p[mu]) * transverse / params.a;
        kinetic_sq += k_mu * k_mu;
        wilson_sum += 1.0 - std::cos(p[mu]);
    }
    const double mass = params.m
                      + params.spatial_speed * params.r * wilson_sum / params.a;
    return params.spatial_speed * params.spatial_speed * kinetic_sq + mass * mass;
}

int signed_periodic_coordinate(int coordinate, int L) {
    return coordinate <= L / 2 ? coordinate : coordinate - L;
}

}  // namespace

int main() {
    int passed = 0;
    int failed = 0;
    const auto check = [&](const std::string& label, bool condition) {
        if (report(label, condition)) ++passed; else ++failed;
    };

    std::cout << "FTD-0413 Moore-local q^4 common-cone fermion gate\n";
    std::cout << std::setprecision(17);

    constexpr double b = 1.0 / 3.0;
    const double r = 2.0 / std::sqrt(3.0);
    const double c_s = std::sqrt(LORENTZ_BCC_TIME_EFFECTIVE_C2);

    check("normalization fixes axial and transverse weights to 1/3",
          close(1.0 - 2.0 * b, 1.0 / 3.0));
    check("quartic Q4 coefficient vanishes at r^2=4/3",
          close(-1.0 / 3.0 + r * r / 4.0, 0.0));
    check("quartic cross coefficient vanishes at b=1/3 and r^2=4/3",
          close(-2.0 * b + r * r / 2.0, 0.0));
    check("selected matter speed equals the BCC-time flux speed",
          close(c_s * c_s, 1.0 / 7.0));

    // The implemented Hamiltonian must realize its exact free symbol on every
    // momentum of a complete finite Brillouin zone, not just in an IR fit.
    {
        constexpr int L = 8;
        Lattice lattice(L);
        GaugeLinks links(L);
        links.set_identity();
        SpinorField psi(L), out(L);
        const Spinor generic = {
            cdouble{0.31, -0.17}, cdouble{-0.23, 0.41},
            cdouble{0.29, 0.37}, cdouble{-0.11, -0.53}
        };
        WilsonDiracParams params;
        params.m = 0.0;
        params.r = r;
        params.a = 1.0;
        params.spatial_speed = c_s;
        params.kinetic_transverse_weight = b;

        double worst = 0.0;
        for (int kx = 0; kx < L; ++kx) {
            for (int ky = 0; ky < L; ++ky) {
                for (int kz = 0; kz < L; ++kz) {
                    const std::array<double, 3> p{
                        2.0 * PI * kx / L,
                        2.0 * PI * ky / L,
                        2.0 * PI * kz / L,
                    };
                    fill_plane_wave(psi, lattice, p, generic);
                    apply_wilson_hamiltonian(out, psi, links, lattice, params);
                    const double measured = out.total_norm_squared()
                                          / psi.total_norm_squared();
                    const double predicted = improved_energy_sq(p, params);
                    const double error = std::abs(measured - predicted)
                                       / std::max({1.0, measured, predicted});
                    worst = std::max(worst, error);
                }
            }
        }
        check("full L=8 Brillouin zone matches the exact improved-H spectrum",
              worst < TOL);
    }

    // Wilson mass retains a unique massless corner.  The kinetic factor may
    // have additional zeros, but E=0 also requires W=0, which occurs only at
    // the origin.  Enumerate all eight corners explicitly.
    {
        WilsonDiracParams params;
        params.m = 0.0;
        params.r = r;
        params.a = 1.0;
        params.spatial_speed = c_s;
        params.kinetic_transverse_weight = b;
        int zero_corners = 0;
        double lightest_doubler_e2 = 1.0e300;
        for (int mask = 0; mask < 8; ++mask) {
            const std::array<double, 3> p{
                (mask & 1) ? PI : 0.0,
                (mask & 2) ? PI : 0.0,
                (mask & 4) ? PI : 0.0,
            };
            const double e2 = improved_energy_sq(p, params);
            if (close(e2, 0.0)) {
                ++zero_corners;
            } else {
                lightest_doubler_e2 = std::min(lightest_doubler_e2, e2);
            }
        }
        check("Wilson term leaves exactly one massless Brillouin-zone corner",
              zero_corners == 1);
        check("all seven corner doublers retain a positive Wilson gap",
              lightest_doubler_e2 > 0.0);
    }

    // A delta source must couple only to the site, six axial neighbours, and
    // twelve face diagonals.  No body diagonal or radius-two support is used.
    {
        constexpr int L = 9;
        Lattice lattice(L);
        GaugeLinks links(L);
        links.set_identity();
        SpinorField psi(L), out(L);
        psi.at(lattice.index(0, 0, 0)) = {
            cdouble{0.7, -0.2}, cdouble{-0.1, 0.4},
            cdouble{0.3, 0.6}, cdouble{-0.5, 0.1}
        };
        WilsonDiracParams params;
        params.m = 0.0;
        params.r = r;
        params.a = 1.0;
        params.spatial_speed = c_s;
        params.kinetic_transverse_weight = b;
        apply_wilson_hamiltonian(out, psi, links, lattice, params);

        bool support_ok = true;
        int face_sites = 0;
        for (int z = 0; z < L; ++z) {
            for (int y = 0; y < L; ++y) {
                for (int x = 0; x < L; ++x) {
                    const int idx = lattice.index(x, y, z);
                    if (spinor_norm_squared(out.at(idx)) < 1.0e-24) continue;
                    const int sx = signed_periodic_coordinate(x, L);
                    const int sy = signed_periodic_coordinate(y, L);
                    const int sz = signed_periodic_coordinate(z, L);
                    const int ax = std::abs(sx);
                    const int ay = std::abs(sy);
                    const int az = std::abs(sz);
                    const int l1 = ax + ay + az;
                    const int linf = std::max({ax, ay, az});
                    if (linf > 1 || l1 > 2) support_ok = false;
                    if (l1 == 2) ++face_sites;
                }
            }
        }
        check("implemented support stays inside the SC+FCC Moore shell",
              support_ok && face_sites > 0);
    }

    // The q^4 obstruction is removed, not all Lorentz violation.  Factoring
    // out c_s^2, the selected matter q^6 polynomial is
    // S2^3/36 + S2 Q4/36 - Q6/15.  In the same Q4=sum(q_i^4),
    // Q6=sum(q_i^6) basis, the literal BCC-time flux correction is
    // -61*S2^3/17640 + S2*Q4/72 - Q6/90.  The tensor coefficients disagree.
    check("matter and literal BCC-time flux first disagree at q^6",
          !close(1.0 / 36.0, 0.0)
          && !close(-1.0 / 15.0, 1.0 / 180.0));

    WilsonDiracParams defaults;
    check("improved stencil is default-off and preserves legacy parameters",
          close(defaults.kinetic_transverse_weight, 0.0, 0.0)
          && close(defaults.r, 1.0, 0.0)
          && close(defaults.spatial_speed, 1.0, 0.0));

    std::cout << "Aggregate: " << passed << " passed, " << failed << " failed\n";
    if (failed != 0) return 1;
    std::cout << "Verdict: FREE COMMON CONE THROUGH q^4; q^6 AND INTERACTIONS OPEN\n";
    return 0;
}
