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

// ============================================================================
// Section: cpu_gpu_parity  (Wave 5.4 Phase 1)
//
// Build a moderately-sized particle system that exercises the GPU threshold
// (N >= 8), run compute_all_forces via tick() twice — once with use_gpu
// off, once with use_gpu on — and compare the per-particle force components
// term-by-term. This is the definitive CPU/GPU numerical parity check
// for the Wave 5.4 pair-force kernel (Coulomb + gravity).
// ============================================================================

static void section_cpu_gpu_parity() {
    // 12 particles in a 3x2x2 grid with spacing 30 (avoids r_eff annihilation
    // contact ~5 and keeps the potential smooth). All particles locked so
    // integration is a no-op and we can read force_diag from the first
    // compute_all_forces call cleanly.
    const double SPACING = 30.0;

    auto build = [SPACING](ftd::ParticleEngine& pe) {
        pe.set_damping_enabled(false);
        pe.set_softening(1.0);

        int id = 0;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    int8_t charge = (id % 3 == 0) ? +1
                                  : (id % 3 == 1) ? -1
                                  : 0;
                    double mass = ftd::K_B * (1.0 + 0.1 * (id % 5));
                    int aid = pe.add_particle(charge,
                        {static_cast<double>(i) * SPACING,
                         static_cast<double>(j) * SPACING,
                         static_cast<double>(k) * SPACING},
                        {0, 0, 0}, mass);
                    pe.particles()[aid].locked = true;  // freeze for force query
                    ++id;
                }
            }
        }
        pe.toggles.coulomb = true;
        pe.toggles.gravity = true;
        pe.toggles.damping = false;
        pe.toggles.strong = false;
        pe.toggles.exchange = false;
        pe.toggles.lorentz = false;
        pe.toggles.magnetic_dipole = false;
        pe.toggles.spin_orbit = false;
        pe.toggles.radiation = false;
        pe.toggles.relativistic = false;
    };

    // Note: CPU `compute_all_forces()` runs through Barnes-Hut with
    // THETA_BH = 0.5, which opens monopole approximations for internal
    // nodes. For 12 particles in a 3x2x2 grid this measurably diverges
    // from exact O(N²). The GPU kernel is exact O(N²), so comparing
    // `force_diag()` from CPU and GPU directly would confound a real
    // parity bug with the Barnes-Hut approximation.
    //
    // Instead we build a "reference" CPU answer by looping
    // `compute_pairwise_force(i, j)` directly — this is the same inner
    // function the GPU kernel mirrors — and compare *that* to the GPU
    // force_diag. Any deviation is a real parity bug in the kernel.

    std::cout << "\n--- PGP1: GPU coulomb+gravity path produces forces ---\n";
    ftd::ParticleEngine pe_gpu;
    build(pe_gpu);
    pe_gpu.set_use_gpu(true);
    pe_gpu.run(1);
    const auto& fd_gpu = pe_gpu.force_diag();
    double gpu_total = 0.0;
    for (const auto& d : fd_gpu) gpu_total += d.f_coulomb.mag() + d.f_gravity.mag();
    std::cout << "  GPU N=" << fd_gpu.size()
              << " sum|f_c|+|f_g|=" << gpu_total
              << " f_c[0]=(" << fd_gpu[0].f_coulomb.x << ","
              << fd_gpu[0].f_coulomb.y << ","
              << fd_gpu[0].f_coulomb.z << ")\n";
    ftd::test::check("PGP1: GPU path produces nonzero forces",
                     gpu_total > 1e-30);

    std::cout << "\n--- PGP2: CPU pairwise reference is computed ---\n";
    // Build an independent CPU engine so we can call compute_pairwise_force
    // without running the full octree-based compute_all_forces.
    ftd::ParticleEngine pe_ref;
    build(pe_ref);
    pe_ref.set_use_gpu(false);
    // Need force_diag to exist; trigger one tick to allocate it, then
    // overwrite forces via direct pairwise loop.
    pe_ref.run(1);
    int N = static_cast<int>(pe_ref.particles().size());
    std::vector<ftd::Vec3> ref_coulomb(N), ref_gravity(N), ref_total(N);
    for (int i = 0; i < N; ++i) {
        ftd::Vec3 fc, fg;
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            // Call CPU pair force with gravity toggle off to isolate
            // coulomb-only contribution. Then call it again with coulomb
            // off to isolate gravity-only. This is the cleanest "CPU
            // reference" because it exercises the same single-pair code
            // path the GPU kernel mirrors.
            pe_ref.toggles.coulomb = true;
            pe_ref.toggles.gravity = false;
            fc += pe_ref.compute_pairwise_force(i, j);
            pe_ref.toggles.coulomb = false;
            pe_ref.toggles.gravity = true;
            fg += pe_ref.compute_pairwise_force(i, j);
            pe_ref.toggles.coulomb = true;
            pe_ref.toggles.gravity = true;
        }
        ref_coulomb[i] = fc;
        ref_gravity[i] = fg;
        ref_total[i]   = fc + fg;
    }
    double ref_total_mag = 0.0;
    for (int i = 0; i < N; ++i) ref_total_mag += ref_total[i].mag();
    std::cout << "  REF N=" << N << " sum|F_total|=" << ref_total_mag
              << " ref_coulomb[0]=(" << ref_coulomb[0].x << ","
              << ref_coulomb[0].y << "," << ref_coulomb[0].z << ")\n";
    ftd::test::check("PGP2: CPU pairwise reference nonzero", ref_total_mag > 1e-30);

    std::cout << "\n--- PGP3: GPU vs CPU pairwise reference parity ---\n";
    double max_c_abs = 0.0, max_c_rel = 0.0;
    double max_g_abs = 0.0, max_g_rel = 0.0;
    double max_total_abs = 0.0;
    for (int i = 0; i < N && i < static_cast<int>(fd_gpu.size()); ++i) {
        ftd::Vec3 dc = {
            ref_coulomb[i].x - fd_gpu[i].f_coulomb.x,
            ref_coulomb[i].y - fd_gpu[i].f_coulomb.y,
            ref_coulomb[i].z - fd_gpu[i].f_coulomb.z,
        };
        ftd::Vec3 dg = {
            ref_gravity[i].x - fd_gpu[i].f_gravity.x,
            ref_gravity[i].y - fd_gpu[i].f_gravity.y,
            ref_gravity[i].z - fd_gpu[i].f_gravity.z,
        };
        ftd::Vec3 dt = {
            ref_total[i].x - (fd_gpu[i].f_coulomb.x + fd_gpu[i].f_gravity.x),
            ref_total[i].y - (fd_gpu[i].f_coulomb.y + fd_gpu[i].f_gravity.y),
            ref_total[i].z - (fd_gpu[i].f_coulomb.z + fd_gpu[i].f_gravity.z),
        };
        double c_abs = dc.mag();
        double g_abs = dg.mag();
        double t_abs = dt.mag();
        double c_ref = ref_coulomb[i].mag();
        double g_ref = ref_gravity[i].mag();
        max_c_abs     = std::max(max_c_abs, c_abs);
        max_g_abs     = std::max(max_g_abs, g_abs);
        max_total_abs = std::max(max_total_abs, t_abs);
        if (c_ref > 1e-30) max_c_rel = std::max(max_c_rel, c_abs / c_ref);
        if (g_ref > 1e-30) max_g_rel = std::max(max_g_rel, g_abs / g_ref);
    }
    std::cout << "  max |F_c_ref - F_c_gpu| = " << max_c_abs
              << " (rel " << max_c_rel << ")\n";
    std::cout << "  max |F_g_ref - F_g_gpu| = " << max_g_abs
              << " (rel " << max_g_rel << ")\n";
    std::cout << "  max |F_total_ref - F_total_gpu| = " << max_total_abs << "\n";
    ftd::test::check("PGP3: coulomb parity within 1e-12 abs", max_c_abs < 1e-12);
    ftd::test::check("PGP4: coulomb parity within 1e-12 rel", max_c_rel < 1e-12);
    ftd::test::check("PGP5: gravity parity within 1e-12 abs", max_g_abs < 1e-12);
    ftd::test::check("PGP6: total force parity within 1e-12", max_total_abs < 1e-12);

    std::cout << "\n--- PGP7: GPU threshold N<8 → CPU fallback ---\n";
    ftd::ParticleEngine pe_tiny;
    pe_tiny.set_damping_enabled(false);
    pe_tiny.set_softening(1.0);
    int t0 = pe_tiny.add_particle(+1, { 0,  0, 0});
    int t1 = pe_tiny.add_particle(-1, {30,  0, 0});
    int t2 = pe_tiny.add_particle(+1, { 0, 30, 0});
    int t3 = pe_tiny.add_particle(-1, {30, 30, 0});
    pe_tiny.particles()[t0].locked = true;
    pe_tiny.particles()[t1].locked = true;
    pe_tiny.particles()[t2].locked = true;
    pe_tiny.particles()[t3].locked = true;
    pe_tiny.toggles.coulomb = true;
    pe_tiny.toggles.gravity = true;
    pe_tiny.toggles.damping = false;
    pe_tiny.set_use_gpu(true);
    pe_tiny.run(1);
    const auto& fd_tiny = pe_tiny.force_diag();
    double tiny_total = 0.0;
    for (const auto& d : fd_tiny) tiny_total += d.f_coulomb.mag() + d.f_gravity.mag();
    std::cout << "  tiny N=" << fd_tiny.size()
              << " sum|f|=" << tiny_total << "\n";
    ftd::test::check("PGP7: N<8 system still computes nonzero forces via CPU fallback",
                     tiny_total > 1e-30);

    std::cout << "\n--- PGP8: advanced toggle (strong) forces CPU fallback ---\n";
    ftd::ParticleEngine pe_adv;
    build(pe_adv);
    pe_adv.toggles.strong = true;   // any advanced toggle triggers CPU path
    pe_adv.set_use_gpu(true);
    pe_adv.run(1);
    const auto& fd_adv = pe_adv.force_diag();
    bool adv_has_forces = false;
    for (const auto& d : fd_adv) {
        if (d.f_coulomb.mag() + d.f_gravity.mag() > 1e-30) { adv_has_forces = true; break; }
    }
    std::cout << "  adv N=" << fd_adv.size()
              << " has_forces=" << (adv_has_forces ? "yes" : "no") << "\n";
    ftd::test::check("PGP8: advanced-toggle system produces forces via CPU fallback",
                     adv_has_forces);
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

    ftd::test::section("cpu_gpu_parity");
    section_cpu_gpu_parity();

    return ftd::test::finalize();
}
