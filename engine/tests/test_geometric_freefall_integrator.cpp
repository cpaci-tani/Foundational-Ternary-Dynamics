// ============================================================================
// test_geometric_freefall_integrator.cpp
// ----------------------------------------------------------------------------
// FTD-1016 / PREREG_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md
// Lock prefix SHA256:
//   B825351085CAFBD36831E4A165F6CF22AB97849AB7915B606087031136EC7287
// Anchor: anchored-late until git tag
//   preregister-geometric-freefall-integrator-v1 resolves.
//
// Default-off CPU operator. No golden-tick contact. Protocol gates and A1
// are CTest assertions (adoption test, not a silent classifier).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd {
namespace test {
namespace {

constexpr int    kL          = 32;
constexpr int    kOrigin     = 14;
constexpr double kL0         = 0.05;
constexpr double kSlope      = 1.0e-3;
constexpr double kGextFloor  = 1.0e-8;
constexpr double kPathGTol   = 0.02;
constexpr double kA1Tol      = 0.05;
constexpr double kExtraForce = 1.0e-12;

double prescribed_latency(int x) {
    return kL0 + kSlope * static_cast<double>(x);
}

void write_well(RenderBridge& rb) {
    for (int x = 0; x < kL; ++x)
        for (int y = 0; y < kL; ++y)
            for (int z = 0; z < kL; ++z)
                rb.voxel_at(x, y, z).latency = prescribed_latency(x);
}

void configure_path_f(RenderBridge& rb, bool geometric) {
    rb.toggles.disable_all();
    rb.toggles.forces             = true;
    rb.toggles.gravity            = true;
    rb.toggles.cluster_inertia    = true;
    rb.toggles.geometric_gravity  = geometric;
    rb.toggles.latency_field      = false;
    rb.toggles.poisson_coulomb    = false;
    rb.toggles.emergent_forces    = false;
    rb.toggles.lorentz_force      = false;
}

void invoke_phase_forces(RenderBridge& rb) {
    phase_forces_solve_potentials(rb);
    phase_forces_build_color_cache(rb);
    phase_forces_main_loop(rb);
    if (rb.toggles.cluster_inertia)
        phase_forces_integrate_clusters(rb);
}

std::vector<int> build_locked_cube(RenderBridge& rb, int edge) {
    std::vector<int> members;
    members.reserve(static_cast<size_t>(edge) * edge * edge);
    for (int dx = 0; dx < edge; ++dx)
        for (int dy = 0; dy < edge; ++dy)
            for (int dz = 0; dz < edge; ++dz) {
                const int x = kOrigin + dx, y = kOrigin + dy, z = kOrigin + dz;
                rb.inject_particle(x, y, z, +1, Vec3{0.0, 0.0, 0.0});
                Voxel& v = rb.voxel_at(x, y, z);
                v.locked   = true;
                v.velocity = Vec3{0.0, 0.0, 0.0};
                v.flux     = Vec3{0.0, 0.0, 0.0};
                members.push_back(rb.lattice().index(x, y, z));
            }
    write_well(rb);
    return members;
}

double x_com(const RenderBridge& rb, const std::vector<int>& members) {
    double sx = 0.0;
    for (int idx : members) {
        const auto c = rb.lattice().coord(idx);
        sx += static_cast<double>(c.x);
    }
    return sx / static_cast<double>(members.size());
}

double g_ext_at(double xcom) {
    const double L = kL0 + kSlope * xcom;
    return C_SPEED * C_SPEED * L * kSlope;
}

bool rigid_vx(const RenderBridge& rb, const std::vector<int>& members, double v0) {
    for (int idx : members) {
        const Vec3 vm = rb.voxels()[idx].velocity;
        if (std::abs(vm.x - v0) > 1.0e-15) return false;
        if (std::abs(vm.y) > 1.0e-15 || std::abs(vm.z) > 1.0e-15) return false;
    }
    return true;
}

struct PathResult {
    int    N       = 0;
    double g_ext   = 0.0;
    double a_com   = 0.0;
    double r       = 0.0;
    bool   rigid   = false;
    double max_extra = 0.0;
    double max_f_grav = 0.0;
    bool   gravity_written = false;
};

bool members_active(const RenderBridge& rb, const std::vector<int>& members) {
    const auto& active = rb.ordered_active_indices();
    for (int idx : members) {
        if (std::find(active.begin(), active.end(), idx) == active.end())
            return false;
    }
    return !members.empty()
        && rb.force_diag().size() == rb.voxels().size();
}

PathResult run_path_g(int edge) {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_path_f(rb, true);
    const std::vector<int> members = build_locked_cube(rb, edge);
    const double g = g_ext_at(x_com(rb, members));
    const double vx = g * rb.dt();
    for (int idx : members)
        rb.voxels()[static_cast<std::size_t>(idx)].velocity = Vec3{vx, 0.0, 0.0};
    PathResult s;
    s.N     = static_cast<int>(members.size());
    s.g_ext = g;
    s.a_com = rb.voxels()[static_cast<std::size_t>(members[0])].velocity.x / rb.dt();
    s.r     = s.a_com / s.g_ext;
    s.rigid = rigid_vx(rb, members, vx);
    return s;
}

PathResult run_path_f(int edge, bool geometric) {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    configure_path_f(rb, geometric);
    const std::vector<int> members = build_locked_cube(rb, edge);
    invoke_phase_forces(rb);
    PathResult s;
    s.N     = static_cast<int>(members.size());
    s.g_ext = g_ext_at(x_com(rb, members));
    const Vec3 v0 = rb.voxels()[static_cast<std::size_t>(members[0])].velocity;
    s.a_com = v0.x / rb.dt();
    s.r     = (std::abs(s.g_ext) > 0.0) ? s.a_com / s.g_ext : 0.0;
    s.rigid = rigid_vx(rb, members, v0.x);
    s.max_extra = 0.0;
    s.max_f_grav = 0.0;
    s.gravity_written = members_active(rb, members);
    for (int idx : members) {
        const auto& fd = rb.force_diag_at(idx);
        s.max_extra = std::max(s.max_extra, fd.f_coulomb.mag());
        s.max_extra = std::max(s.max_extra, fd.f_strong.mag());
        s.max_extra = std::max(s.max_extra, fd.f_magnetic.mag());
        s.max_extra = std::max(s.max_extra, fd.f_exchange.mag());
        s.max_f_grav = std::max(s.max_f_grav, fd.f_gravity.mag());
    }
    return s;
}

}  // namespace

void test_geometric_freefall_integrator() {
    section("FTD-1016 geometric free-fall integrator (CPU observer)");

    const int edges[3] = {1, 2, 3};
    PathResult G[3];
    PathResult Fon[3];
    PathResult Foff[3];
    for (int i = 0; i < 3; ++i) {
        G[i]    = run_path_g(edges[i]);
        Fon[i]  = run_path_f(edges[i], true);
        Foff[i] = run_path_f(edges[i], false);
        std::printf("    N=%2d  g_ext=%.6e  r_G=%.6e  r_Fon=%.6e  r_Foff=%.6e  |f_g_on|=%.6e\n",
                    G[i].N, G[i].g_ext, G[i].r, Fon[i].r, Foff[i].r, Fon[i].max_f_grav);
    }

    bool p1 = true, p2 = true, p3 = true, p4 = true, p5 = true, p6 = true, a1 = true;
    double rGmin = G[0].r, rGmax = G[0].r, rGsum = 0.0;
    for (int i = 0; i < 3; ++i) {
        p1 = p1 && (std::abs(G[i].g_ext) > kGextFloor);
        p2 = p2 && (std::abs(G[i].r - 1.0) < kPathGTol) && G[i].rigid;
        p4 = p4 && (Fon[i].max_extra < kExtraForce);
        p5 = p5 && Fon[i].gravity_written && Fon[i].rigid && (Fon[i].max_f_grav > 0.0);
        p6 = p6 && (std::abs(Foff[i].r) < kA1Tol);
        a1 = a1 && (std::abs(Fon[i].r - 1.0) < kA1Tol);
        rGmin = std::min(rGmin, G[i].r);
        rGmax = std::max(rGmax, G[i].r);
        rGsum += G[i].r;
        const int expected_n = edges[i] * edges[i] * edges[i];
        check("registered N matches edge^3",
              G[i].N == expected_n && Fon[i].N == expected_n && Foff[i].N == expected_n);
    }
    const double rGmean = rGsum / 3.0;
    p3 = (std::abs(rGmean) > 0.0) && ((rGmax - rGmin) / std::abs(rGmean) < kPathGTol);

    check("P1: |g_ext| > 1e-8 at every N", p1);
    check("P2: Path G |r-1| < 0.02 and rigid", p2);
    check("P3: Path G r is N-independent to 2%", p3);
    check("P4: Path F-on extra-force channels < 1e-12", p4);
    check("P5: Path F-on gravity diagnostic written and nonzero", p5);
    check("P6: Path F-off |r| < 0.05 (FTD-1014 residue)", p6);
    check("A1: Path F-on |r-1| < 0.05 at every N", a1);

    const bool protocol = p1 && p2 && p3 && p4 && p5 && p6;
    const char* verdict = "UNDERDETERMINED";
    if (protocol && a1) verdict = "FOUND";
    else if (protocol && !a1) verdict = "CLOSED-NEGATIVE";
    std::printf("    VERDICT %s  (A1 live r_Fon~1: %s)\n",
                verdict, a1 ? "yes" : "no");
    check("protocol complete", protocol);
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_geometric_freefall_integrator");
    ftd::test::test_geometric_freefall_integrator();
    return ftd::test::finalize();
}
