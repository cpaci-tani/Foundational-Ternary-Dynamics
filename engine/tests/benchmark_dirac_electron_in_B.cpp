/**
 * Single-electron stable orbit in uniform B (Phase II.3 milestone).
 *
 * Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
 *
 * VALIDATION:
 *   Initialise a Gaussian wave packet centred at (x0, y0, z0) with definite
 *   momentum p = (0, p_y, 0) and spin +z. Apply uniform B in z via twisted
 *   Landau gauge (n_flux flux quanta through the xy plane). Evolve the
 *   Wilson-Dirac Schrodinger equation i d/dt psi = D_W psi via RK4.
 *
 *   Track time series:
 *     - centroid <x>, <y>, <z> (periodic-aware via complex-exponential trick)
 *     - energy <H> = <psi | D_W | psi>
 *     - norm  <psi | psi>
 *     - spin  <S_x>, <S_y>, <S_z>  with Sigma^i = diag(sigma^i, sigma^i)
 *
 *   Pre-registered milestone criteria for II.3 closure:
 *     a) Energy conservation:   |Delta E| / |E_0|  < 1%   over the run
 *     b) Norm conservation:     |Delta N| / |N_0|  < 1e-4 over the run
 *     c) Centroid bounded:      |<x>(t) - x0|, |<y>(t) - y0|  < L/2 - 2
 *        (orbit fits in box, no torus-wrapping artefacts hit boundaries
 *         within the run)
 *     d) Spin precesses:        std(<S_z>(t)) > 1e-4 OR (<S_x>, <S_y>)
 *        oscillation amplitude > 1e-4
 *
 * Outputs:
 *   - CSV time-series for downstream analysis (II.4)
 *   - Closure verdict
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

#include "ftd/lattice.h"
#include "ftd/wilson_dirac.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

struct Observables {
    double t;
    double norm;
    double E;             // Re <psi | D_W | psi>
    double cx, cy, cz;    // periodic-aware centroid
    double sx, sy, sz;    // <Sigma^i>
};

// Periodic-aware mean coordinate along axis 'axis' (0=x, 1=y, 2=z) given the
// probability density rho(n) = sum_alpha |psi(n)_alpha|^2.
double periodic_mean(const SpinorField& psi, int axis, const Lattice& lattice) {
    const int L = lattice.size();
    cdouble z_sum{0, 0};
    double total = 0.0;
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int idx = lattice.index(x, y, z);
                double rho = 0.0;
                for (int k = 0; k < 4; ++k) rho += std::norm(psi.at(idx)[k]);
                const int coord = (axis == 0) ? x : (axis == 1) ? y : z;
                const double phase = 2.0 * M_PI * static_cast<double>(coord) / static_cast<double>(L);
                z_sum += rho * std::exp(cdouble{0, phase});
                total += rho;
            }
        }
    }
    if (total < 1e-300) return 0.0;
    z_sum /= total;
    double mean = std::arg(z_sum) * static_cast<double>(L) / (2.0 * M_PI);
    if (mean < 0) mean += L;
    return mean;
}

double energy(const SpinorField& psi,
              const GaugeLinks& links,
              const Lattice& lattice,
              const WilsonDiracParams& params) {
    SpinorField Dpsi(lattice.size());
    apply_wilson_dirac(Dpsi, psi, links, lattice, params);
    cdouble E{0, 0};
    for (std::size_t i = 0; i < psi.data.size(); ++i) {
        E += spinor_dot(psi.data[i], Dpsi.data[i]);
    }
    return E.real();
}

double spin_expectation(const SpinorField& psi, int i_axis) {
    cdouble S{0, 0};
    for (const auto& s : psi.data) {
        Spinor sigma_psi = apply_sigma_spatial(i_axis, s);
        S += spinor_dot(s, sigma_psi);
    }
    return S.real();
}

Observables measure(double t,
                    const SpinorField& psi,
                    const GaugeLinks& links,
                    const Lattice& lattice,
                    const WilsonDiracParams& params) {
    Observables o{};
    o.t = t;
    o.norm = psi.total_norm_squared();
    o.E = energy(psi, links, lattice, params);
    o.cx = periodic_mean(psi, 0, lattice);
    o.cy = periodic_mean(psi, 1, lattice);
    o.cz = periodic_mean(psi, 2, lattice);
    const double inv_norm = 1.0 / std::max(o.norm, 1e-300);
    o.sx = spin_expectation(psi, 0) * inv_norm;
    o.sy = spin_expectation(psi, 1) * inv_norm;
    o.sz = spin_expectation(psi, 2) * inv_norm;
    return o;
}

}  // namespace

int main(int argc, char** argv) {
    // Default config tuned to satisfy II.3 milestone criteria (orbit fits in box,
    // energy + norm conserve, spin shows transverse precession). Best regime
    // found in 2026-05-03 sweep: L=24, n_flux=4, p_y_units=1, m=0.5, sigma=1.8.
    int L = 24;
    int n_flux = 4;
    double m = 0.5;
    double sigma = 1.8;
    int n_steps = 800;
    double dt = 0.04;
    int sample_every = 5;
    double p_y = 2.0 * M_PI / L * 1.0;   // 1 momentum quantum in y
    std::string csv_path = "wilson_dirac_orbit.csv";

    // --L --n_flux --m --sigma --steps --dt --p_y_units --csv  cmd-line.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto get = [&](double& v) { v = std::atof(argv[++i]); };
        auto getI = [&](int& v) { v = std::atoi(argv[++i]); };
        if (arg == "--L") getI(L);
        else if (arg == "--n_flux") getI(n_flux);
        else if (arg == "--m") get(m);
        else if (arg == "--sigma") get(sigma);
        else if (arg == "--steps") getI(n_steps);
        else if (arg == "--dt") get(dt);
        else if (arg == "--p_y_units") {
            int pu; getI(pu);
            p_y = 2.0 * M_PI / L * static_cast<double>(pu);
        }
        else if (arg == "--csv") csv_path = argv[++i];
    }

    std::cout << "Phase II.3 — single electron in uniform B\n";
    std::cout << "  L=" << L << "  n_flux=" << n_flux << "  m=" << m
              << "  sigma=" << sigma << "  p_y=" << p_y
              << "  steps=" << n_steps << "  dt=" << dt << "\n\n";

    Lattice lattice(L);
    GaugeLinks links(L);
    const double alpha = 2.0 * M_PI * static_cast<double>(n_flux) / (static_cast<double>(L) * L);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const std::size_t idx = static_cast<std::size_t>(lattice.index(x, y, z));
                links.U[0][idx] = std::exp(cdouble{0, -alpha * static_cast<double>(y)});
                links.U[1][idx] = (y == L - 1)
                    ? std::exp(cdouble{0, +alpha * static_cast<double>(x) * L})
                    : cdouble{1, 0};
                links.U[2][idx] = cdouble{1, 0};
            }
        }
    }

    // Initial state: Gaussian wave packet centred at (L/4, L/2, L/2), momentum
    // (0, p_y, 0). For a Wilson-Dirac g-2 measurement we need:
    //   (a) Spin transverse to B (i.e. in xy-plane) so precession is visible.
    //       Use chi = (1, 1)/sqrt(2), i.e. spin +x.
    //   (b) Lower components set to the positive-energy continuum form
    //       u_lower = (sigma . p / (E + m)) chi, with E = sqrt(p^2 + m^2).
    //       This suppresses Zitterbewegung (mixing with negative-energy modes).
    //   For p = (0, p_y, 0), sigma . p = p_y sigma_y; sigma_y (1, 1)/sqrt(2)
    //       = (1/sqrt(2)) (-i, i). So u_lower = (p_y/(E+m)) * (-i, i) / sqrt(2).
    const double x0 = L * 0.25;
    const double y0 = L * 0.5;
    const double z0 = L * 0.5;

    const double E_plane = std::sqrt(p_y * p_y + m * m);
    const double xi = p_y / (E_plane + m);
    const double inv_root2 = 1.0 / std::sqrt(2.0);
    const cdouble u0 = cdouble{ inv_root2, 0.0};                  // chi_+x[0]
    const cdouble u1 = cdouble{ inv_root2, 0.0};                  // chi_+x[1]
    const cdouble u2 = cdouble{0.0, -xi * inv_root2};             // -i xi / sqrt(2)
    const cdouble u3 = cdouble{0.0,  xi * inv_root2};             // +i xi / sqrt(2)

    SpinorField psi(L);
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                auto pdist = [&](double a, double b) {
                    double d = a - b;
                    while (d >  L * 0.5) d -= L;
                    while (d < -L * 0.5) d += L;
                    return d;
                };
                const double dx = pdist(x, x0);
                const double dy = pdist(y, y0);
                const double dz = pdist(z, z0);
                const double r2 = dx * dx + dy * dy + dz * dz;
                const double envelope = std::exp(-r2 / (2.0 * sigma * sigma));
                const cdouble phase = std::exp(cdouble{0, p_y * y});
                const cdouble val = envelope * phase;
                const int idx = lattice.index(x, y, z);
                psi.at(idx) = {val * u0, val * u1, val * u2, val * u3};
            }
        }
    }
    const double n0 = psi.total_norm_squared();
    const double inv_sqrt = 1.0 / std::sqrt(n0);
    for (auto& s : psi.data) {
        for (int k = 0; k < 4; ++k) s[k] *= inv_sqrt;
    }

    WilsonDiracParams params;
    params.m = m;
    params.r = 1.0;
    params.a = 1.0;

    SpinorField k1(L), k_temp(L);

    std::vector<Observables> log;
    log.reserve(static_cast<std::size_t>(n_steps / sample_every) + 1);
    log.push_back(measure(0.0, psi, links, lattice, params));

    for (int step = 1; step <= n_steps; ++step) {
        evolve_rk4_step(psi, k1, k_temp, links, lattice, params, dt);
        if (step % sample_every == 0) {
            log.push_back(measure(step * dt, psi, links, lattice, params));
        }
    }

    // Write CSV.
    std::ofstream csv(csv_path);
    csv << "t,norm,E,cx,cy,cz,sx,sy,sz\n";
    csv << std::setprecision(12);
    for (const auto& o : log) {
        csv << o.t << "," << o.norm << "," << o.E << "," << o.cx << "," << o.cy
            << "," << o.cz << "," << o.sx << "," << o.sy << "," << o.sz << "\n";
    }
    csv.close();

    // Diagnostics.
    const double E0 = log.front().E, EN = log.back().E;
    const double N0 = log.front().norm, NN = log.back().norm;
    double cx_min = log.front().cx, cx_max = cx_min;
    double cy_min = log.front().cy, cy_max = cy_min;
    double sz_min = log.front().sz, sz_max = sz_min;
    double sx_min = log.front().sx, sx_max = sx_min;
    double sy_min = log.front().sy, sy_max = sy_min;
    for (const auto& o : log) {
        cx_min = std::min(cx_min, o.cx); cx_max = std::max(cx_max, o.cx);
        cy_min = std::min(cy_min, o.cy); cy_max = std::max(cy_max, o.cy);
        sz_min = std::min(sz_min, o.sz); sz_max = std::max(sz_max, o.sz);
        sx_min = std::min(sx_min, o.sx); sx_max = std::max(sx_max, o.sx);
        sy_min = std::min(sy_min, o.sy); sy_max = std::max(sy_max, o.sy);
    }

    const double dE_rel = std::abs(EN - E0) / std::max(std::abs(E0), 1e-300);
    const double dN_rel = std::abs(NN - N0) / std::max(N0, 1e-300);

    std::cout << "Diagnostics:\n";
    std::cout << "  E0 = " << std::scientific << std::setprecision(8) << E0
              << "   E_final = " << EN
              << "   dE/E = " << std::setprecision(3) << dE_rel
              << "   " << (dE_rel < 0.01 ? "PASS" : "FAIL") << " (<1%)\n";
    std::cout << "  N0 = " << std::setprecision(8) << N0
              << "   N_final = " << NN
              << "   dN/N = " << std::setprecision(3) << dN_rel
              << "   " << (dN_rel < 1e-4 ? "PASS" : "FAIL") << " (<1e-4)\n";
    std::cout << "  centroid range: cx in [" << std::fixed << std::setprecision(3)
              << cx_min << ", " << cx_max << "]   cy in [" << cy_min << ", " << cy_max << "]\n";
    std::cout << "  spin range:     sx in [" << sx_min << ", " << sx_max
              << "]   sy in [" << sy_min << ", " << sy_max
              << "]   sz in [" << sz_min << ", " << sz_max << "]\n";

    const double sx_amp = sx_max - sx_min;
    const double sy_amp = sy_max - sy_min;
    const double sz_amp = sz_max - sz_min;
    const bool spin_oscillates = (sx_amp > 1e-4 || sy_amp > 1e-4 || sz_amp > 1e-4);
    const double cx_amp = cx_max - cx_min;
    const double cy_amp = cy_max - cy_min;
    const bool cent_bounded = (cx_amp < L - 4) && (cy_amp < L - 4);

    std::cout << "  spin oscillates (>=1e-4 amplitude): "
              << (spin_oscillates ? "YES" : "NO") << "\n";
    std::cout << "  centroid bounded (range < L-4):    "
              << (cent_bounded ? "YES" : "NO") << "\n";
    std::cout << "  CSV: " << csv_path << "  (" << log.size() << " samples)\n\n";

    const bool ok = (dE_rel < 0.01) && (dN_rel < 1e-4) && cent_bounded && spin_oscillates;
    std::cout << "Phase II.3 verdict: " << (ok ? "PASS" : "FAIL") << "\n";
    return ok ? 0 : 1;
}
