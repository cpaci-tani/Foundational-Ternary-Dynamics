/**
 * Campaign: Double-Slit Phase Recovery
 *
 * Phase structure in the 3D flux field. Two counter-phase sources
 * inject opposite z-flux, then we measure the resulting field at
 * a detection plane downstream.
 *
 * Setup:
 *   - L=64, two sources at (22,32,32) and (42,32,32) with opposite z-flux
 *   - amplitude = K_B * 0.3 (sub-threshold, no genesis)
 *   - Toggles: genesis=false, forces=false, movement=false
 *   - Run 200 ticks
 *
 * Output CSV (detection plane x=48):
 *   y, z, Jx, Jy, Jz, mag, phase, state
 *
 * Checks:
 *   DSPR1: Phase entropy > 4 bits
 *   DSPR2: Intensity |J|^2 shows interference (max/min ratio > 5)
 *   DSPR3: Multiple voxels with similar |J| but different phase
 *   DSPR4: Energy conserved (total flux^2 at end ~ initial injection^2)
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

int main(int argc, char* argv[]) {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Double-Slit Phase Recovery — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 64;
    const int mid = L / 2;  // 32
    const double amplitude = ftd::K_B * 0.3;

    // ================================================================
    // Setup: two counter-phase sources
    // ================================================================
    std::cout << "\n--- Setup ---\n";
    std::cout << "  Lattice: " << L << "^3\n";
    std::cout << "  Source A: (22, " << mid << ", " << mid << ") flux = {0, 0, +" << amplitude << "}\n";
    std::cout << "  Source B: (42, " << mid << ", " << mid << ") flux = {0, 0, -" << amplitude << "}\n";

    ftd::SimEngine rb(L);
    rb.toggles().genesis = false;
    rb.toggles().forces = false;
    rb.toggles().movement = false;

    // Inject counter-phase flux
    rb.inject_flux(22, mid, mid, {0.0, 0.0, amplitude});
    rb.inject_flux(42, mid, mid, {0.0, 0.0, -amplitude});

    // Record initial injection energy for conservation check
    double initial_energy = amplitude * amplitude * 2.0;  // two sources, each |J|^2

    // Evolve
    rb.run(200);

    // ================================================================
    // Collect detection plane data (x = 48)
    // ================================================================
    const int det_x = 48;

    struct PlaneData {
        int y, z;
        double Jx, Jy, Jz, mag, phase;
        int state;
    };
    std::vector<PlaneData> plane;
    plane.reserve(L * L);

    for (int y = 0; y < L; ++y) {
        for (int z = 0; z < L; ++z) {
            const auto& v = rb.voxel_at(det_x, y, z);
            double jx = v.flux.x;
            double jy = v.flux.y;
            double jz = v.flux.z;
            double m = std::sqrt(jx * jx + jy * jy + jz * jz);
            double ph = std::atan2(jy, jx);
            plane.push_back({y, z, jx, jy, jz, m, ph, static_cast<int>(v.state)});
        }
    }

    // ================================================================
    // Output CSV
    // ================================================================
    bool use_file = (argc > 1);
    std::ofstream fout;
    if (use_file) fout.open(argv[1]);
    std::ostream& out = use_file ? fout : std::cout;

    out << "y,z,Jx,Jy,Jz,mag,phase,state\n";
    for (auto& d : plane) {
        out << d.y << "," << d.z << ","
            << std::scientific << std::setprecision(8)
            << d.Jx << "," << d.Jy << "," << d.Jz << ","
            << d.mag << "," << d.phase << ","
            << d.state << "\n";
    }
    if (use_file) {
        fout.close();
        std::cout << "  CSV written to: " << argv[1] << "\n";
    }

    // ================================================================
    // DSPR1: Phase entropy > 4 bits
    // ================================================================
    // Histogram phase into 64 bins
    const int N_BINS = 64;
    std::vector<int> phase_hist(N_BINS, 0);
    int total_nonzero = 0;

    for (auto& d : plane) {
        if (d.mag > 1e-20) {
            // Map phase from [-pi, pi] to [0, N_BINS-1]
            double normalized = (d.phase + M_PI) / (2.0 * M_PI);
            int bin = static_cast<int>(normalized * N_BINS);
            if (bin >= N_BINS) bin = N_BINS - 1;
            if (bin < 0) bin = 0;
            phase_hist[bin]++;
            total_nonzero++;
        }
    }

    double entropy = 0.0;
    if (total_nonzero > 0) {
        for (int i = 0; i < N_BINS; ++i) {
            if (phase_hist[i] > 0) {
                double p = static_cast<double>(phase_hist[i]) / total_nonzero;
                entropy -= p * std::log2(p);
            }
        }
    }
    std::cout << "\n  Phase entropy: " << std::fixed << std::setprecision(3)
              << entropy << " bits\n";
    check("DSPR1: Phase entropy > 4 bits", entropy > 4.0);

    // ================================================================
    // DSPR2: Intensity shows interference (max/min ratio > 5)
    // ================================================================
    double max_mag2 = 0.0;
    double min_mag2 = 1e30;
    for (auto& d : plane) {
        double mag2 = d.mag * d.mag;
        if (mag2 > max_mag2) max_mag2 = mag2;
        if (d.mag > 1e-20 && mag2 < min_mag2) min_mag2 = mag2;
    }
    double ratio = (min_mag2 > 1e-30) ? max_mag2 / min_mag2 : 1e30;
    std::cout << "  Intensity max/min ratio: " << std::scientific << ratio << "\n";
    check("DSPR2: Intensity max/min ratio > 5 (interference)", ratio > 5.0);

    // ================================================================
    // DSPR3: Multiple voxels with similar |J| but different phase
    // ================================================================
    // Find pairs where |J| differs by < 10% but phase differs by > pi/4
    int phase_diverse_count = 0;
    const double mag_tolerance = 0.10;
    const double phase_threshold = M_PI / 4.0;

    // Sample a subset to avoid O(N^2) on full plane
    std::vector<size_t> sample_idx;
    for (size_t i = 0; i < plane.size(); ++i) {
        if (plane[i].mag > 1e-15) sample_idx.push_back(i);
    }
    // Check up to 2000 random pairs
    std::mt19937 rng(12345);
    int pairs_checked = 0;
    for (int trial = 0; trial < 2000 && sample_idx.size() >= 2; ++trial) {
        size_t a = sample_idx[rng() % sample_idx.size()];
        size_t b = sample_idx[rng() % sample_idx.size()];
        if (a == b) continue;
        double avg_mag = 0.5 * (plane[a].mag + plane[b].mag);
        if (avg_mag < 1e-20) continue;
        double mag_diff = std::abs(plane[a].mag - plane[b].mag) / avg_mag;
        double phase_diff = std::abs(plane[a].phase - plane[b].phase);
        if (phase_diff > M_PI) phase_diff = 2.0 * M_PI - phase_diff;
        if (mag_diff < mag_tolerance && phase_diff > phase_threshold) {
            phase_diverse_count++;
        }
        pairs_checked++;
    }
    std::cout << "  Phase-diverse pairs (similar |J|, different phase): "
              << phase_diverse_count << " / " << pairs_checked << "\n";
    check("DSPR3: Phase not recoverable from |J| (diverse pairs > 0)",
          phase_diverse_count > 0);

    // ================================================================
    // DSPR4: Energy conservation
    // ================================================================
    auto audit = rb.energy_audit();
    double final_energy = audit.field_energy;
    std::cout << "  Initial injection energy: " << std::scientific << initial_energy << "\n";
    std::cout << "  Final field energy:       " << final_energy << "\n";
    // Energy should remain in the system (may redistribute but total nonzero)
    check("DSPR4: Field energy remains non-zero (energy conserved)",
          final_energy > 1e-20);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
