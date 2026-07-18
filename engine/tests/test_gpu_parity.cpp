/**
 * GPU vs CPU parity tests for the FTD CUDA engine.
 *
 * Verifies that GpuEngine produces identical (or near-identical) results
 * to the CPU RenderBridge for the same initial conditions.
 *
 * Tests:
 *   GP1: SoA upload/download round-trip preserves all fields
 *   GP2: Single tick with one particle — GPU vs CPU match
 *   GP3: Multi-tick vacuum wave propagation — max difference < tolerance
 *   GP4: Wavepacket injection parity
 *   GP5: Energy audit parity after 100 ticks
 *   GP6: Gauss projection parity (FFT vs SOR)
 */

#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include <cstdio>
#include <cmath>
#include <vector>

using namespace ftd;

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

// ============================================================
// GP1: SoA round-trip (upload → download preserves all fields)
// ============================================================
static void test_soa_roundtrip() {
    std::printf("\n--- GP1: SoA Upload/Download Round-Trip ---\n");
    constexpr int L = 16;

    gpu::GpuEngine gpu(L);

    // Inject a particle with known values
    Vec3 flux_val(0.1, -0.2, 0.3);
    gpu.inject_particle(5, 6, 7, +1, flux_val, +1, 2);

    // Download and verify
    std::vector<Voxel> voxels;
    gpu.sync_to_host(voxels);

    int idx = 5 * L * L + 6 * L + 7;  // x-major: x*L² + y*L + z
    const auto& v = voxels[idx];

    CHECK(v.state == +1, "State preserved");
    CHECK_CLOSE(v.flux.x, 0.1, 1e-15, "Flux.x preserved");
    CHECK_CLOSE(v.flux.y, -0.2, 1e-15, "Flux.y preserved");
    CHECK_CLOSE(v.flux.z, 0.3, 1e-15, "Flux.z preserved");
    CHECK(v.spin == +1, "Spin preserved");
    CHECK(v.color == 2, "Color preserved");
    CHECK(v.particle_id >= 0, "Particle ID assigned");

    // Verify void sites are zero
    int void_idx = 0;
    const auto& v0 = voxels[void_idx];
    CHECK(v0.state == 0, "Void site state is 0");
    CHECK_CLOSE(v0.flux.mag(), 0.0, 1e-15, "Void site flux is zero");
}

// ============================================================
// GP2: Single-tick parity (GPU vs CPU with one particle)
// ============================================================
static void test_single_tick_parity() {
    std::printf("\n--- GP2: Single Tick Parity (1 particle) ---\n");
    constexpr int L = 32;

    // CPU engine (force CPU-only so we get a true CPU reference)
    RenderBridge cpu(L);
    cpu.force_cpu();
    cpu.toggles.enable_all();
    cpu.toggles.genesis = false;       // Disable stochastic processes
    cpu.toggles.movement = false;      // Keep particle stationary
    // Converge the CPU Gauss/Poisson solve: at the default iteration count
    // the SOR solution around a freshly injected point charge is still far
    // from the converged field (GP6 measures the truncation directly), and
    // the one-tick field energy read ~6% above the FFT-exact GPU value.
    // With a converged CPU reference the comparison is solver-exact vs
    // solver-exact and the 2% family tolerance is honest.
    cpu.set_sor_iterations(500);
    cpu.inject_particle(L/2, L/2, L/2, +1,
                        Vec3(0, 0, K_B), +1, 1);

    // GPU engine
    gpu::GpuEngine gpu(L);
    gpu.toggles.enable_all();
    gpu.toggles.genesis = false;
    gpu.toggles.movement = false;
    gpu.inject_particle(L/2, L/2, L/2, +1,
                        Vec3(0, 0, K_B), +1, 1);

    // Run 1 tick each
    cpu.tick();
    gpu.tick();

    // Compare energy audits
    auto cpu_ea = cpu.energy_audit();
    auto gpu_ea = gpu.energy_audit();

    // FFT Poisson (GPU) is more accurate than SOR (CPU), so this is an
    // implementation-family parity check rather than a bitwise oracle.
    double tol = 0.02;  // 2% tolerance for Gauss/Coulomb solver differences
    CHECK_CLOSE(gpu_ea.field_energy, cpu_ea.field_energy,
                cpu_ea.field_energy * tol + 1e-10, "Field energy match");
    CHECK_CLOSE(gpu_ea.wave_energy, cpu_ea.wave_energy,
                cpu_ea.wave_energy * tol + 1e-10, "Wave energy match");
    CHECK(gpu_ea.manifested_count == cpu_ea.manifested_count,
          "Particle count match");
    CHECK(gpu_ea.charge_total == cpu_ea.charge_total,
          "Charge conservation match");
}

// ============================================================
// GP3: Vacuum wave propagation (100 ticks, no particles)
// ============================================================
static void test_vacuum_wave_parity() {
    std::printf("\n--- GP3: Vacuum Wave Propagation (100 ticks) ---\n");
    constexpr int L = 32;

    // Set up identical initial conditions: flux pulse at center
    auto setup = [&](auto& engine) {
        // Disable particles and Gauss/Coulomb (pure wave test)
        engine.toggles.disable_all();
        engine.toggles.wave_propagation = true;
        engine.toggles.damping = true;
        // Explicitly OFF: dual_substrate, coupling, genesis, gauss, forces, movement
        // This ensures single-substrate wave equation: flux is propagated directly.
    };

    RenderBridge cpu(L);
    cpu.force_cpu();
    setup(cpu);
    // Inject flux pulse at center (no manifested state, just flux)
    int cx = L/2, cy = L/2, cz = L/2;
    int cidx = cpu.lattice().index(cx, cy, cz);
    cpu.voxels()[cidx].flux = Vec3(0, 0, 1.0);

    gpu::GpuEngine gpu(L);
    setup(gpu);
    // Must inject into host shadow and push
    gpu.inject_particle(cx, cy, cz, 0, Vec3(0, 0, 1.0), 0, 0);

    // Run 100 ticks
    cpu.run(100);
    gpu.run(100);

    // Compare total energy
    auto cpu_ea = cpu.energy_audit();
    auto gpu_ea = gpu.energy_audit();

    CHECK_CLOSE(gpu_ea.total_energy, cpu_ea.total_energy,
                cpu_ea.total_energy * 0.001 + 1e-12,
                "Total energy match (100 ticks vacuum)");

    // Download full state and compare max difference
    std::vector<Voxel> gpu_voxels;
    gpu.sync_to_host(gpu_voxels);

    double max_flux_diff = 0.0;
    int N = L * L * L;
    for (int i = 0; i < N; ++i) {
        double dx = std::abs(gpu_voxels[i].flux.x - cpu.voxels()[i].flux.x);
        double dy = std::abs(gpu_voxels[i].flux.y - cpu.voxels()[i].flux.y);
        double dz = std::abs(gpu_voxels[i].flux.z - cpu.voxels()[i].flux.z);
        double d = std::max({dx, dy, dz});
        if (d > max_flux_diff) max_flux_diff = d;
    }

    std::printf("  INFO: Max flux component difference = %.6e\n", max_flux_diff);
    CHECK(max_flux_diff < 1e-8, "Flux field parity within 1e-8");
}

// ============================================================
// GP4: Wavepacket injection parity
// ============================================================
static void test_wavepacket_parity() {
    std::printf("\n--- GP4: Wavepacket Injection Parity ---\n");
    constexpr int L = 32;

    RenderBridge cpu(L);
    cpu.force_cpu();
    cpu.inject_wavepacket(L/2, L/2, L/2, +1, 3.0, K_B);

    gpu::GpuEngine gpu(L);
    gpu.inject_wavepacket(L/2, L/2, L/2, +1, 3.0, K_B);

    // Compare energy
    auto cpu_ea = cpu.energy_audit();
    auto gpu_ea = gpu.energy_audit();

    CHECK_CLOSE(gpu_ea.field_energy, cpu_ea.field_energy,
                cpu_ea.field_energy * 0.001 + 1e-12,
                "Wavepacket field energy match");
    CHECK(gpu_ea.manifested_count == cpu_ea.manifested_count,
          "Wavepacket manifested count match");
}

// ============================================================
// GP5: Energy audit after 100 ticks with particle
// ============================================================
static void test_energy_audit_parity() {
    std::printf("\n--- GP5: Energy Audit Parity (100 ticks, 1 particle) ---\n");
    constexpr int L = 32;

    auto setup = [](auto& engine) {
        engine.toggles.enable_all();
        engine.toggles.genesis = false;   // No stochastic genesis
        engine.toggles.movement = false;  // Keep particle in place
    };

    RenderBridge cpu(L);
    cpu.force_cpu();
    setup(cpu);
    cpu.inject_particle(L/2, L/2, L/2, +1, Vec3(0, 0, K_B), +1, 1);

    gpu::GpuEngine gpu(L);
    setup(gpu);
    gpu.inject_particle(L/2, L/2, L/2, +1, Vec3(0, 0, K_B), +1, 1);

    cpu.run(100);
    gpu.run(100);

    auto cpu_ea = cpu.energy_audit();
    auto gpu_ea = gpu.energy_audit();

    // FFT vs SOR will differ, so use 5% tolerance
    double tol = 0.05;
    CHECK_CLOSE(gpu_ea.total_energy, cpu_ea.total_energy,
                cpu_ea.total_energy * tol + 1e-10,
                "Total energy within 5%");
    CHECK(gpu_ea.charge_total == cpu_ea.charge_total,
          "Charge conservation");
    CHECK(gpu_ea.manifested_count == cpu_ea.manifested_count,
          "Particle count");
}

// ============================================================
// GP6: Gauss projection quality (FFT should be better than SOR)
// ============================================================
static void test_gauss_quality() {
    std::printf("\n--- GP6: Gauss Projection Quality (FFT vs SOR) ---\n");
    constexpr int L = 32;

    auto setup = [](auto& engine) {
        engine.toggles.enable_all();
        engine.toggles.genesis = false;
        engine.toggles.movement = false;
    };

    RenderBridge cpu(L);
    cpu.force_cpu();
    setup(cpu);
    cpu.inject_particle(L/2, L/2, L/2, +1, Vec3(0, 0, K_B), +1, 1);

    gpu::GpuEngine gpu(L);
    setup(gpu);
    gpu.inject_particle(L/2, L/2, L/2, +1, Vec3(0, 0, K_B), +1, 1);

    // Run 50 ticks to let Gauss projection work
    cpu.run(50);
    gpu.run(50);

    auto cpu_ea = cpu.energy_audit();
    auto gpu_ea = gpu.energy_audit();

    std::printf("  INFO: CPU Gauss violation = %.6e (SOR, 30 iters)\n", cpu_ea.gauss_violation);
    std::printf("  INFO: GPU Gauss violation = %.6e (FFT, exact)\n", gpu_ea.gauss_violation);

    // FFT should have lower or comparable Gauss violation
    // (It's exact to machine precision, SOR is approximate)
    CHECK(gpu_ea.gauss_violation <= cpu_ea.gauss_violation * 1.1 + 1e-10,
          "GPU Gauss violation <= CPU Gauss violation");
}

// ============================================================
int main() {
    std::printf("============================================================\n");
    std::printf("  FTD GPU vs CPU Parity Tests\n");
    std::printf("============================================================\n");

    test_soa_roundtrip();
    test_single_tick_parity();
    test_vacuum_wave_parity();
    test_wavepacket_parity();
    test_energy_audit_parity();
    test_gauss_quality();

    std::printf("\n============================================================\n");
    std::printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    std::printf("============================================================\n");

    return tests_failed > 0 ? 1 : 0;
}
