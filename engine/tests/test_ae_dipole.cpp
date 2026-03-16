/**
 * Test: AE Electric Dipole-Dipole Interaction
 *
 * Verifies dipole-dipole force between atoms with nonzero dipole moments
 * (computed from bond structure + electronegativity differences).
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
    std::cout << "  TEST: AE Electric Dipole-Dipole Force\n";
    std::cout << "================================================================\n";

    // ---- DD1: Two polar molecules (O-H...H-O) → nonzero force ----
    std::cout << "\n--- DD1: Polar molecules → nonzero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Molecule 1: O at (0,0,0), H at (3,0,0)
        int o1 = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {3, 0, 0});
        ae.create_bond(o1, h1, 1);

        // Molecule 2: O at (20,0,0), H at (23,0,0)
        int o2 = ae.add_atom(8, {20, 0, 0});
        int h2 = ae.add_atom(1, {23, 0, 0});
        ae.create_bond(o2, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.dipole_dipole = true;

        // Compute dipole moments
        ae.tick();
        // Check that dipole moments are nonzero
        check("DD1a: O1 has nonzero dipole", ae.atoms()[0].dipole_moment.mag() > 1e-10);
        check("DD1b: force diag nonzero", ae.force_diag()[0].f_dipole.mag() > 1e-30);
    }

    // ---- DD2: Nonpolar pair (He-He) → zero dipole, zero force ----
    std::cout << "\n--- DD2: Nonpolar → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.add_atom(2, {0, 0, 0});
        ae.add_atom(2, {10, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.dipole_dipole = true;

        ftd::Vec3 f = ae.compute_force(0);
        check("DD2: zero force for nonpolar atoms (no bonds, no dipole)", f.mag() < 1e-30);
    }

    // ---- DD3: Toggle OFF → zero ----
    std::cout << "\n--- DD3: Toggle OFF → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int o1 = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {3, 0, 0});
        ae.create_bond(o1, h1, 1);
        int o2 = ae.add_atom(8, {20, 0, 0});
        int h2 = ae.add_atom(1, {23, 0, 0});
        ae.create_bond(o2, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.dipole_dipole = false;

        ftd::Vec3 f = ae.compute_force(0);
        check("DD3: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- DD4: Force decays with distance ----
    std::cout << "\n--- DD4: Distance decay ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Set dipole moments directly for clean test
        int o1 = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {3, 0, 0});
        ae.create_bond(o1, h1, 1);
        int o2 = ae.add_atom(8, {15, 0, 0});
        int h2 = ae.add_atom(1, {18, 0, 0});
        ae.create_bond(o2, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.dipole_dipole = true;

        // Manually set dipole moments for controlled test
        ae.atoms()[0].dipole_moment = {1, 0, 0};
        ae.atoms()[2].dipole_moment = {1, 0, 0};

        double f_near = ae.compute_force(0).mag();

        // Move molecule 2 farther
        ae.atoms()[2].position = {30, 0, 0};
        ae.atoms()[3].position = {33, 0, 0};

        double f_far = ae.compute_force(0).mag();

        std::cout << "  f_near=" << f_near << " f_far=" << f_far << "\n";
        // 1/r^4: doubling distance should decrease by ~16x
        if (f_near > 1e-30 && f_far > 1e-30) {
            double ratio = f_near / f_far;
            std::cout << "  ratio=" << ratio << " (expect >4 for 1/r^4)\n";
            check("DD4: force decreases with distance", ratio > 4.0);
        } else {
            check("DD4: force decreases with distance", f_near > f_far);
        }
    }

    // ---- DD5: Symmetric nonpolar bond (H-H) → zero dipole ----
    std::cout << "\n--- DD5: Symmetric bond → zero dipole ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int h1 = ae.add_atom(1, {0, 0, 0});
        int h2 = ae.add_atom(1, {3, 0, 0});
        ae.create_bond(h1, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.dipole_dipole = true;
        ae.toggles.electronegativity = true;

        // Compute dipole moments (both H have same chi → zero chi_diff)
        ae.tick();
        // Each H should have near-zero dipole (same electronegativity)
        double d1 = ae.atoms()[0].dipole_moment.mag();
        double d2 = ae.atoms()[1].dipole_moment.mag();
        std::cout << "  dipole_H1=" << d1 << " dipole_H2=" << d2 << "\n";
        check("DD5: symmetric H-H bond → near-zero dipole", d1 < 1e-10 && d2 < 1e-10);
    }

    // ---- DD6: Diagnostic component ----
    std::cout << "\n--- DD6: Diagnostic ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        ae.atoms().reserve(4);
        int o1 = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {3, 0, 0});
        ae.create_bond(o1, h1, 1);
        int o2 = ae.add_atom(8, {15, 0, 0});
        int h2 = ae.add_atom(1, {18, 0, 0});
        ae.create_bond(o2, h2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.dipole_dipole = true;

        ae.tick();
        const auto& fd = ae.force_diag();
        if (fd.size() >= 1) {
            check("DD6: dipole diag populated", fd[0].f_dipole.mag() > 1e-30);
        } else {
            check("DD6: dipole diag populated", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All dipole-dipole tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
