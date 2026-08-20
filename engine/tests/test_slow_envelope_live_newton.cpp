// ============================================================================
// test_slow_envelope_live_newton.cpp
// ----------------------------------------------------------------------------
// FTD-1022 / PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1_1.md
// Lock prefix SHA256:
//   5D0BB44FFDDEF81C1B3E84DFB46F45A79508DFDEDC9EAE611594776B07843FF9
// Anchor: anchored-late until git tag
//   preregister-slow-envelope-live-newton-v1-1 resolves.
// v1 (4C82033F…9D25) UNDERDETERMINED on P6 (COM g vs cluster-mean a).
// v1.1: g is the member-mean Q0 stencil. Sites and A2/A3 unchanged.
//
// 3^3 locked probe, freeze vs live vs self-force. No golden-tick contact.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

namespace ftd {
namespace test {
namespace {

constexpr int    kL         = 32;
constexpr int    kSEdge     = 5;
constexpr int    kOx        = 6;
constexpr int    kOy        = 13;
constexpr int    kOz        = 13;
constexpr int    kPEdge     = 3;
constexpr int    kPox       = 17;
constexpr int    kPoy       = 14;
constexpr int    kPoz       = 14;
constexpr int    kCx        = 18;
constexpr int    kCy        = 15;
constexpr int    kCz        = 15;
constexpr double kGextFloor = 1.0e-8;
constexpr double kATol      = 0.05;
constexpr double kExtraForce = 1.0e-12;
constexpr double kSelfFrac  = 0.20;
constexpr double kRigidTol  = 1.0e-12;

void invoke_phase_forces(RenderBridge& rb) {
    phase_forces_solve_potentials(rb);
    phase_forces_build_color_cache(rb);
    phase_forces_main_loop(rb);
    if (rb.toggles.cluster_inertia)
        phase_forces_integrate_clusters(rb);
}

std::set<int> cube_indices(const RenderBridge& rb, int ox, int oy, int oz, int edge) {
    std::set<int> out;
    for (int dx = 0; dx < edge; ++dx)
        for (int dy = 0; dy < edge; ++dy)
            for (int dz = 0; dz < edge; ++dz)
                out.insert(rb.lattice().index(ox + dx, oy + dy, oz + dz));
    return out;
}

double source_x_com() {
    return static_cast<double>(kOx) + 0.5 * static_cast<double>(kSEdge - 1);
}

void place_cube(RenderBridge& rb, int ox, int oy, int oz, int edge) {
    for (int dx = 0; dx < edge; ++dx)
        for (int dy = 0; dy < edge; ++dy)
            for (int dz = 0; dz < edge; ++dz) {
                const int x = ox + dx, y = oy + dy, z = oz + dz;
                rb.inject_particle(x, y, z, +1, Vec3{0.0, 0.0, 0.0});
                Voxel& v = rb.voxel_at(x, y, z);
                v.locked   = true;
                v.velocity = Vec3{0.0, 0.0, 0.0};
                v.flux     = Vec3{0.0, 0.0, 0.0};
            }
}

std::vector<int> probe_members(const RenderBridge& rb) {
    std::vector<int> m;
    m.reserve(static_cast<std::size_t>(kPEdge * kPEdge * kPEdge));
    for (int dx = 0; dx < kPEdge; ++dx)
        for (int dy = 0; dy < kPEdge; ++dy)
            for (int dz = 0; dz < kPEdge; ++dz)
                m.push_back(rb.lattice().index(kPox + dx, kPoy + dy, kPoz + dz));
    return m;
}

Vec3 g_at_site(RenderBridge& rb, int x, int y, int z) {
    const int i = rb.lattice().index(x, y, z);
    const double L0 = rb.voxels()[static_cast<std::size_t>(i)].latency;
    const double dx = rb.voxel_at(x + 2, y, z).latency
                    - rb.voxel_at(x - 2, y, z).latency;
    const double dy = rb.voxel_at(x, y + 2, z).latency
                    - rb.voxel_at(x, y - 2, z).latency;
    const double dz = rb.voxel_at(x, y, z + 2).latency
                    - rb.voxel_at(x, y, z - 2).latency;
    const Vec3 grad = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
    return grad * (C_SPEED * C_SPEED * L0);
}

Vec3 g_bar_probe(RenderBridge& rb) {
    Vec3 acc{0.0, 0.0, 0.0};
    int n = 0;
    for (int dx = 0; dx < kPEdge; ++dx)
        for (int dy = 0; dy < kPEdge; ++dy)
            for (int dz = 0; dz < kPEdge; ++dz) {
                acc = acc + g_at_site(rb, kPox + dx, kPoy + dy, kPoz + dz);
                ++n;
            }
    const double inv = 1.0 / static_cast<double>(n);
    return {acc.x * inv, acc.y * inv, acc.z * inv};
}

bool rigid_vx(const RenderBridge& rb, const std::vector<int>& members, double v0) {
    for (int idx : members) {
        const Vec3 vm = rb.voxels()[static_cast<std::size_t>(idx)].velocity;
        if (std::abs(vm.x - v0) > kRigidTol) return false;
        if (std::abs(vm.y) > kRigidTol || std::abs(vm.z) > kRigidTol) return false;
    }
    return true;
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
    bool   rigid = false;
    bool   overlap = false;
};

Arm finish_kick(RenderBridge& rb, bool live_tick) {
    Arm s;
    const auto src = cube_indices(rb, kOx, kOy, kOz, kSEdge);
    const auto pr  = cube_indices(rb, kPox, kPoy, kPoz, kPEdge);
    for (int i : pr)
        if (src.count(i)) s.overlap = true;
    if (live_tick) {
        configure_kick(rb, true);
        rb.tick();
    } else {
        configure_kick(rb, false);
        invoke_phase_forces(rb);
    }
    const Vec3 g = g_bar_probe(rb);
    s.g_x = g.x;
    const int ci = rb.lattice().index(kCx, kCy, kCz);
    s.L = rb.voxels()[static_cast<std::size_t>(ci)].latency;
    const auto mem = probe_members(rb);
    s.a_x = rb.voxels()[static_cast<std::size_t>(mem[0])].velocity.x / rb.dt();
    s.r = (std::abs(s.g_x) > 0.0) ? s.a_x / s.g_x : 0.0;
    s.rigid = rigid_vx(rb, mem, rb.voxels()[static_cast<std::size_t>(mem[0])].velocity.x);
    const auto& fd = rb.force_diag_at(ci);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    return s;
}

Arm run_Z() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_cube(rb, kOx, kOy, kOz, kSEdge);
    rb.tick();
    place_cube(rb, kPox, kPoy, kPoz, kPEdge);
    const Vec3 g = g_bar_probe(rb);
    Arm s = finish_kick(rb, false);
    s.g_x = g.x;
    s.L = rb.voxels()[static_cast<std::size_t>(rb.lattice().index(kCx, kCy, kCz))].latency;
    s.r = (std::abs(s.g_x) > 0.0) ? s.a_x / s.g_x : 0.0;
    return s;
}

Arm run_L() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_cube(rb, kOx, kOy, kOz, kSEdge);
    rb.tick();
    place_cube(rb, kPox, kPoy, kPoz, kPEdge);
    configure_step_s(rb);
    rb.tick();
    return finish_kick(rb, false);
}

Arm run_T() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_cube(rb, kOx, kOy, kOz, kSEdge);
    rb.tick();
    place_cube(rb, kPox, kPoy, kPoz, kPEdge);
    return finish_kick(rb, true);
}

Arm run_S() {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_cube(rb, kPox, kPoy, kPoz, kPEdge);
    rb.tick();
    return finish_kick(rb, false);
}

}  // namespace

void test_slow_envelope_live_newton() {
    section("FTD-1022 slow-envelope live Newton (CPU observer)");

    const Arm z = run_Z();
    const Arm l = run_L();
    const Arm t = run_T();
    const Arm s = run_S();
    const double dx = static_cast<double>(kCx) - source_x_com();
    const double da = (std::abs(z.a_x) > 0.0)
        ? std::abs(l.a_x - z.a_x) / std::abs(z.a_x) : 0.0;
    const double dg = (std::abs(z.g_x) > 0.0)
        ? std::abs(l.g_x - z.g_x) / std::abs(z.g_x) : 0.0;
    const double dT = (std::abs(l.a_x) > 1.0e-18)
        ? std::abs(t.a_x - l.a_x) / std::abs(l.a_x) : 0.0;
    const double rhoS = (std::abs(z.a_x) > 0.0)
        ? std::abs(s.a_x) / std::abs(z.a_x) : 0.0;

    std::printf("    arm Z freeze: g=%.6e a=%.6e r=%.6e L=%.6e rigid=%d\n",
                z.g_x, z.a_x, z.r, z.L, z.rigid ? 1 : 0);
    std::printf("    arm L live:   g=%.6e a=%.6e r=%.6e L=%.6e rigid=%d\n",
                l.g_x, l.a_x, l.r, l.L, l.rigid ? 1 : 0);
    std::printf("    arm T tick:   g=%.6e a=%.6e r=%.6e L=%.6e\n",
                t.g_x, t.a_x, t.r, t.L);
    std::printf("    arm S self:   g=%.6e a=%.6e r=%.6e L=%.6e\n",
                s.g_x, s.a_x, s.r, s.L);
    std::printf("    delta_a=%.6e  delta_g=%.6e  delta_T=%.6e  rho_S=%.6e  dx=%.3f\n",
                da, dg, dT, rhoS, dx);

    const bool p1 = std::abs(z.g_x) > kGextFloor;
    const bool p2 = (z.g_x * dx) < 0.0;
    const bool p3 = !z.overlap && kCx >= 4 && kCx <= 27;
    const bool p4 = z.max_extra < kExtraForce && l.max_extra < kExtraForce;
    const bool p5 = z.rigid && l.rigid && s.rigid;
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
    check("P3: probe disjoint, COM in bulk", p3);
    check("P4: extra-force channels < 1e-12", p4);
    check("P5: rigid 3^3 cluster", p5);
    check("P6: freeze |r_Z-1| < 0.05", p6);
    check("P7: |g_L| > 1e-8", p7);
    check("P8: g_L toward the source", p8);
    check("P9: self-force rho_S < 0.20", p9);
    check("P10: tick vs split |delta_T| < 0.05", p10);
    check("A1: live |r_L-1| < 0.05", a1);

    const bool protocol = p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8 && p9 && p10;
    const char* cls = "NONE";
    if (protocol && a1 && a2 && a3) cls = "test-body recovered";
    else if (protocol && a1) cls = "live wiring, envelope still responds";
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
    ftd::test::init("test_slow_envelope_live_newton");
    ftd::test::test_slow_envelope_live_newton();
    return ftd::test::finalize();
}
