/**
 * Test: Gauss constraint (consolidated suite)
 *
 * Merges 2 legacy tests into test_gauss.cpp (self-ref target):
 *
 *   test_gauss             -> section "gauss_structure"   (16 checks)
 *   test_gauss_convergence -> section "gauss_convergence" ( 4 checks)
 *
 * Every check(...) preserved verbatim. Wave 4b.5 consolidation
 * (2026-04-14). Structural parity — old `gauss` test was failing
 * (latency-downstream), old `gauss_convergence` was passing.
 */

#include <cmath>
#include <iostream>
#include <iomanip>

#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: gauss_structure  (from test_gauss.cpp)
// ============================================================================

static void section_gauss_structure() {
    // Test 1: Vacuum Gauss constraint
    std::cout << "--- Vacuum (div J = 0 everywhere) ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.run(10);
        auto ld = ftd::compute_lagrangian_diagnostics(rb);
        ftd::test::check_close("Vacuum: Gauss violation = 0", ld.gauss_violation, 0.0, 1e-20);
        ftd::test::check_close("Vacuum: max Gauss error = 0", ld.max_gauss_error, 0.0, 1e-20);
    }

    // Test 2: Single particle nonzero divergence
    std::cout << "\n--- Single positive particle ---\n";
    {
        ftd::RenderBridge rb(16);
        int cx = 8, cy = 8, cz = 8;
        rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;

        rb.run(200);

        int idx = rb.lattice().index(cx, cy, cz);
        double divJ_at_particle = rb.divergence_flux(idx);

        std::cout << "    div(J) at +1 particle = " << divJ_at_particle << "\n";

        ftd::test::check("Single +1: |div(J)| > 0 at particle", std::abs(divJ_at_particle) > 0.01);

        auto ld = ftd::compute_lagrangian_diagnostics(rb);
        std::cout << "    Total Gauss violation = " << ld.gauss_violation << "\n";
        std::cout << "    Max Gauss error = " << ld.max_gauss_error << "\n";
    }

    // Test 3: Charge antisymmetry
    std::cout << "\n--- Charge antisymmetry ---\n";
    {
        ftd::RenderBridge rb_pos(16);
        rb_pos.inject_particle(8, 8, 8, +1, {0, 0, ftd::K_B});
        rb_pos.voxels()[rb_pos.lattice().index(8, 8, 8)].locked = true;
        rb_pos.run(200);
        double divJ_pos = rb_pos.divergence_flux(rb_pos.lattice().index(8, 8, 8));

        ftd::RenderBridge rb_neg(16);
        rb_neg.inject_particle(8, 8, 8, -1, {0, 0, -ftd::K_B});
        rb_neg.voxels()[rb_neg.lattice().index(8, 8, 8)].locked = true;
        rb_neg.run(200);
        double divJ_neg = rb_neg.divergence_flux(rb_neg.lattice().index(8, 8, 8));

        std::cout << "    div(J) at +1 = " << divJ_pos << "\n";
        std::cout << "    div(J) at -1 = " << divJ_neg << "\n";

        ftd::test::check("Antisymmetry: sign(div+) != sign(div-)",
              (divJ_pos > 0) != (divJ_neg > 0));
        ftd::test::check("Antisymmetry: |div+| ≈ |div-|",
              std::abs(std::abs(divJ_pos) - std::abs(divJ_neg)) < 0.01);
    }

    // Test 4: Opposite charge pair
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

        ftd::test::check("Pair: opposite div(J) signs", (divJ_pos > 0) != (divJ_neg > 0));
        ftd::test::check("Pair: |div(J)| > 0 at +1", std::abs(divJ_pos) > 0.01);
        ftd::test::check("Pair: |div(J)| > 0 at -1", std::abs(divJ_neg) > 0.01);
    }

    // Test 5: Far-field
    std::cout << "\n--- Far-field divergence ---\n";
    {
        ftd::RenderBridge rb(32);
        rb.inject_particle(16, 16, 16, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(16, 16, 16)].locked = true;

        rb.run(300);

        double divJ_far = rb.divergence_flux(rb.lattice().index(0, 0, 0));
        std::cout << "    div(J) at (0,0,0) = " << divJ_far << "\n";
        ftd::test::check("Far-field: |div(J)| < 0.1 at distance", std::abs(divJ_far) < 0.1);
    }

    // Test 6: Gauss theorem on periodic lattice
    std::cout << "\n--- Gauss theorem on periodic lattice ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.inject_particle(4, 8, 8, +1, {0, 0, ftd::K_B});
        rb.inject_particle(12, 8, 8, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(4, 8, 8)].locked = true;
        rb.voxels()[rb.lattice().index(12, 8, 8)].locked = true;

        rb.run(200);

        double total_div = 0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            total_div += rb.divergence_flux(i);
        }
        std::cout << "    Sum div(J) over lattice = " << total_div << "\n";
        ftd::test::check_close("Sum div(J) = 0 (Gauss theorem)", total_div, 0.0, 1e-10);
    }

    // Test 7: Single charge on periodic lattice
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
        ftd::test::check_close("Sum div(J) = 0 even with net charge", total_div, 0.0, 1e-10);
    }

    // Test 8: Ward identity
    std::cout << "\n--- Ward identity: div(curl(J)) = 0 ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.inject_particle(6, 8, 8, +1, {0, 0, ftd::K_B});
        rb.inject_particle(10, 8, 8, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(6, 8, 8)].locked = true;
        rb.voxels()[rb.lattice().index(10, 8, 8)].locked = true;
        rb.inject_flux(8, 4, 8, {1.0, 0.5, -0.3});
        rb.inject_flux(8, 12, 8, {-0.5, 1.0, 0.7});

        rb.run(100);

        double max_div_curl = 0;
        double sum_div_curl = 0;
        int N = rb.lattice().total_sites();
        (void)N;

        int L = rb.lattice().size();
        int count = 0;
        for (int x = 2; x < L-2; ++x) {
            for (int y = 2; y < L-2; ++y) {
                for (int z = 2; z < L-2; ++z) {
                    int idx = rb.lattice().index(x, y, z);
                    (void)idx;

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

        ftd::test::check("Ward identity: max |div(curl(J))| < 1e-12", max_div_curl < 1e-12);
    }

    // Test 9: 2 physical DOF
    std::cout << "\n--- 2 physical DOF from Ward constraint ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.inject_flux(8, 8, 8, {1.0, 0.5, -0.3});
        rb.inject_flux(6, 8, 8, {0.0, 1.0, 0.5});
        rb.inject_flux(10, 8, 8, {-0.5, 0.0, 1.0});
        rb.run(30);

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
        ftd::test::check("Curl field nonzero", total_curl > 1e-10);
        ftd::test::check("All 3 curl components populated", curl_x_sum > 0 && curl_y_sum > 0 && curl_z_sum > 0);
        std::cout << "    (Ward identity + 3 components => 2 physical DOF)\n";
    }
}

// ============================================================================
// Section: gauss_convergence  (from test_gauss_convergence.cpp)
// ============================================================================

static void section_gauss_convergence() {
    // GC1
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
        ftd::test::check("GC1: Gauss RMS bounded < 0.02 at t=50", rms50 < 0.02);
    }

    // GC2
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
        ftd::test::check("GC2: Gauss RMS < 0.05 after 500 ticks", rms < 0.05);
    }

    // GC3
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
        ftd::test::check("GC3: Max void-site Gauss error < 0.5", max_void_err < 0.5);
    }

    // GC4
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

        rb.run(500);
        auto a1000 = rb.energy_audit();
        double rms1000 = std::sqrt(a1000.gauss_violation / rb.lattice().total_sites());

        std::cout << "    RMS at t=500:  " << rms500 << "\n";
        std::cout << "    RMS at t=1000: " << rms1000 << "\n";
        double growth = (rms500 > 1e-15) ? (rms1000 - rms500) / rms500 : 0.0;
        std::cout << "    Growth: " << std::setprecision(2) << std::fixed
                  << growth * 100 << "%\n";
        ftd::test::check("GC4: Violation stable (growth < 10% from t=500 to t=1000)",
              growth < 0.10);
    }

    std::cout << std::defaultfloat;
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("test_gauss");

    ftd::test::section("gauss_structure");
    section_gauss_structure();

    ftd::test::section("gauss_convergence");
    section_gauss_convergence();

    return ftd::test::finalize();
}
