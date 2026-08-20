// ============================================================================
// test_one_well_redshift_falling.cpp
// ----------------------------------------------------------------------------
// FTD-1019 / PREREG_ONE_WELL_REDSHIFT_FALLING_v1_1.md
// Lock prefix SHA256:
//   5DB20B6F59BA192F782772D91AB37894295A86FD15A81096D1A44EE4D8F5D0F5
// Anchor: anchored-late until git tag
//   preregister-one-well-redshift-falling-v1-1 resolves.
// v1 (EA504199…B4F6) UNDERDETERMINED on P1; this instrument is the v1.1
// physics execution (P1 threshold 0.9999). Sites and A1/A2 unchanged.
//
// One sourced-then-frozen Poisson well: rest clocks (FC-2) and FTD-1016
// falling. No golden-tick contact. Protocol gates and A1/A2 are CTest
// assertions.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/proper_time_rate.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

namespace ftd {
namespace test {
namespace {

constexpr int    kL         = 32;
constexpr int    kEdge      = 5;
constexpr int    kOx        = 6;
constexpr int    kOy        = 13;
constexpr int    kOz        = 13;
constexpr int    kNx        = 18;
constexpr int    kNy        = 15;
constexpr int    kNz        = 15;
constexpr int    kFx        = 24;
constexpr int    kFy        = 15;
constexpr int    kFz        = 15;
constexpr int    kNTau      = 20;
constexpr double kGextFloor = 1.0e-8;
constexpr double kATol      = 0.05;
constexpr double kExtraForce = 1.0e-12;
constexpr double kFreezeTol = 1.0e-15;

void invoke_phase_forces(RenderBridge& rb) {
    phase_forces_solve_potentials(rb);
    phase_forces_build_color_cache(rb);
    phase_forces_main_loop(rb);
    if (rb.toggles.cluster_inertia)
        phase_forces_integrate_clusters(rb);
}

std::set<int> source_indices(const RenderBridge& rb) {
    std::set<int> out;
    for (int dx = 0; dx < kEdge; ++dx)
        for (int dy = 0; dy < kEdge; ++dy)
            for (int dz = 0; dz < kEdge; ++dz)
                out.insert(rb.lattice().index(kOx + dx, kOy + dy, kOz + dz));
    return out;
}

double source_x_com() {
    return static_cast<double>(kOx) + 0.5 * static_cast<double>(kEdge - 1);
}

void place_source(RenderBridge& rb) {
    for (int dx = 0; dx < kEdge; ++dx)
        for (int dy = 0; dy < kEdge; ++dy)
            for (int dz = 0; dz < kEdge; ++dz) {
                const int x = kOx + dx, y = kOy + dy, z = kOz + dz;
                rb.inject_particle(x, y, z, +1, Vec3{0.0, 0.0, 0.0});
                Voxel& v = rb.voxel_at(x, y, z);
                v.locked   = true;
                v.velocity = Vec3{0.0, 0.0, 0.0};
                v.flux     = Vec3{0.0, 0.0, 0.0};
            }
}

int place_locked_test_body(RenderBridge& rb, int x, int y, int z) {
    rb.inject_particle(x, y, z, +1, Vec3{0.0, 0.0, 0.0});
    Voxel& v = rb.voxel_at(x, y, z);
    v.locked   = true;
    v.velocity = Vec3{0.0, 0.0, 0.0};
    v.flux     = Vec3{0.0, 0.0, 0.0};
    v.tau      = 0.0;
    return rb.lattice().index(x, y, z);
}

Vec3 g_ext_at_near(RenderBridge& rb) {
    const int i = rb.lattice().index(kNx, kNy, kNz);
    const double L0 = rb.voxels()[static_cast<std::size_t>(i)].latency;
    const double dx = rb.voxel_at(kNx + 2, kNy, kNz).latency
                    - rb.voxel_at(kNx - 2, kNy, kNz).latency;
    const double dy = rb.voxel_at(kNx, kNy + 2, kNz).latency
                    - rb.voxel_at(kNx, kNy - 2, kNz).latency;
    const double dz = rb.voxel_at(kNx, kNy, kNz + 2).latency
                    - rb.voxel_at(kNx, kNy, kNz - 2).latency;
    const Vec3 grad = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
    return grad * (C_SPEED * C_SPEED * L0);
}

void configure_step_s(RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.gravity              = true;
    rb.toggles.latency_field        = true;
    rb.toggles.forces               = false;
    rb.toggles.movement             = false;
    rb.toggles.geometric_gravity    = false;
    rb.toggles.field_energy_gravity = false;
    rb.toggles.cluster_inertia      = false;
    rb.toggles.poisson_coulomb      = false;
    rb.toggles.emergent_forces      = false;
    rb.toggles.lorentz_force        = false;
    rb.toggles.de_broglie_clock     = false;
}

void configure_clock_ticks(RenderBridge& rb) {
    rb.toggles.latency_field     = false;
    rb.toggles.de_broglie_clock  = true;
    rb.toggles.forces            = false;
    rb.toggles.movement          = false;
    rb.toggles.geometric_gravity = false;
    rb.toggles.gravity           = false;
}

void configure_step_f(RenderBridge& rb, bool geometric) {
    rb.toggles.forces            = true;
    rb.toggles.gravity           = true;
    rb.toggles.cluster_inertia   = true;
    rb.toggles.geometric_gravity = geometric;
    rb.toggles.latency_field     = false;
    rb.toggles.movement          = false;
    rb.toggles.de_broglie_clock  = false;
    rb.toggles.poisson_coulomb   = false;
    rb.toggles.emergent_forces   = false;
    rb.toggles.lorentz_force     = false;
}

struct ClockResult {
    double L_near = 0.0;
    double L_far  = 0.0;
    double L_near_after = 0.0;
    double L_far_after  = 0.0;
    double gamma_near = 0.0;
    double gamma_far  = 0.0;
    double tau_near = 0.0;
    double tau_far  = 0.0;
    double rho      = 0.0;
    bool   near_in_source = false;
    bool   far_in_source  = false;
};

ClockResult run_clocks() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_source(rb);
    rb.tick();
    const auto src = source_indices(rb);
    ClockResult s;
    const int in = place_locked_test_body(rb, kNx, kNy, kNz);
    const int ifr = place_locked_test_body(rb, kFx, kFy, kFz);
    s.near_in_source = src.count(in) > 0;
    s.far_in_source  = src.count(ifr) > 0;
    s.L_near = rb.voxels()[static_cast<std::size_t>(in)].latency;
    s.L_far  = rb.voxels()[static_cast<std::size_t>(ifr)].latency;
    s.gamma_near = proper_time_rate(s.L_near, 0.0);
    s.gamma_far  = proper_time_rate(s.L_far, 0.0);
    configure_clock_ticks(rb);
    for (int t = 0; t < kNTau; ++t)
        rb.tick();
    s.L_near_after = rb.voxels()[static_cast<std::size_t>(in)].latency;
    s.L_far_after  = rb.voxels()[static_cast<std::size_t>(ifr)].latency;
    s.tau_near = rb.voxels()[static_cast<std::size_t>(in)].tau;
    s.tau_far  = rb.voxels()[static_cast<std::size_t>(ifr)].tau;
    const double pred = (s.gamma_far > 0.0) ? (s.gamma_near / s.gamma_far) : 0.0;
    const double meas = (s.tau_far > 0.0) ? (s.tau_near / s.tau_far) : 0.0;
    s.rho = (pred > 0.0) ? (meas / pred) : 0.0;
    return s;
}

struct FallResult {
    double g_ext_x = 0.0;
    double a_x     = 0.0;
    double r       = 0.0;
    double max_extra = 0.0;
    double max_f_grav = 0.0;
};

FallResult run_falling(bool geometric) {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_source(rb);
    rb.tick();
    place_locked_test_body(rb, kNx, kNy, kNz);
    const Vec3 g = g_ext_at_near(rb);
    FallResult s;
    s.g_ext_x = g.x;
    configure_step_f(rb, geometric);
    invoke_phase_forces(rb);
    const int i = rb.lattice().index(kNx, kNy, kNz);
    const Voxel& pv = rb.voxels()[static_cast<std::size_t>(i)];
    s.a_x = pv.velocity.x / rb.dt();
    s.r = (std::abs(s.g_ext_x) > 0.0) ? s.a_x / s.g_ext_x : 0.0;
    const auto& fd = rb.force_diag_at(i);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    s.max_f_grav = fd.f_gravity.mag();
    return s;
}

}  // namespace

void test_one_well_redshift_falling() {
    section("FTD-1019 one-well redshift + falling (CPU observer)");

    const ClockResult clk = run_clocks();
    const FallResult on  = run_falling(true);
    const FallResult off = run_falling(false);
    const double dx = static_cast<double>(kNx) - source_x_com();
    const double pred = (clk.gamma_far > 0.0) ? (clk.gamma_near / clk.gamma_far) : 0.0;

    std::printf("    L_n=%.6e  L_f=%.6e  G_n=%.6e  G_f=%.6e  pred=%.6e\n",
                clk.L_near, clk.L_far, clk.gamma_near, clk.gamma_far, pred);
    std::printf("    tau_n=%.6e  tau_f=%.6e  rho=%.6e\n",
                clk.tau_near, clk.tau_far, clk.rho);
    std::printf("    g_ext_x=%.6e  r_on=%.6e  r_off=%.6e\n",
                on.g_ext_x, on.r, off.r);

    const bool p1 = pred < 0.9999 && clk.gamma_near > 0.0 && clk.gamma_far > 0.0;
    const bool p2 = clk.L_near > clk.L_far;
    const bool p3 = !clk.near_in_source && !clk.far_in_source
                 && kNx >= 4 && kNx <= 27 && kFx >= 4 && kFx <= 27;
    const bool p4 = std::abs(clk.L_near_after - clk.L_near) < kFreezeTol
                 && std::abs(clk.L_far_after - clk.L_far) < kFreezeTol;
    const bool p5 = clk.tau_near > 0.0 && clk.tau_far > 0.0;
    const bool p6 = on.max_extra < kExtraForce;
    const bool p7 = on.max_f_grav > 0.0;
    const bool p8 = std::abs(off.r) < kATol;
    const bool p9 = std::abs(on.g_ext_x) > kGextFloor && (on.g_ext_x * dx) < 0.0;
    const bool a1 = std::abs(clk.rho - 1.0) < kATol;
    const bool a2 = std::abs(on.r - 1.0) < kATol;

    check("P1: clock contrast pred < 0.9999", p1);
    check("P2: near well is deeper", p2);
    check("P3: clocks are test bodies in bulk", p3);
    check("P4: well frozen through clock ticks", p4);
    check("P5: both clocks accumulated tau", p5);
    check("P6: falling extra-force channels < 1e-12", p6);
    check("P7: F-on gravity diagnostic nonzero", p7);
    check("P8: F-off |r| < 0.05", p8);
    check("P9: falling well toward the source", p9);
    check("A1: rest-clock rho within 0.05 of 1", a1);
    check("A2: falling |r_on-1| < 0.05", a2);

    const bool protocol = p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8 && p9;
    const char* verdict = "UNDERDETERMINED";
    if (protocol && a1 && a2) verdict = "FOUND";
    else if (protocol && !(a1 && a2)) verdict = "CLOSED-NEGATIVE";
    std::printf("    VERDICT %s  (A1 clocks: %s  A2 falling: %s)\n",
                verdict, a1 ? "yes" : "no", a2 ? "yes" : "no");
    check("protocol complete", protocol);
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_one_well_redshift_falling");
    ftd::test::test_one_well_redshift_falling();
    return ftd::test::finalize();
}
