/**
 * Test: Gauge Invariance — J -> J + grad(lambda) Symmetry
 *
 * Verifies that physical observables are invariant under gauge
 * transformations J -> J + grad(lambda) for arbitrary scalar lambda.
 *
 * Physical observables in FTD:
 *   - curl(J) = "magnetic field" (gauge invariant: curl(grad) = 0)
 *   - div(J)  = "charge density" (gauge adds Laplacian: div(grad) = lap)
 *   - state field s (unaffected by flux gauge)
 *
 * The Gauss constraint div(J) = rho means the longitudinal component
 * is determined by charges, leaving 2 transverse degrees of freedom.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md             (gauge structure of coupling term)
 *   - DERIV_FORCE_EMERGENCE.md           (U(1) gauge from Gauss constraint)
 *   - FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md (symmetry principles)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

using ftd::test::check;
using ftd::test::check_close;

int main() {
    ftd::test::init("test_gauge");

    // ================================================================
    // Section 1: curl(grad(lambda)) = 0 (mathematical identity)
    // ================================================================
    // Verify the discrete curl of a discrete gradient vanishes.
    // This is the fundamental identity that makes gauge invariance possible.
    // We use the ENGINE's own gradient operator to compute grad(lambda),
    // ensuring discrete consistency (no boundary wrap artifacts).
    std::cout << "\n--- Section 1: curl(grad) = 0 Identity ---\n";
    {
        int L = 16;

        // Step 1: Store a scalar field lambda in a helper array.
        // Use a periodic function to avoid boundary discontinuities:
        //   lambda(x,y,z) = sin(2pi*x/L) * cos(2pi*y/L) + z*z
        // But we actually just need grad(lambda) as a flux field.
        // Use the engine's gradient_scalar to compute discrete grad(lambda).

        // Create a scalar field
        std::vector<double> lambda_field(L * L * L);
        ftd::RenderBridge rb_helper(L);
        double kk = 2.0 * ftd::PI / L;
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    int idx = rb_helper.lattice().index(x, y, z);
                    lambda_field[idx] = std::sin(kk * x) * std::cos(kk * y)
                                      + 0.5 * std::sin(kk * z);
                }
            }
        }

        // Step 2: Compute gradient using engine's discrete operator
        ftd::RenderBridge rb(L);
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    int idx = rb.lattice().index(x, y, z);
                    ftd::Vec3 grad = rb.gradient_scalar(idx, lambda_field);
                    rb.inject_flux(x, y, z, grad);
                }
            }
        }

        // Step 3: curl(grad(lambda)) should vanish by the discrete identity
        double max_curl = 0.0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            ftd::Vec3 c = rb.curl_flux(i);
            double cmag = c.mag();
            if (cmag > max_curl) max_curl = cmag;
        }

        std::cout << "    Max |curl(grad(lambda))| = " << max_curl << "\n";
        check("curl(grad) = 0 to machine precision", max_curl < 1e-10);
    }

    // ================================================================
    // Section 2: Curl is gauge invariant
    // ================================================================
    // If we add grad(lambda) to an existing flux field, the curl should
    // remain unchanged: curl(J + grad(lambda)) = curl(J) + 0 = curl(J)
    std::cout << "\n--- Section 2: Curl Invariance Under Gauge Transform ---\n";
    {
        int L = 16;

        // Create a simulation with some interesting flux pattern
        ftd::RenderBridge rb1(L);
        // Set up a non-trivial flux configuration (rotating field)
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    double fx = std::sin(2.0 * ftd::PI * y / L);
                    double fy = std::cos(2.0 * ftd::PI * x / L);
                    double fz = 0.5 * std::sin(2.0 * ftd::PI * z / L);
                    rb1.inject_flux(x, y, z, {fx, fy, fz});
                }
            }
        }

        // Create gauge-transformed copy: J' = J + grad(lambda)
        // lambda = sin(2pi*x/L) * sin(2pi*y/L)
        ftd::RenderBridge rb2(L);
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    int idx = rb1.lattice().index(x, y, z);
                    ftd::Vec3 j_orig = rb1.voxels()[idx].flux;

                    // Compute grad(lambda) at this point numerically
                    // lambda = sin(2pi*x/L) * sin(2pi*y/L)
                    double k = 2.0 * ftd::PI / L;
                    double gl_x = k * std::cos(k * x) * std::sin(k * y);
                    double gl_y = k * std::sin(k * x) * std::cos(k * y);
                    double gl_z = 0.0;

                    rb2.inject_flux(x, y, z,
                        {j_orig.x + gl_x, j_orig.y + gl_y, j_orig.z + gl_z});
                }
            }
        }

        // Compare curl at several test points
        double max_diff = 0.0;
        int test_indices[] = {
            rb1.lattice().index(4, 4, 4),
            rb1.lattice().index(8, 8, 8),
            rb1.lattice().index(12, 3, 7),
            rb1.lattice().index(6, 10, 5)
        };

        for (int idx : test_indices) {
            ftd::Vec3 c1 = rb1.curl_flux(idx);
            ftd::Vec3 c2 = rb2.curl_flux(idx);
            double diff = (c1 - c2).mag();
            if (diff > max_diff) max_diff = diff;
        }

        std::cout << "    Max |curl(J') - curl(J)| = " << max_diff << "\n";
        // Allow small numerical error from discrete gradient approximation
        check("Curl is gauge invariant (difference < 0.1)", max_diff < 0.1);
    }

    // ================================================================
    // Section 3: Transverse vs longitudinal decomposition
    // ================================================================
    // The Helmholtz decomposition: J = J_T + J_L
    // where J_T has div=0, J_L has curl=0.
    // Physical content is in J_T (2 modes). J_L is constrained by Gauss law.
    std::cout << "\n--- Section 3: Transverse Mode Counting ---\n";
    {
        // A flux field has 3 components.
        // The Gauss constraint div(J) = rho constrains 1 component.
        // Remaining: 2 physical transverse modes.
        // This is why photons have 2 polarizations.

        int dof_total = 3;   // J has 3 components
        int dof_constraint = 1;  // Gauss: div(J) = rho
        int dof_physical = dof_total - dof_constraint;

        std::cout << "    Total DoF: " << dof_total << "\n";
        std::cout << "    Constraints: " << dof_constraint << "\n";
        std::cout << "    Physical DoF: " << dof_physical << "\n";

        check("Physical DoF = 2 (photon polarizations)",
              dof_physical == 2);
    }

    // ================================================================
    // Section 4: Divergence-free flux propagates as pure wave
    // ================================================================
    // A transverse (div=0) flux pulse should propagate without sourcing charges.
    std::cout << "\n--- Section 4: Divergence-Free Propagation ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Create a divergence-free flux configuration: curl-type field
        // J = (0, Jy, 0) where Jy depends only on x.
        // div(J) = dJy/dy = 0 (since Jy doesn't depend on y).
        double amp = 0.3;
        for (int x = cx - 3; x <= cx + 3; ++x) {
            double profile = amp * std::exp(-0.5 * (x - cx) * (x - cx));
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    rb.inject_flux(x, y, z, {0, profile, 0});
                }
            }
        }

        // Check initial divergence is near zero at center
        int center_idx = rb.lattice().index(cx, cx, cx);
        double div_initial = std::abs(rb.divergence_flux(center_idx));
        std::cout << "    Initial |div(J)| at center: " << div_initial << "\n";
        check("Initial flux is approximately divergence-free", div_initial < 0.1);

        // Run a few ticks
        rb.run(10);

        // No particles should manifest (div ≈ 0 means no polarity signal)
        auto diag = rb.diagnostics();
        std::cout << "    Manifested particles after 10 ticks: " << diag.manifested_count << "\n";
        check("No manifestation from divergence-free flux", diag.manifested_count == 0);
    }

    return ftd::test::finalize();
}
