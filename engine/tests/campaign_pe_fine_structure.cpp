/**
 * Campaign: PE Fine Structure
 *
 * Tests multiple Phase 2 forces working together:
 * spin-orbit + relativistic corrections produce fine structure splitting.
 * Hydrogen-like system with spin on the electron.
 */

#include <cmath>
#include <iostream>
#include "ftd/particle_engine.h"
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
    std::cout << "  CAMPAIGN: PE Fine Structure (Combined Phase 2 Forces)\n";
    std::cout << "================================================================\n";

    // ---- FS1: Spin-orbit + relativistic changes energy vs Coulomb-only ----
    std::cout << "\n--- FS1: Fine structure correction changes total energy ---\n";
    {
        // Coulomb-only hydrogen
        ftd::ParticleEngine pe_bare;
        pe_bare.set_damping_enabled(false);
        pe_bare.set_dt(0.01);
        pe_bare.add_particle(+1, {0, 0, 0});
        pe_bare.particles()[0].locked = true;
        pe_bare.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, +1, 0);

        pe_bare.toggles.coulomb = true;
        pe_bare.toggles.gravity = false;

        pe_bare.run(500);
        auto d_bare = pe_bare.diagnostics();

        // Coulomb + spin-orbit + relativistic
        ftd::ParticleEngine pe_fs;
        pe_fs.set_damping_enabled(false);
        pe_fs.set_dt(0.01);
        pe_fs.add_particle(+1, {0, 0, 0});
        pe_fs.particles()[0].locked = true;
        pe_fs.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, +1, 0);

        pe_fs.toggles.coulomb = true;
        pe_fs.toggles.gravity = false;
        pe_fs.toggles.spin_orbit = true;
        pe_fs.toggles.relativistic = true;

        pe_fs.run(500);
        auto d_fs = pe_fs.diagnostics();

        double delta_E = std::abs(d_fs.total_energy - d_bare.total_energy);
        std::cout << "  E_bare=" << d_bare.total_energy << " E_fs=" << d_fs.total_energy
                  << " delta=" << delta_E << "\n";
        check("FS1: fine structure changes energy", delta_E > 1e-15);
    }

    // ---- FS2: Spin-up vs spin-down orbits differ ----
    std::cout << "\n--- FS2: Spin-up vs spin-down energy splitting ---\n";
    {
        // Spin up
        ftd::ParticleEngine pe_up;
        pe_up.set_damping_enabled(false);
        pe_up.set_dt(0.01);
        pe_up.add_particle(+1, {0, 0, 0});
        pe_up.particles()[0].locked = true;
        pe_up.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, +1, 0);

        pe_up.toggles.coulomb = true;
        pe_up.toggles.gravity = false;
        pe_up.toggles.spin_orbit = true;

        pe_up.run(500);
        auto d_up = pe_up.diagnostics();

        // Spin down
        ftd::ParticleEngine pe_dn;
        pe_dn.set_damping_enabled(false);
        pe_dn.set_dt(0.01);
        pe_dn.add_particle(+1, {0, 0, 0});
        pe_dn.particles()[0].locked = true;
        pe_dn.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, -1, 0);

        pe_dn.toggles.coulomb = true;
        pe_dn.toggles.gravity = false;
        pe_dn.toggles.spin_orbit = true;

        pe_dn.run(500);
        auto d_dn = pe_dn.diagnostics();

        double splitting = std::abs(d_up.total_energy - d_dn.total_energy);
        std::cout << "  E_up=" << d_up.total_energy << " E_dn=" << d_dn.total_energy
                  << " splitting=" << splitting << "\n";
        check("FS2: spin-up and spin-down have different energies", splitting > 1e-15);
    }

    // ---- FS3: Relativistic correction is small relative to Coulomb ----
    std::cout << "\n--- FS3: Relativistic correction small vs Coulomb ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0});
        pe.particles()[0].locked = true;
        pe.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;

        pe.toggles.relativistic = false;
        ftd::Vec3 f_nr = pe.compute_force(1);

        pe.toggles.relativistic = true;
        ftd::Vec3 f_r = pe.compute_force(1);

        double correction = (f_r - f_nr).mag();
        double ratio = correction / f_nr.mag();
        std::cout << "  |F_nr|=" << f_nr.mag() << " correction=" << correction
                  << " ratio=" << ratio << "\n";
        // Relativistic correction should be small (v << c)
        check("FS3: relativistic correction < 10% of Coulomb", ratio < 0.1);
    }

    // ---- FS4: All Phase 2 forces together → system still stable ----
    std::cout << "\n--- FS4: All forces → stable orbit ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.01);
        pe.add_particle(+1, {0, 0, 0}, {}, ftd::K_B, 2.48, +1, 1);
        pe.particles()[0].locked = true;
        pe.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, -1, 2);

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.exchange = true;
        pe.toggles.strong = true;
        pe.toggles.lorentz = true;
        pe.toggles.magnetic_dipole = true;
        pe.toggles.spin_orbit = true;
        pe.toggles.radiation = true;
        pe.toggles.relativistic = true;

        pe.run(1000);

        // System should not explode — particles should remain finite
        const auto& p = pe.particles();
        double r = (p[1].position - p[0].position).mag();
        double v = p[1].velocity.mag();
        std::cout << "  r=" << r << " v=" << v << "\n";
        check("FS4a: separation finite (< 10000)", r < 10000.0);
        check("FS4b: velocity finite (< C_SPEED)", v < ftd::C_SPEED * 1.001);
    }

    // ---- FS5: Force diagnostics show all components ----
    std::cout << "\n--- FS5: Combined force diagnostics ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.01);
        pe.add_particle(+1, {0, 0, 0}, {0, 0.01, 0}, ftd::K_B, 2.48, +1, 1);
        pe.particles()[0].locked = true;
        pe.particles()[0].prev_acceleration = {0.01, 0, 0};
        pe.add_particle(-1, {15, 0, 0}, {0, 0.03, 0}, ftd::K_B, 2.48, -1, 2);
        pe.particles()[1].prev_acceleration = {-0.01, 0, 0};

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.spin_orbit = true;
        pe.toggles.relativistic = true;
        pe.toggles.radiation = true;
        pe.toggles.magnetic_dipole = true;

        pe.tick();
        const auto& fd = pe.force_diag();
        if (fd.size() >= 2) {
            // Particle 1 (the electron) should have multiple force components
            const auto& d = fd[1];
            int nonzero = 0;
            if (d.f_coulomb.mag() > 1e-30) nonzero++;
            if (d.f_spin_orbit.mag() > 1e-30) nonzero++;
            if (d.f_relativistic.mag() > 1e-30) nonzero++;
            if (d.f_radiation.mag() > 1e-30) nonzero++;
            if (d.f_magnetic_dipole.mag() > 1e-30) nonzero++;
            std::cout << "  nonzero force components: " << nonzero << "/5\n";
            check("FS5: at least 3 force components nonzero", nonzero >= 3);
        } else {
            check("FS5: at least 3 force components nonzero", false);
        }
    }

    // ---- FS6: Radiation causes energy loss in orbiting system ----
    std::cout << "\n--- FS6: Radiation + orbit → energy loss ---\n";
    {
        ftd::ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_dt(0.01);
        pe.add_particle(+1, {0, 0, 0});
        pe.particles()[0].locked = true;
        pe.add_particle(-1, {20, 0, 0}, {0, 0.02, 0}, ftd::K_B, 2.48, +1, 0);

        pe.toggles.coulomb = true;
        pe.toggles.gravity = false;
        pe.toggles.radiation = true;

        // Build up prev_acceleration
        pe.run(50);
        auto d0 = pe.diagnostics();

        pe.run(500);
        auto d1 = pe.diagnostics();

        std::cout << "  E0=" << d0.total_energy << " E1=" << d1.total_energy << "\n";
        check("FS6: energy decreases with radiation", d1.total_energy < d0.total_energy);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All fine structure campaign tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
