/**
 * CPU/CUDA parity for the E1 / FTD-0337 velocity-Verlet (KDK) wave integrator.
 *
 * Default OFF => golden-neutral. This test enables the term on a wave-only
 * lattice (no Gauss, no forces, no genesis) and requires bit-level field
 * agreement, including dt<1. Dual-substrate is a second closed case.
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace {

using ftd::RenderBridge;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;

int passed = 0;
int failed = 0;

void check(const char* name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
    ok ? ++passed : ++failed;
}

int index_of(int x, int y, int z, int L) {
    return x * L * L + y * L + z;
}

double vec_error(const Vec3& a, const Vec3& b) {
    return std::max({std::abs(a.x - b.x), std::abs(a.y - b.y),
                     std::abs(a.z - b.z)});
}

double field_error(const Voxel& a, const Voxel& b) {
    return std::max({
        vec_error(a.flux, b.flux), vec_error(a.wave_vel, b.wave_vel),
        vec_error(a.flux_L, b.flux_L), vec_error(a.flux_R, b.flux_R),
        vec_error(a.wave_vel_L, b.wave_vel_L),
        vec_error(a.wave_vel_R, b.wave_vel_R),
    });
}

double max_field_error(const std::vector<Voxel>& a,
                       const std::vector<Voxel>& b) {
    double result = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i)
        result = std::max(result, field_error(a[i], b[i]));
    return result;
}

std::vector<Voxel> wave_seed(int L, bool dual) {
    std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = index_of(x, y, z, L);
        const double q = 1e-3 * (1.0 + 3.0 * x + 5.0 * y + 7.0 * z);
        seed[i].flux = Vec3(q, -2.0 * q, 0.5 * q);
        seed[i].wave_vel = Vec3(-0.3 * q, 0.7 * q, -0.9 * q);
        if (dual) {
            seed[i].flux_L = Vec3(1.1 * q, -1.2 * q, 1.3 * q);
            seed[i].flux_R = Vec3(-1.4 * q, 1.5 * q, -1.6 * q);
            seed[i].wave_vel_L = Vec3(1.7 * q, -1.8 * q, 1.9 * q);
            seed[i].wave_vel_R = Vec3(-2.0 * q, 2.1 * q, -2.2 * q);
            seed[i].flux = seed[i].flux_L + seed[i].flux_R;
            seed[i].wave_vel = seed[i].wave_vel_L + seed[i].wave_vel_R;
        }
    }
    return seed;
}

using Configure = std::function<void(TermToggles&)>;

void wave_only_verlet(TermToggles& t, bool dual) {
    t.wave_propagation = true;
    t.verlet_wave_integrator = true;
    t.dual_substrate = dual;
}

void wave_only_default(TermToggles& t, bool dual) {
    t.wave_propagation = true;
    t.dual_substrate = dual;
}

std::vector<Voxel> run_cpu(int L, const std::vector<Voxel>& seed,
                           const Configure& configure, double dt, int ticks) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    configure(bridge.toggles);
    bridge.set_dt(dt);
    bridge.voxels() = seed;
    for (int n = 0; n < ticks; ++n) bridge.tick();
    return static_cast<const RenderBridge&>(bridge).voxels();
}

std::vector<Voxel> run_gpu(int L, const std::vector<Voxel>& seed,
                           const Configure& configure, double dt, int ticks) {
    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    configure(engine.toggles);
    engine.set_dt(dt);
    engine.upload_from_host(seed);
    for (int n = 0; n < ticks; ++n) engine.tick();
    std::vector<Voxel> out;
    engine.sync_to_host(out);
    return out;
}

void run_case(const char* name, bool dual, double dt, int ticks) {
    std::printf("\n%s (dual=%d dt=%.3f ticks=%d)\n", name, dual ? 1 : 0, dt, ticks);
    constexpr int L = 8;
    const auto seed = wave_seed(L, dual);
    const Configure verlet = [dual](TermToggles& t) { wave_only_verlet(t, dual); };
    const Configure def = [dual](TermToggles& t) { wave_only_default(t, dual); };

    const auto cpu_v = run_cpu(L, seed, verlet, dt, ticks);
    const auto gpu_v = run_gpu(L, seed, verlet, dt, ticks);
    const auto cpu_def = run_cpu(L, seed, def, dt, ticks);

    const double parity = max_field_error(cpu_v, gpu_v);
    const double vs_default = max_field_error(cpu_v, cpu_def);
    std::printf("    parity=%.3e  verlet-vs-default=%.3e\n", parity, vs_default);

    check((std::string(name) + " CPU/GPU Verlet field parity").c_str(),
          parity < 1e-12);
    check((std::string(name) + " Verlet differs from unit-step leapfrog").c_str(),
          vs_default > 1e-18);
}

}  // namespace

int main() {
    std::printf("GPU Verlet wave integrator parity\n");
    run_case("V1 single-substrate dt=1", false, 1.0, 4);
    run_case("V2 single-substrate dt=0.5", false, 0.5, 4);
    run_case("V3 dual-substrate dt=0.5", true, 0.5, 4);

    ftd::gpu::GpuEngine engine(8);
    engine.toggles.disable_all();
    engine.toggles.wave_propagation = true;
    engine.toggles.verlet_wave_integrator = true;
    engine.set_dt(0.5);
    check("GpuEngine honors dt<1 under Verlet", std::abs(engine.dt() - 0.5) < 1e-15);

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
