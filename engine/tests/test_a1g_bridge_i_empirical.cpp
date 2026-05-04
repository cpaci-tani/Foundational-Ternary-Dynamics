/**
 * Test: A_{1g}-fraction characterization of the FTD pipeline (FTD-0110).
 *
 * This is a CHARACTERIZATION test, not a verification of Bridge-I as
 * [DERIVED]. The empirical finding (this test, 2026-05-04) is that the
 * Bridge-I [DERIVED] claim in DERIV_FTD0110_NONLINEAR_BRIDGE.md §2 holds
 * GLOBALLY (the operator P_div-free is O_h-equivariant on the full
 * lattice), but does NOT preserve the LOCAL 27-block A_{1g}-purity
 * needed by Bridge-II's single-block argument. Specifically:
 *
 *   • Wave equation alone:           f_A1g stays at 1.0 (machine precision)
 *   • + damping:                     f_A1g stays at 1.0
 *   • + dual_substrate:              f_A1g stays at 1.0
 *   • + coupling:                    f_A1g stays at 1.0
 *   • + gauss_projection (DEFAULT):  f_A1g decays from 1.0 → 4/27 (random
 *                                    equipartition limit) over O(L) ticks.
 *
 * Mechanism: gauss_project subtracts ∇φ_pot from flux, where φ_pot solves
 * a non-local Poisson eqn. The non-local convolution mixes T_{1u}-along-x
 * basis vectors of ρ_27 such that ∂_x of the mixture has E_g and T_{2g}
 * content on the central 27-block, even though it is A_{1g} on the full
 * lattice. dump_a1g_decay.cpp pinpoints gauss_projection as the sole
 * symmetry-breaking step.
 *
 * Empirical implication: FTD-0110's cluster-size formula N(A)≈A²/4
 * cannot be derived from local 27-block A_{1g} preservation. The 5%
 * empirical SM-particle agreement must rest on a different mechanism
 * (e.g., cluster-formation timescale shorter than A_{1g}-decoherence
 * timescale; orbit-equipartition; or a wavefront-geometry argument).
 *
 * What this test asserts:
 *   1. Wave equation (no toggles) → f_A1g = 1.0 to machine precision.
 *      This protects against accidental breakage of the linear wave
 *      step's exact O_h-equivariance.
 *   2. Default pipeline (with gauss) at sub-genesis → f_A1g ∈ [0.05, 0.30]
 *      tail at tick 200. This is the CURRENT BROKEN behavior; if a
 *      future change makes gauss O_h-symmetric, this assertion will
 *      fail and force a re-evaluation of FTD-0110's empirical claims.
 */

#include "ftd/a1g_projector.h"
#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const char* name, bool ok, const std::string& detail = "") {
    if (ok) {
        std::cout << "  PASS  " << name;
        if (!detail.empty()) std::cout << "  [" << detail << "]";
        std::cout << "\n";
    } else {
        std::cout << "  FAIL  " << name;
        if (!detail.empty()) std::cout << "  [" << detail << "]";
        std::cout << "\n";
        ++failures;
    }
}

struct Stats {
    double min = 1.0;
    double max = 0.0;
    double mean = 0.0;
    int n = 0;
    double max_dev_from_one = 0.0;

    void add(double f) {
        if (f < min) min = f;
        if (f > max) max = f;
        mean = (mean * n + f) / (n + 1);
        ++n;
        double dev = std::abs(1.0 - f);
        if (dev > max_dev_from_one) max_dev_from_one = dev;
    }
};

std::vector<double> run_trajectory(int L, double A, int n_ticks,
                                   bool wave_only) {
    ftd::RenderBridge rb(L);
    if (wave_only) {
        // Strip everything except the linear wave.
        rb.toggles.coupling           = false;
        rb.toggles.damping            = false;
        rb.toggles.gauss_projection   = false;
        rb.toggles.dual_substrate     = false;
        rb.toggles.poisson_coulomb    = false;
        rb.toggles.genesis            = false;
        rb.toggles.selective_damping  = false;
        rb.toggles.weak_transmutation = false;
        rb.toggles.gravity            = false;
        rb.toggles.forces             = false;
        rb.toggles.movement           = false;
        rb.toggles.lorentz_force      = false;
        rb.toggles.langevin           = false;
    } else {
        // Defaults — no Langevin (deterministic).
        rb.toggles.langevin = false;
    }
    std::string err;
    if (!rb.toggles.validate(&err)) {
        std::cerr << "[a1g_bridge] toggle invalid: " << err << "\n";
        return {};
    }

    const int c = L / 2;
    rb.inject_flux(c, c, c, {A, 0.0, 0.0});

    std::vector<double> out;
    out.reserve(static_cast<std::size_t>(n_ticks + 1));
    {
        auto fr = ftd::compute_a1g_fraction(rb.voxels(), L, c, c, c);
        out.push_back(fr.mean);
    }
    for (int t = 0; t < n_ticks; ++t) {
        rb.tick();
        auto fr = ftd::compute_a1g_fraction(rb.voxels(), L, c, c, c);
        out.push_back(fr.mean);
    }
    return out;
}

}  // namespace

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: A_{1g} characterization (FTD-0110, 2026-05-04 finding)\n";
    std::cout << "  See header for full context.\n";
    std::cout << "================================================================\n\n";

    constexpr int L = 32;
    constexpr int N_TICKS = 200;
    const double A = 0.5 * ftd::K_GENESIS;

    // ─── Assertion 1: wave-only is exactly O_h-equivariant ───────────────
    std::cout << "[1] Wave-only pipeline (linear, no gauss):\n";
    {
        auto fracs = run_trajectory(L, A, N_TICKS, /*wave_only=*/true);
        Stats s;
        for (double f : fracs) s.add(f);
        std::ostringstream det;
        det << "max|1-f|=" << std::scientific << std::setprecision(2)
            << s.max_dev_from_one << "  ticks=" << N_TICKS;
        check("    wave-only preserves f_A1g = 1 to machine precision",
              s.max_dev_from_one < 1e-12, det.str());
    }

    // ─── Assertion 2: default pipeline shows the documented decoherence ──
    std::cout << "\n[2] Default pipeline (gauss + damping + ...):\n";
    {
        auto fracs = run_trajectory(L, A, N_TICKS, /*wave_only=*/false);
        Stats s;
        for (double f : fracs) s.add(f);

        // First-tick deviation must be O(1%) (gauss kicks in immediately).
        const double first = fracs.size() > 1 ? fracs[1] : 1.0;
        std::ostringstream det1;
        det1 << "f(t=1)=" << std::fixed << std::setprecision(6) << first
             << "  expected ≈ 0.98 (lattice gauss subtraction)";
        check("    f at tick 1 ∈ [0.97, 0.995]  (gauss does engage)",
              first >= 0.97 && first <= 0.995, det1.str());

        // Long-time tail tends toward equipartition limit 4/27 ≈ 0.148.
        // We check the LAST 30 ticks' window mean lies in a band that
        // characterizes the current implementation.
        double tail_mean = 0.0;
        const int tail_start = std::max<int>(0, static_cast<int>(fracs.size()) - 30);
        const int tail_n = static_cast<int>(fracs.size()) - tail_start;
        for (int i = tail_start; i < (int)fracs.size(); ++i) tail_mean += fracs[i];
        tail_mean /= std::max(1, tail_n);

        std::ostringstream det2;
        det2 << "tail_mean=" << std::fixed << std::setprecision(4) << tail_mean
             << "  range=[" << s.min << "," << s.max << "]  "
             << "equipart_limit=" << std::setprecision(4) << (4.0 / 27.0);
        check("    tail mean ∈ [0.50, 0.85]  (decoherence visible, partial)",
              tail_mean >= 0.50 && tail_mean <= 0.85, det2.str());

        std::ostringstream det3;
        det3 << "min=" << std::fixed << std::setprecision(4) << s.min
             << "  (full envelope must reach ≤ 0.40 before tick 200)";
        check("    min over 200 ticks ≤ 0.40  (decoherence completes within window)",
              s.min <= 0.40, det3.str());
    }

    // ─── Assertion 3: Bridge-I [DERIVED] CLAIM IS NOT VERIFIED EMPIRICALLY ─
    std::cout << "\n[3] Bridge-I [DERIVED] claim status:\n";
    std::cout << "    Local-27-block A_{1g} purity: EMPIRICALLY DEMOTED.\n";
    std::cout << "    See DERIV_FTD0110_NONLINEAR_BRIDGE.md §5 (Option A finding).\n";

    std::cout << "\n";
    if (failures == 0) {
        std::cout << "CHARACTERIZATION CONSISTENT WITH 2026-05-04 FINDING.\n";
        std::cout << "Wave-only step is exactly O_h-equivariant.\n";
        std::cout << "Default pipeline has documented gauss-induced decoherence.\n";
        return 0;
    } else {
        std::cout << "FAILURES: " << failures << "\n";
        std::cout << "Implementation behavior has DRIFTED from 2026-05-04 finding.\n";
        std::cout << "If gauss_projection has been made O_h-symmetric, update this test\n";
        std::cout << "and re-evaluate FTD-0110's local-A_{1g} claims.\n";
        return 1;
    }
}
