#pragma once
/**
 * @file engine/include/ftd/scale_context.h
 * @purpose Read-only "scale-context readout admissibility gate" (C_scale).
 *          Decides whether an engine cloud is eligible for *public physical
 *          readout* (the Koopman α estimator) by validating that it is
 *          scale-separated from both the lattice scale a and the box scale L,
 *          self-confined, and stationary.
 * @canonical docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md
 *            (chain U → B_Ω → C_scale → K_red → W_U → α).
 * @consumers engine/tests/dump_koopman_trajectory.cpp,
 *            engine/tests/test_scale_context.cpp.
 *
 * INVARIANT — α-BLINDNESS BY CONTRACT:
 *   This module MUST NEVER #include, reference, or depend on ALPHA / ALPHA_EFT,
 *   the Koopman eigenvalue, x_+, 1/137, or any QED/coupling observable. Its
 *   ONLY inputs are lattice geometry, the flux field |J|², and the
 *   observation-only genesis/evaporation counters. It accepts or rejects a
 *   cloud on shape and self-consistency BEFORE any α extraction runs, so it
 *   structurally cannot launder an α target. Every threshold below is an
 *   [IMPOSED engineering default], not a theorem value.
 *
 * READ-ONLY / GOLDEN-NEUTRAL:
 *   measure_scale_context()/ScaleContextTracker take `const RenderBridge&` and
 *   use only its const accessors. They are never called from tick(), so the
 *   golden hash (test_render_bridge_golden) is preserved by construction.
 *
 * NOTE — distinct from `ScaleBridge` (engine/include/ftd/scale.h): that type
 * coarsens/refines state BETWEEN scale levels; this module diagnoses a cloud
 * AT a single scale. No relationship beyond the shared word "scale".
 */

#include <cmath>
#include <cstddef>
#include <deque>

#include "ftd/render_bridge_diagnostics.h"  // ScaleContextDiagnostics + enums
#include "ftd/voxel.h"                       // Vec3

namespace ftd {

class RenderBridge;  // forward decl — only const accessors are used in the .cpp

// ---------------------------------------------------------------------------
// Configuration — every field is an [IMPOSED engineering default]. The defaults
// are mutually self-consistent (e.g. kappa_min < zeta_max·L for any usable L),
// then calibrated against a known-good reference cloud (see the spec's
// threshold table). NONE is derived from or tuned to hit α⁻¹ = 137.036.
// ---------------------------------------------------------------------------
struct ScaleContextConfig {
    // ---- support definition (flux-energy primary) ----
    double energy_threshold = 1e-4;   // [IMPOSED] |J|² floor defining support (≈|J| 0.01)
    bool   union_with_state = false;  // [IMPOSED] also include voxels with state≠0

    // ---- golden scale window (κ = R_eff/a, ζ = R_eff/L) ----
    double kappa_min = 3.0;           // [IMPOSED] R_eff must exceed ~3 voxels (UV decoupling)
    double zeta_max  = 0.25;          // [IMPOSED] R_eff must stay below L/4 (IR decoupling)

    // ---- volume fraction (f_active) ----
    double f_active_max      = 0.10;  // [IMPOSED] occupancy ceiling (0 < f ≪ 1)
    double f_active_evap_min = 1e-5;  // [IMPOSED] below this ⇒ Evaporating

    // ---- shell dominance (β = δ_shell/R_eff) ----
    double beta_max = 0.60;           // [IMPOSED] surface-to-volume ceiling

    // ---- self-confinement (flux balance at R_eff) ----
    double phi_balance_tol = 0.15;    // [IMPOSED] |Φout−Φret|/(Φout+Φret) ceiling
    double dPhi_dR_max     = 0.0;     // [IMPOSED] slope must be strictly < this (i.e. < 0)

    // ---- stationarity gates ----
    double dR_dt_tol  = 0.02;         // [IMPOSED] |dR_eff/dt| ceiling (voxels/tick)
    double dJ2_dt_tol = 0.02;         // [IMPOSED] relative |d⟨J²⟩/dt| ceiling
    double B_t_tol    = 1.0;          // [IMPOSED] |⟨B(t)⟩| ceiling (events/tick, windowed)

    // ---- rolling estimation ----
    int    window   = 64;             // [IMPOSED] tracker window length (ticks)
    double tau_bath = 50.0;           // [IMPOSED] bath reference for Θ = τ_cloud/τ_bath

    // ---- radial shell partition (Φ balance + quantiles) ----
    int    n_shells    = 64;          // [IMPOSED] # radial bins (deep enough for large L half-box)
    double shell_width = 1.0;         // [IMPOSED] voxels per shell

    // ---- gating mode ----
    bool   gate_active = false;       // [IMPOSED] false ⇒ status forced to DiagnosticOnly
                                      //   (observe-only: never blocks an existing run)
};

// ---------------------------------------------------------------------------
// Minimum-image displacement under periodic boundary conditions. Maps each
// component of (p − center) into (−L/2, L/2]. The Lattice class lacks this;
// it owns integer wrapping (wrap()) but not continuous min-image distance.
// `center` is continuous (the circular-mean center from measure_scale_context).
// ---------------------------------------------------------------------------
inline Vec3 min_image_disp(const Vec3& p, const Vec3& center, int L) {
    const double Ld = static_cast<double>(L);
    auto wrap1 = [Ld](double d) {
        return d - Ld * std::floor(d / Ld + 0.5);
    };
    return Vec3(wrap1(p.x - center.x), wrap1(p.y - center.y), wrap1(p.z - center.z));
}

// ---------------------------------------------------------------------------
// Single-shot, stateless measurement: geometry + flux + boundary susceptibility
// + classification. Rolling fields (dR_dt, dJ2_dt, tau_cloud, Theta) are left 0
// and `stationary` is trivially true, so the verdict reflects geometry +
// self-confinement only. Use ScaleContextTracker for the rolling/windowed gate.
// ---------------------------------------------------------------------------
ScaleContextDiagnostics measure_scale_context(const RenderBridge& rb,
                                              const ScaleContextConfig& cfg);

// ---------------------------------------------------------------------------
// Stateful tracker: ingest one tick at a time, maintain a rolling window, and
// emit the fully-populated diagnostics (including dR_dt / dJ2_dt / tau_cloud /
// Theta and the windowed stationarity verdict). Read-only on the bridge.
// ---------------------------------------------------------------------------
class ScaleContextTracker {
public:
    explicit ScaleContextTracker(ScaleContextConfig cfg = {}) : cfg_(cfg) {}

    // Measure this tick, push to the window, fill rolling fields, re-classify.
    ScaleContextDiagnostics ingest(const RenderBridge& rb);

    const ScaleContextConfig& config() const { return cfg_; }
    bool warmed_up() const { return reff_.size() >= 4; }
    const ScaleContextDiagnostics& latest() const { return latest_; }

private:
    ScaleContextConfig cfg_;
    // x-axis for the rolling slopes is an internal monotonic ingest counter
    // (one ingest == one tick in the dumper), NOT rb.current_tick(): a caller
    // may re-measure a frozen state without ticking, and slopes must still be
    // well defined (distinct x values).
    long long n_ingested_ = 0;
    std::deque<double> idxv_;    // ingest indices (slope x-axis)
    std::deque<double> reff_;    // R_eff samples
    std::deque<double> j2_;      // J2_total samples
    std::deque<double> bt_;      // B(t) samples
    ScaleContextDiagnostics latest_{};
};

}  // namespace ftd
