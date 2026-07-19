// ============================================================================
// test_gauss_law_fidelity.cpp  (engine-fidelity investigation, 2026-07-16)
// ----------------------------------------------------------------------------
// MEASURES the realized fraction of the Gauss constraint  div(J) = s  AT A
// MANIFESTED CHARGE SITE, and isolates which mechanism limits it. This is a
// documentation-of-reality test in the test_conservation_profile.cpp mold:
// its primary output is the printed live measurement; assertions pin only
// robust structural facts, not quality targets.
//
// ── Why the charge site is the interesting site ─────────────────────────────
// The engine's own residual metric (EnergyAudit.gauss_violation,
// diagnostics_compute.cpp:89-95) deliberately EXCLUDES manifested sites, on
// the rationale that the projection's correction pass skips them
// (poisson_solvers.cpp:198). But the CENTRAL-DIFFERENCE divergence at a site
// reads only the six FACE-NEIGHBOR fluxes — all vacuum for an isolated
// charge, all corrected by the projection — so div(J) at the charge site IS
// controllable by the projector and IS the axiom's non-trivial content
// (div J = s with s ≠ 0). No existing diagnostic watches it. This test does.
//
// Exact-enforcement reference: with central differences, div(J) = 1 at an
// isolated unit charge requires outward radial flux |J| = 1/3 at each face
// neighbor (div = 3 · J_rad(r=1) = 1). The engine's own solve targets
// rho = coulomb_charge_coupling · (s − mean_charge), i.e. 1 − 1/N here.
//
// ── Candidate limiting mechanisms probed (one toggle at a time) ─────────────
//  (a) correction skips manifested sites   → exact_dual_gauss=true
//      (irrelevant for div AT the site — central-diff div doesn't read the
//       site's own J — but changes div at r=1 neighbors)
//  (b) stencil mismatch: the solve uses the 18-pt Laplacian (sor_sweep_18pt)
//      while div is 6-pt central; the correction composition
//      div_c ∘ grad_c is the ±2h Laplacian, matching NEITHER. See
//      constants.h:328-353 HONEST NOTE + matched_poisson.h:7-19.
//      → matched-stencil CG projection (ftd::eft::matched_gauss_project)
//  (c) selective damping drains near-particle flux each tick between
//      projections (phase_write runs BEFORE gauss_project in the tick)
//      → damping=false (+ selective_damping=false for validity)
//  (d) 6 SOR iterations/tick (constants.h:354) → set_sor_iterations(100)
//  (e) wave propagation + coupling actively erase the projected monopole:
//      a Coulomb-profile J has ∇²J = ∇(div J) = ∇ρ ≠ 0, so the leapfrog
//      pushes against it every tick; the coupling source g_c·∇s (G_C·grad s,
//      phase_read.cpp:122-124) points TOWARD a positive charge, i.e. it
//      sources div(J) < 0 at a +1 site — opposite in sign to the Gauss
//      target. → wave_propagation=false + coupling=false, and the converse
//      probe gauss_projection=false (what the dynamics do unconstrained).
//
// ── Experiments ─────────────────────────────────────────────────────────────
//  GF-A  Projection operator in ISOLATION (no dynamics): single +1 charge,
//        J=0, only gauss_projection on; each tick = one more projection
//        application (warm-started phi, exactly as in production).
//        A1 sor=6 · A2 sor=100 · A3 sor=6+exact_dual_gauss ·
//        A4 sor=100+exact_dual_gauss · A5 matched-stencil CG (repeated).
//  GF-B  LIVE shipping-default dynamics (enable_all, genesis/movement off —
//        the GP-KCOMP-SHELL configuration of DERIV_KCOMP_VOLUMETRIC_SHELL.md):
//        B1 defaults · B2 +exact_dual_gauss · B3 damping off ·
//        B4 sor=100 · B5 wave+coupling off · B6 gauss OFF (unconstrained
//        dynamics) · B7 exact+sor=100 · B8 matched CG in place of the
//        production projector. B1 ends with a FREEZE probe: dynamics off,
//        gauss only, 200 further projections from the settled live field.
//  GF-C  L=33 size check (B1 replica) + wavepacket-IC tie-in to the
//        GP-KCOMP-SHELL J(r=1) = 9.898e-3 measurement (128^3 GPU).
//
// All CPU (force_cpu), deterministic, golden-neutral (no engine sources are
// modified; production defaults untouched). GPU/FFT counterpart:
// test_gauss_law_fidelity_gpu.cpp (WSL2 CUDA build).
//
// ── Measured (2026-07-16, L=17 CPU, seed 1234; f = div_c(site)/target) ──────
//   A1 sor=6           one-shot f = +0.400 ; 200 applications -> +0.9996
//   A2 sor=100         one-shot f = +0.416 ; 200 applications -> +0.9996
//   A3/A4 exact_dual   IDENTICAL at the site (skip only affects the site's
//                      own J, which central-difference div never reads)
//   A5 matched CG      central-div f -> +0.517 ; its OWN (backward-div) Ward
//                      identity at the site -> +0.9998 ; vacuum residual
//                      == mean_charge = 1/N exactly (rho=s, no mean subtract)
//   B1 live defaults   tail f = -0.095  (WRONG SIGN: Jrad(r=1) = -3.2e-2,
//                      inward at a +1 charge) while gauss_violation, the
//                      vacuum-only metric, reads a healthy 2.7e-3
//   B2 +exact_dual     -0.095 (no change)   B3 no damping  -0.105 (slightly
//                      worse)               B4 sor=100     -0.095 (no change)
//   B5 no wave+coup    +0.988  <- the dynamics ARE the mechanism
//   B6 gauss OFF       -0.106  <- projector nets ~1 point at the site
//   B7 exact+sor100    -0.095               B8 matched CG live  +0.274
//   C1/C2 L=33         -0.095 (size-independent; wavepacket IC converges to
//                      the same attractor); campaign-binned <|J|>(r=1,18) =
//                      1.44e-2 (cf. GP-KCOMP 128^3 GPU: 9.898e-3)
//
// Mechanism verdict: (e) dominates — the coupling source G_C·grad(s) points
// TOWARD a positive charge (sources div J < 0 at the site) and the leapfrog
// re-integrates the anti-enforcement drive stored in wave_vel every tick;
// the projector corrects flux only and cannot outpace it. (a)/(c)/(d) are
// measured at zero-to-negligible effect; (b) reduces the per-application
// gain to ~40% but does not cap enforcement in isolation.
//
// ── Post-amendment (2026-07-18, Term-2 electric coupling sign flip) ─────────
// The mechanism verdict drove the repair: lagrangian.h Term 2 amended to
// +g_c·s·(∇·J) (phase_read source −g_c·∇s, outward at a +1 charge) so both
// interaction terms prefer the constraint manifold. Re-measured same protocol:
//   GF-A projector isolation   f -> +0.9992   BIT-IDENTICAL to pre-fix (the
//                              operator is untouched; its unit-charge fixed
//                              point stays at W_SC(L) — the frozen §9.1
//                              prediction of EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS
//                              is UPHELD)
//   B1 live defaults tail      f = +0.1145 (was −0.0948): RIGHT-SIGNED, the
//                              live attractor is now constraint-aligned;
//                              Jrad(r=1) = +3.82e-2 OUTWARD (was −3.16e-2)
//   B1 FREEZE                  recovers +0.9996 (unchanged)
//   B2/B3/B4                   ~+0.11 (same insensitivity pattern as pre-fix)
// Remaining gap to f = 1: the wave_vel longitudinal reservoir the flux-only
// projector never cleans + the drive amplitude 3·g_c = 0.256 vs the κ = 1
// target. Completion (velocity-sector projection) is a separate [OPEN] scope
// decision — see SPEC_FTD_LAGRANGIAN.md §3.3 amendment note.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/diagnostics_compute.h"
#include "ftd/eft/matched_poisson.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <vector>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Site probe — all conventions mirror the engine exactly.
// ---------------------------------------------------------------------------
struct SiteProbe {
    double div_c = 0.0;     // central-difference div at the site (engine convention)
    double div_b = 0.0;     // backward-difference div (matched-stencil convention)
    double target = 0.0;    // coulomb_charge_coupling * (s - mean_charge)
    double frac = 0.0;      // div_c / target  — the realized-enforcement fraction
    double j_rad_r1 = 0.0;  // SIGNED outward radial flux, averaged over 6 face nbrs
    double j_mag_r1 = 0.0;  // |J| averaged over the 6 face neighbors
    double vac_resid = 0.0; // div_c - target at the +x face neighbor (vacuum site)
    double gv = 0.0;        // EnergyAudit.gauss_violation (global, vacuum-only)
};

static SiteProbe probe_site(RenderBridge& rb, int site) {
    SiteProbe p;
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const auto& tern = rb.ternary_field();
    const int N = static_cast<int>(lat.total_sites());
    const double mean_charge =
        static_cast<double>(tern.charge_sum()) / static_cast<double>(N);
    const double coupling = rb.toggles.coulomb_charge_coupling;

    // Engine central-difference divergence (poisson_solvers.cpp divergence_flux_at).
    const auto& nb = lat.neighbors_6(site);
    p.div_c = (vox[nb[0]].flux.x - vox[nb[1]].flux.x) * 0.5
            + (vox[nb[2]].flux.y - vox[nb[3]].flux.y) * 0.5
            + (vox[nb[4]].flux.z - vox[nb[5]].flux.z) * 0.5;
    p.div_b = ::ftd::eft::detail::divergence_back(vox, lat, site);

    p.target = coupling * (static_cast<double>(tern.state_at(site)) - mean_charge);
    p.frac = (std::abs(p.target) > 1e-300) ? p.div_c / p.target : 0.0;

    // Signed outward radial component + magnitude at the 6 face neighbors.
    p.j_rad_r1 = (  vox[nb[0]].flux.x - vox[nb[1]].flux.x
                  + vox[nb[2]].flux.y - vox[nb[3]].flux.y
                  + vox[nb[4]].flux.z - vox[nb[5]].flux.z ) / 6.0;
    p.j_mag_r1 = ( vox[nb[0]].flux.mag() + vox[nb[1]].flux.mag()
                 + vox[nb[2]].flux.mag() + vox[nb[3]].flux.mag()
                 + vox[nb[4]].flux.mag() + vox[nb[5]].flux.mag() ) / 6.0;

    // Vacuum residual at the +x face neighbor — the constraint the engine's
    // own gauss_violation metric DOES watch (target there is -mean_charge).
    {
        const int nvac = nb[0];
        const auto& nn = lat.neighbors_6(nvac);
        const double div_vac = (vox[nn[0]].flux.x - vox[nn[1]].flux.x) * 0.5
                             + (vox[nn[2]].flux.y - vox[nn[3]].flux.y) * 0.5
                             + (vox[nn[4]].flux.z - vox[nn[5]].flux.z) * 0.5;
        p.vac_resid = div_vac
            - coupling * (static_cast<double>(tern.state_at(nvac)) - mean_charge);
    }

    p.gv = compute_energy_audit(rb).gauss_violation;
    return p;
}

static bool should_print(int t, int total) {
    static const int marks[] = {1, 2, 3, 5, 10, 20, 50, 100, 200, 500, 1000};
    for (int m : marks) if (t == m) return true;
    return t == total;
}

static void print_row(const char* label, int t, const SiteProbe& p) {
    std::printf("    [%-14s] t=%4d  divC=%+.4e  target=%+.4e  f=%+8.4f  "
                "Jrad(r1)=%+.4e  |J|(r1)=%.4e  divB=%+.4e  vacRes=%+.2e  gv=%.3e\n",
                label, t, p.div_c, p.target, p.frac,
                p.j_rad_r1, p.j_mag_r1, p.div_b, p.vac_resid, p.gv);
}

// ---------------------------------------------------------------------------
// Config runner. Each config gets a FRESH bridge (fresh warm-start state),
// a single +1 particle at center (locked; or a wavepacket for the tie-in),
// and `ticks` applications of either the production tick or the matched-
// stencil CG projection.
// ---------------------------------------------------------------------------
struct RunCfg {
    const char* label;
    int L = 17;
    int ticks = 200;
    int sor = -1;               // -1 = engine default (SOR_ITERATIONS = 6)
    bool matched = false;       // apply matched_gauss_project instead of tick()
    bool matched_after_tick = false;  // live tick, then matched projection (B8)
    bool wavepacket = false;    // Gaussian wavepacket IC instead of bare charge
    std::function<void(RenderBridge&)> configure;
    std::function<void(RenderBridge&, int site)> post;  // optional post-run probe
};

struct RunResult {
    SiteProbe first;            // after application 1
    SiteProbe last;             // after the final application
    double frac_mean_tail = 0.0;  // mean f over the last min(100, ticks) ticks
};

static RunResult run_config(const RunCfg& cfg) {
    RenderBridge rb(cfg.L);
    rb.force_cpu();
    rb.seed_rng(1234);
    cfg.configure(rb);
    if (cfg.sor > 0) rb.set_sor_iterations(cfg.sor);

    const int c = cfg.L / 2;
    if (cfg.wavepacket) {
        rb.inject_wavepacket(c, c, c, +1);  // sigma=3.0, amplitude=K_B defaults
    } else {
        rb.inject_particle(c, c, c, +1, Vec3{0.0, 0.0, 0.0});
    }
    const int site = rb.lattice().index(c, c, c);
    rb.voxels()[site].locked = true;  // pin against evaporation/movement

    const int tail_start = std::max(1, cfg.ticks - 99);
    double tail_sum = 0.0;
    int tail_n = 0;
    RunResult res;

    for (int t = 1; t <= cfg.ticks; ++t) {
        if (cfg.matched) {
            ::ftd::eft::matched_gauss_project(rb);
        } else {
            rb.tick();
            if (cfg.matched_after_tick) ::ftd::eft::matched_gauss_project(rb);
        }
        const bool pr = should_print(t, cfg.ticks);
        if (pr || t >= tail_start || t == 1) {
            SiteProbe p = probe_site(rb, site);
            if (t == 1) res.first = p;
            if (t == cfg.ticks) res.last = p;
            if (t >= tail_start) { tail_sum += p.frac; ++tail_n; }
            if (pr) print_row(cfg.label, t, p);
        }
    }
    res.frac_mean_tail = (tail_n > 0) ? tail_sum / tail_n : 0.0;
    std::printf("    [%-14s] mean f over last %d ticks: %+.4f\n",
                cfg.label, tail_n, res.frac_mean_tail);
    if (cfg.post) cfg.post(rb, site);
    return res;
}

// ---------------------------------------------------------------------------
// Toggle configurations
// ---------------------------------------------------------------------------

// GF-A baseline: projection is the ONLY operator that ever touches flux.
// (disable_all leaves phase_write a flux no-op: delta_j_ == 0 with wave and
// coupling off, wave_vel == 0, damping off — same isolation as CP-3.)
static void cfg_projection_only(RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.strict_validation = true;
    rb.toggles.gauss_projection = true;
}

static void cfg_nothing(RenderBridge& rb) {  // for the matched-CG-only run
    rb.toggles.disable_all();
    rb.toggles.strict_validation = true;
}

// GF-B baseline: shipping defaults, single stationary particle — the
// GP-KCOMP-SHELL configuration (enable_all, genesis=false, movement=false).
static void cfg_live_defaults(RenderBridge& rb) {
    rb.toggles.enable_all();
    rb.toggles.genesis = false;
    rb.toggles.movement = false;
    rb.toggles.strict_validation = true;
}

// ---------------------------------------------------------------------------
// GF-A: the projection operator in isolation
// ---------------------------------------------------------------------------
static RunResult r_A1, r_A2, r_A4, r_A5;

void test_gf_a_projection_in_isolation() {
    section("GF-A: projection operator in isolation (single +1 charge, J=0, no dynamics)");
    std::printf("    exact-enforcement reference: div=target needs Jrad(r1) = target/3 = %.6f\n",
                (1.0 - 1.0 / (17.0 * 17.0 * 17.0)) / 3.0);
    std::printf("    constants: SOR_ITERATIONS=%d  SOR_OMEGA=%.2f  DAMPING=%.6e  G_C=%.6e  C_WAVE^2=%.6f\n",
                SOR_ITERATIONS, SOR_OMEGA, DAMPING, G_C, C_WAVE * C_WAVE);

    r_A1 = run_config({"A1 sor=6", 17, 200, -1, false, false, false,
                       cfg_projection_only, nullptr});
    r_A2 = run_config({"A2 sor=100", 17, 200, 100, false, false, false,
                       cfg_projection_only, nullptr});
    RunResult r_A3 = run_config({"A3 sor=6+exact", 17, 200, -1, false, false, false,
        [](RenderBridge& rb) { cfg_projection_only(rb); rb.toggles.exact_dual_gauss = true; },
        nullptr});
    r_A4 = run_config({"A4 sor100+exact", 17, 200, 100, false, false, false,
        [](RenderBridge& rb) { cfg_projection_only(rb); rb.toggles.exact_dual_gauss = true; },
        nullptr});
    r_A5 = run_config({"A5 matchedCG", 17, 200, -1, true, false, false,
        cfg_nothing,
        [](RenderBridge& rb, int /*site*/) {
            // One extra call to read the solver's own vacuum report. The
            // matched solver targets rho = s WITHOUT the mean-charge
            // subtraction the production solver bakes into its source
            // (poisson_solvers.cpp:164), and torus solvability forces the CG
            // to drop the source mean — so a UNIFORM residual of exactly
            // mean_charge = 1/N remains at every vacuum site. Measured:
            // 2.035e-4 = 1/4913 at L=17. That is a target-convention offset,
            // not a convergence failure; we pin it exactly.
            auto rpt = ::ftd::eft::matched_gauss_project(rb);
            const double mean_charge = 1.0 / (17.0 * 17.0 * 17.0);
            std::printf("    [A5 matchedCG  ] CG converged=%d iters=%d  "
                        "deep-vacuum RMS div after=%.3e (= mean_charge %.3e + CG tol)\n",
                        rpt.converged ? 1 : 0, rpt.iterations,
                        rpt.deep_vacuum_rms_div_after, mean_charge);
            check("GF-A5: matched CG vacuum residual == mean_charge (its rho=s target keeps the 1/N offset)",
                  std::abs(rpt.deep_vacuum_rms_div_after - mean_charge) < 1e-6,
                  "The matched-stencil CG vacuum residual no longer equals the "
                  "mean-charge solvability offset 1/N — either the solver "
                  "regressed or its source convention changed (matched_poisson.h).");
        }});

    const int N = 17 * 17 * 17;
    check_close("GF-0: solve target at the charge site = 1 - 1/N (engine's own rho)",
                r_A1.first.target, 1.0 - 1.0 / static_cast<double>(N), 1e-9);

    // The load-bearing structural fact: one production projection realizes a
    // PARTIAL, non-degenerate fraction of the site target. (Exact enforcement
    // would be f=1; a no-op would be f=0. The stencil-mismatch analysis
    // predicts an O(0.1-0.5) fraction; the measured value is printed above.)
    check("GF-A1: one production projection realizes a PARTIAL fraction (0.01 < f < 0.99)",
          r_A1.first.frac > 0.01 && r_A1.first.frac < 0.99,
          "One projection either fully enforces the site constraint (f~1: the "
          "stencil-mismatch analysis is obsolete) or does nothing (f~0: the "
          "projection is broken at the site). Either way the fidelity map in "
          "this file's header needs rewriting.");

    check("GF-A2: a saturated solve (sor=100) does not enforce LESS than sor=6 one-shot",
          r_A2.first.frac >= r_A1.first.frac - 0.02,
          "More SOR iterations made the one-shot site enforcement WORSE by >2 "
          "percentage points — inconsistent with under-convergence semantics.");
}

// ---------------------------------------------------------------------------
// GF-B: live shipping-default dynamics
// ---------------------------------------------------------------------------
static RunResult r_B1, r_B6;

void test_gf_b_live_dynamics() {
    section("GF-B: live dynamics at L=17 (enable_all; genesis/movement off; 1000 ticks)");

    r_B1 = run_config({"B1 defaults", 17, 1000, -1, false, false, false,
        cfg_live_defaults,
        [](RenderBridge& rb, int site) {
            // FREEZE probe: apply ONLY the production projector to the
            // settled live FLUX configuration. This separates "the projector
            // is weak at the site" from "the dynamics undo what it achieves".
            //
            // The live bridge cannot be frozen in place through the public
            // API: phase_write's leapfrog (wave_vel += delta_j; flux +=
            // wave_vel) is NOT gated by wave_propagation, and the delta_j_
            // buffers (private, stale from the last live phase_read) plus
            // the wave_vel field would keep re-injecting flux every tick —
            // an in-place "freeze" measures those hidden reservoirs, not the
            // projector. (That is itself a finding: the projection corrects
            // flux only; wave_vel/delta_j carry the standing
            // anti-enforcement drive it never touches.) So instead the
            // settled flux is COPIED onto a fresh projection-only bridge
            // where every reservoir (wave_vel, delta_j, warm phi) is
            // genuinely zero — the exact GF-A condition with the live field
            // as initial data.
            (void)site;
            const int L = rb.lattice().size();
            const int N = static_cast<int>(rb.lattice().total_sites());
            RenderBridge fresh(L);
            fresh.force_cpu();
            fresh.seed_rng(1234);
            cfg_projection_only(fresh);
            const int c = L / 2;
            fresh.inject_particle(c, c, c, +1, Vec3{0.0, 0.0, 0.0});
            {
                const auto& src = rb.voxels();
                auto& dst = fresh.voxels();
                for (int i = 0; i < N; ++i) dst[i].flux = src[i].flux;
            }
            const int fsite = fresh.lattice().index(c, c, c);
            fresh.voxels()[fsite].locked = true;
            std::printf("    [B1 FREEZE     ] settled live flux copied to a fresh projection-only bridge:\n");
            SiteProbe pf;
            for (int k = 1; k <= 200; ++k) {
                fresh.tick();
                if (k == 1 || k == 10 || k == 50 || k == 200) {
                    pf = probe_site(fresh, fsite);
                    print_row("B1 freeze", k, pf);
                }
            }
            check("GF-B1f: projector alone RECOVERS the site constraint from the live-settled flux (f > 0.9 after 200 apps)",
                  pf.frac > 0.9,
                  "Given the live-settled flux as initial data and every "
                  "dynamical reservoir empty, 200 projector applications no "
                  "longer restore div(J)=s at the charge site — the live flux "
                  "configuration has left the projector's convergent regime, "
                  "contradicting the dynamics-reinjection mechanism split "
                  "measured here.");
        }});

    RunResult r_B2 = run_config({"B2 +exact", 17, 1000, -1, false, false, false,
        [](RenderBridge& rb) { cfg_live_defaults(rb); rb.toggles.exact_dual_gauss = true; },
        nullptr});
    RunResult r_B3 = run_config({"B3 no damping", 17, 1000, -1, false, false, false,
        [](RenderBridge& rb) {
            cfg_live_defaults(rb);
            rb.toggles.damping = false;
            rb.toggles.selective_damping = false;  // validity: selective requires damping
        },
        nullptr});
    RunResult r_B4 = run_config({"B4 sor=100", 17, 1000, 100, false, false, false,
        cfg_live_defaults, nullptr});
    RunResult r_B5 = run_config({"B5 no wave", 17, 1000, -1, false, false, false,
        [](RenderBridge& rb) {
            cfg_live_defaults(rb);
            rb.toggles.wave_propagation = false;
            rb.toggles.coupling = false;
        },
        nullptr});
    r_B6 = run_config({"B6 gauss OFF", 17, 1000, -1, false, false, false,
        [](RenderBridge& rb) { cfg_live_defaults(rb); rb.toggles.gauss_projection = false; },
        nullptr});
    RunResult r_B7 = run_config({"B7 exact+sor100", 17, 1000, 100, false, false, false,
        [](RenderBridge& rb) { cfg_live_defaults(rb); rb.toggles.exact_dual_gauss = true; },
        nullptr});
    RunResult r_B8 = run_config({"B8 matched live", 17, 1000, -1, false, true, false,
        [](RenderBridge& rb) { cfg_live_defaults(rb); rb.toggles.gauss_projection = false; },
        nullptr});

    (void)r_B7; (void)r_B8;

    // The headline comparison: live dynamics vs the solver's own capability.
    check("GF-B1: live-defaults realized fraction sits BELOW the solver's one-shot capability",
          r_B1.frac_mean_tail < r_A2.first.frac,
          "The live tick cycle now realizes at least as much of div(J)=s at "
          "the charge site as an isolated saturated projection — the "
          "dynamics-erasure characterization no longer holds; re-measure the "
          "mechanism split.");

    // ── Documentation-of-reality pins (measured 2026-07-16, L=17, seed 1234;
    //    deterministic CPU run — tolerances absorb ULP-level drift only). ──
    check("GF-A1r: repeated projection IN ISOLATION converges at the site (f(200) > 0.99; measured +0.9996)",
          r_A1.last.frac > 0.99,
          "The production projector, applied repeatedly with no dynamics, no "
          "longer converges to div(J)=s at the charge site. The isolation "
          "story (stencil mismatch slows but does not cap enforcement at odd "
          "L) has changed — re-derive the mechanism split.");
    check("GF-B1s: live defaults settle WRONG-SIGNED at the site (tail f < -0.02; measured -0.095)",
          r_B1.frac_mean_tail < -0.02,
          "The live steady state no longer has an inward-pointing near-field "
          "at a +1 charge. The coupling-source sign analysis (G_C*grad(s) "
          "points toward a positive charge) and the fidelity table in this "
          "file's header need re-measurement.");
    check("GF-B5p: removing wave+coupling restores near-full enforcement (tail > 0.95; measured +0.988)",
          r_B5.frac_mean_tail > 0.95,
          "With the wave/coupling dynamics off, the per-tick projector no "
          "longer holds the site constraint — some OTHER operator now erodes "
          "it; the wave-dynamics-dominant mechanism split has changed.");
    check("GF-B6p: gauss OFF settles within 0.03 of gauss ON (projector nets ~1 point at the site)",
          std::abs(r_B6.frac_mean_tail - r_B1.frac_mean_tail) < 0.03,
          "The projector's net contribution at the charge site under live "
          "defaults is no longer negligible — the near-charge field is no "
          "longer set almost entirely by the wave+coupling steady state.");
    check("GF-B234p: exact_dual_gauss / damping / SOR=100 each move the live tail by < 0.02",
          std::abs(r_B2.frac_mean_tail - r_B1.frac_mean_tail) < 0.02 &&
          std::abs(r_B3.frac_mean_tail - r_B1.frac_mean_tail) < 0.02 &&
          std::abs(r_B4.frac_mean_tail - r_B1.frac_mean_tail) < 0.02,
          "One of the candidate mechanisms (correction skip at manifested "
          "sites / selective damping / SOR iteration count) now materially "
          "changes the live site enforcement — the ruled-out list in this "
          "file's header is stale.");
}

// ---------------------------------------------------------------------------
// GF-C: size check + wavepacket tie-in (CPU analog of GP-KCOMP-SHELL)
// ---------------------------------------------------------------------------
void test_gf_c_size_and_wavepacket() {
    section("GF-C: L=33 size check + wavepacket-IC tie-in (DERIV_KCOMP J(r=1)=9.898e-3 at 128^3 GPU)");

    RunResult c1 = run_config({"C1 L33 defaults", 33, 1000, -1, false, false, false,
        cfg_live_defaults, nullptr});
    (void)c1;

    RunResult c2 = run_config({"C2 L33 wavepkt", 33, 1000, -1, false, false, true,
        cfg_live_defaults,
        [](RenderBridge& rb, int site) {
            // GP-KCOMP-SHELL binning replica: <|J|> over the round(r)==1 shell,
            // i.e. the 6 face + 12 edge neighbors (sqrt(2) rounds to 1).
            const auto& vox = rb.voxels();
            const auto& lat = rb.lattice();
            const int L = lat.size();
            const int cx = L / 2;
            double sum = 0.0; int cnt = 0; double jpeak = 0.0;
            for (int dx = -2; dx <= 2; ++dx)
              for (int dy = -2; dy <= 2; ++dy)
                for (int dz = -2; dz <= 2; ++dz) {
                    const double r = std::sqrt(double(dx*dx + dy*dy + dz*dz));
                    const int i = lat.index(cx + dx, cx + dy, cx + dz);
                    if (static_cast<int>(std::round(r)) == 1) {
                        sum += vox[i].flux.mag();
                        ++cnt;
                    }
                }
            for (int i = 0; i < static_cast<int>(lat.total_sites()); ++i)
                jpeak = std::max(jpeak, vox[i].flux.mag());
            std::printf("    [C2 L33 wavepkt] campaign-binned <|J|>(r=1, %d sites)=%.4e  "
                        "J_peak=%.4e  (GPU 128^3 reference: 9.898e-3 / 2.879e-2)\n",
                        cnt, sum / cnt, jpeak);
            (void)site;
        }});
    (void)c2;
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_gauss_law_fidelity");

    ftd::test::test_gf_a_projection_in_isolation();
    ftd::test::test_gf_b_live_dynamics();
    ftd::test::test_gf_c_size_and_wavepacket();

    return ftd::test::finalize();
}
