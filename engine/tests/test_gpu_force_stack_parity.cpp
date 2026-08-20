/**
 * CPU/CUDA parity for the pairwise force stack and cluster inertia.
 *
 * Yukawa (strong_force) and exchange (exchange_force) share host/device
 * helpers with the CUDA kernels. cluster_inertia is a serial 1-thread
 * flood-fill matching CPU DFS order. Default OFF => golden-neutral.
 * This is a completeness contract, not a derivation.
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

bool close(double a, double b, double tol = 1e-12) {
    return std::abs(a - b) <= tol;
}

void stamp_pair(std::vector<Voxel>& seed, int L, int x1, int x2,
                int8_t spin1, int8_t spin2, int8_t color1, int8_t color2,
                bool locked) {
    const int cy = L / 2, cz = L / 2;
    const int i1 = index_of(x1, cy, cz, L);
    const int i2 = index_of(x2, cy, cz, L);
    seed[i1].state = 1;
    seed[i1].spin = spin1;
    seed[i1].color = color1;
    seed[i1].locked = locked;
    seed[i1].flux = Vec3{K_B, 0.0, 0.0};
    seed[i1].particle_id = 1;
    seed[i2].state = 1;
    seed[i2].spin = spin2;
    seed[i2].color = color2;
    seed[i2].locked = locked;
    seed[i2].flux = Vec3{0.0, K_B, 0.0};
    seed[i2].particle_id = 2;
}

struct ForceSample {
    double sx, sy, sz;
    double ex, ey, ez;
    double vx, vy, vz;
};

ForceSample cpu_tick(int L, const std::vector<Voxel>& seed, TermToggles t) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles = t;
    bridge.voxels() = seed;
    bridge.tick();
    const int i = index_of(L / 2 - 2, L / 2, L / 2, L);
    const auto fd = bridge.force_diag_at(L / 2 - 2, L / 2, L / 2);
    const auto& v = bridge.voxels()[static_cast<std::size_t>(i)];
    return {fd.f_strong.x, fd.f_strong.y, fd.f_strong.z,
            fd.f_exchange.x, fd.f_exchange.y, fd.f_exchange.z,
            v.velocity.x, v.velocity.y, v.velocity.z};
}

ForceSample gpu_tick(int L, const std::vector<Voxel>& seed, TermToggles t) {
    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    engine.toggles = t;
    engine.upload_from_host(seed);
    engine.tick();
    const int i = index_of(L / 2 - 2, L / 2, L / 2, L);
    const auto& fd = engine.force_diag();
    std::vector<Voxel> out;
    engine.sync_to_host(out);
    const auto& v = out[static_cast<std::size_t>(i)];
    return {
        fd.strong_x[static_cast<std::size_t>(i)],
        fd.strong_y[static_cast<std::size_t>(i)],
        fd.strong_z[static_cast<std::size_t>(i)],
        fd.exchange_x[static_cast<std::size_t>(i)],
        fd.exchange_y[static_cast<std::size_t>(i)],
        fd.exchange_z[static_cast<std::size_t>(i)],
        v.velocity.x, v.velocity.y, v.velocity.z
    };
}

bool samples_match(const ForceSample& a, const ForceSample& b) {
    return close(a.sx, b.sx) && close(a.sy, b.sy) && close(a.sz, b.sz)
        && close(a.ex, b.ex) && close(a.ey, b.ey) && close(a.ez, b.ez)
        && close(a.vx, b.vx) && close(a.vy, b.vy) && close(a.vz, b.vz);
}

}  // namespace

int main() {
    std::printf("GPU force-stack parity (Yukawa / exchange / cluster)\n");
    constexpr int L = 16;
    const int x1 = L / 2 - 2;
    const int x2 = L / 2 + 2;

    {
        std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
        stamp_pair(seed, L, x1, x2, 0, 0, 0, 0, false);
        TermToggles t;
        t.disable_all();
        t.strong_force = true;
        t.movement = false;
        const auto cpu = cpu_tick(L, seed, t);
        const auto gpu = gpu_tick(L, seed, t);
        std::printf("    Yukawa F_strong CPU=(%.8g,%.8g,%.8g) GPU=(%.8g,%.8g,%.8g)\n",
                    cpu.sx, cpu.sy, cpu.sz, gpu.sx, gpu.sy, gpu.sz);
        check("Yukawa-only CPU/GPU force+velocity parity", samples_match(cpu, gpu));
        check("Yukawa produces a nonzero attractive force", std::abs(cpu.sx) > 1e-12);
    }

    {
        std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
        stamp_pair(seed, L, x1, x2, +1, +1, 0, 0, false);
        TermToggles t;
        t.disable_all();
        t.poisson_coulomb = true;
        t.exchange_force = true;
        t.movement = false;
        const auto cpu = cpu_tick(L, seed, t);
        const auto gpu = gpu_tick(L, seed, t);
        std::printf("    Exchange F_ex CPU=(%.8g,%.8g,%.8g) GPU=(%.8g,%.8g,%.8g)\n",
                    cpu.ex, cpu.ey, cpu.ez, gpu.ex, gpu.ey, gpu.ez);
        check("Exchange-only CPU/GPU force+velocity parity", samples_match(cpu, gpu));
        check("Exchange produces a nonzero same-spin repulsion", std::abs(cpu.ex) > 1e-12);
    }

    {
        std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
        stamp_pair(seed, L, x1, x1 + 1, 0, 0, 1, 2, true);
        TermToggles t;
        t.disable_all();
        t.forces = true;
        t.color_forces = true;
        t.cluster_inertia = true;
        t.movement = false;
        const auto cpu = cpu_tick(L, seed, t);
        const auto gpu = gpu_tick(L, seed, t);
        std::printf("    Cluster V CPU=(%.8g,%.8g,%.8g) GPU=(%.8g,%.8g,%.8g)\n",
                    cpu.vx, cpu.vy, cpu.vz, gpu.vx, gpu.vy, gpu.vz);
        check("cluster_inertia CPU/GPU COM-velocity parity", samples_match(cpu, gpu));
        check("locked cluster acquires a shared COM velocity",
              std::abs(cpu.vx) + std::abs(cpu.vy) + std::abs(cpu.vz) > 1e-12);
    }

    {
        std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
        stamp_pair(seed, L, x1, x2, 0, 0, 1, 2, false);
        seed[static_cast<std::size_t>(index_of(x1, L / 2, L / 2, L))].locked = true;
        TermToggles t;
        t.disable_all();
        t.color_forces = true;
        t.cluster_inertia = true;
        t.movement = false;
        std::string error;
        check("colour-only cluster_inertia validates", t.validate(&error));
        const auto cpu = cpu_tick(L, seed, t);
        const auto gpu = gpu_tick(L, seed, t);
        std::printf("    Colour-cluster V CPU=(%.8g,%.8g,%.8g) GPU=(%.8g,%.8g,%.8g)\n",
                    cpu.vx, cpu.vy, cpu.vz, gpu.vx, gpu.vy, gpu.vz);
        check("colour-only cluster_inertia CPU/GPU COM-velocity parity",
              samples_match(cpu, gpu));
        check("locked colour probe acquires cluster velocity from a distant source",
              std::abs(cpu.vx) + std::abs(cpu.vy) + std::abs(cpu.vz) > 1e-12);
    }

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
