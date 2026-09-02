/**
 * Behavioral tests for the s0-cell-* flux-cell scenarios.
 *
 * A flux cell is a localized field configuration whose energy is meant to
 * stay above vacuum after its pump is disconnected. These checks measure only
 * native observables through the regional ledger in ftd/flux_cell.h:
 *
 *   - the kick-drift Hamiltonian H_wave (conserved by the periodic wave map)
 *   - the electric / magnetic channels U_E, U_B and their balance coordinate
 *   - the flux-potential channel U_J (the Gauss-built capacitor channel)
 *   - ring circulation Gamma_J, disk flux Phi_B, net Poynting flow, flux dyad
 *   - region retention and boundary loss under three boundary laws
 *   - the pump-off ledger: charge for N ticks, disconnect, hold
 *
 * No check asserts a capacitor, inductor, battery, or particle identity. Every
 * retention number is reported as a metric and stated in the scenario
 * validation text as a measurement, not as a storage claim.
 */

#include "ftd/constants.h"
#include "ftd/flux_cell.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/test_telemetry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

namespace {

constexpr int L = 33;
constexpr int HOLD_TICKS = 300;
constexpr int SAMPLE_EVERY = 5;
constexpr int PUMP_TICKS = 20;

// Gates. Exact-symmetry gates sit far above rounding (measured 1e-14..1e-16).
// Measured-value pins record the 2026-09-02 L=33 CPU run of record with a
// tolerance wide enough for platform rounding and narrow enough that a change
// in the wave map, the Gauss solver, or the seed geometry trips them. Every
// pinned number is also emitted as a metric so drift is visible before it
// fails a gate.
constexpr double HAMILTONIAN_DRIFT_GATE = 1e-9;   // |H - H0| / H0 (measured <= 6e-14)
constexpr double SYMMETRY_GATE = 1e-9;            // |sum S| / sum |S|, |sum J| / sqrt(tr dyad)
constexpr double MIRROR_GATE = 1e-12;             // reverse-vs-forward equality
constexpr double EXCHANGE_GATE = 0.25;            // U_E must reach this fraction of U_E+U_B
constexpr double OPEN_LOSS_GATE = 1e-3;           // open box keeps < 0.1% of H0 (measured 2.5e-7)
constexpr double PUMP_LEDGER_GATE = 1e-9;         // |W_in - H_hold| / H_hold
constexpr double PIN_REL = 0.05;                  // relative tolerance on pinned energies
constexpr double PIN_ABS = 0.05;                  // absolute tolerance on pinned fractions

// Pinned measurements (L=33, 300-tick hold, CPU, 2026-09-02).
constexpr double PIN_CAPACITOR_GAP_UJ_T1 = 11.7583;     // gap U_J after the first projection
constexpr double PIN_CAPACITOR_RETENTION = 0.3151;      // gap U_J(300) / U_J(1)
constexpr double PIN_CAPACITOR_GAUSS_END = 0.10;        // max Gauss error bound at tick 300 (measured 0.073)
constexpr double PIN_TORUS_RETENTION = 0.3634;          // ring-region (U_E+U_B) retention
constexpr double PIN_TORUS_UE_FRAC_MAX = 0.7139;        // peak electric share of U_E+U_B
constexpr double PIN_SCRAMBLED_RETENTION = 0.9144;      // scrambled ring-region retention
constexpr double PIN_SCRAMBLED_OVER_COHERENT_MIN = 2.0; // measured 2.52
constexpr double PIN_TRIAD_OFFDIAG_MAX = 0.03;          // dyad off-diagonal / trace bound (measured 0.0191)
constexpr double PIN_TRIAD_RETENTION = 0.1537;          // arm-region retention
// Membrane family pins (L=33, 300-tick hold, CPU, 2026-09-02).
constexpr double PIN_MEMBRANE_RETENTION = 1.0033;        // inner ball, 3-cell clocked shell
constexpr double PIN_MEMBRANE_TRANSPARENT = 0.2228;      // same shell at omega0 = 0.05
constexpr double PIN_MEMBRANE_RETENTION_T1 = 0.5756;     // 1-cell shell
constexpr double PIN_MEMBRANE_RETENTION_T2 = 0.9913;     // 2-cell shell
constexpr double PIN_GATED_W_OUT = 0.038076;             // port current integral, ticks 150..300
constexpr double PIN_GATED_CLOSED_DRIFT_MAX = 0.02;      // |dH_cell/H| over the closed phase (measured 0.0058)
constexpr double PIN_GATED_RAW_RATIO_MIN = 0.7;          // W_out / cell loss (measured 0.834)
constexpr double PIN_GATED_CORRECTED_RATIO = 1.0;        // after subtracting the closed-phase wall leak (measured 1.05)
constexpr double PIN_GATED_CORRECTED_TOL = 0.2;
constexpr double PIN_PUMPED_W_IN = 0.049280;             // engine-booked pump work
constexpr double PIN_PUMPED_RETENTION = 0.8349;          // inner ball after 300 held ticks

double mid() { return (L - 1) * 0.5; }

std::unique_ptr<ftd::RenderBridge> make_at(int lattice, const char* id) {
    auto rb = std::make_unique<ftd::RenderBridge>(lattice);
    rb->force_cpu();
    const bool ok = ftd::dispatch_scenario(*rb, id);
    ftd::test::check((std::string(id) + " dispatched").c_str(), ok);
    return rb;
}

std::unique_ptr<ftd::RenderBridge> make(const char* id) { return make_at(L, id); }

ftd::FluxCellRegion whole_box() {
    return ftd::FluxCellRegion{mid(), mid(), mid(), 2.0 * L};
}

ftd::FluxCellRegion ring_region(const ftd::FluxCellTorusSpec& spec) {
    return ftd::FluxCellRegion{spec.cx, spec.cy, spec.cz,
                               spec.major_radius + 3.0 * spec.tube_sigma};
}

// Interior kick-drift Hamiltonian under the Reflective boundary law: the
// Neumann ghost shell is a copy of the first interior layer, so the
// Laplacian samples clamp to [1, L-2]. Mirrors test_boundary_scenario_physics.
double reflective_interior_hamiltonian(const ftd::RenderBridge& rb) {
    const int N = rb.lattice().size();
    const auto sample = [&](int sx, int sy, int sz) -> const ftd::Vec3& {
        sx = std::clamp(sx, 1, N - 2);
        sy = std::clamp(sy, 1, N - 2);
        sz = std::clamp(sz, 1, N - 2);
        return rb.voxels()[static_cast<std::size_t>(
            rb.lattice().index(sx, sy, sz))].flux;
    };
    const int faces[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };
    const int edges[12][3] = {
        {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
        {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
        {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
    };
    double kinetic = 0.0, cross = 0.0, gradient = 0.0;
    for (int x = 1; x < N - 1; ++x)
    for (int y = 1; y < N - 1; ++y)
    for (int z = 1; z < N - 1; ++z) {
        const auto& v = rb.voxels()[static_cast<std::size_t>(
            rb.lattice().index(x, y, z))];
        ftd::Vec3 face, edge;
        for (const auto& o : faces) face += sample(x + o[0], y + o[1], z + o[2]);
        for (const auto& o : edges) edge += sample(x + o[0], y + o[1], z + o[2]);
        const ftd::Vec3 lap = face * (1.0 / 3.0) + edge * (1.0 / 6.0)
                            - sample(x, y, z) * 4.0;
        kinetic += v.wave_vel.mag2();
        cross += v.wave_vel.dot(lap);
        gradient -= v.flux.dot(lap);
    }
    const double c2 = ftd::C_SPEED * ftd::C_SPEED;
    return 0.5 * kinetic + 0.5 * c2 * cross + 0.5 * c2 * gradient;
}

// Kick-drift Hamiltonian INCLUDING the de Broglie clock term at manifested
// sites: the acceleration is a = c²·L18(J) − ω₀²·[s≠0]·J, so the modified
// Hamiltonian is ½Σ|W|² + ½Σ W·a + V with V = −½c²ΣJ·L18(J) + ½ω₀²Σ_m|J|².
// Reduces to the periodic wave Hamiltonian when no site is manifested.
double kg_hamiltonian(const ftd::RenderBridge& rb) {
    const int N = rb.lattice().size();
    const double c2 = ftd::C_SPEED * ftd::C_SPEED;
    const double w2 = rb.toggles.de_broglie_clock
        ? rb.toggles.omega0 * rb.toggles.omega0 : 0.0;
    double kinetic = 0.0, cross = 0.0, potential = 0.0;
    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        const int idx = rb.lattice().index(x, y, z);
        const auto& v = rb.voxels()[static_cast<std::size_t>(idx)];
        const ftd::Vec3 lap = rb.laplacian_flux(idx);
        ftd::Vec3 accel = lap * c2;
        if (v.state != 0) accel -= v.flux * w2;
        kinetic += v.wave_vel.mag2();
        cross += v.wave_vel.dot(accel);
        potential += -c2 * v.flux.dot(lap);
        if (v.state != 0) potential += w2 * v.flux.mag2();
    }
    return 0.5 * kinetic + 0.5 * cross + 0.5 * potential;
}

struct MembraneTrace {
    double H0 = 0.0;
    double drift_max = 0.0;
    double region_U0 = 0.0;
    double region_U_end = 0.0;
    double region_fill = 0.0;
    double leak_integral = 0.0;
};

// Holds a membrane cell for `ticks` ticks, tracking the KG Hamiltonian and
// the inner-region energy (U_E + U_B).
MembraneTrace run_membrane_hold(ftd::RenderBridge& rb, const ftd::FluxCellRegion& region,
                                int ticks, const char* tag) {
    MembraneTrace tr;
    const ftd::FluxCellLedger first = ftd::compute_flux_cell_ledger(rb, region);
    tr.H0 = kg_hamiltonian(rb);
    tr.region_U0 = first.U_E + first.U_B;
    tr.region_fill = static_cast<double>(first.site_count)
                   / static_cast<double>(rb.lattice().total_sites());
    const std::string prefix(tag);
    ftd::test::metric((prefix + ".H0").c_str(), tr.H0, 0);
    ftd::test::metric((prefix + ".region_U0").c_str(), tr.region_U0, 0);
    for (int t = 1; t <= ticks; ++t) {
        rb.tick();
        const ftd::FluxCellLedger reg = ftd::compute_flux_cell_ledger(rb, region);
        tr.leak_integral += reg.P_leak;
        if (t % SAMPLE_EVERY == 0 || t == ticks) {
            const double H = kg_hamiltonian(rb);
            tr.drift_max = std::max(tr.drift_max,
                                    std::fabs(H - tr.H0) / std::max(1e-300, std::fabs(tr.H0)));
        }
        if (t % 50 == 0 || t == ticks)
            ftd::test::metric((prefix + ".region_U").c_str(), reg.U_E + reg.U_B, t);
        if (t == ticks) tr.region_U_end = reg.U_E + reg.U_B;
    }
    ftd::test::metric((prefix + ".drift_max").c_str(), tr.drift_max, ticks);
    ftd::test::metric((prefix + ".retention").c_str(),
                      tr.region_U0 > 0.0 ? tr.region_U_end / tr.region_U0 : 0.0, ticks);
    ftd::test::metric((prefix + ".region_fill_fraction").c_str(), tr.region_fill, ticks);
    ftd::test::metric((prefix + ".leak_integral").c_str(), tr.leak_integral, ticks);
    return tr;
}

// Fresh membrane bridge with a shell of the given thickness and the canonical
// ring inside it, on the membrane profile (bare wave map + clock at omega0).
std::unique_ptr<ftd::RenderBridge> make_membrane_bridge(double thickness, double omega0,
                                                        ftd::FluxCellMembraneSpec* shell_out,
                                                        bool seed_ring = true) {
    auto rb = std::make_unique<ftd::RenderBridge>(L);
    rb->force_cpu();
    for (const auto& spec : ftd::TOGGLE_SPECS) rb->toggles.*(spec.field) = false;
    rb->toggles.wave_propagation = true;
    rb->toggles.de_broglie_clock = true;
    rb->toggles.omega0 = omega0;
    rb->toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    // Keep the ring identical across thicknesses: size it from the thickest
    // shell so only the wall changes between runs.
    const ftd::FluxCellMembraneSpec reference = ftd::default_flux_cell_membrane_spec(L, 3.0);
    ftd::FluxCellMembraneSpec shell = reference;
    shell.thickness = thickness;
    shell.inner_radius = reference.inner_radius + (3.0 - thickness);
    ftd::seed_flux_cell_membrane(*rb, shell);
    if (seed_ring)
        ftd::seed_flux_cell_torus(*rb, ftd::flux_cell_membrane_ring_spec(reference), 1.0);
    if (shell_out) *shell_out = shell;
    return rb;
}

// Angular flow moment of the Poynting field about a centre over a region:
// L_S = sum (r - c) x S. Observer-level circulation measure only.
ftd::Vec3 angular_flow_moment(const ftd::RenderBridge& rb, const ftd::FluxCellRegion& region) {
    const int N = rb.lattice().size();
    ftd::Vec3 out;
    auto pdelta = [N](double a, double b) {
        double d = a - b;
        while (d >  0.5 * N) d -= N;
        while (d < -0.5 * N) d += N;
        return d;
    };
    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        const double dx = pdelta(x, region.cx), dy = pdelta(y, region.cy), dz = pdelta(z, region.cz);
        if (dx * dx + dy * dy + dz * dz > region.radius * region.radius) continue;
        const ftd::Vec3 S = rb.poynting_vector(rb.lattice().index(x, y, z));
        out += ftd::Vec3(dy * S.z - dz * S.y, dz * S.x - dx * S.z, dx * S.y - dy * S.x);
    }
    return out;
}

struct HoldTrace {
    double H0 = 0.0;
    double drift_max = 0.0;          // max |H - H0| / H0 over samples
    double balance_first = 0.0;      // (U_E - U_B)/(U_E + U_B) at tick 0
    double balance_max = -1.0;       // max over samples
    double ue_frac_max = 0.0;        // max U_E / (U_E + U_B)
    double poynting_ratio_max = 0.0; // max |sum S| / sum |S|
    double region_U0 = 0.0;          // U_E + U_B in the region at tick 0
    double region_U_end = 0.0;       // same at the last tick
    double region_fill = 0.0;        // region sites / box sites (uniform-fill reference)
    double leak_integral = 0.0;      // sum over ticks of P_leak (region faces)
    double support_end = 0.0;        // support radius at the last tick
    double gamma0 = 0.0, gamma_end = 0.0;
    double phi_b0 = 0.0;
};

HoldTrace run_hold(ftd::RenderBridge& rb, const ftd::FluxCellRegion& region,
                   const ftd::FluxCellTorusSpec& ring, const char* tag,
                   bool use_reflective_hamiltonian = false) {
    HoldTrace tr;
    const auto box = whole_box();
    auto hamiltonian = [&]() {
        return use_reflective_hamiltonian
            ? reflective_interior_hamiltonian(rb)
            : ftd::compute_flux_cell_ledger(rb, box).H_wave;
    };
    ftd::FluxCellLedger first = ftd::compute_flux_cell_ledger(rb, box);
    ftd::FluxCellLedger first_region = ftd::compute_flux_cell_ledger(rb, region);
    tr.H0 = hamiltonian();
    tr.balance_first = ftd::flux_cell_eb_balance(first);
    tr.balance_max = tr.balance_first;
    tr.region_U0 = first_region.U_E + first_region.U_B;
    tr.region_fill = static_cast<double>(first_region.site_count)
                   / static_cast<double>(rb.lattice().total_sites());
    tr.gamma0 = ftd::flux_cell_ring_circulation(rb, ring.cx, ring.cy, ring.cz,
                                                ring.major_radius);
    tr.phi_b0 = ftd::flux_cell_disk_magnetic_flux(rb, ring.cx, ring.cy, ring.cz,
                                                  ring.major_radius);
    std::string prefix(tag);
    ftd::test::metric((prefix + ".H0").c_str(), tr.H0, 0);
    ftd::test::metric((prefix + ".U_E0").c_str(), first.U_E, 0);
    ftd::test::metric((prefix + ".U_B0").c_str(), first.U_B, 0);
    ftd::test::metric((prefix + ".U_J0").c_str(), first.U_J, 0);
    ftd::test::metric((prefix + ".gamma0").c_str(), tr.gamma0, 0);
    ftd::test::metric((prefix + ".phi_B0").c_str(), tr.phi_b0, 0);

    for (int t = 1; t <= HOLD_TICKS; ++t) {
        rb.tick();
        const ftd::FluxCellLedger reg = ftd::compute_flux_cell_ledger(rb, region);
        tr.leak_integral += reg.P_leak;
        if (t % SAMPLE_EVERY != 0 && t != HOLD_TICKS) continue;
        const ftd::FluxCellLedger all = ftd::compute_flux_cell_ledger(rb, box);
        const double H = hamiltonian();
        tr.drift_max = std::max(tr.drift_max, std::fabs(H - tr.H0) / std::fabs(tr.H0));
        const double bal = ftd::flux_cell_eb_balance(all);
        tr.balance_max = std::max(tr.balance_max, bal);
        const double em = all.U_E + all.U_B;
        if (em > 0.0) tr.ue_frac_max = std::max(tr.ue_frac_max, all.U_E / em);
        if (all.S_abs_total > 0.0)
            tr.poynting_ratio_max = std::max(
                tr.poynting_ratio_max, all.S_total.mag() / all.S_abs_total);
        if (t == HOLD_TICKS) {
            tr.region_U_end = reg.U_E + reg.U_B;
            tr.support_end = all.support_radius;
            tr.gamma_end = ftd::flux_cell_ring_circulation(
                rb, ring.cx, ring.cy, ring.cz, ring.major_radius);
        }
        if (t % 50 == 0 || t == HOLD_TICKS) {
            ftd::test::metric((prefix + ".H").c_str(), H, t);
            ftd::test::metric((prefix + ".balance").c_str(), bal, t);
            ftd::test::metric((prefix + ".region_U").c_str(), reg.U_E + reg.U_B, t);
        }
    }
    ftd::test::metric((prefix + ".drift_max").c_str(), tr.drift_max, HOLD_TICKS);
    ftd::test::metric((prefix + ".balance_max").c_str(), tr.balance_max, HOLD_TICKS);
    ftd::test::metric((prefix + ".ue_frac_max").c_str(), tr.ue_frac_max, HOLD_TICKS);
    ftd::test::metric((prefix + ".poynting_ratio_max").c_str(), tr.poynting_ratio_max, HOLD_TICKS);
    ftd::test::metric((prefix + ".retention").c_str(),
                      tr.region_U0 > 0.0 ? tr.region_U_end / tr.region_U0 : 0.0, HOLD_TICKS);
    ftd::test::metric((prefix + ".region_fill_fraction").c_str(), tr.region_fill, HOLD_TICKS);
    ftd::test::metric((prefix + ".leak_integral").c_str(), tr.leak_integral, HOLD_TICKS);
    ftd::test::metric((prefix + ".support_end").c_str(), tr.support_end, HOLD_TICKS);
    ftd::test::metric((prefix + ".gamma_end").c_str(), tr.gamma_end, HOLD_TICKS);
    return tr;
}

}  // namespace

int main() {
    ftd::test::init("test_flux_cell_scenario_physics");
    const ftd::FluxCellTorusSpec ring = ftd::default_flux_cell_torus_spec(L);

    // ── V0: Gauss-charged flux capacitor ───────────────────────────────
    ftd::test::section("s0-cell-capacitor: Gauss-charged flux gap");
    {
        auto rb = make("s0-cell-capacitor");
        const int gap_half = std::max(2, L / 8);
        const int plate_half = std::max(1, L / 8);
        const int plate_sites = (2 * plate_half + 1) * (2 * plate_half + 1);
        const ftd::FluxCellRegion gap{mid(), mid(), mid(), gap_half - 0.5};

        const ftd::EnergyAudit audit0 = rb->energy_audit();
        ftd::test::check("plates are net neutral", audit0.charge_total == 0);
        ftd::test::check("both plates manifested",
                         audit0.manifested_count == 2 * plate_sites);
        const ftd::FluxCellLedger gap0 = ftd::compute_flux_cell_ledger(*rb, gap);
        ftd::test::check("gap U_J is exactly zero before the first tick", gap0.U_J == 0.0);
        ftd::test::check("gap U_E is exactly zero before the first tick", gap0.U_E == 0.0);

        rb->tick();
        const ftd::FluxCellLedger gap1 = ftd::compute_flux_cell_ledger(*rb, gap);
        const ftd::EnergyAudit audit1 = rb->energy_audit();
        ftd::test::metric("capacitor.gap_U_J", gap1.U_J, 1);
        ftd::test::metric("capacitor.max_gauss_error", audit1.max_gauss_error, 1);
        ftd::test::metric("capacitor.C_eff",
                          gap1.U_J > 0.0
                              ? static_cast<double>(plate_sites) * plate_sites / (2.0 * gap1.U_J)
                              : 0.0, 1);
        ftd::test::check("gap U_J is positive after the first Gauss projection",
                         gap1.U_J > 0.0);
        ftd::test::check("gap flux points from + plate to - plate (+x)",
                         gap1.J_total.x > 0.0
                         && std::fabs(gap1.J_total.y) < 1e-9 * gap1.J_total.x
                         && std::fabs(gap1.J_total.z) < 1e-9 * gap1.J_total.x);

        double gap_min = gap1.U_J, gap_max = gap1.U_J;
        for (int t = 2; t <= HOLD_TICKS; ++t) {
            rb->tick();
            const ftd::FluxCellLedger g = ftd::compute_flux_cell_ledger(*rb, gap);
            gap_min = std::min(gap_min, g.U_J);
            gap_max = std::max(gap_max, g.U_J);
            if (t % 50 == 0) ftd::test::metric("capacitor.gap_U_J", g.U_J, t);
        }
        const ftd::FluxCellLedger gap_end = ftd::compute_flux_cell_ledger(*rb, gap);
        const ftd::EnergyAudit audit_end = rb->energy_audit();
        ftd::test::metric("capacitor.gap_U_J_min", gap_min, HOLD_TICKS);
        ftd::test::metric("capacitor.gap_U_J_max", gap_max, HOLD_TICKS);
        ftd::test::metric("capacitor.retention", gap_end.U_J / gap1.U_J, HOLD_TICKS);
        ftd::test::metric("capacitor.max_gauss_error", audit_end.max_gauss_error, HOLD_TICKS);
        ftd::test::check("gap U_J stays positive through the hold", gap_min > 0.0);
        ftd::test::check_close("pinned: gap U_J after the first projection",
                               gap1.U_J, PIN_CAPACITOR_GAP_UJ_T1, PIN_REL * PIN_CAPACITOR_GAP_UJ_T1);
        ftd::test::check_close("pinned: gap retention after 300 ticks (rings, then relaxes)",
                               gap_end.U_J / gap1.U_J, PIN_CAPACITOR_RETENTION, PIN_ABS);
        ftd::test::check("Gauss residual relaxes below the end-of-hold bound",
                         audit_end.max_gauss_error < PIN_CAPACITOR_GAUSS_END);
        ftd::test::check("plates remain locked, neutral, and unmoved",
                         audit_end.charge_total == 0
                         && audit_end.manifested_count == 2 * plate_sites);
    }

    // ── V1: ring reservoir on the periodic free wave map ───────────────
    ftd::test::section("s0-cell-torus: periodic free-wave hold");
    HoldTrace base;
    double base_UJ0 = 0.0;
    {
        auto rb = make("s0-cell-torus");
        const ftd::FluxCellLedger l0 = ftd::compute_flux_cell_ledger(*rb, whole_box());
        base_UJ0 = l0.U_J;
        ftd::test::check("ring starts with zero electric energy", l0.U_E == 0.0);
        ftd::test::check("ring starts with positive magnetic energy", l0.U_B > 0.0);
        base = run_hold(*rb, ring_region(ring), ring, "torus");
        ftd::test::check("ring circulation is positive", base.gamma0 > 0.0);
        ftd::test::check("disk magnetic flux is positive", base.phi_b0 > 0.0);
        ftd::test::check("balance starts fully magnetic (-1)", base.balance_first == -1.0);
        ftd::test::check("kick-drift Hamiltonian conserved over the hold",
                         base.drift_max < HAMILTONIAN_DRIFT_GATE);
        ftd::test::check("electric channel receives at least 25% of U_E+U_B",
                         base.ue_frac_max >= EXCHANGE_GATE);
        ftd::test::check("net Poynting flow vanishes by symmetry",
                         base.poynting_ratio_max < SYMMETRY_GATE);
        const double retention = base.region_U_end / base.region_U0;
        ftd::test::check_close("pinned: peak electric share of U_E+U_B",
                               base.ue_frac_max, PIN_TORUS_UE_FRAC_MAX, PIN_ABS);
        ftd::test::check_close("pinned: ring-region retention after 300 ticks",
                               retention, PIN_TORUS_RETENTION, PIN_ABS);
        ftd::test::check_close("ring disperses to the uniform-fill fraction of its region",
                               retention, base.region_fill, PIN_ABS);
    }

    ftd::test::section("s0-cell-torus-reverse: opposite-circulation control");
    {
        auto rb = make("s0-cell-torus-reverse");
        const ftd::FluxCellLedger l0 = ftd::compute_flux_cell_ledger(*rb, whole_box());
        const double gamma = ftd::flux_cell_ring_circulation(
            *rb, ring.cx, ring.cy, ring.cz, ring.major_radius);
        const double phi_b = ftd::flux_cell_disk_magnetic_flux(
            *rb, ring.cx, ring.cy, ring.cz, ring.major_radius);
        ftd::test::check_close("reverse ring has the same Hamiltonian",
                               l0.H_wave, base.H0, MIRROR_GATE * std::fabs(base.H0));
        ftd::test::check_close("reverse ring has the same U_J",
                               l0.U_J, base_UJ0, MIRROR_GATE * base_UJ0);
        ftd::test::check_close("reverse circulation is equal and opposite",
                               gamma, -base.gamma0, MIRROR_GATE * std::fabs(base.gamma0));
        ftd::test::check_close("reverse disk flux is equal and opposite",
                               phi_b, -base.phi_b0, MIRROR_GATE * std::fabs(base.phi_b0));
        const HoldTrace tr = run_hold(*rb, ring_region(ring), ring, "torus_reverse");
        ftd::test::check("reverse ring conserves the Hamiltonian",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
    }

    ftd::test::section("s0-cell-torus-scrambled: zero-circulation control");
    {
        auto rb = make("s0-cell-torus-scrambled");
        const ftd::FluxCellLedger l0 = ftd::compute_flux_cell_ledger(*rb, whole_box());
        const double gamma = ftd::flux_cell_ring_circulation(
            *rb, ring.cx, ring.cy, ring.cz, ring.major_radius);
        ftd::test::metric("torus_scrambled.gamma0", gamma, 0);
        ftd::test::check_close("scrambled ring has identical pointwise U_J",
                               l0.U_J, base_UJ0, 1e-15 * base_UJ0);
        ftd::test::check("scrambled ring circulation vanishes",
                         std::fabs(gamma) < 1e-9 * std::fabs(base.gamma0));
        ftd::test::check("scrambled ring stores more gradient energy than the coherent ring",
                         l0.H_wave > base.H0);
        const HoldTrace tr = run_hold(*rb, ring_region(ring), ring, "torus_scrambled");
        ftd::test::check("scrambled ring conserves the Hamiltonian",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        const double retention = tr.region_U_end / tr.region_U0;
        const double vs_coherent = retention / (base.region_U_end / base.region_U0);
        ftd::test::metric("torus_scrambled.retention_vs_coherent", vs_coherent, HOLD_TICKS);
        ftd::test::check_close("pinned: scrambled ring-region retention after 300 ticks",
                               retention, PIN_SCRAMBLED_RETENTION, PIN_ABS);
        // Contrary to the naive expectation, the sign-scrambled ring holds
        // MORE energy in its region: the quadrant sign flips inject zone-edge
        // content whose lattice group velocity is small, while the coherent
        // ring's long-wavelength content leaves at ~C_SPEED.
        ftd::test::check("scrambled ring retains more than the coherent ring (zone-edge content)",
                         vs_coherent > PIN_SCRAMBLED_OVER_COHERENT_MIN);
    }

    ftd::test::section("s0-cell-torus-open: dispersal-boundary loss control");
    {
        auto rb = make("s0-cell-torus-open");
        ftd::test::check("open box uses the Dispersal boundary law",
                         rb->toggles.flux_boundary == ftd::FluxBoundaryMode::Dispersal);
        const double H0 = ftd::compute_flux_cell_ledger(*rb, whole_box()).H_wave;
        for (int t = 1; t <= HOLD_TICKS; ++t) {
            rb->tick();
            if (t % 50 == 0)
                ftd::test::metric("torus_open.H",
                                  ftd::compute_flux_cell_ledger(*rb, whole_box()).H_wave, t);
        }
        const double H_end = ftd::compute_flux_cell_ledger(*rb, whole_box()).H_wave;
        ftd::test::metric("torus_open.H_end_over_H0", H_end / H0, HOLD_TICKS);
        ftd::test::check("open box loses more than 99.9% of its Hamiltonian",
                         H_end < OPEN_LOSS_GATE * H0);
    }

    ftd::test::section("s0-cell-torus-walled: reflective-box hold");
    {
        auto rb = make("s0-cell-torus-walled");
        ftd::test::check("walled box uses the Reflective boundary law",
                         rb->toggles.flux_boundary == ftd::FluxBoundaryMode::Reflective);
        const HoldTrace tr = run_hold(*rb, ring_region(ring), ring, "torus_walled", true);
        const double H_end = reflective_interior_hamiltonian(*rb);
        ftd::test::metric("torus_walled.H_end_over_H0", H_end / tr.H0, HOLD_TICKS);
        ftd::test::check("walled box conserves its interior Hamiltonian",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        ftd::test::check("walled box net Poynting flow vanishes by symmetry",
                         tr.poynting_ratio_max < SYMMETRY_GATE);
    }

    // ── V2: three-axis standing arms ───────────────────────────────────
    ftd::test::section("s0-cell-triad: zero-momentum standing arms");
    {
        auto rb = make("s0-cell-triad");
        const ftd::FluxCellLedger l0 = ftd::compute_flux_cell_ledger(*rb, whole_box());
        const double trace = l0.dyad[0][0] + l0.dyad[1][1] + l0.dyad[2][2];
        double off_max = 0.0, diag_spread = 0.0;
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
                if (a != b) off_max = std::max(off_max, std::fabs(l0.dyad[a][b]));
        for (int a = 0; a < 3; ++a)
            diag_spread = std::max(diag_spread,
                                   std::fabs(l0.dyad[a][a] - trace / 3.0));
        ftd::test::metric("triad.dyad_trace", trace, 0);
        ftd::test::metric("triad.dyad_offdiag_max_over_trace", off_max / trace, 0);
        ftd::test::metric("triad.dyad_diag_spread_over_trace", diag_spread / trace, 0);
        ftd::test::check("triad stores positive energy", l0.H_wave > 0.0 && trace > 0.0);
        ftd::test::check("triad starts with nonzero electric and magnetic channels",
                         l0.U_E > 0.0 && l0.U_B > 0.0);
        ftd::test::check("flux dyad is isotropic (equal diagonals)",
                         diag_spread < SYMMETRY_GATE * trace);
        // The three arms overlap at the centre, so the dyad carries genuine
        // off-diagonal overlap terms (D_c psi_a * D_c psi_b products are even
        // in every coordinate). Measured 1.9% of the trace; pinned as a bound.
        ftd::test::check("flux dyad off-diagonal overlap terms stay below 3% of the trace",
                         off_max < PIN_TRIAD_OFFDIAG_MAX * trace);
        ftd::test::check("net flux vector vanishes",
                         l0.J_total.mag() < SYMMETRY_GATE * std::sqrt(trace));
        ftd::test::check("net Poynting flow vanishes at tick 0",
                         l0.S_abs_total > 0.0
                         && l0.S_total.mag() < SYMMETRY_GATE * l0.S_abs_total);
        const ftd::FluxCellRegion arms{mid(), mid(), mid(),
                                       std::max(3.0, L / 6.0) * 2.0};
        const HoldTrace tr = run_hold(*rb, arms, ring, "triad");
        ftd::test::check("triad conserves the Hamiltonian",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        ftd::test::check("triad net Poynting flow vanishes through the hold",
                         tr.poynting_ratio_max < SYMMETRY_GATE);
        const double retention = tr.region_U_end / tr.region_U0;
        ftd::test::check_close("pinned: arm-region retention after 300 ticks",
                               retention, PIN_TRIAD_RETENTION, PIN_ABS);
        ftd::test::check_close("arms disperse to the uniform-fill fraction of their region",
                               retention, tr.region_fill, PIN_ABS);
    }

    // ── Phase 2/3: pump for N ticks, disconnect, hold ──────────────────
    ftd::test::section("pump-off control: charge the ring dynamically, disconnect, hold");
    {
        auto rb = make("s0-cell-torus");
        rb->clearField();
        const auto box = whole_box();
        ftd::test::check("cleared cell starts at zero Hamiltonian",
                         ftd::compute_flux_cell_ledger(*rb, box).H_wave == 0.0);
        double W_in = 0.0;
        for (int n = 0; n < PUMP_TICKS; ++n) {
            rb->tick();
            const double before = ftd::compute_flux_cell_ledger(*rb, box).H_wave;
            ftd::seed_flux_cell_torus(*rb, ring, 1.0 / PUMP_TICKS);
            const double after = ftd::compute_flux_cell_ledger(*rb, box).H_wave;
            W_in += after - before;
        }
        const double H_disconnect = ftd::compute_flux_cell_ledger(*rb, box).H_wave;
        ftd::test::metric("pump.W_in", W_in, PUMP_TICKS);
        ftd::test::metric("pump.H_at_disconnect", H_disconnect, PUMP_TICKS);
        ftd::test::check("pump work is positive", W_in > 0.0);

        double drift_max = 0.0;
        for (int t = 1; t <= HOLD_TICKS; ++t) {
            rb->tick();
            if (t % SAMPLE_EVERY != 0 && t != HOLD_TICKS) continue;
            const double H = ftd::compute_flux_cell_ledger(*rb, box).H_wave;
            drift_max = std::max(drift_max, std::fabs(H - H_disconnect) / H_disconnect);
            if (t % 100 == 0) ftd::test::metric("pump.H_hold", H, PUMP_TICKS + t);
        }
        const double H_end = ftd::compute_flux_cell_ledger(*rb, box).H_wave;
        ftd::test::metric("pump.hold_drift_max", drift_max, PUMP_TICKS + HOLD_TICKS);
        ftd::test::check("held energy stays constant after disconnection",
                         drift_max < HAMILTONIAN_DRIFT_GATE);
        ftd::test::check("injected work equals the held Hamiltonian (ledger closes)",
                         std::fabs(W_in - H_end) < PUMP_LEDGER_GATE * H_end);
    }

    // ── Membrane: locked clocked shell as a mass-gap wall ──────────────
    ftd::test::section("s0-cell-torus-membrane: clocked shell membrane hold");
    double membrane_retention = 0.0;
    double membrane_fill = 0.0;
    {
        auto rb = make("s0-cell-torus-membrane");
        ftd::test::check("membrane profile enables the de Broglie clock at omega0 = 1",
                         rb->toggles.de_broglie_clock && rb->toggles.omega0 == 1.0);
        const ftd::FluxCellRegion inner = rb->flux_cell_region();
        ftd::test::check("membrane scenario registered its inner region", inner.radius > 0.0);
        const ftd::EnergyAudit a0 = rb->energy_audit();
        const ftd::FluxCellLedger l0 = ftd::compute_flux_cell_ledger(*rb, inner);
        ftd::test::check("audit carries the regional ledger", a0.cell_site_count == l0.site_count
                         && l0.site_count > 0);
        ftd::test::check_close("audit cell U_B matches the ledger", a0.cell_U_B, l0.U_B,
                               1e-12 * std::max(1.0, l0.U_B));
        ftd::test::check_close("audit cell H matches the ledger", a0.cell_H_wave, l0.H_wave,
                               1e-12 * std::max(1.0, std::fabs(l0.H_wave)));
        // Alternating (x+y+z) parity on a spherical shell leaves a small parity
        // imbalance; the shell is near-neutral, and with Gauss projection off
        // its charge plays no role in the dynamics.
        ftd::test::check("shell is near-neutral (parity imbalance below 5% of its sites)",
                         std::abs(a0.charge_total) < 0.05 * a0.manifested_count);
        ftd::test::check("shell is manifested", a0.manifested_count > 0);
        ftd::test::metric("membrane.shell_sites", a0.manifested_count, 0);
        ftd::test::metric("membrane.shell_charge", a0.charge_total, 0);
        const MembraneTrace tr = run_membrane_hold(*rb, inner, HOLD_TICKS, "membrane");
        membrane_retention = tr.region_U_end / tr.region_U0;
        membrane_fill = tr.region_fill;
        ftd::test::check("KG Hamiltonian (wave + clock) conserved in the membrane",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        ftd::test::check("membrane retains at least three times the uniform-fill fraction",
                         membrane_retention > 3.0 * tr.region_fill);
        ftd::test::check("membrane retention exceeds the bare-map ring retention",
                         membrane_retention > base.region_U_end / base.region_U0);
        ftd::test::check_close("pinned: membrane inner-ball retention",
                               membrane_retention, PIN_MEMBRANE_RETENTION, PIN_ABS);
        const ftd::EnergyAudit a_end = rb->energy_audit();
        ftd::test::check("shell survives the hold intact",
                         a_end.manifested_count == a0.manifested_count
                         && a_end.charge_total == a0.charge_total);
    }

    ftd::test::section("membrane transparent control: omega0 -> 0.05");
    {
        auto rb = make("s0-cell-torus-membrane");
        rb->toggles.omega0 = 0.05;  // mass gap far below the ring's mode content
        const ftd::FluxCellRegion inner = rb->flux_cell_region();
        const MembraneTrace tr = run_membrane_hold(*rb, inner, HOLD_TICKS, "membrane_transparent");
        const double retention = tr.region_U_end / tr.region_U0;
        ftd::test::check("transparent shell still conserves the KG Hamiltonian",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        ftd::test::check("transparent shell loses most of the membrane's retention",
                         retention < 0.5 * membrane_retention);
        ftd::test::check_close("transparent shell relaxes toward uniform fill",
                               retention, tr.region_fill, 0.15);
        ftd::test::check_close("pinned: transparent-shell retention",
                               retention, PIN_MEMBRANE_TRANSPARENT, PIN_ABS);
    }

    ftd::test::section("membrane thickness scaling: evanescent wall");
    {
        double retention_by_t[3] = {0.0, 0.0, 0.0};
        for (int t = 1; t <= 3; ++t) {
            ftd::FluxCellMembraneSpec shell;
            auto rb = make_membrane_bridge(static_cast<double>(t), 1.0, &shell);
            const ftd::FluxCellRegion inner{shell.cx, shell.cy, shell.cz, shell.inner_radius - 0.5};
            const std::string tag = "membrane_t" + std::to_string(t);
            const MembraneTrace tr = run_membrane_hold(*rb, inner, HOLD_TICKS, tag.c_str());
            retention_by_t[t - 1] = tr.region_U_end / tr.region_U0;
            ftd::test::check((tag + " conserves the KG Hamiltonian").c_str(),
                             tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        }
        ftd::test::check("retention rises monotonically with shell thickness (evanescent decay)",
                         retention_by_t[0] < retention_by_t[1] && retention_by_t[1] < retention_by_t[2]);
        ftd::test::check_close("pinned: 1-cell shell retention", retention_by_t[0],
                               PIN_MEMBRANE_RETENTION_T1, PIN_ABS);
        ftd::test::check_close("pinned: 2-cell shell retention", retention_by_t[1],
                               PIN_MEMBRANE_RETENTION_T2, PIN_ABS);
        ftd::test::metric("membrane.retention_t1", retention_by_t[0], HOLD_TICKS);
        ftd::test::metric("membrane.retention_t2", retention_by_t[1], HOLD_TICKS);
        ftd::test::metric("membrane.retention_t3", retention_by_t[2], HOLD_TICKS);
    }

    // ── Port: scheduled aperture, Phase-4 discharge ledger ─────────────
    ftd::test::section("s0-cell-torus-membrane-gated: scheduled discharge port");
    {
        auto rb = make("s0-cell-torus-membrane-gated");
        ftd::test::check("gated profile arms flux_cell_port", rb->toggles.flux_cell_port);
        ftd::test::check("port configured and closed at tick 0",
                         rb->flux_cell_port_configured() && !rb->flux_cell_port_open()
                         && rb->flux_cell_port().open_tick == 150);
        const ftd::FluxCellRegion inner = rb->flux_cell_region();
        // Whole-cell ledger region: everything up to the port's accounting
        // surface (the plug's outer face, r = inner + thickness - 0.5), so the
        // only exit is the hole; H_kg includes the clock potential at shell
        // sites, which is where the ring's evanescent tails keep their energy.
        const ftd::FluxCellMembraneSpec shell = ftd::default_flux_cell_membrane_spec(L, 3.0);
        const ftd::FluxCellRegion cell{shell.cx, shell.cy, shell.cz,
                                       shell.inner_radius + shell.thickness - 0.5};
        const double Hcell_start = ftd::compute_flux_cell_ledger(*rb, cell).H_kg;
        ftd::test::metric("gated.cell_H_start", Hcell_start, 0);
        const int open_tick = rb->flux_cell_port().open_tick;
        double U_at_open = 0.0, Ucell_at_open = 0.0, H_at_open = 0.0, Hcell_at_open = 0.0;
        bool work_zero_before = true;
        for (int t = 1; t <= HOLD_TICKS; ++t) {
            rb->tick();
            if (t < open_tick && rb->flux_cell_port_work_out() != 0.0) work_zero_before = false;
            if (t == open_tick) {
                const ftd::FluxCellLedger l = ftd::compute_flux_cell_ledger(*rb, inner);
                const ftd::FluxCellLedger lc = ftd::compute_flux_cell_ledger(*rb, cell);
                U_at_open = l.U_E + l.U_B;
                H_at_open = l.H_wave;
                Ucell_at_open = lc.U_E + lc.U_B;
                Hcell_at_open = lc.H_kg;
                ftd::test::metric("gated.cell_U_at_open", Ucell_at_open, t);
                ftd::test::metric("gated.cell_H_at_open", Hcell_at_open, t);
                ftd::test::metric("gated.region_H_at_open", H_at_open, t);
                ftd::test::check("port opens on its scheduled tick", rb->flux_cell_port_open());
                ftd::test::check("opening expired shell sites", rb->flux_cell_port_site_count() > 0);
                ftd::test::check("opened plug has a one-layer flux surface",
                                 rb->flux_cell_port_surface_count() > 0
                                 && rb->flux_cell_port_surface_count() < rb->flux_cell_port_site_count());
                ftd::test::metric("gated.port_sites", rb->flux_cell_port_site_count(), t);
                ftd::test::metric("gated.port_surface_sites", rb->flux_cell_port_surface_count(), t);
                ftd::test::metric("gated.region_U_at_open", U_at_open, t);
            }
            if (t % 50 == 0) {
                const ftd::FluxCellLedger l = ftd::compute_flux_cell_ledger(*rb, inner);
                ftd::test::metric("gated.region_U", l.U_E + l.U_B, t);
                ftd::test::metric("gated.region_H", l.H_wave, t);
                ftd::test::metric("gated.cell_H", ftd::compute_flux_cell_ledger(*rb, cell).H_kg, t);
                ftd::test::metric("gated.port_work_out", rb->flux_cell_port_work_out(), t);
                ftd::test::metric("gated.port_poynting_out", rb->flux_cell_port_poynting_out(), t);
            }
        }
        ftd::test::check("no port flux is booked before the port opens", work_zero_before);
        const ftd::FluxCellLedger l_end = ftd::compute_flux_cell_ledger(*rb, inner);
        const ftd::FluxCellLedger lc_end = ftd::compute_flux_cell_ledger(*rb, cell);
        const double U_end = l_end.U_E + l_end.U_B;
        const double Ucell_end = lc_end.U_E + lc_end.U_B;
        const double W_out = rb->flux_cell_port_work_out();
        ftd::test::metric("gated.delta_U_cell_after_open", Ucell_end - Ucell_at_open, HOLD_TICKS);
        ftd::test::metric("gated.cell_ledger_ratio",
                          (Ucell_at_open - Ucell_end) > 0.0 ? W_out / (Ucell_at_open - Ucell_end) : 0.0,
                          HOLD_TICKS);
        const ftd::EnergyAudit a_end = rb->energy_audit();
        ftd::test::check("audit reports the open port and its work",
                         a_end.cell_port_open == 1 && a_end.cell_port_work_out == W_out);
        const double H_end = l_end.H_wave;
        const double H_loss = H_at_open - H_end;
        const double Hcell_end = lc_end.H_kg;
        const double Hcell_loss = Hcell_at_open - Hcell_end;
        ftd::test::metric("gated.cell_H_closed_phase_drift",
                          (Hcell_at_open - Hcell_start) / Hcell_start, open_tick);
        ftd::test::metric("gated.delta_H_cell_after_open", -Hcell_loss, HOLD_TICKS);
        const double raw_ratio = Hcell_loss > 0.0 ? W_out / Hcell_loss : 0.0;
        ftd::test::metric("gated.ledger_ratio_cell", raw_ratio, HOLD_TICKS);
        // The closed phase measures the wall's own leakage rate (evanescent
        // tails beyond the accounting surface plus discretization); the same
        // rate continues after opening and is not port flux.
        const double closed_drift = (Hcell_start - Hcell_at_open) / Hcell_start;
        const double background = (Hcell_start - Hcell_at_open)
                                * static_cast<double>(HOLD_TICKS - open_tick)
                                / static_cast<double>(open_tick);
        const double corrected_ratio = (Hcell_loss - background) > 0.0
            ? W_out / (Hcell_loss - background) : 0.0;
        ftd::test::metric("gated.ledger_ratio_cell_corrected", corrected_ratio, HOLD_TICKS);
        ftd::test::check("closed phase: cell Hamiltonian nearly constant (wall leakage small)",
                         std::fabs(closed_drift) < PIN_GATED_CLOSED_DRIFT_MAX);
        ftd::test::check("port current accounts for most of the cell's Hamiltonian loss",
                         raw_ratio > PIN_GATED_RAW_RATIO_MIN);
        ftd::test::check_close("port ledger closes after subtracting the closed-phase wall leak",
                               corrected_ratio, PIN_GATED_CORRECTED_RATIO, PIN_GATED_CORRECTED_TOL);
        ftd::test::check_close("pinned: port work W_out", W_out, PIN_GATED_W_OUT,
                               PIN_REL * PIN_GATED_W_OUT);
        ftd::test::metric("gated.W_out", W_out, HOLD_TICKS);
        ftd::test::metric("gated.poynting_out", rb->flux_cell_port_poynting_out(), HOLD_TICKS);
        ftd::test::metric("gated.delta_U_inner_after_open", U_end - U_at_open, HOLD_TICKS);
        ftd::test::metric("gated.delta_H_inner_after_open", -H_loss, HOLD_TICKS);
        ftd::test::metric("gated.ledger_ratio_U", (U_at_open - U_end) > 0.0 ? W_out / (U_at_open - U_end) : 0.0, HOLD_TICKS);
        ftd::test::metric("gated.ledger_ratio_H", H_loss > 0.0 ? W_out / H_loss : 0.0, HOLD_TICKS);
        ftd::test::check("energy leaves through the port after opening", W_out > 0.0);
        ftd::test::check("inner Hamiltonian falls after the port opens", H_end < H_at_open);
    }

    // ── Pump: Phase-2 dynamical charge of an empty membrane ────────────
    ftd::test::section("s0-cell-membrane-pumped: flux-pump charge cycle");
    {
        auto rb = make("s0-cell-membrane-pumped");
        ftd::test::check("pumped profile arms flux_pump", rb->toggles.flux_pump);
        ftd::test::check("pump configured for 20 ticks, none applied",
                         rb->flux_pump_configured() && rb->flux_pump_ticks_total() == 20
                         && rb->flux_pump_ticks_applied() == 0);
        const ftd::FluxCellRegion inner = rb->flux_cell_region();
        ftd::test::check("membrane starts field-free", kg_hamiltonian(*rb) == 0.0);
        for (int t = 1; t <= 20; ++t) rb->tick();
        const double H_disconnect = kg_hamiltonian(*rb);
        const double W_in = rb->flux_pump_work();
        ftd::test::metric("pumped.W_in", W_in, 20);
        ftd::test::metric("pumped.H_at_disconnect", H_disconnect, 20);
        ftd::test::check("pump applied exactly its 20 ticks", rb->flux_pump_ticks_applied() == 20);
        ftd::test::check("engine-booked pump work equals the KG Hamiltonian at disconnection",
                         std::fabs(W_in - H_disconnect) < PUMP_LEDGER_GATE * H_disconnect);
        ftd::test::check_close("pinned: pump work W_in", W_in, PIN_PUMPED_W_IN, PIN_REL * PIN_PUMPED_W_IN);
        const MembraneTrace tr = run_membrane_hold(*rb, inner, HOLD_TICKS, "pumped");
        ftd::test::check("pump stays off after disconnection",
                         rb->flux_pump_ticks_applied() == 20 && rb->flux_pump_work() == W_in);
        ftd::test::check("held energy is constant after disconnection",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
        const ftd::EnergyAudit a_end = rb->energy_audit();
        ftd::test::check("audit reports the pump ledger",
                         a_end.cell_pump_ticks_applied == 20 && a_end.cell_pump_ticks_total == 20
                         && a_end.cell_pump_work == W_in);
        ftd::test::check_close("pinned: pumped inner-ball retention",
                               tr.region_U_end / tr.region_U0, PIN_PUMPED_RETENTION, PIN_ABS);
        ftd::test::metric("pumped.retention_vs_membrane",
                          membrane_retention > 0.0 ? (tr.region_U_end / tr.region_U0) / membrane_retention : 0.0,
                          HOLD_TICKS);
    }

    // ── Transfer: two tangent membranes, port at the contact point ─────
    ftd::test::section("s0-cell-membrane-transfer: receiver ledger (L=49)");
    {
        // Two cells need room: at L=49 each has outer radius 11.5, a three-cell
        // wall, and an 8.5-radius interior.
        constexpr int LT = 49;
        const double midT = (LT - 1) * 0.5;
        auto rb = make_at(LT, "s0-cell-membrane-transfer");
        ftd::test::check("transfer profile arms flux_cell_port", rb->toggles.flux_cell_port);
        const double half_sep = std::max(2.0, std::floor(LT / 4.0));
        const double r_out = half_sep - 0.5;
        const ftd::FluxCellRegion cell_a{midT - half_sep, midT, midT, r_out - 0.5};
        const ftd::FluxCellRegion cell_b = rb->flux_cell_region();
        ftd::test::check("registered region is the receiver on +x",
                         cell_b.cx > midT && cell_b.radius > 0.0);
        const int open_tick = rb->flux_cell_port().open_tick;
        const double HA0 = ftd::compute_flux_cell_ledger(*rb, cell_a).H_kg;
        const double HB0 = ftd::compute_flux_cell_ledger(*rb, cell_b).H_kg;
        ftd::test::metric("transfer.H_A0", HA0, 0);
        ftd::test::metric("transfer.H_B0", HB0, 0);
        ftd::test::check("source holds the energy, receiver starts empty",
                         HA0 > 0.0 && HB0 == 0.0);
        double HA_open = 0.0, HB_open = 0.0;
        bool work_zero_before = true, receiver_quiet_before = true;
        for (int t = 1; t <= HOLD_TICKS; ++t) {
            rb->tick();
            if (t < open_tick) {
                if (rb->flux_cell_port_work_out() != 0.0) work_zero_before = false;
            }
            if (t == open_tick) {
                HA_open = ftd::compute_flux_cell_ledger(*rb, cell_a).H_kg;
                HB_open = ftd::compute_flux_cell_ledger(*rb, cell_b).H_kg;
                if (HB_open > 1e-3 * HA_open) receiver_quiet_before = false;
                ftd::test::check("transfer port opens on schedule",
                                 rb->flux_cell_port_open() && rb->flux_cell_port_site_count() > 0);
                ftd::test::metric("transfer.port_sites", rb->flux_cell_port_site_count(), t);
                ftd::test::metric("transfer.port_surface_sites", rb->flux_cell_port_surface_count(), t);
                ftd::test::metric("transfer.H_A_at_open", HA_open, t);
                ftd::test::metric("transfer.H_B_at_open", HB_open, t);
            }
            if (t % 50 == 0) {
                ftd::test::metric("transfer.H_A", ftd::compute_flux_cell_ledger(*rb, cell_a).H_kg, t);
                ftd::test::metric("transfer.H_B", ftd::compute_flux_cell_ledger(*rb, cell_b).H_kg, t);
                ftd::test::metric("transfer.W_port", rb->flux_cell_port_work_out(), t);
            }
        }
        const double HA_end = ftd::compute_flux_cell_ledger(*rb, cell_a).H_kg;
        const double HB_end = ftd::compute_flux_cell_ledger(*rb, cell_b).H_kg;
        const double W_port = rb->flux_cell_port_work_out();
        const double A_loss = HA_open - HA_end;
        const double B_gain = HB_end - HB_open;
        ftd::test::metric("transfer.W_port", W_port, HOLD_TICKS);
        ftd::test::metric("transfer.A_loss", A_loss, HOLD_TICKS);
        ftd::test::metric("transfer.B_gain", B_gain, HOLD_TICKS);
        ftd::test::metric("transfer.closed_phase_drift_A", (HA0 - HA_open) / HA0, open_tick);
        ftd::test::metric("transfer.B_gain_over_A_loss", A_loss > 0.0 ? B_gain / A_loss : 0.0, HOLD_TICKS);
        ftd::test::metric("transfer.W_port_over_B_gain", B_gain > 0.0 ? W_port / B_gain : 0.0, HOLD_TICKS);
        ftd::test::metric("transfer.B_fraction_of_A0", HB_end / HA0, HOLD_TICKS);
        ftd::test::check("no port flux and a quiet receiver before the port opens",
                         work_zero_before && receiver_quiet_before);
        ftd::test::check("receiver gains energy after the port opens", B_gain > 0.0);
        ftd::test::check("source loses energy after the port opens", A_loss > 0.0);
        ftd::test::check("port current flows from source to receiver", W_port > 0.0);
        ftd::test::check("transfer surface holds the void sites of the contact plane",
                         rb->flux_cell_port_surface_count() > 0);
        // Pinned 2026-09-02 (L=49): closed-phase drift 0.021 over 100 ticks;
        // A loses 0.0530, B gains 0.0137 (0.26 of A's loss, 0.0133 of A's
        // start), W_port = 0.0195 crosses the contact plane (W_port/B_gain 1.42:
        // the channel is two-way and both cells keep leaking at their wall rate).
        ftd::test::check("closed phase: source cell drifts below 5% before the port opens",
                         std::fabs(HA0 - HA_open) / HA0 < 0.05);
        ftd::test::check_close("pinned: receiver gain as a fraction of the source start",
                               HB_end / HA0, 0.0133, 0.005);
        ftd::test::check_close("pinned: port current over receiver gain",
                               B_gain > 0.0 ? W_port / B_gain : 0.0, 1.42, 0.3);
    }

    // ── Electron seeds as flux cells: the spec's section-10 falsifier ──
    ftd::test::section("electron seeds as flux cells: retention, circulation, angular flow");
    {
        struct Probe { const char* id; double radius; };
        const Probe probes[] = {
            {"s0-vacuum-electron", std::max(3.0, L / 6.0) + 2.0},
            {"s0-seed-de-broglie-clock", 6.0},
        };
        for (const Probe& pr : probes) {
            auto rb = make(pr.id);
            const ftd::FluxCellRegion region{mid(), mid(), mid(), pr.radius};
            const std::string tag = std::string("electron.") + pr.id;
            const ftd::FluxCellLedger l0 = ftd::compute_flux_cell_ledger(*rb, region);
            // The electron seed is a curl-free radial J with W=0, so its E+B
            // channels start near zero; the region Hamiltonian (clock term
            // included where the profile has it) is the honest stored energy.
            const double U0 = l0.H_kg;
            const double fill = static_cast<double>(l0.site_count)
                              / static_cast<double>(rb->lattice().total_sites());
            ftd::test::metric((tag + ".H0").c_str(), U0, 0);
            ftd::test::metric((tag + ".UEB0").c_str(), l0.U_E + l0.U_B, 0);
            ftd::test::metric((tag + ".region_fill_fraction").c_str(), fill, 0);
            double poynting_ratio_max = 0.0, LS_max = 0.0, leak = 0.0;
            for (int t = 1; t <= HOLD_TICKS; ++t) {
                rb->tick();
                const ftd::FluxCellLedger l = ftd::compute_flux_cell_ledger(*rb, region);
                leak += l.P_leak;
                if (t % SAMPLE_EVERY != 0 && t != HOLD_TICKS) continue;
                if (l.S_abs_total > 0.0)
                    poynting_ratio_max = std::max(poynting_ratio_max, l.S_total.mag() / l.S_abs_total);
                LS_max = std::max(LS_max, angular_flow_moment(*rb, region).mag());
                if (t % 100 == 0 || t == HOLD_TICKS)
                    ftd::test::metric((tag + ".region_H").c_str(), l.H_kg, t);
            }
            const ftd::FluxCellLedger l_end = ftd::compute_flux_cell_ledger(*rb, region);
            const double retention = U0 > 0.0 ? l_end.H_kg / U0 : 0.0;
            ftd::test::metric((tag + ".retention").c_str(), retention, HOLD_TICKS);
            ftd::test::metric((tag + ".leak_integral").c_str(), leak, HOLD_TICKS);
            ftd::test::metric((tag + ".net_poynting_ratio_max").c_str(), poynting_ratio_max, HOLD_TICKS);
            ftd::test::metric((tag + ".angular_flow_moment_max").c_str(), LS_max, HOLD_TICKS);
            ftd::test::metric((tag + ".support_radius_end").c_str(), l_end.support_radius, HOLD_TICKS);
            ftd::test::check((tag + ": seeded energy is positive").c_str(), U0 > 0.0);
            // Neither seed has a closed wall: the electron template is a
            // curl-free radial J that disperses like every unwalled
            // configuration (retention 0.073 vs fill 0.050), and the clocked
            // 7x7x7 block radiates its k=0 oscillation away (0.046 vs 0.026).
            // Both are closed negatives for a self-confined flux cell; neither
            // carries net Poynting circulation or an angular-flow moment above
            // rounding.
            ftd::test::check_close((tag + ": dressing disperses to uniform fill (closed negative for a self-confined flux cell)").c_str(),
                                   retention, fill, 0.1);
            ftd::test::check((tag + ": no net Poynting circulation or angular-flow moment").c_str(),
                             poynting_ratio_max < 1e-9 && LS_max < 1e-9);
        }
    }

    // ── Resonant pump: scan the increment spacing on the membrane cell ─
    ftd::test::section("resonant pump scan: booked work versus increment spacing");
    double W_every_tick = 0.0, W_best = 0.0;
    int P_best = 1;
    {
        const ftd::FluxCellMembraneSpec reference = ftd::default_flux_cell_membrane_spec(L, 3.0);
        const ftd::FluxCellTorusSpec pump_ring = ftd::flux_cell_membrane_ring_spec(reference);
        const int periods[] = {1, 2, 3, 4, 6, 8, 10, 12, 15, 18, 20, 24};
        double W_by_period_8 = 0.0, W_by_period_3 = 0.0;
        for (const int P : periods) {
            auto rb = make_membrane_bridge(3.0, 1.0, nullptr, /*seed_ring=*/false);
            rb->toggles.flux_pump = true;
            rb->set_flux_pump(pump_ring, 20, P);
            for (int t = 0; t < 20 * P + 2; ++t) rb->tick();
            const double W = rb->flux_pump_work();
            ftd::test::metric("resonant.W_in_by_period", W, P);
            ftd::test::check(("period " + std::to_string(P) + " applied all 20 increments").c_str(),
                             rb->flux_pump_ticks_applied() == 20);
            if (P == 1) W_every_tick = W;
            if (P == 3) W_by_period_3 = W;
            if (P == 8) W_by_period_8 = W;
            if (W > W_best) { W_best = W; P_best = P; }
        }
        ftd::test::metric("resonant.P_best", P_best, 0);
        ftd::test::metric("resonant.W_best_over_W_every_tick",
                          W_every_tick > 0.0 ? W_best / W_every_tick : 0.0, 0);
        ftd::test::check("some spacing delivers more work than pumping every tick",
                         W_best > W_every_tick);
        // Measured 2026-09-02: W(8)/W(1) = 1.37 (constructive), W(3)/W(1) = 0.10
        // (destructive): the booked work depends on the phase the increment
        // meets, exactly the -2 J.L(delta) term.
        ftd::test::check("the 8-tick spacing is the constructive optimum of the scan", P_best == 8);
        ftd::test::check_close("pinned: constructive gain W(8)/W(1)",
                               W_by_period_8 / W_every_tick, 1.368, 0.1);
        ftd::test::check("anti-phase spacing (3 ticks) delivers under 30% of the every-tick work",
                         W_by_period_3 < 0.3 * W_every_tick);
        // Scaling with increment count at the best spacing: fixed per-increment
        // amplitude (1/20 of the ring), N = 5, 10, 20 increments.
        double W_by_N[3] = {0.0, 0.0, 0.0};
        const int counts[3] = {5, 10, 20};
        for (int k = 0; k < 3; ++k) {
            auto rb = make_membrane_bridge(3.0, 1.0, nullptr, /*seed_ring=*/false);
            rb->toggles.flux_pump = true;
            ftd::FluxCellTorusSpec spec = pump_ring;
            spec.amplitude = pump_ring.amplitude * counts[k] / 20.0;  // delta = ring/20 each
            rb->set_flux_pump(spec, counts[k], P_best);
            for (int t = 0; t < counts[k] * P_best + 2; ++t) rb->tick();
            W_by_N[k] = rb->flux_pump_work();
            ftd::test::metric("resonant.W_in_by_count", W_by_N[k], counts[k]);
        }
        const double exponent = (W_by_N[0] > 0.0 && W_by_N[2] > 0.0)
            ? std::log(W_by_N[2] / W_by_N[0]) / std::log(4.0) : 0.0;
        ftd::test::metric("resonant.scaling_exponent_5_to_20", exponent, 0);
        // Measured 1.16: weakly super-linear, far from the coherent N^2 of a
        // closed single-mode oscillator -- the cell is a leaky, multi-mode
        // reservoir and increments partly dephase between arrivals.
        ftd::test::check("stored work grows faster than linearly at the resonant spacing",
                         exponent > 1.0);
        ftd::test::check("but far below the coherent N^2 law (leaky multi-mode reservoir)",
                         exponent < 1.6);
    }

    ftd::test::section("s0-cell-membrane-pumped-resonant: resonant charge cycle");
    {
        auto rb = make("s0-cell-membrane-pumped-resonant");
        ftd::test::check("resonant profile arms flux_pump with a spaced schedule",
                         rb->toggles.flux_pump && rb->flux_pump_period() > 1);
        const int P = rb->flux_pump_period();
        ftd::test::metric("resonant_scenario.period", P, 0);
        const ftd::FluxCellRegion inner = rb->flux_cell_region();
        for (int t = 0; t < 20 * P + 2; ++t) rb->tick();
        const double W_in = rb->flux_pump_work();
        const double H_disconnect = kg_hamiltonian(*rb);
        ftd::test::metric("resonant_scenario.W_in", W_in, 20 * P);
        ftd::test::check("resonant pump applied exactly 20 increments", rb->flux_pump_ticks_applied() == 20);
        ftd::test::check("engine-booked resonant pump work equals the Hamiltonian at disconnection",
                         std::fabs(W_in - H_disconnect) < PUMP_LEDGER_GATE * H_disconnect);
        ftd::test::check("resonant charging books more work than the every-tick pump",
                         W_in > PIN_PUMPED_W_IN * 1.2);
        ftd::test::check_close("pinned: resonant scenario pump work", W_in, 0.067419, PIN_REL * 0.067419);
        const MembraneTrace tr = run_membrane_hold(*rb, inner, HOLD_TICKS, "resonant_scenario");
        ftd::test::check("resonant cell holds its energy after disconnection",
                         tr.drift_max < HAMILTONIAN_DRIFT_GATE);
    }

    return ftd::test::finalize();
}
