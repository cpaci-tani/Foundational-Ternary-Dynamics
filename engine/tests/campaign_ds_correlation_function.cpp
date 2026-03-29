/**
 * Campaign: Dual-Substrate Correlation Function E(theta)
 *
 * THE critical experiment: measures E(theta) from physically embedded
 * detectors on the 3D lattice. Three measurement modes compared:
 *
 * Mode 0 (Passive): External sign-projection measurement on propagated flux.
 *   Expected result: classical 2D sawtooth E(theta) = -(1 - 2|theta|/pi).
 *
 * Mode 1 (Active detectors, external measurement): Locked detector voxels
 *   with coupling enabled; still uses external sign-projection readout.
 *
 * Mode 2 (Active detectors, dynamical measurement): Detector response
 *   measured as flux CHANGE (delta) caused by arriving entangled pair.
 *
 * For each mode, 13 angles (0-180 deg, 15 deg steps), N_PAIRS=1000 per angle.
 * CHSH computed at optimal angles: a1=0, a2=pi/2, b1=pi/4, b2=3pi/4.
 *
 * Checks:
 *   DSCF1: E(0) < -0.8 for Mode 0 (strong anti-correlation)
 *   DSCF2: |E(90)| < 0.3 for Mode 0 (weak at orthogonal)
 *   DSCF3: Mode 0 matches classical within 15% (MAE)
 *   DSCF4: S_CHSH <= 2.05 for Mode 0
 *   DSCF5: Report all three S values honestly
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

// Sign function: returns +1 or -1 (never 0)
int sign_proj(double v) { return (v >= 0.0) ? +1 : -1; }

int main(int argc, char* argv[]) {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: DS Correlation Function E(theta) — 5 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 64;         // Full size — GPU handles this instantly
    const int mid = L / 2;
    const int N_PAIRS = 500;   // Good statistics on GPU
    const int N_ANGLES = 13;  // 0 to 180 in 15 deg steps
    const int N_MODES = 3;

    std::mt19937 rng(54321);
    std::uniform_real_distribution<double> phi_dist(0.0, 2.0 * ftd::PI);

    // Angle grid: 0, 15, 30, ..., 180 degrees
    std::vector<double> angles(N_ANGLES);
    for (int i = 0; i < N_ANGLES; ++i)
        angles[i] = i * ftd::PI / (N_ANGLES - 1);  // 0 to pi

    // E(theta) storage: [mode][angle_index]
    std::vector<std::vector<double>> E_meas(N_MODES, std::vector<double>(N_ANGLES, 0.0));
    std::vector<std::vector<double>> E_class(N_MODES, std::vector<double>(N_ANGLES, 0.0));
    std::vector<std::vector<double>> E_quant(N_MODES, std::vector<double>(N_ANGLES, 0.0));

    // Classical 2D sawtooth: E(theta) = -(1 - 2|theta|/pi)
    auto E_classical = [](double theta) {
        double t = std::fmod(std::abs(theta), 2.0 * M_PI);
        if (t > M_PI) t = 2.0 * M_PI - t;
        return -(1.0 - 2.0 * t / M_PI);
    };

    // ================================================================
    // Mode 0: Passive (external measurement)
    // ================================================================
    std::cout << "\n--- Mode 0: Passive (external measurement) ---\n";
    for (int ai = 0; ai < N_ANGLES; ++ai) {
        double theta = angles[ai];
        double sum_corr = 0.0;

        for (int p = 0; p < N_PAIRS; ++p) {
            ftd::SimEngine rb(L);
            rb.toggles().wave_propagation = true;
            rb.toggles().coupling = false;
            rb.toggles().genesis = false;
            rb.toggles().forces = false;
            rb.toggles().movement = false;

            double phi = phi_dist(rng);
            double amp = ftd::K_B;

            // Inject entangled pair manually
            rb.inject_particle(mid - 1, mid, mid, +1,
                               {amp * std::cos(phi), amp * std::sin(phi), 0.0});
            rb.inject_particle(mid + 1, mid, mid, -1,
                               {-amp * std::cos(phi), -amp * std::sin(phi), 0.0});

            // Propagate flux
            rb.run(40);

            // Read flux at detector positions
            const auto& vA = rb.voxel_at(mid - 10, mid, mid);
            const auto& vB = rb.voxel_at(mid + 10, mid, mid);

            // Detector A at 0 deg, detector B at theta
            int outcome_A = sign_proj(vA.flux.x * std::cos(0.0) + vA.flux.y * std::sin(0.0));
            int outcome_B = sign_proj(vB.flux.x * std::cos(theta) + vB.flux.y * std::sin(theta));

            sum_corr += outcome_A * outcome_B;
        }

        E_meas[0][ai] = sum_corr / N_PAIRS;
        E_class[0][ai] = E_classical(theta);
        E_quant[0][ai] = -std::cos(theta);
    }
    std::cout << "  Mode 0 complete.\n";

    // ================================================================
    // Mode 1: Active detectors, external measurement
    // ================================================================
    std::cout << "\n--- Mode 1: Active detectors (external measurement) ---\n";
    for (int ai = 0; ai < N_ANGLES; ++ai) {
        double theta = angles[ai];
        double sum_corr = 0.0;

        for (int p = 0; p < N_PAIRS; ++p) {
            ftd::SimEngine rb(L);
            rb.toggles().wave_propagation = true;
            rb.toggles().coupling = true;  // Enable g_c * grad(s)
            rb.toggles().genesis = false;
            rb.toggles().forces = false;
            rb.toggles().movement = false;

            // Place locked detector structures via inject_particle
            rb.inject_particle(mid - 10, mid, mid, +1, {0.01, 0, 0});
            rb.inject_particle(mid + 10, mid, mid, +1, {0.01, 0, 0});

            // Equilibrate detectors
            rb.run(50);

            double phi = phi_dist(rng);
            double amp = ftd::K_B;

            // Inject entangled pair
            rb.inject_particle(mid - 1, mid, mid, +1,
                               {amp * std::cos(phi), amp * std::sin(phi), 0.0});
            rb.inject_particle(mid + 1, mid, mid, -1,
                               {-amp * std::cos(phi), -amp * std::sin(phi), 0.0});

            // Propagate
            rb.run(40);

            // Read flux at detectors
            const auto& vA = rb.voxel_at(mid - 10, mid, mid);
            const auto& vB = rb.voxel_at(mid + 10, mid, mid);

            int outcome_A = sign_proj(vA.flux.x * std::cos(0.0) + vA.flux.y * std::sin(0.0));
            int outcome_B = sign_proj(vB.flux.x * std::cos(theta) + vB.flux.y * std::sin(theta));

            sum_corr += outcome_A * outcome_B;
        }

        E_meas[1][ai] = sum_corr / N_PAIRS;
        E_class[1][ai] = E_classical(theta);
        E_quant[1][ai] = -std::cos(theta);
    }
    std::cout << "  Mode 1 complete.\n";

    // ================================================================
    // Mode 2: Active detectors, dynamical measurement (delta flux)
    // ================================================================
    std::cout << "\n--- Mode 2: Active detectors (dynamical measurement) ---\n";
    for (int ai = 0; ai < N_ANGLES; ++ai) {
        double theta = angles[ai];
        double sum_corr = 0.0;

        for (int p = 0; p < N_PAIRS; ++p) {
            ftd::SimEngine rb(L);
            rb.toggles().wave_propagation = true;
            rb.toggles().coupling = true;
            rb.toggles().genesis = false;
            rb.toggles().forces = false;
            rb.toggles().movement = false;

            // Place locked detector structures via inject_particle
            rb.inject_particle(mid - 10, mid, mid, +1, {0.01, 0, 0});
            rb.inject_particle(mid + 10, mid, mid, +1, {0.01, 0, 0});

            // Equilibrate
            rb.run(50);

            // Record baseline flux at detectors
            ftd::Vec3 base_A = rb.voxel_at(mid - 10, mid, mid).flux;
            ftd::Vec3 base_B = rb.voxel_at(mid + 10, mid, mid).flux;

            double phi = phi_dist(rng);
            double amp = ftd::K_B;

            // Inject entangled pair
            rb.inject_particle(mid - 1, mid, mid, +1,
                               {amp * std::cos(phi), amp * std::sin(phi), 0.0});
            rb.inject_particle(mid + 1, mid, mid, -1,
                               {-amp * std::cos(phi), -amp * std::sin(phi), 0.0});

            // Propagate
            rb.run(40);

            // Measure flux CHANGE at detectors
            ftd::Vec3 post_A = rb.voxel_at(mid - 10, mid, mid).flux;
            ftd::Vec3 post_B = rb.voxel_at(mid + 10, mid, mid).flux;

            ftd::Vec3 delta_A = {post_A.x - base_A.x, post_A.y - base_A.y, post_A.z - base_A.z};
            ftd::Vec3 delta_B = {post_B.x - base_B.x, post_B.y - base_B.y, post_B.z - base_B.z};

            // outcome = sign(delta_flux dot detector_sensitivity_axis)
            int outcome_A = sign_proj(delta_A.x * std::cos(0.0) + delta_A.y * std::sin(0.0));
            int outcome_B = sign_proj(delta_B.x * std::cos(theta) + delta_B.y * std::sin(theta));

            sum_corr += outcome_A * outcome_B;
        }

        E_meas[2][ai] = sum_corr / N_PAIRS;
        E_class[2][ai] = E_classical(theta);
        E_quant[2][ai] = -std::cos(theta);
    }
    std::cout << "  Mode 2 complete.\n";

    // ================================================================
    // CHSH computation for each mode
    // ================================================================
    // Optimal CHSH angles: a1=0, a2=pi/2, b1=pi/4, b2=3pi/4
    // We need E at relative angles: a1-b1=pi/4, a1-b2=3pi/4, a2-b1=pi/4, a2-b2=pi/4
    // But our E(theta) is measured at 15-deg steps. Nearest indices:
    //   pi/4 = 45 deg -> index 3
    //   3pi/4 = 135 deg -> index 9
    //   pi/2 - pi/4 = pi/4 -> index 3
    //   pi/2 - 3pi/4 = -pi/4 -> use |pi/4| -> index 3
    // S = |E(a1,b1) - E(a1,b2)| + |E(a2,b1) + E(a2,b2)|
    // E(a1,b1) = E(|a1-b1|) = E(pi/4), E(a1,b2) = E(3pi/4)
    // E(a2,b1) = E(|pi/2 - pi/4|) = E(pi/4), E(a2,b2) = E(|pi/2 - 3pi/4|) = E(pi/4)
    int idx_45 = 3;   // 45 deg
    int idx_135 = 9;  // 135 deg

    std::vector<double> S_chsh(N_MODES, 0.0);
    for (int m = 0; m < N_MODES; ++m) {
        double E_a1b1 = E_meas[m][idx_45];   // E(pi/4)
        double E_a1b2 = E_meas[m][idx_135];  // E(3pi/4)
        double E_a2b1 = E_meas[m][idx_45];   // E(pi/4)
        double E_a2b2 = E_meas[m][idx_45];   // E(pi/4)
        S_chsh[m] = std::abs(E_a1b1 - E_a1b2) + std::abs(E_a2b1 + E_a2b2);
    }

    // ================================================================
    // CSV Output
    // ================================================================
    std::ostream* out = &std::cout;
    std::ofstream file;
    if (argc > 1) {
        file.open(argv[1]);
        if (file.is_open()) out = &file;
    }

    *out << "mode,theta_deg,E_measured,E_classical,E_quantum,n_pairs\n";
    for (int m = 0; m < N_MODES; ++m) {
        for (int ai = 0; ai < N_ANGLES; ++ai) {
            double deg = angles[ai] * 180.0 / ftd::PI;
            *out << m << ","
                 << std::setprecision(1) << deg << ","
                 << std::setprecision(6) << E_meas[m][ai] << ","
                 << std::setprecision(6) << E_class[m][ai] << ","
                 << std::setprecision(6) << E_quant[m][ai] << ","
                 << N_PAIRS << "\n";
        }
    }
    if (file.is_open()) file.close();

    // Print CHSH results to stdout
    std::cout << "\n--- CHSH Results ---\n";
    for (int m = 0; m < N_MODES; ++m) {
        std::cout << "  CHSH Mode " << m << ": S = "
                  << std::setprecision(4) << S_chsh[m] << "\n";
    }

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // DSCF1: E(0) < -0.8 for Mode 0 (strong anti-correlation)
    std::cout << "  E(0) Mode 0 = " << E_meas[0][0] << "\n";
    check("DSCF1: E(0) < -0.8 for Mode 0 (strong anti-correlation)",
          E_meas[0][0] < -0.8);

    // DSCF2: |E(90)| < 0.3 for Mode 0 (weak correlation at orthogonal)
    int idx_90 = N_ANGLES / 2;  // index 6 = 90 deg
    std::cout << "  E(90) Mode 0 = " << E_meas[0][idx_90] << "\n";
    check("DSCF2: |E(90)| < 0.3 for Mode 0 (weak at orthogonal)",
          std::abs(E_meas[0][idx_90]) < 0.3);

    // DSCF3: Mode 0 matches classical within 15% (mean absolute error)
    double mae = 0.0;
    for (int ai = 0; ai < N_ANGLES; ++ai) {
        mae += std::abs(E_meas[0][ai] - E_class[0][ai]);
    }
    mae /= N_ANGLES;
    std::cout << "  Mode 0 MAE vs classical: " << mae << "\n";
    check("DSCF3: Mode 0 matches classical within 15% (MAE)",
          mae < 0.15);

    // DSCF4: S_CHSH <= 2.05 for Mode 0 (allows small statistical noise)
    std::cout << "  S_CHSH Mode 0 = " << S_chsh[0] << "\n";
    check("DSCF4: S_CHSH <= 2.05 for Mode 0",
          S_chsh[0] <= 2.05);

    // DSCF5: Report all three S values honestly
    bool all_finite = std::isfinite(S_chsh[0]) &&
                      std::isfinite(S_chsh[1]) &&
                      std::isfinite(S_chsh[2]);
    check("DSCF5: All three S values reported (finite)",
          all_finite);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  \n";
    std::cout << "  NOTE: Mode 0 is the substrate (local HV) baseline.\n";
    std::cout << "  Modes 1-2 test whether active detector structures or\n";
    std::cout << "  dynamical measurement modify the correlation function.\n";
    std::cout << "  Any S > 2 departure from Mode 0 would indicate the\n";
    std::cout << "  measurement apparatus physically alters correlations.\n";
    std::cout << "================================================================\n";
    return failures;
}
