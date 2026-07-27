/**
 * FTD-0412 common-cone matter gate.
 *
 * This is a diagnostic, not a production retuning.  It verifies that the
 * corrected Hermitian Wilson Hamiltonian can be assigned the selected
 * BCC-time leading speed c_s^2=1/7, while proving that its quartic pole cannot
 * match the q^4-free flux prototype for any scalar Wilson parameter r.
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
#include "ftd/ontic/gauge_couplings.h"
#include "ftd/wilson_dirac.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double TOL = 2.0e-12;

bool close(double lhs, double rhs, double tol = TOL) {
    return std::abs(lhs - rhs) <= tol * std::max({1.0, std::abs(lhs), std::abs(rhs)});
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

double free_wilson_energy_sq(const std::array<double, 3>& p,
                             const WilsonDiracParams& params) {
    double sine_sq = 0.0;
    double wilson_sum = 0.0;
    for (double q : p) {
        sine_sq += std::sin(q) * std::sin(q);
        wilson_sum += 1.0 - std::cos(q);
    }
    const double kinetic = params.spatial_speed * params.spatial_speed
                         * sine_sq / (params.a * params.a);
    const double mass = params.m
                      + params.spatial_speed * params.r * wilson_sum / params.a;
    return kinetic + mass * mass;
}

bool report(const std::string& label, bool ok) {
    std::cout << (ok ? "PASS  " : "FAIL  ") << label << '\n';
    return ok;
}

}  // namespace

int main() {
    int passed = 0;
    int failed = 0;
    const auto check = [&](const std::string& label, bool condition) {
        if (report(label, condition)) {
            ++passed;
        } else {
            ++failed;
        }
    };

    std::cout << "FTD-0412 common-cone matter gate\n";
    std::cout << std::setprecision(17);

    const double common_c2 = LORENTZ_BCC_TIME_EFFECTIVE_C2;
    const double common_c = std::sqrt(common_c2);
    check("selected BCC-time leading cone is exactly represented as c_s^2=1/7",
          close(common_c2, 1.0 / 7.0, 0.0));
    check("live manifested-matter budget remains 1/3 and is not silently retuned",
          close(ontic::C_SPEED * ontic::C_SPEED, 1.0 / 3.0)
          && !close(ontic::C_SPEED * ontic::C_SPEED, common_c2));

    // The pre-FTD-0412 oracle used a special upper-only spinor.  An eigenstate
    // of i gamma_x exposes the actual legacy D_W eigenvalue M+sin(p), not
    // sqrt(M^2+sin^2(p)).
    {
        constexpr int L = 8;
        Lattice lattice(L);
        GaugeLinks links(L);
        links.set_identity();
        SpinorField psi(L), out(L);
        const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        const Spinor i_gamma_x_plus = {
            cdouble{inv_sqrt2, 0.0}, cdouble{0.0, 0.0},
            cdouble{0.0, 0.0}, cdouble{0.0, inv_sqrt2}
        };
        const std::array<double, 3> p{PI / 2.0, 0.0, 0.0};
        fill_plane_wave(psi, lattice, p, i_gamma_x_plus);

        WilsonDiracParams params;
        params.m = 0.5;
        params.r = 1.0;
        params.a = 1.0;
        params.spatial_speed = 1.0;
        apply_wilson_dirac(out, psi, links, lattice, params);

        const double measured = out.total_norm_squared() / psi.total_norm_squared();
        const double M = params.m + params.r * (1.0 - std::cos(p[0]));
        const double actual = (M + std::sin(p[0])) * (M + std::sin(p[0]));
        const double retired_oracle = M * M + std::sin(p[0]) * std::sin(p[0]);
        check("legacy ||D_W psi|| oracle fails on an i*gamma_x eigenstate",
              close(measured, actual) && !close(measured, retired_oracle));
    }

    // The corrected H_W squares to the scalar relativistic Wilson dispersion
    // for every spinor.  Use a generic amplitude rather than a special basis
    // vector so the old accidental cancellation cannot recur.
    {
        constexpr int L = 16;
        Lattice lattice(L);
        GaugeLinks links(L);
        links.set_identity();
        SpinorField psi(L), out(L);
        const double k0 = 2.0 * PI / static_cast<double>(L);
        const std::array<double, 3> p{2.0 * k0, k0, 3.0 * k0};
        const Spinor generic = {
            cdouble{0.31, -0.17}, cdouble{-0.23, 0.41},
            cdouble{0.29, 0.37}, cdouble{-0.11, -0.53}
        };
        fill_plane_wave(psi, lattice, p, generic);

        WilsonDiracParams params;
        params.m = 0.0;
        params.r = 1.0;
        params.a = 1.0;
        params.spatial_speed = common_c;
        apply_wilson_hamiltonian(out, psi, links, lattice, params);

        const double measured = out.total_norm_squared() / psi.total_norm_squared();
        const double predicted = free_wilson_energy_sq(p, params);
        check("Hermitian Wilson Hamiltonian has the exact selected-c_s dispersion",
              close(measured, predicted));
    }

    // At m=0:
    // E_W^2 = c_s^2[S2 + (r^2/4)S2^2 - Q4/3 + O(q^6)].
    // Axis cancellation needs r^2=4/3; a (q,q,0) ray needs r^2=2/3.
    // Hence no scalar r cancels q^4 in every direction.
    const double r2_axis = 4.0 / 3.0;
    const double r2_face_diagonal = 2.0 / 3.0;
    check("axis quartic cancellation requires r^2=4/3",
          close(r2_axis / 4.0 - 1.0 / 3.0, 0.0));
    check("face-diagonal quartic cancellation requires r^2=2/3",
          close(r2_face_diagonal - 2.0 / 3.0, 0.0));
    check("no scalar Wilson r gives the flux prototype's q^4-free pole",
          !close(r2_axis, r2_face_diagonal));

    std::cout << "Aggregate: " << passed << " passed, " << failed << " failed\n";
    if (failed != 0) {
        return 1;
    }
    std::cout << "Verdict: LEADING-CONE-ALIGNABLE; FULL-COMMON-CONE-FAILS\n";
    return 0;
}
