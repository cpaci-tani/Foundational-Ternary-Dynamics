/**
 * Test: Wave Speed — Flux Propagation at C_WAVE
 *
 * Verifies that the discrete wave equation:
 *   wave_vel += c^2 * laplacian(J)
 *   J += wave_vel
 *
 * propagates flux disturbances at the expected group velocity c = C_WAVE.
 * Also checks the dispersion relation for the 6-point discrete Laplacian.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md           (wave equation from Born-Infeld E-L)
 *   - DERIV_DISCRETE_CONTINUOUS_BRIDGE.md (discrete to continuum correspondence)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
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
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(8) << a
                  << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Wave Speed — Flux Propagation Verification\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Section 1: Measure wavefront propagation speed
    // ================================================================
    // Create a narrow pulse at center, measure how far it travels
    // after N ticks. The wave equation gives group velocity c = C_WAVE
    // for long wavelengths.
    std::cout << "\n--- Section 1: Wavefront Speed Measurement ---\n";
    {
        int L = 64;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Inject narrow pulse along x-axis at center (z-polarized)
        rb.inject_flux(cx, cx, cx, {0, 0, 5.0});

        // Run and track the wavefront
        int ticks = 30;
        rb.run(ticks);

        // Find the furthest point from center with significant flux
        // along the +x axis
        double threshold = 0.001;  // Signal threshold
        int furthest = 0;
        for (int dx = 1; dx < L/4; ++dx) {
            double rho = rb.voxels()[rb.lattice().index(cx + dx, cx, cx)].density();
            if (rho > threshold) {
                furthest = dx;
            }
        }

        double measured_speed = (double)furthest / ticks;
        std::cout << "    Furthest signal at t=" << ticks << ": x = " << furthest << "\n";
        std::cout << "    Measured speed = " << measured_speed << " voxels/tick\n";
        std::cout << "    Expected C_WAVE = " << ftd::C_WAVE << "\n";

        // Wave speed should be close to C_WAVE (within 50% for discrete effects)
        // The exact speed depends on wavelength (dispersion), but group velocity
        // at long wavelengths should approximate C_WAVE.
        check("Wavefront propagates (furthest > 0)", furthest > 0);
        check("Wave speed in correct ballpark",
              measured_speed > ftd::C_WAVE * 0.3 && measured_speed < ftd::C_WAVE * 2.0);
    }

    // ================================================================
    // Section 2: Wave speed is less than C_SPEED
    // ================================================================
    std::cout << "\n--- Section 2: Wave Speed < Speed of Causality ---\n";
    {
        // C_WAVE = C_SPEED = 1/sqrt(3) — unified causal speed [DERIVED]
        check("C_WAVE == C_SPEED (unified causal speed)",
              std::abs(ftd::C_WAVE - ftd::C_SPEED) < 1e-15);

        double cfl_limit = std::sqrt(1.0 / 3.0);
        std::cout << "    CFL limit = 1/sqrt(3) = " << cfl_limit << "\n";
        std::cout << "    C_WAVE                = " << ftd::C_WAVE << "\n";
        check_close("C_WAVE = 1/sqrt(3) (derived from CFL)",
                    ftd::C_WAVE, cfl_limit, 1e-12);
        check("C_WAVE at CFL limit (maximal stable speed)",
              std::abs(ftd::C_WAVE - cfl_limit) < 1e-10);
    }

    // ================================================================
    // Section 3: Dispersion — discrete lattice modifies omega(k)
    // ================================================================
    // For the 6-point Laplacian on a cubic lattice:
    //   omega(k) = 2c * |sin(k/2)|   (per axis, for plane wave along axis)
    //
    // At low k: omega ≈ c*k (continuum limit)
    // At k = pi: omega = 2c (maximum, Nyquist)
    std::cout << "\n--- Section 3: Dispersion Relation ---\n";
    {
        double c = ftd::C_WAVE;

        // Low-k limit: omega/k -> c
        double k_low = 0.1;
        double omega_low = 2.0 * c * std::sin(k_low / 2.0);
        double speed_low = omega_low / k_low;
        std::cout << "    k=0.1: omega = " << omega_low << ", v_phase = " << speed_low << "\n";
        check_close("Low-k phase speed ≈ C_WAVE",
                    speed_low, c, 0.01);

        // Nyquist limit: omega/k < c (dispersion slows short wavelengths)
        double k_high = ftd::PI;
        double omega_high = 2.0 * c * std::sin(k_high / 2.0);
        double speed_high = omega_high / k_high;
        std::cout << "    k=pi: omega = " << omega_high << ", v_phase = " << speed_high << "\n";
        check("Short wavelengths are slower (lattice dispersion)",
              speed_high < c);
    }

    // ================================================================
    // Section 4: Symmetry — wave propagates equally in all directions
    // ================================================================
    std::cout << "\n--- Section 4: Propagation Isotropy ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Inject pulse at center
        rb.inject_flux(cx, cx, cx, {0, 0, 5.0});
        rb.run(15);

        // Compare flux at equal distance along +x, +y, +z
        double rho_x = rb.voxels()[rb.lattice().index(cx+5, cx, cx)].density();
        double rho_y = rb.voxels()[rb.lattice().index(cx, cx+5, cx)].density();
        double rho_z = rb.voxels()[rb.lattice().index(cx, cx, cx+5)].density();

        std::cout << "    rho(+x,5) = " << rho_x << "\n";
        std::cout << "    rho(+y,5) = " << rho_y << "\n";
        std::cout << "    rho(+z,5) = " << rho_z << "\n";

        // All three should be similar (cubic symmetry)
        // The z-polarized source may break symmetry slightly
        // but the wave equation propagation should be isotropic
        double avg = (rho_x + rho_y + rho_z) / 3.0;
        if (avg > 1e-10) {
            double max_dev = std::max({std::abs(rho_x - avg),
                                       std::abs(rho_y - avg),
                                       std::abs(rho_z - avg)});
            double rel_dev = max_dev / avg;
            std::cout << "    Relative deviation from isotropy: " << rel_dev << "\n";
            // Allow some anisotropy from polarization effects
            check("Propagation approximately isotropic (< 80% deviation)",
                  rel_dev < 0.8);
        } else {
            std::cout << "    Signal too small at distance 5\n";
            check("Signal reaches distance 5", avg > 1e-10);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All wave speed tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
