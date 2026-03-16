/**
 * Test: AE Thermostat (Berendsen velocity rescaling)
 *
 * Verifies that the thermostat regulates temperature toward a target.
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
    std::cout << "  TEST: AE Thermostat (Berendsen)\n";
    std::cout << "================================================================\n";

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
        check("TH1: temperature moves toward target", err1 < err0);
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
        check("TH2: KE unchanged when thermostat off", std::abs(ke_ratio - 1.0) < 0.01);
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
        check("TH3: no rescaling when target=0", std::abs(ke_ratio - 1.0) < 0.01);
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
        check("TH4: temperature decreases (cooling)", d1.temperature < d0.temperature);
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
        check("TH5: energy changes with thermostat (heating adds KE)",
              std::abs(d1.total_energy - d0.total_energy) > 1e-10);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All thermostat tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";
    return failures;
}
