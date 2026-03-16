/**
 * Test: AtomToggles — Per-force toggle control for AtomEngine
 *
 * Verifies that each toggle in AtomToggles enables/disables its force,
 * and that force_diag_ correctly decomposes forces by type.
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
    std::cout << "  TEST: AtomToggles — Per-Force Control\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Section 1: Default toggles match legacy behavior
    // ================================================================
    std::cout << "\n--- Section 1: Default Toggle State ---\n";
    {
        ftd::AtomEngine ae;
        check("Default: ionic ON", ae.toggles.ionic == true);
        check("Default: van_der_waals ON", ae.toggles.van_der_waals == true);
        check("Default: covalent_bonds ON", ae.toggles.covalent_bonds == true);
        check("Default: auto_bonding ON", ae.toggles.auto_bonding == true);
        check("Default: damping OFF", ae.toggles.damping == false);
        check("Default: h_bonds OFF", ae.toggles.h_bonds == false);
        check("Default: angle_strain OFF", ae.toggles.angle_strain == false);
    }

    // ================================================================
    // Section 2: Backward-compatible setters still work
    // ================================================================
    std::cout << "\n--- Section 2: Backward Compatibility ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(true);
        check("set_damping_enabled(true) → toggles.damping=true", ae.toggles.damping == true);
        check("damping_enabled() reflects toggles", ae.damping_enabled() == true);

        ae.set_bonding_enabled(false);
        check("set_bonding_enabled(false) → toggles.auto_bonding=false", ae.toggles.auto_bonding == false);
        check("bonding_enabled() reflects toggles", ae.bonding_enabled() == false);
    }

    // ================================================================
    // Section 3: Ionic toggle — ON produces force, OFF produces zero
    // ================================================================
    std::cout << "\n--- Section 3: Ionic Toggle ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Na+ and Cl- ions
        ae.add_atom(11, {0, 0, 0}, {}, +1);   // Na+
        ae.add_atom(17, {5, 0, 0}, {}, -1);   // Cl-

        // Ionic ON, vdW OFF, bonds OFF
        ae.toggles.ionic = true;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ftd::Vec3 f_on = ae.compute_force(0);

        ae.toggles.ionic = false;
        ftd::Vec3 f_off = ae.compute_force(0);

        check("Ionic ON → nonzero force", f_on.mag() > 1e-10);
        check("Ionic OFF → zero force", f_off.mag() < 1e-30);
        check("Ionic attractive (Na+ toward Cl-)", f_on.x > 0);
    }

    // ================================================================
    // Section 4: Van der Waals toggle
    // ================================================================
    std::cout << "\n--- Section 4: Van der Waals Toggle ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Two neutral He atoms at moderate distance
        ae.add_atom(2, {0, 0, 0});
        ae.add_atom(2, {8, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.covalent_bonds = false;

        ae.toggles.van_der_waals = true;
        ftd::Vec3 f_on = ae.compute_force(0);

        ae.toggles.van_der_waals = false;
        ftd::Vec3 f_off = ae.compute_force(0);

        check("vdW ON → nonzero force", f_on.mag() > 1e-20);
        check("vdW OFF → zero force", f_off.mag() < 1e-30);
    }

    // ================================================================
    // Section 5: Covalent bond toggle
    // ================================================================
    std::cout << "\n--- Section 5: Covalent Bond Toggle ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Two H atoms bonded
        int id0 = ae.add_atom(1, {0, 0, 0});
        int id1 = ae.add_atom(1, {3, 0, 0});
        ae.create_bond(id0, id1, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;

        ae.toggles.covalent_bonds = true;
        ftd::Vec3 f_on = ae.compute_force(0);

        ae.toggles.covalent_bonds = false;
        ftd::Vec3 f_off = ae.compute_force(0);

        check("Bond ON → nonzero force", f_on.mag() > 1e-20);
        check("Bond OFF → zero force", f_off.mag() < 1e-30);
    }

    // ================================================================
    // Section 6: Force diagnostic decomposition
    // ================================================================
    std::cout << "\n--- Section 6: Force Diagnostics ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        // Na+ and Cl- at distance ~500 (near vdW sigma ~450, in attractive well)
        int id0 = ae.add_atom(11, {0, 0, 0}, {}, +1);
        int id1 = ae.add_atom(17, {500, 0, 0}, {}, -1);
        ae.create_bond(id0, id1, 1);

        ae.toggles.ionic = true;
        ae.toggles.van_der_waals = true;
        ae.toggles.covalent_bonds = true;

        // Run one tick to populate force_diag_
        ae.tick();

        const auto& fd = ae.force_diag();
        check("force_diag has 2 entries", fd.size() == 2);

        if (fd.size() >= 2) {
            check("Ionic diag nonzero", fd[0].f_ionic.mag() > 1e-10);
            check("vdW diag nonzero", fd[0].f_vdw.mag() > 1e-20);
            check("Bond diag nonzero", fd[0].f_bond.mag() > 1e-20);

            // Total should equal sum
            ftd::Vec3 tot = fd[0].total();
            ftd::Vec3 expected = fd[0].f_ionic + fd[0].f_vdw + fd[0].f_bond;
            double diff = (tot - expected).mag();
            check("total() equals sum of components", diff < 1e-15);
        }
    }

    // ================================================================
    // Section 7: enable_all() and minimal()
    // ================================================================
    std::cout << "\n--- Section 7: enable_all() / minimal() ---\n";
    {
        ftd::AtomToggles t;
        t.enable_all();
        check("enable_all: h_bonds ON", t.h_bonds == true);
        check("enable_all: torsional ON", t.torsional == true);
        check("enable_all: thermostat ON", t.thermostat == true);

        t.minimal();
        check("minimal: ionic ON", t.ionic == true);
        check("minimal: h_bonds OFF", t.h_bonds == false);
        check("minimal: thermostat OFF", t.thermostat == false);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All atom toggle tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
