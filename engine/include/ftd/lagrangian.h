#pragma once
// Complete Discrete FTD Lagrangian (6 active terms + Rayleigh dissipation)
//
// L_FTD = L_KINETIC + L_GRADIENT + L_BI + L_COUPLING + L_VELOCITY + L_GAUSS
//
// Field sector:      L_field = ½|Δ_t J|² - ½c²Σ_μ w_μ|ΔJ_μ|²
// Particle sector:   L_BI = -K_B √(1 - v²)
// Interaction:       L_coupling = -g_c·s·(∇·J) - g_c·s·(v·J)
// Constraint:        L_gauss = -λ_G·(∇·J - ρ)²
// Dissipation:       R = (α/2)|wave_vel|²
//
// The discrete action S = Σ_v L(v) is an exact finite sum — not an
// approximation of an integral. The tick cycle IS the Euler-Lagrange
// equations of this action.

#include "voxel.h"
#include "render_bridge.h"
#include "constants.h"
#include <array>

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
// Models vacuum drag as energy sink. DAMPING = alpha [IMPOSED — see ontic.h ASSUMP.6].
inline double rayleigh_dissipation(const Voxel& v) {
    return 0.5 * DAMPING * v.wave_vel.mag2();
}

// ============================================================================
// Field-Sector Terms (the wave equation's energy)
// ============================================================================

// Term 5: Field kinetic energy  ½|Δ_t J|² = ½|wave_vel|²
// The canonical momentum of the flux field.
// EL: ∂L/∂(Δ_t J) = wave_vel (the conjugate momentum).
// NOT double-counted with Born-Infeld: BI uses particle velocity v.speed();
// this uses field oscillation velocity wave_vel. Different physical quantities.
inline double field_kinetic_term(const Vec3& wave_vel) {
    return 0.5 * wave_vel.mag2();
}

// Term 6: Field gradient energy (18-point isotropic stencil)
// -½c² [Σ_face (1/3)|ΔJ|² + Σ_edge (1/6)|ΔJ|²]
// Variational derivative δ/δJ reproduces the 18-point Laplacian:
//   (1/3)·Σ_face J(n) + (1/6)·Σ_edge J(n) - 4·J(v)
// which is exactly the stencil used by phase_read().
inline double field_gradient_term(const Vec3& flux_here,
                                  const std::array<int, 6>& nbr6,
                                  const std::array<int, 12>& nbr12,
                                  const std::vector<Voxel>& voxels) {
    double grad_sq = 0.0;
    for (int n : nbr6) {
        Vec3 d = voxels[n].flux - flux_here;
        grad_sq += (1.0 / 3.0) * d.mag2();
    }
    for (int n : nbr12) {
        Vec3 d = voxels[n].flux - flux_here;
        grad_sq += (1.0 / 6.0) * d.mag2();
    }
    return -0.5 * (C_WAVE * C_WAVE) * grad_sq;
}

// ============================================================================
// Composite Lagrangian Densities
// ============================================================================

// Interaction-only Lagrangian density (4 terms — excludes field sector)
inline double lagrangian_density(const Voxel& v, double divJ, double rho_charge) {
    return born_infeld_term(v) + coupling_term(v, divJ) + gauss_term(divJ, rho_charge);
}

// Interaction-only with velocity coupling (4 terms — excludes field sector)
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
    // Field-sector terms (the wave equation's energy)
    double field_kinetic_sum = 0.0;    // Σ ½|wave_vel|²
    double field_gradient_sum = 0.0;   // Σ -½c² w_μ|ΔJ|²

    // Per-term sums (4 interaction + dissipation)
    double born_infeld_sum = 0.0;
    double coupling_sum = 0.0;
    double velocity_coupling_sum = 0.0;
    double gauss_sum = 0.0;
    double dissipation_sum = 0.0;

    // Totals
    double total_lagrangian = 0.0;     // Complete 6-term Lagrangian
    double total_hamiltonian = 0.0;

    // Discrete action: S = Σ_v L(v)
    // Exact finite sum. Not an approximation of an integral.
    double total_action = 0.0;

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

// Euler-Lagrange residual: how well does the tick satisfy δS/δJ = 0?
// After phase_read(), delta_j_[i] should equal c²∇²J + g_c∇(s) + g_c∇×(s·v).
// This struct measures the discrepancy (should be machine-epsilon ~1e-15).
struct ELResidual {
    double rms = 0.0;       // RMS residual over all sites
    double max_abs = 0.0;   // Maximum absolute residual
};

// Compute Lagrangian diagnostics from a RenderBridge snapshot
LagrangianDiag compute_lagrangian_diagnostics(const RenderBridge& rb);

// Compute EL residual: independently recomputes the field EOM and compares
// against the stored delta_j_ buffer. Call after phase_read() for meaningful results.
ELResidual compute_el_residual(const RenderBridge& rb);

// Particle EL residual: verifies that force_diag_[i] matches the Lagrangian
// partial derivatives δL/δx for manifested particles.
// For each manifested voxel, independently recomputes:
//   - EM:      F_EM = -α·s·∇(φ_C)       (Poisson) or -α·s·∇(∇·J) (legacy)
//   - Gravity: F_grav = G_N·∇ρ(tier-2)   (tier-2 stencil, r=2)
//   - Lorentz: F_mag = α·s·(v × ∇×J)    (when |v| > ε)
// and compares against stored force_diag_[i].
struct ParticleELResidual {
    double rms = 0.0;
    double max_abs = 0.0;
    int particle_count = 0;
};

// Compute particle EL residual: independently recomputes all forces on manifested
// particles from Lagrangian partial derivatives and compares to force_diag_ buffer.
// Call after tick() for meaningful results (force_diag_ is populated by phase_forces).
ParticleELResidual compute_particle_el_residual(const RenderBridge& rb);

}  // namespace ftd
