/**
 * Wilson-Dirac CPU/GPU parity (Phase II.2-E milestone).
 *
 * Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
 * Spec:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md  section 8.5
 *
 * VALIDATION:
 *   For random spinor and random U(1) gauge link inputs, the GPU
 *   implementation of D_W must reproduce the CPU implementation to
 *   ~machine precision per site. Tolerance: 1e-12 worst per-site
 *   relative error. The CPU and GPU run the same operator with the
 *   same constant shift (m + 3r/a) and same chiral-basis gamma matrices;
 *   only the order of vector instructions and FMA contraction may differ.
 *
 *   Tests run on three configurations:
 *     L=8 with identity links (baseline)
 *     L=12 with random U(1) links (typical)
 *     L=16 with twisted Landau gauge for uniform B (physics-relevant)
 *
 * Outcomes:
 *   PASS  -> Phase II.2-E milestone CLOSED, Phase II.2 fully closed
 *   FAIL  -> investigation required (likely an indexing or sign mismatch)
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
#include "ftd/wilson_dirac_gpu.h"

using namespace ftd;
using namespace ftd::wilson_dirac;

namespace {

constexpr double TOL = 1e-12;

double diff_per_site_max_rel(const SpinorField& a, const SpinorField& b) {
    double worst = 0.0;
    for (std::size_t i = 0; i < a.data.size(); ++i) {
        double diff_sq = 0.0;
        double ref_sq = 0.0;
        for (int k = 0; k < 4; ++k) {
            const auto d = a.data[i][k] - b.data[i][k];
            diff_sq += std::norm(d);
            ref_sq  += std::norm(b.data[i][k]);
        }
        if (ref_sq < 1e-300) continue;  // skip zero-spinor sites
        const double rel = std::sqrt(diff_sq / ref_sq);
        if (rel > worst) worst = rel;
    }
    return worst;
}

void fill_random_spinor(SpinorField& psi, std::mt19937& rng) {
    std::normal_distribution<double> d(0.0, 1.0);
    for (auto& s : psi.data) {
        s = {cdouble{d(rng), d(rng)}, cdouble{d(rng), d(rng)},
             cdouble{d(rng), d(rng)}, cdouble{d(rng), d(rng)}};
    }
}

void fill_random_u1(GaugeLinks& links, std::mt19937& rng) {
    std::uniform_real_distribution<double> p(-M_PI, M_PI);
    for (int mu = 0; mu < 3; ++mu) {
        for (auto& U : links.U[mu]) {
            U = std::exp(cdouble{0.0, p(rng)});
        }
    }
}

bool run_case(const char* name, int L, int link_kind, std::mt19937& rng) {
    Lattice lattice(L);
    GaugeLinks links(L);

    if (link_kind == 0) {
        links.set_identity();
    } else if (link_kind == 1) {
        fill_random_u1(links, rng);
    } else {
        // Twisted Landau gauge for uniform B in z (1 flux quantum).
        const double alpha = 2.0 * M_PI / (static_cast<double>(L) * L);
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
    }

    SpinorField psi(L);
    fill_random_spinor(psi, rng);

    WilsonDiracParams params;
    params.m = 0.5;
    params.r = 1.0;
    params.a = 1.0;

    SpinorField out_cpu(L), out_gpu(L);
    apply_wilson_dirac(out_cpu, psi, links, lattice, params);
    apply_wilson_dirac_gpu(out_gpu, psi, links, lattice, params);

    const double worst = diff_per_site_max_rel(out_gpu, out_cpu);
    const bool ok = worst < TOL;

    std::cout << "  " << name
              << "  L=" << L << "  worst per-site rel_err = "
              << std::scientific << std::setprecision(6) << worst
              << "  " << (ok ? "PASS" : "FAIL") << "\n";
    return ok;
}

}  // namespace

int main() {
    std::cout << "Wilson-Dirac CPU/GPU parity (Phase II.2-E)\n";
    std::cout << "Spec: SPEC_WILSON_DIRAC_FTD.md section 8.5\n\n";

    std::mt19937 rng(0xa5a5beef);

    int passed = 0, failed = 0;
    if (run_case("identity links     ",  8, 0, rng)) ++passed; else ++failed;
    if (run_case("random U(1) links  ", 12, 1, rng)) ++passed; else ++failed;
    if (run_case("uniform B (twisted)", 16, 2, rng)) ++passed; else ++failed;

    std::cout << "\nAggregate: " << passed << " passed, " << failed << " failed\n";

    if (failed == 0) {
        std::cout << "Phase II.2-E milestone: CLOSED. CPU/GPU parity verified.\n";
        return 0;
    } else {
        std::cout << "Phase II.2-E milestone: INVESTIGATION REQUIRED.\n";
        return 1;
    }
}
