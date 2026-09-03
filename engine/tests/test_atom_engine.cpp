/**
 * Test: AtomEngine (Scale 2) unit tests
 *
 * Checks covering injection, properties, forces, bonding,
 * conservation laws, and integration.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
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
    std::cout << "  TEST: AtomEngine (Scale 2)\n";
    std::cout << "================================================================\n\n";

    using namespace ftd;

    // ── AE1: Atom injection ─────────────────────────────────────────
    {
        std::cout << "--- AE1: Atom injection ---\n";
        AtomEngine ae;
        int id = ae.add_atom(1, {10.0, 20.0, 30.0});
        check("id == 0", id == 0);
        check("Z stored correctly", ae.atoms()[0].Z == 1);
        check("position stored", ae.atoms()[0].position.x == 10.0);
    }

    // ── AE2: Atomic properties ──────────────────────────────────────
    {
        std::cout << "\n--- AE2: Atomic properties ---\n";
        AtomicProperties h = compute_atomic_properties(1, 0);
        check("H mass > 0", h.mass > 0.0);
        check("H radius > 0", h.radius > 0.0);
        check("H vdw_epsilon > 0", h.vdw_epsilon > 0.0);
        check("H vdw_sigma > 0", h.vdw_sigma > 0.0);
        check("H max_bonds == 1", h.max_bonds == 1);

        AtomicProperties c = compute_atomic_properties(6, 6);
        check("C max_bonds == 4", c.max_bonds == 4);
        check("C mass > H mass", c.mass > h.mass);
        check("C radius < H radius", c.radius < h.radius);  // heavier atom is smaller

        AtomicProperties li = compute_atomic_properties(3, 4);
        AtomicProperties ne = compute_atomic_properties(10, 10);
        AtomicProperties na = compute_atomic_properties(11, 12);
        AtomicClosureContext fe_ctx = compute_atomic_closure_context(26);
        AtomicClosureContext ce_ctx = compute_atomic_closure_context(58);
        AtomicClosureContext u_ctx  = compute_atomic_closure_context(92);

        check("H closure Z stored", h.closure_context.Z == 1);
        check("H closure n_shell == 1", h.closure_context.n_shell == 1);
        check_close("H closure r_cloud == R_BOHR",
                    h.closure_context.r_cloud, R_BOHR, R_BOHR * 1e-12);
        check("Li simulation radius < H simulation radius", li.radius < h.radius);
        check("Li closure cloud > H closure cloud (new shell)",
              li.closure_context.r_cloud > h.closure_context.r_cloud);
        check("Period 2 closure cloud contracts Li > C > Ne",
              li.closure_context.r_cloud > c.closure_context.r_cloud &&
              c.closure_context.r_cloud > ne.closure_context.r_cloud);
        check("Na closure cloud > Ne closure cloud (period reset)",
              na.closure_context.r_cloud > ne.closure_context.r_cloud);
        check("Ne closure regime == ShellClosed",
              ne.closure_context.regime == AtomicClosureRegime::ShellClosed);
        check("Fe closure regime == TransitionBlock",
              fe_ctx.regime == AtomicClosureRegime::TransitionBlock);
        check("Ce closure regime == Lanthanide",
              ce_ctx.regime == AtomicClosureRegime::Lanthanide);
        check("U closure regime == Actinide",
              u_ctx.regime == AtomicClosureRegime::Actinide);

        AtomicClosureConfig scaled_cfg;
        scaled_cfg.lattice_spacing = 2.0;
        scaled_cfg.box_extent = 1024.0;
        scaled_cfg.tau_reference = 10.0;
        AtomicClosureContext h_scaled = compute_atomic_closure_context(1, scaled_cfg);
        check_close("closure kappa = r_cloud/a",
                    h_scaled.kappa, h_scaled.r_cloud / 2.0, h_scaled.r_cloud * 1e-12);
        check_close("closure zeta = r_cloud/L",
                    h_scaled.zeta, h_scaled.r_cloud / 1024.0, h_scaled.r_cloud * 1e-12);
    }

    // ── AE3: Free atom (no forces) ─────────────────────────────────
    {
        std::cout << "\n--- AE3: Free atom drift ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_dt(0.01);
        // Use velocity well below C_SPEED (= 1/√3 ≈ 0.577) to avoid speed clamping
        Vec3 v0 = {0.1, 0.0, 0.0};
        ae.add_atom(2, {0.0, 0.0, 0.0}, v0);  // single He (noble, no bonds)
        double x0 = ae.atoms()[0].position.x;
        ae.run(100);
        double x1 = ae.atoms()[0].position.x;
        double expected = x0 + v0.x * 100 * 0.01;
        check_close("x = x0 + v*t", x1, expected, std::abs(expected) * 0.001 + 1e-10);
    }

    // ── AE4: Ionic attraction (Na+ ← Cl-) ──────────────────────────
    {
        std::cout << "\n--- AE4: Ionic attraction ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Use separation >> vdW sigma so Coulomb 1/r² dominates LJ 1/r⁷
        double sigma_na = compute_atomic_properties(11).vdw_sigma;
        double sep = sigma_na * 5.0;  // far beyond LJ range
        ae.add_atom(11, {0.0, 0.0, 0.0}, {}, +1);    // Na+
        ae.add_atom(17, {sep, 0.0, 0.0}, {}, -1);     // Cl-
        Vec3 f = ae.compute_force(0);
        std::cout << "        sigma_na=" << sigma_na << " sep=" << sep << " f.x=" << f.x << "\n";
        check("Na+ force points toward Cl-", f.x > 0.0);
    }

    // ── AE5: Ionic repulsion (Na+ → Na+) ───────────────────────────
    {
        std::cout << "\n--- AE5: Ionic repulsion ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Use separation >> vdW sigma so Coulomb dominates
        double sigma_na = compute_atomic_properties(11).vdw_sigma;
        double sep = sigma_na * 5.0;
        ae.add_atom(11, {0.0, 0.0, 0.0}, {}, +1);    // Na+
        ae.add_atom(11, {sep, 0.0, 0.0}, {}, +1);     // Na+
        Vec3 f = ae.compute_force(0);
        std::cout << "        sep=" << sep << " f.x=" << f.x << "\n";
        check("Na+ force points away from Na+", f.x < 0.0);
    }

    // ── AE6: vdW attraction (long range) ────────────────────────────
    {
        std::cout << "\n--- AE6: vdW attraction (long range) ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Two neutral noble gas atoms far apart (beyond sigma)
        double sigma_he = compute_atomic_properties(2, 2).vdw_sigma;
        double r_far = sigma_he * 2.0;  // well beyond sigma → attractive regime
        ae.add_atom(2, {0.0, 0.0, 0.0});  // He
        ae.add_atom(2, {r_far, 0.0, 0.0});  // He
        Vec3 f = ae.compute_force(0);
        check("vdW force toward partner at long range", f.x > 0.0);
    }

    // ── AE7: vdW repulsion (short range) ────────────────────────────
    {
        std::cout << "\n--- AE7: vdW repulsion (short range) ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Two noble atoms very close (inside sigma)
        double sigma_he = compute_atomic_properties(2, 2).vdw_sigma;
        double r_close = sigma_he * 0.8;
        ae.add_atom(2, {0.0, 0.0, 0.0});  // He
        ae.add_atom(2, {r_close, 0.0, 0.0});  // He
        Vec3 f = ae.compute_force(0);
        check("vdW force away from partner at short range", f.x < 0.0);
    }

    // ── AE8: Bond spring force ──────────────────────────────────────
    {
        std::cout << "\n--- AE8: Bond spring force ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        int id_a = ae.add_atom(1, {0.0, 0.0, 0.0});   // H
        // Place partner at 1.5 * r_eq (stretched bond, in vdW attractive regime)
        double sigma_h = compute_atomic_properties(1, 0).vdw_sigma;
        double stretch = sigma_h * 1.5;  // r_eq ≈ sigma, so this is 50% stretched
        int id_b = ae.add_atom(1, {stretch, 0.0, 0.0});
        ae.create_bond(id_a, id_b, 1);

        double r_eq = ae.atoms()[0].bonds[0].r_eq;
        double k = ae.atoms()[0].bonds[0].k_bond;

        Vec3 f = ae.compute_force(0);
        std::cout << "        sigma_h=" << sigma_h << " stretch=" << stretch
                  << " r_eq=" << r_eq << " f.x=" << f.x << "\n";
        // Bond stretched (r > r_eq) → restoring force toward partner
        check("bond restoring force toward partner", f.x > 0.0);
        // Force magnitude: bond contributes -k*(r-r_eq) which is attractive
        double expected_bond_f = k * (stretch - r_eq);
        check("bond force reasonable", expected_bond_f > 0.0);
    }

    // ── AE9: Bond formation ─────────────────────────────────────────
    {
        std::cout << "\n--- AE9: Bond formation ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(true);
        double sigma_h = compute_atomic_properties(1, 0).vdw_sigma;
        double r_bond = sigma_h * 1.0;  // within 1.2 * sigma → should bond
        ae.add_atom(1, {0.0, 0.0, 0.0});
        ae.add_atom(1, {r_bond, 0.0, 0.0});
        ae.tick();  // bonding check happens during tick
        check("bond formed between H atoms",
              ae.atoms()[0].bonds.size() == 1 && ae.atoms()[1].bonds.size() == 1);
    }

    // ── AE10: Speed limit ───────────────────────────────────────────
    {
        std::cout << "\n--- AE10: Speed limit ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        Vec3 fast = {10.0, 10.0, 10.0};
        ae.add_atom(1, {0.0, 0.0, 0.0}, fast);
        ae.tick();
        double v = ae.atoms()[0].velocity.mag();
        check("speed clamped to C_SPEED", v <= C_SPEED + 1e-12);
    }

    // ── AE11: Energy conservation (vdW, no damping) ─────────────────
    {
        std::cout << "\n--- AE11: Energy conservation ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        ae.set_dt(0.001);  // small dt for accuracy

        // Two He atoms with initial velocity, no bonds
        double sigma_he = compute_atomic_properties(2, 2).vdw_sigma;
        ae.add_atom(2, {0.0, 0.0, 0.0}, {0.001, 0.0, 0.0});
        ae.add_atom(2, {sigma_he * 3.0, 0.0, 0.0}, {-0.001, 0.0, 0.0});

        // Let it settle for a few ticks, then measure
        ae.run(10);
        double E0 = ae.diagnostics().total_energy;
        ae.run(1000);
        double E1 = ae.diagnostics().total_energy;
        double dE = std::abs(E1 - E0) / (std::abs(E0) + 1e-30);
        check("energy conserved < 0.01%", dE < 1e-4);
        std::cout << "        dE/E = " << std::scientific << dE << "\n";
    }

    // ── AE12: Momentum conservation ─────────────────────────────────
    {
        std::cout << "\n--- AE12: Momentum conservation ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        ae.set_dt(0.001);

        double sigma_he = compute_atomic_properties(2, 2).vdw_sigma;
        ae.add_atom(2, {0.0, 0.0, 0.0}, {0.001, 0.0, 0.0});
        ae.add_atom(2, {sigma_he * 3.0, 0.0, 0.0}, {-0.001, 0.0, 0.0});

        ae.run(10);
        Vec3 p0 = ae.diagnostics().total_momentum;
        ae.run(1000);
        Vec3 p1 = ae.diagnostics().total_momentum;
        double dp = (p1 - p0).mag();
        double p_mag = p0.mag() + 1e-30;
        check("momentum conserved", dp / p_mag < 1e-4 || dp < 1e-15);
        std::cout << "        |dp| = " << std::scientific << dp << "\n";
    }

    // ── AE13: No NaN at soft limit ──────────────────────────────────
    {
        std::cout << "\n--- AE13: No NaN at soft limit ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        // Two atoms at same position
        ae.add_atom(1, {0.0, 0.0, 0.0});
        ae.add_atom(1, {0.0, 0.0, 0.0});
        Vec3 f = ae.compute_force(0);
        bool finite = std::isfinite(f.x) && std::isfinite(f.y) && std::isfinite(f.z);
        check("force is finite at r=0", finite);
    }

    // ── AE14: Temperature calc ──────────────────────────────────────
    {
        std::cout << "\n--- AE14: Temperature calculation ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        Vec3 v = {0.1, 0.0, 0.0};
        ae.add_atom(2, {0.0, 0.0, 0.0}, v);

        AtomDiagnostics d = ae.diagnostics();
        double mass = ae.atoms()[0].mass;
        double expected_T = 2.0 * (0.5 * mass * v.mag2()) / 3.0;
        check_close("T = 2*KE/(3*N)", d.temperature, expected_T, expected_T * 0.01);
    }

    // ── AE14a: Finite-domain input guards ──────────────────────────
    {
        std::cout << "\n--- AE14a: Finite-domain input guards ---\n";
        AtomEngine ae;
        bool rejected_dt = false, rejected_atom = false;
        try { ae.set_dt(std::numeric_limits<double>::quiet_NaN()); }
        catch (const std::invalid_argument&) { rejected_dt = true; }
        try { ae.add_atom(1, {std::numeric_limits<double>::infinity(), 0.0, 0.0}); }
        catch (const std::invalid_argument&) { rejected_atom = true; }
        check("non-finite dt rejected", rejected_dt);
        check("non-finite atom position rejected", rejected_atom);
        std::string state_error;
        check("fresh AtomEngine state validates", ae.validate_state(&state_error));
    }

    // ── AE15: OnticEntity conversion ────────────────────────────────
    {
        std::cout << "\n--- AE14b: Diagnostics follow active model ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.set_damping_enabled(false);
        ae.add_atom(1, {0.0, 0.0, 0.0}, {}, 1);
        ae.add_atom(1, {3.0, 0.0, 0.0}, {}, -1);

        // Fractional charge is force-authoritative; formal charge remains
        // unchanged so this catches accidental fallback to Atom::charge.
        ae.atoms()[0].q_frac = 0.25;
        ae.atoms()[1].q_frac = -0.5;
        AtomDiagnostics active = ae.diagnostics();
        const double r = std::sqrt(9.0 + ae.softening() * ae.softening());
        const double expected_ionic = ALPHA * 0.25 * -0.5 / (4.0 * PI * r);
        check_close("ionic PE uses q_frac", active.total_pe_ionic,
                    expected_ionic, std::abs(expected_ionic) * 1e-12 + 1e-18);
        check("default tracked accounting complete", active.energy_complete);
        check("static no-bonding profile conservative", active.energy_conservative);

        ae.toggles.ionic = false;
        ae.toggles.van_der_waals = false;
        AtomDiagnostics disabled = ae.diagnostics();
        check("disabled ionic PE is zero", disabled.total_pe_ionic == 0.0);
        check("disabled vdW PE is zero", disabled.total_pe_vdw == 0.0);

        check("bond fixture created", ae.create_bond(0, 1));
        check("enabled bond PE is tracked", ae.diagnostics().total_pe_bond > 0.0);
        ae.toggles.covalent_bonds = false;
        check("disabled bond PE is zero", ae.diagnostics().total_pe_bond == 0.0);

        AtomEngine angle_ae;
        angle_ae.set_bonding_enabled(false);
        angle_ae.set_damping_enabled(false);
        angle_ae.toggles.ionic = false;
        angle_ae.toggles.van_der_waals = false;
        angle_ae.toggles.covalent_bonds = false;
        const int center = angle_ae.add_atom(8, {0.0, 0.0, 0.0});
        const int t1 = angle_ae.add_atom(1, {3.0, 0.0, 0.0});
        const int t2 = angle_ae.add_atom(1, {0.0, 3.0, 0.0});
        check("angle fixture bond 1", angle_ae.create_bond(center, t1));
        check("angle fixture bond 2", angle_ae.create_bond(center, t2));
        angle_ae.toggles.angle_strain = true;
        AtomDiagnostics angle_diag = angle_ae.diagnostics();
        check("angle PE is tracked", angle_diag.total_pe_angle > 0.0);
        check("tracked angle accounting is complete", angle_diag.energy_complete);
        check("static angle profile can claim conservation", angle_diag.energy_conservative);
    }

    // ── AE15: OnticEntity conversion ────────────────────────────────
    {
        std::cout << "\n--- AE15: OnticEntity conversion ---\n";
        AtomEngine ae;
        ae.add_atom(6, {0.0, 0.0, 0.0});
        OnticEntity oe = ae.atoms()[0].as_ontic();
        check("state == Z", oe.state == 6);
        check("energy > 0", oe.energy > 0.0);
        check("boundary > 0", oe.boundary > 0.0);
    }

    // ── AE16: Locked atom immobile ──────────────────────────────────
    {
        std::cout << "\n--- AE16: Locked atom immobile ---\n";
        AtomEngine ae;
        ae.set_bonding_enabled(false);
        ae.add_locked_atom(1, {50.0, 50.0, 50.0});
        ae.add_atom(1, {55.0, 50.0, 50.0}, {0.01, 0.0, 0.0});
        // A stale constrained velocity must remain visible in total KE but
        // cannot contaminate the free-particle temperature.
        ae.atoms()[0].velocity = {10.0, 0.0, 0.0};
        AtomDiagnostics constrained = ae.diagnostics();
        const double mobile_ke = 0.5 * ae.atoms()[1].mass * 0.01 * 0.01;
        check_close("locked KE excluded from temperature", constrained.temperature,
                    2.0 * mobile_ke / 3.0, 1e-15);
        check("locked KE remains in total energy", constrained.total_ke > mobile_ke);
        Vec3 pos0 = ae.atoms()[0].position;
        ae.run(100);
        Vec3 pos1 = ae.atoms()[0].position;
        double drift = (pos1 - pos0).mag();
        check("locked atom didn't move", drift < 1e-15);
    }

    // ── AE18: Atomic state vector A_atom extensions ─────────────────────────────
    {
        std::cout << "\n--- AE18: Atomic state vector A_atom extensions ---\n";
        AtomicProperties pC = compute_atomic_properties(6, 6); // Carbon
        check("Carbon alpha_pol is reasonable (> 0)", pC.alpha_pol > 0.0);
        check("Carbon e_ion is reasonable (> 0)", pC.e_ion > 0.0);
        check("Carbon sigma_scatter is reasonable (> 0)", pC.sigma_scatter > 0.0);

        AtomicProperties pH = compute_atomic_properties(1, 0); // Hydrogen
        check("Hydrogen e_ion is distinct from Carbon e_ion", pH.e_ion != pC.e_ion);
    }

    // ── Summary ─────────────────────────────────────────────────────
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  ALL CHECKS PASSED\n";
    } else {
        std::cout << "  " << failures << " CHECK(S) FAILED\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
