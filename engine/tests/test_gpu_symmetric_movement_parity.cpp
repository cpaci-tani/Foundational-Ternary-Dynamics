/**
 * CPU/CUDA parity for symmetric_movement_order.
 *
 * Default OFF => golden-neutral. When on, both backends Fisher-Yates the
 * movement traversal with VoxelRng::MovementShuffle and apply the same
 * axis permutation. This pins contended-site order, not a derivation.
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdio>
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

bool close(double a, double b, double tol = 2e-13) {
    return std::abs(a - b) <= tol;
}

bool same_voxel(const Voxel& a, const Voxel& b) {
    return a.state == b.state
        && close(a.velocity.x, b.velocity.x)
        && close(a.velocity.y, b.velocity.y)
        && close(a.velocity.z, b.velocity.z)
        && close(a.remainder.x, b.remainder.x)
        && close(a.remainder.y, b.remainder.y)
        && close(a.remainder.z, b.remainder.z)
        && close(a.flux.x, b.flux.x) && close(a.flux.y, b.flux.y)
        && close(a.flux.z, b.flux.z)
        && a.color == b.color && a.spin == b.spin;
}

std::vector<Voxel> chain_seed(int L) {
    std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
    for (int x = 5; x <= 7; ++x) {
        const int i = index_of(x, 5, 5, L);
        seed[i].state = 1;
        seed[i].velocity = Vec3{5.0, 0.0, 0.0};
        seed[i].flux = Vec3{ftd::K_B, 0.0, 0.0};
        seed[i].particle_id = x;
    }
    return seed;
}

void configure_sym(TermToggles& t, bool symmetric) {
    t.movement = true;
    t.reflective_boundary = true;
    t.symmetric_movement_order = symmetric;
    t.langevin_seed = 42;
}

}  // namespace

int main() {
    std::printf("GPU symmetric movement order parity\n");
    constexpr int L = 16;
    constexpr int ticks = 10;
    const auto seed = chain_seed(L);

    RenderBridge cpu(L);
    cpu.force_cpu();
    cpu.toggles.disable_all();
    configure_sym(cpu.toggles, true);
    cpu.voxels() = seed;
    for (int n = 0; n < ticks; ++n) cpu.tick();

    ftd::gpu::GpuEngine gpu(L);
    gpu.toggles.disable_all();
    configure_sym(gpu.toggles, true);
    gpu.upload_from_host(seed);
    for (int n = 0; n < ticks; ++n) gpu.tick();
    std::vector<Voxel> gpu_v;
    gpu.sync_to_host(gpu_v);

    const auto& cpu_v = static_cast<const RenderBridge&>(cpu).voxels();
    bool identical = cpu_v.size() == gpu_v.size();
    int charge_cpu = 0, charge_gpu = 0;
    for (std::size_t i = 0; i < cpu_v.size(); ++i) {
        charge_cpu += cpu_v[i].state;
        charge_gpu += gpu_v[i].state;
        if (!same_voxel(cpu_v[i], gpu_v[i])) identical = false;
    }
    check("symmetric movement CPU/GPU voxel image", identical);
    check("symmetric movement conserves charge on both backends",
          charge_cpu == 3 && charge_gpu == 3);

    RenderBridge cpu_asym(L);
    cpu_asym.force_cpu();
    cpu_asym.toggles.disable_all();
    configure_sym(cpu_asym.toggles, false);
    cpu_asym.voxels() = seed;
    for (int n = 0; n < ticks; ++n) cpu_asym.tick();
    bool diverged = false;
    const auto& asym = static_cast<const RenderBridge&>(cpu_asym).voxels();
    for (std::size_t i = 0; i < cpu_v.size(); ++i) {
        if (cpu_v[i].state != asym[i].state) {
            diverged = true;
            break;
        }
    }
    check("symmetric order diverges from default X-major order", diverged);

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
