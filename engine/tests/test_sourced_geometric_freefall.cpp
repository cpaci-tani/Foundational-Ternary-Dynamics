// ============================================================================
// test_sourced_geometric_freefall.cpp
// ----------------------------------------------------------------------------
// FTD-1017 / PREREG_SOURCED_GEOMETRIC_FREEFALL_v1.md
// Lock prefix SHA256:
//   A428956329AFC7DAD006178368FDA19ABE337754F20FD7372EECA376CE240D39
// Anchor: anchored-late until git tag
//   preregister-sourced-geometric-freefall-v1 resolves.
//
// Sourced-then-frozen well. No golden-tick contact. Protocol gates and A1
// are CTest assertions.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <utility>
#include <vector>

namespace ftd {
namespace test {
namespace {

constexpr int    kL        = 32;
constexpr int    kEdge     = 5;
constexpr int    kOx       = 6;
constexpr int    kOy       = 13;
constexpr int    kOz       = 13;
constexpr int    kPx       = 18;
constexpr int    kPy       = 15;
constexpr int    kPz       = 15;
constexpr double kGextFloor = 1.0e-8;
constexpr double kA1Tol     = 0.05;
constexpr double kExtraForce = 1.0e-12;

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

Vec3 g_ext_at_probe(RenderBridge& rb) {
    const auto& lat = rb.lattice();
    const int i = lat.index(kPx, kPy, kPz);
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

void configure_step_f(RenderBridge& rb, bool geometric) {
    rb.toggles.forces            = true;
    rb.toggles.gravity           = true;
    rb.toggles.cluster_inertia   = true;
    rb.toggles.geometric_gravity = geometric;
    rb.toggles.latency_field     = false;
    rb.toggles.movement          = false;
    rb.toggles.poisson_coulomb   = false;
    rb.toggles.emergent_forces   = false;
    rb.toggles.lorentz_force     = false;
}

struct PathResult {
    double g_ext_x = 0.0;
    double a_x     = 0.0;
    double r       = 0.0;
    double max_extra = 0.0;
    double max_f_grav = 0.0;
    int    probe_idx = -1;
    bool   probe_in_source = false;
};

PathResult run_path(bool geometric) {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_step_s(rb);
    place_source(rb);
    rb.tick();
    const auto src = source_indices(rb);
    PathResult s;
    s.probe_idx = place_probe(rb);
    s.probe_in_source = src.count(s.probe_idx) > 0;
    const Vec3 g = g_ext_at_probe(rb);
    s.g_ext_x = g.x;
    configure_step_f(rb, geometric);
    invoke_phase_forces(rb);
    const Voxel& pv = rb.voxels()[static_cast<std::size_t>(s.probe_idx)];
    s.a_x = pv.velocity.x / rb.dt();
    s.r = (std::abs(s.g_ext_x) > 0.0) ? s.a_x / s.g_ext_x : 0.0;
    const auto& fd = rb.force_diag_at(s.probe_idx);
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    s.max_f_grav = fd.f_gravity.mag();
    return s;
}

}  // namespace

void test_sourced_geometric_freefall() {
    section("FTD-1017 sourced geometric free-fall (CPU observer)");

    const PathResult on  = run_path(true);
    const PathResult off = run_path(false);
    const double dx = static_cast<double>(kPx) - source_x_com();

    std::printf("    g_ext_x=%.6e  a_on=%.6e  r_on=%.6e  r_off=%.6e  |f_g|=%.6e  dx=%.3f\n",
                on.g_ext_x, on.a_x, on.r, off.r, on.max_f_grav, dx);

    const bool p1 = std::abs(on.g_ext_x) > kGextFloor;
    const bool p2 = (on.g_ext_x * dx) < 0.0;
    const bool p3 = !on.probe_in_source && kPx >= 4 && kPx <= 27;
    const bool p4 = on.max_extra < kExtraForce;
    const bool p5 = on.max_f_grav > 0.0;
    const bool p6 = std::abs(off.r) < kA1Tol;
    const bool a1 = std::abs(on.r - 1.0) < kA1Tol;

    check("P1: |g_ext| > 1e-8 at the probe", p1);
    check("P2: g_ext points toward the source", p2);
    check("P3: probe is a test body in bulk", p3);
    check("P4: extra-force channels < 1e-12", p4);
    check("P5: F-on gravity diagnostic nonzero", p5);
    check("P6: F-off |r| < 0.05 (FTD-1014 residue)", p6);
    check("A1: F-on |r-1| < 0.05", a1);

    const bool protocol = p1 && p2 && p3 && p4 && p5 && p6;
    const char* verdict = "UNDERDETERMINED";
    if (protocol && a1) verdict = "FOUND";
    else if (protocol && !a1) verdict = "CLOSED-NEGATIVE";
    std::printf("    VERDICT %s  (A1 r_on~1: %s)\n", verdict, a1 ? "yes" : "no");
    check("protocol complete", protocol);
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_sourced_geometric_freefall");
    ftd::test::test_sourced_geometric_freefall();
    return ftd::test::finalize();
}
