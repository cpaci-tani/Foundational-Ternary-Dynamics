/**
 * @file test_gpu_eft_parity.cpp
 * @brief GPU vs CPU parity tests for the EFT operators and blocking map.
 */

#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/gpu_buffers.h"
#include "ftd/eft/gpu_dual_cell_fields.cuh"
#include "ftd/eft/reaction_operators.h"
#include "ftd/constants.h"
#include <cstdio>
#include <cmath>
#include <vector>

using namespace ftd;

// Forward declare CPU helper from campaign sweeps to avoid duplicate implementation
inline double cell_J(const ftd::eft::DualCellFields& f, int x, int y, int z, int axis) {
    const int L = f.L;
    auto wrap_idx = [L](int i) { return ((i % L) + L) % L; };
    if (axis == 0) return 0.5 * (f.phi_x[f.index(x, y, z)] + f.phi_x[f.index(wrap_idx(x - 1), y, z)]);
    if (axis == 1) return 0.5 * (f.phi_y[f.index(x, y, z)] + f.phi_y[f.index(x, wrap_idx(y - 1), z)]);
    return 0.5 * (f.phi_z[f.index(x, y, z)] + f.phi_z[f.index(x, y, wrap_idx(z - 1))]);
}

static inline double op_J2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double Jx = cell_J(f, x, y, z, 0);
    const double Jy = cell_J(f, x, y, z, 1);
    const double Jz = cell_J(f, x, y, z, 2);
    return Jx * Jx + Jy * Jy + Jz * Jz;
}

static inline double op_divJ2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double d = ftd::eft::div_face_at(f, x, y, z);
    return d * d;
}

static inline double op_curlJ2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const int L = f.L;
    auto wrap_idx = [L](int i) { return ((i % L) + L) % L; };
    const double dJz_dy = 0.5 * (cell_J(f, x, wrap_idx(y + 1), z, 2) - cell_J(f, x, wrap_idx(y - 1), z, 2));
    const double dJy_dz = 0.5 * (cell_J(f, x, y, wrap_idx(z + 1), 1) - cell_J(f, x, y, wrap_idx(z - 1), 1));
    const double cx = dJz_dy - dJy_dz;
    const double dJx_dz = 0.5 * (cell_J(f, x, y, wrap_idx(z + 1), 0) - cell_J(f, x, y, wrap_idx(z - 1), 0));
    const double dJz_dx = 0.5 * (cell_J(f, wrap_idx(x + 1), y, z, 2) - cell_J(f, wrap_idx(x - 1), y, z, 2));
    const double cy = dJx_dz - dJz_dx;
    const double dJy_dx = 0.5 * (cell_J(f, wrap_idx(x + 1), y, z, 1) - cell_J(f, wrap_idx(x - 1), y, z, 1));
    const double dJx_dy = 0.5 * (cell_J(f, x, wrap_idx(y + 1), z, 0) - cell_J(f, x, wrap_idx(y - 1), z, 0));
    const double cz = dJy_dx - dJx_dy;
    return cx * cx + cy * cy + cz * cz;
}

static inline double op_JdotDivJ(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const int L = f.L;
    auto wrap_idx = [L](int i) { return ((i % L) + L) % L; };
    const double Jx = cell_J(f, x, y, z, 0);
    const double Jy = cell_J(f, x, y, z, 1);
    const double Jz = cell_J(f, x, y, z, 2);
    const double gx = 0.5 * (ftd::eft::div_face_at(f, wrap_idx(x + 1), y, z) -
                             ftd::eft::div_face_at(f, wrap_idx(x - 1), y, z));
    const double gy = 0.5 * (ftd::eft::div_face_at(f, x, wrap_idx(y + 1), z) -
                             ftd::eft::div_face_at(f, x, wrap_idx(y - 1), z));
    const double gz = 0.5 * (ftd::eft::div_face_at(f, x, y, wrap_idx(z + 1)) -
                             ftd::eft::div_face_at(f, x, y, wrap_idx(z - 1)));
    return Jx * gx + Jy * gy + Jz * gz;
}

static inline double op_J4(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double j2 = op_J2(f, x, y, z);
    return j2 * j2;
}

static inline double op_stateSq(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double s = static_cast<double>(f.rho_cell[f.index(x, y, z)]);
    return s * s;
}

static std::array<double, 10> mean_operators_pair_cpu(const ftd::eft::SnapshotPair& p) {
    std::array<double, 10> acc{};
    const int L = p.L();
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                acc[0] += op_J2(p.before, x, y, z);
                acc[1] += op_divJ2(p.before, x, y, z);
                acc[2] += op_curlJ2(p.before, x, y, z);
                acc[3] += op_JdotDivJ(p.before, x, y, z);
                acc[4] += op_J4(p.before, x, y, z);
                acc[5] += op_stateSq(p.before, x, y, z);
                acc[6] += ftd::eft::op_reactionDensity(p, x, y, z);
                acc[7] += ftd::eft::op_genesisFlux(p, x, y, z);
                acc[8] += ftd::eft::op_evapFlux(p, x, y, z);
                acc[9] += ftd::eft::op_JdotDeltaS(p, x, y, z);
            }
        }
    }
    const double inv_N = 1.0 / static_cast<double>(L * L * L);
    for (int a = 0; a < 10; ++a) acc[a] *= inv_N;
    return acc;
}

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { tests_passed++; std::printf("  PASS: %s\n", msg); } \
    else { tests_failed++; std::printf("  FAIL: %s\n", msg); } \
} while(0)

#define CHECK_CLOSE(a, b, tol, msg) do { \
    double _a = (a), _b = (b), _t = (tol); \
    if (std::abs(_a - _b) <= _t) { tests_passed++; std::printf("  PASS: %s (%.6e vs %.6e, diff=%.2e)\n", msg, _a, _b, std::abs(_a-_b)); } \
    else { tests_failed++; std::printf("  FAIL: %s (%.6e vs %.6e, diff=%.2e > tol %.2e)\n", msg, _a, _b, std::abs(_a-_b), _t); } \
} while(0)

int main() {
    std::printf("============================================================\n");
    std::printf("  FTD GPU vs CPU EFT Operator Parity Tests\n");
    std::printf("============================================================\n");

    constexpr int L = 16;
    gpu::GpuEngine gpu(L);
    RenderBridge cpu(L);
    cpu.force_cpu();

    // Disable stochastic loops & forces to get bit-exact wave parity
    auto configure = [](auto& eng) {
        eng.toggles.disable_all();
        eng.toggles.wave_propagation = true;
        eng.toggles.damping = true;
    };
    configure(gpu);
    configure(cpu);

    // Setup identical initial conditions with random states & fluxes
    std::vector<Voxel> host_init(L * L * L);
    std::vector<Voxel> host_after(L * L * L);
    for (int i = 0; i < L * L * L; ++i) {
        host_init[i].state = (i % 7 == 0) ? 1 : ((i % 11 == 0) ? -1 : 0);
        host_init[i].flux = Vec3(sin(i * 0.1), cos(i * 0.15), sin(i * 0.2) * 0.5);

        // Transition: simulate some evaporation (state -> 0) and genesis (0 -> state)
        host_after[i].state = (i % 5 == 0) ? 1 : ((i % 13 == 0) ? -1 : 0);
        host_after[i].flux = Vec3(sin(i * 0.12), cos(i * 0.17), sin(i * 0.22) * 0.5);
    }

    // CPU Before Setup
    cpu.voxels() = host_init;
    const auto cpu_before = ftd::eft::render_bridge_to_dual_cell_fields(cpu);
    
    // GPU Before Setup
    gpu.upload_from_host(host_init);
    eft::gpu::GpuDualCellFields gpu_before;
    gpu_before.allocate(L);
    eft::gpu::gpu_render_bridge_to_dual_cell_fields(gpu.bufs(), gpu_before);

    // CPU After Setup
    cpu.voxels() = host_after;
    const auto cpu_after = ftd::eft::render_bridge_to_dual_cell_fields(cpu);

    // GPU After Setup
    gpu.upload_from_host(host_after);
    eft::gpu::GpuDualCellFields gpu_after;
    gpu_after.allocate(L);
    eft::gpu::gpu_render_bridge_to_dual_cell_fields(gpu.bufs(), gpu_after);

    // Coarse-grain (b=2)
    const auto cpu_c2_before = ftd::eft::block_dual_cell_b2(cpu_before);
    const auto cpu_c2_after = ftd::eft::block_dual_cell_b2(cpu_after);

    eft::gpu::GpuDualCellFields gpu_c2_before;
    gpu_c2_before.allocate(L / 2);
    eft::gpu::gpu_block_dual_cell_b2(gpu_before, gpu_c2_before);

    eft::gpu::GpuDualCellFields gpu_c2_after;
    gpu_c2_after.allocate(L / 2);
    eft::gpu::gpu_block_dual_cell_b2(gpu_after, gpu_c2_after);

    // Evaluate Fine Operators (CPU vs GPU)
    ftd::eft::SnapshotPair cpu_pair_fine{cpu_before, cpu_after};
    eft::gpu::GpuSnapshotPair gpu_pair_fine{gpu_before, gpu_after};

    auto cpu_fine_means = mean_operators_pair_cpu(cpu_pair_fine);
    double gpu_fine_means[10] = {0.0};
    eft::gpu::gpu_compute_eft_means(gpu_pair_fine, gpu_fine_means);

    std::printf("\n--- Fine Scale Operator Parity (L=16) ---\n");
    for (int a = 0; a < 10; ++a) {
        char msg[64];
        std::sprintf(msg, "Op O%d Fine Parity", a + 1);
        CHECK_CLOSE(gpu_fine_means[a], cpu_fine_means[a], 1e-12, msg);
    }

    // Evaluate Coarse Operators (CPU vs GPU)
    ftd::eft::SnapshotPair cpu_pair_c2{cpu_c2_before, cpu_c2_after};
    eft::gpu::GpuSnapshotPair gpu_pair_c2{gpu_c2_before, gpu_c2_after};

    auto cpu_c2_means = mean_operators_pair_cpu(cpu_pair_c2);
    double gpu_c2_means[10] = {0.0};
    eft::gpu::gpu_compute_eft_means(gpu_pair_c2, gpu_c2_means);

    std::printf("\n--- Coarse Scale Operator Parity (Lc=8) ---\n");
    for (int a = 0; a < 10; ++a) {
        char msg[64];
        std::sprintf(msg, "Op O%d Coarse Parity", a + 1);
        CHECK_CLOSE(gpu_c2_means[a], cpu_c2_means[a], 1e-12, msg);
    }

    // Free device structures
    gpu_before.free();
    gpu_after.free();
    gpu_c2_before.free();
    gpu_c2_after.free();

    std::printf("\n============================================================\n");
    std::printf("  EFT GPU Parity Results: %d passed, %d failed\n", tests_passed, tests_failed);
    std::printf("============================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
