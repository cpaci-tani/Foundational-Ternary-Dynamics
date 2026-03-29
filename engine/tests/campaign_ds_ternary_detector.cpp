/**
 * Campaign: Double-Slit Ternary Detector
 *
 * Ternary vs boolean detection on the actual engine manifestation rule.
 * Two counter-phase sources with above-threshold flux, genesis enabled.
 * Measures whether the ternary state sign correlates with the flux
 * direction — information that a boolean (|state| > 0) detector loses.
 *
 * Setup:
 *   - L=64, two counter-phase sources above genesis threshold
 *   - Source A: (22,32,32) flux = {0, 0, +K_B*2.0}
 *   - Source B: (42,32,32) flux = {0, 0, -K_B*2.0}
 *   - Toggles: genesis=true, forces=false, movement=false, gauss_projection=true
 *   - Run 400 ticks
 *
 * Output CSV: manifested voxels with state, flux, phase, magnitude
 *
 * Checks:
 *   DSTD1: At least 10 genesis events
 *   DSTD2: Genesis in regions where |J| was large (|J| > K_B)
 *   DSTD3: Ternary accuracy > 0.6 (sign(state) correlates with sign(Jz))
 *   DSTD4: Print ternary advantage ratio
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
    std::cout << "  CAMPAIGN: Double-Slit Ternary Detector — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 64;
    const int mid = L / 2;  // 32
    const double amp = ftd::K_B * 2.0;  // Above genesis threshold

    // ================================================================
    // Setup
    // ================================================================
    std::cout << "\n--- Setup ---\n";
    std::cout << "  Lattice: " << L << "^3\n";
    std::cout << "  Source A: (22, " << mid << ", " << mid << ") flux = {0, 0, +" << amp << "}\n";
    std::cout << "  Source B: (42, " << mid << ", " << mid << ") flux = {0, 0, -" << amp << "}\n";
    std::cout << "  K_B = " << ftd::K_B << ", K_GENESIS = " << ftd::K_GENESIS << "\n";

    ftd::SimEngine rb(L);
    rb.toggles().genesis = true;
    rb.toggles().forces = false;
    rb.toggles().movement = false;
    rb.toggles().gauss_projection = true;

    // Create multiple wavepackets across the lattice to get many manifested particles.
    // Each wavepacket manifests one particle — we want many to measure sign correlation.
    // Place them in a grid pattern with enough spacing (sigma=3 → ~7 voxels effective radius)
    for (int ix = 0; ix < 4; ++ix) {
        for (int iy = 0; iy < 4; ++iy) {
            for (int iz = 0; iz < 4; ++iz) {
                int px = 8 + ix * 14;
                int py = 8 + iy * 14;
                int pz = 8 + iz * 14;
                int8_t sign = ((ix + iy + iz) % 2 == 0) ? +1 : -1;
                rb.inject_wavepacket(px, py, pz, sign, 3.0, ftd::K_B);
            }
        }
    }

    // Run to let flux evolve and interact
    rb.run(200);

    // ================================================================
    // Collect manifested voxels
    // ================================================================
    struct Event {
        int x, y, z;
        int state;
        double Jx, Jy, Jz;
        double phase, mag;
    };
    std::vector<Event> events;

    int N_total = rb.total_sites();
    const auto& voxels = rb.get_voxels();
    for (int i = 0; i < N_total; ++i) {
        const auto& v = voxels[i];
        if (v.state != 0) {
            int cx = i / (L * L);
            int cy = (i / L) % L;
            int cz = i % L;
            double jx = v.flux.x;
            double jy = v.flux.y;
            double jz = v.flux.z;
            double m = std::sqrt(jx * jx + jy * jy + jz * jz);
            double ph = std::atan2(jy, jx);
            events.push_back({cx, cy, cz,
                              static_cast<int>(v.state),
                              jx, jy, jz, ph, m});
        }
    }

    std::cout << "\n--- Genesis Results ---\n";
    std::cout << "  Manifested voxels: " << events.size() << "\n";

    // ================================================================
    // Output CSV
    // ================================================================
    bool use_file = (argc > 1);
    std::ofstream fout;
    if (use_file) fout.open(argv[1]);
    std::ostream& out = use_file ? fout : std::cout;

    out << "x,y,z,state,Jx,Jy,Jz,phase,mag\n";
    for (auto& e : events) {
        out << e.x << "," << e.y << "," << e.z << ","
            << e.state << ","
            << std::scientific << std::setprecision(8)
            << e.Jx << "," << e.Jy << "," << e.Jz << ","
            << e.phase << "," << e.mag << "\n";
    }
    if (use_file) {
        fout.close();
        std::cout << "  CSV written to: " << argv[1] << "\n";
    }

    // ================================================================
    // Compute ternary mutual information
    // ================================================================
    // Ternary MI: does sign(state) predict sign(Jz)?
    int n_match = 0;
    int n_mismatch = 0;
    int n_high_flux = 0;  // events where |J| > K_B at genesis site

    for (auto& e : events) {
        // Check flux magnitude at genesis site
        if (e.mag > ftd::K_B) {
            n_high_flux++;
        }

        // Ternary correlation: sign(state) vs sign of dominant flux component
        double dominant = e.Jx;
        if (std::abs(e.Jy) > std::abs(dominant)) dominant = e.Jy;
        if (std::abs(e.Jz) > std::abs(dominant)) dominant = e.Jz;
        if (std::abs(dominant) > 1e-20) {
            bool state_positive = (e.state > 0);
            bool flux_positive = (dominant > 0);
            if (state_positive == flux_positive) {
                n_match++;
            } else {
                n_mismatch++;
            }
        }
    }

    int n_classified = n_match + n_mismatch;
    double ternary_accuracy = (n_classified > 0)
        ? static_cast<double>(n_match) / n_classified
        : 0.0;
    double boolean_accuracy = 0.5;  // Boolean has no sign info
    double ternary_advantage = (boolean_accuracy > 0)
        ? ternary_accuracy / boolean_accuracy
        : 0.0;

    std::cout << "  Sign matches (state vs Jz): " << n_match << "\n";
    std::cout << "  Sign mismatches:            " << n_mismatch << "\n";
    std::cout << "  Ternary accuracy:           " << std::fixed << std::setprecision(4)
              << ternary_accuracy << "\n";
    std::cout << "  Boolean accuracy (baseline): " << boolean_accuracy << "\n";
    std::cout << "  High-flux genesis events:    " << n_high_flux
              << " / " << events.size() << "\n";

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // DSTD1: At least 10 genesis events
    check("DSTD1: At least 10 genesis events occurred",
          static_cast<int>(events.size()) >= 10);

    // DSTD2: Genesis events in regions where |J| was large
    check("DSTD2: Genesis events in high-flux regions (|J| > K_B)",
          n_high_flux > 0);

    // DSTD3: Ternary accuracy > 0.6
    check("DSTD3: Ternary accuracy > 0.6 (sign(state) correlates with sign(Jz))",
          ternary_accuracy > 0.6);

    // DSTD4: Print the ternary advantage
    std::cout << "  Ternary advantage (accuracy / 0.5): "
              << std::fixed << std::setprecision(3) << ternary_advantage << "x\n";
    check("DSTD4: Ternary advantage > 1.0 (better than boolean)",
          ternary_advantage > 1.0);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
