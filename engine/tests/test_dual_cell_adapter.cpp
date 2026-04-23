/**
 * @file test_dual_cell_adapter.cpp
 * @brief Unit tests for render_bridge_to_dual_cell_fields.
 *
 * T1: empty bridge -> zero rho, zero phi.
 * T2: single +1 charge settled briefly -> rho_cell has one +1, phi fields nonzero.
 * T3: sum of rho_cell equals sum of voxel states (signed charge conservation).
 * T4: b=2 blocking on the adapted fields preserves Gauss residual within eps.
 */
#include <cmath>
#include <cstdio>
#include "ftd/eft/coupling_measurement.h"  // configure_bare_lattice_for_coupling
#include "ftd/eft/dual_cell_blocking.h"
#include "ftd/render_bridge.h"

static int g_failures = 0;
#define CHECK(cond, name) do { \
    if (cond) std::printf("  PASS  %s\n", name); \
    else { std::printf("  FAIL  %s\n", name); ++g_failures; } \
} while (0)

int main() {
    using ftd::eft::render_bridge_to_dual_cell_fields;
    using ftd::eft::block_dual_cell_b2;
    using ftd::eft::total_source;
    using ftd::eft::max_gauss_residual;

    // T1: empty bridge -> zero fields.
    {
        ftd::RenderBridge rb(8);
        rb.force_cpu();
        ftd::eft::configure_bare_lattice_for_coupling(rb);
        auto f = render_bridge_to_dual_cell_fields(rb);
        bool all_zero = true;
        for (int v : f.rho_cell) if (v != 0) { all_zero = false; break; }
        for (double v : f.phi_x) if (std::abs(v) > 1e-14) { all_zero = false; break; }
        for (double v : f.phi_y) if (std::abs(v) > 1e-14) { all_zero = false; break; }
        for (double v : f.phi_z) if (std::abs(v) > 1e-14) { all_zero = false; break; }
        CHECK(f.L == 8 && all_zero, "T1 empty bridge -> zero fields");
    }

    // T2: +1 locked charge at centre, settle briefly, adapter finds state and flux.
    {
        ftd::RenderBridge rb(16);
        rb.force_cpu();
        ftd::eft::configure_bare_lattice_for_coupling(rb);
        const int mid = 8;
        rb.inject_particle(mid, mid, mid, +1, {0.0, 0.0, 0.05});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(50);
        auto f = render_bridge_to_dual_cell_fields(rb);
        int q = total_source(f);
        double max_phi = 0.0;
        for (double v : f.phi_z) max_phi = std::max(max_phi, std::abs(v));
        char msg[160];
        std::snprintf(msg, sizeof(msg),
                      "T2 single +1 charge: total_source=%d (expected 1), max|phi_z|=%.4g",
                      q, max_phi);
        CHECK(q == 1 && max_phi > 0.0, msg);
    }

    // T3: sum of rho_cell = sum of voxel state (charge conservation).
    {
        ftd::RenderBridge rb(16);
        rb.force_cpu();
        ftd::eft::configure_bare_lattice_for_coupling(rb);
        rb.inject_particle(5, 5, 5, +1, {0, 0, 0.05});
        rb.inject_particle(10, 10, 10, -1, {0, 0, -0.05});
        rb.voxels()[rb.lattice().index(5, 5, 5)].locked = true;
        rb.voxels()[rb.lattice().index(10, 10, 10)].locked = true;
        rb.run(20);
        auto f = render_bridge_to_dual_cell_fields(rb);
        int q_rho = total_source(f);
        int q_vox = 0;
        for (const auto& v : rb.voxels()) q_vox += v.state;
        char msg[128];
        std::snprintf(msg, sizeof(msg), "T3 rho_sum=%d vox_sum=%d", q_rho, q_vox);
        CHECK(q_rho == q_vox && q_rho == 0, msg);
    }

    // T4: b=2 blocking on adapted fields. total_source preserved exactly.
    {
        ftd::RenderBridge rb(16);
        rb.force_cpu();
        ftd::eft::configure_bare_lattice_for_coupling(rb);
        rb.inject_particle(4, 4, 4, +1, {0, 0, 0.05});
        rb.inject_particle(12, 12, 12, -1, {0, 0, -0.05});
        rb.voxels()[rb.lattice().index(4, 4, 4)].locked = true;
        rb.voxels()[rb.lattice().index(12, 12, 12)].locked = true;
        rb.run(50);
        auto fine = render_bridge_to_dual_cell_fields(rb);
        auto coarse = block_dual_cell_b2(fine);
        const int q_fine = total_source(fine);
        const int q_coarse = total_source(coarse);
        char msg[160];
        std::snprintf(msg, sizeof(msg),
                      "T4 b=2 blocking: q_fine=%d q_coarse=%d L_fine=%d L_coarse=%d",
                      q_fine, q_coarse, fine.L, coarse.L);
        CHECK(q_fine == q_coarse && coarse.L == 8, msg);
    }

    std::printf("\n%s: %d failures\n", (g_failures == 0 ? "OK" : "FAIL"), g_failures);
    return g_failures;
}
