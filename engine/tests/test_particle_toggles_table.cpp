/**
 * Test: ParticleToggles characterization (ticket 3.3)
 *
 * Pins the EXACT post-construction toggle state, enable_all()/minimal()
 * profiles, validate() verdicts, and the string get_toggle/set_toggle
 * round-trip for ParticleEngine — BEFORE the ADR-0013 table-driven refactor.
 *
 * This is a pure toggle-state characterization: it never computes a force or
 * touches physics, so it is independent of the pre-existing G_PE-scale
 * physics failures in test_particle_toggles (FTD-0131; see CHECKLIST_ENGINE).
 * Expected values are hand-copied from particle_engine.h at refactor time; if
 * the table-driven port changes any of them the refactor is wrong.
 */

#include <iostream>
#include <string>
#include "ftd/particle_engine.h"

static int failures = 0;
static void check(const char* name, bool condition) {
    if (condition) { std::cout << "  PASS  " << name << "\n"; }
    else           { std::cout << "  FAIL  " << name << "\n"; ++failures; }
}

int main() {
    std::cout << "== ParticleToggles characterization ==\n";

    // ---- Section A: post-construction defaults (verbatim) ----
    {
        ftd::ParticleToggles t;
        check("default coulomb=ON",             t.coulomb            == true);
        check("default gravity=ON",             t.gravity            == true);
        check("default damping=ON",             t.damping            == true);
        check("default lorentz=OFF",            t.lorentz            == false);
        check("default exchange=OFF",           t.exchange           == false);
        check("default strong=OFF",             t.strong             == false);
        check("default radiation=OFF",          t.radiation          == false);
        check("default spin_orbit=OFF",         t.spin_orbit         == false);
        check("default relativistic=OFF",       t.relativistic       == false);
        check("default magnetic_dipole=OFF",    t.magnetic_dipole    == false);
        check("default relativistic_verlet=OFF", t.relativistic_verlet == false);
    }

    // ---- Section B: enable_all() → every toggle ON ----
    {
        ftd::ParticleToggles t;
        t.enable_all();
        check("enable_all coulomb",             t.coulomb            == true);
        check("enable_all gravity",             t.gravity            == true);
        check("enable_all damping",             t.damping            == true);
        check("enable_all lorentz",             t.lorentz            == true);
        check("enable_all exchange",            t.exchange           == true);
        check("enable_all strong",              t.strong             == true);
        check("enable_all radiation",           t.radiation          == true);
        check("enable_all spin_orbit",          t.spin_orbit         == true);
        check("enable_all relativistic",        t.relativistic       == true);
        check("enable_all magnetic_dipole",     t.magnetic_dipole    == true);
        check("enable_all relativistic_verlet", t.relativistic_verlet == true);
    }

    // ---- Section C: minimal() profile (NOTE: relativistic_verlet=ON,
    //      which DIFFERS from the constructor default of OFF) ----
    {
        ftd::ParticleToggles t;
        t.enable_all();   // dirty first so minimal() must clear the extras
        t.minimal();
        check("minimal coulomb=ON",              t.coulomb            == true);
        check("minimal gravity=ON",              t.gravity            == true);
        check("minimal damping=ON",              t.damping            == true);
        check("minimal relativistic_verlet=ON",  t.relativistic_verlet == true);
        check("minimal lorentz=OFF",             t.lorentz            == false);
        check("minimal exchange=OFF",            t.exchange           == false);
        check("minimal strong=OFF",              t.strong             == false);
        check("minimal radiation=OFF",           t.radiation          == false);
        check("minimal spin_orbit=OFF",          t.spin_orbit         == false);
        check("minimal relativistic=OFF",        t.relativistic       == false);
        check("minimal magnetic_dipole=OFF",     t.magnetic_dipole    == false);
    }

    // ---- Section D: validate() verdicts (OR-dependency on coulomb|gravity) ----
    {
        std::string err;
        ftd::ParticleToggles def;
        check("validate default → valid", def.validate(&err) == true);

        ftd::ParticleToggles so; so.coulomb = false; so.gravity = false; so.spin_orbit = true;
        check("spin_orbit w/o coulomb|gravity → invalid", so.validate() == false);

        ftd::ParticleToggles so_ok; so_ok.coulomb = true; so_ok.gravity = false; so_ok.spin_orbit = true;
        check("spin_orbit w/ coulomb → valid", so_ok.validate() == true);

        ftd::ParticleToggles md; md.coulomb = false; md.gravity = false; md.magnetic_dipole = true;
        check("magnetic_dipole w/o coulomb|gravity → invalid", md.validate() == false);

        ftd::ParticleToggles md_ok; md_ok.coulomb = false; md_ok.gravity = true; md_ok.magnetic_dipole = true;
        check("magnetic_dipole w/ gravity → valid", md_ok.validate() == true);
    }

    // ---- Section E: string get/set round-trip via the ScaleEngine surface ----
    {
        const char* names[] = {
            "coulomb", "gravity", "damping", "lorentz", "exchange", "strong",
            "radiation", "spin_orbit", "relativistic", "magnetic_dipole",
            "relativistic_verlet"
        };
        ftd::ParticleEngine pe;
        // Defaults visible through get_toggle.
        check("get_toggle coulomb default true",  pe.get_toggle("coulomb") == true);
        check("get_toggle lorentz default false", pe.get_toggle("lorentz") == false);
        // Round-trip every known name.
        bool roundtrip_ok = true;
        for (const char* n : names) {
            pe.set_toggle(n, true);
            if (pe.get_toggle(n) != true) roundtrip_ok = false;
            pe.set_toggle(n, false);
            if (pe.get_toggle(n) != false) roundtrip_ok = false;
        }
        check("set/get round-trip all 11 names", roundtrip_ok);
        // Unknown name is inert.
        check("unknown get_toggle → false", pe.get_toggle("does_not_exist") == false);
        pe.set_toggle("coulomb", true);
        pe.set_toggle("does_not_exist", true);      // must not throw / must not corrupt
        check("unknown set_toggle no-op", pe.get_toggle("does_not_exist") == false &&
                                          pe.get_toggle("coulomb") == true);
    }

    std::cout << (failures == 0 ? "ALL PASS\n" : (std::to_string(failures) + " FAIL\n"));
    return failures;
}
