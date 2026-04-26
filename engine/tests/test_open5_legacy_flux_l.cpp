/**
 * @file test_open5_legacy_flux_l.cpp
 * @brief OPEN-5 micro-regression: legacy single-substrate inject_flux must
 *        leave flux_L untouched when toggles.dual_substrate=false.
 *
 * The original failure: test_dual_substrate's "Legacy: flux_L = 0" check
 * failed because GpuEngine has its own `toggles` field (default
 * dual_substrate=true) and inject_flux branched on the GPU's local toggles,
 * not the bridge's. Tests that set rb.toggles.dual_substrate=false BEFORE
 * the first tick (and thus before any tick-time toggle sync) hit the GPU
 * default and incorrectly populated flux_L.
 *
 * Fix (engine/src/injection.cpp, 2026-04-25): inject_flux_cpu /
 * inject_particle_cpu / inject_wavepacket_cpu now write `gpu->toggles =
 * rb.toggles` immediately before the GPU call, so the GPU branch sees the
 * caller's actual toggle state.
 *
 * This mini-regression isolates that contract from the rest of the
 * dual_substrate test, which has unrelated DS-CHIRALITY failures tracked
 * separately.
 */

#include <cstdio>
#include "ftd/render_bridge.h"

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  OPEN-5 micro-regression: legacy inject_flux leaves flux_L = 0\n");
    std::printf("================================================================\n\n");

    int failures = 0;

    auto check = [&](const char* name, bool ok) {
        std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
        if (!ok) ++failures;
    };

    // Same setup as test_dual_substrate's DS-COMPATIBILITY block:
    {
        ftd::RenderBridge legacy(16);
        legacy.toggles.dual_substrate = false;
        legacy.toggles.genesis        = false;
        legacy.toggles.forces         = false;
        legacy.toggles.movement       = false;
        legacy.toggles.gauss_projection = false;
        legacy.inject_flux(8, 8, 8, ftd::Vec3(0.3, 0.1, 0.0));
        for (int t = 0; t < 5; ++t) legacy.tick();

        // The contract: legacy mode must leave flux_L exactly zero on every
        // voxel — neither inject_flux nor any tick path should populate it.
        const auto& voxels = legacy.voxels();
        const int N = legacy.lattice().total_sites();
        double dual_energy = 0.0;
        for (int i = 0; i < N; ++i) dual_energy += voxels[i].flux_L.mag2();

        std::printf("  legacy mode flux_L total energy: %.20f\n", dual_energy);
        check("Legacy: flux_L = 0 (dual_substrate=false respected)",
              dual_energy < 1e-20);

        // Sanity: legacy mode actually has observable flux energy
        double obs_energy = 0.0;
        for (int i = 0; i < N; ++i) obs_energy += voxels[i].flux.mag2();
        std::printf("  legacy mode observable flux energy: %.6f\n", obs_energy);
        check("Legacy: observable flux populated", obs_energy > 0.0001);
    }

    // Inverse contract: when dual_substrate=true, flux_L IS populated.
    {
        ftd::RenderBridge dual(16);
        dual.toggles.dual_substrate = true;
        dual.toggles.genesis        = false;
        dual.toggles.forces         = false;
        dual.toggles.movement       = false;
        dual.toggles.gauss_projection = false;
        dual.inject_flux(8, 8, 8, ftd::Vec3(0.3, 0.1, 0.0));

        const auto& voxels = dual.voxels();
        const int idx = dual.lattice().index(8, 8, 8);
        std::printf("  dual mode flux_L at (8,8,8): (%.4f, %.4f, %.4f)\n",
                    voxels[idx].flux_L.x, voxels[idx].flux_L.y, voxels[idx].flux_L.z);
        check("Dual: flux_L populated when dual_substrate=true",
              voxels[idx].flux_L.mag2() > 0.0);
    }

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: OPEN-5 micro-regression PASS (3/3 checks)\n");
    } else {
        std::printf("  RESULT: %d failures\n", failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
