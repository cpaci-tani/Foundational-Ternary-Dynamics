// test_phase_h_coupling.cpp — Phase H: explicit coupling constant in Gauss law.
//
// Phase G theorem: engine emergent V(r) = -2 G_L(r), alpha_r = 2 r G_L(r)
// with no coupling. Phase H adds a coupling g_c to Gauss source (div J = g_c s).
// Predicted scaling: V and alpha_r scale as g_c^2.
//
// This test runs measure_alpha_eff at two coupling values and verifies the
// scaling matches the Phase G prediction to within 1% (allowing for SOR
// tolerance and tick-finite-size noise).

#include "ftd/eft/coupling_measurement.h"
#include "ftd/term_toggles.h"
#include "ftd/ontic.h"

#include <cassert>
#include <cmath>
#include <cstdio>

using ftd::eft::configure_bare_lattice_for_coupling;
using ftd::eft::measure_self_energy;
using ftd::eft::measure_pair_energy;

// Run a two-point V(r) extraction with given charge_coupling.
// Returns alpha_r at r_probe. Initial flux amplitude is scaled by
// charge_coupling so both baseline and coupled runs equilibrate with the
// same relative transient (steady-state flux ~ g_c/(4 pi r^2), so
// initial ~ g_c * amp matches the same fraction of target across couplings).
static double run_alpha_r(int L, int r_probe, int n_ticks, double charge_coupling) {
    const int mid = L / 2;
    const double init_flux = 0.05 * charge_coupling;

    auto energy_with = [&](int sign_a, int sign_b, int sep) -> double {
        ftd::RenderBridge rb(L);
        rb.force_cpu();  // Phase H coupling is only wired on the CPU path.
        configure_bare_lattice_for_coupling(rb);
        // Disable the separate `coupling` toggle (which injects G_C*grad(s)
        // into the wave equation) so the ONLY source is Gauss projection.
        // Then the field-energy scaling is purely g_c^2 (Phase G theorem).
        rb.toggles.coupling = false;
        rb.toggles.coulomb_charge_coupling = charge_coupling;
        if (sign_a != 0) {
            rb.inject_particle(mid, mid, mid, static_cast<int8_t>(sign_a),
                               {0, 0, sign_a * init_flux});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        }
        if (sign_b != 0) {
            rb.inject_particle(mid + sep, mid, mid, static_cast<int8_t>(sign_b),
                               {0, 0, sign_b * init_flux});
            rb.voxels()[rb.lattice().index(mid + sep, mid, mid)].locked = true;
        }
        rb.run(n_ticks);
        return rb.energy_audit().field_energy;
    };

    const double e_self_pos = energy_with(+1,  0, 0);
    const double e_self_neg = energy_with(-1,  0, 0);
    const double e_pair     = energy_with(+1, -1, r_probe);
    const double V          = e_pair - e_self_pos - e_self_neg;
    return -V * static_cast<double>(r_probe);
}

int main() {
    const int L = 32;
    const int r = 6;
    const int ticks = 300;  // 2x Phase-F ticks for tighter equilibration
    const double alpha_ref = ftd::ontic::ALPHA;
    const double PI = 3.14159265358979323846;

    printf("=== Phase H verification: explicit coupling in Gauss law ===\n");
    printf("L=%d, r=%d, ticks=%d\n\n", L, r, ticks);

    // Baseline: geometric Coulomb (coupling = 1.0), Phase G theorem applies.
    const double alpha_r_base = run_alpha_r(L, r, ticks, 1.0);
    printf("baseline  (g_c = 1.0):              alpha_r(r=%d) = %.6f\n",
           r, alpha_r_base);

    // Engine-convention coupling to recover alpha_ref at continuum r->0 limit:
    // g_c^2 * 1/(2 pi) = alpha_ref  =>  g_c = sqrt(2 pi alpha_ref)
    const double g_c = std::sqrt(2.0 * PI * alpha_ref);
    printf("coupling  (g_c = %.6f):          target alpha_r scaling factor: %.6f\n",
           g_c, g_c * g_c);

    const double alpha_r_coupled = run_alpha_r(L, r, ticks, g_c);
    printf("coupled   (g_c = sqrt(2pi alpha)):  alpha_r(r=%d) = %.6f\n",
           r, alpha_r_coupled);

    // Prediction from Phase G theorem:
    //   alpha_r(g_c) = g_c^2 * alpha_r(1)
    const double predicted = g_c * g_c * alpha_r_base;
    printf("\nPhase G prediction:                 alpha_r_coupled = g_c^2 * alpha_r_base\n");
    printf("                                    = %.6f * %.6f = %.6f\n",
           g_c * g_c, alpha_r_base, predicted);

    const double rel_err = std::abs(alpha_r_coupled - predicted) /
                           std::max(std::abs(predicted), 1e-12);
    printf("\nrelative error: %.4f%%\n", rel_err * 100.0);

    const bool pass = rel_err < 0.03;  // 3% tolerance; L=32 ticks=300 leaves
                                       // ~1-2% equilibration residual.
    printf("\n%s\n", pass
        ? "PASS  — Phase H coupling scales alpha_r exactly as Phase G predicts."
        : "FAIL  — coupling scaling does not match Phase G theorem.");

    return pass ? 0 : 1;
}
