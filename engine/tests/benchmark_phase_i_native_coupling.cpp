/**
 * Phase I — FTD-Native Coupling Cross-Check (FTD-0125)
 *
 * Pre-registration:
 *   docs/theory/10_eft_program/PREREG_PHASE_I_NATIVE_COUPLING.md
 *   git tag: preregister-phase-i-native-coupling-v1 (commit e1f8157)
 *
 * Hypothesis (committed BEFORE this measurement):
 *   The engine's wave-propagation channel realizes alpha_r factorizing as
 *
 *       alpha_r(r, L) = G_C^2 * 2 * r * G_L(r)                 [engine convention]
 *                     = (1/x_+) * 2 * r * G_L(r)
 *
 *   where G_L(r) is the SC7 lattice Poisson Green's function (Phase G [THEOREM])
 *   and 1/x_+ is the FTD-native coupling derived from the master quadratic
 *   [THEOREM] (FTD-0001).
 *
 * Cross-check: the bare engine measurement of alpha_r (per
 *   experiment_interaction_potential in benchmark_emergent_alpha.cpp), divided
 *   by the FFT-precomputed 2*r*G_L(r), should equal G_C^2 = 1/x_+ to
 *   engine precision at every (L, r) tested.
 *
 * Outcome A (pre-registered, expected): g_engine_sq matches 1/x_+ at every
 *   fixture within 1e-3 relative tolerance (allowing for engine equilibration
 *   tick-count and finite-r convergence). PASS.
 *
 * Outcome B / C: investigation required, see PREREG section 4.
 *
 * Build:
 *   First regenerate fixtures: python scripts/proofs/generate_phase_i_lattice_green_fixtures.py > engine/tests/phase_i_green_fixtures.h
 *   Then: cmake --build engine/build --target benchmark_phase_i_native_coupling
 *
 * Run:
 *   cd engine/build && ./benchmark_phase_i_native_coupling
 *   (or via WSL2: engine/build_wsl/benchmark_phase_i_native_coupling)
 */

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "phase_i_green_fixtures.h"

// -------------------------------------------------------------
// Bare-lattice configuration: only the wave-propagation channel +
// gauss-projection are active. No explicit forces. Replicates the
// configuration of E2 (interaction potential) in
// benchmark_emergent_alpha.cpp.
// -------------------------------------------------------------
static void configure_bare_lattice(ftd::RenderBridge& rb) {
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = false;
    rb.toggles.damping = false;
    rb.toggles.forces = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.gravity = false;
    rb.toggles.movement = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.color_forces = false;
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field = false;
}

// -------------------------------------------------------------
// Measure alpha_r(L, r) at one (L, r) pair, equilibrating to steady state.
//   Returns alpha_r = -V(r) * r where V(r) = E_pair - 2 * E_self.
// -------------------------------------------------------------
struct AlphaResult {
    double E_self_pos;
    double E_self_neg;
    double E_pair;
    double V;
    double alpha_r;
};

static AlphaResult measure_alpha_r(int L, int r, int ticks) {
    const int mid = L / 2;

    // Single-charge self-energy (positive)
    double E_self_pos = 0.0;
    {
        ftd::RenderBridge rb(L);
        configure_bare_lattice(rb);
        rb.inject_particle(mid, mid, mid, +1, {0.0, 0.0, ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(ticks);
        E_self_pos = rb.energy_audit().field_energy;
    }

    // Single-charge self-energy (negative)
    double E_self_neg = 0.0;
    {
        ftd::RenderBridge rb(L);
        configure_bare_lattice(rb);
        rb.inject_particle(mid, mid, mid, -1, {0.0, 0.0, -ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(ticks);
        E_self_neg = rb.energy_audit().field_energy;
    }

    // Pair energy at separation r
    double E_pair = 0.0;
    {
        ftd::RenderBridge rb(L);
        configure_bare_lattice(rb);
        rb.inject_particle(mid, mid, mid, +1, {0.0, 0.0, ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.inject_particle(mid + r, mid, mid, -1, {0.0, 0.0, -ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
        rb.run(ticks);
        E_pair = rb.energy_audit().field_energy;
    }

    AlphaResult result{};
    result.E_self_pos = E_self_pos;
    result.E_self_neg = E_self_neg;
    result.E_pair = E_pair;
    result.V = E_pair - (E_self_pos + E_self_neg);
    result.alpha_r = -result.V * static_cast<double>(r);
    return result;
}

// -------------------------------------------------------------
// Phase I cross-check: g_engine_sq = alpha_r / (2 r G_L(r)) ==? 1/x_+
// -------------------------------------------------------------
int main(int argc, char** argv) {
    const int default_ticks = 200;
    const int ticks = (argc > 1) ? std::atoi(argv[1]) : default_ticks;

    const double g_FTD_sq_expected = ftd::ALPHA_EFT;  // = G_C^2 = 1/x_+ from constants.h
    const double tol_relative = 1e-3;  // 1000 ppm — generous; pre-registered

    std::cerr << "Phase I cross-check (FTD-0125): engine wave-propagation channel\n";
    std::cerr << "Pre-reg: PREREG_PHASE_I_NATIVE_COUPLING.md (tag preregister-phase-i-native-coupling-v1)\n";
    std::cerr << "Equilibration ticks per measurement: " << ticks << "\n";
    std::cerr << "Expected g_FTD^2 = 1/x_+ = " << std::setprecision(15) << g_FTD_sq_expected << "\n";
    std::cerr << "Tolerance: " << tol_relative << " relative\n";
    std::cerr << "\n";

    int n_pass = 0;
    int n_fail = 0;
    int n_total = phase_i::kNumFixtures;

    std::cout << "phase," << "L," << "r," << "alpha_r," << "two_r_G_L,"
              << "g_engine_sq," << "g_FTD_sq," << "rel_err," << "verdict\n";

    for (int i = 0; i < n_total; ++i) {
        const phase_i::GreenFixture& fix = phase_i::kFixtures[i];

        AlphaResult ar = measure_alpha_r(fix.L, fix.r, ticks);
        const double g_engine_sq = ar.alpha_r / fix.two_r_G;
        const double rel_err = std::abs(g_engine_sq - g_FTD_sq_expected) / g_FTD_sq_expected;
        const bool pass = rel_err < tol_relative;

        std::cout << "phase_i_cross_check,"
                  << fix.L << ","
                  << fix.r << ","
                  << std::setprecision(10) << ar.alpha_r << ","
                  << std::setprecision(10) << fix.two_r_G << ","
                  << std::setprecision(10) << g_engine_sq << ","
                  << std::setprecision(10) << g_FTD_sq_expected << ","
                  << std::setprecision(4) << rel_err << ","
                  << (pass ? "PASS" : "FAIL")
                  << "\n";

        std::cerr << "  L=" << fix.L << " r=" << fix.r
                  << "  alpha_r=" << std::setprecision(8) << ar.alpha_r
                  << "  2rG=" << std::setprecision(8) << fix.two_r_G
                  << "  g_engine^2=" << std::setprecision(8) << g_engine_sq
                  << "  rel=" << std::setprecision(3) << rel_err
                  << "  " << (pass ? "PASS" : "FAIL") << "\n";

        if (pass) ++n_pass; else ++n_fail;
    }

    std::cerr << "\n";
    std::cerr << "Aggregate: " << n_pass << " / " << n_total << " PASS\n";

    if (n_fail == 0) {
        std::cerr << "OUTCOME A (pre-registered, expected): g_engine^2 = g_FTD^2 = 1/x_+\n";
        std::cerr << "  Engine wave-propagation channel realizes the master-quadratic-derived\n";
        std::cerr << "  coupling self-consistently across all tested (L, r). Phase I closure\n";
        std::cerr << "  POSITIVE.\n";
        return 0;
    } else {
        std::cerr << "NON-A OUTCOME — see per-fixture rel_err values above. Investigate.\n";
        return 1;
    }
}
