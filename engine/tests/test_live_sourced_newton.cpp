// ============================================================================
// test_live_sourced_newton.cpp
// ----------------------------------------------------------------------------
// FTD-1021 / PREREG_LIVE_SOURCED_NEWTON_v1.md
// Lock prefix SHA256:
//   9D76CCBB63C05BEDE4B07A71DC68CA96A18305888D9071BA093A206B398D7EEF
// Anchor: anchored-late until git tag
//   preregister-live-sourced-newton-v1 resolves.
//
// Freeze vs live Poisson occupancy vs self-force. No golden-tick contact.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <set>

namespace ftd {
namespace test {
namespace {

constexpr int    kL         = 32;
constexpr int    kEdge      = 5;
constexpr int    kOx        = 6;
constexpr int    kOy        = 13;
constexpr int    kOz        = 13;
constexpr int    kPx        = 18;
constexpr int    kPy        = 15;
constexpr int    kPz        = 15;
constexpr double kGextFloor = 1.0e-8;
constexpr double kATol      = 0.05;
constexpr double kExtraForce = 1.0e-12;
constexpr double kSelfFrac  = 0.20;

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

int place_probe(RenderBridge& rb) {
    rb.inject_particle(kPx, kPy, kPz, +1, Vec3{0.0, 0.0, 0.0});
    Voxel& v = rb.voxel_at(kPx, kPy, kPz);
    v.locked   = true;
    v.velocity = Vec3{0.0, 0.0, 0.0};
    v.flux     = Vec3{0.0, 0.0, 0.0};
    return rb.lattice().index(kPx, kPy, kPz);
}

Vec3 g_at_probe(RenderBridge& rb) {
    const int i = rb.lattice().index(kPx, kPy, kPz);
    const double L0 = rb.voxels()[static_cast<std::size_t>(i)].latency;
    const double dx = rb.voxel_at(kPx + 2, kPy, kPz).latency
                    - rb.voxel_at(kPx - 2, kPy, kPz).latency;
    const double dy = rb.voxel_at(kPx, kPy + 2, kPz).latency
                    - rb.voxel_at(kPx, kPy - 2, kPz).latency;
    const double dz = rb.voxel_at(kPx, kPy, kPz + 2).latency
                    - rb.voxel_at(kPx, kPy, kPz - 2).latency;
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
}

void configure_kick(RenderBridge& rb, bool latency_on) {
    rb.toggles.forces            = true;
    rb.toggles.gravity           = true;
    rb.toggles.cluster_inertia   = true;
    rb.toggles.geometric_gravity = true;
    rb.toggles.latency_field     = latency_on;
    rb.toggles.movement          = false;
    rb.toggles.poisson_coulomb   = false;
    rb.toggles.emergent_forces   = false;
    rb.toggles.lorentz_force     = false;
    rb.toggles.field_energy_gravity = false;
}

struct Arm {
    double g_x = 0.0;
    double a_x = 0.0;
    double r   = 0.0;
    double L   = 0.0;
    double max_extra = 0.0;
    double max_f_grav = 0.0;
    int    probe_idx = -1;
    bool   probe_in_source = false;
};

Arm run_Z() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_source(rb);
    rb.tick();
    const auto src = source_indices(rb);
    Arm s;
    s.probe_idx = place_probe(rb);
    s.probe_in_source = src.count(s.probe_idx) > 0;
    const Vec3 g = g_at_probe(rb);
    s.g_x = g.x;
    s.L = rb.voxels()[static_cast<std::size_t>(s.probe_idx)].latency;
    configure_kick(rb, false);
    invoke_phase_forces(rb);
    const Voxel& pv = rb.voxels()[static_cast<std::size_t>(s.probe_idx)];
    s.a_x = pv.velocity.x / rb.dt();
    s.r = (std::abs(s.g_x) > 0.0) ? s.a_x / s.g_x : 0.0;
    const auto& fd = rb.force_diag_at(s.probe_idx);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    s.max_f_grav = fd.f_gravity.mag();
    return s;
}

Arm run_L() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_source(rb);
    rb.tick();
    Arm s;
    s.probe_idx = place_probe(rb);
    configure_step_s(rb);
    rb.tick();
    const Vec3 g = g_at_probe(rb);
    s.g_x = g.x;
    s.L = rb.voxels()[static_cast<std::size_t>(s.probe_idx)].latency;
    configure_kick(rb, false);
    invoke_phase_forces(rb);
    const Voxel& pv = rb.voxels()[static_cast<std::size_t>(s.probe_idx)];
    s.a_x = pv.velocity.x / rb.dt();
    s.r = (std::abs(s.g_x) > 0.0) ? s.a_x / s.g_x : 0.0;
    const auto& fd = rb.force_diag_at(s.probe_idx);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    s.max_f_grav = fd.f_gravity.mag();
    return s;
}

Arm run_T() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_source(rb);
    rb.tick();
    Arm s;
    s.probe_idx = place_probe(rb);
    configure_kick(rb, true);
    rb.tick();
    const Vec3 g = g_at_probe(rb);
    s.g_x = g.x;
    s.L = rb.voxels()[static_cast<std::size_t>(s.probe_idx)].latency;
    const Voxel& pv = rb.voxels()[static_cast<std::size_t>(s.probe_idx)];
    s.a_x = pv.velocity.x / rb.dt();
    s.r = (std::abs(s.g_x) > 0.0) ? s.a_x / s.g_x : 0.0;
    const auto& fd = rb.force_diag_at(s.probe_idx);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    s.max_f_grav = fd.f_gravity.mag();
    return s;
}

Arm run_S() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    Arm s;
    s.probe_idx = place_probe(rb);
    rb.tick();
    const Vec3 g = g_at_probe(rb);
    s.g_x = g.x;
    s.L = rb.voxels()[static_cast<std::size_t>(s.probe_idx)].latency;
    configure_kick(rb, false);
    invoke_phase_forces(rb);
    const Voxel& pv = rb.voxels()[static_cast<std::size_t>(s.probe_idx)];
    s.a_x = pv.velocity.x / rb.dt();
    s.r = (std::abs(s.g_x) > 0.0) ? s.a_x / s.g_x : 0.0;
    const auto& fd = rb.force_diag_at(s.probe_idx);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    s.max_f_grav = fd.f_gravity.mag();
    return s;
}

}  // namespace

void test_live_sourced_newton() {
    section("FTD-1021 live sourced Newton (CPU observer)");

    const Arm z = run_Z();
    const Arm l = run_L();
    const Arm t = run_T();
    const Arm s = run_S();
    const double dx = static_cast<double>(kPx) - source_x_com();
    const double da = (std::abs(z.a_x) > 0.0)
        ? std::abs(l.a_x - z.a_x) / std::abs(z.a_x) : 0.0;
    const double dg = (std::abs(z.g_x) > 0.0)
        ? std::abs(l.g_x - z.g_x) / std::abs(z.g_x) : 0.0;
    const double dT = (std::abs(l.a_x) > 1.0e-18)
        ? std::abs(t.a_x - l.a_x) / std::abs(l.a_x) : 0.0;
    const double rhoS = (std::abs(z.a_x) > 0.0)
        ? std::abs(s.a_x) / std::abs(z.a_x) : 0.0;

    std::printf("    arm Z freeze: g=%.6e a=%.6e r=%.6e L=%.6e |f_g|=%.6e\n",
                z.g_x, z.a_x, z.r, z.L, z.max_f_grav);
    std::printf("    arm L live:   g=%.6e a=%.6e r=%.6e L=%.6e |f_g|=%.6e\n",
                l.g_x, l.a_x, l.r, l.L, l.max_f_grav);
    std::printf("    arm T tick:   g=%.6e a=%.6e r=%.6e L=%.6e\n",
                t.g_x, t.a_x, t.r, t.L);
    std::printf("    arm S self:   g=%.6e a=%.6e r=%.6e L=%.6e\n",
                s.g_x, s.a_x, s.r, s.L);
    std::printf("    delta_a=%.6e  delta_g=%.6e  delta_T=%.6e  rho_S=%.6e  dx=%.3f\n",
                da, dg, dT, rhoS, dx);

    const bool p1 = std::abs(z.g_x) > kGextFloor;
    const bool p2 = (z.g_x * dx) < 0.0;
    const bool p3 = !z.probe_in_source && kPx >= 4 && kPx <= 27;
    const bool p4 = z.max_extra < kExtraForce && l.max_extra < kExtraForce;
    const bool p5 = z.max_f_grav > 0.0;
    const bool p6 = std::abs(z.r - 1.0) < kATol;
    const bool p7 = std::abs(l.g_x) > kGextFloor;
    const bool p8 = (l.g_x * dx) < 0.0;
    const bool p9 = rhoS < kSelfFrac;
    const bool p10 = dT < kATol || std::abs(l.a_x) < 1.0e-18;
    const bool a1 = std::abs(l.r - 1.0) < kATol;
    const bool a2 = da < kATol;
    const bool a3 = dg < kATol;

    check("P1: |g_Z| > 1e-8", p1);
    check("P2: g_Z toward the source", p2);
    check("P3: probe is a test body in bulk", p3);
    check("P4: extra-force channels < 1e-12", p4);
    check("P5: Z gravity diagnostic nonzero", p5);
    check("P6: freeze replica |r_Z-1| < 0.05", p6);
    check("P7: |g_L| > 1e-8", p7);
    check("P8: g_L toward the source", p8);
    check("P9: self-force rho_S < 0.20", p9);
    check("P10: tick vs split |delta_T| < 0.05", p10);
    check("A1: live |r_L-1| < 0.05", a1);

    const bool protocol = p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8 && p9 && p10;
    const char* cls = "NONE";
    if (protocol && a1 && a2 && a3) cls = "live wiring + test-body";
    else if (protocol && a1) cls = "live wiring, well responds";
    else if (protocol && !a1) cls = "CLOSED-NEGATIVE";
    const char* verdict = "UNDERDETERMINED";
    if (protocol && a1) verdict = "FOUND";
    else if (protocol && !a1) verdict = "CLOSED-NEGATIVE";
    std::printf("    CLASS %s  A2=%s A3=%s  VERDICT %s\n",
                cls, a2 ? "yes" : "no", a3 ? "yes" : "no", verdict);
    check("protocol complete", protocol);
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_live_sourced_newton");
    ftd::test::test_live_sourced_newton();
    return ftd::test::finalize();
}
