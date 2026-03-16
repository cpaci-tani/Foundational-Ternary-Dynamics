/**
 * Diagnostic: Particle Lifetime & Energy Loss
 *
 * Probes three issues:
 *   PL1: Slow particle survival (v=0.01, 0.02 — were evaporating)
 *   PL2: Energy loss rate vs velocity (quantify radiation)
 *   PL3: Orbital energy budget (where does the energy go?)
 *   PL4: Same-charge repulsion survival (FD4 regression)
 *   PL5: Portability flux transfer verification
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

struct PInfo {
    int x, y, z, idx;
    bool found = false;
};

PInfo find_particle(const ftd::RenderBridge& rb, int8_t sign) {
    PInfo p;
    for (int i = 0; i < rb.lattice().total_sites(); ++i) {
        if (rb.voxels()[i].state == sign) {
            auto c = rb.lattice().coord(i);
            p = {c.x, c.y, c.z, i, true};
            return p;
        }
    }
    return p;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  DIAGNOSTIC: Particle Lifetime & Energy Loss\n";
    std::cout << "================================================================\n";

    // ================================================================
    // PL1: Slow particles should survive now (portability fix)
    // ================================================================
    std::cout << "\n--- PL1: Slow particle survival ---\n";
    {
        double test_v[] = {0.005, 0.01, 0.02, 0.03, 0.05};
        for (double v0 : test_v) {
            const int L = 32;
            ftd::RenderBridge rb(L);
            int mid = L / 2;

            double iso = ftd::K_B / std::sqrt(3.0);
            rb.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.run(500);  // Settle

            int idx = rb.lattice().index(mid, mid, mid);
            rb.voxels()[idx].velocity = {0.0, 0.0, v0};
            rb.voxels()[idx].locked = false;

            int survived = 0;
            for (int t = 0; t < 500; ++t) {
                rb.tick();
                auto p = find_particle(rb, +1);
                if (!p.found) break;
                survived = t + 1;
            }

            double v_final = -1;
            auto p = find_particle(rb, +1);
            if (p.found) v_final = rb.voxels()[p.idx].speed();

            std::cout << "  v0=" << std::setw(5) << std::setprecision(3) << std::fixed
                      << v0 << "  survived=" << std::setw(4) << survived
                      << "/500  v_final=" << std::setw(8) << std::setprecision(5)
                      << v_final << "\n";
        }
        std::cout << std::setprecision(6) << std::defaultfloat;

        // Test: v=0.01 should survive (was evaporating before portability fix)
        {
            const int L = 32;
            ftd::RenderBridge rb(L);
            int mid = L / 2;
            double iso = ftd::K_B / std::sqrt(3.0);
            rb.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.run(500);
            int idx = rb.lattice().index(mid, mid, mid);
            rb.voxels()[idx].velocity = {0.0, 0.0, 0.01};
            rb.voxels()[idx].locked = false;
            for (int t = 0; t < 500; ++t) rb.tick();
            auto p = find_particle(rb, +1);
            check("PL1: v=0.01 particle survives 500 ticks", p.found);
        }
    }

    // ================================================================
    // PL2: Energy loss rate — track KE and field energy separately
    // ================================================================
    std::cout << "\n--- PL2: Energy loss rate vs velocity ---\n";
    {
        double test_v[] = {0.02, 0.05, 0.10};
        for (double v0 : test_v) {
            const int L = 32;
            ftd::RenderBridge rb(L);
            int mid = L / 2;

            double iso = ftd::K_B / std::sqrt(3.0);
            rb.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.run(500);

            auto ea0 = rb.energy_audit();
            double E_settled = ea0.total_energy;

            int idx = rb.lattice().index(mid, mid, mid);
            rb.voxels()[idx].velocity = {0.0, 0.0, v0};
            rb.voxels()[idx].locked = false;

            // Track energy every 100 ticks
            std::cout << "  v0=" << std::fixed << std::setprecision(3) << v0 << ":\n";
            std::cout << "    tick   KE         field      wave       total      density\n";
            for (int t = 0; t < 500; ++t) {
                rb.tick();
                if (t % 100 == 99) {
                    auto ea = rb.energy_audit();
                    auto p = find_particle(rb, +1);
                    double rho = p.found ? rb.voxels()[p.idx].density() : 0;
                    std::cout << "    " << std::setw(4) << (t + 1)
                              << "  " << std::scientific << std::setprecision(3)
                              << ea.particle_ke
                              << "  " << ea.field_energy
                              << "  " << ea.wave_energy
                              << "  " << ea.total_energy
                              << "  " << std::fixed << std::setprecision(4) << rho << "\n";
                }
            }
        }
        std::cout << std::setprecision(6) << std::defaultfloat;
    }

    // ================================================================
    // PL3: Orbital energy budget — where does energy go?
    // ================================================================
    std::cout << "\n--- PL3: Orbital energy budget ---\n";
    {
        const int L = 48;
        ftd::RenderBridge rb(L);
        int mid = L / 2;

        // Proton at center (locked)
        double iso = ftd::K_B / std::sqrt(3.0);
        rb.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Electron at r=8 with circular velocity
        int r0 = 8;
        double v_circ = std::sqrt(ftd::ALPHA / r0);
        rb.inject_particle(mid + r0, mid, mid, -1, {iso, iso, iso});
        rb.voxels()[rb.lattice().index(mid + r0, mid, mid)].locked = true;
        rb.run(500);  // Both settle

        // Unlock electron with tangential velocity
        int eidx = rb.lattice().index(mid + r0, mid, mid);
        rb.voxels()[eidx].velocity = {0.0, v_circ, 0.0};
        rb.voxels()[eidx].locked = false;

        auto ea0 = rb.energy_audit();
        double E0 = ea0.total_energy;

        std::cout << "  r0=" << r0 << ", v_circ=" << std::setprecision(4) << v_circ << "\n";
        std::cout << "  E_settled=" << std::setprecision(4) << E0 << "\n";
        std::cout << "  tick    r      speed    KE        field     total     dE/E0\n";

        int evap_tick = -1;
        for (int t = 0; t < 5000; ++t) {
            rb.tick();
            if (t % 500 == 499) {
                auto ea = rb.energy_audit();
                auto p = find_particle(rb, -1);
                if (!p.found) {
                    std::cout << "  " << std::setw(5) << (t + 1) << "  Electron evaporated!\n";
                    evap_tick = t + 1;
                    break;
                }
                double px = p.x, py = p.y, pz = p.z;
                double dist = std::sqrt((px - mid) * (px - mid) +
                                        (py - mid) * (py - mid) +
                                        (pz - mid) * (pz - mid));
                double spd = rb.voxels()[p.idx].speed();
                double dE = (ea.total_energy - E0) / E0 * 100;
                std::cout << "  " << std::setw(5) << (t + 1)
                          << std::fixed
                          << "  " << std::setw(6) << std::setprecision(1) << dist
                          << "  " << std::setw(7) << std::setprecision(4) << spd
                          << std::scientific << std::setprecision(2)
                          << "  " << std::setw(9) << ea.particle_ke
                          << "  " << std::setw(9) << ea.field_energy
                          << "  " << std::setw(9) << ea.total_energy
                          << std::fixed << std::setprecision(1)
                          << "  " << std::setw(6) << dE << "%\n";
            }
        }

        if (evap_tick > 0) {
            check("PL3: Electron survives > 2000 ticks in orbit", evap_tick > 2000);
        } else {
            check("PL3: Electron survives 5000 ticks in orbit", true);
        }
    }
    std::cout << std::setprecision(6) << std::defaultfloat;

    // ================================================================
    // PL4: Same-charge repulsion — both survive
    // ================================================================
    std::cout << "\n--- PL4: Same-charge repulsion survival ---\n";
    {
        const int L = 32;
        ftd::RenderBridge rb(L);
        int mid = L / 2;
        double iso = ftd::K_B / std::sqrt(3.0);

        // Two +1 particles at separation 6
        rb.inject_particle(mid - 3, mid, mid, +1, {iso, iso, iso});
        rb.inject_particle(mid + 3, mid, mid, +1, {iso, iso, iso});
        rb.voxels()[rb.lattice().index(mid - 3, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid + 3, mid, mid)].locked = true;
        rb.run(500);

        // Unlock both
        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            if (rb.voxels()[i].state == +1)
                rb.voxels()[i].locked = false;
        }

        int survived = 0;
        int count_alive = 0;
        for (int t = 0; t < 1000; ++t) {
            rb.tick();
            count_alive = 0;
            for (int i = 0; i < rb.lattice().total_sites(); ++i) {
                if (rb.voxels()[i].state == +1) count_alive++;
            }
            if (count_alive == 0) break;
            survived = t + 1;
        }
        std::cout << "  Survived " << survived << "/1000 ticks, " << count_alive << " alive\n";
        check("PL4: Both same-charge particles survive 1000 ticks", count_alive == 2);
    }

    // ================================================================
    // PL5: Portability flux verification — does moved particle retain density?
    // ================================================================
    std::cout << "\n--- PL5: Portability flux verification ---\n";
    {
        const int L = 32;
        ftd::RenderBridge rb(L);
        int mid = L / 2;

        double iso = ftd::K_B / std::sqrt(3.0);
        rb.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(500);

        // Record steady-state density
        int idx = rb.lattice().index(mid, mid, mid);
        double rho_settled = rb.voxels()[idx].density();

        // Set slow velocity and unlock
        rb.voxels()[idx].velocity = {0.0, 0.0, 0.01};
        rb.voxels()[idx].locked = false;

        // Track density at particle site through moves
        std::cout << "  Settled density: " << std::setprecision(4) << rho_settled << "\n";
        std::cout << "  tick   pos_z   density    wave_vel\n";
        int last_z = mid;
        for (int t = 0; t < 300; ++t) {
            rb.tick();
            auto p = find_particle(rb, +1);
            if (!p.found) {
                std::cout << "  " << std::setw(4) << t << "  EVAPORATED\n";
                break;
            }
            double rho = rb.voxels()[p.idx].density();
            double wv = rb.voxels()[p.idx].wave_vel.mag();
            if (p.z != last_z || t % 50 == 0) {
                std::cout << "  " << std::setw(4) << t
                          << "  " << std::setw(5) << p.z
                          << "  " << std::setw(10) << std::setprecision(6) << rho
                          << "  " << std::setw(10) << wv << "\n";
                last_z = p.z;
            }
        }
        auto p = find_particle(rb, +1);
        check("PL5: Slow particle (v=0.01) retains density after moves", p.found);
        if (p.found) {
            double rho_final = rb.voxels()[p.idx].density();
            std::cout << "  Final density: " << std::setprecision(6) << rho_final
                      << " (threshold: " << ftd::K_B * 1e-4 << ")\n";
            check("PL5b: Final density > evap threshold",
                  rho_final > ftd::K_B * 1e-4);
        }
    }
    std::cout << std::setprecision(6) << std::defaultfloat;

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All particle lifetime checks PASSED.\n";
    } else {
        std::cout << "  " << failures << " check(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
