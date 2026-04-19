#pragma once
/**
 * @file ftd/eft/gauss_projection_ext.h
 * @brief High-tolerance Gauss projection for EFT measurements (post-campaign).
 *
 * Motivation
 * ----------
 * Phase 1C (Ward identities) and Phase 2 (β-function) both found that
 * residual ∇·J − ρ in vacuum voxels is dominated by the engine's fixed
 * 6-iteration SOR sweep per tick (see engine/src/poisson_solvers.cpp,
 * `SOR_ITERATIONS = 6`). At ω = 1.75 this converges to O(1%) of |J|_max,
 * not the machine-precision level the SPEC §4.3 pre-registration assumed.
 *
 * This header provides `eft::gauss_project_converged(rb, tol, max_cycles)`
 * which repeatedly triggers the engine's gauss_projection phase WITHOUT
 * running any other dynamics, until the pointwise residual falls below
 * `tol`. Used for tightening Ward-identity measurements and pre-β
 * α_eff extraction without altering the engine's hot-path tick behaviour.
 *
 * Mechanism
 * ---------
 * One could reimplement SOR externally, but that duplicates
 * `poisson_solvers.cpp`. Cleaner: temporarily disable every other toggle
 * that perturbs the flux field, call `rb.tick()` K times (each runs 6 SOR
 * sweeps of the existing solver), check residual, stop when below
 * tolerance, restore toggles.
 *
 * Per-tick cost with everything off except gauss_projection is
 * dominated by the six SOR sweeps + a minimal phase bookkeeping, so
 * effective iterations = 6 × max_cycles. For L = 32 and 100 cycles
 * the runtime is under a second.
 *
 * Epistemic status: infrastructure only. No new physics; this does not
 * alter engine dynamics in any hot path. Tag: [TOOLING].
 */

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "ftd/render_bridge.h"
#include "ftd/term_toggles.h"

namespace ftd {
namespace eft {

/// Result of a gauss-projection-to-tolerance run.
struct ConvergenceReport {
    int cycles = 0;                 ///< number of tick() calls actually performed
    double initial_max_residual = 0.0;
    double final_max_residual = 0.0;
    double final_rms_residual = 0.0;
    double tolerance = 0.0;
    bool converged = false;
};

/// Measure max |∇·J − s| over VACUUM voxels (s == 0). This is the same
/// quantity that `ftd::eft::gauss_identity()` reports — we inline it here
/// to avoid pulling the Ward-identity header as a dependency.
inline void _measure_vacuum_gauss_residual(const RenderBridge& rb,
                                           double& max_abs, double& rms)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int N = lat.total_sites();
    max_abs = 0.0;
    double sum_sq = 0.0;
    long long n = 0;
    for (int i = 0; i < N; ++i) {
        if (vox[i].state != 0) continue;
        const double div = rb.divergence_flux(i);
        const double v = div;  // ρ = state = 0 in vacuum
        const double a = std::abs(v);
        if (a > max_abs) max_abs = a;
        sum_sq += v * v;
        ++n;
    }
    rms = (n > 0) ? std::sqrt(sum_sq / static_cast<double>(n)) : 0.0;
}

/// Iterate gauss projection until max |∇·J − s| over vacuum voxels is
/// below `tol`, or until `max_cycles` iterations have run.
///
/// Every toggle that could perturb the flux field is disabled for the
/// duration of the iteration and restored on exit. Particles
/// (state ≠ 0) are intentionally not updated by gauss projection; their
/// flux is left alone, consistent with
/// `engine/src/poisson_solvers.cpp::gauss_project_cpu`.
inline ConvergenceReport gauss_project_converged(
    RenderBridge& rb, double tol = 1e-6, int max_cycles = 500)
{
    ConvergenceReport rpt;
    rpt.tolerance = tol;

    // Snapshot toggles and disable everything except gauss_projection.
    const TermToggles saved = rb.toggles;
    rb.toggles.wave_propagation  = false;
    rb.toggles.coupling          = false;
    rb.toggles.damping           = false;
    rb.toggles.selective_damping = false;
    rb.toggles.genesis           = false;
    rb.toggles.forces            = false;
    rb.toggles.gravity           = false;
    rb.toggles.poisson_coulomb   = false;
    rb.toggles.movement          = false;
    rb.toggles.lorentz_force     = false;
    rb.toggles.larmor_radiation  = false;
    rb.toggles.dual_substrate    = false;
    rb.toggles.color_forces      = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.strong_force      = false;
    rb.toggles.triad_binding     = false;
    rb.toggles.pair_production   = false;
    rb.toggles.exchange_force    = false;
    rb.toggles.latency_field     = false;
    rb.toggles.emergent_forces   = false;
    rb.toggles.gauss_projection  = true;

    double max_abs = 0.0, rms = 0.0;
    _measure_vacuum_gauss_residual(rb, max_abs, rms);
    rpt.initial_max_residual = max_abs;

    for (int c = 0; c < max_cycles; ++c) {
        rb.tick();
        ++rpt.cycles;
        _measure_vacuum_gauss_residual(rb, max_abs, rms);
        if (max_abs < tol) {
            rpt.converged = true;
            break;
        }
    }

    rpt.final_max_residual = max_abs;
    rpt.final_rms_residual = rms;

    // Restore toggles.
    rb.toggles = saved;
    return rpt;
}

}  // namespace eft
}  // namespace ftd
