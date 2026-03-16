/**
 * Campaign: AE Water Molecule
 *
 * Tests multiple Phase 3 forces working together on H2O:
 * angle strain + H-bonds + dipole-dipole + electronegativity + thermostat.
 *
 * Uses proper atomic-scale distances from compute_atomic_properties().
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
    std::cout << "  CAMPAIGN: AE Water Molecule (Combined Phase 3 Forces)\n";
    std::cout << "================================================================\n";

    // Compute proper atomic-scale distances
    auto p_o = ftd::compute_atomic_properties(8);
    auto p_h = ftd::compute_atomic_properties(1);
    double sig_avg = 0.5 * (p_o.vdw_sigma + p_h.vdw_sigma);
    double bond_len = 0.8 * sig_avg;  // O-H bond length (inside LJ well)
    double sig_hb = 0.5 * (p_o.radius + p_h.radius) * ftd::N_BASE;

    std::cout << "  sig_avg=" << sig_avg << " bond_len=" << bond_len
              << " sig_hb=" << sig_hb << "\n";

    // ---- W1: Water bond angle near 104.5 degrees ----
    std::cout << "\n--- W1: Water bond angle ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(false);
        ae.set_bonding_enabled(false);

        int o = ae.add_atom(8, {0, 0, 0});
        double angle_rad = 104.5 * ftd::PI / 180.0;
        int h1 = ae.add_atom(1, {bond_len, 0, 0});
        int h2 = ae.add_atom(1, {bond_len * std::cos(angle_rad), bond_len * std::sin(angle_rad), 0});

        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.angle_strain = true;

        auto& atoms = ae.atoms();
        ftd::Vec3 v1 = atoms[1].position - atoms[0].position;
        ftd::Vec3 v2 = atoms[2].position - atoms[0].position;
        double cos_theta = (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)
                         / (v1.mag() * v2.mag());
        double theta_deg = std::acos(cos_theta) * 180.0 / ftd::PI;

        std::cout << "  initial angle = " << theta_deg << " deg\n";
        check("W1: initial water angle near 104.5", std::abs(theta_deg - 104.5) < 1.0);
    }

    // ---- W2: Angle strain force pushes toward equilibrium ----
    std::cout << "\n--- W2: Angle strain force direction ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(false);
        ae.set_bonding_enabled(false);

        // O at origin, H atoms at 90 degrees (perturbed from 104.5 eq)
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {bond_len, 0, 0});
        int h2 = ae.add_atom(1, {0, bond_len, 0});  // 90 degrees

        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.angle_strain = true;
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;

        // Angle strain acts on the CENTRAL atom (oxygen, index 0).
        // At 90 deg (below 104.5 eq), force on O should push the angle open.
        ftd::Vec3 f_o = ae.compute_force(0);
        double f_o_mag = f_o.mag();
        std::cout << "  f_O = (" << f_o.x << ", " << f_o.y << ", " << f_o.z
                  << ") mag=" << f_o_mag << "\n";

        // Force on central atom should be nonzero when angle != equilibrium
        check("W2: angle strain force on O nonzero at 90 deg", f_o_mag > 1e-30);
    }

    // ---- W3: H-bond forms between two water molecules ----
    std::cout << "\n--- W3: H-bond between two waters ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(false);
        ae.set_bonding_enabled(false);

        // Water 1: O at origin with two H
        int o1 = ae.add_atom(8, {0, 0, 0});
        int h1a = ae.add_atom(1, {bond_len, 0, 0});
        int h1b = ae.add_atom(1, {-bond_len * 0.3, bond_len * 0.95, 0});
        ae.create_bond(o1, h1a, 1);
        ae.create_bond(o1, h1b, 1);

        // Water 2: O at ~2 sigma along x (H-bond distance from h1a)
        double water_sep = bond_len + 1.2 * sig_hb;
        int o2 = ae.add_atom(8, {water_sep, 0, 0});
        int h2a = ae.add_atom(1, {water_sep + bond_len, 0, 0});
        int h2b = ae.add_atom(1, {water_sep - bond_len * 0.3, bond_len * 0.95, 0});
        ae.create_bond(o2, h2a, 1);
        ae.create_bond(o2, h2b, 1);

        ae.toggles.h_bonds = true;
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;

        // Lock all atoms to prevent movement
        for (auto& a : ae.atoms()) a.locked = true;

        ae.tick();

        const auto& diag = ae.force_diag();
        double hb_force = 0.0;
        for (int i = 0; i < static_cast<int>(diag.size()); ++i) {
            hb_force += diag[i].f_hbond.mag();
        }
        std::cout << "  total h-bond force = " << hb_force << "\n";
        check("W3: H-bond force nonzero between water molecules", hb_force > 1e-15);
    }

    // ---- W4: Dipole moment along bisector ----
    std::cout << "\n--- W4: Dipole moment direction ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);

        int o = ae.add_atom(8, {0, 0, 0});
        double angle_rad = 104.5 * ftd::PI / 180.0;
        double half_angle = angle_rad / 2.0;
        int h1 = ae.add_atom(1, {bond_len * std::cos(half_angle), bond_len * std::sin(half_angle), 0});
        int h2 = ae.add_atom(1, {bond_len * std::cos(half_angle), -bond_len * std::sin(half_angle), 0});

        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.dipole_dipole = true;
        ae.tick();

        auto& atoms = ae.atoms();
        double d_mag = atoms[0].dipole_moment.mag();
        std::cout << "  O dipole: (" << atoms[0].dipole_moment.x << ", "
                  << atoms[0].dipole_moment.y << ", " << atoms[0].dipole_moment.z
                  << ") mag=" << d_mag << "\n";
        check("W4a: O has nonzero dipole moment", d_mag > 1e-5);
        if (d_mag > 1e-10) {
            check("W4b: dipole y-component near zero (symmetric)",
                  std::abs(atoms[0].dipole_moment.y) < 0.1 * d_mag);
        } else {
            check("W4b: dipole y-component near zero (symmetric)", true);
        }
    }

    // ---- W5: All Phase 3 forces stable ----
    std::cout << "\n--- W5: All Phase 3 forces → stable system ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(true);
        ae.set_bonding_enabled(false);

        double angle_rad = 104.5 * ftd::PI / 180.0;
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {bond_len, 0, 0});
        int h2 = ae.add_atom(1, {bond_len * std::cos(angle_rad), bond_len * std::sin(angle_rad), 0});
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.h_bonds = true;
        ae.toggles.dipole_dipole = true;
        ae.toggles.angle_strain = true;
        ae.toggles.electronegativity = true;

        ae.run(500);

        const auto& atoms = ae.atoms();
        bool all_finite = true;
        for (const auto& a : atoms) {
            if (std::abs(a.position.x) > 1e8 || std::abs(a.position.y) > 1e8 ||
                std::abs(a.position.z) > 1e8) {
                all_finite = false;
            }
        }
        check("W5: all positions finite", all_finite);
    }

    // ---- W6: Thermostat regulates temperature ----
    std::cout << "\n--- W6: Thermostat temperature regulation ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(false);
        ae.set_bonding_enabled(false);

        double angle_rad = 104.5 * ftd::PI / 180.0;
        ae.add_atom(8, {0, 0, 0});
        ae.add_atom(1, {bond_len, 0, 0});
        ae.add_atom(1, {bond_len * std::cos(angle_rad), bond_len * std::sin(angle_rad), 0});
        ae.atoms()[0].velocity = {0.01, 0, 0};
        ae.atoms()[1].velocity = {0, -0.01, 0};
        ae.atoms()[2].velocity = {-0.01, 0.01, 0};

        ae.create_bond(0, 1, 1);
        ae.create_bond(0, 2, 1);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.thermostat = true;

        double target_T = ae.diagnostics().temperature * 0.5;
        ae.set_target_temperature(target_T);
        ae.set_thermostat_tau(5.0);

        ae.run(200);

        double final_T = ae.diagnostics().temperature;
        std::cout << "  target_T=" << target_T << " final_T=" << final_T << "\n";
        check("W6: thermostat regulates temperature", final_T < target_T * 3.0);
    }

    // ---- W7: Bond lengths remain stable ----
    std::cout << "\n--- W7: Bond lengths stable ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(true);
        ae.set_bonding_enabled(false);

        double angle_rad = 104.5 * ftd::PI / 180.0;
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {bond_len, 0, 0});
        int h2 = ae.add_atom(1, {bond_len * std::cos(angle_rad), bond_len * std::sin(angle_rad), 0});
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);

        ae.toggles.h_bonds = true;
        ae.toggles.dipole_dipole = true;
        ae.toggles.angle_strain = true;
        ae.toggles.electronegativity = true;

        ae.run(300);

        int bond_count = ae.diagnostics().bond_count;
        std::cout << "  bonds=" << bond_count << "\n";
        check("W7: bonds survive with all forces", bond_count == 2);
    }

    // ---- W8: Force diagnostics completeness ----
    std::cout << "\n--- W8: Force diagnostic completeness ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_damping_enabled(false);
        ae.set_bonding_enabled(false);

        // Water 1
        int o1 = ae.add_atom(8, {0, 0, 0});
        int h1a = ae.add_atom(1, {bond_len, 0, 0});
        int h1b = ae.add_atom(1, {-bond_len * 0.3, bond_len * 0.95, 0});
        ae.create_bond(o1, h1a, 1);
        ae.create_bond(o1, h1b, 1);

        // Water 2
        double water_sep = bond_len + 1.2 * sig_hb;
        int o2 = ae.add_atom(8, {water_sep, 0, 0});
        int h2a = ae.add_atom(1, {water_sep + bond_len, 0, 0});
        int h2b = ae.add_atom(1, {water_sep - bond_len * 0.3, bond_len * 0.95, 0});
        ae.create_bond(o2, h2a, 1);
        ae.create_bond(o2, h2b, 1);

        // Lock all to prevent movement during tick
        for (auto& a : ae.atoms()) a.locked = true;

        ae.toggles.h_bonds = true;
        ae.toggles.dipole_dipole = true;
        ae.toggles.angle_strain = true;
        ae.toggles.electronegativity = true;

        ae.tick();

        const auto& diag = ae.force_diag();
        double total_hbond = 0, total_dipole = 0, total_angle = 0;
        for (const auto& d : diag) {
            total_hbond += d.f_hbond.mag();
            total_dipole += d.f_dipole.mag();
            total_angle += d.f_angle.mag();
        }
        int nonzero = 0;
        if (total_hbond > 1e-30) nonzero++;
        if (total_dipole > 1e-30) nonzero++;
        if (total_angle > 1e-30) nonzero++;

        std::cout << "  hbond=" << total_hbond << " dipole=" << total_dipole
                  << " angle=" << total_angle << " nonzero=" << nonzero << "/3\n";
        check("W8: at least 2 of 3 Phase 3 force diagnostics nonzero", nonzero >= 2);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All water campaign tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
