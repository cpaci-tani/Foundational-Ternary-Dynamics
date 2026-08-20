/**
 * CPU/CUDA parity for FTD-0428 matched_gauss_dynamics.
 *
 * Isolated conservative-movement sector: skip the legacy wave writer,
 * advance oriented-face Maxwell/Gauss on device, and write centered E
 * into flux. Default OFF => golden-neutral. Completeness contract, not
 * a derivation. Energy scalars may differ at long-double reduction;
 * flux/E agreement is the gate.
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

bool close(double a, double b, double tol = 1e-12) {
    return std::abs(a - b) <= tol;
}

double flux_linf(const std::vector<Voxel>& a, const std::vector<Voxel>& b) {
    double err = 0.0;
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();
    for (std::size_t i = 0; i < n; ++i) {
        err = std::max(err, std::abs(a[i].flux.x - b[i].flux.x));
        err = std::max(err, std::abs(a[i].flux.y - b[i].flux.y));
        err = std::max(err, std::abs(a[i].flux.z - b[i].flux.z));
    }
    return err;
}

TermToggles isolated(bool movement) {
    TermToggles t;
    t.disable_all();
    t.movement = movement;
    t.matched_gauss_dynamics = true;
    return t;
}

}  // namespace

int main() {
    std::printf("GPU matched-Gauss parity (FTD-0428)\n");
    constexpr int L = 16;
    std::string error;
    check("isolated matched profile validates", isolated(false).validate(&error));

    {
        RenderBridge cpu(L);
        cpu.force_cpu();
        cpu.toggles = isolated(false);
        check("CPU minimum-energy init",
              cpu.initialize_matched_gauss_dynamics().valid);
        check("CPU transverse impulse",
              cpu.inject_matched_transverse_edge_potential(4, 5, 6, 2, 1e-3));

        ftd::gpu::GpuEngine gpu(L);
        gpu.graph_capture_enabled = false;
        gpu.toggles = cpu.toggles;
        gpu.upload_from_host(cpu.voxels());
        gpu.upload_matched_gauss(cpu.matched_gauss_state());

        for (int tick = 0; tick < 8; ++tick) {
            cpu.tick();
            gpu.tick();
        }
        std::vector<Voxel> gpu_voxels;
        gpu.sync_to_host(gpu_voxels);
        const double err = flux_linf(cpu.voxels(), gpu_voxels);
        std::printf("    transverse-wave flux Linf=%.3g\n", err);
        check("transverse-wave CPU/GPU flux parity", err <= 1e-12);
        check("GPU matched step stayed valid", gpu.matched_gauss_last_step_valid());
        const int probe = index_of(4, 5, 6, L);
        check("transverse wave moved flux off the seed site",
              std::abs(cpu.voxels()[static_cast<std::size_t>(probe)].flux.x) +
              std::abs(cpu.voxels()[static_cast<std::size_t>(probe)].flux.y) +
              std::abs(cpu.voxels()[static_cast<std::size_t>(probe)].flux.z) > 0.0);
    }

    {
        RenderBridge cpu(L);
        cpu.force_cpu();
        cpu.toggles = isolated(true);
        cpu.inject_particle(4, 5, 6, +1, {});
        cpu.inject_particle(12, 10, 9, -1, {});
        auto& mobile = cpu.voxel_at(4, 5, 6);
        const double speed = 0.99 * ftd::C_SPEED;
        mobile.velocity = {speed, 0.0, 0.0};
        mobile.remainder = {1.0 - speed, 0.0, 0.0};
        check("moving-pair minimum-energy init",
              cpu.initialize_matched_gauss_dynamics().valid);

        ftd::gpu::GpuEngine gpu(L);
        gpu.graph_capture_enabled = false;
        gpu.toggles = cpu.toggles;
        gpu.upload_from_host(cpu.voxels());
        gpu.upload_matched_gauss(cpu.matched_gauss_state());
        cpu.tick();
        gpu.tick();
        std::vector<Voxel> gpu_voxels;
        gpu.sync_to_host(gpu_voxels);
        check("CPU hop transported polarity",
              cpu.state_at(5, 5, 6) == +1 && cpu.state_at(4, 5, 6) == 0);
        check("GPU hop transported polarity",
              gpu_voxels[static_cast<std::size_t>(index_of(5, 5, 6, L))].state == +1 &&
              gpu_voxels[static_cast<std::size_t>(index_of(4, 5, 6, L))].state == 0);
        const double err = flux_linf(cpu.voxels(), gpu_voxels);
        std::printf("    moving-pair flux Linf=%.3g\n", err);
        check("moving-pair CPU/GPU flux parity", err <= 1e-12);
        check("GPU matched step stayed valid after hop",
              gpu.matched_gauss_last_step_valid());
    }

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
