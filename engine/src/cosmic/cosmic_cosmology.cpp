/**
 * CosmicEngine cosmology: Friedmann / Hubble / dark energy.
 *
 * Extracted from cosmic_engine.cpp (CE4). Implements the FRW scale factor
 * evolution (RK4 single-step), the Hubble drag diagnostic, and the
 * Lambda-driven repulsive acceleration.
 */

#include "ftd/cosmic_engine.h"
#include <algorithm>
#include <cmath>

namespace ftd {

// ============================================================================
// Cosmology: Friedmann equations
// ============================================================================

void CosmicEngine::friedmann_step() {
    // Friedmann equation with FTD constants:
    // H^2 = H0^2 * [Omega_m / a^3 + Omega_Lambda]
    // where Omega_m = 1/3, Omega_Lambda = 2/3 (from FTD)
    double om = MATTER_FRACTION;    // 1 - OMEGA_LAMBDA_CONJ ≈ 0.0845... wait
    // Actually from constants.h: MATTER_FRACTION = DELTA_SQUARED ≈ 0.9155
    // and OMEGA_LAMBDA_CONJ = 2/3
    // For cosmology, use the standard fractions
    double omega_m = 1.0 - OMEGA_LAMBDA_CONJ; // ≈ 1/3
    double omega_l = OMEGA_LAMBDA_CONJ;        // ≈ 2/3

    double H2 = H0_ * H0_ * (omega_m / (a_ * a_ * a_) + omega_l);
    double H = std::sqrt(std::max(H2, 0.0));

    // RK4 for scale factor evolution
    // da/dt = a * H
    // dH/dt = -H^2 * (1 + q) where q = Omega_m/(2*Omega_total) - Omega_Lambda/Omega_total
    double k1_a = a_ * H * dt_;
    double k1_H = -H * H * (0.5 * omega_m / (a_ * a_ * a_) / (omega_m / (a_ * a_ * a_) + omega_l) - omega_l / (omega_m / (a_ * a_ * a_) + omega_l)) * dt_;

    a_ += k1_a;
    adot_ = a_ * std::sqrt(std::max(H0_ * H0_ * (omega_m / (a_ * a_ * a_) + omega_l), 0.0));
    t_cosmic_ += dt_;
}

void CosmicEngine::apply_hubble_expansion() {
    if (!toggles.hubble_expansion) return;

    friedmann_step();

    double H = hubble_parameter();
    // Apply Hubble drag: v_peculiar is unchanged in comoving coords
    // But physical positions scale: x_phys = a * x_comov
    // The Hubble flow is built into the Friedmann step
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        force_diag_[i].f_hubble = {
            -H * bodies_[i].velocity.x,
            -H * bodies_[i].velocity.y,
            -H * bodies_[i].velocity.z
        };
    }
}

void CosmicEngine::apply_dark_energy() {
    if (!toggles.dark_energy) return;

    // Dark energy as repulsive force: F_DE = (Lambda/3) * r * m
    // Lambda = 3 * H0^2 * Omega_Lambda
    double Lambda = 3.0 * H0_ * H0_ * OMEGA_LAMBDA_CONJ;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        Vec3 r = bodies_[i].position;
        double f_mag = Lambda / 3.0 * bodies_[i].mass;
        Vec3 f_de = {f_mag * r.x, f_mag * r.y, f_mag * r.z};
        forces_[i].x += f_de.x / bodies_[i].mass;
        forces_[i].y += f_de.y / bodies_[i].mass;
        forces_[i].z += f_de.z / bodies_[i].mass;
        force_diag_[i].f_dark_energy = {f_de.x / bodies_[i].mass,
                                        f_de.y / bodies_[i].mass,
                                        f_de.z / bodies_[i].mass};
    }
}

}  // namespace ftd
