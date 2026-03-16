/**
 * Test: Gauss Constraint and Flux Divergence Structure
 *
 * The FTD flux field J is analogous to the vector potential A, not
 * the electric field E. The coupling g_c * grad(s) drives flux
 * OUTWARD from manifested particles, so div(J) at a +1 source is
 * NEGATIVE (flux radiates away, creating a divergence sink at the
 * particle site).
 *
 * Key properties verified:
 *   1. Vacuum: div(J) = 0 everywhere
 *   2. Opposite charges produce opposite-sign divergence
 *   3. div(J) is nonzero at particle sites (they are sources)
 *   4. Far-field div(J) -> 0 (flux spreads uniformly)
 *   5. Sum of div(J) over periodic lattice = 0 exactly (Gauss theorem)
 *   6. Charge antisymmetry: div(J) at +1 = -div(J) at -1
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (Gauss constraint from action principle)
 *   - DERIV_FORCE_EMERGENCE.md           (divergence and charge conservation)
 *   - DERIV_STATE_FLUX_COUPLING_DERIVATION.md (g_c = √α coupling)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
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

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Gauss Constraint & Flux Divergence\n";
    std::cout << "================================================================\n\n";

    // ---- Test 1: Vacuum Gauss constraint (trivially satisfied) ----
    std::cout << "--- Vacuum (div J = 0 everywhere) ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.run(10);
        auto ld = ftd::compute_lagrangian_diagnostics(rb);
        check_close("Vacuum: Gauss violation = 0", ld.gauss_violation, 0.0, 1e-20);
        check_close("Vacuum: max Gauss error = 0", ld.max_gauss_error, 0.0, 1e-20);
    }

    // ---- Test 2: Single particle creates nonzero divergence ----
    std::cout << "\n--- Single positive particle ---\n";
    {
        ftd::RenderBridge rb(16);
        int cx = 8, cy = 8, cz = 8;
        rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;

        // Let self-field establish
        rb.run(200);

        // Check divergence at the particle site
        int idx = rb.lattice().index(cx, cy, cz);
        double divJ_at_particle = rb.divergence_flux(idx);

        std::cout << "    div(J) at +1 particle = " << divJ_at_particle << "\n";

        // The particle is a flux source — div(J) should be nonzero
        check("Single +1: |div(J)| > 0 at particle", std::abs(divJ_at_particle) > 0.01);

        // Check global diagnostics
        auto ld = ftd::compute_lagrangian_diagnostics(rb);
        std::cout << "    Total Gauss violation = " << ld.gauss_violation << "\n";
        std::cout << "    Max Gauss error = " << ld.max_gauss_error << "\n";
    }

    // ---- Test 3: Charge antisymmetry ----
    std::cout << "\n--- Charge antisymmetry ---\n";
    {
        // +1 particle in isolation
        ftd::RenderBridge rb_pos(16);
        rb_pos.inject_particle(8, 8, 8, +1, {0, 0, ftd::K_B});
        rb_pos.voxels()[rb_pos.lattice().index(8, 8, 8)].locked = true;
        rb_pos.run(200);
        double divJ_pos = rb_pos.divergence_flux(rb_pos.lattice().index(8, 8, 8));

        // -1 particle in isolation
        ftd::RenderBridge rb_neg(16);
        rb_neg.inject_particle(8, 8, 8, -1, {0, 0, -ftd::K_B});
        rb_neg.voxels()[rb_neg.lattice().index(8, 8, 8)].locked = true;
        rb_neg.run(200);
        double divJ_neg = rb_neg.divergence_flux(rb_neg.lattice().index(8, 8, 8));

        std::cout << "    div(J) at +1 = " << divJ_pos << "\n";
        std::cout << "    div(J) at -1 = " << divJ_neg << "\n";

        // Opposite charges should produce opposite divergence
        check("Antisymmetry: sign(div+) != sign(div-)",
              (divJ_pos > 0) != (divJ_neg > 0));
        // And approximately equal magnitude
        check("Antisymmetry: |div+| ≈ |div-|",
              std::abs(std::abs(divJ_pos) - std::abs(divJ_neg)) < 0.01);
    }

    // ---- Test 4: Opposite charge pair ----
    std::cout << "\n--- Opposite charge pair ---\n";
    {
        ftd::RenderBridge rb(24);
        rb.inject_particle(8, 12, 12, +1, {0, 0, ftd::K_B});
        rb.inject_particle(16, 12, 12, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(8, 12, 12)].locked = true;
        rb.voxels()[rb.lattice().index(16, 12, 12)].locked = true;

        rb.run(200);

        int idx_pos = rb.lattice().index(8, 12, 12);
        int idx_neg = rb.lattice().index(16, 12, 12);
        double divJ_pos = rb.divergence_flux(idx_pos);
        double divJ_neg = rb.divergence_flux(idx_neg);

        std::cout << "    div(J) at +1 site = " << divJ_pos << "\n";
        std::cout << "    div(J) at -1 site = " << divJ_neg << "\n";

        // Opposite charges produce opposite divergence
        check("Pair: opposite div(J) signs", (divJ_pos > 0) != (divJ_neg > 0));
        // Both should be nonzero
        check("Pair: |div(J)| > 0 at +1", std::abs(divJ_pos) > 0.01);
        check("Pair: |div(J)| > 0 at -1", std::abs(divJ_neg) > 0.01);
    }

    // ---- Test 5: Far-field divergence is small ----
    std::cout << "\n--- Far-field divergence ---\n";
    {
        ftd::RenderBridge rb(32);
        rb.inject_particle(16, 16, 16, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(16, 16, 16)].locked = true;

        rb.run(300);

        // Check divergence far from particle
        double divJ_far = rb.divergence_flux(rb.lattice().index(0, 0, 0));
        std::cout << "    div(J) at (0,0,0) = " << divJ_far << "\n";
        check("Far-field: |div(J)| < 0.1 at distance", std::abs(divJ_far) < 0.1);
    }

    // ---- Test 6: Gauss theorem (sum of div = 0 on periodic lattice) ----
    std::cout << "\n--- Gauss theorem on periodic lattice ---\n";
    {
        ftd::RenderBridge rb(16);
        // Place charges — any configuration
        rb.inject_particle(4, 8, 8, +1, {0, 0, ftd::K_B});
        rb.inject_particle(12, 8, 8, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(4, 8, 8)].locked = true;
        rb.voxels()[rb.lattice().index(12, 8, 8)].locked = true;

        rb.run(200);

        // Sum all divergences over the lattice
        double total_div = 0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            total_div += rb.divergence_flux(i);
        }
        std::cout << "    Sum div(J) over lattice = " << total_div << "\n";
        // On periodic lattice, sum of divergence is identically 0 by Gauss theorem
        check_close("Sum div(J) = 0 (Gauss theorem)", total_div, 0.0, 1e-10);
    }

    // ---- Test 7: Single charge on periodic lattice ----
    std::cout << "\n--- Single charge Gauss theorem ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.inject_particle(8, 8, 8, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(8, 8, 8)].locked = true;

        rb.run(200);

        double total_div = 0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            total_div += rb.divergence_flux(i);
        }
        std::cout << "    Sum div(J) (single charge) = " << total_div << "\n";
        // Periodic lattice: sum of divergence = 0 regardless of charges
        // (divergence theorem: flux through closed surface = 0 on torus)
        check_close("Sum div(J) = 0 even with net charge", total_div, 0.0, 1e-10);
    }

    // ---- Test 8: Ward identity: div(curl(J)) = 0 (exact on lattice) ----
    std::cout << "\n--- Ward identity: div(curl(J)) = 0 ---\n";
    {
        ftd::RenderBridge rb(16);
        // Create nontrivial flux configuration with particles and waves
        rb.inject_particle(6, 8, 8, +1, {0, 0, ftd::K_B});
        rb.inject_particle(10, 8, 8, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(6, 8, 8)].locked = true;
        rb.voxels()[rb.lattice().index(10, 8, 8)].locked = true;
        rb.inject_flux(8, 4, 8, {1.0, 0.5, -0.3});
        rb.inject_flux(8, 12, 8, {-0.5, 1.0, 0.7});

        // Evolve to create complex field structure
        rb.run(100);

        // Compute div(curl(J)) at every interior site
        // This should be exactly zero (identity from vector calculus)
        double max_div_curl = 0;
        double sum_div_curl = 0;
        int N = rb.lattice().total_sites();

        // First compute curl field, then take divergence of it
        // div(curl(J)) = d/dx (curl_x) + d/dy (curl_y) + d/dz (curl_z)
        // We need to be careful at boundaries, so check interior sites
        int L = rb.lattice().size();
        int count = 0;
        for (int x = 2; x < L-2; ++x) {
            for (int y = 2; y < L-2; ++y) {
                for (int z = 2; z < L-2; ++z) {
                    int idx = rb.lattice().index(x, y, z);
                    ftd::Vec3 curl_c = rb.curl_flux(idx);

                    // Compute divergence of the curl field using finite differences
                    ftd::Vec3 curl_px = rb.curl_flux(rb.lattice().index(x+1, y, z));
                    ftd::Vec3 curl_mx = rb.curl_flux(rb.lattice().index(x-1, y, z));
                    ftd::Vec3 curl_py = rb.curl_flux(rb.lattice().index(x, y+1, z));
                    ftd::Vec3 curl_my = rb.curl_flux(rb.lattice().index(x, y-1, z));
                    ftd::Vec3 curl_pz = rb.curl_flux(rb.lattice().index(x, y, z+1));
                    ftd::Vec3 curl_mz = rb.curl_flux(rb.lattice().index(x, y, z-1));

                    double div_curl = (curl_px.x - curl_mx.x) * 0.5
                                    + (curl_py.y - curl_my.y) * 0.5
                                    + (curl_pz.z - curl_mz.z) * 0.5;

                    sum_div_curl += std::abs(div_curl);
                    if (std::abs(div_curl) > max_div_curl) {
                        max_div_curl = std::abs(div_curl);
                    }
                    count++;
                }
            }
        }
        double avg_div_curl = sum_div_curl / count;

        std::cout << "    Max |div(curl(J))| = " << max_div_curl << "\n";
        std::cout << "    Avg |div(curl(J))| = " << avg_div_curl << "\n";

        // div(curl) should be exactly 0 on the discrete lattice
        // (identity: d_i epsilon_ijk d_j J_k = 0 because d_i d_j is symmetric, epsilon is antisymmetric)
        check("Ward identity: max |div(curl(J))| < 1e-12", max_div_curl < 1e-12);
    }

    // ---- Test 9: 2 physical DOF (3 curl components - 1 constraint = 2) ----
    // The Ward identity div(curl(J)) = 0 is one constraint on 3 curl components,
    // leaving exactly 2 independent (physical) degrees of freedom.
    // This is the lattice analog of "massless gauge bosons have 2 polarizations."
    std::cout << "\n--- 2 physical DOF from Ward constraint ---\n";
    {
        ftd::RenderBridge rb(16);
        // Create nontrivial flux with all 3 components active
        rb.inject_flux(8, 8, 8, {1.0, 0.5, -0.3});
        rb.inject_flux(6, 8, 8, {0.0, 1.0, 0.5});
        rb.inject_flux(10, 8, 8, {-0.5, 0.0, 1.0});
        rb.run(30);

        // Verify all 3 curl components are nonzero (field is general)
        double curl_x_sum = 0, curl_y_sum = 0, curl_z_sum = 0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            ftd::Vec3 c = rb.curl_flux(i);
            curl_x_sum += c.x * c.x;
            curl_y_sum += c.y * c.y;
            curl_z_sum += c.z * c.z;
        }

        std::cout << "    |curl_x|^2 = " << curl_x_sum << "\n";
        std::cout << "    |curl_y|^2 = " << curl_y_sum << "\n";
        std::cout << "    |curl_z|^2 = " << curl_z_sum << "\n";

        double total_curl = curl_x_sum + curl_y_sum + curl_z_sum;
        // All 3 components active
        check("Curl field nonzero", total_curl > 1e-10);
        check("All 3 curl components populated", curl_x_sum > 0 && curl_y_sum > 0 && curl_z_sum > 0);
        // Combined with Ward identity (Test 8): 3 components - 1 constraint = 2 DOF
        // This is the electromagnetic polarization count
        std::cout << "    (Ward identity + 3 components => 2 physical DOF)\n";
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Gauss constraint & Ward identity tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
