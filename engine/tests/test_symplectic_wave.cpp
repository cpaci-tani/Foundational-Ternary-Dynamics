/**
 * @file test_symplectic_wave.cpp
 * @brief Symplectic Leapfrog wave propagation energy conservation test.
 */

#include "test_helpers.h"
#include "ftd/render_bridge.h"
#include <iostream>
#include <cmath>

int main() {
    using namespace ftd;
    using namespace ftd::test;

    Counter c;
    std::cout << "============================================================\n";
    std::cout << "  Scale 0: Symplectic Leapfrog Wave Energy Conservation Test\n";
    std::cout << "============================================================\n\n";

    // Helper to inject an exact transverse wave solution:
    // J_y = A * sin(2*pi*x/L)
    // wave_vel_y = -A * C_WAVE * (2*pi/L) * cos(2*pi*x/L)
    auto inject_transverse_wave = [](RenderBridge& rb, double A) {
        int L = rb.lattice().size();
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    int i = rb.lattice().index(x, y, z);
                    double phase = 2.0 * PI * x / L;
                    rb.voxels()[i].flux = {0.0, A * std::sin(phase), 0.0};
                    rb.voxels()[i].wave_vel = {0.0, -A * C_WAVE * (2.0 * PI / L) * std::cos(phase), 0.0};
                }
            }
        }
    };

    // ---- Option A: Classical Integration (dt=1.0) ----
    {
        RenderBridge rb_classic(16);
        prepare_bridge(rb_classic, /*force_cpu=*/true);
        rb_classic.toggles.wave_propagation = true;
        rb_classic.toggles.symplectic_leapfrog = false;
        rb_classic.toggles.damping = false;

        inject_transverse_wave(rb_classic, 1.0);
        auto a0 = rb_classic.energy_audit();
        double e0 = a0.E_field_energy + a0.B_field_energy;
        rb_classic.run(100);
        auto a1 = rb_classic.energy_audit();
        double e1 = a1.E_field_energy + a1.B_field_energy;
        double drift_classic = std::abs(e1 - e0) / (e0 > 0.0 ? e0 : 1.0);
        std::cout << "    Classical EM Wave Energy Drift (dt=1.0): " << drift_classic << "\n";
    }

    // ---- Option B: Symplectic Leapfrog (dt=0.05) ----
    {
        RenderBridge rb_sym(16);
        prepare_bridge(rb_sym, /*force_cpu=*/true);
        rb_sym.toggles.wave_propagation = true;
        rb_sym.toggles.symplectic_leapfrog = true;
        rb_sym.toggles.damping = false;
        rb_sym.set_dt(0.05);

        inject_transverse_wave(rb_sym, 1.0);
        auto a0 = rb_sym.energy_audit();
        double e0 = a0.E_field_energy + a0.B_field_energy;
        rb_sym.run(100);
        auto a1 = rb_sym.energy_audit();
        double e1 = a1.E_field_energy + a1.B_field_energy;
        double drift_sym = std::abs(e1 - e0) / (e0 > 0.0 ? e0 : 1.0);
        std::cout << "    Symplectic Leapfrog EM Wave Energy Drift (dt=0.05): " << drift_sym << "\n";

        // With the symplectic integrator and smaller dt, the drift is highly suppressed
        check("Symplectic Leapfrog wave energy drift < 0.005", drift_sym < 0.005, &c);
    }

    return report_and_exit_code(c, "Symplectic Wave conservation");
}
