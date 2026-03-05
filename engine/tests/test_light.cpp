/**
 * Test: Light & Photon Properties — 8 Checks
 *
 * Verifies that the engine's wave equation naturally produces massless,
 * frequency-bearing, linearly-propagating EM waves — i.e., LIGHT — without
 * any explicit photon code.
 *
 * These are EMERGENT properties: nowhere in the 6 rules does "photon" appear.
 * Light is what flux waves look like when you examine their collective behavior.
 *
 * Tests:
 *   LIGHT-1: Zero rest mass — no mass gap in dispersion relation
 *   LIGHT-2: Energy scales with frequency — blue light carries more energy
 *   LIGHT-3: Vacuum photon stability — no energy loss without matter
 *   LIGHT-4: Speed independent of amplitude — linearity of wave equation
 *   LIGHT-5: Two-color superposition — red and blue don't interact
 *   LIGHT-6: Dispersive broadening — lattice acts as a prism
 *   LIGHT-7: C_WAVE ≡ C_SPEED — massless waves travel at the speed limit
 *   LIGHT-8: No longitudinal propagation — Gauss constraint kills it
 *
 * Constants: C_WAVE = C_SPEED = 1/√3 ≈ 0.5774
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
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

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-1: Zero Rest Mass
// ─────────────────────────────────────────────────────────────────────────────
static void test_zero_rest_mass() {
    std::printf("\n--- LIGHT-1: Zero rest mass ---\n");

    constexpr int L = 32;
    constexpr double A = 0.1;
    constexpr double C2 = ftd::C_WAVE * ftd::C_WAVE;  // 1/3

    // Test 3 modes: n=1 (long wavelength), n=4, n=8 (short wavelength)
    // Same 1-tick eigenvalue extraction as the dispersion test (which passes to 10⁻¹⁶)
    int modes[] = {1, 4, 8};
    double max_m2 = 0.0;

    for (int mi = 0; mi < 3; ++mi) {
        int n = modes[mi];
        double k = 2.0 * M_PI * n / L;
        double omega2_theory = 4.0 * C2 * std::sin(k / 2.0) * std::sin(k / 2.0);

        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Initialize standing wave across ENTIRE 3D volume (uniform in y,z)
        // so that y,z terms in 6-point Laplacian cancel out
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            int idx = rb.lattice().index(x, y, z);
            rb.voxels()[idx].flux.z = A * std::sin(k * x);
        }

        // Run exactly 1 tick
        rb.tick();

        // Sample at x=1 (sin(k) ≠ 0 for all reasonable modes)
        int x_sample = 1;
        int idx_sample = rb.lattice().index(x_sample, L / 2, L / 2);
        double J_z_orig = A * std::sin(k * x_sample);
        double wv_z = rb.voxels()[idx_sample].wave_vel.z;
        double omega2_measured = (std::abs(J_z_orig) > 1e-15) ? std::abs(wv_z / J_z_orig) : 0.0;

        // Mass gap: m² = ω²_measured − ω²_theory (should be 0 for massless)
        double m2 = omega2_measured - omega2_theory;
        if (std::abs(m2) > max_m2) max_m2 = std::abs(m2);

        std::printf("  INFO: n=%d, k=%.4f, ω²_theory=%.10f, ω²_measured=%.10f, m²=%.2e\n",
                    n, k, omega2_theory, omega2_measured, m2);
    }

    check("LIGHT-1: Mass gap |m²| < 1e-10 for all modes (photon is massless)",
          max_m2 < 1e-10);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-2: Energy Scales with Frequency (Color)
// ─────────────────────────────────────────────────────────────────────────────
static void test_energy_frequency() {
    std::printf("\n--- LIGHT-2: Energy scales with frequency (color) ---\n");

    // Physics: E = ℏω for a photon. For same amplitude A, the oscillation
    // kinetic energy Σ|wv|² = ω²·A²·L³/2 scales with ω². Initialize proper
    // traveling waves and compare their kinetic energies at t=0.

    constexpr int L = 32;
    constexpr double A = 0.1;
    constexpr double C2 = ftd::C_WAVE * ftd::C_WAVE;

    // "Red" light: n=1 (long wavelength, low frequency)
    // "Blue" light: n=8 (short wavelength, high frequency)
    int n_red = 1, n_blue = 8;
    double k_red = 2.0 * M_PI * n_red / L;
    double k_blue = 2.0 * M_PI * n_blue / L;
    double omega_red = 2.0 * ftd::C_WAVE * std::sin(k_red / 2.0);
    double omega_blue = 2.0 * ftd::C_WAVE * std::sin(k_blue / 2.0);

    // Helper: initialize a traveling wave and return kinetic energy (wave_vel²)
    auto measure_kinetic = [&](double k, double omega) -> double {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Traveling wave: J_z = A·sin(kx), wv_z = -ω·A·cos(kx)
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            int idx = rb.lattice().index(x, y, z);
            rb.voxels()[idx].flux.z = A * std::sin(k * x);
            rb.voxels()[idx].wave_vel.z = -omega * A * std::cos(k * x);
        }

        // Measure kinetic energy at t=0 (before any ticks)
        auto ea = rb.energy_audit();
        return ea.wave_energy;  // Σ|wv|² = ω²·A²·Σcos²(kx) ∝ ω²
    };

    double KE_red = measure_kinetic(k_red, omega_red);
    double KE_blue = measure_kinetic(k_blue, omega_blue);
    double ratio = KE_blue / KE_red;
    double theory_ratio = (omega_blue * omega_blue) / (omega_red * omega_red);

    std::printf("  INFO: KE_red  = %.6e (n=%d, ω=%.6f)\n", KE_red, n_red, omega_red);
    std::printf("  INFO: KE_blue = %.6e (n=%d, ω=%.6f)\n", KE_blue, n_blue, omega_blue);
    std::printf("  INFO: KE_blue/KE_red = %.2f (theory ω²_blue/ω²_red = %.2f)\n",
                ratio, theory_ratio);

    // Blue light carries more energy than red — ratio should match ω² scaling
    check("LIGHT-2: Blue light carries more energy (KE ratio within 20% of ω² theory)",
          ratio > theory_ratio * 0.8 && ratio < theory_ratio * 1.2);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-3: Vacuum Photon Stability
// ─────────────────────────────────────────────────────────────────────────────
static void test_vacuum_stability() {
    std::printf("\n--- LIGHT-3: Vacuum photon stability ---\n");

    // Physics: A photon in empty space doesn't decay. With selective_damping=true
    // and no particles present, damping coefficient is 0 everywhere.
    //
    // Note: |J|²+|wv|² is NOT the leapfrog conserved quantity (that involves |∇J|²),
    // so we can't check absolute conservation. Instead we verify that selective_damping
    // with no particles produces IDENTICAL results to no damping at all.

    constexpr int L = 32;
    constexpr double A = 0.1;
    constexpr int n = 4;
    constexpr int TICKS = 100;
    double k = 2.0 * M_PI * n / L;
    double omega = 2.0 * ftd::C_WAVE * std::sin(k / 2.0);

    // Helper: initialize and run a traveling wave, return final field+wave energy
    auto run_wave = [&](bool use_selective_damping, bool use_uniform_damping) -> double {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.selective_damping = use_selective_damping;
        rb.toggles.damping = use_uniform_damping;

        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            int idx = rb.lattice().index(x, y, z);
            rb.voxels()[idx].flux.y = A * std::sin(k * x);
            rb.voxels()[idx].wave_vel.y = -omega * A * std::cos(k * x);
        }

        rb.run(TICKS);
        auto ea = rb.energy_audit();
        return ea.field_energy + ea.wave_energy;
    };

    // Run 1: No damping at all (pure wave propagation)
    double E_nodamp = run_wave(false, false);

    // Run 2: Selective damping ON, but no particles → should equal no damping
    double E_selective = run_wave(true, false);

    // Run 3: Uniform (legacy) damping ON → energy should decrease
    double E_uniform = run_wave(false, true);

    double diff_selective = std::abs(E_selective - E_nodamp) / E_nodamp * 100.0;
    double loss_uniform = (E_nodamp - E_uniform) / E_nodamp * 100.0;

    std::printf("  INFO: E(no damping)       = %.10e\n", E_nodamp);
    std::printf("  INFO: E(selective damping) = %.10e\n", E_selective);
    std::printf("  INFO: E(uniform damping)   = %.10e\n", E_uniform);
    std::printf("  INFO: |selective - nodamp| / nodamp = %.6f%%\n", diff_selective);
    std::printf("  INFO: Uniform damping energy loss    = %.2f%%\n", loss_uniform);

    // Selective damping with no particles = no damping (exact match)
    // Uniform damping removes energy (proves damping works, but photons survive without it)
    check("LIGHT-3: Vacuum photon stable (selective_damping = no damping when no matter)",
          diff_selective < 0.001 && loss_uniform > 1.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-4: Speed Independent of Amplitude (Linearity)
// ─────────────────────────────────────────────────────────────────────────────
static void test_speed_amplitude_independence() {
    std::printf("\n--- LIGHT-4: Speed independent of amplitude ---\n");

    constexpr int L = 32;
    constexpr double SIGMA = 3.0;
    constexpr int x0 = 8;
    constexpr int TICKS = 30;

    // Helper: find wavefront position (first x where sum|J_z| across yz exceeds threshold)
    auto find_wavefront = [&](ftd::RenderBridge& rb, double threshold) -> int {
        // Scan from high x downward to find the leading edge
        for (int x = L - 1; x >= 0; --x) {
            double sum_Jz = 0.0;
            for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                int idx = rb.lattice().index(x, y, z);
                sum_Jz += std::abs(rb.voxels()[idx].flux.z);
            }
            if (sum_Jz > threshold) return x;
        }
        return -1;
    };

    double amplitudes[] = {0.01, 0.1};  // dim and bright
    int wavefront_pos[2];

    for (int ai = 0; ai < 2; ++ai) {
        double A = amplitudes[ai];
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Gaussian pulse: J_z = A·exp(-(x-x0)²/(2σ²)), wv_z = same (outgoing in +x)
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            double dx = x - x0;
            double g = A * std::exp(-dx * dx / (2.0 * SIGMA * SIGMA));
            if (g < 1e-15) continue;
            int idx = rb.lattice().index(x, y, z);
            rb.voxels()[idx].flux.z = g;
            rb.voxels()[idx].wave_vel.z = g;  // +x directed
        }

        rb.run(TICKS);

        // Use threshold proportional to amplitude (same relative level)
        double threshold = A * 0.01 * L * L;  // 1% of peak × L² sites in a slice
        wavefront_pos[ai] = find_wavefront(rb, threshold);
        std::printf("  INFO: A=%.2f, wavefront at x=%d after %d ticks\n",
                    A, wavefront_pos[ai], TICKS);
    }

    int diff = std::abs(wavefront_pos[0] - wavefront_pos[1]);
    std::printf("  INFO: Wavefront position difference = %d voxels\n", diff);

    check("LIGHT-4: Dim and bright pulses arrive together (wavefront diff ≤ 1 voxel)",
          diff <= 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-5: Two-Color Superposition (Independence)
// ─────────────────────────────────────────────────────────────────────────────
static void test_two_color_superposition() {
    std::printf("\n--- LIGHT-5: Two-color superposition ---\n");

    constexpr int L = 32;
    constexpr double A = 0.1;
    constexpr int TICKS = 10;

    int n_red = 2, n_blue = 6;
    double k_red = 2.0 * M_PI * n_red / L;
    double k_blue = 2.0 * M_PI * n_blue / L;

    // Red only: y-polarized, n=2
    ftd::RenderBridge rb_red(L);
    rb_red.toggles.disable_all();
    rb_red.toggles.wave_propagation = true;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        int idx = rb_red.lattice().index(x, y, z);
        rb_red.voxels()[idx].flux.y = A * std::sin(k_red * x);
    }
    rb_red.run(TICKS);
    auto ea_red = rb_red.energy_audit();
    double E_red = ea_red.field_energy + ea_red.wave_energy;

    // Blue only: z-polarized, n=6
    ftd::RenderBridge rb_blue(L);
    rb_blue.toggles.disable_all();
    rb_blue.toggles.wave_propagation = true;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        int idx = rb_blue.lattice().index(x, y, z);
        rb_blue.voxels()[idx].flux.z = A * std::sin(k_blue * x);
    }
    rb_blue.run(TICKS);
    auto ea_blue = rb_blue.energy_audit();
    double E_blue = ea_blue.field_energy + ea_blue.wave_energy;

    // Combined: both colors at once (orthogonal polarizations)
    ftd::RenderBridge rb_both(L);
    rb_both.toggles.disable_all();
    rb_both.toggles.wave_propagation = true;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        int idx = rb_both.lattice().index(x, y, z);
        rb_both.voxels()[idx].flux.y = A * std::sin(k_red * x);
        rb_both.voxels()[idx].flux.z = A * std::sin(k_blue * x);
    }
    rb_both.run(TICKS);
    auto ea_both = rb_both.energy_audit();
    double E_both = ea_both.field_energy + ea_both.wave_energy;

    double E_sum = E_red + E_blue;
    double error_pct = std::abs(E_both - E_sum) / E_both * 100.0;

    std::printf("  INFO: E_red = %.6e (n=%d, y-pol)\n", E_red, n_red);
    std::printf("  INFO: E_blue = %.6e (n=%d, z-pol)\n", E_blue, n_blue);
    std::printf("  INFO: E_combined = %.6e\n", E_both);
    std::printf("  INFO: E_red + E_blue = %.6e\n", E_sum);
    std::printf("  INFO: Superposition error = %.6f%%\n", error_pct);

    check("LIGHT-5: E_combined = E_red + E_blue within 1% (two colors don't interact)",
          error_pct < 1.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-6: Dispersive Broadening (Lattice Prism)
// ─────────────────────────────────────────────────────────────────────────────
static void test_dispersive_broadening() {
    std::printf("\n--- LIGHT-6: Dispersive broadening (lattice prism) ---\n");

    constexpr int L = 32;
    constexpr double A = 0.5;
    constexpr int TICKS = 20;
    int mid = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    // Delta pulse at x=mid: contains ALL frequencies
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        int idx = rb.lattice().index(mid, y, z);
        rb.voxels()[idx].flux.z = A;
        rb.voxels()[idx].wave_vel.z = A;  // outgoing
    }

    rb.run(TICKS);

    // Measure spatial width: count x-slices with significant energy
    double peak_energy = 0.0;
    double slice_energy[32] = {};
    for (int x = 0; x < L; ++x) {
        double sum = 0.0;
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            int idx = rb.lattice().index(x, y, z);
            double Jz = rb.voxels()[idx].flux.z;
            double wvz = rb.voxels()[idx].wave_vel.z;
            sum += Jz * Jz + wvz * wvz;
        }
        slice_energy[x] = sum;
        if (sum > peak_energy) peak_energy = sum;
    }

    // Count slices above 1% of peak
    double threshold = peak_energy * 0.01;
    int width = 0;
    for (int x = 0; x < L; ++x) {
        if (slice_energy[x] > threshold) ++width;
    }

    std::printf("  INFO: Peak slice energy = %.6e\n", peak_energy);
    std::printf("  INFO: Spatial width (>1%% peak) = %d voxels after %d ticks\n", width, TICKS);

    check("LIGHT-6: Delta pulse broadens to ≥ 4 voxels (lattice dispersion = prism effect)",
          width >= 4);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-7: C_WAVE ≡ C_SPEED (Massless = Speed of Light)
// ─────────────────────────────────────────────────────────────────────────────
static void test_speed_identity() {
    std::printf("\n--- LIGHT-7: C_WAVE ≡ C_SPEED ---\n");

    // Compile-time check (this would fail to compile if violated)
    static_assert(ftd::C_WAVE == ftd::C_SPEED,
                  "C_WAVE must equal C_SPEED: massless waves travel at the causal speed limit");

    double inv_sqrt3 = 1.0 / std::sqrt(3.0);
    double cw_err = std::abs(ftd::C_WAVE - inv_sqrt3);
    double cs_err = std::abs(ftd::C_SPEED - inv_sqrt3);

    std::printf("  INFO: C_WAVE  = %.20f\n", ftd::C_WAVE);
    std::printf("  INFO: C_SPEED = %.20f\n", ftd::C_SPEED);
    std::printf("  INFO: 1/√3    = %.20f\n", inv_sqrt3);
    std::printf("  INFO: |C_WAVE - 1/√3|  = %.2e\n", cw_err);
    std::printf("  INFO: |C_SPEED - 1/√3| = %.2e\n", cs_err);

    check("LIGHT-7: C_WAVE = C_SPEED = 1/√3 (massless waves at causal speed limit)",
          cw_err < 1e-15 && cs_err < 1e-15);
}

// ─────────────────────────────────────────────────────────────────────────────
// LIGHT-8: No Longitudinal Propagation (Gauss Constraint)
// ─────────────────────────────────────────────────────────────────────────────
static void test_no_longitudinal() {
    std::printf("\n--- LIGHT-8: No longitudinal propagation ---\n");

    // Physics: EM waves are transverse (2 polarizations). The Gauss constraint
    // ∇·J = ρ (with ρ=0 in vacuum) removes the longitudinal degree of freedom.
    //
    // Method: Initialize identical-amplitude modes, one transverse (J_z∝sin(kx))
    // and one longitudinal (J_x∝sin(kx)). After a few ticks with Gauss projection,
    // the longitudinal mode should lose most of its energy (projected out),
    // while the transverse mode retains its energy (∇·J_trans = 0 already).

    constexpr int L = 32;
    constexpr double A = 0.1;
    constexpr int n = 4;
    constexpr int TICKS = 5;  // Just a few ticks — Gauss acts immediately
    double k = 2.0 * M_PI * n / L;

    // Helper: initialize mode, run TICKS, return total energy
    auto run_mode = [&](bool longitudinal) -> std::pair<double, double> {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;

        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            int idx = rb.lattice().index(x, y, z);
            double val = A * std::sin(k * x);
            if (longitudinal)
                rb.voxels()[idx].flux.x = val;  // J_x ∝ sin(kx): ∇·J = Ak·cos(kx) ≠ 0
            else
                rb.voxels()[idx].flux.z = val;  // J_z ∝ sin(kx): ∇·J = 0 (transverse)
        }

        auto ea0 = rb.energy_audit();
        double E0 = ea0.field_energy + ea0.wave_energy;

        rb.run(TICKS);

        auto ea1 = rb.energy_audit();
        double E1 = ea1.field_energy + ea1.wave_energy;
        return {E0, E1};
    };

    auto [E0_trans, E1_trans] = run_mode(false);
    auto [E0_long, E1_long] = run_mode(true);

    double retain_trans = E1_trans / E0_trans * 100.0;
    double retain_long = E1_long / E0_long * 100.0;

    std::printf("  INFO: Transverse: E0=%.6e → E1=%.6e (retained %.1f%%)\n",
                E0_trans, E1_trans, retain_trans);
    std::printf("  INFO: Longitudinal: E0=%.6e → E1=%.6e (retained %.1f%%)\n",
                E0_long, E1_long, retain_long);

    // Transverse should retain most energy, longitudinal should lose most
    check("LIGHT-8: Transverse retains > 50% energy AND longitudinal retains < transverse",
          retain_trans > 50.0 && retain_long < retain_trans);
}

// ─────────────────────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::printf("================================================================\n");
    std::printf("  TEST: Light & Photon Properties — 8 Checks\n");
    std::printf("================================================================\n");

    test_zero_rest_mass();          // LIGHT-1
    test_energy_frequency();        // LIGHT-2
    test_vacuum_stability();        // LIGHT-3
    test_speed_amplitude_independence(); // LIGHT-4
    test_two_color_superposition(); // LIGHT-5
    test_dispersive_broadening();   // LIGHT-6
    test_speed_identity();          // LIGHT-7
    test_no_longitudinal();         // LIGHT-8

    std::printf("\n================================================================\n");
    if (g_failures == 0)
        std::printf("  All 8 light tests PASSED.\n");
    else
        std::printf("  %d test(s) FAILED.\n", g_failures);
    std::printf("================================================================\n");

    return g_failures;
}
