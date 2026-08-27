#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

struct DifferenceSummary {
    double max_flux = 0.0;
    double max_wave_vel = 0.0;
    int state_count = 0;
    int spin_count = 0;
    int color_count = 0;

    bool diverged(double tolerance) const {
        return !std::isfinite(max_flux) || !std::isfinite(max_wave_vel)
            || max_flux > tolerance || max_wave_vel > tolerance
            || state_count > 0 || spin_count > 0 || color_count > 0;
    }
};

DifferenceSummary compare_voxels(const std::vector<ftd::Voxel>& gpu,
                                 const std::vector<ftd::Voxel>& cpu) {
    DifferenceSummary result;
    if (gpu.size() != cpu.size()) {
        result.state_count = 1;
        return result;
    }
    for (std::size_t i = 0; i < gpu.size(); ++i) {
        const auto& vg = gpu[i];
        const auto& vc = cpu[i];
        result.max_flux = std::max(result.max_flux, std::max({
            std::abs(vg.flux.x - vc.flux.x),
            std::abs(vg.flux.y - vc.flux.y),
            std::abs(vg.flux.z - vc.flux.z),
        }));
        result.max_wave_vel = std::max(result.max_wave_vel, std::max({
            std::abs(vg.wave_vel.x - vc.wave_vel.x),
            std::abs(vg.wave_vel.y - vc.wave_vel.y),
            std::abs(vg.wave_vel.z - vc.wave_vel.z),
        }));
        result.state_count += (vg.state != vc.state);
        result.spin_count += (vg.spin != vc.spin);
        result.color_count += (vg.color != vc.color);
    }
    return result;
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  TEST-DIVERGENCE: CPU/GPU Parity Divergence Checker\n");
    std::printf("================================================================\n");

    // L=16 retains the nonlinear genesis/coupling/Langevin interaction while
    // avoiding 32^3 * 5000 redundant CPU SOR sweeps per tick. 256 sweeps are
    // sufficient for the live 1e-5 parity threshold on this bounded lattice.
    const int L = 16;
    const double A = 30.0;
    const unsigned int seed = 0xE0102000u;
    constexpr double tolerance = 1e-5;

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
    rb_cpu.set_sor_iterations(256);

    if (rb_gpu.backend_kind() != ftd::Backend::Kind::Gpu) {
        std::fprintf(stderr, "FAIL: test requires an active CUDA backend\n");
        return EXIT_FAILURE;
    }

    // Prove the comparison predicate used below is failure-capable instead of
    // relying only on a live run that is expected to match.
    std::vector<ftd::Voxel> synthetic_cpu(1);
    std::vector<ftd::Voxel> synthetic_gpu(1);
    synthetic_gpu[0].state = 1;
    if (!compare_voxels(synthetic_gpu, synthetic_cpu).diverged(tolerance)) {
        std::fprintf(stderr, "FAIL: mismatch predicate did not detect synthetic divergence\n");
        return EXIT_FAILURE;
    }

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

        const DifferenceSummary diff = compare_voxels(voxels_gpu, voxels_cpu);

        std::printf("  Max Flux Diff     : %.8e\n", diff.max_flux);
        std::printf("  Max Wave_vel Diff : %.8e\n", diff.max_wave_vel);
        std::printf("  State Mismatches  : %d\n", diff.state_count);
        std::printf("  Spin Mismatches   : %d\n", diff.spin_count);
        std::printf("  Color Mismatches  : %d\n", diff.color_count);

        if (diff.diverged(tolerance)) {
            std::printf("--- State/Spin Mismatch Details (first 20) ---\n");
            int printed = 0;
            for (int i = 0; i < N && printed < 20; ++i) {
                const auto& vg = voxels_gpu[i];
                const auto& vc = voxels_cpu[i];
                bool is_mismatch = (vg.state != vc.state) || (vg.spin != vc.spin);
                if (is_mismatch) {
                    int ix = i % L;
                    int iy = (i / L) % L;
                    int iz = i / (L * L);
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
            return EXIT_FAILURE;
        }
    }

    std::printf("================================================================\n");
    return EXIT_SUCCESS;
}
