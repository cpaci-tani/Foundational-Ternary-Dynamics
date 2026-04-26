/**
 * Performance benchmark for the FTD engine.
 *
 * Measures per-tick timing at different lattice sizes and particle counts.
 * Uses the toggle system to isolate individual phase contributions.
 *
 * Output: human-readable timing report on stdout.
 * CTest pass criterion: completes without error (no assertions).
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>  // RF-9: render_bridge.h no longer transitively pulls this in

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace ftd;
using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::duration<double, std::milli>;

// Run N ticks and return average time in ms
static double bench_ticks(RenderBridge& bridge, int n_ticks) {
    // Warm up
    bridge.tick();

    auto t0 = Clock::now();
    for (int i = 0; i < n_ticks; ++i) {
        bridge.tick();
    }
    auto t1 = Clock::now();
    return Ms(t1 - t0).count() / n_ticks;
}

// Inject N random particles into the lattice
static void inject_particles(RenderBridge& bridge, int count, int seed = 42) {
    std::mt19937 rng(seed);
    int L = bridge.lattice().size();
    std::uniform_int_distribution<int> pos_dist(1, L - 2);
    std::uniform_int_distribution<int> sign_dist(0, 1);

    for (int i = 0; i < count; ++i) {
        int x = pos_dist(rng);
        int y = pos_dist(rng);
        int z = pos_dist(rng);
        int8_t state = sign_dist(rng) ? +1 : -1;
        bridge.inject_particle(x, y, z, state,
                               Vec3(0.0, 0.0, K_B * state),
                               state, static_cast<int8_t>(1 + (i % 3)));
    }
}

// Measure contribution of a single toggle by comparing on vs off
struct PhaseTiming {
    const char* name;
    double ms_with;   // tick time with this phase enabled
    double ms_without; // tick time with this phase disabled
    double ms_delta;   // contribution = with - without
};

static PhaseTiming measure_phase(int lattice_size, int n_particles,
                                  const char* name,
                                  void (*toggle_off)(TermToggles&),
                                  int n_ticks = 20) {
    PhaseTiming result;
    result.name = name;

    // With phase ON (all defaults)
    {
        RenderBridge bridge(lattice_size);
        inject_particles(bridge, n_particles);
        bridge.toggles.enable_all();
        result.ms_with = bench_ticks(bridge, n_ticks);
    }

    // With phase OFF
    {
        RenderBridge bridge(lattice_size);
        inject_particles(bridge, n_particles);
        bridge.toggles.enable_all();
        toggle_off(bridge.toggles);
        result.ms_without = bench_ticks(bridge, n_ticks);
    }

    result.ms_delta = result.ms_with - result.ms_without;
    if (result.ms_delta < 0.0) result.ms_delta = 0.0;
    return result;
}

int main() {
    std::printf("============================================================\n");
    std::printf("  FTD Engine Performance Benchmark\n");
    std::printf("============================================================\n\n");

#ifdef _OPENMP
    std::printf("  OpenMP: ENABLED (%d threads available)\n", omp_get_max_threads());
#else
    std::printf("  OpenMP: DISABLED (serial execution)\n");
#endif

    std::printf("  sizeof(Voxel) = %zu bytes\n", sizeof(Voxel));
    std::printf("  sizeof(Vec3)  = %zu bytes\n", sizeof(Vec3));
    std::printf("\n");

    // --- Section 1: Full tick timing at different lattice sizes ---
    std::printf("--- Full Tick Timing (all physics, 0 particles) ---\n");
    std::printf("  %-10s  %10s  %12s  %10s\n", "Lattice", "ms/tick", "Voxels", "MB");
    std::printf("  %-10s  %10s  %12s  %10s\n", "-------", "-------", "------", "--");

    int sizes[] = {8, 16, 24, 32, 48, 64};
    for (int L : sizes) {
        int N = L * L * L;
        double mb = N * sizeof(Voxel) / (1024.0 * 1024.0);

        RenderBridge bridge(L);
        double ms = bench_ticks(bridge, 30);
        std::printf("  %3d^3       %10.2f  %12d  %10.1f\n", L, ms, N, mb);
    }
    std::printf("\n");

    // --- Section 2: Scaling with particle count ---
    std::printf("--- Particle Count Scaling (32^3 lattice) ---\n");
    std::printf("  %-12s  %10s\n", "Particles", "ms/tick");
    std::printf("  %-12s  %10s\n", "---------", "-------");

    int particle_counts[] = {0, 10, 50, 100, 200, 500};
    for (int np : particle_counts) {
        RenderBridge bridge(32);
        inject_particles(bridge, np);
        double ms = bench_ticks(bridge, 20);
        std::printf("  %5d         %10.2f\n", np, ms);
    }
    std::printf("\n");

    // --- Section 3: Per-phase contribution (32^3, 100 particles) ---
    std::printf("--- Per-Phase Contribution (32^3, 100 particles) ---\n");
    std::printf("  %-25s  %10s\n", "Phase", "ms (est.)");
    std::printf("  %-25s  %10s\n", "-------------------------", "---------");

    struct ToggleSpec {
        const char* name;
        void (*toggle_off)(TermToggles&);
    };

    ToggleSpec phases[] = {
        {"wave_propagation+coupling",
         [](TermToggles& t) { t.wave_propagation = false; t.coupling = false; }},
        {"damping+genesis",
         [](TermToggles& t) { t.damping = false; t.genesis = false; }},
        {"gauss_projection",
         [](TermToggles& t) { t.gauss_projection = false; }},
        {"forces",
         [](TermToggles& t) { t.forces = false; }},
        {"movement",
         [](TermToggles& t) { t.movement = false; }},
    };

    for (auto& spec : phases) {
        auto pt = measure_phase(32, 100, spec.name, spec.toggle_off, 15);
        std::printf("  %-25s  %10.2f\n", pt.name, pt.ms_delta);
    }

    // Total for reference
    {
        RenderBridge bridge(32);
        inject_particles(bridge, 100);
        double ms = bench_ticks(bridge, 15);
        std::printf("  %-25s  %10.2f\n", "--- TOTAL ---", ms);
    }
    std::printf("\n");

    // --- Section 4: Force scaling test ---
    std::printf("--- Force Phase Scaling (32^3, forces-only delta) ---\n");
    std::printf("  %-12s  %10s\n", "Particles", "ms forces");
    std::printf("  %-12s  %10s\n", "---------", "---------");

    int force_counts[] = {10, 50, 100, 200, 500};
    for (int np : force_counts) {
        auto pt = measure_phase(32, np, "forces",
                                [](TermToggles& t) { t.forces = false; }, 10);
        std::printf("  %5d         %10.2f\n", np, pt.ms_delta);
    }
    std::printf("\n");

    // --- Section 5: Memory footprint ---
    std::printf("--- Memory Footprint ---\n");
    std::printf("  %-10s  %10s  %10s  %10s  %10s\n",
                "Lattice", "Voxels", "ForceDiag", "Buffers", "Total");
    std::printf("  %-10s  %10s  %10s  %10s  %10s\n",
                "-------", "------", "---------", "-------", "-----");

    int mem_sizes[] = {32, 48, 64, 96, 128};
    for (int L : mem_sizes) {
        long long N = (long long)L * L * L;
        double voxel_mb = N * sizeof(Voxel) / (1024.0 * 1024.0);
        double fdiag_mb = N * sizeof(ForceDiag) / (1024.0 * 1024.0);
        double buf_mb = N * (sizeof(Vec3) + sizeof(double)) / (1024.0 * 1024.0); // delta_j + phi
        double total_mb = voxel_mb + fdiag_mb + buf_mb;
        std::printf("  %3d^3       %8.1f MB  %8.1f MB  %8.1f MB  %8.1f MB\n",
                    L, voxel_mb, fdiag_mb, buf_mb, total_mb);
    }
    std::printf("\n");

    // --- Section 6: Interactive performance summary ---
    std::printf("--- Interactive Performance (estimated Hz) ---\n");
    std::printf("  %-10s  %10s  %10s  %10s\n",
                "Lattice", "ms/tick", "Hz", "Status");
    std::printf("  %-10s  %10s  %10s  %10s\n",
                "-------", "-------", "------", "------");

    int hz_sizes[] = {32, 48, 64};
    for (int L : hz_sizes) {
        RenderBridge bridge(L);
        inject_particles(bridge, 20);
        double ms = bench_ticks(bridge, 10);
        double hz = 1000.0 / ms;
        const char* status = hz > 60.0 ? "REALTIME" :
                             hz > 10.0 ? "INTERACTIVE" :
                             hz > 1.0  ? "BATCH" : "SLOW";
        std::printf("  %3d^3       %10.2f  %8.0f Hz  %10s\n", L, ms, hz, status);
    }
    std::printf("\n");

    std::printf("============================================================\n");
    std::printf("  Benchmark complete.\n");
    std::printf("============================================================\n");

    return 0;
}
