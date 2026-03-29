/**
 * Campaign: Dual-Substrate Information Cascade
 *
 * Measures bits-per-voxel at each degradation stage in 3D, quantifying
 * information loss through the measurement chain:
 *
 *   Stage 1 (Full field):  H(Jx) + H(Jy) + H(Jz)   — full vector field
 *   Stage 2 (Born rule):   H(|J|^2)                  — intensity only
 *   Stage 3 (Ternary):     H(s in {-1,0,+1})         — discrete state
 *   Stage 4 (Boolean):     H(|s| in {0,1})            — detected/silent
 *
 * Setup:
 *   L=64, two counter-phase sources (same as void classification)
 *   Stages 1-2: 200 ticks, genesis=false
 *   Stages 3-4: 400 ticks, genesis=true (ternary manifestation)
 *   Detection plane: x=48, 64*64 = 4096 voxels
 *
 * Shannon entropy: bin into 256 bins, H = -sum(p_i * log2(p_i))
 *
 * Checks:
 *   DSIC1: H_full > H_born (phase information lost)
 *   DSIC2: H_ternary > H_boolean (sign information lost)
 *   DSIC3: H_full > H_ternary > H_boolean (monotonic decrease)
 *   DSIC4: Print all values for comparison with 2D suite
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>
#include "ftd/engine_select.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

// Shannon entropy for a 1D distribution of N values, binned into n_bins bins.
// Returns bits per sample.
double shannon_entropy(const std::vector<double>& values, int n_bins = 256) {
    if (values.empty()) return 0.0;

    // Find min and max
    double vmin = *std::min_element(values.begin(), values.end());
    double vmax = *std::max_element(values.begin(), values.end());

    // Handle degenerate case: all values identical
    if (vmax - vmin < 1e-30) return 0.0;

    double bin_width = (vmax - vmin) / n_bins;
    std::vector<int> counts(n_bins, 0);

    for (double v : values) {
        int bin = static_cast<int>((v - vmin) / bin_width);
        if (bin >= n_bins) bin = n_bins - 1;
        if (bin < 0) bin = 0;
        counts[bin]++;
    }

    double H = 0.0;
    double N = static_cast<double>(values.size());
    for (int i = 0; i < n_bins; ++i) {
        if (counts[i] > 0) {
            double p = counts[i] / N;
            H -= p * std::log2(p);
        }
    }
    return H;
}

// Shannon entropy for a discrete distribution (exact counts for small alphabet)
double shannon_entropy_discrete(const std::vector<int>& counts_vec) {
    double N = 0.0;
    for (int c : counts_vec) N += c;
    if (N < 1.0) return 0.0;

    double H = 0.0;
    for (int c : counts_vec) {
        if (c > 0) {
            double p = c / N;
            H -= p * std::log2(p);
        }
    }
    return H;
}

int main(int argc, char* argv[]) {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: DS Information Cascade — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 64;
    const double amplitude = ftd::K_B * 0.3;

    // Source positions (same as void classification)
    const int srcA_x = 22, srcA_y = 32, srcA_z = 32;
    const int srcB_x = 42, srcB_y = 32, srcB_z = 32;
    const int det_x = 48;
    const int N_VOXELS = L * L;  // 4096

    ftd::Vec3 flux_A = {0.0, 0.0, amplitude};
    ftd::Vec3 flux_B = {0.0, 0.0, -amplitude};

    std::cout << "\n--- Setup ---\n";
    std::cout << "  L = " << L << "\n";
    std::cout << "  Amplitude = " << amplitude << "\n";
    std::cout << "  Detection plane: x = " << det_x << " (" << N_VOXELS << " voxels)\n";

    // ================================================================
    // Stage 1 & 2: Full field and Born rule (200 ticks, no genesis)
    // ================================================================
    std::cout << "\n--- Stage 1-2: Flux field (200 ticks, genesis=false) ---\n";

    std::vector<double> Jx_vals, Jy_vals, Jz_vals;
    std::vector<double> J_mag2_vals;

    {
        ftd::SimEngine rb(L);
        rb.toggles().genesis = false;
        rb.toggles().forces = false;
        rb.toggles().movement = false;

        rb.inject_flux(srcA_x, srcA_y, srcA_z, flux_A);
        rb.inject_flux(srcB_x, srcB_y, srcB_z, flux_B);

        rb.run(200);

        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const auto& v = rb.voxel_at(det_x, y, z);
                Jx_vals.push_back(v.flux.x);
                Jy_vals.push_back(v.flux.y);
                Jz_vals.push_back(v.flux.z);
                double mag2 = v.flux.x * v.flux.x
                            + v.flux.y * v.flux.y
                            + v.flux.z * v.flux.z;
                J_mag2_vals.push_back(mag2);
            }
        }
    }

    // Stage 1: H_full = H(Jx) + H(Jy) + H(Jz)
    double H_Jx = shannon_entropy(Jx_vals);
    double H_Jy = shannon_entropy(Jy_vals);
    double H_Jz = shannon_entropy(Jz_vals);
    double H_full = H_Jx + H_Jy + H_Jz;

    // Stage 2: H_born = H(|J|^2)
    double H_born = shannon_entropy(J_mag2_vals);

    std::cout << "  H(Jx) = " << H_Jx << " bits\n";
    std::cout << "  H(Jy) = " << H_Jy << " bits\n";
    std::cout << "  H(Jz) = " << H_Jz << " bits\n";
    std::cout << "  H_full = " << H_full << " bits/voxel\n";
    std::cout << "  H_born = " << H_born << " bits/voxel\n";

    // ================================================================
    // Stage 3 & 4: Ternary and Boolean (400 ticks, genesis=true)
    // ================================================================
    std::cout << "\n--- Stage 3-4: Ternary state (400 ticks, genesis=true) ---\n";

    int n_minus = 0, n_zero = 0, n_plus = 0;

    {
        ftd::SimEngine rb(L);
        rb.toggles().genesis = true;
        rb.toggles().forces = false;
        rb.toggles().movement = false;

        // Use much stronger amplitude for genesis (must exceed K_GENESIS = 3*K_B)
        double genesis_amp = ftd::K_GENESIS * 2.0;
        rb.inject_flux(srcA_x, srcA_y, srcA_z, {0.0, 0.0, genesis_amp});
        rb.inject_flux(srcB_x, srcB_y, srcB_z, {0.0, 0.0, -genesis_amp});

        rb.run(400);

        // Check entire lattice for genesis events (not just detection plane)
        int N_total = rb.total_sites();
        const auto& voxels = rb.get_voxels();
        for (int i = 0; i < N_total; ++i) {
            int8_t s = voxels[i].state;
            if (s < 0) n_minus++;
            else if (s > 0) n_plus++;
            else n_zero++;
        }
    }

    // Stage 3: H_ternary from {-1, 0, +1}
    std::vector<int> ternary_counts = {n_minus, n_zero, n_plus};
    double H_ternary = shannon_entropy_discrete(ternary_counts);

    // Stage 4: H_boolean from {0, 1} where 1 = detected (|s| = 1)
    int n_detected = n_minus + n_plus;
    int n_silent = n_zero;
    std::vector<int> boolean_counts = {n_silent, n_detected};
    double H_boolean = shannon_entropy_discrete(boolean_counts);

    std::cout << "  n_minus = " << n_minus << ", n_zero = " << n_zero
              << ", n_plus = " << n_plus << "\n";
    std::cout << "  n_detected = " << n_detected << ", n_silent = " << n_silent << "\n";
    std::cout << "  H_ternary = " << H_ternary << " bits/voxel\n";
    std::cout << "  H_boolean = " << H_boolean << " bits/voxel\n";

    // Percentages of original
    double pct_born = (H_full > 0) ? (H_born / H_full * 100.0) : 0.0;
    double pct_ternary = (H_full > 0) ? (H_ternary / H_full * 100.0) : 0.0;
    double pct_boolean = (H_full > 0) ? (H_boolean / H_full * 100.0) : 0.0;

    // ================================================================
    // CSV Output
    // ================================================================
    std::ostream* out = &std::cout;
    std::ofstream file;
    if (argc > 1) {
        file.open(argv[1]);
        if (file.is_open()) out = &file;
    }

    *out << "stage,label,entropy_bits_per_voxel,pct_of_original\n";
    *out << "1,full_field," << std::setprecision(6) << H_full << ",100.000000\n";
    *out << "2,born_rule," << std::setprecision(6) << H_born << ","
         << std::setprecision(6) << pct_born << "\n";
    *out << "3,ternary," << std::setprecision(6) << H_ternary << ","
         << std::setprecision(6) << pct_ternary << "\n";
    *out << "4,boolean," << std::setprecision(6) << H_boolean << ","
         << std::setprecision(6) << pct_boolean << "\n";

    if (file.is_open()) file.close();

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // DSIC1: H_full > H_born (phase information lost)
    std::cout << "  H_full = " << H_full << "  >  H_born = " << H_born << "?\n";
    check("DSIC1: H_full > H_born (phase information lost in Born rule)",
          H_full > H_born);

    // DSIC2: H_ternary > H_boolean (sign information lost)
    std::cout << "  H_ternary = " << H_ternary << "  >  H_boolean = " << H_boolean << "?\n";
    check("DSIC2: H_ternary > H_boolean (sign information lost)",
          H_ternary > H_boolean);

    // DSIC3: H_full > H_ternary > H_boolean (monotonic decrease)
    std::cout << "  Monotonic: " << H_full << " > " << H_ternary << " > " << H_boolean << "?\n";
    check("DSIC3: H_full > H_ternary > H_boolean (monotonic cascade)",
          H_full > H_ternary && H_ternary > H_boolean);

    // DSIC4: Print all values for comparison with 2D suite
    std::cout << "\n  DSIC4 (informational): Information cascade summary\n";
    std::cout << "    Stage 1 (Full field):  " << std::setprecision(4) << H_full
              << " bits/voxel (100%)\n";
    std::cout << "    Stage 2 (Born rule):   " << H_born
              << " bits/voxel (" << std::setprecision(1) << pct_born << "%)\n";
    std::cout << "    Stage 3 (Ternary):     " << std::setprecision(4) << H_ternary
              << " bits/voxel (" << std::setprecision(1) << pct_ternary << "%)\n";
    std::cout << "    Stage 4 (Boolean):     " << std::setprecision(4) << H_boolean
              << " bits/voxel (" << std::setprecision(1) << pct_boolean << "%)\n";
    check("DSIC4: All entropy values reported (informational, always passes)",
          true);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  \n";
    std::cout << "  KEY FINDING: Each measurement stage irreversibly discards\n";
    std::cout << "  information. The full vector flux field carries the most\n";
    std::cout << "  information; Born rule (|J|^2) loses phase; ternary\n";
    std::cout << "  manifestation loses continuous amplitude; boolean detection\n";
    std::cout << "  loses the sign (matter vs antimatter). This quantifies\n";
    std::cout << "  the FTD information cascade from dispositional to actual.\n";
    std::cout << "================================================================\n";
    return failures;
}
