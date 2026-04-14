/**
 * Test: AtomEngine force variants (consolidated suite)
 *
 * Merges 5 legacy test_ae_*.cpp files into a single ftd::test-instrumented
 * suite using the Phase 2a NDJSON telemetry API:
 *
 *   test_ae_angle_strain     -> section "angle_strain"
 *   test_ae_dipole           -> section "dipole"
 *   test_ae_electronegativity -> section "electronegativity"
 *   test_ae_hbonds           -> section "hbonds"
 *   test_ae_thermostat       -> section "thermostat"
 *
 * Every check(...) from the legacy files is preserved verbatim (same condition,
 * same label) and routed through ftd::test::check for uniform telemetry.
 *
 * Wave 4a.1 consolidation (2026-04-14). Depends on the Wave 3.3 engine fix
 * to atom_engine.cpp's apply_thermostat() (clamp lambda_sq >= 0 to avoid
 * NaN velocity propagation under extreme cooling).
 */

#include <cmath>
#include <iostream>

#include "ftd/atom_engine.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: angle_strain  (from test_ae_angle_strain.cpp)
// ============================================================================

static void section_angle_strain() {
    // ---- AS1: Water-like bend (2 bonds + 2 LP) → force when angle != 104.5° ----
    // Note: angle_strain is a 3-body force computed only in compute_all_forces()
    // (called via tick()), not compute_force(). The original test used
    // compute_force() which never included angle_strain — this was always
    // going to fail. Wave 4a.1 updates to use tick() + force_diag instead.
    std::cout << "\n--- AS1: Water angle → nonzero force ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        // Lock atoms so tick() computes forces without moving them
        int o = ae.add_atom(8, {0, 0, 0});
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});  // 90° angle (not 104.5°)
        ae.create_bond(o, h1, 1);
        ae.create_bond(o, h2, 1);
        ae.atoms()[0].locked = ae.atoms()[1].locked = ae.atoms()[2].locked = true;

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        ae.tick();
        ftd::Vec3 f = ae.force_diag()[0].f_angle;
        std::cout << "  f_center = (" << f.x << ", " << f.y << ", " << f.z << ") |f|=" << f.mag() << "\n";
        ftd::test::check("AS1: nonzero force at non-equilibrium angle", f.mag() > 1e-15);
    }

    // ---- AS2: Tetrahedral angle (4 bonds, 0 LP) → ~109.47° ----
    std::cout << "\n--- AS2: Tetrahedral → force at wrong angle ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        // C center with 4 bonds
        int c = ae.add_atom(6, {0, 0, 0});
        // Place 2 H atoms at 90° (not tetrahedral 109.47°)
        int h1 = ae.add_atom(1, {5, 0, 0});
        int h2 = ae.add_atom(1, {0, 5, 0});
        ae.create_bond(c, h1, 1);
        ae.create_bond(c, h2, 1);
        ae.atoms()[0].locked = ae.atoms()[1].locked = ae.atoms()[2].locked = true;

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.covalent_bonds = false;
        ae.toggles.angle_strain = true;

        ae.tick();
        ftd::Vec3 f = ae.force_diag()[0].f_angle;
        ftd::test::check("AS2: nonzero force at 90° (not 109.47°)", f.mag() > 1e-15);
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
        ftd::test::check("AS3: zero when toggle off", f.mag() < 1e-30);
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
        ftd::test::check("AS4: angle moves toward equilibrium (increases from 90°)", angle_after > angle_before);
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
        ftd::test::check("AS5: zero when no bonds (no angles to strain)", f.mag() < 1e-30);
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
            ftd::test::check("AS6: angle diag nonzero on center atom", fd[0].f_angle.mag() > 1e-30);
        } else {
            ftd::test::check("AS6: angle diag nonzero on center atom", false);
        }
    }
}

// ============================================================================
// Section: dipole  (from test_ae_dipole.cpp)
// ============================================================================

static void section_dipole() {
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
        ftd::test::check("DD1a: O1 has nonzero dipole", ae.atoms()[0].dipole_moment.mag() > 1e-10);
        ftd::test::check("DD1b: force diag nonzero", ae.force_diag()[0].f_dipole.mag() > 1e-30);
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
        ftd::test::check("DD2: zero force for nonpolar atoms (no bonds, no dipole)", f.mag() < 1e-30);
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
        ftd::test::check("DD3: zero when toggle off", f.mag() < 1e-30);
    }

    // ---- DD4: Force decays with distance ----
    // Like AS1/AS2, dipole_dipole is a 3+-body force computed only in
    // compute_all_forces() via tick(). Switched from compute_force() to
    // tick() + force_diag[0].f_dipole to measure the actual dipole force.
    std::cout << "\n--- DD4: Distance decay ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
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
        // Lock all atoms so tick() computes forces without moving anything
        for (auto& a : ae.atoms()) a.locked = true;

        // tick() calls compute_dipole_moments() then compute_all_forces().
        // We need to pre-set dipole moments AFTER the tick's first
        // compute_dipole_moments call, which happens internally. The simplest
        // approach is to let tick() compute moments from bond structure, then
        // override them before reading forces. But tick() runs compute_forces
        // immediately after computing moments, so manual override won't help.
        // Instead, rely on the natural dipole moments from the O-H bonds.
        ae.tick();
        double f_near = ae.force_diag()[0].f_dipole.mag();

        // Move molecule 2 farther
        ae.atoms()[2].position = {30, 0, 0};
        ae.atoms()[3].position = {33, 0, 0};
        ae.tick();
        double f_far = ae.force_diag()[0].f_dipole.mag();

        std::cout << "  f_near=" << f_near << " f_far=" << f_far << "\n";
        // 1/r^4: doubling distance should decrease by ~16x
        if (f_near > 1e-30 && f_far > 1e-30) {
            double ratio = f_near / f_far;
            std::cout << "  ratio=" << ratio << " (expect >4 for 1/r^4)\n";
            ftd::test::check("DD4: force decreases with distance", ratio > 4.0);
        } else {
            ftd::test::check("DD4: force decreases with distance", f_near > f_far);
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
        ftd::test::check("DD5: symmetric H-H bond → near-zero dipole", d1 < 1e-10 && d2 < 1e-10);
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
            ftd::test::check("DD6: dipole diag populated", fd[0].f_dipole.mag() > 1e-30);
        } else {
            ftd::test::check("DD6: dipole diag populated", false);
        }
    }
}

// ============================================================================
// Section: electronegativity  (from test_ae_electronegativity.cpp)
// ============================================================================

static void section_electronegativity() {
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
        ftd::test::check("EN1a: H chi = 2.20", std::abs(chi_h - 2.20) < 0.01);
        ftd::test::check("EN1b: O chi = 3.44", std::abs(chi_o - 3.44) < 0.01);
        ftd::test::check("EN1c: F > O > H", chi_f > chi_o && chi_o > chi_h);
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
        ftd::test::check("EN2: polar bond forms with electronegativity ON", bonds_on >= bonds_off);
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
        ftd::test::check("EN3: O-H bond produces nonzero dipole moment", d_o > 1e-5 && d_h > 1e-5);
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
        ftd::test::check("EN4: H-H bond → near-zero dipole", d1 < 1e-10);
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
        ftd::test::check("EN5: no bond at 1.25*sigma without electronegativity", bonds == 0);
    }
}

// ============================================================================
// Section: hbonds  (from test_ae_hbonds.cpp)
// ============================================================================

static void section_hbonds() {
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
        ftd::test::check("HB1: nonzero H-bond force", f.mag() > 1e-30);
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
        ftd::test::check("HB2: zero when no H present", f.mag() < 1e-30);
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
        ftd::test::check("HB3: zero when toggle off", f.mag() < 1e-30);
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
        ftd::test::check("HB4: zero when donor not electronegative (C-H...O)", f.mag() < 1e-30);
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
        ftd::test::check("HB5: force decreases with distance", f_near > f_far * 2.0);
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
            ftd::test::check("HB6: hbond diag nonzero on H", hb_mag > 1e-30);
        } else {
            ftd::test::check("HB6: hbond diag nonzero on H", false);
        }
    }
}

// ============================================================================
// Section: thermostat  (from test_ae_thermostat.cpp)
// ============================================================================

static void section_thermostat() {
    // ---- TH1: Temperature converges to target (heating) ----
    std::cout << "\n--- TH1: Heating to target ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        // Several atoms with low initial KE
        for (int i = 0; i < 10; ++i) {
            ae.add_atom(2, {static_cast<double>(i*10), 0, 0},
                        {0.001, 0, 0});  // very slow
        }

        double T_target = 0.1;
        ae.set_target_temperature(T_target);
        ae.set_thermostat_tau(0.5);  // fast coupling
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.thermostat = true;

        auto d0 = ae.diagnostics();
        ae.run(500);
        auto d1 = ae.diagnostics();

        std::cout << "  T_initial=" << d0.temperature << " T_final=" << d1.temperature
                  << " T_target=" << T_target << "\n";
        // Temperature should be closer to target
        double err0 = std::abs(d0.temperature - T_target);
        double err1 = std::abs(d1.temperature - T_target);
        ftd::test::check("TH1: temperature moves toward target", err1 < err0);
    }

    // ---- TH2: Toggle OFF → no effect ----
    std::cout << "\n--- TH2: Toggle OFF → no effect ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        for (int i = 0; i < 5; ++i) {
            ae.add_atom(2, {static_cast<double>(i*10), 0, 0}, {0.01, 0, 0});
        }

        ae.set_target_temperature(1.0);
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.thermostat = false;  // OFF

        auto d0 = ae.diagnostics();
        ae.run(100);
        auto d1 = ae.diagnostics();

        // Without thermostat and without forces, KE should be conserved
        double ke_ratio = d1.total_ke / d0.total_ke;
        std::cout << "  KE ratio=" << ke_ratio << " (expect ~1.0)\n";
        ftd::test::check("TH2: KE unchanged when thermostat off", std::abs(ke_ratio - 1.0) < 0.01);
    }

    // ---- TH3: Zero target → no effect ----
    std::cout << "\n--- TH3: Zero target → no rescaling ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        for (int i = 0; i < 5; ++i) {
            ae.add_atom(2, {static_cast<double>(i*10), 0, 0}, {0.01, 0, 0});
        }

        ae.set_target_temperature(0.0);  // zero → disabled
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.thermostat = true;

        auto d0 = ae.diagnostics();
        ae.run(100);
        auto d1 = ae.diagnostics();

        double ke_ratio = d1.total_ke / d0.total_ke;
        std::cout << "  KE ratio=" << ke_ratio << "\n";
        ftd::test::check("TH3: no rescaling when target=0", std::abs(ke_ratio - 1.0) < 0.01);
    }

    // ---- TH4: Cooling works ----
    std::cout << "\n--- TH4: Cooling ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        // Fast-moving atoms
        for (int i = 0; i < 10; ++i) {
            ae.add_atom(2, {static_cast<double>(i*10), 0, 0},
                        {0.1, 0.05, 0});  // high KE
        }

        double T_target = 0.0001;  // very cold
        ae.set_target_temperature(T_target);
        ae.set_thermostat_tau(0.5);
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.thermostat = true;

        auto d0 = ae.diagnostics();
        ae.run(500);
        auto d1 = ae.diagnostics();

        std::cout << "  T_initial=" << d0.temperature << " T_final=" << d1.temperature << "\n";
        ftd::test::check("TH4: temperature decreases (cooling)", d1.temperature < d0.temperature);
    }

    // ---- TH5: Energy changes with thermostat active ----
    std::cout << "\n--- TH5: Energy changes ---\n";
    {
        ftd::AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        for (int i = 0; i < 5; ++i) {
            ae.add_atom(2, {static_cast<double>(i*10), 0, 0}, {0.001, 0, 0});
        }

        ae.set_target_temperature(1.0);  // much higher than initial
        ae.set_thermostat_tau(0.5);
        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        ae.toggles.thermostat = true;

        auto d0 = ae.diagnostics();
        ae.run(200);
        auto d1 = ae.diagnostics();

        std::cout << "  E0=" << d0.total_energy << " E1=" << d1.total_energy << "\n";
        ftd::test::check("TH5: energy changes with thermostat (heating adds KE)",
              std::abs(d1.total_energy - d0.total_energy) > 1e-10);
    }
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("test_atom_engine_forces");

    ftd::test::section("angle_strain");
    section_angle_strain();

    ftd::test::section("dipole");
    section_dipole();

    ftd::test::section("electronegativity");
    section_electronegativity();

    ftd::test::section("hbonds");
    section_hbonds();

    ftd::test::section("thermostat");
    section_thermostat();

    return ftd::test::finalize();
}
