// ============================================================================
// test_gpu_geometric_gravity_parity.cpp
// ----------------------------------------------------------------------------
// FTD-1018 / PREREG_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md
// Lock prefix SHA256:
//   624969CA01DC55906B55D409A38F56CFD539FFA592DB70E237E881375CF2EE9E
// Anchor: anchored-late until git tag
//   preregister-gpu-geometric-gravity-parity-v1 resolves.
//
// Native CUDA port of FTD-1016. No golden-tick contact. Protocol gates and
// A1 are CTest assertions.
// ============================================================================

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd {
namespace test {
namespace {

constexpr int    kL         = 32;
constexpr int    kPx        = 14;
constexpr int    kPy        = 14;
constexpr int    kPz        = 14;
constexpr double kL0        = 0.05;
constexpr double kSlope     = 1.0e-3;
constexpr double kA1Tol     = 1.0e-10;
constexpr double kOffTol    = 1.0e-12;
constexpr double kExtraForce = 1.0e-12;

int index_of(int x, int y, int z, int L) {
    return x * L * L + y * L + z;
}

double prescribed_latency(int x) {
    return kL0 + kSlope * static_cast<double>(x);
}

std::vector<Voxel> make_seed() {
    std::vector<Voxel> seed(static_cast<std::size_t>(kL * kL * kL));
    for (int x = 0; x < kL; ++x)
        for (int y = 0; y < kL; ++y)
            for (int z = 0; z < kL; ++z)
                seed[static_cast<std::size_t>(index_of(x, y, z, kL))].latency =
                    prescribed_latency(x);
    Voxel& v = seed[static_cast<std::size_t>(index_of(kPx, kPy, kPz, kL))];
    v.state       = 1;
    v.locked      = false;
    v.particle_id = 1;
    v.flux        = Vec3{0.0, 0.0, 0.0};
    v.velocity    = Vec3{0.0, 0.0, 0.0};
    return seed;
}

void configure(TermToggles& t, bool geometric) {
    t.disable_all();
    t.forces            = true;
    t.gravity           = true;
    t.geometric_gravity = geometric;
    t.movement          = false;
    t.latency_field     = false;
    t.cluster_inertia   = false;
    t.poisson_coulomb   = false;
    t.emergent_forces   = false;
    t.lorentz_force     = false;
}

struct Sample {
    Vec3   f_gravity{};
    Vec3   velocity{};
    double max_extra = 0.0;
};

Sample cpu_tick(bool geometric) {
    RenderBridge bridge(kL);
    bridge.force_cpu();
    configure(bridge.toggles, geometric);
    bridge.voxels() = make_seed();
    bridge.tick();
    const int i = index_of(kPx, kPy, kPz, kL);
    const auto fd = bridge.force_diag_at(kPx, kPy, kPz);
    Sample s;
    s.f_gravity = fd.f_gravity;
    s.velocity  = bridge.voxels()[static_cast<std::size_t>(i)].velocity;
    s.max_extra = std::max(std::max(fd.f_coulomb.mag(), fd.f_strong.mag()),
                           std::max(fd.f_magnetic.mag(), fd.f_exchange.mag()));
    return s;
}

Sample gpu_tick(bool geometric) {
    gpu::GpuEngine engine(kL);
    engine.graph_capture_enabled = false;
    configure(engine.toggles, geometric);
    engine.upload_from_host(make_seed());
    engine.tick();
    const int i = index_of(kPx, kPy, kPz, kL);
    const auto& fd = engine.force_diag();
    std::vector<Voxel> out;
    engine.sync_to_host(out);
    Sample s;
    const std::size_t ii = static_cast<std::size_t>(i);
    s.f_gravity = {fd.gravity_x[ii], fd.gravity_y[ii], fd.gravity_z[ii]};
    s.velocity  = out[ii].velocity;
    const double extra_c = std::hypot(fd.coulomb_x[ii], fd.coulomb_y[ii], fd.coulomb_z[ii]);
    const double extra_s = std::hypot(fd.strong_x[ii], fd.strong_y[ii], fd.strong_z[ii]);
    const double extra_m = std::hypot(fd.magnetic_x[ii], fd.magnetic_y[ii], fd.magnetic_z[ii]);
    const double extra_e = std::hypot(fd.exchange_x[ii], fd.exchange_y[ii], fd.exchange_z[ii]);
    s.max_extra = std::max(std::max(extra_c, extra_s), std::max(extra_m, extra_e));
    return s;
}

double max_abs_diff(const Vec3& a, const Vec3& b) {
    return std::max({std::abs(a.x - b.x), std::abs(a.y - b.y), std::abs(a.z - b.z)});
}

}  // namespace

void test_gpu_geometric_gravity_parity() {
    section("FTD-1018 GPU geometric-gravity parity");

    const Sample cpu_on  = cpu_tick(true);
    const Sample gpu_on  = gpu_tick(true);
    const Sample cpu_off = cpu_tick(false);
    const Sample gpu_off = gpu_tick(false);

    const double dF = max_abs_diff(cpu_on.f_gravity, gpu_on.f_gravity);
    const double dV = max_abs_diff(cpu_on.velocity, gpu_on.velocity);

    std::printf("    F_cpu=(%.6e, %.6e, %.6e)  F_gpu=(%.6e, %.6e, %.6e)\n",
                cpu_on.f_gravity.x, cpu_on.f_gravity.y, cpu_on.f_gravity.z,
                gpu_on.f_gravity.x, gpu_on.f_gravity.y, gpu_on.f_gravity.z);
    std::printf("    dF=%.3e  dV=%.3e  |F_cpu|=%.6e  |F_gpu|=%.6e\n",
                dF, dV, cpu_on.f_gravity.mag(), gpu_on.f_gravity.mag());
    std::printf("    |F_cpu_off|=%.3e  |F_gpu_off|=%.3e\n",
                cpu_off.f_gravity.mag(), gpu_off.f_gravity.mag());

    const bool p1 = cpu_on.f_gravity.mag() > 0.0;
    const bool p2 = gpu_on.f_gravity.mag() > 0.0;
    const bool p3 = cpu_on.max_extra < kExtraForce && gpu_on.max_extra < kExtraForce;
    const bool p4 = cpu_off.f_gravity.mag() < kOffTol && gpu_off.f_gravity.mag() < kOffTol
                 && cpu_off.velocity.mag() < kOffTol && gpu_off.velocity.mag() < kOffTol;
    const bool a1 = dF < kA1Tol && dV < kA1Tol;

    check("P1: CPU ON |F_gravity| > 0", p1);
    check("P2: GPU ON |F_gravity| > 0", p2);
    check("P3: extra-force channels < 1e-12 on both ON paths", p3);
    check("P4: toggle-OFF residue < 1e-12 on both backends", p4);
    check("A1: CPU/GPU dF and dV < 1e-10", a1);

    const bool protocol = p1 && p2 && p3 && p4;
    const char* verdict = "UNDERDETERMINED";
    if (protocol && a1) verdict = "FOUND";
    else if (protocol && !a1) verdict = "CLOSED-NEGATIVE";
    std::printf("    VERDICT %s  (A1 CUDA=CPU: %s)\n", verdict, a1 ? "yes" : "no");
    check("protocol complete", protocol);
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_gpu_geometric_gravity_parity");
    ftd::test::test_gpu_geometric_gravity_parity();
    return ftd::test::finalize();
}
