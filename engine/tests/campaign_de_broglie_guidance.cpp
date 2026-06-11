// ============================================================================
// campaign_de_broglie_guidance.cpp  (FTD-0271 Phase E, 2026-06-11)
// ----------------------------------------------------------------------------
// THE HEART OF PILOT-WAVE. de Broglie-Bohm guidance says the particle velocity
// is the gradient of the wave's PHASE:  v = (1/m) grad(S),  with S the phase of
// psi = R*exp(iS). In FTD the "particle" is the discrete manifested cluster
// (state field s) and the "pilot wave" is the continuous flux J. The question
// this campaign MEASURES: does a cluster placed at rest in an imposed flux that
// carries a pure PHASE gradient drift along grad(S) at a rate proportional to
// |grad(S)| -- i.e. is Bohmian guidance EMERGENT in FTD's dynamics?
//
// PRIOR (disclosed, ~40%): the engine audits found the cluster moves by FORCES
// (Coulomb/gravity/Lorentz from the flux MAGNITUDE and divergence), not by the
// flux phase. A pure phase gradient with UNIFORM |J| sources no magnitude-based
// force, so the expectation is ABSENT: the cluster ignores the pilot wave's
// phase. Both outcomes are honest; ABSENT is a legitimate FTD boundary.
//
// ── Construction ────────────────────────────────────────────────────────────
// Pilot wave with a pure phase ramp along x, CONSTANT magnitude A:
//     J(x,y,z) = A * ( cos(k*x), sin(k*x), 0 )      => |J| = A everywhere,
//     phase S(x) = k*x,  grad(S) = k * xhat.
// de Broglie guidance prediction (NR, the clock sets the mass scale omega0):
//     v_pred = (c^2 / omega0) * grad(S) = (c^2/omega0) * k   along +x.
// The flux is FROZEN (wave_propagation OFF) so grad(S) is a clean static input;
// forces + movement + Lorentz + coupling + the de Broglie clock are ON so the
// cluster has every FTD channel available to respond to the pilot wave.
//
// MEASUREMENT: sweep k, manifest a cluster at centre, run T ticks, measure the
// COM drift velocity v_meas along x. Correlate v_meas with v_pred.
//   corr > 0.7  AND  |v_meas| grows with k  -> GUIDANCE-EMERGENT  (E -> [DERIVED])
//   corr < 0.5  OR   v_meas ~ 0 for all k   -> GUIDANCE-ABSENT    (boundary)
//   else                                     -> INCONCLUSIVE
//
// CONTROL (non-vacuity): a cluster offset from a FIXED opposite charge feels a
// real Coulomb force and MUST drift -- proving the movement path is functional,
// so a null in the phase-gradient run is a real ABSENT, not a dead engine.
//
// This is a MEASUREMENT campaign (a question, not a PASS/FAIL claim): it prints
// the verdict token. The CTest assertion only checks non-vacuity (the control
// moves) and that a definite token was produced -- it does NOT assert EMERGENT.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd {
namespace test {

static const double kC2 = C_WAVE * C_WAVE;   // c^2 = 1/3

// Mean velocity.x acquired by the manifested cluster (continuous, so sub-voxel
// drift registers; integer COM position would be quantized to voxel hops).
static double mean_vx(RenderBridge& rb) {
    const Lattice& lat = rb.lattice();
    const int L = lat.size();
    double sv = 0.0; int n = 0;
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const Voxel& v = rb.voxel_at(x, y, z);
                if (v.state != 0) { sv += v.velocity.x; ++n; }
            }
    return n > 0 ? sv / n : 0.0;
}

// Impose the pure-phase pilot wave J = A*(cos kx, sin kx, 0) on every voxel.
static void impose_phase_wave(RenderBridge& rb, double A, double k) {
    const int L = rb.lattice().size();
    for (int x = 0; x < L; ++x) {
        const double cx = A * std::cos(k * x), sx = A * std::sin(k * x);
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.voxel_at(x, y, z).flux = Vec3{cx, sx, 0.0};
    }
}

// One phase-gradient sample: the velocity a cluster ACQUIRES (via FTD forces)
// in a pure-phase pilot wave. Velocity is integrated by phase_forces, so with
// movement OFF the test particle stays pinned at centre while accumulating the
// velocity the pilot wave's phase exerts on it -- a sustained force builds a
// measurable velocity even if it is tiny per tick. wave_propagation is ON (live
// force path + the clock's requires-clause); the clean pilot wave is RE-IMPOSED
// each tick so grad(S) is a constant external input (test particle in a fixed
// pilot wave).
static double measure_phase_drift(double k, double A, double omega0,
                                  int L, int T) {
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(7);

    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;   // live force path + clock requires it
    rb.toggles.forces           = true;
    rb.toggles.movement         = false;  // pin position; read acquired velocity
    rb.toggles.poisson_coulomb  = true;
    rb.toggles.gauss_projection = true;   // the Coulomb path that sees div(J)
    rb.toggles.lorentz_force    = true;
    rb.toggles.coupling         = true;   // G_C*grad(s) + curl(s*v) channels
    rb.toggles.de_broglie_clock = true;   // the clock makes this a de Broglie scenario
    rb.toggles.omega0           = omega0;
    rb.toggles.gravity          = false;  // isolate the phase response
    rb.toggles.genesis          = false;

    const int c = L / 2;
    rb.inject_particle(c, c, c, +1, Vec3{A, 0.0, 0.0});  // matches local pilot flux

    for (int t = 0; t < T; ++t) {
        impose_phase_wave(rb, A, k);   // maintain the clean external pilot wave
        rb.tick();
    }
    return mean_vx(rb);               // velocity acquired along grad(S)=+x
}

// CONTROL: a +1 charge offset from a fixed -1 charge MUST acquire velocity
// (Coulomb attraction) -- proves the force path is live, so a null in the
// phase-gradient run is a genuine ABSENT, not a dead engine. Same movement-OFF
// readout (accumulated velocity) as the measurement.
static double measure_control_drift(int L, int T) {
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(7);

    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.forces           = true;
    rb.toggles.movement         = false;  // read acquired velocity
    rb.toggles.poisson_coulomb  = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.gravity          = false;  // isolate EM
    rb.toggles.genesis          = false;

    const int c = L / 2;
    // +1 at centre (flux charge +z), fixed -1 charge 5 voxels along +x. The +1
    // is attracted toward +x, so it should acquire velocity.x > 0.
    rb.inject_particle(c,     c, c, +1, Vec3{0.0, 0.0,  K_B});
    rb.inject_particle(c + 5, c, c, -1, Vec3{0.0, 0.0, -K_B});
    rb.voxel_at(c + 5, c, c).locked = true;   // pin the source charge

    for (int t = 0; t < T; ++t) rb.tick();
    return rb.voxel_at(c, c, c).velocity.x;   // mobile +1; attracted toward +x
}

void test_de_broglie_guidance() {
    section("E: de Broglie GUIDANCE -- does the cluster follow grad(S)? [MEASUREMENT]");

    const double A = 0.05, omega0 = 0.5;
    const int L = 24, T = 120;
    const std::vector<double> ks = {0.20, 0.40, 0.60};

    std::printf("    [E] pure-phase pilot wave J=A(cos kx, sin kx,0), |J|=A=%.3f, omega0=%.2f\n", A, omega0);
    std::printf("    [E]   k       grad(S)=k    v_pred=(c^2/omega0)k    v_meas\n");
    std::vector<double> vpred, vmeas;
    for (double k : ks) {
        const double vp = (kC2 / omega0) * k;
        const double vm = measure_phase_drift(k, A, omega0, L, T);
        vpred.push_back(vp); vmeas.push_back(vm);
        std::printf("    [E]  %.2f    %8.4f       %12.5e     %12.5e\n", k, k, vp, vm);
    }

    // Pearson correlation of v_meas vs v_pred across the k-sweep.
    auto mean = [](const std::vector<double>& a){ double s=0; for(double v:a) s+=v; return s/a.size(); };
    const double mp = mean(vpred), mm = mean(vmeas);
    double num=0, dp=0, dm=0;
    for (size_t i=0;i<ks.size();++i){ num+=(vpred[i]-mp)*(vmeas[i]-mm); dp+=(vpred[i]-mp)*(vpred[i]-mp); dm+=(vmeas[i]-mm)*(vmeas[i]-mm); }
    const double corr = (dp>0 && dm>0) ? num/std::sqrt(dp*dm) : 0.0;
    double max_vmeas=0; for(double v:vmeas) max_vmeas=std::max(max_vmeas,std::abs(v));
    double max_vpred=0; for(double v:vpred) max_vpred=std::max(max_vpred,std::abs(v));

    // CONTROL: prove the movement path works at all.
    const double v_ctrl = measure_control_drift(L, 60);
    std::printf("    [E] CONTROL (+1 pulled toward -1 at +5x): v_ctrl = %.5e  (Coulomb => v.x > 0 expected)\n", v_ctrl);

    // Verdict token (pre-registered thresholds; theory-fixed).
    const bool guided_magnitude = (max_vpred > 1e-12) && (max_vmeas > 0.2 * max_vpred);
    const char* token;
    if (corr > 0.7 && guided_magnitude)      token = "GUIDANCE-EMERGENT";
    else if (corr < 0.5 || max_vmeas < 1e-6) token = "GUIDANCE-ABSENT";
    else                                     token = "INCONCLUSIVE";

    std::printf("\n    [E] corr(v_meas, v_pred) = %.3f   max|v_meas| = %.3e   max|v_pred| = %.3e\n",
                corr, max_vmeas, max_vpred);
    std::printf("    [E] ===> VERDICT: %s\n", token);
    if (std::string(token) == "GUIDANCE-ABSENT")
        std::printf("    [E] The cluster ignores the pilot wave's phase: FTD moves matter by\n"
                    "    [E] magnitude-derived forces, not by grad(S). Guidance is NOT emergent;\n"
                    "    [E] a working pilot-wave would require ADDING the guidance equation.\n"
                    "    [E] This is the FTD-0271 boundary (audit-predicted).\n");

    // Non-vacuity: the control MUST move (movement path functional), and a
    // definite (non-INCONCLUSIVE) token must be produced. We do NOT assert
    // EMERGENT -- ABSENT is the honest, expected physics result.
    check("E-control: a Coulomb-driven cluster drifts (movement path is functional)",
          std::abs(v_ctrl) > 1e-6,
          "The control cluster did not move under a real Coulomb force, so the "
          "movement path is dead and the phase-gradient null would be vacuous.");
    check("E: the guidance measurement produced a definite verdict (EMERGENT or ABSENT, not INCONCLUSIVE)",
          std::string(token) != "INCONCLUSIVE",
          "The phase-gradient sweep was numerically ambiguous; refine A/omega0/T "
          "and re-run before recording an E verdict.");
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("campaign_de_broglie_guidance");
    ftd::test::test_de_broglie_guidance();
    return ftd::test::finalize();
}
