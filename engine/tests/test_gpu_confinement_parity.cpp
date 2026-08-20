/**
 * CPU/CUDA parity for TermToggles::confinement.
 *
 * Default color_forces r>=8 is harmonic (F∝r). confinement replaces that
 * shell with ParticleEngine's constant F = SIGMA_STRING * cf.
 * [SELECTION], not FTD-0025. Default OFF => golden-neutral.
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using ftd::K_B;
using ftd::RenderBridge;
using ftd::SIGMA_STRING;
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

std::vector<Voxel> pair_seed(int L, int sep) {
    std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const int i1 = index_of(cx - sep / 2, cy, cz, L);
    const int i2 = index_of(cx + sep / 2, cy, cz, L);
    seed[i1].state = 1;
    seed[i1].color = 1;
    seed[i1].flux = Vec3{K_B, 0.0, 0.0};
    seed[i1].particle_id = 1;
    seed[i2].state = 1;
    seed[i2].color = 2;
    seed[i2].flux = Vec3{0.0, K_B, 0.0};
    seed[i2].particle_id = 2;
    return seed;
}

void configure_color(TermToggles& t, bool linear) {
    t.forces = true;
    t.color_forces = true;
    t.confinement = linear;
    t.movement = false;
}

double cpu_strong_mag(int L, int sep, bool linear) {
    const auto seed = pair_seed(L, sep);
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    configure_color(bridge.toggles, linear);
    bridge.voxels() = seed;
    bridge.tick();
    const int cx = L / 2, cy = L / 2, cz = L / 2;
    return bridge.force_diag_at(cx - sep / 2, cy, cz).f_strong.mag();
}

double gpu_strong_mag(int L, int sep, bool linear) {
    const auto seed = pair_seed(L, sep);
    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    configure_color(engine.toggles, linear);
    engine.upload_from_host(seed);
    engine.tick();
    const auto& fd = engine.force_diag();
    const int i = index_of(L / 2 - sep / 2, L / 2, L / 2, L);
    const double fx = fd.strong_x[static_cast<std::size_t>(i)];
    const double fy = fd.strong_y[static_cast<std::size_t>(i)];
    const double fz = fd.strong_z[static_cast<std::size_t>(i)];
    return std::sqrt(fx * fx + fy * fy + fz * fz);
}

}  // namespace

int main() {
    std::printf("GPU confinement colour-string parity\n");
    constexpr int L = 32;
    constexpr int sep_far = 10;
    constexpr int sep_farther = 12;

    const double cpu_lin = cpu_strong_mag(L, sep_far, true);
    const double gpu_lin = gpu_strong_mag(L, sep_far, true);
    const double cpu_harm = cpu_strong_mag(L, sep_far, false);
    const double gpu_harm = gpu_strong_mag(L, sep_far, false);
    const double cpu_lin_b = cpu_strong_mag(L, sep_farther, true);

    std::printf("    linear r=10 CPU=%.8g GPU=%.8g  harmonic CPU=%.8g GPU=%.8g\n",
                cpu_lin, gpu_lin, cpu_harm, gpu_harm);

    check("linear confinement CPU/GPU |F_strong| parity at r=10",
          std::abs(cpu_lin - gpu_lin) < 1e-12);
    check("harmonic colour CPU/GPU |F_strong| parity at r=10",
          std::abs(cpu_harm - gpu_harm) < 1e-12);
    check("linear |F| matches SIGMA_STRING (different-color |cf|=1)",
          std::abs(cpu_lin - SIGMA_STRING) < 1e-12);
    check("linear |F| is independent of r in the r>=8 shell",
          std::abs(cpu_lin - cpu_lin_b) < 1e-12);
    check("confinement changes the r>=8 colour force vs harmonic default",
          std::abs(cpu_lin - cpu_harm) > 1e-6);

    TermToggles missing;
    missing.disable_all();
    missing.confinement = true;
    std::string err;
    check("confinement requires color_forces",
          !missing.validate(&err) && err.find("color_forces") != std::string::npos);

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
