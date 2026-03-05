#pragma once
// Logic-First FTD Lagrangian (4 active terms + Rayleigh dissipation)
//
// L_FTD = L_BI + L_COUPLING + L_VELOCITY + L_GAUSS
//
// R = (DAMPING/2) * |J_dot|^2  (Rayleigh dissipation function)

#include "voxel.h"
#include "render_bridge.h"
#include "constants.h"

namespace ftd {

// ============================================================================
// Active Per-Site Lagrangian Terms (4 terms — logic-derived)
// ============================================================================

// Term 1: Born-Infeld core  -K_B * sqrt(1 - v^2)
// Encodes rest mass and speed limit.
inline double born_infeld_term(const Voxel& v) {
    return v.born_infeld_core();
}

// Term 2: State-flux coupling (electric)  -g_c * s * div(J)
// EL equation for J -> g_c * grad(s) source term (phase_read).
// EL equation for s -> Coulomb force F = -alpha * s * grad(div J).
inline double coupling_term(const Voxel& v, double divJ) {
    return -G_C * v.state * divJ;
}

// Term 3: Velocity coupling (magnetic)  -g_c * s * (v . J)
// EL equation -> Lorentz force F = g_c * q * v x curl(J).
// Zero for stationary particles (v=0).
inline double velocity_coupling_term(const Voxel& v) {
    double v_dot_J = v.velocity.x * v.flux.x
                   + v.velocity.y * v.flux.y
                   + v.velocity.z * v.flux.z;
    return -G_C * v.state * v_dot_J;
}

// Term 4: Gauss constraint  -lambda_G * (div J - rho_charge)^2
// Enforces charge conservation. lambda_G -> infinity is exact constraint.
inline constexpr double LAMBDA_G = 100.0;

inline double gauss_term(double divJ, double rho_charge) {
    double violation = divJ - rho_charge;
    return -LAMBDA_G * violation * violation;
}

// Rayleigh dissipation function: R = (DAMPING/2) * |wave_vel|^2
// Models vacuum drag as energy sink. DAMPING = alpha (derived).
inline double rayleigh_dissipation(const Voxel& v) {
    return 0.5 * DAMPING * v.wave_vel.mag2();
}

// ============================================================================
// Composite Lagrangian Densities
// ============================================================================

// Logic-first 4-term Lagrangian density at a site
inline double lagrangian_density(const Voxel& v, double divJ, double rho_charge) {
    return born_infeld_term(v) + coupling_term(v, divJ) + gauss_term(divJ, rho_charge);
}

// Full 4-term density (includes velocity coupling)
inline double lagrangian_density_full(const Voxel& v, double divJ, double rho_charge) {
    return born_infeld_term(v)
         + coupling_term(v, divJ)
         + velocity_coupling_term(v)
         + gauss_term(divJ, rho_charge);
}

// ============================================================================
// Hamiltonian Density (Legendre transform of Born-Infeld)
// ============================================================================

// H_BI = K_B / sqrt(1 - v^2)
inline double hamiltonian_density(const Voxel& v, double divJ, double rho_charge) {
    double spd2 = v.speed() * v.speed();
    double h_bi;
    if (spd2 >= 1.0) {
        h_bi = 1e30;
    } else {
        h_bi = K_B / std::sqrt(1.0 - spd2);
    }
    return h_bi - coupling_term(v, divJ) - gauss_term(divJ, rho_charge);
}

// ============================================================================
// Variational Force Functions
// ============================================================================

// Coulomb force from coupling term EL equation:
//   F = -alpha * s * grad(div J)
inline Vec3 coupling_force(int8_t state, Vec3 grad_divJ) {
    return grad_divJ * (-ALPHA * state);
}

// Gravity force from density gradient:
//   F = G_N * grad(density)
inline Vec3 bi_gravity_force(Vec3 grad_density) {
    return grad_density * G_N;
}

// ============================================================================
// Global Diagnostics
// ============================================================================

struct LagrangianDiag {
    // Per-term sums (4 active + dissipation)
    double born_infeld_sum = 0.0;
    double coupling_sum = 0.0;
    double velocity_coupling_sum = 0.0;
    double gauss_sum = 0.0;
    double dissipation_sum = 0.0;

    // Totals
    double total_lagrangian = 0.0;
    double total_hamiltonian = 0.0;

    // Constraint violations
    double gauss_violation = 0.0;    // sum |div J - rho|^2
    double max_gauss_error = 0.0;    // max |div J - rho| over all sites

    // Conservation checks
    double total_flux_mag = 0.0;     // sum |J| (flux conservation)
    double total_wave_energy = 0.0;  // sum |wave_vel|^2 / 2 (kinetic)

    // Counters
    int manifested_count = 0;
    int locked_count = 0;
};

// Compute Lagrangian diagnostics from a RenderBridge snapshot
LagrangianDiag compute_lagrangian_diagnostics(const RenderBridge& rb);

}  // namespace ftd
