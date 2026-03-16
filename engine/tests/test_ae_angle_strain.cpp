/**
 * Test: AE Angle Strain (VSEPR)
 *
 * Verifies harmonic angle potential V = K_ANGLE * (theta - theta_eq)^2 / 2
 * with equilibrium angles from VSEPR theory.
 */

#include <cmath>
#include <iostream>
#include "ftd/atom_engine.h"
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
    std::cout << "  TEST: AE Angle Strain (VSEPR)\n";
    std::cout << "================================================================\n";

    // ---- AS1: Water-like bend (2 bonds + 2 LP) → force when angle != 104.5° ----
    std::cout << "\n--- AS1: Water angle → nonzero force ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // O center with 2 bonds (water-like: valence_electrons=2 from max_bonds)
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});  // 90° angle (not 104.5°)
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        ftd::Vec3 f = ae.compute_force(0);
        std::cout << "  f_center = (" << f.x << ", " << f.y << ", " << f.z << ") |f|=" << f.mag() << "\n";
        check("AS1: nonzero force at non-equilibrium angle", f.mag() > 1e-15);
    }

    // ---- AS2: Tetrahedral angle (4 bonds, 0 LP) → ~109.47° ----
    std::cout << "\n--- AS2: Tetrahedral → force at wrong angle ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // C center with 4 bonds
        int c = ae.add_atom(6, {0, 0, 0});
        // Place 2 H atoms at 90° (not tetrahedral 109.47°)
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});
        ae.create_bond(c, h1, 1);
        ae.create_bond(c, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        ftd::Vec3 f = ae.compute_force(0);
        check("AS2: nonzero force at 90° (not 109.47°)", f.mag() > 1e-15);
    }

    // ---- AS3: Toggle OFF → zero ----
    std::cout << "\n--- AS3: Toggle OFF → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = false;

        ftd::Vec3 f = ae.compute_force(0);
        check("AS3: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- AS4: Force restores toward equilibrium ----
    std::cout << "\n--- AS4: Restoring force direction ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        // Water-like: O with 2 bonds
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});  // 90° (< 104.5° equilibrium)
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        // Run a few ticks — angle should increase toward 104.5°
        // Measure initial angle
        auto angle = [&]() {
            auto& atoms = ae.atoms();
            ftd::Vec3 r1 = atoms[1].position - atoms[0].position;
            ftd::Vec3 r2 = atoms[2].position - atoms[0].position;
            double m1 = std::sqrt(r1.mag2());
            double m2 = std::sqrt(r2.mag2());
            double cos_t = (r1.x*r2.x + r1.y*r2.y + r1.z*r2.z) / (m1 * m2);
            return std::acos(std::max(-1.0, std::min(1.0, cos_t))) * 180.0 / ftd::PI;
        };

        double angle_before = angle();
        ae.run(100);
        double angle_after = angle();

        std::cout << "  angle_before=" << angle_before << "° angle_after=" << angle_after << "°\n";
        // 90° should move toward 104.5° → angle should increase
        check("AS4: angle moves toward equilibrium (increases from 90°)", angle_after > angle_before);
    }

    // ---- AS5: No bonds → zero ----
    std::cout << "\n--- AS5: No bonds → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.add_atom(8, {0, 0, 0});
        ae.add_atom(1, {5, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        ftd::Vec3 f = ae.compute_force(0);
        check("AS5: zero when no bonds (no angles to strain)", f.mag() < 1e-30);
    }

    // ---- AS6: Diagnostic component ----
    std::cout << "\n--- AS6: Diagnostic ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        ae.tick();
        const auto& fd = ae.force_diag();
        if (fd.size() >= 1) {
            check("AS6: angle diag nonzero on center atom", fd[0].f_angle.mag() > 1e-30);
        } else {
            check("AS6: angle diag nonzero on center atom", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All angle strain tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
