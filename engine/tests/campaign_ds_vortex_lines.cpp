/**
 * Campaign: Double-Slit Vortex Lines
 *
 * 3D vortex filament detection in a double-slit interference field.
 * Two sources with opposite y-flux and slight z-offset create a
 * topologically rich interference pattern. Vortex cores (phase
 * singularities) are detected via winding number on 2x2 plaquettes,
 * then connected into filaments by flood-fill.
 *
 * Setup:
 *   - L=64, two sources with opposite y-flux, z-offset by 4
 *   - Source A: (22,32,30) flux = {0, +amplitude, 0}
 *   - Source B: (42,32,34) flux = {0, -amplitude, 0}
 *   - amplitude = K_B * 0.3
 *   - Toggles: genesis=false, forces=false, movement=false
 *   - Run 200 ticks
 *
 * Output CSV: vortex locations with winding, filament_id, J_mag
 *
 * Checks:
 *   DSVL1: Total vortex count > 0
 *   DSVL2: At least one filament has length > 3
 *   DSVL3: Mean |J| at vortex sites < 0.1 * max |J| in field
 *   DSVL4: Print total vortex count and number of distinct filaments
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

// Wrap a phase difference into [-pi, pi]
double wrap_phase(double dp) {
    while (dp > M_PI) dp -= 2.0 * M_PI;
    while (dp < -M_PI) dp += 2.0 * M_PI;
    return dp;
}

int main(int argc, char* argv[]) {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Double-Slit Vortex Lines — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 64;
    const int mid = L / 2;  // 32
    const double amplitude = ftd::K_B * 0.3;

    // ================================================================
    // Setup
    // ================================================================
    std::cout << "\n--- Setup ---\n";
    std::cout << "  Lattice: " << L << "^3\n";
    std::cout << "  Source A: (22, " << mid << ", 30) flux = {0, +" << amplitude << ", 0}\n";
    std::cout << "  Source B: (42, " << mid << ", 34) flux = {0, -" << amplitude << ", 0}\n";

    ftd::SimEngine rb(L);
    rb.toggles().genesis = false;
    rb.toggles().forces = false;
    rb.toggles().movement = false;

    // Inject y-directed flux with opposite signs and z-offset
    rb.inject_flux(22, mid, 30, {0.0, amplitude, 0.0});
    rb.inject_flux(42, mid, 34, {0.0, -amplitude, 0.0});

    // Evolve
    rb.run(200);

    // ================================================================
    // Compute global max |J| for later comparison
    // ================================================================
    double global_max_mag = 0.0;
    int N_total = rb.total_sites();
    const auto& all_voxels = rb.get_voxels();
    for (int i = 0; i < N_total; ++i) {
        double m = all_voxels[i].flux.mag();
        if (m > global_max_mag) global_max_mag = m;
    }
    std::cout << "  Global max |J|: " << std::scientific << global_max_mag << "\n";

    // ================================================================
    // Vortex detection: scan 2x2 plaquettes in x-y plane at each z
    // ================================================================
    struct Vortex {
        int x, y, z;
        double winding;
        int filament_id;
        double J_mag;
    };
    std::vector<Vortex> vortices;

    // For each z-slice, scan x-y plaquettes
    for (int z = 0; z < L; ++z) {
        for (int x = 0; x + 1 < L; ++x) {
            for (int y = 0; y + 1 < L; ++y) {
                // Four corners of the plaquette
                auto& v00 = rb.voxel_at(x, y, z);
                auto& v10 = rb.voxel_at(x + 1, y, z);
                auto& v11 = rb.voxel_at(x + 1, y + 1, z);
                auto& v01 = rb.voxel_at(x, y + 1, z);

                // Phase at each corner: theta = atan2(Jy, Jx)
                double theta00 = std::atan2(v00.flux.y, v00.flux.x);
                double theta10 = std::atan2(v10.flux.y, v10.flux.x);
                double theta11 = std::atan2(v11.flux.y, v11.flux.x);
                double theta01 = std::atan2(v01.flux.y, v01.flux.x);

                // Winding number: sum of wrapped phase differences around plaquette
                double winding = wrap_phase(theta10 - theta00)
                               + wrap_phase(theta11 - theta10)
                               + wrap_phase(theta01 - theta11)
                               + wrap_phase(theta00 - theta01);

                if (std::abs(winding) > M_PI) {
                    // Vortex detected — record at plaquette center
                    double mag = 0.25 * (v00.flux.mag() + v10.flux.mag()
                                       + v11.flux.mag() + v01.flux.mag());
                    vortices.push_back({x, y, z, winding, -1, mag});
                }
            }
        }
    }

    std::cout << "  Total vortices detected: " << vortices.size() << "\n";

    // ================================================================
    // Filament assignment: greedy flood-fill (union-find)
    // Two vortices are connected if they differ by at most 1 in each coord.
    // ================================================================
    // Simple union-find
    std::vector<int> parent(vortices.size());
    for (size_t i = 0; i < vortices.size(); ++i) parent[i] = static_cast<int>(i);

    // Find with path compression
    auto find = [&](int i) -> int {
        while (parent[i] != i) {
            parent[i] = parent[parent[i]];
            i = parent[i];
        }
        return i;
    };

    // Union
    auto unite = [&](int a, int b) {
        int ra = find(a);
        int rb_id = find(b);
        if (ra != rb_id) parent[ra] = rb_id;
    };

    // Connect adjacent vortices (differ by at most 1 in each coord)
    for (size_t i = 0; i < vortices.size(); ++i) {
        for (size_t j = i + 1; j < vortices.size(); ++j) {
            int dx = std::abs(vortices[i].x - vortices[j].x);
            int dy = std::abs(vortices[i].y - vortices[j].y);
            int dz = std::abs(vortices[i].z - vortices[j].z);
            if (dx <= 1 && dy <= 1 && dz <= 1) {
                unite(static_cast<int>(i), static_cast<int>(j));
            }
        }
    }

    // Assign filament IDs and count sizes
    std::vector<int> filament_size;
    std::vector<int> root_to_id;
    int n_filaments = 0;

    for (size_t i = 0; i < vortices.size(); ++i) {
        int root = find(static_cast<int>(i));
        // Find or create filament ID for this root
        int fid = -1;
        for (size_t k = 0; k < root_to_id.size(); k += 2) {
            if (root_to_id[k] == root) {
                fid = root_to_id[k + 1];
                break;
            }
        }
        if (fid < 0) {
            fid = n_filaments++;
            root_to_id.push_back(root);
            root_to_id.push_back(fid);
            filament_size.push_back(0);
        }
        vortices[i].filament_id = fid;
        filament_size[fid]++;
    }

    // Find longest filament
    int max_filament_length = 0;
    for (int sz : filament_size) {
        if (sz > max_filament_length) max_filament_length = sz;
    }

    std::cout << "  Distinct filaments: " << n_filaments << "\n";
    std::cout << "  Longest filament:   " << max_filament_length << " vortices\n";

    // Mean |J| at vortex sites
    double sum_mag = 0.0;
    for (auto& vx : vortices) sum_mag += vx.J_mag;
    double mean_vortex_mag = (vortices.size() > 0)
        ? sum_mag / vortices.size()
        : 0.0;
    std::cout << "  Mean |J| at vortex cores: " << std::scientific << mean_vortex_mag << "\n";
    std::cout << "  Threshold (0.1 * max |J|): " << 0.1 * global_max_mag << "\n";

    // ================================================================
    // Output CSV
    // ================================================================
    bool use_file = (argc > 1);
    std::ofstream fout;
    if (use_file) fout.open(argv[1]);
    std::ostream& out = use_file ? fout : std::cout;

    out << "x,y,z,winding,filament_id,J_mag\n";
    for (auto& vx : vortices) {
        out << vx.x << "," << vx.y << "," << vx.z << ","
            << std::fixed << std::setprecision(6) << vx.winding << ","
            << vx.filament_id << ","
            << std::scientific << std::setprecision(8) << vx.J_mag << "\n";
    }
    if (use_file) {
        fout.close();
        std::cout << "  CSV written to: " << argv[1] << "\n";
    }

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // DSVL1: Total vortex count > 0
    check("DSVL1: Total vortex count > 0",
          vortices.size() > 0);

    // DSVL2: At least one filament has length > 3
    check("DSVL2: At least one filament has length > 3 (connected chain)",
          max_filament_length > 3);

    // DSVL3: Mean |J| at vortex sites < 0.1 * max |J| (dark spots)
    bool dark_cores = (global_max_mag > 1e-20)
        ? (mean_vortex_mag < 0.1 * global_max_mag)
        : false;
    check("DSVL3: Vortex cores are dark (mean |J| < 0.1 * max |J|)",
          dark_cores);

    // DSVL4: Print summary
    std::cout << "  DSVL4 summary: " << vortices.size() << " vortices in "
              << n_filaments << " filaments\n";
    check("DSVL4: Vortex filaments detected and counted",
          vortices.size() > 0 && n_filaments > 0);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
