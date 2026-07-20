/**
 * @file campaign_genesis_energy_ledger.cpp
 * @brief Does a REAL (genesis-created) manifested charge lock the SAME
 *        constraint self-energy W_SC(L) that a SYNTHETIC unit charge does?
 *
 * Implements the measurement registered by
 *   docs/theory/10_eft_program/preregistrations/PREREG_GENESIS_ENERGY_LEDGER_v1.md
 * (tag `preregister-genesis-energy-ledger-v1`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec and locked in the same commit; it makes no claim,
 * promotes no tag, and alters no engine physics.
 *
 * ── The question ─────────────────────────────────────────────────────────
 * DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md proposes M_REST = W_SC: the rest
 * mass IS the energy the Gauss constraint forces a manifested voxel to hold.
 * The self-energy pinning campaign (FTD-0388) measured this for a SYNTHETIC
 * charge — injected directly as state=+1, J=0 everywhere else, then relaxed
 * under the Gauss projector alone. This campaign asks the sharper question:
 * does a charge born from REAL genesis dynamics (wave propagation, coupling,
 * a stochastic threshold-crossing nucleation event) relax to the SAME fixed
 * point, or does it carry something different?
 *
 * ── The confound this design is built around (prereg §2) ────────────────
 * gauss_project_cpu's correction is `voxels[i].flux -= grad_phi` — a PURE
 * GRADIENT, hence curl-free by construction (verified by reading
 * poisson_solvers.cpp before locking this design). Repeated projection can
 * only ever correct the LONGITUDINAL (divergence) part of J; any TRANSVERSE
 * (divergence-free) content present when projection starts is invariant
 * under it, forever. The synthetic-charge campaign started from J=0 (zero
 * transverse content by construction) and so trivially stayed there. A
 * genesis-born charge's surrounding field is whatever real dynamics left
 * behind — it is NOT guaranteed to be transverse-free. This campaign is
 * designed to keep that contamination small (a curl-free RADIAL seed pulse,
 * a particle born at rest, freezing at the earliest possible tick) and,
 * critically, to MEASURE whether it is small rather than assume it:
 * comparing an early-freeze arm against a late-freeze arm directly exposes
 * any growing transverse-debris contribution.
 *
 * ── Design (prereg §3) ───────────────────────────────────────────────────
 *  Arm S  (sanity / instrument validation): EXACT reproduction of the
 *         FTD-0388 selfenergy-pinning GF-A protocol — synthetic +1 charge,
 *         J=0, gauss_projection-only relaxation to the residual-1e-8 fixed
 *         point. Must reproduce the frozen closed-form prediction
 *         E_half(L=17) = 0.478917129 (P1/W_SC family) — this is the
 *         VALIDITY GATE; if it fails, the harness has a bug and G is
 *         uninterpretable.
 *  Arm G-early: a curl-free radial flux pulse is seeded off a generic
 *         (non-lattice-symmetric) virtual center so exactly one site
 *         crosses K_GENESIS; realistic dynamics (wave+coupling+gauss+
 *         genesis+damping) run tick-by-tick until that site manifests;
 *         frozen at that EXACT tick (dynamics toggles OFF, gauss_projection
 *         alone continues) and relaxed to the same residual-1e-8 fixed
 *         point. E_half is the observable of record.
 *  Arm G-late: identical to G-early but dynamics continue kLateExtraTicks
 *         further (matter allowed to radiate/settle longer) before freezing
 *         and relaxing. G-late − G-early is the debris-growth diagnostic.
 *
 * Energy observable, reused VERBATIM from
 * scripts/proofs/prereg_selfenergy_pinning_predictions.py's canonical
 * "tracker convention": E_half = (1/2) * Sum_over_all_voxels |J|^2.
 *
 * OUTPUT: CSV to stdout (GATE rows for validity, ROW rows for the
 * convergence trace, SUMMARY rows for final E_half per arm). stderr carries
 * progress. NO verdict is computed here; the prereg's outcome map is
 * applied afterward by the analyst.
 *
 * Deterministic; CPU-forced (force_cpu()); no RNG beyond the engine's own
 * seeded genesis draw.
 *
 * Epistemic status: [PRE-REGISTERED MEASUREMENT INSTRUMENT].
 */

#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

// ── Pre-registered fixed parameters (prereg §3) ────────────────────────────
constexpr int    kL             = 17;     // matches the FTD-0388 pinning L-triple
constexpr double kFrozenEhalf17 = 0.478917129;   // P1/W_SC family, exact closed-form
constexpr double kFrozenEhalf17_P2 = 0.151842301; // matched-18pt alternative (discriminator)
constexpr double kResidualGate  = 1e-8;
constexpr int    kResidualCap   = 5000;   // generous vs the L=17 selfenergy cap (1380)
constexpr int    kSorItersRelax = 6;      // one production-default sweep per applied projection
constexpr int    kMaxWaitTicks  = 200;    // Phase-A genesis-wait ceiling
constexpr int    kLateExtraTicks = 5;     // G-late's extra dynamics window past manifestation

// Curl-free radial seed (prereg §3): a generic (non-lattice-symmetric) virtual
// center so exactly one lattice site is nearest and gets the peak amplitude —
// avoids the r=0 direction singularity and avoids symmetric multi-site ties.
constexpr double kSeedOffX = 0.31, kSeedOffY = 0.17, kSeedOffZ = 0.07;
constexpr double kSeedAmp   = 3.0;
constexpr double kSeedSigma = 0.45;
constexpr double kSeedCutR  = 4.0;

double e_half(const ftd::RenderBridge& rb) {
    double e = 0.0;
    for (const auto& v : rb.voxels()) e += v.flux.mag2();
    return 0.5 * e;
}

int manifested_count(const ftd::RenderBridge& rb) {
    int n = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0) ++n;
    return n;
}

// Relax to the Gauss-projector fixed point on a FRESH bridge that only ever
// carries the copied state+flux — NOT the source bridge in place.
//
// Why fresh, not in-place (found during pre-lock instrument validation, this
// file): phase_write's leapfrog (`wave_vel += delta_j_; flux += wave_vel`) is
// NOT gated by wave_propagation — only phase_read (which COMPUTES delta_j_)
// is. Toggling wave_propagation off after Phase A's real dynamics leaves
// delta_j_ at its last-computed, generally-nonzero value, and every
// subsequent projector-only tick re-adds that STALE value into wave_vel,
// which compounds into flux — an unbounded runaway (observed: e_half
// 6 -> 1e6 within ~50 applications). This exact gotcha is documented in
// test_gauss_law_fidelity.cpp's own freeze arm ("copy the flux onto a fresh
// bridge instead" of an in-place toggle flip); the FTD-0388 selfenergy
// campaigns avoid it entirely by using a fresh bridge from tick 0. Mirrored
// here: construct fresh, copy state (for the projector's rho source) and
// flux (the field being relaxed), leave wave_vel at its zero-initialized
// default, and never enable wave_propagation on this bridge — delta_j_
// then stays exactly zero for the bridge's entire lifetime, matching Arm S.
int relax_to_fixed_point(const ftd::RenderBridge& src, ftd::RenderBridge& out, const char* tag) {
    out.toggles.disable_all();
    out.toggles.gauss_projection = true;
    out.set_sor_iterations(kSorItersRelax);
    const int L = src.lattice().size();
    const auto& sv = src.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = (x * L + y) * L + z;
        if (sv[i].state != 0)
            out.inject_particle(x, y, z, sv[i].state, ftd::Vec3(0, 0, 0));
        if (sv[i].flux.mag2() > 0.0)
            out.inject_flux_add(x, y, z, sv[i].flux);
    }
    double prev = e_half(out);
    int apps = 0;
    for (; apps < kResidualCap; ++apps) {
        out.tick();
        const double cur = e_half(out);
        if (apps < 5 || apps % 500 == 0 || !std::isfinite(cur) || cur > 1e6) {
            std::printf("ROW,%s,%d,%.12f\n", tag, apps, cur);
        }
        if (!std::isfinite(cur) || cur > 1e12) {
            std::printf("GATE,%s,diverged_at_app,%d\n", tag, apps);
            prev = cur;
            ++apps;
            break;
        }
        if (std::fabs(cur - prev) < kResidualGate) { prev = cur; ++apps; break; }
        prev = cur;
    }
    std::printf("GATE,%s,relax_applications,%d\n", tag, apps);
    std::printf("GATE,%s,relax_converged,%d\n", tag, (apps < kResidualCap) ? 1 : 0);
    return apps;
}

void configure_dynamics(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation  = true;
    rb.toggles.coupling          = true;
    rb.toggles.gauss_projection  = true;
    rb.toggles.genesis           = true;
    rb.toggles.damping           = true;
    rb.toggles.selective_damping = true;
    // movement/forces/gravity/poisson_coulomb/lorentz_force/weak_transmutation
    // stay OFF: minimal toggle set, no channel that could move or transmute
    // the newborn particle during the tiny birth window (prereg §3).
}

// Seed the curl-free radial pulse; returns the count of sites with |J| >
// K_GENESIS immediately after injection (the pre-tick validity gate).
int seed_radial_pulse(ftd::RenderBridge& rb) {
    const double cx = (kL - 1) / 2.0 + kSeedOffX;
    const double cy = (kL - 1) / 2.0 + kSeedOffY;
    const double cz = (kL - 1) / 2.0 + kSeedOffZ;
    const int lo = 0, hi = kL - 1;
    const int ilo = std::max(lo, static_cast<int>(cx - kSeedCutR));
    const int ihi = std::min(hi, static_cast<int>(cx + kSeedCutR) + 1);
    for (int x = ilo; x <= ihi; ++x)
    for (int y = std::max(lo, static_cast<int>(cy - kSeedCutR));
             y <= std::min(hi, static_cast<int>(cy + kSeedCutR) + 1); ++y)
    for (int z = std::max(lo, static_cast<int>(cz - kSeedCutR));
             z <= std::min(hi, static_cast<int>(cz + kSeedCutR) + 1); ++z) {
        const double dx = x - cx, dy = y - cy, dz = z - cz;
        const double r2 = dx*dx + dy*dy + dz*dz;
        if (r2 > kSeedCutR * kSeedCutR) continue;
        const double r = std::sqrt(r2);
        if (r < 1e-9) continue;  // never sampled given the generic offset; guard anyway
        const double amp = kSeedAmp * std::exp(-r2 / (2.0 * kSeedSigma * kSeedSigma));
        if (amp < 1e-9) continue;
        rb.inject_flux_add(x, y, z, ftd::Vec3(amp * dx / r, amp * dy / r, amp * dz / r));
    }
    int n_above = 0;
    for (const auto& v : rb.voxels()) if (v.flux.mag() > ftd::K_GENESIS) ++n_above;
    return n_above;
}

}  // namespace

int main() {
    std::printf("# campaign_genesis_energy_ledger — PREREG_GENESIS_ENERGY_LEDGER_v1\n");
    std::printf("# L=%d  K_GENESIS=%.10f  K_MANIFEST=%.16f  W_SC(L=17) frozen=%.9f (P2 alt=%.9f)\n",
                kL, ftd::K_GENESIS, ftd::K_MANIFEST, kFrozenEhalf17, kFrozenEhalf17_P2);

    // ── Arm S: synthetic-charge sanity/validation (FTD-0388 GF-A reproduction) ──
    {
        ftd::RenderBridge seed(kL);
        seed.force_cpu();
        const int mid = kL / 2;
        seed.inject_particle(mid, mid, mid, +1, ftd::Vec3(0, 0, 0));
        ftd::RenderBridge rb(kL);
        rb.force_cpu();
        const int apps = relax_to_fixed_point(seed, rb, "S");
        const double e = e_half(rb);
        std::printf("SUMMARY,S,e_half,%.12f,applications,%d\n", e, apps);
        std::fprintf(stderr, "[S] e_half = %.12f (frozen pred %.9f, diff %.6e)\n",
                     e, kFrozenEhalf17, e - kFrozenEhalf17);
    }

    // ── Arms G-early / G-late: real genesis-born charge ─────────────────────
    for (int arm = 0; arm < 2; ++arm) {
        const char* tag = (arm == 0) ? "G-early" : "G-late";
        ftd::RenderBridge rb(kL);
        rb.force_cpu();
        configure_dynamics(rb);

        const int n_above = seed_radial_pulse(rb);
        std::printf("GATE,%s,sites_above_threshold_preseed,%d\n", tag, n_above);
        std::printf("GATE,%s,e_half_raw_seed,%.12f\n", tag, e_half(rb));
        std::fprintf(stderr, "[%s] sites above K_GENESIS after seed: %d (need exactly 1); "
                     "raw seed e_half = %.6f\n", tag, n_above, e_half(rb));

        int fire_tick = -1;
        for (int t = 0; t <= kMaxWaitTicks; ++t) {
            if (manifested_count(rb) >= 1) { fire_tick = t; break; }
            rb.tick();
        }
        std::printf("GATE,%s,fire_tick,%d\n", tag, fire_tick);
        std::printf("GATE,%s,manifested_at_fire,%d\n", tag, manifested_count(rb));

        if (fire_tick < 0) {
            std::printf("GATE,%s,VOID_no_manifestation,1\n", tag);
            std::fprintf(stderr, "[%s] VOID: no manifestation within %d ticks\n",
                         tag, kMaxWaitTicks);
            continue;
        }

        if (arm == 1) {
            for (int i = 0; i < kLateExtraTicks; ++i) rb.tick();
            std::printf("GATE,%s,extra_dynamics_ticks,%d\n", tag, kLateExtraTicks);
            std::printf("GATE,%s,manifested_after_extra,%d\n", tag, manifested_count(rb));
        }

        const double e_pre_relax = e_half(rb);
        ftd::RenderBridge relaxed(kL);
        relaxed.force_cpu();
        const int apps = relax_to_fixed_point(rb, relaxed, tag);
        const double e = e_half(relaxed);
        std::printf("SUMMARY,%s,e_half,%.12f,applications,%d,e_half_prerelax,%.12f,"
                    "final_manifested,%d\n",
                    tag, e, apps, e_pre_relax, manifested_count(relaxed));
        std::fprintf(stderr, "[%s] e_half = %.12f (frozen pred %.9f, diff %.6e)\n",
                     tag, e, kFrozenEhalf17, e - kFrozenEhalf17);
    }

    std::printf("# done — verdict applied against PREREG §5 by the analyst, not here\n");
    return 0;
}
