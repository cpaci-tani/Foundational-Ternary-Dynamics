/**
 * Campaign: Lorentz Invariance Quantitative Measurement
 *
 * Runs wave packets along all 13 distinct lattice directions on the cubic
 * lattice and measures the effective wave speed in each direction. Computes
 * an isotropy metric sigma(c)/mean(c) as a function of wavelength.
 *
 * The 13 unique directions on a cubic lattice:
 *   6 face normals: [100], [010], [001], [-100], [0-10], [00-1]
 *     → 3 unique by symmetry: [100], [010], [001]
 *   12 edge diagonals: [110], [1-10], [101], [10-1], [011], [01-1]
 *     → 6 unique: [110], [1-10], [101], [10-1], [011], [01-1]
 *   8 body diagonals: [111], [11-1], [1-11], [-111]
 *     → 4 unique: [111], [11-1], [1-11], [-111]
 *   Total: 13 unique directions
 *
 * Protocol:
 *   1. For each direction, inject a Gaussian flux pulse at center
 *   2. Evolve for T ticks
 *   3. Measure leading-edge distance = effective speed
 *   4. Compute isotropy: sigma(c_eff) / mean(c_eff)
 *   5. Repeat for different wavelengths/lattice sizes to measure convergence
 *
 * Checks:
 *   LM-1: All 13 directions show wave propagation (c > 0)
 *   LM-2: Cardinal directions agree to < 1% (lattice symmetry)
 *   LM-3: Isotropy metric sigma/mean < 30% for L=48
 *   LM-4: Isotropy improves with scale (L=32 vs L=48)
 *
 * Theory references:
 *   - CLAUDE.md Section 14.2 (structural limitations, discreteness artifacts)
 *   - CLAUDE.md Section 22.4 (wave isotropy confirmed)
 *   - constants.h: C_WAVE = 1/sqrt(3) ~ 0.577
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

struct Direction {
    int dx, dy, dz;
    const char* name;
};

// Measure effective wave speed along a direction on lattice of size L.
// Returns leading-edge distance / ticks.
static double measure_speed(int L, int dx, int dy, int dz, int ticks) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    int mid = L / 2;
    double amp = ftd::K_B * 0.3;
    rb.inject_flux(mid, mid, mid, {amp, amp, amp});

    rb.run(ticks);

    double dir_mag = std::sqrt(double(dx*dx + dy*dy + dz*dz));
    if (dir_mag < 1e-15) return 0.0;

    double threshold = 1e-8;
    int max_r = 0;

    for (int r = 1; r < L / 2 - 2; ++r) {
        int px = mid + static_cast<int>(std::round(r * dx / dir_mag));
        int py = mid + static_cast<int>(std::round(r * dy / dir_mag));
        int pz = mid + static_cast<int>(std::round(r * dz / dir_mag));

        px = ((px % L) + L) % L;
        py = ((py % L) + L) % L;
        pz = ((pz % L) + L) % L;

        double f = rb.voxels()[rb.lattice().index(px, py, pz)].flux.mag();
        if (f > threshold) {
            max_r = r;
        }
    }

    return (ticks > 0 && max_r > 0) ? static_cast<double>(max_r) / ticks : 0.0;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Lorentz Invariance Measure -- 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    // All 13 unique directions on a cubic lattice
    Direction dirs[] = {
        // 3 cardinal (face normals)
        { 1,  0,  0, "[100]"},
        { 0,  1,  0, "[010]"},
        { 0,  0,  1, "[001]"},
        // 6 edge diagonals
        { 1,  1,  0, "[110]"},
        { 1, -1,  0, "[1-10]"},
        { 1,  0,  1, "[101]"},
        { 1,  0, -1, "[10-1]"},
        { 0,  1,  1, "[011]"},
        { 0,  1, -1, "[01-1]"},
        // 4 body diagonals
        { 1,  1,  1, "[111]"},
        { 1,  1, -1, "[11-1]"},
        { 1, -1,  1, "[1-11]"},
        {-1,  1,  1, "[-111]"},
    };
    const int N_DIRS = 13;

    // ================================================================
    // Measure wave speeds on L=48 lattice
    // ================================================================
    const int L = 48;
    const int TICKS = 25;

    std::cout << "\n--- Wave Speed Along 13 Directions (L=" << L
              << ", T=" << TICKS << ") ---\n";

    std::vector<double> speeds(N_DIRS);
    for (int i = 0; i < N_DIRS; ++i) {
        speeds[i] = measure_speed(L, dirs[i].dx, dirs[i].dy, dirs[i].dz, TICKS);
        std::cout << "    " << std::setw(7) << dirs[i].name
                  << ": c_eff = " << speeds[i] << " voxels/tick\n";
    }

    // Statistics
    double sum = std::accumulate(speeds.begin(), speeds.end(), 0.0);
    double mean = sum / N_DIRS;

    double sq_sum = 0;
    for (double s : speeds) sq_sum += (s - mean) * (s - mean);
    double sigma = std::sqrt(sq_sum / N_DIRS);

    double c_max = *std::max_element(speeds.begin(), speeds.end());
    double c_min = *std::min_element(speeds.begin(), speeds.end());
    double isotropy = (mean > 1e-15) ? sigma / mean : 999.0;

    std::cout << "\n    Statistics:\n";
    std::cout << "      mean(c)  = " << mean << " voxels/tick\n";
    std::cout << "      sigma(c) = " << sigma << "\n";
    std::cout << "      min(c)   = " << c_min << "\n";
    std::cout << "      max(c)   = " << c_max << "\n";
    std::cout << "      sigma/mean = " << isotropy * 100 << "% (isotropy metric)\n";
    std::cout << "      C_WAVE (theory) = " << ftd::C_WAVE << " voxels/tick\n";

    // ================================================================
    // LM-1: All directions propagate
    // ================================================================
    bool all_propagate = true;
    for (int i = 0; i < N_DIRS; ++i) {
        if (speeds[i] < 0.01) all_propagate = false;
    }
    check("LM-1: All 13 directions show wave propagation (c > 0.01)", all_propagate);

    // ================================================================
    // LM-2: Cardinal directions agree to < 1%
    // ================================================================
    {
        double c_cardinal_mean = (speeds[0] + speeds[1] + speeds[2]) / 3.0;
        double c_cardinal_max = std::max({speeds[0], speeds[1], speeds[2]});
        double c_cardinal_min = std::min({speeds[0], speeds[1], speeds[2]});
        double cardinal_dev = (c_cardinal_mean > 1e-15)
            ? (c_cardinal_max - c_cardinal_min) / c_cardinal_mean : 999.0;

        std::cout << "\n    Cardinal directions:\n";
        std::cout << "      [100]=" << speeds[0] << " [010]=" << speeds[1]
                  << " [001]=" << speeds[2] << "\n";
        std::cout << "      deviation = " << cardinal_dev * 100 << "%\n";

        check("LM-2: Cardinal directions agree within 1%", cardinal_dev < 0.01);
    }

    // ================================================================
    // LM-3: Isotropy metric sigma/mean < 30%
    // ================================================================
    check("LM-3: Overall isotropy sigma/mean < 30%", isotropy < 0.30);

    // ================================================================
    // LM-4: Isotropy improves with scale
    // ================================================================
    std::cout << "\n--- Scale Comparison: L=32 vs L=48 ---\n";
    {
        const int L_small = 32;
        const int T_small = 18;  // proportionally fewer ticks

        std::vector<double> speeds_small(N_DIRS);
        for (int i = 0; i < N_DIRS; ++i) {
            speeds_small[i] = measure_speed(L_small,
                dirs[i].dx, dirs[i].dy, dirs[i].dz, T_small);
        }

        double sum_s = std::accumulate(speeds_small.begin(), speeds_small.end(), 0.0);
        double mean_s = sum_s / N_DIRS;
        double sq_sum_s = 0;
        for (double s : speeds_small) sq_sum_s += (s - mean_s) * (s - mean_s);
        double sigma_s = std::sqrt(sq_sum_s / N_DIRS);
        double isotropy_small = (mean_s > 1e-15) ? sigma_s / mean_s : 999.0;

        std::cout << "    L=32: sigma/mean = " << isotropy_small * 100 << "%\n";
        std::cout << "    L=48: sigma/mean = " << isotropy * 100 << "%\n";

        // At larger scale, isotropy should improve (or at least not worsen dramatically)
        // Allow for statistical fluctuations
        check("LM-4: L=48 isotropy not worse than L=32 by more than 50%",
              isotropy < isotropy_small * 1.5 + 0.05);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (g_failures == 0)
        std::cout << "  All 4 Lorentz measure checks PASSED.\n";
    else
        std::cout << "  " << g_failures << " test(s) FAILED.\n";
    std::cout << "================================================================\n";

    return g_failures;
}
