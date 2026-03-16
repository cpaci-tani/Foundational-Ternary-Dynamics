/**
 * Test: AE Hydrogen Bonds (LJ 10-12 + angular)
 *
 * Verifies H-bond force between D-H...A where D and A are electronegative.
 * LJ 10-12 potential with cos²(theta_DHA) angular dependence.
 *
 * Uses proper atomic-scale distances derived from compute_atomic_properties().
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
    std::cout << "  TEST: AE Hydrogen Bonds\n";
    std::cout << "================================================================\n";

    // Compute proper atomic-scale distances
    auto p_o = ftd::compute_atomic_properties(8);
    auto p_h = ftd::compute_atomic_properties(1);
    double sig_hb = 0.5 * (p_o.radius + p_h.radius) * ftd::N_BASE;
    double bond_len = 0.5 * sig_hb;     // D-H bond length (inside well)
    double hb_dist = 1.2 * sig_hb;       // H...A distance (in attractive region)

    std::cout << "  sig_hb=" << sig_hb << " bond_len=" << bond_len
              << " hb_dist=" << hb_dist << "\n";

    // ---- HB1: H bonded to O, near another O → nonzero H-bond ----
    std::cout << "\n--- HB1: D-H...A produces force ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int d = ae.add_atom(8, {0, 0, 0});
        int h = ae.add_atom(1, {bond_len, 0, 0});
        ae.create_bond(d, h, 1);
        ae.add_atom(8, {bond_len + hb_dist, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.h_bonds = true;

        ftd::Vec3 f = ae.compute_force(1);
        std::cout << "  f = (" << f.x << ", " << f.y << ", " << f.z << ") |f|=" << f.mag() << "\n";
        check("HB1: nonzero H-bond force", f.mag() > 1e-30);
    }

    // ---- HB2: No H involved → zero ----
    std::cout << "\n--- HB2: No hydrogen → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.add_atom(8, {0, 0, 0});
        ae.add_atom(8, {hb_dist, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.h_bonds = true;

        ftd::Vec3 f = ae.compute_force(0);
        check("HB2: zero when no H present", f.mag() < 1e-30);
    }

    // ---- HB3: Toggle OFF → zero ----
    std::cout << "\n--- HB3: Toggle OFF → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int d = ae.add_atom(8, {0, 0, 0});
        int h = ae.add_atom(1, {bond_len, 0, 0});
        ae.create_bond(d, h, 1);
        ae.add_atom(8, {bond_len + hb_dist, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.h_bonds = false;

        ftd::Vec3 f = ae.compute_force(1);
        check("HB3: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- HB4: H bonded to C (not electronegative) → no H-bond ----
    std::cout << "\n--- HB4: Non-electronegative donor → zero ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int c = ae.add_atom(6, {0, 0, 0});
        int h = ae.add_atom(1, {bond_len, 0, 0});
        ae.create_bond(c, h, 1);
        ae.add_atom(8, {bond_len + hb_dist, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.h_bonds = true;

        ftd::Vec3 f = ae.compute_force(1);
        check("HB4: zero when donor not electronegative (C-H...O)", f.mag() < 1e-30);
    }

    // ---- HB5: Force depends on distance (decays at large r) ----
    std::cout << "\n--- HB5: Distance dependence ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        int d = ae.add_atom(8, {0, 0, 0});
        int h = ae.add_atom(1, {bond_len, 0, 0});
        ae.create_bond(d, h, 1);
        ae.add_atom(8, {bond_len + hb_dist, 0, 0});

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.h_bonds = true;

        double f_near = ae.compute_force(1).mag();

        // Move acceptor 3x farther
        ae.atoms()[2].position = {bond_len + 3.0 * hb_dist, 0, 0};
        double f_far = ae.compute_force(1).mag();

        std::cout << "  f_near=" << f_near << " f_far=" << f_far << "\n";
        check("HB5: force decreases with distance", f_near > f_far * 2.0);
    }

    // ---- HB6: Diagnostic component ----
    std::cout << "\n--- HB6: Diagnostic ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        int d = ae.add_atom(8, {0, 0, 0});
        int h = ae.add_atom(1, {bond_len, 0, 0});
        ae.create_bond(d, h, 1);
        ae.add_atom(8, {bond_len + hb_dist, 0, 0});

        // Lock all atoms so tick() doesn't move them
        ae.atoms()[0].locked = true;
        ae.atoms()[1].locked = true;
        ae.atoms()[2].locked = true;

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.h_bonds = true;

        ae.tick();
        const auto& fd = ae.force_diag();
        if (fd.size() >= 2) {
            double hb_mag = fd[1].f_hbond.mag();
            std::cout << "  fd[1].f_hbond=" << hb_mag << "\n";
            check("HB6: hbond diag nonzero on H", hb_mag > 1e-30);
        } else {
            check("HB6: hbond diag nonzero on H", false);
        }
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All hydrogen bond tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
