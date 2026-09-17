/**
 * Test: flux_pump work accounting under dual_substrate.
 *
 * WHY. `apply_flux_cell_pump_increment` (engine/src/flux_cell.cpp) books the
 * injected work from the AGGREGATE registers (v.flux, v.wave_vel):
 *
 *     dH = 1/2 c^2 [ W.L(d) - 2 J.L(d) - d.L(d) ]
 *
 * while the same function's dual branch applies the increment as d/2 to
 * flux_L AND d/2 to flux_R. The obvious worry is that the dual writes inject
 * energy the aggregate booking never sees. They do not, and this test is the
 * standing proof:
 *
 *   - the two half-increments sum to exactly the same +d the aggregate
 *     register receives (flux_L + flux_R == flux is the invariant the dual
 *     path maintains: phase_write rebuilds flux := flux_L + flux_R every tick,
 *     and gauss_project applies half its correction to each substrate), and
 *   - every energy quantity the engine reports — EnergyAudit::field_energy and
 *     FluxCellLedger::H_wave alike — is computed from the AGGREGATE registers,
 *     so the aggregate dH is the change in the engine's own energy.
 *
 * The declared flux_pump/dual_substrate conflict in term_toggles.h is
 * non-fatal without strict_validation, so this combination is reachable in
 * practice; these checks say what it does rather than leaving it untested. The
 * shipped s0-cell-* regression (test_flux_cell_scenario_physics.cpp) runs
 * single-substrate only, so dual had no coverage before this file.
 *
 * Checks:
 *   PDW-1: single substrate — one raw increment's booked dH equals the exact
 *          change in the region Hamiltonian (control).
 *   PDW-2: dual substrate — same, to the same tolerance. If the dual writes
 *          injected unbooked energy this is where it would show.
 *   PDW-3: dual substrate — the increment preserves flux == flux_L + flux_R,
 *          the invariant the aggregate booking rests on.
 *   PDW-4: dual substrate — full ledger closure over a real pumped run:
 *          H_wave(end) - H_wave(start) == flux_pump_work() over 10 increments
 *          plus a free hold, with no other term in the tick.
 *   PDW-5: the substrate-sum Hamiltonian H(J_L,W_L) + H(J_R,W_R) is a DIFFERENT
 *          conserved quantity whose increment change is exactly half the booked
 *          value. Recorded so the convention is explicit: the engine books
 *          against the aggregate Hamiltonian it reports, not against the sum of
 *          the two substrate Hamiltonians. "Fixing" the booking to the
 *          substrate sum would put the pump ledger a factor of 2 away from
 *          every energy number the audit prints.
 */

#include "ftd/constants.h"
#include "ftd/flux_cell.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>

namespace {

constexpr int L = 17;
constexpr int PUMP_TICKS = 10;
constexpr int HOLD_TICKS = 40;
// The pump ledger gate the shipped single-substrate regression uses.
constexpr double LEDGER_GATE = 1e-9;

ftd::FluxCellRegion whole_box() {
    ftd::FluxCellRegion r;
    r.cx = (L - 1) * 0.5;
    r.cy = (L - 1) * 0.5;
    r.cz = (L - 1) * 0.5;
    r.radius = 4.0 * L;             // covers every site
    return r;
}

std::unique_ptr<ftd::RenderBridge> make_bridge(bool dual) {
    auto rb = std::make_unique<ftd::RenderBridge>(L);
    rb->force_cpu();
    rb->toggles.disable_all();
    rb->toggles.wave_propagation = true;
    rb->toggles.dual_substrate = dual;
    rb->toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    // A ring of circulating flux so J (and therefore the -2 J.L(d) cross term
    // in the booked work) is non-trivial. seed_flux_cell_torus splits the seed
    // half/half across the substrates when dual is on.
    ftd::FluxCellTorusSpec seed = ftd::default_flux_cell_torus_spec(L);
    seed.amplitude = 0.25;
    ftd::seed_flux_cell_torus(*rb, seed, 1.0);
    return rb;
}

ftd::FluxCellTorusSpec pump_spec() {
    ftd::FluxCellTorusSpec s = ftd::default_flux_cell_torus_spec(L);
    s.amplitude = 0.12;
    s.cz += 1.0;                    // offset from the seeded ring
    return s;
}

double region_H(const ftd::RenderBridge& rb) {
    return ftd::compute_flux_cell_ledger(rb, whole_box()).H_wave;
}

// The periodic 18-point Laplacian of an arbitrary per-site vector field —
// the same operator the ledger and the pump-work formula use.
ftd::Vec3 lap18(const std::vector<ftd::Vec3>& f, const ftd::Lattice& lat, int i) {
    ftd::Vec3 out;
    for (const int n : lat.neighbors_6(i))
        out += f[static_cast<std::size_t>(n)] * (1.0 / 3.0);
    for (const int n : lat.neighbors_12(i))
        out += f[static_cast<std::size_t>(n)] * (1.0 / 6.0);
    out -= f[static_cast<std::size_t>(i)] * 4.0;
    return out;
}

// H(J, W) = sum_i [ 1/2|W|^2 + 1/2 c^2 W.L18(J) - 1/2 c^2 J.L18(J) ] evaluated
// on an explicit (flux, wave_vel) pair — used for the L and R substrates,
// which the aggregate ledger does not expose.
double substrate_H(const ftd::RenderBridge& rb, bool left) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const std::size_t N = vox.size();
    std::vector<ftd::Vec3> J(N), W(N);
    for (std::size_t i = 0; i < N; ++i) {
        J[i] = left ? vox[i].flux_L : vox[i].flux_R;
        W[i] = left ? vox[i].wave_vel_L : vox[i].wave_vel_R;
    }
    constexpr double C2 = ftd::C_SPEED * ftd::C_SPEED;
    double h = 0.0;
    for (std::size_t i = 0; i < N; ++i) {
        const ftd::Vec3 lj = lap18(J, lat, static_cast<int>(i));
        h += 0.5 * W[i].mag2() + 0.5 * C2 * W[i].dot(lj)
           - 0.5 * C2 * J[i].dot(lj);
    }
    return h;
}

double max_dual_residual(const ftd::RenderBridge& rb) {
    double worst = 0.0;
    for (const auto& v : rb.voxels()) {
        const ftd::Vec3 d = v.flux - (v.flux_L + v.flux_R);
        worst = std::max(worst, d.mag());
    }
    return worst;
}

// One raw increment, outside the tick loop: booked value vs the exact change
// in the region Hamiltonian.
void one_increment(bool dual, const char* label, double& rel_err,
                   double& half_ratio, double& dual_residual) {
    auto rb = make_bridge(dual);
    const ftd::FluxCellPumpProfile profile =
        ftd::build_flux_cell_pump_profile(*rb, pump_spec(), PUMP_TICKS);

    const double H_before = region_H(*rb);
    const double sub_before = dual ? substrate_H(*rb, true) + substrate_H(*rb, false) : 0.0;

    const double booked = ftd::apply_flux_cell_pump_increment(*rb, profile);

    const double H_after = region_H(*rb);
    const double sub_after = dual ? substrate_H(*rb, true) + substrate_H(*rb, false) : 0.0;

    const double actual = H_after - H_before;
    rel_err = std::fabs(booked - actual) / std::max(1e-12, std::fabs(actual));
    half_ratio = dual && std::fabs(sub_after - sub_before) > 1e-15
        ? booked / (sub_after - sub_before) : 0.0;
    dual_residual = dual ? max_dual_residual(*rb) : 0.0;

    std::printf("    %s: booked=%.15g  actual dH=%.15g  rel=%.3e\n",
                label, booked, actual, rel_err);
    if (dual)
        std::printf("    %s: substrate-sum dH=%.15g  booked/substrate-sum=%.9f\n",
                    label, sub_after - sub_before, half_ratio);
}

}  // namespace

int main() {
    ftd::test::init("test_flux_cell_pump_dual_work");

    double rel_single = 0.0, rel_dual = 0.0, ratio = 0.0, resid = 0.0, unused = 0.0;

    // ------------------------------------------------------------------
    ftd::test::section("PDW-1: single substrate increment (control)");
    {
        one_increment(/*dual=*/false, "single", rel_single, unused, unused);
        ftd::test::metric("pdw1.rel_err", rel_single, 0);
        ftd::test::check("PDW-1: booked work == exact region-Hamiltonian change",
                         rel_single < LEDGER_GATE);
    }

    // ------------------------------------------------------------------
    ftd::test::section("PDW-2/3/5: dual substrate increment");
    {
        one_increment(/*dual=*/true, "dual", rel_dual, ratio, resid);
        ftd::test::metric("pdw2.rel_err", rel_dual, 0);
        ftd::test::metric("pdw3.dual_residual", resid, 0);
        ftd::test::metric("pdw5.booked_over_substrate_sum", ratio, 0);

        ftd::test::check("PDW-2: with dual_substrate ON the booked work still "
                         "equals the exact region-Hamiltonian change (the two "
                         "half-increments are the aggregate increment, not "
                         "extra unbooked energy)",
                         rel_dual < LEDGER_GATE);
        ftd::test::check("PDW-3: the increment preserves flux == flux_L + flux_R",
                         resid < 1e-12);
        // Analytic: for J_L -> J_L + d/2 and J_R -> J_R + d/2 the substrate
        // Hamiltonians change by half the aggregate value, because H is
        // quadratic and the cross terms halve. Pinned so the convention the
        // engine books against cannot drift silently.
        ftd::test::check_close("PDW-5: booked work is exactly twice the "
                               "substrate-sum change (aggregate convention, "
                               "matching every reported energy channel)",
                               ratio, 2.0, 1e-6);
    }

    // ------------------------------------------------------------------
    ftd::test::section("PDW-4: dual substrate — full pumped-run ledger closure");
    {
        auto rb = make_bridge(true);
        rb->toggles.flux_pump = true;
        rb->set_flux_pump(pump_spec(), PUMP_TICKS, 1);

        const double H0 = region_H(*rb);
        rb->run(PUMP_TICKS + HOLD_TICKS);
        const double H1 = region_H(*rb);
        const double W_in = rb->flux_pump_work();
        const double actual = H1 - H0;
        const double rel = std::fabs(W_in - actual) / std::max(1e-12, std::fabs(actual));

        std::printf("    W_in=%.15g  dH=%.15g  rel=%.3e  applied=%d/%d\n",
                    W_in, actual, rel, rb->flux_pump_ticks_applied(), PUMP_TICKS);
        ftd::test::metric("pdw4.W_in", W_in, PUMP_TICKS + HOLD_TICKS);
        ftd::test::metric("pdw4.dH", actual, PUMP_TICKS + HOLD_TICKS);
        ftd::test::metric("pdw4.rel_err", rel, PUMP_TICKS + HOLD_TICKS);

        ftd::test::check("PDW-4a: the pump applied exactly its scheduled ticks",
                         rb->flux_pump_ticks_applied() == PUMP_TICKS);
        ftd::test::check("PDW-4b: the pump injected measurable energy",
                         std::fabs(W_in) > 1e-9);
        ftd::test::check("PDW-4c: dual-substrate pump ledger closes "
                         "(H_end - H_start == booked work)",
                         rel < LEDGER_GATE);
        ftd::test::check("PDW-4d: the run kept flux == flux_L + flux_R",
                         max_dual_residual(*rb) < 1e-12);
    }

    return ftd::test::finalize();
}
