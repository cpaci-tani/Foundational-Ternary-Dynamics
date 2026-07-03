/**
 * Test: AtomToggles characterization (ticket 3.3)
 *
 * Pins the EXACT post-construction toggle state, enable_all()/minimal()
 * profiles, validate() verdicts, and the string get_toggle/set_toggle
 * round-trip for AtomEngine — BEFORE the ADR-0013 table-driven refactor.
 *
 * Pure toggle-state characterization (no force/physics). Expected values are
 * hand-copied from atom_engine.h; the table-driven port must reproduce them.
 */

#include <iostream>
#include <string>
#include "ftd/atom_engine.h"

static int failures = 0;
static void check(const char* name, bool condition) {
    if (condition) { std::cout << "  PASS  " << name << "\n"; }
    else           { std::cout << "  FAIL  " << name << "\n"; ++failures; }
}

int main() {
    std::cout << "== AtomToggles characterization ==\n";

    // ---- Section A: post-construction defaults (verbatim) ----
    {
        ftd::AtomToggles t;
        check("default ionic=ON",              t.ionic              == true);
        check("default van_der_waals=ON",      t.van_der_waals      == true);
        check("default covalent_bonds=ON",     t.covalent_bonds     == true);
        check("default auto_bonding=ON",       t.auto_bonding       == true);
        check("default damping=OFF",           t.damping            == false);
        check("default h_bonds=OFF",           t.h_bonds            == false);
        check("default dipole_dipole=OFF",     t.dipole_dipole      == false);
        check("default angle_strain=OFF",      t.angle_strain       == false);
        check("default torsional=OFF",         t.torsional          == false);
        check("default improper_torsional=OFF", t.improper_torsional == false);
        check("default thermostat=OFF",        t.thermostat         == false);
        check("default electronegativity=OFF", t.electronegativity  == false);
    }

    // ---- Section B: enable_all() → every toggle ON ----
    {
        ftd::AtomToggles t;
        t.enable_all();
        check("enable_all ionic",              t.ionic              == true);
        check("enable_all van_der_waals",      t.van_der_waals      == true);
        check("enable_all covalent_bonds",     t.covalent_bonds     == true);
        check("enable_all auto_bonding",       t.auto_bonding       == true);
        check("enable_all damping",            t.damping            == true);
        check("enable_all h_bonds",            t.h_bonds            == true);
        check("enable_all dipole_dipole",      t.dipole_dipole      == true);
        check("enable_all angle_strain",       t.angle_strain       == true);
        check("enable_all torsional",          t.torsional          == true);
        check("enable_all improper_torsional", t.improper_torsional == true);
        check("enable_all thermostat",         t.thermostat         == true);
        check("enable_all electronegativity",  t.electronegativity  == true);
    }

    // ---- Section C: minimal() profile (== constructor defaults for Atom) ----
    {
        ftd::AtomToggles t;
        t.enable_all();   // dirty first
        t.minimal();
        check("minimal ionic=ON",              t.ionic              == true);
        check("minimal van_der_waals=ON",      t.van_der_waals      == true);
        check("minimal covalent_bonds=ON",     t.covalent_bonds     == true);
        check("minimal auto_bonding=ON",       t.auto_bonding       == true);
        check("minimal damping=OFF",           t.damping            == false);
        check("minimal h_bonds=OFF",           t.h_bonds            == false);
        check("minimal dipole_dipole=OFF",     t.dipole_dipole      == false);
        check("minimal angle_strain=OFF",      t.angle_strain       == false);
        check("minimal torsional=OFF",         t.torsional          == false);
        check("minimal improper_torsional=OFF", t.improper_torsional == false);
        check("minimal thermostat=OFF",        t.thermostat         == false);
        check("minimal electronegativity=OFF", t.electronegativity  == false);
    }

    // ---- Section D: validate() verdicts (single AND-dependencies) ----
    {
        ftd::AtomToggles def;
        check("validate default → valid", def.validate() == true);

        ftd::AtomToggles a; a.covalent_bonds = false; a.angle_strain = true;
        check("angle_strain w/o covalent_bonds → invalid", a.validate() == false);

        ftd::AtomToggles to; to.covalent_bonds = false; to.torsional = true;
        check("torsional w/o covalent_bonds → invalid", to.validate() == false);

        ftd::AtomToggles ip; ip.covalent_bonds = false; ip.improper_torsional = true;
        check("improper_torsional w/o covalent_bonds → invalid", ip.validate() == false);

        ftd::AtomToggles th; th.damping = false; th.thermostat = true;
        check("thermostat w/o damping → invalid", th.validate() == false);

        ftd::AtomToggles dd; dd.electronegativity = false; dd.dipole_dipole = true;
        check("dipole_dipole w/o electronegativity → invalid", dd.validate() == false);

        ftd::AtomToggles a_ok; a_ok.covalent_bonds = true; a_ok.angle_strain = true;
        check("angle_strain w/ covalent_bonds → valid", a_ok.validate() == true);

        ftd::AtomToggles th_ok; th_ok.damping = true; th_ok.thermostat = true;
        check("thermostat w/ damping → valid", th_ok.validate() == true);

        ftd::AtomToggles dd_ok; dd_ok.electronegativity = true; dd_ok.dipole_dipole = true;
        check("dipole_dipole w/ electronegativity → valid", dd_ok.validate() == true);
    }

    // ---- Section E: string get/set round-trip via the ScaleEngine surface ----
    {
        const char* names[] = {
            "ionic", "van_der_waals", "covalent_bonds", "auto_bonding", "damping",
            "h_bonds", "dipole_dipole", "angle_strain", "torsional",
            "improper_torsional", "thermostat", "electronegativity"
        };
        ftd::AtomEngine ae;
        check("get_toggle ionic default true",   ae.get_toggle("ionic") == true);
        check("get_toggle damping default false", ae.get_toggle("damping") == false);
        bool roundtrip_ok = true;
        for (const char* n : names) {
            ae.set_toggle(n, true);
            if (ae.get_toggle(n) != true) roundtrip_ok = false;
            ae.set_toggle(n, false);
            if (ae.get_toggle(n) != false) roundtrip_ok = false;
        }
        check("set/get round-trip all 12 names", roundtrip_ok);
        check("unknown get_toggle → false", ae.get_toggle("does_not_exist") == false);
        ae.set_toggle("ionic", true);
        ae.set_toggle("does_not_exist", true);
        check("unknown set_toggle no-op", ae.get_toggle("does_not_exist") == false &&
                                          ae.get_toggle("ionic") == true);
    }

    std::cout << (failures == 0 ? "ALL PASS\n" : (std::to_string(failures) + " FAIL\n"));
    return failures;
}
