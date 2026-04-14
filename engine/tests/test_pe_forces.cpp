/**
 * Test: ParticleEngine force variants (consolidated suite)
 *
 * Merges 7 legacy test_pe_*.cpp files into a single ftd::test-instrumented
 * suite using the Phase 2a NDJSON telemetry API:
 *
 *   test_pe_exchange         -> section "exchange"
 *   test_pe_lorentz          -> section "lorentz"
 *   test_pe_magnetic_dipole  -> section "magnetic_dipole"
 *   test_pe_radiation        -> section "radiation"
 *   test_pe_relativistic     -> section "relativistic"
 *   test_pe_spin_orbit       -> section "spin_orbit"
 *   test_pe_strong           -> section "strong"
 *
 * Every check(...) from the legacy files is preserved verbatim (same condition,
 * same label) and routed through ftd::test::check for uniform telemetry.
 */

#include <cmath>
#include <iostream>

#include "ftd/particle_engine.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// --- Section: exchange  (from test_pe_exchange.cpp) ---

static void section_exchange() {
    // ---- EX1: Same spin, same charge -> nonzero repulsive force ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Two electrons: charge -1, spin +1, at separation 3
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("EX1a: nonzero exchange force", f.mag() > 1e-20);
        ftd::test::check("EX1b: repulsive (away from j, f.x < 0)", f.x < 0);
    }

    // ---- EX2: Different spin -> zero exchange force ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, -1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("EX2: zero force for different spin", f.mag() < 1e-30);
    }

    // ---- EX3: Same spin, different charge -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("EX3: zero force for different charge", f.mag() < 1e-30);
    }

    // ---- EX4: Toggle OFF -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = false;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("EX4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- EX5: Exponential decay with distance ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {2, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;
        double f_near = pe.compute_force(0).mag();

        pe.particles()[1].position = {6, 0, 0};
        double f_far = pe.compute_force(0).mag();

        ftd::test::check("EX5: force decreases with distance", f_near > f_far * 5.0);
    }

    // ---- EX6: Diagnostic component matches ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(-1, {3, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            ftd::test::check("EX6: exchange diag nonzero", fd[0].f_exchange.mag() > 1e-20);
        } else {
            ftd::test::check("EX6: exchange diag nonzero", false);
        }
    }
}

// --- Section: lorentz  (from test_pe_lorentz.cpp) ---

static void section_lorentz() {
    // ---- LZ1: Moving charge near spinning particle -> nonzero force ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("LZ1: nonzero Lorentz force", f.mag() > 1e-30);
    }

    // ---- LZ2: Stationary charge -> zero Lorentz ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("LZ2: zero force when v=0", f.mag() < 1e-30);
    }

    // ---- LZ3: Force perpendicular to velocity ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 10, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::Vec3 v = {0.1, 0, 0};
        double v_dot_f = v.x * f.x + v.y * f.y + v.z * f.z;
        double relative = (f.mag() > 1e-30) ? std::abs(v_dot_f) / (v.mag() * f.mag()) : 0.0;
        std::cout << "  v.F=" << v_dot_f << " |v|=" << v.mag() << " |F|=" << f.mag()
                  << " cos=" << relative << "\n";
        ftd::test::check("LZ3: F perpendicular to v (cos < 0.1)", relative < 0.1);
    }

    // ---- LZ4: Toggle OFF -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = false;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("LZ4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- LZ5: No dipole source -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});
        pe.add_particle(+1, {10, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("LZ5: zero when no dipole sources", f.mag() < 1e-30);
    }

    // ---- LZ6: Diagnostic component ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.lorentz = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            ftd::test::check("LZ6: lorentz diag nonzero", fd[0].f_lorentz.mag() > 1e-30);
        } else {
            ftd::test::check("LZ6: lorentz diag nonzero", false);
        }
    }
}

// --- Section: magnetic_dipole  (from test_pe_magnetic_dipole.cpp) ---

static void section_magnetic_dipole() {
    // ---- MD1: Aligned dipoles along separation axis -> attractive ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].spin_axis = {1, 0, 0};
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[1].spin_axis = {1, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        ftd::Vec3 f = pe.compute_force(0);
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        ftd::test::check("MD1: aligned along axis -> attractive (f.x > 0)", f.x > 0);
    }

    // ---- MD2: Anti-aligned dipoles along axis -> repulsive ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].spin_axis = {1, 0, 0};
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[1].spin_axis = {-1, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        ftd::Vec3 f = pe.compute_force(0);
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        ftd::test::check("MD2: anti-aligned along axis -> repulsive (f.x < 0)", f.x < 0);
    }

    // ---- MD3: Zero spin_axis -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0});
        pe.add_particle(+1, {10, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("MD3: zero force when spin_axis=0", f.mag() < 1e-30);
    }

    // ---- MD4: Toggle OFF -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = false;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("MD4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- MD5: Force decays as 1/r^4 ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].spin_axis = {0, 0, 1};
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[1].spin_axis = {0, 0, 1};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        double f_near = pe.compute_force(0).mag();

        pe.particles()[1].position = {10, 0, 0};
        double f_far = pe.compute_force(0).mag();

        double ratio = f_near / f_far;
        std::cout << "  f_near=" << f_near << " f_far=" << f_far
                  << " ratio=" << ratio << " (expect ~16)\n";
        ftd::test::check("MD5: force ratio ~16 when r doubles (1/r^4)",
                         ratio > 10.0 && ratio < 25.0);
    }

    // ---- MD6: Diagnostic component ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.magnetic_dipole = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            ftd::test::check("MD6: magnetic_dipole diag nonzero",
                             fd[0].f_magnetic_dipole.mag() > 1e-30);
        } else {
            ftd::test::check("MD6: magnetic_dipole diag nonzero", false);
        }
    }
}

// --- Section: radiation  (from test_pe_radiation.cpp) ---

static void section_radiation() {
    // ---- RD1: Accelerating charge -> force opposes motion ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});
        pe.particles()[0].prev_acceleration = {0.05, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("RD1: radiation force opposes motion (f.x < 0)", f.x < 0);
    }

    // ---- RD2: Zero acceleration -> zero radiation ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("RD2: zero force when a_prev=0", f.mag() < 1e-30);
    }

    // ---- RD3: Toggle OFF -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});
        pe.particles()[0].prev_acceleration = {0.05, 0, 0};

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = false;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("RD3: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- RD4: Higher acceleration -> larger radiation force ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {0, 0, 0}, {0.1, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        pe.particles()[0].prev_acceleration = {0.01, 0, 0};
        double f_small = pe.compute_force(0).mag();

        pe.particles()[0].prev_acceleration = {0.1, 0, 0};
        double f_large = pe.compute_force(0).mag();

        double ratio = f_large / f_small;
        ftd::test::check("RD4: 10x acceleration -> ~100x force",
                         ratio > 50.0 && ratio < 200.0);
    }

    // ---- RD5: System loses energy over time ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.1);

        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.particles()[0].locked = true;
        pe.add_particle(-1, {20, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, -1, 0);

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        pe.run(10);
        auto d0 = pe.diagnostics();

        pe.run(200);
        auto d1 = pe.diagnostics();

        ftd::test::check("RD5: energy decreases with radiation",
                         d1.total_energy < d0.total_energy);
    }
}

// --- Section: relativistic  (from test_pe_relativistic.cpp) ---

static void section_relativistic() {
    // ---- RE1: High-speed -> reduced acceleration ----
    {
        ftd::ParticleEngine pe_slow;
        pe_slow.set_damping_enabled(false);
        pe_slow.add_particle(+1, {0, 0, 0});
        pe_slow.add_particle(-1, {10, 0, 0});

        pe_slow.toggles.coulomb = true;
        pe_slow.toggles.gravity = false;
        pe_slow.toggles.relativistic = true;

        ftd::Vec3 f_slow = pe_slow.compute_force(0);

        ftd::ParticleEngine pe_fast;
        pe_fast.set_damping_enabled(false);
        pe_fast.add_particle(+1, {0, 0, 0}, {0.5 * ftd::C_SPEED, 0, 0});
        pe_fast.add_particle(-1, {10, 0, 0});

        pe_fast.toggles.coulomb = true;
        pe_fast.toggles.gravity = false;
        pe_fast.toggles.relativistic = true;

        ftd::Vec3 f_fast = pe_fast.compute_force(0);

        ftd::test::check("RE1: fast particle has reduced net force",
                         f_fast.mag() < f_slow.mag());
    }

    // ---- RE2: v=0 -> no relativistic correction ----
    {
        ftd::ParticleEngine pe_on;
        pe_on.set_damping_enabled(false);
        pe_on.add_particle(+1, {0, 0, 0});
        pe_on.add_particle(-1, {10, 0, 0});

        pe_on.toggles.coulomb = true;
        pe_on.toggles.gravity = false;
        pe_on.toggles.relativistic = true;

        ftd::Vec3 f_on = pe_on.compute_force(0);

        ftd::ParticleEngine pe_off;
        pe_off.set_damping_enabled(false);
        pe_off.add_particle(+1, {0, 0, 0});
        pe_off.add_particle(-1, {10, 0, 0});

        pe_off.toggles.coulomb = true;
        pe_off.toggles.gravity = false;
        pe_off.toggles.relativistic = false;

        ftd::Vec3 f_off = pe_off.compute_force(0);

        double diff = (f_on - f_off).mag();
        ftd::test::check("RE2: no correction at v=0", diff < 1e-20);
    }

    // ---- RE3: Toggle OFF -> same as non-relativistic ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {0.3 * ftd::C_SPEED, 0, 0});
        pe.add_particle(-1, {10, 0, 0});

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;

        pe.toggles.relativistic = false;
        ftd::Vec3 f_off = pe.compute_force(0);

        pe.toggles.relativistic = true;
        ftd::Vec3 f_on = pe.compute_force(0);

        ftd::test::check("RE3: relativistic ON changes force at v>0",
                         (f_on - f_off).mag() > 1e-15);
    }

    // ---- RE4: Gamma factor matches expected ----
    {
        double v = 0.5 * ftd::C_SPEED;
        double beta2 = (v * v) / (ftd::C_SPEED * ftd::C_SPEED);
        double gamma = 1.0 / std::sqrt(1.0 - beta2);

        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {v, 0, 0});
        pe.add_particle(-1, {10, 0, 0});

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;

        pe.toggles.relativistic = false;
        double f_nr = pe.compute_force(0).mag();

        pe.toggles.relativistic = true;
        double f_r = pe.compute_force(0).mag();

        double expected_ratio = 1.0 / gamma;
        double actual_ratio = f_r / f_nr;
        double err = std::abs(actual_ratio - expected_ratio) / expected_ratio;
        std::cout << "  gamma=" << gamma << " expected_ratio=" << expected_ratio
                  << " actual=" << actual_ratio << " err=" << err << "\n";
        ftd::test::check("RE4: force ratio matches 1/gamma within 1%", err < 0.01);
    }

    // ---- RE5: Speed limit still enforced ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.1);
        pe.add_particle(+1, {0, 0, 0}, {0.9 * ftd::C_SPEED, 0, 0});
        pe.add_particle(-1, {10, 0, 0});

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.relativistic = true;

        pe.run(100);

        double v_final = pe.particles()[0].velocity.mag();
        ftd::test::check("RE5: speed <= C_SPEED",
                         v_final <= ftd::C_SPEED * 1.001);
    }
}

// --- Section: spin_orbit  (from test_pe_spin_orbit.cpp) ---

static void section_spin_orbit() {
    // ---- SO1: Orbiting particle with spin -> nonzero force ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        ftd::Vec3 f = pe.compute_force(0);
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        ftd::test::check("SO1: nonzero spin-orbit force", f.mag() > 1e-30);
    }

    // ---- SO2: Zero spin -> zero force ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0});
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("SO2: zero force when spin=0", f.mag() < 1e-30);
    }

    // ---- SO3: Zero velocity -> zero orbital L -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("SO3: zero force when v=0 (no L)", f.mag() < 1e-30);
    }

    // ---- SO4: Toggle OFF -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = false;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("SO4: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- SO5: Opposite spin -> opposite force direction ----
    {
        ftd::ParticleEngine pe_up;
        pe_up.set_damping_enabled(false);
        pe_up.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe_up.add_particle(+1, {0, 0, 0});
        pe_up.toggles.coulomb = false;
        pe_up.toggles.gravity = false;
        pe_up.toggles.spin_orbit = true;
        ftd::Vec3 f_up = pe_up.compute_force(0);

        ftd::ParticleEngine pe_dn;
        pe_dn.set_damping_enabled(false);
        pe_dn.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, -1, 0);
        pe_dn.add_particle(+1, {0, 0, 0});
        pe_dn.toggles.coulomb = false;
        pe_dn.toggles.gravity = false;
        pe_dn.toggles.spin_orbit = true;
        ftd::Vec3 f_dn = pe_dn.compute_force(0);

        std::cout << "  f_up = (" << f_up.x << ", " << f_up.y << ", " << f_up.z << ")\n";
        std::cout << "  f_dn = (" << f_dn.x << ", " << f_dn.y << ", " << f_dn.z << ")\n";

        double sum = (f_up + f_dn).mag();
        double diff = (f_up - f_dn).mag();
        ftd::test::check("SO5: opposite spin -> opposite force (|f_up+f_dn| << |f_up-f_dn|)",
                         sum < diff * 0.01);
    }

    // ---- SO6: Diagnostic component ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(-1, {10, 0, 0}, {0, 0.05, 0}, ftd::K_B, 2.48, +1, 0);
        pe.add_particle(+1, {0, 0, 0});

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            ftd::test::check("SO6: spin_orbit diag nonzero",
                             fd[0].f_spin_orbit.mag() > 1e-30);
        } else {
            ftd::test::check("SO6: spin_orbit diag nonzero", false);
        }
    }
}

// --- Section: strong  (from test_pe_strong.cpp) ---

static void section_strong() {
    // ---- ST1: Different colors attract ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 2);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("ST1: different colors -> attractive (f.x > 0)", f.x > 0);
    }

    // ---- ST2: Same colors repel ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("ST2: same colors -> repulsive (f.x < 0)", f.x < 0);
    }

    // ---- ST3: Colorless -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 0);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("ST3: colorless -> zero force", f.mag() < 1e-30);
    }

    // ---- ST4: Toggle OFF -> zero ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 2);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = false;

        ftd::Vec3 f = pe.compute_force(0);
        ftd::test::check("ST4: toggle off -> zero", f.mag() < 1e-30);
    }

    // ---- ST5: Color factor ratio ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);

        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        double f_same = pe.compute_force(0).mag();

        pe.particles()[1].color = 2;
        double f_diff = pe.compute_force(0).mag();

        double ratio = f_same / f_diff;
        ftd::test::check("ST5: |F_same|/|F_diff| ~ 0.5",
                         std::abs(ratio - 0.5) < 0.1);
    }

    // ---- ST6: Force diagnostic ----
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, 0, 1);
        pe.add_particle(+1, {5, 0, 0}, {}, ftd::K_B, 2.48, 0, 2);

        pe.toggles.coulomb = false;
        pe.toggles.gravity = false;
        pe.toggles.strong = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            ftd::test::check("ST6: strong diag nonzero", fd[0].f_strong.mag() > 1e-20);
        } else {
            ftd::test::check("ST6: strong diag nonzero", false);
        }
    }
}

// --- main ---

int main() {
    ftd::test::init("test_pe_forces");

    ftd::test::section("exchange");
    section_exchange();

    ftd::test::section("lorentz");
    section_lorentz();

    ftd::test::section("magnetic_dipole");
    section_magnetic_dipole();

    ftd::test::section("radiation");
    section_radiation();

    ftd::test::section("relativistic");
    section_relativistic();

    ftd::test::section("spin_orbit");
    section_spin_orbit();

    ftd::test::section("strong");
    section_strong();

    return ftd::test::finalize();
}
