/**
 * GPU Shell Battery — Understanding Self-Field Dynamics
 *
 * Runs multiple configurations at 128^3 to understand how the
 * electron's self-field depends on:
 *   1. Damping mode (uniform vs selective vs OFF)
 *   2. Tick count (convergence behavior)
 *   3. Particle type (+1 vs -1, locked vs unlocked)
 *   4. Multi-particle configurations (dipole, triad)
 *
 * Uses multiple radius metrics:
 *   - r_eff: flux-weighted RMS (sensitive to long tails)
 *   - r_50:  radius containing 50% of field energy
 *   - r_90:  radius containing 90% of field energy
 *   - r_shell: 1% of peak boundary
 */

#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "ftd/gpu_engine.h"
#include "ftd/constants.h"

using namespace ftd;
using namespace ftd::gpu;

struct ShellMetrics {
    double J_peak;
    double r_eff;       // flux-weighted RMS radius
    double r_50;        // 50% energy radius
    double r_90;        // 90% energy radius
    int    r_shell;     // 1% of J(r=1) boundary
    double E_field;     // total field energy
    double E_ratio;     // E_field / K_B^2
};

ShellMetrics measure_shell(const std::vector<Voxel>& voxels, int L, int cx, int cy, int cz) {
    const int MAX_R = 60;
    std::vector<double> flux_sum(MAX_R + 1, 0.0);
    std::vector<double> energy_at_r(MAX_R + 1, 0.0);
    std::vector<int> count(MAX_R + 1, 0);
    double sum_r2_j2 = 0.0, sum_j2 = 0.0;
    double total_E = 0.0;
    double J_peak = 0.0;

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                double dx = x - cx, dy = y - cy, dz = z - cz;
                double r2 = dx*dx + dy*dy + dz*dz;
                double r = std::sqrt(r2);
                int ri = static_cast<int>(std::round(r));

                int idx = x*L*L + y*L + z;
                double j2 = voxels[idx].flux.mag2();
                double jmag = std::sqrt(j2);

                if (ri <= MAX_R) {
                    flux_sum[ri] += jmag;
                    energy_at_r[ri] += 0.5 * j2;
                    count[ri]++;
                }
                sum_r2_j2 += r2 * j2;
                sum_j2 += j2;
                total_E += 0.5 * j2;
                if (jmag > J_peak) J_peak = jmag;
            }
        }
    }

    ShellMetrics m;
    m.J_peak = J_peak;
    m.r_eff = (sum_j2 > 1e-30) ? std::sqrt(sum_r2_j2 / sum_j2) : 0.0;
    m.E_field = total_E;
    m.E_ratio = total_E / (K_B * K_B);

    // Average flux per shell
    std::vector<double> avg(MAX_R + 1, 0.0);
    for (int r = 0; r <= MAX_R; ++r)
        if (count[r] > 0) avg[r] = flux_sum[r] / count[r];

    // r_shell: 1% of J(r=1)
    m.r_shell = MAX_R;
    if (avg[1] > 0) {
        double thresh = 0.01 * avg[1];
        for (int r = 2; r <= MAX_R; ++r) {
            if (avg[r] < thresh) { m.r_shell = r; break; }
        }
    }

    // r_50, r_90: cumulative energy
    double cumE = 0.0;
    m.r_50 = MAX_R; m.r_90 = MAX_R;
    for (int r = 0; r <= MAX_R; ++r) {
        cumE += energy_at_r[r];
        if (m.r_50 == MAX_R && cumE >= 0.50 * total_E) m.r_50 = r;
        if (m.r_90 == MAX_R && cumE >= 0.90 * total_E) m.r_90 = r;
    }

    return m;
}

void print_metrics(const char* label, const ShellMetrics& m) {
    std::printf("  %-35s  J_pk=%.4e  r_eff=%6.2f  r_50=%5.1f  r_90=%5.1f  r_sh=%3d  E/K²=%.6f\n",
                label, m.J_peak, m.r_eff, m.r_50, m.r_90, m.r_shell, m.E_ratio);
}

int main() {
    std::printf("================================================================\n");
    std::printf("  GPU Shell Battery — Self-Field Dynamics Study\n");
    std::printf("  Lattice: 128^3 | GPU: GPU\n");
    std::printf("================================================================\n\n");

    constexpr int L = 128;
    constexpr int C = L / 2;

    // ================================================================
    // TEST 1: Convergence over time (uniform damping)
    // ================================================================
    std::printf("--- TEST 1: Convergence vs tick count (uniform damping) ---\n");
    std::printf("  %-35s  %10s  %8s  %7s  %7s  %5s  %10s\n",
                "Config", "J_peak", "r_eff", "r_50", "r_90", "r_sh", "E/K_B^2");
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = false;  // uniform damping
        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        {
            std::vector<Voxel> v(L*L*L);
            gpu.sync_to_host(v);
            v[C*L*L + C*L + C].locked = true;
            gpu.upload_from_host(v);
        }

        int ticks[] = {100, 200, 500, 1000, 2000, 4000, 8000};
        int prev = 0;
        for (int t : ticks) {
            gpu.run(t - prev);
            prev = t;
            std::vector<Voxel> v(L*L*L);
            gpu.sync_to_host(v);
            auto m = measure_shell(v, L, C, C, C);
            char label[64];
            std::snprintf(label, sizeof(label), "t=%d (uniform damp)", t);
            print_metrics(label, m);
        }
    }

    // ================================================================
    // TEST 2: Damping mode comparison at t=2000
    // ================================================================
    std::printf("\n--- TEST 2: Damping mode comparison at t=2000 ---\n");
    std::printf("  %-35s  %10s  %8s  %7s  %7s  %5s  %10s\n",
                "Config", "J_peak", "r_eff", "r_50", "r_90", "r_sh", "E/K_B^2");

    // 2a: Uniform damping
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = false;
        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        print_metrics("Uniform damping (alpha)", measure_shell(v, L, C, C, C));
    }

    // 2b: Selective damping
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = true;
        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        print_metrics("Selective damping", measure_shell(v, L, C, C, C));
    }

    // 2c: No damping
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.damping = false;
        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        print_metrics("No damping", measure_shell(v, L, C, C, C));
    }

    // ================================================================
    // TEST 3: Particle type comparison at t=2000 (uniform damping)
    // ================================================================
    std::printf("\n--- TEST 3: Particle type comparison at t=2000 ---\n");
    std::printf("  %-35s  %10s  %8s  %7s  %7s  %5s  %10s\n",
                "Config", "J_peak", "r_eff", "r_50", "r_90", "r_sh", "E/K_B^2");

    // 3a: +1 particle
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = false;
        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        print_metrics("+1 particle (electron)", measure_shell(v, L, C, C, C));
    }

    // 3b: -1 particle (positron)
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = false;
        gpu.inject_particle(C, C, C, -1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        print_metrics("-1 particle (positron)", measure_shell(v, L, C, C, C));
    }

    // ================================================================
    // TEST 4: Dipole (+1 and -1 separated)
    // ================================================================
    std::printf("\n--- TEST 4: Dipole configurations at t=2000 ---\n");
    std::printf("  %-35s  %10s  %8s  %7s  %7s  %5s  %10s\n",
                "Config", "J_peak", "r_eff", "r_50", "r_90", "r_sh", "E/K_B^2");

    int seps[] = {5, 10, 20, 40};
    for (int sep : seps) {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = false;
        int x1 = C - sep/2, x2 = C + sep/2;
        gpu.inject_particle(x1, C, C, +1, {0, 0, K_B}, 0, 0);
        gpu.inject_particle(x2, C, C, -1, {0, 0, K_B}, 0, 0);
        {
            std::vector<Voxel> v(L*L*L);
            gpu.sync_to_host(v);
            v[x1*L*L + C*L + C].locked = true;
            v[x2*L*L + C*L + C].locked = true;
            gpu.upload_from_host(v);
        }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        // Measure around the +1 particle
        auto m = measure_shell(v, L, x1, C, C);
        char label[64];
        std::snprintf(label, sizeof(label), "Dipole sep=%d (around +1)", sep);
        print_metrics(label, m);
    }

    // ================================================================
    // TEST 5: Coupling OFF (no source term, just initial pulse)
    // ================================================================
    std::printf("\n--- TEST 5: Coupling OFF (no continuous source) ---\n");
    std::printf("  %-35s  %10s  %8s  %7s  %7s  %5s  %10s\n",
                "Config", "J_peak", "r_eff", "r_50", "r_90", "r_sh", "E/K_B^2");
    {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        gpu.toggles.selective_damping = false;
        gpu.toggles.coupling = false;  // No state-flux source!
        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }

        int ticks2[] = {100, 500, 1000, 2000};
        int prev2 = 0;
        for (int t : ticks2) {
            gpu.run(t - prev2);
            prev2 = t;
            std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
            auto m = measure_shell(v, L, C, C, C);
            char label[64];
            std::snprintf(label, sizeof(label), "Coupling OFF, t=%d", t);
            print_metrics(label, m);
        }
    }

    std::printf("\n================================================================\n");
    std::printf("  Battery complete.\n");
    std::printf("================================================================\n");
    return 0;
}
