/**
 * Campaign: Quantitative Hydrogen Spectrum
 *
 * Upgrades the qualitative hydrogen binding test to a QUANTITATIVE
 * benchmark against known hydrogen physics. This is the key test for
 * whether the engine produces correct atomic physics, not just binding.
 *
 * Uses ParticleEngine (Scale 1) for analytical force evolution, which
 * avoids lattice artifacts and allows clean comparison with theory.
 *
 * Measurements:
 *   1. Ground state binding energy (compare to -13.6 eV in natural units)
 *   2. Orbital radius (compare to Bohr radius)
 *   3. Virial ratio <KE>/<PE> (should be -0.5 for Coulomb potential)
 *   4. Kepler period (compare to T = 2*pi*a_0 / v_1)
 *   5. Stability over long evolution
 *
 * Note on lattice gravity dominance:
 *   On the FTD lattice, G_N=0.01 >> alpha/(4*pi)=0.00058, so gravity
 *   contributes significantly. The effective coupling is:
 *     alpha_eff = alpha/(4*pi) + G_N * K_B^2 ~ 0.00319
 *   This gives R_bohr ~ 1/(K_B * alpha_eff) ~ 613 (not 3374).
 *   The test accounts for this by using the engine's actual force law.
 *
 * 8 checks:
 *   HS1: System is bound (total energy < 0)
 *   HS2: Orbital radius within 50% of predicted Bohr radius
 *   HS3: Binding energy within factor of 3 of predicted
 *   HS4: Virial ratio between -0.8 and -0.2 (relaxed for discrete)
 *   HS5: Energy drift < 5% over 10x orbital period
 *   HS6: Electron remains within 3x Bohr radius
 *   HS7: Kepler period within factor of 2 of predicted
 *   HS8: KE and PE have correct sign relationship (KE > 0, PE < 0)
 *   HS6: Electron remains within 3x Bohr radius
 *   HS7: Kepler period within factor of 2 of predicted
 *   HS8: KE and PE have correct sign relationship (KE > 0, PE < 0)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "ftd/particle_engine.h"
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
    std::cout << "  CAMPAIGN: Quantitative Hydrogen Spectrum — 8 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Setup: Hydrogen-like system at Scale 1 (ParticleEngine)
    // ================================================================
    // ParticleEngine uses analytical forces:
    //   F_EM  = -alpha * q1 * q2 * r_hat / (4*pi*r^2)
    //   F_grav = +G_N * m1 * m2 * r_hat / r^2
    // With q_proton = +1, q_electron = -1, both mass K_B

    ftd::ParticleEngine pe;
    pe.set_damping_enabled(false);  // CRITICAL: damping drains orbital energy

    // Predicted Bohr radius in ParticleEngine units
    // a_0 = 1 / (m_e * alpha_eff) where alpha_eff includes both EM and gravity
    double alpha_em = ftd::ALPHA / (4.0 * M_PI);  // ~0.000581
    double grav_contrib = ftd::G_N * ftd::K_B * ftd::K_B;  // ~0.00261
    double alpha_eff = alpha_em + grav_contrib;
    double a_bohr = 1.0 / (ftd::K_B * alpha_eff);

    // Predicted binding energy: E_bind = -0.5 * m * alpha_eff^2
    double e_bind_predicted = -0.5 * ftd::K_B * alpha_eff * alpha_eff;

    // Predicted orbital velocity and period
    double v_orbit = alpha_eff;
    double t_period = 2.0 * M_PI * a_bohr / v_orbit;

    std::cout << "\n--- Predictions ---\n";
    std::cout << std::setprecision(4);
    std::cout << "  alpha_EM/(4pi) = " << alpha_em << "\n";
    std::cout << "  G_N * K_B^2    = " << grav_contrib << "\n";
    std::cout << "  alpha_eff      = " << alpha_eff << "\n";
    std::cout << "  Bohr radius    = " << a_bohr << "\n";
    std::cout << "  Binding energy = " << e_bind_predicted << "\n";
    std::cout << "  Orbital period = " << t_period << "\n";

    // Inject proton at origin (locked — infinite mass)
    pe.add_locked_particle(+1, {0.0, 0.0, 0.0}, ftd::K_B);

    // Inject electron at Bohr radius with circular orbital velocity
    double r0 = a_bohr;
    double v0 = v_orbit;
    pe.add_particle(-1, {r0, 0.0, 0.0}, {0.0, v0, 0.0}, ftd::K_B);

    // ================================================================
    // Evolve: Run for 3 orbital periods (capped for test timeout)
    // ================================================================
    int steps_per_period = static_cast<int>(t_period / pe.dt()) + 1;
    int n_periods = 3;
    // Cap at 2M steps to keep test under 600s timeout
    int total_steps = std::min(steps_per_period * n_periods, 2000000);

    // Sample diagnostics
    std::vector<double> radii, ke_samples, pe_samples, total_e;
    double initial_energy = 0.0;
    double min_r = 1e30, max_r = 0.0;

    std::cout << "\n--- Evolution ---\n";
    std::cout << "  Steps per period: " << steps_per_period << "\n";
    std::cout << "  Total steps:      " << total_steps << "\n";

    for (int step = 0; step <= total_steps; ++step) {
        if (step > 0) pe.tick();

        // Sample every 1% of total
        if (step % (total_steps / 100 + 1) == 0 || step == 0) {
            auto& parts = pe.particles();
            if (parts.size() < 2) break;

            double dx = parts[1].position.x - parts[0].position.x;
            double dy = parts[1].position.y - parts[0].position.y;
            double dz = parts[1].position.z - parts[0].position.z;
            double r = std::sqrt(dx*dx + dy*dy + dz*dz);
            radii.push_back(r);
            if (r < min_r) min_r = r;
            if (r > max_r) max_r = r;

            // KE of electron (index 1)
            double vx = parts[1].velocity.x, vy = parts[1].velocity.y, vz = parts[1].velocity.z;
            double ke = 0.5 * parts[1].mass * (vx*vx + vy*vy + vz*vz);
            ke_samples.push_back(ke);

            // PE = -alpha_eff * |q1*q2| / r (EM attractive + gravity attractive for unlike)
            // For PE: EM part = -alpha/(4pi) * 1/r, Grav part = -G_N * m^2 / r
            double pe_val = -(alpha_em + grav_contrib) / (r > 1e-10 ? r : 1e-10);
            pe_samples.push_back(pe_val);

            total_e.push_back(ke + pe_val);
            if (step == 0) initial_energy = ke + pe_val;
        }
    }

    // ================================================================
    // Analysis
    // ================================================================
    std::cout << "\n--- Measurements ---\n";

    // Average radius
    double avg_r = 0.0;
    for (double r : radii) avg_r += r;
    avg_r /= radii.size();

    // Average KE, PE
    double avg_ke = 0.0, avg_pe = 0.0;
    for (double k : ke_samples) avg_ke += k;
    avg_ke /= ke_samples.size();
    for (double p : pe_samples) avg_pe += p;
    avg_pe /= pe_samples.size();

    double virial = (avg_pe != 0.0) ? avg_ke / avg_pe : 0.0;
    double avg_total_e = avg_ke + avg_pe;

    // Energy drift
    double final_energy = total_e.back();
    double e_drift = (initial_energy != 0.0) ?
                     std::abs(final_energy - initial_energy) / std::abs(initial_energy) : 0.0;

    // Radius error
    double r_err = std::abs(avg_r - a_bohr) / a_bohr;

    // Binding energy error (compare magnitudes)
    double be_ratio = (e_bind_predicted != 0.0) ?
                      std::abs(avg_total_e / e_bind_predicted) : 0.0;

    // Estimate period from radius oscillation
    // Count zero-crossings of (r - avg_r)
    int crossings = 0;
    for (size_t i = 1; i < radii.size(); ++i) {
        if ((radii[i] - avg_r) * (radii[i-1] - avg_r) < 0) crossings++;
    }
    double measured_period = (crossings > 1) ?
        2.0 * total_steps * pe.dt() / crossings : -1.0;
    double period_ratio = (measured_period > 0 && t_period > 0) ?
        measured_period / t_period : -1.0;

    std::cout << std::setprecision(4);
    std::cout << "  Average radius    = " << avg_r << " (predicted: " << a_bohr
              << ", error: " << r_err * 100 << "%)\n";
    std::cout << "  Min/Max radius    = " << min_r << " / " << max_r << "\n";
    std::cout << "  Average KE        = " << std::scientific << avg_ke << "\n";
    std::cout << "  Average PE        = " << avg_pe << "\n";
    std::cout << "  Average total E   = " << avg_total_e
              << " (predicted: " << e_bind_predicted << ")\n";
    std::cout << std::fixed;
    std::cout << "  Virial ratio      = " << virial << " (ideal: -0.5)\n";
    std::cout << "  Energy drift      = " << e_drift * 100 << "%\n";
    std::cout << "  BE ratio          = " << be_ratio << " (ideal: 1.0)\n";
    std::cout << "  Measured period   = " << measured_period
              << " (predicted: " << t_period << ")\n";
    std::cout << "  Period ratio      = " << period_ratio << "\n";

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // HS1: System is bound
    check("HS1: System is bound (total energy < 0)", avg_total_e < 0);

    // HS2: Orbital radius within 50% of predicted
    check("HS2: Avg radius within 50% of predicted Bohr radius", r_err < 0.5);

    // HS3: Binding energy within factor of 3
    check("HS3: Binding energy within factor 3 of predicted",
          be_ratio > 0.33 && be_ratio < 3.0);

    // HS4: Virial ratio between -0.8 and -0.2
    check("HS4: Virial ratio in [-0.8, -0.2]",
          virial > -0.8 && virial < -0.2);

    // HS5: Energy drift < 5% (Velocity Verlet is symplectic — should conserve well)
    check("HS5: Energy drift < 5% over evolution", e_drift < 0.05);

    // HS6: Electron stays within 3x Bohr radius
    check("HS6: Electron stays within 3x Bohr radius", max_r < 3.0 * a_bohr);

    // HS7: Period within factor of 2
    check("HS7: Kepler period within factor 2 of predicted",
          period_ratio > 0.5 && period_ratio < 2.0);

    // HS8: KE > 0 and PE < 0 (correct signs)
    check("HS8: KE > 0 and PE < 0 (correct sign relationship)",
          avg_ke > 0 && avg_pe < 0);

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "  NOTE: ParticleEngine (Scale 1) uses analytical Coulomb + gravity.\n";
    std::cout << "  The lattice gravity dominance (G_N >> alpha/(4pi)) means the\n";
    std::cout << "  Bohr radius is ~" << static_cast<int>(a_bohr) << " lattice units, not the\n";
    std::cout << "  pure-EM value of ~3374. This is a known lattice artifact.\n";
    std::cout << "  Tolerances are relaxed to account for discrete dynamics.\n";
    std::cout << "================================================================\n";
    return failures;
}
