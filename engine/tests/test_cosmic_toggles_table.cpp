/**
 * Test: CosmicToggles characterization (ticket 3.3)
 *
 * Pins the EXACT post-construction toggle state, enable_all()/minimal()
 * profiles, and the string get_toggle/set_toggle round-trip for CosmicEngine
 * — BEFORE the ADR-0013 table-driven refactor. CosmicToggles::validate() is
 * NEW in this ticket (Cosmic had none); its assertions are added in the same
 * commit that introduces it (see Section D below).
 *
 * Pure toggle-state characterization (no physics). Expected values are
 * hand-copied from cosmic_engine.h; the table-driven port must reproduce them.
 */

#include <iostream>
#include <string>
#include "ftd/cosmic_engine.h"

static int failures = 0;
static void check(const char* name, bool condition) {
    if (condition) { std::cout << "  PASS  " << name << "\n"; }
    else           { std::cout << "  FAIL  " << name << "\n"; ++failures; }
}

int main() {
    std::cout << "== CosmicToggles characterization ==\n";

    // ---- Section A: post-construction defaults (verbatim) ----
    {
        ftd::CosmicToggles t;
        check("default gravity=ON",              t.gravity             == true);
        check("default sph_gas=ON",              t.sph_gas             == true);
        check("default hubble_expansion=ON",     t.hubble_expansion    == true);
        check("default dark_energy=OFF",         t.dark_energy         == false);
        check("default dark_matter_halos=OFF",   t.dark_matter_halos   == false);
        check("default black_hole_accretion=OFF", t.black_hole_accretion == false);
        check("default cosmic_radiation=OFF",    t.cosmic_radiation    == false);
        check("default star_formation=OFF",      t.star_formation      == false);
        check("default stellar_evolution=OFF",   t.stellar_evolution   == false);
        check("default galaxy_mergers=OFF",      t.galaxy_mergers      == false);
        check("default magnetic_fields=OFF",     t.magnetic_fields     == false);
        check("default radiation_pressure=OFF",  t.radiation_pressure  == false);
        check("default relativistic_jets=OFF",   t.relativistic_jets   == false);
        check("default gravitational_waves=OFF", t.gravitational_waves == false);
    }

    // ---- Section B: enable_all() → every implemented toggle ON ----
    {
        ftd::CosmicToggles t;
        t.enable_all();
        check("enable_all gravity",              t.gravity             == true);
        check("enable_all sph_gas",              t.sph_gas             == true);
        check("enable_all hubble_expansion",     t.hubble_expansion    == true);
        check("enable_all dark_energy",          t.dark_energy         == true);
        check("enable_all rejects reserved dark_matter_halos", t.dark_matter_halos == false);
        check("enable_all black_hole_accretion", t.black_hole_accretion == true);
        check("enable_all rejects reserved cosmic_radiation", t.cosmic_radiation == false);
        check("enable_all star_formation",       t.star_formation      == true);
        check("enable_all stellar_evolution",    t.stellar_evolution   == true);
        check("enable_all rejects reserved galaxy_mergers", t.galaxy_mergers == false);
        check("enable_all magnetic_fields",      t.magnetic_fields     == true);
        check("enable_all radiation_pressure",   t.radiation_pressure  == true);
        check("enable_all relativistic_jets",    t.relativistic_jets   == true);
        check("enable_all gravitational_waves",  t.gravitational_waves == true);
    }

    // ---- Section C: minimal() profile (== constructor defaults for Cosmic) ----
    {
        ftd::CosmicToggles t;
        t.enable_all();   // dirty first
        t.minimal();
        check("minimal gravity=ON",              t.gravity             == true);
        check("minimal sph_gas=ON",              t.sph_gas             == true);
        check("minimal hubble_expansion=ON",     t.hubble_expansion    == true);
        check("minimal dark_energy=OFF",         t.dark_energy         == false);
        check("minimal dark_matter_halos=OFF",   t.dark_matter_halos   == false);
        check("minimal black_hole_accretion=OFF", t.black_hole_accretion == false);
        check("minimal cosmic_radiation=OFF",    t.cosmic_radiation    == false);
        check("minimal star_formation=OFF",      t.star_formation      == false);
        check("minimal stellar_evolution=OFF",   t.stellar_evolution   == false);
        check("minimal galaxy_mergers=OFF",      t.galaxy_mergers      == false);
        check("minimal magnetic_fields=OFF",     t.magnetic_fields     == false);
        check("minimal radiation_pressure=OFF",  t.radiation_pressure  == false);
        check("minimal relativistic_jets=OFF",   t.relativistic_jets   == false);
        check("minimal gravitational_waves=OFF", t.gravitational_waves == false);
    }

    // ---- Section D: validation rejects advertised terms with no tick phase. ----
    {
        ftd::CosmicToggles def;
        check("validate default → valid", def.validate() == true);
        ftd::CosmicToggles all; all.enable_all();
        check("validate implemented enable_all → valid", all.validate() == true);
        ftd::CosmicToggles none;
        none.gravity = none.sph_gas = none.hubble_expansion = false;
        check("validate all-off → valid", none.validate() == true);
        ftd::CosmicToggles unsupported;
        unsupported.dark_matter_halos = true;
        std::string error;
        check("validate reserved toggle → invalid",
              !unsupported.validate(&error) && error.find("not implemented") != std::string::npos);
    }

    // ---- Section E: string get/set round-trip via the ScaleEngine surface ----
    {
        const char* names[] = {
            "gravity", "sph_gas", "hubble_expansion", "dark_energy",
            "dark_matter_halos", "black_hole_accretion", "cosmic_radiation",
            "star_formation", "stellar_evolution", "galaxy_mergers",
            "magnetic_fields", "radiation_pressure", "relativistic_jets",
            "gravitational_waves"
        };
        ftd::CosmicEngine ce;
        check("get_toggle gravity default true",     ce.get_toggle("gravity") == true);
        check("get_toggle dark_energy default false", ce.get_toggle("dark_energy") == false);
        bool roundtrip_ok = true;
        for (const char* n : names) {
            const bool supported = ce.toggles.is_toggle_supported(n);
            const bool accepted = ce.try_set_toggle(n, true);
            if (accepted != supported || ce.get_toggle(n) != supported) roundtrip_ok = false;
            ce.set_toggle(n, false);
            if (ce.get_toggle(n) != false) roundtrip_ok = false;
        }
        check("set/get accepts only implemented names", roundtrip_ok);
        check("unknown get_toggle → false", ce.get_toggle("does_not_exist") == false);
        ce.set_toggle("gravity", true);
        ce.set_toggle("does_not_exist", true);
        check("unknown set_toggle no-op", ce.get_toggle("does_not_exist") == false &&
                                           ce.get_toggle("gravity") == true);
        std::string error;
        check("typed setter reports unknown toggle",
              !ce.try_set_toggle("does_not_exist", true, &error) && !error.empty());
    }

    std::cout << (failures == 0 ? "ALL PASS\n" : (std::to_string(failures) + " FAIL\n"));
    return failures;
}
