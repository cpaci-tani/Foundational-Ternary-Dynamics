/**
 * Test: ScaleContext readout admissibility gate (C_scale)
 *
 * Verifies the read-only scale-context diagnostics layer that decides whether
 * an engine cloud is eligible for public physical readout. See
 * docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md and
 * engine/include/ftd/scale_context.h.
 *
 * Fixtures are constructed by writing voxel flux directly (no engine ticks),
 * using ONLY axis-aligned voxels at integer radii about the box center L/2 so
 * that the PBC circular-mean center lands exactly on L/2 and every radial unit
 * vector r̂ is an exact ±axis — making Φ_outward / Φ_return deterministic.
 *
 * Coverage:
 *   1. Empty            -> Evaporating       / RejectedScaleContext
 *   2. Tight 2-voxel    -> UVLocked          / RejectedScaleContext   (κ < κ_min)
 *   3. Box-filling flux -> Percolating       / RejectedScaleContext   (flux-energy
 *                          support catches a saturated field with state≡0)
 *   4. Thick bimodal    -> ShellDominated    / RejectedScaleContext   (β > β_max)
 *   5. Pure-outward     -> Φout>0, Φret=0    / RejectedSelfConfinement
 *   6. Pure-inward      -> Φret>0, Φout=0    / RejectedSelfConfinement
 *   7. Confining cloud  -> BoundedAdmissible / Admissible (dΦ/dR<0, fixed point);
 *                          and DiagnosticOnly when the gate is not armed
 *   8a. Tracker, 1 ingest (cold)   -> RejectedNonStationary
 *   8b. Tracker, static, warmed    -> Admissible
 *   8c. Tracker, growing R_eff     -> RejectedNonStationary (|dR/dt| > tol)
 */
#include <cmath>
#include <iostream>
#include "ftd/render_bridge.h"
#include "ftd/scale_context.h"

using ftd::Vec3;
using ftd::ScaleRegime;
using ftd::ReadoutStatus;

static int failures = 0;

static void check(const char* name, bool condition) {
    if (condition) std::cout << "  PASS  " << name << "\n";
    else { std::cout << "  FAIL  " << name << "\n"; ++failures; }
}

// ---- fixture helpers (direct voxel writes; no engine dynamics) -------------

static void clear_flux(ftd::RenderBridge& rb) {
    auto& vox = rb.voxels();
    for (auto& v : vox) v.flux = Vec3();
}

static void set_flux(ftd::RenderBridge& rb, int x, int y, int z, const Vec3& f) {
    rb.voxels()[rb.lattice().index(x, y, z)].flux = f;
}

// Tangential (perpendicular-to-radial) unit vector for an exact ±axis r̂.
static Vec3 tangent_for_axis(const Vec3& rhat) {
    if (std::abs(rhat.x) > 0.5) return Vec3(0, 1, 0);
    return Vec3(1, 0, 0);
}

// Stamp the 6 axis voxels at radius R about center c, flux = mag * r̂ (outward
// if mag>0, inward if mag<0). If `tangential`, flux is perpendicular to r̂.
static void stamp_axis_shell(ftd::RenderBridge& rb, int c, int R, double mag,
                             bool tangential = false) {
    const int off[6][3] = {{ R, 0, 0}, {-R, 0, 0}, {0,  R, 0},
                           {0, -R, 0}, {0, 0,  R}, {0, 0, -R}};
    for (auto& o : off) {
        Vec3 rhat(o[0] ? (o[0] > 0 ? 1.0 : -1.0) : 0.0,
                  o[1] ? (o[1] > 0 ? 1.0 : -1.0) : 0.0,
                  o[2] ? (o[2] > 0 ? 1.0 : -1.0) : 0.0);
        Vec3 f = tangential ? tangent_for_axis(rhat) * std::abs(mag) : rhat * mag;
        set_flux(rb, c + o[0], c + o[1], c + o[2], f);
    }
}

// A self-confining cloud at base radius R about center c:
//   inner shells (R-2,R-1) outward, shell R tangential (sets R_eff≈R), outer
//   shells (R+1,R+2) inward  -> net-outward inside, net-inward outside -> dΦ/dR<0.
static void build_confining(ftd::RenderBridge& rb, int c, int R) {
    clear_flux(rb);
    stamp_axis_shell(rb, c, R - 2, +0.5);
    stamp_axis_shell(rb, c, R - 1, +0.25);
    stamp_axis_shell(rb, c, R,      1.0, /*tangential=*/true);
    stamp_axis_shell(rb, c, R + 1, -0.25);
    stamp_axis_shell(rb, c, R + 2, -0.5);
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  Test: ScaleContext readout admissibility gate\n";
    std::cout << "================================================================\n";

    ftd::ScaleContextConfig cfg;   // [IMPOSED] defaults
    cfg.gate_active = true;        // arm the gate so statuses are exercised

    // --- 1. Empty -> Evaporating ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        auto d = ftd::measure_scale_context(rb, cfg);
        check("1: empty regime == Evaporating", d.regime == ScaleRegime::Evaporating);
        check("1: empty status == RejectedScaleContext",
              d.status == ReadoutStatus::RejectedScaleContext);
        check("1: empty support == 0", d.support_count == 0);
    }

    // --- 2. Tight 2-voxel blob -> UVLocked (kappa < kappa_min) ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        set_flux(rb, 12, 12, 12, Vec3(5, 0, 0));
        set_flux(rb, 13, 12, 12, Vec3(5, 0, 0));
        auto d = ftd::measure_scale_context(rb, cfg);
        check("2: kappa < kappa_min", d.kappa < cfg.kappa_min);
        check("2: regime == UVLocked", d.regime == ScaleRegime::UVLocked);
        check("2: status == RejectedScaleContext",
              d.status == ReadoutStatus::RejectedScaleContext);
    }

    // --- 3. Box-filling flux, state == 0 everywhere -> Percolating ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        { auto& vox = rb.voxels(); for (auto& v : vox) v.flux = Vec3(0.5, 0, 0); }
        auto d = ftd::measure_scale_context(rb, cfg);
        check("3: flux-energy support == all voxels",
              d.support_count == rb.lattice().total_sites());
        check("3: active_fraction > f_active_max", d.active_fraction > cfg.f_active_max);
        check("3: center not well-defined (box-filling)", !d.center_well_defined);
        check("3: regime == Percolating", d.regime == ScaleRegime::Percolating);
        check("3: status == RejectedScaleContext",
              d.status == ReadoutStatus::RejectedScaleContext);
    }

    // --- 4. Bimodal thick shell (inner r=1 + outer r=10) -> ShellDominated ---
    {
        ftd::RenderBridge rb(32); rb.force_cpu();
        stamp_axis_shell(rb, 16, 1, 1.0);    // inner
        stamp_axis_shell(rb, 16, 10, 1.0);   // outer
        auto d = ftd::measure_scale_context(rb, cfg);
        check("4: beta > beta_max", d.beta > cfg.beta_max);
        check("4: kappa >= kappa_min (passes UV)", d.kappa >= cfg.kappa_min);
        check("4: regime == ShellDominated", d.regime == ScaleRegime::ShellDominated);
        check("4: status == RejectedScaleContext",
              d.status == ReadoutStatus::RejectedScaleContext);
    }

    // --- 5. Pure-outward shell -> Phi_outward>0, Phi_return=0 ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        stamp_axis_shell(rb, 12, 5, +1.0);   // outward
        auto d = ftd::measure_scale_context(rb, cfg);
        check("5: phi_outward > 0", d.phi_outward > 0.0);
        check("5: phi_return ~= 0", d.phi_return < 1e-9);
        check("5: phi_balance > 0", d.phi_balance > 0.0);
        check("5: not a confinement fixed point", !d.confinement_fixed_point);
        check("5: status == RejectedSelfConfinement",
              d.status == ReadoutStatus::RejectedSelfConfinement);
    }

    // --- 6. Pure-inward shell -> Phi_return>0, Phi_outward=0 ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        stamp_axis_shell(rb, 12, 5, -1.0);   // inward
        auto d = ftd::measure_scale_context(rb, cfg);
        check("6: phi_return > 0", d.phi_return > 0.0);
        check("6: phi_outward ~= 0", d.phi_outward < 1e-9);
        check("6: phi_balance < 0", d.phi_balance < 0.0);
    }

    // --- 7. Confining cloud -> BoundedAdmissible / Admissible ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        build_confining(rb, 12, 5);
        auto d = ftd::measure_scale_context(rb, cfg);
        check("7: confinement fixed point", d.confinement_fixed_point);
        check("7: dPhi_dR < 0", d.dPhi_dR < 0.0);
        check("7: beta <= beta_max", d.beta <= cfg.beta_max);
        check("7: regime == BoundedAdmissible", d.regime == ScaleRegime::BoundedAdmissible);
        check("7: status == Admissible (gate armed)", d.status == ReadoutStatus::Admissible);

        // Observe-only mode: same cloud, gate not armed -> DiagnosticOnly.
        ftd::ScaleContextConfig cfg_off = cfg; cfg_off.gate_active = false;
        auto d_off = ftd::measure_scale_context(rb, cfg_off);
        check("7: gate off -> regime still BoundedAdmissible",
              d_off.regime == ScaleRegime::BoundedAdmissible);
        check("7: gate off -> status == DiagnosticOnly",
              d_off.status == ReadoutStatus::DiagnosticOnly);
    }

    // --- 8a. Tracker, single (cold) ingest -> RejectedNonStationary ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        build_confining(rb, 12, 5);
        ftd::ScaleContextTracker tr(cfg);
        auto d = tr.ingest(rb);
        check("8a: cold tracker not warmed", !tr.warmed_up());
        check("8a: cold status == RejectedNonStationary",
              d.status == ReadoutStatus::RejectedNonStationary);
    }

    // --- 8b. Tracker, static cloud, warmed -> Admissible ---
    {
        ftd::RenderBridge rb(24); rb.force_cpu();
        build_confining(rb, 12, 5);
        ftd::ScaleContextTracker tr(cfg);
        ftd::ScaleContextDiagnostics d{};
        for (int i = 0; i < cfg.window + 5; ++i) d = tr.ingest(rb);
        check("8b: warmed tracker", tr.warmed_up());
        check("8b: |dR_dt| ~ 0", std::abs(d.dR_dt) <= cfg.dR_dt_tol);
        check("8b: status == Admissible", d.status == ReadoutStatus::Admissible);
    }

    // --- 8c. Tracker, growing R_eff -> RejectedNonStationary (|dR/dt| > tol) ---
    {
        ftd::RenderBridge rb(64); rb.force_cpu();
        ftd::ScaleContextTracker tr(cfg);
        ftd::ScaleContextDiagnostics d{};
        // Grow the confining base radius slowly: R = 10 .. 15 over the window,
        // staying inside the golden window (zeta < 0.25 at L=64 needs R < 16).
        for (int i = 0; i < cfg.window + 4; ++i) {
            int R = 10 + i / 13;   // steps 10,10,...,11,...,15
            build_confining(rb, 32, R);
            d = tr.ingest(rb);
        }
        check("8c: grown |dR_dt| > tol", std::abs(d.dR_dt) > cfg.dR_dt_tol);
        check("8c: status == RejectedNonStationary",
              d.status == ReadoutStatus::RejectedNonStationary);
    }

    std::cout << "----------------------------------------------------------------\n";
    if (failures == 0) std::cout << "  ALL SCALE-CONTEXT TESTS PASSED\n";
    else std::cout << "  " << failures << " FAILURE(S)\n";
    return failures == 0 ? 0 : 1;
}
