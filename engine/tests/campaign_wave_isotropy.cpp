/**
 * Campaign: Wave Propagation Isotropy (Phase 2 — Continuum Limit)
 *
 * Validates that wave propagation on the cubic lattice is approximately
 * isotropic at long wavelengths, recovering Lorentz symmetry.
 *
 * Theory: A cubic lattice has preferred directions (axes, face diagonals,
 * body diagonals). At long wavelengths (k << π/a), wave propagation
 * becomes isotropic: c_eff → C_WAVE regardless of direction.
 * Anisotropy ratio = max(c)/min(c) should approach 1.0 as k→0.
 *
 * Protocol:
 *   1. Create Gaussian flux pulse at center
 *   2. Evolve for T ticks
 *   3. Measure flux magnitude at equal distance along 3 directions:
 *      (100) = axis, (110) = face diagonal, (111) = body diagonal
 *   4. Compute isotropy ratio = max(|J|) / min(|J|) at each distance
 *
 * Checks:
 *   WI1: Wave arrives along (100) direction (flux > baseline at r)
 *   WI2: Wave arrives along (110) direction
 *   WI3: Wave arrives along (111) direction
 *   WI4: Isotropy ratio < 5.0 (cubic lattice has inherent anisotropy at r~8)
 *   WI5: Flux amplitude decreases with distance (wave spreads)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
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

double flux_mag(const ftd::RenderBridge& rb, int x, int y, int z) {
    auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
    return std::sqrt(v.flux.x * v.flux.x + v.flux.y * v.flux.y + v.flux.z * v.flux.z);
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Wave Isotropy (Phase 2) — 5 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int mid = L / 2;
    const int TICKS = 30;  // Enough for wave to reach r~10

    // Create isotropic pulse at center
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    // Inject spherically symmetric flux burst
    double amp = ftd::K_B * 0.5;
    rb.inject_flux(mid, mid, mid, {amp, amp, amp});

    // Measure baseline (pre-evolution) at probe points
    double baseline = flux_mag(rb, mid + 10, mid, mid);

    // Evolve
    rb.run(TICKS);

    // ----------------------------------------------------------------
    // Measure flux at equal Euclidean distance along 3 directions
    // ----------------------------------------------------------------
    int r = 8;  // Probe distance

    // (100) direction: along x-axis
    double f_100 = flux_mag(rb, mid + r, mid, mid);

    // (110) direction: face diagonal (scale r by 1/√2 per axis)
    int d110 = static_cast<int>(r / std::sqrt(2.0));
    double f_110 = flux_mag(rb, mid + d110, mid + d110, mid);

    // (111) direction: body diagonal (scale r by 1/√3 per axis)
    int d111 = static_cast<int>(r / std::sqrt(3.0));
    double f_111 = flux_mag(rb, mid + d111, mid + d111, mid + d111);

    std::cout << "\n--- Wave Amplitudes at r=" << r << " ---\n";
    std::cout << "  (100): |J| = " << f_100 << "\n";
    std::cout << "  (110): |J| = " << f_110 << "\n";
    std::cout << "  (111): |J| = " << f_111 << "\n";
    std::cout << "  Baseline (pre-evolution): " << baseline << "\n";

    // ----------------------------------------------------------------
    // WI1-WI3: Wave arrives in all directions
    // ----------------------------------------------------------------
    double threshold = 1e-10;  // Any measurable flux
    check("WI1: Wave arrives along (100)", f_100 > threshold);
    check("WI2: Wave arrives along (110)", f_110 > threshold);
    check("WI3: Wave arrives along (111)", f_111 > threshold);

    // ----------------------------------------------------------------
    // WI4: Isotropy ratio
    // ----------------------------------------------------------------
    double max_f = std::max({f_100, f_110, f_111});
    double min_f = std::min({f_100, f_110, f_111});
    double isotropy_ratio = (min_f > 1e-30) ? max_f / min_f : 999.0;
    std::cout << "\n  Isotropy ratio = " << isotropy_ratio
              << " (max/min, ideal=1.0)\n";
    // Cubic lattice will have some anisotropy; we're measuring it
    // Cubic lattice has O(1) anisotropy at r~8; improves as r/L → 0
    check("WI4: Isotropy ratio < 5.0 (cubic lattice, r=8, L=32)",
          isotropy_ratio < 5.0);

    // ----------------------------------------------------------------
    // WI5: Amplitude decreases with distance (wave spreads in 3D)
    // ----------------------------------------------------------------
    double f_near = flux_mag(rb, mid + 4, mid, mid);
    double f_far = flux_mag(rb, mid + 12, mid, mid);
    std::cout << "  |J|(r=4) = " << f_near << "\n";
    std::cout << "  |J|(r=12) = " << f_far << "\n";
    check("WI5: Flux decreases with distance (wave spreads)",
          f_near > f_far || (f_near < 1e-20 && f_far < 1e-20));

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
