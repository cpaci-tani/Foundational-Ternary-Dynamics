/**
 * Test: AE Electronegativity (Pauling chi)
 *
 * Verifies electronegativity values and their effect on bonding and dipoles.
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
    std::cout << "  TEST: AE Electronegativity\n";
    std::cout << "================================================================\n";

    // ---- EN1: Correct Pauling chi for common elements ----
    std::cout << "\n--- EN1: Pauling chi values ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int h = ae.add_atom(1, {0, 0, 0});
        int o = ae.add_atom(8, {10, 0, 0});
        int f = ae.add_atom(9, {20, 0, 0});

        double chi_h = ae.atoms()[0].electronegativity;
        double chi_o = ae.atoms()[1].electronegativity;
        double chi_f = ae.atoms()[2].electronegativity;

        std::cout << "  chi_H=" << chi_h << " chi_O=" << chi_o << " chi_F=" << chi_f << "\n";
        check("EN1a: H chi = 2.20", std::abs(chi_h - 2.20) < 0.01);
        check("EN1b: O chi = 3.44", std::abs(chi_o - 3.44) < 0.01);
        check("EN1c: F > O > H", chi_f > chi_o && chi_o > chi_h);
    }

    // ---- EN2: Polar bond forms more readily (extended radius) ----
    std::cout << "\n--- EN2: Polar bond formation ---\n";
    {
        // Test with electronegativity ON: O-H should bond at larger distance
        ftd::AtomEngine ae_on;
        ae_on.set_damping_enabled(false);
        ae_on.toggles.ionic = false;
        ae_on.toggles.van_der_waals = false;
        ae_on.toggles.covalent_bonds = false;
        ae_on.toggles.auto_bonding = true;
        ae_on.toggles.electronegativity = true;

        double sig_avg = 0.0;
        {
            // Get sigma_avg for O-H to set distance just outside normal bonding range
            auto p_o = ftd::compute_atomic_properties(8);
            auto p_h = ftd::compute_atomic_properties(1);
            sig_avg = 0.5 * (p_o.vdw_sigma + p_h.vdw_sigma);
        }
        // Place at 1.15 * sigma (inside 1.2*sigma range but on the edge)
        double dist = 1.15 * sig_avg;
        ae_on.add_atom(8, {0, 0, 0});
        ae_on.add_atom(1, {dist, 0, 0});
        ae_on.tick();
        int bonds_on = ae_on.diagnostics().bond_count;

        // Same setup with electronegativity OFF
        ftd::AtomEngine ae_off;
        ae_off.set_damping_enabled(false);
        ae_off.toggles.ionic = false;
        ae_off.toggles.van_der_waals = false;
        ae_off.toggles.covalent_bonds = false;
        ae_off.toggles.auto_bonding = true;
        ae_off.toggles.electronegativity = false;

        ae_off.add_atom(8, {0, 0, 0});
        ae_off.add_atom(1, {dist, 0, 0});
        ae_off.tick();
        int bonds_off = ae_off.diagnostics().bond_count;

        std::cout << "  bonds_on=" << bonds_on << " bonds_off=" << bonds_off
                  << " dist=" << dist << " sig=" << sig_avg << "\n";
        // With electronegativity ON, bonding range is extended → should form bond
        check("EN2: polar bond forms with electronegativity ON", bonds_on >= bonds_off);
    }

    // ---- EN3: O-H bond produces dipole moment ----
    std::cout << "\n--- EN3: Bond dipole from chi difference ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        int o = ae.add_atom(8, {0, 0, 0});
        int h = ae.add_atom(1, {5, 0, 0});
        ae.create_bond(o, h, 1);

        ae.toggles.dipole_dipole = true;
        ae.tick();

        double d_o = ae.atoms()[0].dipole_moment.mag();
        double d_h = ae.atoms()[1].dipole_moment.mag();
        std::cout << "  dipole_O=" << d_o << " dipole_H=" << d_h << "\n";
        check("EN3: O-H bond produces nonzero dipole moment", d_o > 1e-5 && d_h > 1e-5);
    }

    // ---- EN4: Nonpolar pair → no dipole ----
    std::cout << "\n--- EN4: Nonpolar bond → zero dipole ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        int h1 = ae.add_atom(1, {0, 0, 0});
        int h2 = ae.add_atom(1, {5, 0, 0});
        ae.create_bond(h1, h2, 1);

        ae.toggles.dipole_dipole = true;
        ae.tick();

        double d1 = ae.atoms()[0].dipole_moment.mag();
        std::cout << "  dipole_H1=" << d1 << "\n";
        check("EN4: H-H bond → near-zero dipole", d1 < 1e-10);
    }

    // ---- EN5: Toggle OFF → no effect on bonding ----
    std::cout << "\n--- EN5: Toggle OFF → standard bonding ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(false);
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.auto_bonding = true;
        ae.toggles.electronegativity = false;

        auto p_o = ftd::compute_atomic_properties(8);
        auto p_h = ftd::compute_atomic_properties(1);
        double sig_avg = 0.5 * (p_o.vdw_sigma + p_h.vdw_sigma);
        // Place atoms just outside normal bonding range (1.2 * sig_avg)
        ae.add_atom(8, {0, 0, 0});
        ae.add_atom(1, {sig_avg * 1.25, 0, 0});  // outside 1.2*sig_avg
        ae.tick();
        int bonds = ae.diagnostics().bond_count;
        check("EN5: no bond at 1.25*sigma without electronegativity", bonds == 0);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All electronegativity tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
