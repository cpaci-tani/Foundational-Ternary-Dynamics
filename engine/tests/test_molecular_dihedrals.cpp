/**
 * @file test_molecular_dihedrals.cpp
 * @brief Torsional dihedrals and improper planarity potentials unit test.
 */

#include "ftd/atom_engine.h"
#include "test_helpers.h"
#include <iostream>
#include <cmath>

int main() {
    using namespace ftd;
    using namespace ftd::test;

    Counter c;
    std::cout << "============================================================\n";
    std::cout << "  Scale 2 & 3: Molecular Dihedrals & Improper Planarity Test\n";
    std::cout << "============================================================\n\n";

    // Helper to stabilize bond equilibrium lengths and normalize masses
    auto optimize_system = [](AtomEngine& ae, double mass_val, double k_bond_val) {
        auto index_of_local = [](const AtomEngine& engine, int id) {
            const auto& atoms = engine.atoms();
            for (int i = 0; i < static_cast<int>(atoms.size()); ++i) {
                if (atoms[i].id == id) return i;
            }
            return -1;
        };

        // 1. Normalize masses to mass_val so relaxation happens rapidly
        for (auto& atom : ae.atoms()) {
            atom.mass = mass_val;
        }
        // 2. Set equilibrium bond lengths to initial coordinate distances
        for (auto& atom : ae.atoms()) {
            for (auto& bond : atom.bonds) {
                int partner_idx = index_of_local(ae, bond.partner_id);
                if (partner_idx >= 0) {
                    double dist = (ae.atoms()[partner_idx].position - atom.position).mag();
                    bond.r_eq = dist;
                    bond.k_bond = k_bond_val;
                }
            }
        }
    };

    // ---- Test 1: Dihedral Torsional Strain relaxation ----
    {
        std::cout << "--- Test 1: 4-atom Dihedral Torsional relaxation ---\n";
        AtomEngine ae;
        ae.toggles.minimal();
        ae.toggles.covalent_bonds = true;
        ae.toggles.torsional = true;
        ae.toggles.damping = true;
        ae.toggles.van_der_waals = false; // isolate torsional effects
        ae.toggles.ionic = false;
        ae.toggles.auto_bonding = false;
        
        // Build 4-atom Carbon chain C1-C2-C3-C4
        int c1 = ae.add_atom(6, {0.0, 0.0, 0.0});
        int c2 = ae.add_atom(6, {1.5, 0.0, 0.0});
        int c3 = ae.add_atom(6, {2.0, 1.2, 0.0});
        int c4 = ae.add_atom(6, {2.5, 1.5, 0.5}); // initialized out of plane
        std::cout << "[DEBUG] Atoms created" << std::endl;

        ae.create_bond(c1, c2);
        ae.create_bond(c2, c3);
        ae.create_bond(c3, c4);
        std::cout << "[DEBUG] Bonds created" << std::endl;

        // Stabilize bonds and normalize masses
        optimize_system(ae, 1.0, 2.0);
        std::cout << "[DEBUG] System optimized" << std::endl;
        ae.set_softening(0.0);
        ae.set_dt(0.3);

        // Run relaxation for 10000 ticks
        std::cout << "[DEBUG] Starting ae.run(10000)" << std::endl;
        ae.run(10000);
        std::cout << "[DEBUG] Finished ae.run(10000)" << std::endl;

        // Fetch atom positions
        Vec3 p1 = ae.atoms()[0].position;
        Vec3 p2 = ae.atoms()[1].position;
        Vec3 p3 = ae.atoms()[2].position;
        Vec3 p4 = ae.atoms()[3].position;

        Vec3 b1 = p2 - p1;
        Vec3 b2 = p3 - p2;
        Vec3 b3 = p4 - p3;

        Vec3 m = Vec3::cross(b1, b2);
        Vec3 n = Vec3::cross(b2, b3);

        double m_mag = std::sqrt(m.mag2());
        double n_mag = std::sqrt(n.mag2());

        double costheta = m.dot(n) / (m_mag * n_mag);
        if (costheta > 1.0) costheta = 1.0;
        if (costheta < -1.0) costheta = -1.0;

        double phi = std::acos(costheta);
        double phi_deg = phi * 180.0 / PI;

        std::cout << "    Relaxed Dihedral angle: " << phi_deg << " degrees\n";

        // Under 3-fold periodicity, the stable minima are at 60 (gauche) or 180 (anti).
        // Let's verify that the relaxed dihedral is close to one of the minima within 5 degrees.
        bool at_minimum = (std::abs(phi_deg - 60.0) < 5.0) || 
                          (std::abs(phi_deg - 180.0) < 5.0);
        check("Dihedral angle relaxed to stable minimum (60 or 180 deg)", at_minimum, &c);
    }

    // ---- Test 2: Improper Planarity potential ----
    {
        std::cout << "\n--- Test 2: 4-atom Improper Planarity (sp2 center) ---\n";
        AtomEngine ae;
        ae.toggles.minimal();
        ae.toggles.covalent_bonds = true;
        ae.toggles.improper_torsional = true;
        ae.toggles.damping = true;
        ae.toggles.van_der_waals = false;
        ae.toggles.ionic = false;
        ae.toggles.auto_bonding = false;
        ae.toggles.thermostat = false; // disable thermostat
        
        // Use Boron (Z=5, valence_electrons=3) for sp2 center
        int b0 = ae.add_locked_atom(5, {0.0, 0.0, 0.0});
        int c1 = ae.add_locked_atom(6, {1.5, 0.0, 0.0});
        int c2 = ae.add_locked_atom(6, {-0.75, 1.3, 0.0});
        int c3 = ae.add_atom(6, {-0.75, -1.3, 0.5}); // initialized out of plane in z

        ae.create_bond(b0, c1);
        ae.create_bond(b0, c2);
        ae.create_bond(b0, c3);

        // Stabilize bonds and normalize masses
        optimize_system(ae, 1.0, 2.0);
        ae.set_softening(0.0);
        ae.set_dt(0.3);

        // Run relaxation for 10000 ticks
        ae.run(10000);

        // Check if planar configuration is achieved (z positions of all atoms should be nearly equal)
        double z0 = ae.atoms()[0].position.z;
        double z1 = ae.atoms()[1].position.z;
        double z2 = ae.atoms()[2].position.z;
        double z3 = ae.atoms()[3].position.z;

        std::cout << "    Relaxed Z coordinates: B0=" << z0 << ", C1=" << z1 << ", C2=" << z2 << ", C3=" << z3 << "\n";

        double dz1 = std::abs(z1 - z0);
        double dz2 = std::abs(z2 - z0);
        double dz3 = std::abs(z3 - z0);

        check("Improper planarity restored planar structure (z diff < 0.05)", 
              dz1 < 0.05 && dz2 < 0.05 && dz3 < 0.05, &c);
    }

    return report_and_exit_code(c, "Molecular Dihedrals & Planarity");
}
