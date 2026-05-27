/**
 * Test: Dynamic Cluster-Cluster Interaction (Class C Phase C.2)
 *
 * Implements the Dynamical Scattering Protocol of the Class C Infrastructure Specification (FTD-0222).
 * Simulates two moving opposite-sign point-like clusters injected on parallel axes
 * with opposite initial velocities. Measures the resulting Coulomb attraction deflection
 * angle using ClusterTracker centroid trajectories.
 *
 * Verifies that a smaller impact parameter (initial y-offset) produces a larger
 * scattering deflection angle, confirming the relational force law operationally.
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

// Struct to hold scattering simulation results
struct ScatteringResult {
    double initial_offset = 0.0;
    double final_x1 = 0.0;
    double final_y1 = 0.0;
    double final_x2 = 0.0;
    double final_y2 = 0.0;
    double deflection_angle_deg1 = 0.0;
    double deflection_angle_deg2 = 0.0;
    bool attractive = false;
};

// Runs one scattering simulation with a given initial y-offset (impact parameter)
static ScatteringResult run_scattering_sim(int L, int initial_y_offset, int ticks) {
    ftd::RenderBridge rb(L);
    rb.force_cpu(); // Force CPU execution to guarantee exact coordinate-based drift updates
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces           = true;
    rb.toggles.poisson_coulomb  = true;
    rb.toggles.movement         = true;
    rb.toggles.gravity          = false; // isolate EM

    int mid = L / 2;
    int y1 = mid - initial_y_offset / 2;
    int y2 = mid + initial_y_offset / 2 + (initial_y_offset % 2);

    // Highly charged clusters (state = +/- 8) to produce visible, discrete grid jumps in y-deflection
    int8_t q1 = +8;
    int8_t q2 = -8;

    // Positive charge starts at x=12, y=y1, moving right (vx = 0.1)
    int start_x1 = 12;
    rb.inject_particle(start_x1, y1, mid, q1, {0, 0, ftd::K_B * q1});
    int idx1 = rb.lattice().index(start_x1, y1, mid);
    rb.voxels()[idx1].velocity = {0.1, 0.0, 0.0};

    // Negative charge starts at x=36, y=y2, moving left (vx = -0.1)
    int start_x2 = 36;
    rb.inject_particle(start_x2, y2, mid, q2, {0, 0, ftd::K_B * q2});
    int idx2 = rb.lattice().index(start_x2, y2, mid);
    rb.voxels()[idx2].velocity = {-0.1, 0.0, 0.0};

    // Track coordinates at each tick
    std::vector<double> x1_trajectory = {double(start_x1)};
    std::vector<double> y1_trajectory = {double(y1)};
    std::vector<double> x2_trajectory = {double(start_x2)};
    std::vector<double> y2_trajectory = {double(y2)};

    // Ticks loop
    for (int t = 1; t <= ticks; ++t) {
        rb.tick();

        // Scan lattice to locate the positive and negative charge voxels
        int px1 = -1, py1 = -1;
        int px2 = -1, py2 = -1;
        for (int idx = 0; idx < L*L*L; ++idx) {
            if (rb.voxels()[idx].state > 0) {
                auto coord = rb.lattice().coord(idx);
                px1 = coord.x; py1 = coord.y;
            }
            if (rb.voxels()[idx].state < 0) {
                auto coord = rb.lattice().coord(idx);
                px2 = coord.x; py2 = coord.y;
            }
        }

        if (px1 != -1 && py1 != -1) {
            x1_trajectory.push_back(px1);
            y1_trajectory.push_back(py1);
        }
        if (px2 != -1 && py2 != -1) {
            x2_trajectory.push_back(px2);
            y2_trajectory.push_back(py2);
        }
    }

    ScatteringResult result;
    result.initial_offset = initial_y_offset;

    if (x1_trajectory.size() > 1 && x2_trajectory.size() > 1) {
        result.final_x1 = x1_trajectory.back();
        result.final_y1 = y1_trajectory.back();
        result.final_x2 = x2_trajectory.back();
        result.final_y2 = y2_trajectory.back();

        // Compute overall displacement vectors
        double dx1 = result.final_x1 - start_x1;
        double dy1 = result.final_y1 - y1;
        double dx2 = result.final_x2 - start_x2;
        double dy2 = result.final_y2 - y2;

        // Deflection angles (theta = arctan(|dy| / |dx|))
        double angle1 = std::atan2(std::abs(dy1), std::abs(dx1));
        result.deflection_angle_deg1 = angle1 * (180.0 / ftd::PI);

        double angle2 = std::atan2(std::abs(dy2), std::abs(dx2));
        result.deflection_angle_deg2 = angle2 * (180.0 / ftd::PI);

        // Check if the deflection is attractive (they pull closer in the y axis)
        // Mathematically, final distance in y is smaller than initial distance:
        // (y2 + dy2) - (y1 + dy1) < y2 - y1  =>  dy2 - dy1 < 0
        // Since we expect opposite charges to attract, dy1 should pull towards +y (>=0)
        // and dy2 should pull towards -y (<=0), with at least one moving.
        result.attractive = (dy1 >= 0.0 && dy2 <= 0.0 && (dy1 > 0.0 || dy2 < 0.0));

        std::cout << "  [Trajectory Trace]\n";
        std::cout << "    Particle 1 (positive): start=(" << start_x1 << ", " << y1
                  << ") -> final=(" << result.final_x1 << ", " << result.final_y1 << "), dy=" << dy1 << "\n";
        std::cout << "    Particle 2 (negative): start=(" << start_x2 << ", " << y2
                  << ") -> final=(" << result.final_x2 << ", " << result.final_y2 << "), dy=" << dy2 << "\n";
    }

    return result;
}

int main() {
    ftd::test::init("test_cluster_interaction_dynamic");

    ftd::test::section("dynamic_soliton_scattering");

    constexpr int L = 48;
    constexpr int TICKS = 200;

    std::cout << "Starting Dynamic Soliton Scattering Experiment (L = " << L << ", Ticks = " << TICKS << ")\n";

    // 1. Run scattering at small impact parameter (impact parameter = 4)
    std::cout << "\nRunning Simulation A: Small Impact Parameter (separation = 4)\n";
    ScatteringResult resA = run_scattering_sim(L, 4, TICKS);
    std::cout << "  Initial Offset: " << resA.initial_offset << "\n";
    std::cout << "  Particle 1 Deflection: " << std::fixed << std::setprecision(4) << resA.deflection_angle_deg1 << " deg\n";
    std::cout << "  Particle 2 Deflection: " << resA.deflection_angle_deg2 << " deg\n";
    std::cout << "  Interaction Type:      " << (resA.attractive ? "ATTRACTIVE" : "REPULSIVE/OTHER") << "\n";

    // 2. Run scattering at larger impact parameter (impact parameter = 8)
    std::cout << "\nRunning Simulation B: Larger Impact Parameter (separation = 8)\n";
    ScatteringResult resB = run_scattering_sim(L, 8, TICKS);
    std::cout << "  Initial Offset: " << resB.initial_offset << "\n";
    std::cout << "  Particle 1 Deflection: " << std::fixed << std::setprecision(4) << resB.deflection_angle_deg1 << " deg\n";
    std::cout << "  Particle 2 Deflection: " << resB.deflection_angle_deg2 << " deg\n";
    std::cout << "  Interaction Type:      " << (resB.attractive ? "ATTRACTIVE" : "REPULSIVE/OTHER") << "\n\n";

    // Telemetry and checks
    ftd::test::check("Sim A: Interaction is attractive (particles deflected towards each other)", resA.attractive);
    ftd::test::check("Sim B: Interaction is attractive (particles deflected towards each other)", resB.attractive);
    ftd::test::check("Deflection is non-zero in scattering (Sim A)", resA.deflection_angle_deg1 > 0.1);

    // Crucial check: smaller impact parameter leads to a larger scattering deflection angle
    ftd::test::check("Scattering: smaller offset yields larger deflection angle (theta_A > theta_B)",
                     resA.deflection_angle_deg1 > resB.deflection_angle_deg1);

    return ftd::test::finalize();
}
