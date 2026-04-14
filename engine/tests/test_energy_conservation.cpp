/**
 * Test: Energy Conservation (consolidated suite)
 *
 * Merges 3 legacy tests into a single ftd::test-instrumented suite using
 * the Phase 2a NDJSON telemetry API:
 *
 *   test_energy              -> section "energy_basic"       (4 subtests, 8 checks)
 *   test_energy_conservation -> section "phase4_conservation" (4 groups, 12 checks)
 *   test_energy_tracking     -> section "phase4_tracking"     (5 subtests, 5 checks)
 *
 * Every check(...) from the legacy files is preserved verbatim (same
 * condition, same label) and routed through ftd::test::check for
 * uniform telemetry.
 *
 * Wave 4a.3 consolidation (2026-04-14). Self-ref target: this file
 * replaces the old test_energy_conservation.cpp source. All 3 input
 * families had some failing checks in Wave 1 Pass 1 (energy, energy_
 * conservation, energy_tracking each Failed). This commit preserves
 * structural parity — same checks, same tolerances.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: energy_basic  (from test_energy.cpp)
// ============================================================================

static void section_energy_basic() {
    // ---- Test 1: Vacuum stability ----
    std::cout << "--- Vacuum stability ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.run(100);
        auto ld = ftd::compute_lagrangian_diagnostics(rb);
        ftd::test::check("Vacuum: no manifested particles", ld.manifested_count == 0);
        ftd::test::check_close("Vacuum: total flux = 0", ld.total_flux_mag, 0.0, 1e-15);
        ftd::test::check_close("Vacuum: total wave energy = 0", ld.total_wave_energy, 0.0, 1e-15);
    }

    // ---- Test 2: Single locked particle energy stability ----
    std::cout << "\n--- Single locked particle ---\n";
    {
        ftd::RenderBridge rb(16);
        int cx = 8, cy = 8, cz = 8;
        rb.inject_particle(cx, cy, cz, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cy, cz)].locked = true;
        rb.run(100);

        std::vector<double> H_values;
        for (int t = 0; t < 200; ++t) {
            auto ld = ftd::compute_lagrangian_diagnostics(rb);
            H_values.push_back(ld.total_hamiltonian);
            rb.tick();
        }

        double H_max = H_values[0], H_min = H_values[0];
        for (double h : H_values) {
            H_max = std::max(H_max, h);
            H_min = std::min(H_min, h);
        }
        double H_range = H_max - H_min;
        double H_avg = 0;
        for (double h : H_values) H_avg += h;
        H_avg /= H_values.size();

        std::cout << "    H_avg = " << H_avg << ", range = " << H_range << "\n";
        ftd::test::check("Locked particle: H variation < 50% of avg",
              H_avg > 0.0 ? H_range / H_avg < 0.5 : H_range < 1.0);
        ftd::test::check("Locked particle: still manifested", rb.voxels()[rb.lattice().index(cx,cy,cz)].state != 0);
    }

    // ---- Test 3: Two-particle system energy tracking ----
    std::cout << "\n--- Two-particle energy tracking ---\n";
    {
        ftd::RenderBridge rb(32);
        rb.inject_particle(11, 16, 16, +1, {0, 0, ftd::K_B});
        rb.inject_particle(21, 16, 16, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(11, 16, 16)].locked = true;
        rb.voxels()[rb.lattice().index(21, 16, 16)].locked = true;

        rb.run(200);
        auto ld_start = ftd::compute_lagrangian_diagnostics(rb);
        double H_start = ld_start.total_hamiltonian;

        rb.run(300);
        auto ld_end = ftd::compute_lagrangian_diagnostics(rb);
        double H_end = ld_end.total_hamiltonian;

        std::cout << "    H_start = " << H_start << "\n";
        std::cout << "    H_end   = " << H_end << "\n";

        ftd::test::check("Two-particle: H_end <= H_start (damping removes energy)", H_end <= H_start * 1.01);
        ftd::test::check("Two-particle: still 2 particles", ld_end.manifested_count == 2);
    }

    // ---- Test 4: Wave pulse dissipates with damping ----
    std::cout << "\n--- Wave pulse dissipation ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.inject_flux(8, 8, 8, {0, 0, 0.5});

        rb.run(50);
        auto ld50 = ftd::compute_lagrangian_diagnostics(rb);
        double E50 = ld50.total_wave_energy;

        rb.run(500);
        auto ld550 = ftd::compute_lagrangian_diagnostics(rb);
        double E550 = ld550.total_wave_energy;

        std::cout << "    E(t=50)  = " << E50 << "\n";
        std::cout << "    E(t=550) = " << E550 << "\n";
        ftd::test::check("Wave energy: E(550) < E(50) (damping dissipates)", E550 < E50);
        ftd::test::check("Wave energy: E(550) >= 0", E550 >= 0.0);
        ftd::test::check("No spontaneous particles", ld550.manifested_count == 0);
    }
}

// ============================================================================
// Section: phase4_conservation  (from test_energy_conservation.cpp)
// ============================================================================

static void section_phase4_conservation() {
    // Group 1: Energy Conservation (steady state)
    std::cout << "\n--- Group 1: Energy Conservation (steady state) ---\n";

    // EC1
    {
        ftd::RenderBridge rb(32);
        rb.toggles.selective_damping = false;
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(500);
        auto a0 = rb.energy_audit();
        double e0 = a0.total_energy;
        rb.run(500);
        auto a1 = rb.energy_audit();
        double e1 = a1.total_energy;
        double pct = (e0 > 1e-30) ? 100.0 * std::abs(e1 - e0) / e0 : 0.0;

        std::cout << std::setprecision(6) << std::fixed;
        std::cout << "    E(500)=" << e0 << " E(1000)=" << e1
                  << " change=" << std::setprecision(2) << pct << "%\n";
        ftd::test::check("EC1: Opposite locked pair steady state: |dE/E| < 1%", pct < 1.0);
    }

    // EC2
    {
        ftd::RenderBridge rb(32);
        rb.toggles.selective_damping = false;
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(500);
        auto a0 = rb.energy_audit();
        double e0 = a0.total_energy;
        rb.run(500);
        auto a1 = rb.energy_audit();
        double e1 = a1.total_energy;
        double pct = (e0 > 1e-30) ? 100.0 * std::abs(e1 - e0) / e0 : 0.0;

        std::cout << "    E(500)=" << e0 << " E(1000)=" << e1
                  << " change=" << std::setprecision(2) << pct << "%\n";
        ftd::test::check("EC2: Same-sign locked pair steady state: |dE/E| < 1%", pct < 1.0);
    }

    // EC3
    {
        ftd::RenderBridge rb(32);
        rb.toggles.selective_damping = false;
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(500);
        auto a0 = rb.energy_audit();
        double e0 = a0.total_energy;
        rb.run(500);
        auto a1 = rb.energy_audit();
        double e1 = a1.total_energy;
        double pct = (e0 > 1e-30) ? 100.0 * std::abs(e1 - e0) / e0 : 0.0;

        std::cout << "    E(500)=" << e0 << " E(1000)=" << e1
                  << " change=" << std::setprecision(2) << pct << "%\n";
        ftd::test::check("EC3: Single locked particle steady state: |dE/E| < 1%", pct < 1.0);
    }

    // EC4: Zero self-field injection for 1000 ticks
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        bool all_zero = true;
        for (int t = 0; t < 1000; ++t) {
            rb.tick();
            auto a = rb.energy_audit();
            if (std::abs(a.self_field_injection) > 1e-15) {
                all_zero = false;
                std::cout << "    Tick " << t << ": injection = "
                          << std::scientific << a.self_field_injection << "\n";
                break;
            }
        }
        ftd::test::check("EC4: Self-field injection == 0 for all 1000 ticks", all_zero);
    }

    // Group 2: Particle Stability
    std::cout << "\n--- Group 2: Particle Stability ---\n";

    // EC5
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(1000);
        int count = 0;
        for (int i = 0; i < rb.lattice().total_sites(); ++i)
            if (rb.voxels()[i].state != 0) ++count;

        std::cout << "    Particles at t=1000: " << count << "\n";
        ftd::test::check("EC5: Both locked particles survive 1000 ticks", count == 2);
    }

    // EC6
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});

        int alive_count = 0;
        for (int t = 0; t < 500; ++t) {
            rb.tick();
            bool found = false;
            for (int i = 0; i < rb.lattice().total_sites(); ++i) {
                if (rb.voxels()[i].state != 0) { found = true; break; }
            }
            if (found) ++alive_count;
        }

        std::cout << "    Alive for " << alive_count << " / 500 ticks\n";
        ftd::test::check("EC6: Free particle survives > 400 ticks", alive_count > 400);
    }

    // EC7
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(500);

        std::vector<double> energies;
        for (int t = 0; t < 100; ++t) {
            rb.tick();
            energies.push_back(rb.energy_audit().total_energy);
        }

        double mean = 0;
        for (double e : energies) mean += e;
        mean /= energies.size();

        double var = 0;
        for (double e : energies) var += (e - mean) * (e - mean);
        var /= energies.size();
        double cv = (mean > 1e-15) ? std::sqrt(var) / mean : 0.0;

        std::cout << std::setprecision(6) << std::fixed;
        std::cout << "    Mean energy (t=500-600): " << mean
                  << " CV: " << std::setprecision(4) << cv * 100 << "%\n";
        ftd::test::check("EC7: Energy steady state (CV < 5%)", cv < 0.05);
    }

    // Group 3: Force Accuracy
    std::cout << "\n--- Group 3: Force Accuracy ---\n";

    // EC8
    {
        int L = 32;
        int mid = 16;

        ftd::RenderBridge rb_single(L);
        rb_single.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb_single.voxels()[rb_single.lattice().index(mid, mid, mid)].locked = true;
        rb_single.run(200);
        double pe_self = rb_single.energy_audit().coulomb_pe;

        auto measure_pair_pe = [&](int sep) -> double {
            ftd::RenderBridge rb(L);
            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + sep, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid + sep, mid, mid)].locked = true;
            rb.run(200);
            return rb.energy_audit().coulomb_pe;
        };

        double pe4_total = measure_pair_pe(4);
        double pe8_total = measure_pair_pe(8);

        double pe4_interact = pe4_total - 2.0 * pe_self;
        double pe8_interact = pe8_total - 2.0 * pe_self;

        std::cout << std::setprecision(6) << std::scientific;
        std::cout << "    Self-energy (single +1): " << pe_self << "\n";
        std::cout << "    Total PE(r=4)=" << pe4_total
                  << " Total PE(r=8)=" << pe8_total << "\n";
        std::cout << "    Interaction PE(r=4)=" << pe4_interact
                  << " Interaction PE(r=8)=" << pe8_interact << "\n";

        if (std::abs(pe8_interact) > 1e-15) {
            double ratio = pe4_interact / pe8_interact;
            std::cout << "    Interaction PE ratio: " << std::setprecision(2)
                      << std::fixed << ratio << "\n";
            ftd::test::check("EC8: Closer pair has stronger interaction PE (ratio > 1.0)",
                  ratio > 1.0);
        } else {
            ftd::test::check("EC8: Interaction PE detectable",
                  std::abs(pe4_interact) > std::abs(pe8_interact));
        }
    }

    // EC9
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;
        rb.run(50);

        auto fd = rb.force_diag();
        int idx = rb.lattice().index(mid - 3, mid, mid);
        double fx = fd[idx].f_coulomb.x;
        std::cout << "    F_x on +1 particle: " << std::scientific << fx << "\n";
        ftd::test::check("EC9: Opposite charges attract (F_x > 0)", fx > 0);
    }

    // EC10
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;
        rb.run(50);

        auto fd = rb.force_diag();
        int idx = rb.lattice().index(mid - 3, mid, mid);
        double fx = fd[idx].f_coulomb.x;
        std::cout << "    F_x on left +1 particle: " << std::scientific << fx << "\n";
        ftd::test::check("EC10: Same charges repel (F_x < 0)", fx < 0);
    }

    // Group 4: Gauss Constraint
    std::cout << "\n--- Group 4: Gauss Constraint ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(1000);
        auto a = rb.energy_audit();
        double rms = std::sqrt(a.gauss_violation / rb.lattice().total_sites());

        std::cout << std::setprecision(6) << std::fixed;
        std::cout << "    Max Gauss error: " << a.max_gauss_error
                  << " RMS: " << rms << "\n";
        ftd::test::check("EC11: max |div(J) - s| < 1.5 (particle-site coupling)", a.max_gauss_error < 1.5);
        ftd::test::check("EC12: Gauss RMS < 0.1", rms < 0.1);
    }
}

// ============================================================================
// Section: phase4_tracking  (from test_energy_tracking.cpp)
// ============================================================================

static void section_phase4_tracking() {
    // ET1: Self-field injection is exactly zero
    std::cout << "\n--- ET1: Self-field injection == 0 ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.inject_particle(mid + 6, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + 6, mid, mid)].locked = true;

        bool all_zero = true;
        for (int t = 0; t < 100; ++t) {
            rb.tick();
            auto a = rb.energy_audit();
            if (std::abs(a.self_field_injection) > 1e-15) {
                all_zero = false;
                std::cout << "    Tick " << t << ": injection = "
                          << a.self_field_injection << " (NON-ZERO)\n";
                break;
            }
        }
        ftd::test::check("ET1: Self-field injection == 0 for all 100 ticks", all_zero);
    }

    // ET2: Charge conservation exact
    std::cout << "\n--- ET2: Charge conservation ---\n";
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        auto a0 = rb.energy_audit();
        int q0 = a0.charge_total;

        rb.run(500);

        auto a1 = rb.energy_audit();
        int q1 = a1.charge_total;
        std::cout << "    Q(t=0) = " << q0 << ", Q(t=500) = " << q1 << "\n";
        ftd::test::check("ET2: Charge conserved (net charge unchanged)", q0 == q1);
    }

    // ET3: Energy conservation (Phase 4)
    std::cout << "\n--- ET3: Energy conservation (Phase 4, steady state) ---\n";
    {
        ftd::RenderBridge rb(32);
        rb.toggles.selective_damping = false;
        int mid = 16;
        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;

        rb.run(500);
        auto a0 = rb.energy_audit();
        double e0 = a0.total_energy;

        rb.run(500);

        auto a1 = rb.energy_audit();
        double e1 = a1.total_energy;
        double pct_change = (e0 > 1e-30) ? 100.0 * std::abs(e1 - e0) / e0 : 0.0;
        std::cout << "    E(t=500) = " << e0 << ", E(t=1000) = " << e1
                  << ", change = " << std::setprecision(2) << std::fixed
                  << pct_change << "%\n";
        std::cout << "    Self-field injection (last tick): "
                  << std::scientific << a1.self_field_injection << "\n";
        ftd::test::check("ET3: Steady-state energy drift < 1% over 500 ticks (Phase 4)",
              pct_change < 1.0);
    }

    // ET4: Coulomb PE vs separation
    std::cout << "\n--- ET4: Coulomb PE vs separation ---\n";
    {
        auto measure_pe = [](int separation) -> double {
            ftd::RenderBridge rb(32);
            int mid = 16;
            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + separation, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid + separation, mid, mid)].locked = true;
            rb.run(200);
            auto a = rb.energy_audit();
            return a.coulomb_pe;
        };

        double pe_close = measure_pe(4);
        double pe_far = measure_pe(8);
        std::cout << std::setprecision(6) << std::scientific;
        std::cout << "    Coulomb PE at r=4: " << pe_close << "\n";
        std::cout << "    Coulomb PE at r=8: " << pe_far << "\n";
        ftd::test::check("ET4: Coulomb PE more negative at r=4 than r=8", pe_close < pe_far);
    }

    // ET5: Forces-off self-field injection
    std::cout << "\n--- ET5: Forces-off self-field injection ---\n";
    {
        ftd::RenderBridge rb(16);
        rb.toggles.forces = false;
        rb.toggles.movement = false;
        int mid = 8;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(50);

        double total_injection = 0.0;
        for (int t = 0; t < 50; ++t) {
            rb.tick();
            auto a = rb.energy_audit();
            total_injection += a.self_field_injection;
        }
        std::cout << std::setprecision(6) << std::scientific;
        std::cout << "    Total self-field injection over 50 ticks: " << total_injection << "\n";
        ftd::test::check("ET5: Self-field injection == 0 (floor removed)",
              std::abs(total_injection) < 1e-12);
    }
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("test_energy_conservation");

    ftd::test::section("energy_basic");
    section_energy_basic();

    ftd::test::section("phase4_conservation");
    section_phase4_conservation();

    ftd::test::section("phase4_tracking");
    section_phase4_tracking();

    return ftd::test::finalize();
}
