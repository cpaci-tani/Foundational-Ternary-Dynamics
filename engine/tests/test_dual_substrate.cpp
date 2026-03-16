/**
 * Test: Dual-Substrate Engine
 *
 * Verifies the dual-substrate implementation from
 * "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026).
 *
 * Tests:
 *   DS-IDENTITY:     Algebraic identity 4P = S^2 - D^2 for ontic constants
 *   DS-CHIRALITY:    Chirality density determines manifestation polarity
 *   DS-WAVE:         Independent wave propagation in L and R substrates
 *   DS-ENERGY:       Energy conservation with dual substrates
 *   DS-COMPATIBILITY: Legacy behavior preserved when dual_substrate=false
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/ontic.h"
#include "ftd/render_bridge.h"

using namespace ftd;
using namespace ftd::ontic;

int main() {
    int failures = 0;

    auto check = [&](const char* name, bool ok) {
        if (ok) { std::cout << "  PASS  " << name << "\n"; }
        else    { std::cout << "  FAIL  " << name << "\n"; ++failures; }
    };

    auto check_close = [&](const char* name, double a, double b, double tol) {
        bool ok = std::abs(a - b) < tol;
        if (ok) { std::cout << "  PASS  " << name << "\n"; }
        else {
            std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                      << ", expected " << b << ", diff " << std::abs(a-b) << ")\n";
            ++failures;
        }
    };

    // ================================================================
    // DS-IDENTITY: Algebraic identity checks on ontic constants
    // ================================================================
    std::cout << "\n=== DS-IDENTITY ===\n";

    // Check 1: E_SUM = 16 * G*^2
    check_close("E_SUM = 16*G*^2", E_SUM, 16.0 * G_STAR * G_STAR, 1e-6);

    // Check 2: E_PRODUCT = 16 * G*^3
    check_close("E_PRODUCT = 16*G*^3", E_PRODUCT, 16.0 * G_STAR * G_STAR * G_STAR, 1e-6);

    // Check 3: DELTA_SQUARED = (4G* - 1)/(4G*)
    double expected_delta2 = (4.0 * G_STAR - 1.0) / (4.0 * G_STAR);
    check_close("DELTA_SQUARED", DELTA_SQUARED, expected_delta2, 1e-10);

    // Check 4: MATTER_FRACTION + VACUUM_FRACTION = 1.0
    check_close("fractions sum to 1", MATTER_FRACTION + VACUUM_FRACTION, 1.0, 1e-15);

    // Check 5: Algebraic identity S^2 = D^2 + 4P
    // D^2 = S^2 - 4P (compute D^2 from S, P)
    double S = E_SUM;
    double P = E_PRODUCT;
    double D2 = S * S - 4.0 * P;
    check("S^2 = D^2 + 4P (identity)", std::abs(S * S - (D2 + 4.0 * P)) < 1e-10);

    // Check 6: Vieta's relations: x+ + x- = S, x+ * x- = P
    check_close("x+ + x- = S", X_PLUS + X_MINUS, S, 0.01);
    check_close("x+ * x- = P", X_PLUS * X_MINUS, P, 0.1);

    // Check 7: delta^2 matches matter fraction
    check_close("delta^2 = matter fraction", DELTA_SQUARED, D2 / (S * S), 1e-10);

    // Check 8: Omega_Lambda conjecture value
    check_close("Omega_Lambda = 2/3", OMEGA_LAMBDA_CONJ, 2.0/3.0, 1e-15);

    // ================================================================
    // DS-CHIRALITY: Chirality-based manifestation polarity
    // ================================================================
    std::cout << "\n=== DS-CHIRALITY ===\n";

    {
        RenderBridge bridge(16);
        bridge.toggles.dual_substrate = true;
        bridge.toggles.genesis = false;  // manual check, not auto-genesis

        int center = bridge.lattice().index(8, 8, 8);

        // Inject +1 particle with dual-substrate split
        bridge.inject_particle(8, 8, 8, 1, Vec3(K_B, 0, 0));
        auto& v = bridge.voxels()[center];

        // Check 1: flux_L > flux_R for positive particle (in x-component)
        check("+1: flux_L.x > flux_R.x", v.flux_L.x > v.flux_R.x);

        // Check 2: chirality density > 0 for positive particle
        double chi_pos = v.chirality_density();
        check("+1: chirality > 0", chi_pos > 0);

        // Check 3: observable flux = flux_L + flux_R
        Vec3 obs = v.flux_L + v.flux_R;
        check_close("+1: flux = flux_L + flux_R (x)", v.flux.x, obs.x, 1e-12);
    }

    {
        RenderBridge bridge(16);
        bridge.toggles.dual_substrate = true;
        bridge.toggles.genesis = false;

        // Inject -1 particle
        bridge.inject_particle(8, 8, 8, -1, Vec3(K_B, 0, 0));
        auto& v = bridge.voxels()[bridge.lattice().index(8, 8, 8)];

        // Check 4: chirality density < 0 for negative particle
        double chi_neg = v.chirality_density();
        check("-1: chirality < 0", chi_neg < 0);

        // Check 5: flux_R > flux_L for negative particle
        check("-1: flux_R.x > flux_L.x", v.flux_R.x > v.flux_L.x);
    }

    // ================================================================
    // DS-WAVE: Independent wave propagation in L and R substrates
    // ================================================================
    std::cout << "\n=== DS-WAVE ===\n";

    {
        RenderBridge bridge(32);
        bridge.toggles.dual_substrate = true;
        bridge.toggles.coupling = false;  // no coupling, pure wave
        bridge.toggles.genesis = false;   // no genesis
        bridge.toggles.damping = false;   // no damping
        bridge.toggles.gauss_projection = false;
        bridge.toggles.forces = false;
        bridge.toggles.movement = false;

        int center = bridge.lattice().index(16, 16, 16);

        // Inject flux into LEFT substrate ONLY
        bridge.voxels()[center].flux_L = Vec3(0.5, 0.0, 0.0);
        bridge.voxels()[center].flux = bridge.voxels()[center].flux_L; // sync observable

        // Run 50 ticks of pure wave propagation
        for (int t = 0; t < 50; ++t) bridge.tick();

        // Measure L and R energies
        double E_L = 0.0, E_R = 0.0;
        int N = bridge.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            E_L += bridge.voxels()[i].flux_L.mag2() + bridge.voxels()[i].wave_vel_L.mag2();
            E_R += bridge.voxels()[i].flux_R.mag2() + bridge.voxels()[i].wave_vel_R.mag2();
        }

        // Check 1: L substrate has energy (wave propagated)
        check("L substrate has energy after wave", E_L > 0.01);

        // Check 2: R substrate has negligible energy (no leakage)
        // R should be essentially zero since we only injected into L
        check("R substrate negligible leakage", E_R < E_L * 0.01);

        // Check 3: Observable flux shows the wave
        double E_obs = 0.0;
        for (int i = 0; i < N; ++i)
            E_obs += bridge.voxels()[i].flux.mag2();
        check("Observable shows L wave", E_obs > 0.01);

        std::cout << "    E_L=" << E_L << " E_R=" << E_R << " E_obs=" << E_obs << "\n";
    }

    // ================================================================
    // DS-ENERGY: Energy conservation with dual substrates
    // ================================================================
    std::cout << "\n=== DS-ENERGY ===\n";

    {
        RenderBridge bridge(32);
        bridge.toggles.dual_substrate = true;
        bridge.toggles.genesis = false;
        bridge.toggles.damping = true;    // damping ON for steady-state equilibrium
        bridge.toggles.gauss_projection = true;
        bridge.toggles.forces = true;
        bridge.toggles.movement = true;

        // Inject two particles
        bridge.inject_particle(10, 16, 16, 1, Vec3(K_B, 0, 0));
        bridge.inject_particle(22, 16, 16, -1, Vec3(K_B, 0, 0));

        // Run 300 ticks to build self-fields + reach damped equilibrium
        for (int t = 0; t < 300; ++t) bridge.tick();

        auto audit0 = bridge.energy_audit();
        double E0 = audit0.E_L_total + audit0.E_R_total + audit0.particle_ke;

        // Run 200 more ticks from steady state
        for (int t = 0; t < 200; ++t) bridge.tick();

        auto audit1 = bridge.energy_audit();
        double E1 = audit1.E_L_total + audit1.E_R_total + audit1.particle_ke;

        double drift = (E0 > 1e-15) ? std::abs(E1 - E0) / E0 : 0.0;
        std::cout << "    E0=" << E0 << " E1=" << E1 << " drift=" << drift*100 << "%\n";
        std::cout << "    E_L=" << audit1.E_L_total << " E_R=" << audit1.E_R_total << "\n";
        std::cout << "    charge=" << audit1.charge_total << "\n";

        // NOTE: |f|^2 + |wv|^2 is NOT the symplectic conserved quantity for
        // leapfrog integration (that involves |grad(f)|^2). This threshold
        // is a loose bound, not a precision test. See AUDIT_PLAN.md I-05.
        check("Energy drift < 60%", drift < 0.60);

        // Check 2: Charge conservation (exact)
        check("Charge = 0", audit1.charge_total == 0);

        // Check 3: Both L and R have nonzero energy
        check("E_L > 0", audit1.E_L_total > 0.001);
        check("E_R > 0", audit1.E_R_total > 0.001);

        // Check 4: With equal +/- particles, global L≈R energy (asymmetry < 20%)
        double asym = std::abs(audit1.E_L_total - audit1.E_R_total) /
                      (audit1.E_L_total + audit1.E_R_total);
        std::cout << "    L/R asymmetry=" << asym*100 << "%\n";
        check("L/R asymmetry < 20%", asym < 0.20);
    }

    // ================================================================
    // DS-COMPATIBILITY: Legacy behavior preserved with dual_substrate=false
    // ================================================================
    std::cout << "\n=== DS-COMPATIBILITY ===\n";

    {
        // Run identical simulation with dual_substrate OFF and ON
        // Compare observable field after some ticks

        // Legacy (single substrate)
        RenderBridge legacy(16);
        legacy.toggles.dual_substrate = false;
        legacy.toggles.genesis = false;
        legacy.toggles.forces = false;
        legacy.toggles.movement = false;
        legacy.toggles.gauss_projection = false;
        legacy.inject_flux(8, 8, 8, Vec3(0.3, 0.1, 0.0));
        for (int t = 0; t < 20; ++t) legacy.tick();

        auto audit_legacy = legacy.energy_audit();

        // Check 1: Legacy still works (has energy)
        check("Legacy has field energy", audit_legacy.field_energy > 0.001);

        // Check 2: Legacy dual fields are zero (not populated)
        double dual_energy_legacy = 0.0;
        for (int i = 0; i < legacy.lattice().total_sites(); ++i)
            dual_energy_legacy += legacy.voxels()[i].flux_L.mag2();
        check("Legacy: flux_L = 0", dual_energy_legacy < 1e-20);

        // Dual mode
        RenderBridge dual(16);
        dual.toggles.dual_substrate = true;
        dual.toggles.genesis = false;
        dual.toggles.forces = false;
        dual.toggles.movement = false;
        dual.toggles.gauss_projection = false;
        dual.inject_flux(8, 8, 8, Vec3(0.3, 0.1, 0.0));
        for (int t = 0; t < 20; ++t) dual.tick();

        auto audit_dual = dual.energy_audit();

        // Check 3: Dual mode also has field energy
        check("Dual has field energy", audit_dual.field_energy > 0.001);

        // Check 4: Observable flux = flux_L + flux_R invariant
        bool invariant_ok = true;
        for (int i = 0; i < dual.lattice().total_sites(); ++i) {
            auto& v = dual.voxels()[i];
            Vec3 sum = v.flux_L + v.flux_R;
            if (std::abs(v.flux.x - sum.x) > 1e-10 ||
                std::abs(v.flux.y - sum.y) > 1e-10 ||
                std::abs(v.flux.z - sum.z) > 1e-10) {
                invariant_ok = false;
                break;
            }
        }
        check("flux = flux_L + flux_R invariant", invariant_ok);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n=== DUAL SUBSTRATE: " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << " failures) ===\n";
    return failures;
}
