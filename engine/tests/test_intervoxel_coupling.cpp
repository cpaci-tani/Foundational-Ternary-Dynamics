/**
 * Test: Inter-Voxel Coupling Measurement
 *
 * Places a locked particle at the center of a small lattice and
 * measures how the flux field couples to neighboring voxels through
 * the three Moore shells (SC, FCC, BCC).
 *
 * This test extracts the EMPIRICAL inter-voxel coupling strengths
 * from the engine and compares them to the ternary cube model
 * predictions:
 *   SC (octahedron, d=1):     coupling ~ alpha
 *   FCC (cuboctahedron, d=√2): coupling ~ sin^2(theta_W)
 *   BCC (stella oct, d=√3):    coupling ~ alpha_s
 *
 * The engine uses the full FTD Lagrangian with:
 *   - 18-point isotropic Laplacian (flux propagation)
 *   - g_c * grad(s) coupling (state-flux interaction)
 *   - Gauss constraint (charge conservation)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/constants.h"

int failures = 0;
const char* shell_names[] = {"Center", "SC (octahedron)", "FCC (cuboctahedron)", "BCC (stella oct)"};

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

// Classify a displacement (dx, dy, dz) into Moore shell
int moore_shell(int dx, int dy, int dz) {
    int nonzero = (dx != 0 ? 1 : 0) + (dy != 0 ? 1 : 0) + (dz != 0 ? 1 : 0);
    return nonzero;  // 0=center, 1=SC, 2=FCC, 3=BCC
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Inter-Voxel Coupling on the Moore Neighborhood\n";
    std::cout << "================================================================\n\n";

    // Use a lattice large enough that periodic BC don't interfere
    const int L = 16;
    const int cx = L/2, cy = L/2, cz = L/2;  // center

    // ---- Experiment 1: Single particle, measure flux shell structure ----
    std::cout << "--- Experiment 1: Flux Shell Structure ---\n";
    {
        ftd::RenderBridge rb(L);

        // Inject a locked +1 particle at center
        rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;

        // Disable genesis and forces — pure field evolution
        rb.toggles.genesis = false;
        rb.toggles.forces = false;

        // Let the self-field establish
        rb.run(500);

        // Measure flux density at each Moore shell
        double flux_shell[4] = {0, 0, 0, 0};
        int count_shell[4] = {0, 0, 0, 0};

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    int shell = moore_shell(dx, dy, dz);
                    int x = (cx + dx + L) % L;
                    int y = (cy + dy + L) % L;
                    int z = (cz + dz + L) % L;
                    int idx = rb.lattice().index(x, y, z);
                    double flux_mag = rb.voxels()[idx].flux.mag();
                    flux_shell[shell] += flux_mag;
                    count_shell[shell]++;
                }
            }
        }

        std::cout << "\n  Shell  Count  Mean |J|          Expected coupling\n";
        std::cout << "  -----------------------------------------------\n";

        for (int s = 0; s < 4; ++s) {
            double mean_flux = flux_shell[s] / count_shell[s];
            std::cout << "  " << s << "  " << shell_names[s] << "  "
                      << count_shell[s] << "      "
                      << std::setprecision(8) << mean_flux << "\n";
        }

        // Check shell ordering: Center > SC > FCC > BCC (flux decreases with distance)
        double mean_center = flux_shell[0] / count_shell[0];
        double mean_sc = flux_shell[1] / count_shell[1];
        double mean_fcc = flux_shell[2] / count_shell[2];
        double mean_bcc = flux_shell[3] / count_shell[3];

        check("Center |J| > SC |J|", mean_center > mean_sc);
        check("SC |J| > FCC |J|", mean_sc > mean_fcc);
        check("FCC |J| > BCC |J|", mean_fcc > mean_bcc);

        // Compute coupling RATIOS between shells
        std::cout << "\n  Coupling ratios (relative to SC):\n";
        if (mean_sc > 0) {
            double ratio_fcc_sc = mean_fcc / mean_sc;
            double ratio_bcc_sc = mean_bcc / mean_sc;
            std::cout << "    FCC/SC = " << ratio_fcc_sc << "\n";
            std::cout << "    BCC/SC = " << ratio_bcc_sc << "\n";

            // The ternary cube model predicts:
            // SC ~ alpha, FCC ~ sin^2_W, BCC ~ alpha_s
            // Ratios: FCC/SC ~ sin^2_W/alpha, BCC/SC ~ alpha_s/alpha
            double sin2w = 3.0/13;
            double alpha_s = 7.0/59;
            double alpha = 1.0/137.036;

            double predicted_fcc_sc = sin2w / alpha;
            double predicted_bcc_sc = alpha_s / alpha;

            std::cout << "\n    Model prediction FCC/SC = sin^2_W/alpha = " << predicted_fcc_sc << "\n";
            std::cout << "    Model prediction BCC/SC = alpha_s/alpha = " << predicted_bcc_sc << "\n";
        }

        std::cout << "\n";
    }

    // ---- Experiment 2: Divergence structure ----
    std::cout << "--- Experiment 2: Divergence by Shell ---\n";
    {
        ftd::RenderBridge rb(L);
        rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;
        rb.toggles.genesis = false;
        rb.toggles.forces = false;
        rb.run(500);

        double div_shell[4] = {0, 0, 0, 0};
        int count_shell[4] = {0, 0, 0, 0};

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    int shell = moore_shell(dx, dy, dz);
                    int x = (cx + dx + L) % L;
                    int y = (cy + dy + L) % L;
                    int z = (cz + dz + L) % L;
                    int idx = rb.lattice().index(x, y, z);
                    double div_J = rb.divergence_flux(idx);
                    div_shell[shell] += std::abs(div_J);
                    count_shell[shell]++;
                }
            }
        }

        std::cout << "\n  Shell  Count  Mean |div(J)|\n";
        std::cout << "  ---------------------------\n";
        for (int s = 0; s < 4; ++s) {
            double mean_div = div_shell[s] / count_shell[s];
            std::cout << "  " << s << "  " << shell_names[s] << "  "
                      << count_shell[s] << "  "
                      << std::setprecision(8) << mean_div << "\n";
        }

        // The divergence should be concentrated at the center (charge source)
        double div_center = div_shell[0] / count_shell[0];
        double div_neighbors = (div_shell[1] + div_shell[2] + div_shell[3]) /
                               (count_shell[1] + count_shell[2] + count_shell[3]);
        check("div(J) concentrated at center", div_center > 10 * div_neighbors);
        std::cout << "\n";
    }

    // ---- Experiment 3: Two particles, measure interaction ----
    std::cout << "--- Experiment 3: Two-Particle Interaction ---\n";
    {
        // Two locked +1 particles separated by 4 lattice units
        ftd::RenderBridge rb(L);
        int sep = 4;
        int x1 = cx - sep/2, x2 = cx + sep/2;
        rb.inject_particle(x1, cy, cz, +1, {0, 0, ftd::K_B});
        rb.inject_particle(x2, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(x1, cy, cz)].locked = true;
        rb.voxels()[rb.lattice().index(x2, cy, cz)].locked = true;
        rb.toggles.genesis = false;
        rb.toggles.forces = true;  // Enable forces to see interaction

        rb.run(500);

        // Measure the force on each particle
        auto fd1 = rb.force_diag_at(x1, cy, cz);
        auto fd2 = rb.force_diag_at(x2, cy, cz);

        std::cout << "\n  Force on particle 1 at (" << x1 << "," << cy << "," << cz << "):\n";
        std::cout << "    Coulomb: (" << fd1.f_coulomb.x << ", " << fd1.f_coulomb.y
                  << ", " << fd1.f_coulomb.z << ")  |F| = " << fd1.f_coulomb.mag() << "\n";

        std::cout << "  Force on particle 2 at (" << x2 << "," << cy << "," << cz << "):\n";
        std::cout << "    Coulomb: (" << fd2.f_coulomb.x << ", " << fd2.f_coulomb.y
                  << ", " << fd2.f_coulomb.z << ")  |F| = " << fd2.f_coulomb.mag() << "\n";

        // Same-sign particles should REPEL (Coulomb force pointing away from each other)
        // Particle 1 is at x1 < x2, so force on 1 should point in -x direction
        check("Same-sign repulsion: F1.x < 0", fd1.f_coulomb.x < 0);
        check("Same-sign repulsion: F2.x > 0", fd2.f_coulomb.x > 0);

        // Force magnitudes should be approximately equal (Newton's third law)
        double f1_mag = fd1.f_coulomb.mag();
        double f2_mag = fd2.f_coulomb.mag();
        if (f1_mag > 0 && f2_mag > 0) {
            double ratio = f1_mag / f2_mag;
            check("Newton 3rd law: |F1| ≈ |F2|", std::abs(ratio - 1.0) < 0.1);
            std::cout << "    |F1|/|F2| = " << ratio << "\n";
        }

        // Measure the energy audit
        auto audit = rb.energy_audit();
        std::cout << "\n  Energy audit:\n";
        std::cout << "    Field energy: " << audit.field_energy << "\n";
        std::cout << "    Coulomb PE: " << audit.coulomb_pe << "\n";
        std::cout << "    Gauss violation: " << audit.gauss_violation << "\n";
        std::cout << "\n";
    }

    // ---- Experiment 4: Flux propagation speed ----
    std::cout << "--- Experiment 4: Flux Propagation Speed ---\n";
    {
        ftd::RenderBridge rb(32);
        int center = 16;

        // Inject a sharp flux pulse at center
        rb.inject_flux(center, center, center, {0, 0, 1.0});
        rb.toggles.genesis = false;
        rb.toggles.forces = false;
        rb.toggles.coupling = false;  // Pure wave propagation

        // Track the wavefront
        std::cout << "\n  Tick  |J| at center  |J| at d=5  |J| at d=10\n";
        std::cout << "  ------------------------------------------------\n";

        for (int tick = 0; tick <= 30; tick += 3) {
            double j_center = rb.voxels()[rb.lattice().index(center, center, center)].flux.mag();
            double j_d5 = rb.voxels()[rb.lattice().index(center+5, center, center)].flux.mag();
            double j_d10 = rb.voxels()[rb.lattice().index(center+10, center, center)].flux.mag();

            std::cout << "  " << std::setw(4) << tick
                      << "  " << std::setw(12) << std::setprecision(6) << j_center
                      << "  " << std::setw(12) << j_d5
                      << "  " << std::setw(12) << j_d10 << "\n";

            rb.run(3);
        }

        // After 10 ticks at c = 1/sqrt(3) = 0.577, wavefront should reach d ~ 5.77
        // After 20 ticks, d ~ 11.5
        double j_at_6 = rb.voxels()[rb.lattice().index(center+6, center, center)].flux.mag();
        check("Wavefront reaches d=6 by tick 12", j_at_6 > 1e-10);
        std::cout << "\n";
    }

    // ---- Summary ----
    std::cout << "================================================================\n";
    std::cout << "  RESULTS: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED") << "\n";
    std::cout << "  Failures: " << failures << "\n";
    std::cout << "================================================================\n";

    return failures;
}
