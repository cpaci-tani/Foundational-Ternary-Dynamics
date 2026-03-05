/**
 * GPU performance benchmark for the FTD CUDA engine.
 *
 * Measures GPU tick timing and compares against CPU RenderBridge.
 * Reports speedup ratios for different lattice sizes and configurations.
 *
 * CTest pass criterion: completes without error (no assertions).
 */

#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include <chrono>
#include <cstdio>
#include <vector>

using namespace ftd;
using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::duration<double, std::milli>;

// Benchmark CPU engine
static double bench_cpu(int L, int n_ticks, int n_particles = 0) {
    RenderBridge bridge(L);
    bridge.toggles.enable_all();
    bridge.toggles.genesis = false;

    for (int i = 0; i < n_particles; ++i) {
        int x = 4 + (i * 7) % (L - 8);
        int y = 4 + (i * 11) % (L - 8);
        int z = 4 + (i * 13) % (L - 8);
        int8_t s = (i % 2) ? +1 : -1;
        bridge.inject_particle(x, y, z, s, Vec3(0, 0, K_B * s), s, 1);
    }

    // Warm up
    bridge.tick();

    auto t0 = Clock::now();
    for (int i = 0; i < n_ticks; ++i) bridge.tick();
    auto t1 = Clock::now();

    return Ms(t1 - t0).count() / n_ticks;
}

// Benchmark GPU engine
static double bench_gpu(int L, int n_ticks, int n_particles = 0) {
    gpu::GpuEngine engine(L);
    engine.toggles.enable_all();
    engine.toggles.genesis = false;

    for (int i = 0; i < n_particles; ++i) {
        int x = 4 + (i * 7) % (L - 8);
        int y = 4 + (i * 11) % (L - 8);
        int z = 4 + (i * 13) % (L - 8);
        int8_t s = (i % 2) ? +1 : -1;
        engine.inject_particle(x, y, z, s, Vec3(0, 0, K_B * s), s, 1);
    }

    // Warm up (includes kernel JIT if needed)
    engine.tick();
    engine.tick();

    auto t0 = Clock::now();
    for (int i = 0; i < n_ticks; ++i) engine.tick();
    // Synchronize to ensure all GPU work is done
    cudaDeviceSynchronize();
    auto t1 = Clock::now();

    return Ms(t1 - t0).count() / n_ticks;
}

int main() {
    std::printf("============================================================\n");
    std::printf("  FTD GPU vs CPU Performance Benchmark\n");
    std::printf("============================================================\n\n");

    // Print GPU info
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    std::printf("  GPU: %s (SM %d.%d, %.0f MB VRAM)\n",
                prop.name, prop.major, prop.minor,
                prop.totalGlobalMem / (1024.0 * 1024.0));
    std::printf("  SMs: %d, Max threads/SM: %d\n",
                prop.multiProcessorCount, prop.maxThreadsPerMultiProcessor);
    std::printf("\n");

    // --- Section 1: Lattice size scaling (vacuum, no particles) ---
    std::printf("--- Lattice Size Scaling (vacuum, all physics) ---\n");
    std::printf("  %-8s  %10s  %10s  %10s  %8s\n",
                "Lattice", "CPU ms", "GPU ms", "Speedup", "Voxels");
    std::printf("  %-8s  %10s  %10s  %10s  %8s\n",
                "-------", "------", "------", "-------", "------");

    int sizes[] = {16, 32, 48, 64};
    for (int L : sizes) {
        int n = (L <= 32) ? 50 : 20;
        double cpu_ms = bench_cpu(L, n);
        double gpu_ms = bench_gpu(L, n);
        double speedup = cpu_ms / gpu_ms;
        std::printf("  %3d^3     %10.2f  %10.2f  %9.1fx  %8d\n",
                    L, cpu_ms, gpu_ms, speedup, L*L*L);
    }
    std::printf("\n");

    // --- Section 2: With particles ---
    std::printf("--- With Particles (32^3 lattice) ---\n");
    std::printf("  %-10s  %10s  %10s  %10s\n",
                "Particles", "CPU ms", "GPU ms", "Speedup");
    std::printf("  %-10s  %10s  %10s  %10s\n",
                "---------", "------", "------", "-------");

    int pcounts[] = {0, 10, 50, 100};
    for (int np : pcounts) {
        double cpu_ms = bench_cpu(32, 30, np);
        double gpu_ms = bench_gpu(32, 30, np);
        double speedup = cpu_ms / gpu_ms;
        std::printf("  %5d       %10.2f  %10.2f  %9.1fx\n",
                    np, cpu_ms, gpu_ms, speedup);
    }
    std::printf("\n");

    // --- Section 3: Per-phase GPU timing ---
    std::printf("--- GPU Per-Phase Timing (64^3, 0 particles, 20 ticks) ---\n");

    struct PhaseConfig {
        const char* name;
        bool wave, coupling, damping, gauss, forces, movement, poisson, lorentz;
    };

    // Cumulative: each adds one more phase
    PhaseConfig configs[] = {
        {"wave+coupling", true, true, false, false, false, false, false, false},
        {"+damping",      true, true, true,  false, false, false, false, false},
        {"+gauss",        true, true, true,  true,  false, false, false, false},
        {"+forces",       true, true, true,  true,  true,  false, true,  false},
        {"+movement",     true, true, true,  true,  true,  true,  true,  false},
        {"+lorentz",      true, true, true,  true,  true,  true,  true,  true},
    };

    std::printf("  %-20s  %10s\n", "Configuration", "GPU ms/tick");
    std::printf("  %-20s  %10s\n", "-------------------", "----------");

    for (auto& cfg : configs) {
        gpu::GpuEngine engine(64);
        engine.toggles.wave_propagation = cfg.wave;
        engine.toggles.coupling = cfg.coupling;
        engine.toggles.damping = cfg.damping;
        engine.toggles.selective_damping = false;
        engine.toggles.genesis = false;
        engine.toggles.gauss_projection = cfg.gauss;
        engine.toggles.forces = cfg.forces;
        engine.toggles.movement = cfg.movement;
        engine.toggles.poisson_coulomb = cfg.poisson;
        engine.toggles.lorentz_force = cfg.lorentz;

        // Warm up
        engine.tick();
        engine.tick();

        auto t0 = Clock::now();
        for (int i = 0; i < 20; ++i) engine.tick();
        cudaDeviceSynchronize();
        auto t1 = Clock::now();

        double ms = Ms(t1 - t0).count() / 20.0;
        std::printf("  %-20s  %10.2f\n", cfg.name, ms);
    }
    std::printf("\n");

    // --- Section 4: Throughput at target size ---
    std::printf("--- Throughput Summary ---\n");
    {
        double gpu64 = bench_gpu(64, 20, 10);
        double cpu64 = bench_cpu(64, 10, 10);
        std::printf("  64^3 + 10 particles:\n");
        std::printf("    CPU: %.2f ms/tick (%.0f Hz)\n", cpu64, 1000.0/cpu64);
        std::printf("    GPU: %.2f ms/tick (%.0f Hz)\n", gpu64, 1000.0/gpu64);
        std::printf("    Speedup: %.1fx\n", cpu64 / gpu64);
    }
    std::printf("\n");

    std::printf("============================================================\n");
    std::printf("  Benchmark complete.\n");
    std::printf("============================================================\n");

    return 0;
}
