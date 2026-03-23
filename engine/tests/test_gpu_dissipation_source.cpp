/**
 * Dissipation Source Analysis
 *
 * Question: Where does energy go in each phase of the tick cycle?
 * Can the explicit damping (rate alpha) be derived from the Gauss projection?
 *
 * Measures energy change from each phase independently:
 *   1. Wave propagation alone (no coupling, no damping, no Gauss)
 *   2. Coupling source alone (adds energy)
 *   3. Gauss projection alone (removes energy)
 *   4. Damping alone (removes energy)
 *   5. Gauss + coupling (the natural pair)
 *   6. Everything except explicit damping
 *   7. Everything (baseline)
 *
 * If Gauss+coupling produces the same net dissipation rate as explicit damping,
 * then damping is derivable.
 */

#include <cmath>
#include <cstdio>
#include <vector>
#include "ftd/gpu_engine.h"
#include "ftd/constants.h"

using namespace ftd;
using namespace ftd::gpu;

struct EnergyTrace {
    double E[21];  // energy at ticks 0, 100, 200, ..., 2000
    int n = 0;
};

EnergyTrace run_config(const char* label, bool wave, bool coupling, bool gauss, bool damping, bool selective) {
    constexpr int L = 64;  // Small for speed
    constexpr int C = L/2;

    GpuEngine gpu(L);
    // Start with everything off
    gpu.toggles.wave_propagation = wave;
    gpu.toggles.coupling = coupling;
    gpu.toggles.gauss_projection = gauss;
    gpu.toggles.damping = damping;
    gpu.toggles.selective_damping = selective;
    gpu.toggles.genesis = false;
    gpu.toggles.movement = false;
    gpu.toggles.forces = false;
    gpu.toggles.gravity = false;
    gpu.toggles.lorentz_force = false;
    gpu.toggles.poisson_coulomb = false;

    gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
    {
        std::vector<Voxel> v(L*L*L);
        gpu.sync_to_host(v);
        v[C*L*L + C*L + C].locked = true;
        gpu.upload_from_host(v);
    }

    EnergyTrace t;
    auto audit = gpu.energy_audit();
    t.E[0] = audit.field_energy + audit.wave_energy;
    t.n = 1;

    for (int step = 1; step <= 20; ++step) {
        gpu.run(100);
        audit = gpu.energy_audit();
        t.E[step] = audit.field_energy + audit.wave_energy;
        t.n++;
    }

    // Compute effective damping rate
    // If E(t) = E0 * exp(-2*gamma*t), then gamma = -ln(E(t)/E0) / (2*t)
    // Use last point for steady-state estimate
    double E_final = t.E[20];
    double E_mid = t.E[10];
    double E_early = t.E[2];  // t=200

    std::printf("  %-45s  E0=%.4e  E200=%.4e  E1000=%.4e  E2000=%.4e",
                label, t.E[0], E_early, E_mid, E_final);

    if (E_mid > 1e-30 && t.E[0] > 1e-30) {
        // Effective decay rate from t=200 to t=2000
        double ratio = E_final / E_early;
        if (ratio > 0 && ratio < 1) {
            double gamma_eff = -std::log(ratio) / (2.0 * 1800.0);
            std::printf("  gamma=%.6f (alpha=%.6f, ratio=%.2f)",
                        gamma_eff, ALPHA, gamma_eff / ALPHA);
        } else if (ratio >= 1) {
            double growth = (E_final - E_early) / E_early / 1800.0;
            std::printf("  GROWING rate=%.4e/tick", growth);
        }
    }
    std::printf("\n");

    return t;
}

int main() {
    std::printf("================================================================\n");
    std::printf("  DISSIPATION SOURCE ANALYSIS — 64^3, 2000 ticks\n");
    std::printf("  alpha = %.8f, g_c = alpha^2 = %.4e\n", ALPHA, ALPHA*ALPHA);
    std::printf("================================================================\n\n");

    std::printf("  %-45s  %10s  %10s  %10s  %10s  %s\n",
                "Configuration", "E(0)", "E(200)", "E(1000)", "E(2000)", "Eff. rate");
    std::printf("  %s\n", std::string(120, '-').c_str());

    // 1. Wave only (should conserve energy)
    run_config("Wave only", true, false, false, false, false);

    // 2. Coupling only (should add energy)
    run_config("Coupling only", false, true, false, false, false);

    // 3. Wave + Coupling (no constraints)
    run_config("Wave + Coupling", true, true, false, false, false);

    // 4. Wave + Gauss only (should remove some energy)
    run_config("Wave + Gauss", true, false, true, false, false);

    // 5. Wave + Coupling + Gauss (natural pair, no explicit damping)
    run_config("Wave + Coupling + Gauss (NO damping)", true, true, true, false, false);

    // 6. Wave + Damping only (no source, just decay)
    run_config("Wave + Damping (uniform)", true, false, false, true, false);

    // 7. Full default (wave + coupling + gauss + damping uniform)
    run_config("FULL (uniform damping)", true, true, true, true, false);

    // 8. Full with selective damping
    run_config("FULL (selective damping)", true, true, true, true, true);

    // 9. Just Gauss + Coupling (no wave propagation)
    run_config("Coupling + Gauss (no wave)", false, true, true, false, false);

    // 10. Damping only (no wave, no coupling, no gauss)
    run_config("Damping only (uniform, no wave)", false, false, false, true, false);

    std::printf("\n================================================================\n");
    std::printf("  KEY QUESTION: Does 'Wave+Coupling+Gauss' produce effective\n");
    std::printf("  damping at rate ~alpha WITHOUT explicit damping?\n");
    std::printf("  If yes: damping is DERIVED from Gauss projection.\n");
    std::printf("  If no:  damping is IMPOSED and must remain [SELECTION].\n");
    std::printf("================================================================\n");

    return 0;
}
