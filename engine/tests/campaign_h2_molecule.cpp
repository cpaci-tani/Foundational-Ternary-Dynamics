/**
 * Campaign: H2 Molecule Formation
 *
 * Two hydrogen atoms approach, form a covalent bond,
 * settle into vibrational equilibrium, and conserve energy.
 *
 * This is the minimal chemistry test for Scale 2.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/atom_engine.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition, const char* detail = "") {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        if (detail[0]) std::cout << "        " << detail << "\n";
        ++failures;
    }
}

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: H2 Molecule Formation\n";
    std::cout << "================================================================\n\n";

    using namespace ftd;

    AtomEngine ae;
    ae.set_damping_enabled(false);
    ae.set_bonding_enabled(true);
    ae.set_dt(0.001);

    // Place two H atoms within bonding range
    AtomicProperties h_props = compute_atomic_properties(1, 0);
    double sigma_h = h_props.vdw_sigma;

    // Start just inside bonding threshold (1.2 * sigma)
    double initial_sep = sigma_h * 1.1;
    ae.add_atom(1, {0.0, 0.0, 0.0});
    ae.add_atom(1, {initial_sep, 0.0, 0.0});

    std::cout << "H atom properties:\n";
    std::cout << "  mass     = " << h_props.mass << " MeV\n";
    std::cout << "  radius   = " << h_props.radius << "\n";
    std::cout << "  vdw_eps  = " << h_props.vdw_epsilon << "\n";
    std::cout << "  vdw_sig  = " << sigma_h << "\n";
    std::cout << "  initial separation = " << initial_sep << "\n\n";

    // ── Phase 1: Bond formation ─────────────────────────────────────
    std::cout << "--- Phase 1: Bond formation ---\n";
    ae.tick();  // First tick should trigger bonding
    int bonds_after_1 = ae.diagnostics().bond_count;
    check("bond formed on first tick", bonds_after_1 == 1);

    if (bonds_after_1 == 1) {
        double r_eq = ae.atoms()[0].bonds[0].r_eq;
        double k_bond = ae.atoms()[0].bonds[0].k_bond;
        std::cout << "  r_eq   = " << r_eq << "\n";
        std::cout << "  k_bond = " << k_bond << "\n";
    }

    // ── Phase 2: Vibrational equilibrium ────────────────────────────
    std::cout << "\n--- Phase 2: Vibrational dynamics (5000 ticks) ---\n";
    ae.run(5000);

    // Check atoms are still bonded
    check("bond persists after 5000 ticks", ae.diagnostics().bond_count == 1);

    // Check separation oscillates around r_eq
    Vec3 dr = ae.atoms()[1].position - ae.atoms()[0].position;
    double r_final = dr.mag();
    std::cout << "  final separation = " << r_final << "\n";
    if (bonds_after_1 == 1) {
        double r_eq = ae.atoms()[0].bonds[0].r_eq;
        // Should be within ~50% of r_eq (vibrating)
        check("separation near r_eq", std::abs(r_final - r_eq) < r_eq * 0.5);
    }

    // ── Phase 3: Energy conservation ────────────────────────────────
    std::cout << "\n--- Phase 3: Energy conservation ---\n";
    double E_before = ae.diagnostics().total_energy;
    ae.run(5000);
    double E_after = ae.diagnostics().total_energy;
    double dE = std::abs(E_after - E_before) / (std::abs(E_before) + 1e-30);
    std::cout << "  E_before = " << E_before << "\n";
    std::cout << "  E_after  = " << E_after << "\n";
    std::cout << "  dE/E     = " << std::scientific << dE << "\n";
    check("energy drift < 0.1%", dE < 1e-3);

    // ── Phase 4: Momentum conservation ──────────────────────────────
    std::cout << "\n--- Phase 4: Momentum conservation ---\n";
    Vec3 p = ae.diagnostics().total_momentum;
    double p_mag = p.mag();
    std::cout << "  |p| = " << std::scientific << p_mag << "\n";
    // Started with zero total momentum, should remain near zero
    check("momentum near zero", p_mag < 1e-10);

    // ── Phase 5: Diagnostics sanity ─────────────────────────────────
    std::cout << "\n--- Phase 5: Diagnostics ---\n";
    AtomDiagnostics d = ae.diagnostics();
    std::cout << "  atoms     = " << d.atom_count << "\n";
    std::cout << "  bonds     = " << d.bond_count << "\n";
    std::cout << "  KE        = " << d.total_ke << "\n";
    std::cout << "  PE (ionic) = " << d.total_pe_ionic << "\n";
    std::cout << "  PE (vdW)  = " << d.total_pe_vdw << "\n";
    std::cout << "  PE (bond) = " << d.total_pe_bond << "\n";
    std::cout << "  Total E   = " << d.total_energy << "\n";
    std::cout << "  Temp      = " << d.temperature << "\n";
    check("2 atoms", d.atom_count == 2);
    check("temperature > 0", d.temperature > 0.0);

    // ── Summary ─────────────────────────────────────────────────────
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  ALL CHECKS PASSED — H2 molecule formed and stable\n";
    } else {
        std::cout << "  " << failures << " CHECK(S) FAILED\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
