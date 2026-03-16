/**
 * Test: Gauss Constraint Convergence (Phase B — FDTD Bridge)
 *
 * Verifies that the SOR Gauss projection converges properly
 * and enforces div(J) = s at void sites.
 *
 * The coupling source continuously injects divergence at particle sites,
 * so the Gauss violation reaches a steady state (not zero). What matters:
 * - The RMS stays bounded and small
 * - Void sites are well-corrected
 * - The constraint is stable over long runs
 *
 * 4 checks:
 *
 * GC1: Gauss violation RMS is bounded and stable over time
 * GC2: Gauss violation RMS < 0.05 after 500 ticks (quality)
 * GC3: Max Gauss error at void sites < 0.5 after 500 ticks
 * GC4: Violation does not grow unboundedly (stable at t=1000 vs t=500)
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

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Gauss Constraint Convergence (Phase B) — 4 Checks\n";
    std::cout << "================================================================\n";

    // GC1: Gauss violation RMS stays bounded (< 0.02) from early on
    // The coupling source continuously injects divergence at particle sites,
    // and Gauss projection corrects at void sites. The steady-state RMS
    // should be small and roughly constant.
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(50);
        auto a50 = rb.energy_audit();
        double rms50 = std::sqrt(a50.gauss_violation / rb.lattice().total_sites());

        std::cout << std::setprecision(8) << std::scientific;
        std::cout << "    RMS violation at t=50: " << rms50 << "\n";
        check("GC1: Gauss RMS bounded < 0.02 at t=50", rms50 < 0.02);
    }

    // GC2: Gauss violation RMS < 0.05 after 500 ticks
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(500);
        auto a = rb.energy_audit();
        double rms = std::sqrt(a.gauss_violation / rb.lattice().total_sites());

        std::cout << "    RMS violation at t=500: " << rms << "\n";
        check("GC2: Gauss RMS < 0.05 after 500 ticks", rms < 0.05);
    }

    // GC3: Max Gauss error at void sites < 0.5 after 500 ticks
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(500);

        double max_void_err = 0.0;
        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            if (rb.voxels()[i].state != 0) continue;
            double div = rb.divergence_flux(i);
            double err = std::abs(div);
            if (err > max_void_err) max_void_err = err;
        }

        std::cout << "    Max void-site Gauss error: " << max_void_err << "\n";
        check("GC3: Max void-site Gauss error < 0.5", max_void_err < 0.5);
    }

    // GC4: Violation is stable — does not grow unboundedly over long runs
    // The steady-state RMS at t=1000 should not exceed t=500 by more than 10%.
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(500);
        auto a500 = rb.energy_audit();
        double rms500 = std::sqrt(a500.gauss_violation / rb.lattice().total_sites());

        rb.run(500);  // total: 1000 ticks
        auto a1000 = rb.energy_audit();
        double rms1000 = std::sqrt(a1000.gauss_violation / rb.lattice().total_sites());

        std::cout << "    RMS at t=500:  " << rms500 << "\n";
        std::cout << "    RMS at t=1000: " << rms1000 << "\n";
        double growth = (rms500 > 1e-15) ? (rms1000 - rms500) / rms500 : 0.0;
        std::cout << "    Growth: " << std::setprecision(2) << std::fixed
                  << growth * 100 << "%\n";
        check("GC4: Violation stable (growth < 10% from t=500 to t=1000)",
              growth < 0.10);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All 4 Gauss convergence tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
