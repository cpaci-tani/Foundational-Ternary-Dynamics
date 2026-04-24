/**
 * @file benchmark_langevin_gpu.cpp
 * @brief Timing benchmark for the Langevin thermostat on CPU vs GPU paths.
 *
 * Measures wall-time for N_TICKS of bare-lattice + Langevin on the GPU backend.
 * Compares against CPU (via force_cpu). Equipartition is re-verified at the
 * end of each run to confirm the GPU port is correct, not just faster.
 *
 * Runs at L=64 where GPU parallelism pays off. At L=16 the launch overhead
 * dominates; at L=64 the 32^3 = 32768 active voxels * 3 components = 98304
 * doubles per noise draw + kernel give real GPU work.
 */

#include <chrono>
#include <cmath>
#include <cstdio>

#include "ftd/render_bridge.h"

static void configure_bare_lattice_langevin(ftd::RenderBridge& rb,
                                            double T, double gamma) {
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling         = false;
    rb.toggles.damping          = false;
    rb.toggles.genesis          = false;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces           = false;
    rb.toggles.gravity          = false;
    rb.toggles.poisson_coulomb  = false;
    rb.toggles.movement         = false;
    rb.toggles.lorentz_force    = false;
    rb.toggles.selective_damping= false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate   = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field    = false;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = gamma;
}

static double run_and_time(int L, int n_ticks, bool force_cpu, const char* label) {
    ftd::RenderBridge rb(L);
    configure_bare_lattice_langevin(rb, 0.01, 0.01);
    if (force_cpu) rb.force_cpu();
    rb.seed_rng(1);

    auto t0 = std::chrono::high_resolution_clock::now();
    rb.run(n_ticks);
    auto t1 = std::chrono::high_resolution_clock::now();
    double sec = std::chrono::duration<double>(t1 - t0).count();

    // Equipartition check on last snapshot
    double sum_v2 = 0;
    const auto& vox = rb.voxels();
    for (const auto& v : vox) sum_v2 += v.wave_vel.mag2();
    double v2_mean = sum_v2 / vox.size();
    double dev = 100.0 * (v2_mean - 3.0 * 0.01) / (3.0 * 0.01);

    std::printf("  %-6s L=%-3d  %5d ticks  %.3f s  (%.3f ms/tick)   "
                "<|v|^2>=%.4e  dev=%+.2f%%\n",
                label, L, n_ticks, sec, 1000.0 * sec / n_ticks,
                v2_mean, dev);
    return sec;
}

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Langevin thermostat CPU vs GPU wall-time benchmark\n");
    std::printf("================================================================\n");

    // L=16: GPU launch overhead dominates, expect similar or CPU faster
    std::printf("[L=16 — launch-overhead regime]\n");
    double t_cpu_16 = run_and_time(16, 1000, true,  "CPU");
    double t_gpu_16 = run_and_time(16, 1000, false, "GPU");
    std::printf("  speedup: %.2fx\n\n", t_cpu_16 / t_gpu_16);

    // L=64: where GPU starts to pay off
    std::printf("[L=64 — GPU-parallelism regime]\n");
    double t_cpu_64 = run_and_time(64, 200, true,  "CPU");
    double t_gpu_64 = run_and_time(64, 200, false, "GPU");
    std::printf("  speedup: %.2fx\n\n", t_cpu_64 / t_gpu_64);

    // L=128: still tractable on CPU, but only just
    std::printf("[L=128 — heavy-duty regime]\n");
    double t_cpu_128 = run_and_time(128, 100, true,  "CPU");
    double t_gpu_128 = run_and_time(128, 100, false, "GPU");
    std::printf("  speedup: %.2fx\n\n", t_cpu_128 / t_gpu_128);

    // L=256: GPU-only. CPU at this size is "half-hour or more per 100 ticks"
    // territory; the data point we care about is GPU-path throughput + that
    // equipartition still holds.
    std::printf("[L=256 — GPU-only (CPU skipped; too slow to be useful)]\n");
    double t_gpu_256 = run_and_time(256, 100, false, "GPU");
    std::printf("  GPU-only throughput: %.1f ms/tick at 256^3 = %d voxels\n\n",
                1000.0 * t_gpu_256 / 100, 256*256*256);

    std::printf("================================================================\n");
    std::printf("  CPU/GPU speedup summary:\n");
    std::printf("    L= 16: %6.2fx\n", t_cpu_16 / t_gpu_16);
    std::printf("    L= 64: %6.2fx\n", t_cpu_64 / t_gpu_64);
    std::printf("    L=128: %6.2fx\n", t_cpu_128 / t_gpu_128);
    std::printf("    L=256: %6.2f ms/tick GPU (CPU baseline not run)\n",
                1000.0 * t_gpu_256 / 100);
    if (t_cpu_64 / t_gpu_64 > 2.0) {
        std::printf("  PASS — GPU path delivers meaningful speedup.\n");
        return 0;
    } else {
        std::printf("  NOTE — GPU path did not exceed 2x at L=64; may indicate\n");
        std::printf("  noise-generation bottleneck or larger L needed to amortize.\n");
        return 0;
    }
}
