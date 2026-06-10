#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  TEST-DIVERGENCE: CPU/GPU Parity Divergence Checker\n");
    std::printf("================================================================\n");

    const int L = 32;
    const double A = 30.0;
    const unsigned int seed = 0xE0102000u;

    ftd::RenderBridge rb_gpu(L);
    ftd::RenderBridge rb_cpu(L);

    // Set identical toggles
    for (auto* rb : {&rb_gpu, &rb_cpu}) {
        rb->toggles.disable_all();
        rb->toggles.wave_propagation = true;
        rb->toggles.gauss_projection = true;
        rb->toggles.genesis          = true;
        rb->toggles.coupling         = true;
        rb->toggles.langevin         = true;
        rb->toggles.langevin_T       = 0.005;
        rb->toggles.langevin_gamma   = 0.02;
        rb->toggles.langevin_seed    = seed;
        rb->seed_rng(seed);
    }

    rb_cpu.force_cpu();
    rb_cpu.set_sor_iterations(5000);

    // Inject flux at center
    const int cx = L / 2;
    const int cy = L / 2;
    const int cz = L / 2;
    rb_gpu.inject_flux(cx, cy, cz, {A * ftd::K_GENESIS, 0.0, 0.0});
    rb_cpu.inject_flux(cx, cy, cz, {A * ftd::K_GENESIS, 0.0, 0.0});

    // Run tick by tick and compare
    const int num_ticks = 10;
    const int N = L * L * L;

    for (int t = 1; t <= num_ticks; ++t) {
        std::printf("--- Running Tick %d ---\n", t);
        rb_gpu.tick();
        rb_cpu.tick();

        rb_gpu.sync_from_gpu();

        const auto& voxels_gpu = static_cast<const ftd::RenderBridge&>(rb_gpu).voxels();
        const auto& voxels_cpu = static_cast<const ftd::RenderBridge&>(rb_cpu).voxels();

        double max_diff_flux = 0.0;
        double max_diff_wv = 0.0;
        int diff_state_count = 0;
        int diff_spin_count = 0;
        int diff_color_count = 0;

        for (int i = 0; i < N; ++i) {
            const auto& vg = voxels_gpu[i];
            const auto& vc = voxels_cpu[i];

            double dfx = std::abs(vg.flux.x - vc.flux.x);
            double dfy = std::abs(vg.flux.y - vc.flux.y);
            double dfz = std::abs(vg.flux.z - vc.flux.z);
            double df = std::max({dfx, dfy, dfz});
            if (df > max_diff_flux) max_diff_flux = df;

            double dwx = std::abs(vg.wave_vel.x - vc.wave_vel.x);
            double dwy = std::abs(vg.wave_vel.y - vc.wave_vel.y);
            double dwz = std::abs(vg.wave_vel.z - vc.wave_vel.z);
            double dw = std::max({dwx, dwy, dwz});
            if (dw > max_diff_wv) max_diff_wv = dw;

            if (vg.state != vc.state) diff_state_count++;
            if (vg.spin != vc.spin) diff_spin_count++;
            if (vg.color != vc.color) diff_color_count++;
        }

        std::printf("  Max Flux Diff     : %.8e\n", max_diff_flux);
        std::printf("  Max Wave_vel Diff : %.8e\n", max_diff_wv);
        std::printf("  State Mismatches  : %d\n", diff_state_count);
        std::printf("  Spin Mismatches   : %d\n", diff_spin_count);
        std::printf("  Color Mismatches  : %d\n", diff_color_count);

        if (diff_state_count > 0 || diff_spin_count > 0 || max_diff_flux > 1e-5) {
            std::printf("--- State/Spin Mismatch Details (first 20) ---\n");
            int printed = 0;
            for (int i = 0; i < N && printed < 20; ++i) {
                const auto& vg = voxels_gpu[i];
                const auto& vc = voxels_cpu[i];
                bool is_mismatch = (vg.state != vc.state) || (vg.spin != vc.spin);
                if (is_mismatch) {
                    int iz = i % L;
                    int iy = (i / L) % L;
                    int ix = i / (L * L);
                    std::printf("  Site %d (%d,%d,%d):\n", i, ix, iy, iz);
                    std::printf("    State: GPU=%d, CPU=%d\n", vg.state, vc.state);
                    std::printf("    Spin:  GPU=%d, CPU=%d\n", vg.spin, vc.spin);
                    std::printf("    Flux GPU: (%.6f, %.6f, %.6f)\n", vg.flux.x, vg.flux.y, vg.flux.z);
                    std::printf("    Flux CPU: (%.6f, %.6f, %.6f)\n", vc.flux.x, vc.flux.y, vc.flux.z);
                    std::printf("    Wave GPU: (%.6f, %.6f, %.6f)\n", vg.wave_vel.x, vg.wave_vel.y, vg.wave_vel.z);
                    std::printf("    Wave CPU: (%.6f, %.6f, %.6f)\n", vc.wave_vel.x, vc.wave_vel.y, vc.wave_vel.z);
                    printed++;
                }
            }
            std::printf("  DIVERGED at tick %d!\n", t);
            break;
        }
    }

    std::printf("================================================================\n");
    return 0;
}
