/**
 * Test: Static Cluster-Cluster Interaction (Class C Phase C.1)
 *
 * Implements the Static Template of the Class C Infrastructure Specification (FTD-0222).
 * Verifies that the discrete Poisson potential gradient between two locked, static
 * point-source clusters recovers the physical 1/r^2 Coulomb force law asymptotically
 * at large relational coordinate separations r >> 1.
 *
 * Uses ClusterTracker for operational centroid tracking to demonstrate compatibility
 * with the FTD-0222 discrete-native measurement protocol.
 */

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/cluster_tracker.h"
#include "ftd/test_telemetry.h"

// Helper to extract the electrostatic potential gradient force on a point charge
static ftd::Vec3 extract_coulomb_force(const ftd::RenderBridge& rb, int cx, int cy, int cz) {
    int idx = rb.lattice().index(cx, cy, cz);
    int8_t state = rb.voxels()[idx].state;
    if (rb.toggles.poisson_coulomb && !rb.toggles.emergent_forces) {
        ftd::Vec3 gd = rb.gradient_scalar(idx, rb.phi_coulomb());
        return gd * (-ftd::ALPHA * state);
    }
    return {};
}

int main() {
    ftd::test::init("test_cluster_interaction_static");

    ftd::test::section("static_coulomb_force_law");

    constexpr int L = 48;
    const int mid = L / 2;
    constexpr int SETTLE_TICKS = 200;      // Let SOR Poisson solver warm-start and relax
    constexpr int SOR_ITERATIONS = 25;     // Scientific high-precision setting

    // radii sweep for force fitting
    std::vector<int> radii = {4, 6, 8, 10, 12};
    std::vector<double> forces;
    std::vector<double> log_r;
    std::vector<double> log_f;

    std::cout << "Starting Static Coulomb Force Law Sweep (L = " << L << ", SOR = " << SOR_ITERATIONS << ")\n";
    std::cout << "  r    |  relational_sep  |  F.x (attractive)  |  log(r)  |  log(|F|)\n";
    std::cout << "-------+------------------+--------------------+----------+-----------\n";

    // Configure ClusterTracker for tracking single-voxel point-like clusters
    ftd::ClusterTrackerParams params;
    params.use_moore_neighbors = true;
    params.min_cluster_size = 1; // track point particles
    params.overlap_threshold = 0.5;

    for (int r : radii) {
        // Fresh engine for each measurement to prevent inter-run flux pollution
        ftd::RenderBridge rb(L);
        rb.set_sor_iterations(SOR_ITERATIONS);
        rb.toggles.movement = false;
        rb.toggles.genesis = false;
        rb.toggles.gravity = false; // Isolate EM

        // 1. Inject a static locked +1 cluster at the center
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // 2. Inject a static locked -1 cluster at mid + r
        int probe_x = mid + r;
        rb.inject_particle(probe_x, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(probe_x, mid, mid)].locked = true;

        // Warm up and settle the field solver
        rb.run(SETTLE_TICKS);

        // 3. Track clusters using the ClusterTracker
        ftd::ClusterTracker tracker(params);
        tracker.record(rb);

        // Verify we tracked exactly two clusters
        bool two_tracked = (tracker.alive_count() == 2);

        double measured_r = r;
        if (two_tracked) {
            std::vector<const ftd::ClusterHistory*> hist;
            for (const auto& [_, h] : tracker.histories()) {
                hist.push_back(&h);
            }
            // Sort by centroid x to be deterministic
            std::sort(hist.begin(), hist.end(), [](const ftd::ClusterHistory* a, const ftd::ClusterHistory* b) {
                return a->snapshots.back().centroid_x < b->snapshots.back().centroid_x;
            });
            measured_r = hist[1]->snapshots.back().centroid_x - hist[0]->snapshots.back().centroid_x;
        }

        // 4. Extract electrostatic potential gradient force
        ftd::Vec3 f = extract_coulomb_force(rb, probe_x, mid, mid);
        forces.push_back(f.x); // opposite charges: F.x points leftward (< 0)

        double f_mag = std::abs(f.x);
        if (f_mag > 1e-25) {
            log_r.push_back(std::log(measured_r));
            log_f.push_back(std::log(f_mag));
        }

        std::cout << "  " << std::setw(3) << r
                  << "  |  " << std::setw(14) << std::fixed << std::setprecision(4) << measured_r
                  << "  |  " << std::setw(16) << std::scientific << std::setprecision(6) << f.x
                  << "  |  " << std::setw(6) << std::fixed << std::setprecision(4) << std::log(measured_r)
                  << "  |  " << std::setw(8) << (f_mag > 1e-25 ? std::log(f_mag) : -99.0)
                  << "\n";
    }

    // Perform linear regression to fit log(F) = -p*log(r) + log(g_lat)
    double exponent = 0.0;
    double r_squared = 0.0;
    double g_lat = 0.0;

    if (log_r.size() >= 3) {
        int n = static_cast<int>(log_r.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
        for (int i = 0; i < n; ++i) {
            sx += log_r[i];
            sy += log_f[i];
            sxx += log_r[i] * log_r[i];
            sxy += log_r[i] * log_f[i];
            syy += log_f[i] * log_f[i];
        }
        double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            exponent = (n * sxy - sx * sy) / denom;
            double intercept = (sy - exponent * sx) / n;
            g_lat = std::exp(intercept);

            // Compute coefficient of determination R²
            double ss_res = 0.0, ss_tot = 0.0;
            double ybar = sy / n;
            for (int i = 0; i < n; ++i) {
                double pred = exponent * log_r[i] + intercept;
                ss_res += (log_f[i] - pred) * (log_f[i] - pred);
                ss_tot += (log_f[i] - ybar) * (log_f[i] - ybar);
            }
            r_squared = 1.0 - ss_res / (ss_tot + 1e-30);
        }
    }

    std::cout << "\nCoulomb Force Law Fit Results:\n";
    std::cout << "  Fitted Exponent p = " << std::fixed << std::setprecision(4) << exponent << " (Theory: -2.0)\n";
    std::cout << "  R-squared (R²)    = " << r_squared << "\n";
    std::cout << "  Lattice Coupling  = " << std::scientific << g_lat << "\n\n";

    // Telemetry and checks
    ftd::test::check("Static force is attractive (F.x < 0)", forces[0] < 0);
    ftd::test::check("Force decreases monotonically with distance",
                     std::abs(forces[0]) > std::abs(forces[1]) &&
                     std::abs(forces[1]) > std::abs(forces[2]) &&
                     std::abs(forces[2]) > std::abs(forces[3]) &&
                     std::abs(forces[3]) > std::abs(forces[4]));
    ftd::test::check("Exponent in Coulomb range [-2.8, -1.5] (resolving Green's function recovery)",
                     exponent >= -2.8 && exponent <= -1.5);
    ftd::test::check("Good power-law fit (R² > 0.90)", r_squared > 0.90);

    return ftd::test::finalize();
}
