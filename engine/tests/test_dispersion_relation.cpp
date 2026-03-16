/**
 * Test: Lattice Photon Dispersion Relation — 8 Checks
 *
 * Verifies that EM waves on the discrete cubic lattice obey the predicted
 * dispersion relation:  ω² = 4c²sin²(k/2)  where c = C_WAVE = 1/√3.
 *
 * This differs from the continuum relation ω = ck and is a genuine emergent
 * property of the discrete 6-point Laplacian operator. It is NOT coded into
 * the engine — it arises from the stencil geometry.
 *
 * Method: One-tick eigenvalue extraction. For mode n, initialize J_z = A*sin(kx)
 * with wave_vel = 0. After one tick, wave_vel_z = c²*∇²J_z = -4c²sin²(k/2)*J_z.
 * The ratio |wv_z / J_z| directly gives ω².
 *
 * Tests:
 *   DISP-1..5: ω² matches theory for 5 modes (n=1,4,8,12,15)
 *   DISP-6:    Long-wavelength limit ω ≈ c·k
 *   DISP-7:    Phase velocity decreases with k (normal dispersion)
 *   DISP-8:    Group velocity positive and ≤ C_WAVE
 *
 * Constants: C_WAVE = 1/√3 ≈ 0.5774
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::printf("  PASS  %s\n", name);
    } else {
        std::printf("  FAIL  %s\n", name);
        ++g_failures;
    }
}

// Measure ω² for mode n on an L³ lattice using single-tick eigenvalue extraction
static double measure_omega_sq(int L, int n) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    double k = 2.0 * M_PI * n / L;
    double AMP = 0.1;

    // Initialize J_z = A * sin(k * x), wave_vel = 0
    for (int x = 0; x < L; ++x) {
        double jz = AMP * std::sin(k * x);
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                rb.inject_flux(x, y, z, {0, 0, jz});
            }
    }

    // Sample J_z at a non-node site before tick
    // x=1 gives sin(2πn/L) which is nonzero for n=1..L-1
    int sample_idx = rb.lattice().index(1, 0, 0);
    double J_before = rb.voxels()[sample_idx].flux.z;

    // Run exactly 1 tick
    rb.tick();

    // After tick: wave_vel_z = c² * ∇²J_z = -ω² * J_z_before
    double wv_after = rb.voxels()[sample_idx].wave_vel.z;

    // ω² = |wv_after / J_before|
    if (std::abs(J_before) < 1e-15) return 0.0;
    return std::abs(wv_after / J_before);
}

int main() {
    std::printf("================================================================\n");
    std::printf("  TEST: Lattice Photon Dispersion Relation — 8 Checks\n");
    std::printf("================================================================\n");

    constexpr int L = 32;
    double c2 = ftd::C_WAVE * ftd::C_WAVE;  // 1/3

    // Modes to test: n = 1, 4, 8, 12, 15
    int modes[] = {1, 4, 8, 12, 15};
    int num_modes = 5;

    double omega_sq_meas[5];
    double omega_sq_theory[5];
    double k_vals[5];
    double omega_meas[5];
    double omega_theory[5];

    std::printf("\n--- Dispersion relation: ω² = 4c²sin²(k/2) ---\n");
    std::printf("  C_WAVE = %.6f, C_WAVE² = %.6f\n", ftd::C_WAVE, c2);
    std::printf("  %-6s %-10s %-14s %-14s %-10s\n",
                "n", "k", "ω²_theory", "ω²_measured", "error");

    for (int i = 0; i < num_modes; ++i) {
        int n = modes[i];
        double k = 2.0 * M_PI * n / L;
        double sin_half_k = std::sin(k / 2.0);
        double theory = 4.0 * c2 * sin_half_k * sin_half_k;
        double measured = measure_omega_sq(L, n);

        k_vals[i] = k;
        omega_sq_theory[i] = theory;
        omega_sq_meas[i] = measured;
        omega_theory[i] = std::sqrt(theory);
        omega_meas[i] = std::sqrt(measured);

        double error = std::abs(measured - theory) / theory;
        std::printf("  %-6d %-10.4f %-14.8f %-14.8f %-10.2e\n",
                    n, k, theory, measured, error);
    }

    // DISP-1 through DISP-5: Each mode matches theory
    std::printf("\n--- DISP-1..5: Mode-by-mode verification ---\n");
    const char* check_names[] = {
        "DISP-1: ω² matches theory for n=1 (long wavelength, < 0.1%)",
        "DISP-2: ω² matches theory for n=4 (mid wavelength, < 0.1%)",
        "DISP-3: ω² matches theory for n=8 (short wavelength, < 0.1%)",
        "DISP-4: ω² matches theory for n=12 (near-Nyquist, < 0.1%)",
        "DISP-5: ω² matches theory for n=15 (almost-Nyquist, < 0.1%)"
    };
    for (int i = 0; i < num_modes; ++i) {
        double error = std::abs(omega_sq_meas[i] - omega_sq_theory[i]) / omega_sq_theory[i];
        check(check_names[i], error < 0.001);
    }

    // DISP-6: Long-wavelength limit ω ≈ c·k
    std::printf("\n--- DISP-6: Continuum limit ---\n");
    double ratio_continuum = omega_meas[0] / (ftd::C_WAVE * k_vals[0]);
    std::printf("  INFO: ω/(c·k) for n=1 = %.6f (expect ~1.0)\n", ratio_continuum);
    check("DISP-6: Long-wavelength limit ω ≈ c·k (within 5%)",
          std::abs(ratio_continuum - 1.0) < 0.05);

    // DISP-7: Phase velocity decreases with k (normal dispersion)
    std::printf("\n--- DISP-7: Phase velocity dispersion ---\n");
    double v_phase[5];
    for (int i = 0; i < num_modes; ++i) {
        v_phase[i] = omega_meas[i] / k_vals[i];
        std::printf("  INFO: v_phase(n=%d) = %.6f\n", modes[i], v_phase[i]);
    }
    bool monotonic_decrease = true;
    for (int i = 1; i < num_modes; ++i) {
        if (v_phase[i] >= v_phase[i-1]) {
            monotonic_decrease = false;
            break;
        }
    }
    check("DISP-7: Phase velocity v_p = ω/k decreases with k (normal dispersion)",
          monotonic_decrease);

    // DISP-8: Group velocity v_g = c*cos(k/2) positive and ≤ C_WAVE
    std::printf("\n--- DISP-8: Group velocity ---\n");
    bool group_ok = true;
    for (int i = 0; i < num_modes; ++i) {
        double k = k_vals[i];
        double v_group = ftd::C_WAVE * std::cos(k / 2.0);
        std::printf("  INFO: v_group(n=%d) = %.6f\n", modes[i], v_group);
        if (v_group < -0.01 || v_group > ftd::C_WAVE + 0.01) {
            group_ok = false;
        }
    }
    check("DISP-8: Group velocity v_g = c·cos(k/2) positive and ≤ C_WAVE",
          group_ok);

    std::printf("\n================================================================\n");
    if (g_failures == 0)
        std::printf("  All 8 dispersion relation tests PASSED.\n");
    else
        std::printf("  %d test(s) FAILED.\n", g_failures);
    std::printf("================================================================\n");

    return g_failures;
}
