/**
 * AtomEngine velocity post-processing: speed limit, damping, Berendsen
 * thermostat, and per-atom dipole moment computation.
 *
 * Extracted from atom_engine.cpp (ticket AE3). These run after the
 * Velocity Verlet half-kick in the tick pipeline.
 *
 * compute_dipole_moments is placed here because the dipole moments it
 * produces drive the dipole-dipole force in atom_forces.cpp, and because
 * the dipole field is intrinsically a per-atom state-update pass that
 * naturally groups with the other velocity/state passes.
 */

#include "ftd/atom_engine.h"
#include <cmath>

namespace ftd {

void AtomEngine::enforce_speed_limit() {
    for (auto& a : atoms_) {
        if (a.locked) continue;
        double v = a.velocity.mag();
        if (v > C_SPEED) {
            a.velocity *= (C_SPEED / v);
        }
    }
}

void AtomEngine::apply_damping() {
    if (!toggles.damping) return;
    double factor = 1.0 - DAMPING * dt_;
    if (factor < 0.0) factor = 0.0;
    for (auto& a : atoms_) {
        if (a.locked) continue;
        a.velocity *= factor;
    }
}

void AtomEngine::apply_thermostat() {
    if (!toggles.thermostat || target_temperature_ <= 0.0) return;

    // Compute current temperature: T = 2*KE / (3*N)
    int free_count = 0;
    double ke = 0.0;
    for (const auto& a : atoms_) {
        if (!a.locked) {
            ke += 0.5 * a.mass * a.velocity.mag2();
            free_count++;
        }
    }
    if (free_count == 0) return;
    double T_current = 2.0 * ke / (3.0 * free_count);
    if (T_current < 1e-30) return;

    // Berendsen velocity rescaling.
    // lambda^2 = 1 + (dt/tau)(T_target/T_current - 1)
    // When T_current >> T_target and dt/tau >= 1, the argument can go
    // negative and sqrt would produce NaN, corrupting all velocities and
    // triggering downstream heap corruption in the Barnes-Hut octree.
    // (Wave 3.3 audit, 2026-04-14 — found via test_ae_thermostat TH4
    // crash: T_init=30, T_target=0.0001, dt/tau=2.0 → lambda^2 = -1.)
    // Clamp to >= 0: when cooling is requested faster than the stable
    // Berendsen limit, we take the maximum cooling step (lambda -> 0)
    // rather than producing imaginary velocities.
    double lambda_sq = 1.0 + dt_ / thermostat_tau_
                           * (target_temperature_ / T_current - 1.0);
    if (lambda_sq < 0.0) lambda_sq = 0.0;
    double lambda = std::sqrt(lambda_sq);
    for (auto& a : atoms_) {
        if (!a.locked) a.velocity *= lambda;
    }
}

void AtomEngine::compute_dipole_moments() {
    // Compute each atom's dipole moment from bond structure + electronegativity
    for (auto& a : atoms_) {
        a.dipole_moment = {};
        if (!toggles.dipole_dipole && !toggles.electronegativity) continue;

        for (const auto& bond : a.bonds) {
            int jidx = index_of(bond.partner_id);
            if (jidx < 0) continue;
            const auto& aj = atoms_[jidx];

            // Bond dipole: proportional to electronegativity difference
            double chi_diff = aj.electronegativity - a.electronegativity;
            if (std::abs(chi_diff) < 1e-10) continue;

            Vec3 r_bond = aj.position - a.position;
            double r = std::sqrt(r_bond.mag2());
            if (r < 1e-30) continue;

            // Dipole contribution: mu = chi_diff * bond_vector
            a.dipole_moment += r_bond * chi_diff;
        }
    }
}

}  // namespace ftd
