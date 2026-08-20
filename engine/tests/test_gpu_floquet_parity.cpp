/**
 * CPU/CUDA parity for the FTD-0408 / FTD-0411 period-two Floquet wave kicks.
 *
 * Default OFF => golden-neutral. Wave-only lattice; unit tick; four ticks so
 * both even (+3/13 or BCC even) and odd (−1/13 or BCC odd) kappas fire.
 */

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

std::vector<Voxel> run_cpu(int L, const std::vector<Voxel>& seed,
                           const Configure& configure, int ticks) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    configure(bridge.toggles);
    bridge.voxels() = seed;
    for (int n = 0; n < ticks; ++n) bridge.tick();
    return static_cast<const RenderBridge&>(bridge).voxels();
}

std::vector<Voxel> run_gpu(int L, const std::vector<Voxel>& seed,
                           const Configure& configure, int ticks) {
    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    configure(engine.toggles);
    engine.upload_from_host(seed);
    for (int n = 0; n < ticks; ++n) engine.tick();
    std::vector<Voxel> out;
    engine.sync_to_host(out);
    return out;
}

void run_case(const char* name, const Configure& floquet, bool dual) {
    std::printf("\n%s (dual=%d)\n", name, dual ? 1 : 0);
    constexpr int L = 8;
    constexpr int ticks = 4;
    const auto seed = wave_seed(L, dual);
    const Configure def = [dual](TermToggles& t) {
        t.wave_propagation = true;
        t.dual_substrate = dual;
    };

    const auto cpu_v = run_cpu(L, seed, floquet, ticks);
    const auto gpu_v = run_gpu(L, seed, floquet, ticks);
    const auto cpu_def = run_cpu(L, seed, def, ticks);
    const double parity = max_field_error(cpu_v, gpu_v);
    const double vs_default = max_field_error(cpu_v, cpu_def);
    std::printf("    parity=%.3e  floquet-vs-default=%.3e\n", parity, vs_default);
    check((std::string(name) + " CPU/GPU field parity").c_str(),
          parity < 1e-12);
    check((std::string(name) + " differs from constant C_WAVE^2 kick").c_str(),
          vs_default > 1e-18);
}

}  // namespace

int main() {
    std::printf("GPU Floquet wave-kick parity\n");
    run_case("F1 period-2", [](TermToggles& t) {
        t.wave_propagation = true;
        t.lorentz_period2_floquet = true;
    }, false);
    run_case("F2 BCC-time", [](TermToggles& t) {
        t.wave_propagation = true;
        t.lorentz_bcc_time_floquet = true;
    }, false);
    run_case("F3 period-2 dual", [](TermToggles& t) {
        t.wave_propagation = true;
        t.lorentz_period2_floquet = true;
        t.dual_substrate = true;
    }, true);

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
