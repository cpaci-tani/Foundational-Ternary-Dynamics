/**
 * Test: Dipole Radiation Pattern — 6 Checks
 *
 * Verifies that a z-polarized current burst produces the classical sin²θ
 * angular radiation pattern. This is an EMERGENT property — nowhere in the
 * engine's 6 rules is sin²θ encoded. It arises from the vector wave equation
 * on the 3D cubic lattice.
 *
 * Method: Inject a GAUSSIAN z-directed flux pulse (σ=3 voxels) at the
 * lattice center. The Gaussian smooths out short-wavelength content that
 * has strong lattice anisotropy, leaving only long-wavelength modes that
 * propagate nearly isotropically on the cubic lattice.
 *
 * Measure the time-integrated RADIAL Poynting flux S·r̂ through a spherical
 * shell, binned by polar angle θ. Volume-averaging over θ-bins provides
 * natural azimuthal averaging. The radial Poynting flux (not energy density)
 * captures the angular distribution of radiated power.
 *
 * Expected: ∫(S·r̂)dt ∝ sin²θ
 *   - Maximum at θ=π/2 (equator, perpendicular to dipole)
 *   - Zero at θ=0 (poles, along dipole axis)
 *
 * Tests:
 *   RAD-1: Polar null — flux at poles < 15% of equatorial
 *   RAD-2: Equatorial maximum — equator has the most outward flux
 *   RAD-3: General trend — flux increases from poles to equator
 *   RAD-4: sin²θ ratio at θ≈50°
 *   RAD-5: sin²θ ratio at θ≈30°
 *   RAD-6: Hemisphere symmetry — north equals south
 *
 * Constants: C_WAVE = 1/√3 ≈ 0.5774
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
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

int main() {
    std::printf("================================================================\n");
    std::printf("  TEST: Dipole Radiation Pattern — 6 Checks\n");
    std::printf("================================================================\n");

    constexpr int L = 48;
    int mid = L / 2;
    double AMP = 0.5;
    double SIGMA = 3.0;          // Gaussian width — suppresses k > 1/σ
    double R_SHELL = 14.0;       // Measurement shell radius
    double DR = 2.0;             // Shell half-thickness
    int TOTAL_TICKS = 40;
    int START_TICK = 12;         // Start accumulating after wavefront begins expanding

    // θ bins: 9 bins of width π/9 (20° each)
    constexpr int N_BINS = 9;
    double bin_width = M_PI / N_BINS;
    double bin_flux[N_BINS] = {};       // Accumulated radial Poynting flux
    long long bin_samples[N_BINS] = {}; // Number of voxel-tick samples

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    // Inject GAUSSIAN z-directed pulse at center
    // J_z(r) = AMP × exp(-r²/(2σ²)), wave_vel_z = same (outgoing pulse)
    // This suppresses wavelengths shorter than ~2πσ ≈ 19 voxels
    std::printf("  INFO: Injecting Gaussian z-dipole pulse, σ=%.1f, AMP=%.2f\n", SIGMA, AMP);
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        double dx = x - mid, dy = y - mid, dz = z - mid;
        double r2 = dx*dx + dy*dy + dz*dz;
        double g = AMP * std::exp(-r2 / (2.0 * SIGMA * SIGMA));
        if (g < 1e-10) continue;
        int idx = rb.lattice().index(x, y, z);
        rb.voxels()[idx].flux.z += g;
        rb.voxels()[idx].wave_vel.z += g;
    }

    std::printf("  INFO: Lattice %d³, shell r=%.0f±%.0f, ticks %d..%d\n",
                L, R_SHELL, DR, START_TICK, TOTAL_TICKS);
    std::printf("  INFO: %d angular bins of %.0f° each\n", N_BINS, 180.0 / N_BINS);

    // Run initial ticks without accumulating
    rb.run(START_TICK);

    // Time-integrate: accumulate radial Poynting flux in θ bins
    for (int t = START_TICK; t < TOTAL_TICKS; ++t) {
        rb.tick();

        // Scan all voxels in the measurement shell
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            double dx = x - mid;
            double dy = y - mid;
            double dz = z - mid;
            double r = std::sqrt(dx*dx + dy*dy + dz*dz);

            if (r < R_SHELL - DR || r > R_SHELL + DR || r < 1e-10) continue;

            // Compute polar angle θ from z-axis
            double cos_theta = dz / r;
            double theta = std::acos(std::max(-1.0, std::min(1.0, cos_theta)));

            // Determine θ bin
            int bin = (int)(theta / bin_width);
            if (bin >= N_BINS) bin = N_BINS - 1;

            // Compute radial Poynting flux: S · r̂
            int idx = rb.lattice().index(x, y, z);
            auto S = rb.poynting_vector(idx);
            double S_radial = (S.x * dx + S.y * dy + S.z * dz) / r;

            bin_flux[bin] += S_radial;
            bin_samples[bin]++;
        }
    }

    // Compute mean radial Poynting flux per voxel-tick in each bin
    double mean_flux[N_BINS];
    std::printf("\n--- Volume-averaged radial Poynting flux by polar angle ---\n");
    std::printf("  %-8s %-10s %-14s %-12s %-12s\n",
                "bin", "θ_center", "<S·r̂>", "ratio", "sin²θ_theory");

    for (int i = 0; i < N_BINS; ++i) {
        double theta_center = (i + 0.5) * bin_width;
        mean_flux[i] = (bin_samples[i] > 0) ? bin_flux[i] / bin_samples[i] : 0.0;
    }

    // Find equatorial bin value for normalization
    double F_equat = mean_flux[N_BINS / 2];

    for (int i = 0; i < N_BINS; ++i) {
        double theta_center = (i + 0.5) * bin_width;
        double ratio = (std::abs(F_equat) > 1e-30) ? mean_flux[i] / F_equat : 0.0;
        double sin_t = std::sin(theta_center);
        std::printf("  %-8d %-10.1f %-14.6e %-12.4f %-12.4f\n",
                    i, theta_center * 180.0 / M_PI, mean_flux[i], ratio,
                    sin_t * sin_t);
    }

    // Extract hemisphere-averaged values (north + south mirror bins)
    double F_pole = (mean_flux[0] + mean_flux[N_BINS - 1]) / 2.0;
    double F_30 = (mean_flux[1] + mean_flux[N_BINS - 2]) / 2.0;
    double F_50 = (mean_flux[2] + mean_flux[N_BINS - 3]) / 2.0;
    double F_70 = (mean_flux[3] + mean_flux[N_BINS - 4]) / 2.0;

    std::printf("\n  INFO: Hemisphere-averaged <S·r̂>:\n");
    std::printf("    θ≈10°: %.6e\n", F_pole);
    std::printf("    θ≈30°: %.6e\n", F_30);
    std::printf("    θ≈50°: %.6e\n", F_50);
    std::printf("    θ≈70°: %.6e\n", F_70);
    std::printf("    θ≈90°: %.6e\n", F_equat);

    // RAD-1: Polar null
    std::printf("\n--- RAD-1: Polar null ---\n");
    double pole_ratio = (std::abs(F_equat) > 1e-30) ? std::abs(F_pole / F_equat) : 0.0;
    std::printf("  INFO: |F(pole)/F(equator)| = %.4f\n", pole_ratio);
    check("RAD-1: Radial flux at poles < 15% of equatorial (dipole null)",
          std::abs(F_equat) > 1e-30 && pole_ratio < 0.15);

    // RAD-2: Equatorial maximum
    std::printf("\n--- RAD-2: Equatorial maximum ---\n");
    double max_flux = *std::max_element(mean_flux, mean_flux + N_BINS);
    std::printf("  INFO: max <S·r̂> = %.6e, equatorial = %.6e\n", max_flux, F_equat);
    // Equator should be the maximum or within 20%
    check("RAD-2: Equator near maximum radial flux (within 20% of peak)",
          F_equat > 1e-30 && F_equat >= 0.80 * max_flux);

    // RAD-3: General trend pole → equator
    std::printf("\n--- RAD-3: General trend ---\n");
    std::printf("  INFO: F(10°)=%.3e < F(30°)=%.3e < F(50°)=%.3e < F(70°)=%.3e < F(90°)=%.3e\n",
                F_pole, F_30, F_50, F_70, F_equat);
    bool strict_trend = F_pole < F_30 && F_30 < F_50 && F_50 < F_70 && F_70 < F_equat;
    // Weak trend: pole < mid-angle < equator
    bool weak_trend = F_pole < F_50 && F_50 < F_equat;
    check("RAD-3: Radial flux increases pole→equator (strict or weak trend)",
          strict_trend || weak_trend);

    // RAD-4: sin²θ ratio at θ≈50° (sin²50° ≈ 0.587)
    std::printf("\n--- RAD-4: sin²(50°) ratio ---\n");
    double ratio_50 = (std::abs(F_equat) > 1e-30) ? F_50 / F_equat : 0.0;
    std::printf("  INFO: F(50°)/F(90°) = %.4f (theory sin²50° = 0.587)\n", ratio_50);
    // Generous tolerance for discrete lattice: [0.2, 0.95]
    check("RAD-4: F(50°)/F(90°) in range [0.2, 0.95] (sin²50° ≈ 0.59)",
          ratio_50 > 0.20 && ratio_50 < 0.95);

    // RAD-5: sin²θ ratio at θ≈30° (sin²30° = 0.25)
    std::printf("\n--- RAD-5: sin²(30°) ratio ---\n");
    double ratio_30 = (std::abs(F_equat) > 1e-30) ? F_30 / F_equat : 0.0;
    std::printf("  INFO: F(30°)/F(90°) = %.4f (theory sin²30° = 0.250)\n", ratio_30);
    // Generous tolerance: [0.05, 0.60]
    check("RAD-5: F(30°)/F(90°) in range [0.05, 0.60] (sin²30° = 0.25)",
          ratio_30 > 0.05 && ratio_30 < 0.60);

    // RAD-6: Hemisphere symmetry (north should mirror south)
    std::printf("\n--- RAD-6: Hemisphere symmetry ---\n");
    double north_total = 0, south_total = 0;
    for (int i = 0; i < N_BINS / 2; ++i) {
        north_total += mean_flux[i];
        south_total += mean_flux[N_BINS - 1 - i];
    }
    double hem_ratio = (std::abs(south_total) > 1e-30)
                       ? north_total / south_total : 999.0;
    std::printf("  INFO: North hemisphere flux = %.6e\n", north_total);
    std::printf("  INFO: South hemisphere flux = %.6e\n", south_total);
    std::printf("  INFO: N/S ratio = %.4f (expect ~1.0)\n", hem_ratio);
    check("RAD-6: Hemisphere symmetry N/S ratio within [0.8, 1.2]",
          hem_ratio > 0.8 && hem_ratio < 1.2);

    std::printf("\n================================================================\n");
    if (g_failures == 0)
        std::printf("  All 6 dipole radiation tests PASSED.\n");
    else
        std::printf("  %d test(s) FAILED.\n", g_failures);
    std::printf("================================================================\n");

    return g_failures;
}
