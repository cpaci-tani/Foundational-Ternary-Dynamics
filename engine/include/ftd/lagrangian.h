#pragma once
// Partial discrete field/kinematic action diagnostic
// (6 active terms + Rayleigh dissipation; not the complete production tick)
//
// L_FTD = L_KINETIC + L_GRADIENT + L_BI + L_COUPLING + L_VELOCITY + L_GAUSS
//
// Field sector:      L_field = ½|Δ_t J|² - ½c²Σ_μ w_μ|ΔJ_μ|²
// Kinematic core:    L_BI = -E_REST √(1 - |u|²/C_SPEED² - L²)
//                    (currently evaluated at every voxel and independent of s)
// Diagnostic interaction:
//                    L_coupling = +g_c·s·(∇·J) - g_c·s·(v·J)
// Prescribed field-source interaction actually integrated by phase_read:
//                    I_source = +g_c<s,div J> + g_c<curl J,s v>
//                    (electric sign AMENDED 2026-07-18 — the previous −g_c·s·(∇·J)
//                     was in internal sign conflict with L_GAUSS at charge sites:
//                     its EL source +g_c·∇s drove div J anti-correlated with s,
//                     measured live equilibrium f = −0.095 against the Gauss
//                     target +1. See test_gauss_law_fidelity.cpp + CHANGELOG.)
// Constraint:        L_gauss = -λ_G·(∇·J - ρ)²
// Dissipation:       R = (α/2)|wave_vel|²
//
// The per-slice diagnostic S = Σ_v L_density(v)·V_cell is an exact finite
// volume sum on the unit lattice — not a continuum-limit claim. FTD-0574
// derives the free production field tick from a separate nearest-time-slice
// discrete action and proves that wave_vel is its Legendre momentum. The
// stationary electric term has the correct J-variation. The onsite velocity
// term does NOT generate phase_read's +g_c curl(s v): its J-gradient is
// -g_c s v. That coded source instead follows from +g_c<curl J,s v>.
// Optional matter-force branches remain selected update rules, not variations
// of one common action (FTD-0467/0574).

#include "voxel.h"
#include "render_bridge.h"
#include "constants.h"
#include "volumetric_measure.h"
#include <array>

namespace ftd {

// ============================================================================
// Active Per-Site Lagrangian Terms (4 terms — logic-derived)
// ============================================================================

// Term 1: Born-Infeld core  -K_B * sqrt(1 - v^2)
// Encodes the selected causal kinematic core. The diagnostic currently
// evaluates it at every voxel without a state factor, so it cancels from a
// candidate-state variation and cannot generate genesis (FTD-0567).
inline double born_infeld_term(const Voxel& v) {
    return v.born_infeld_core();
}

// Term 2: State-flux coupling (electric)  +g_c * s * div(J)
// EL equation for J -> -g_c * grad(s) source term (phase_read): at a +1 charge
// the drive points OUTWARD, sourcing div J > 0 — cooperating with the Gauss
// constraint term (div J = ρ ∝ s) instead of fighting it. The pre-2026-07-18
// sign (−g_c·s·divJ, source +g_c·∇s) made the Hamiltonian's coupling energy
// (−L) prefer s·divJ < 0 while L_GAUSS demanded s·divJ > 0 — two terms of the
// same action in conflict at every charge site; the live engine settled the
// compromise at f = −0.095 of the Gauss target (wrong sign). Measured in
// test_gauss_law_fidelity.cpp; the flip aligns both terms on one manifold.
// The central point-probe variation of the written interaction has operator,
// sign, and bare coefficient F = +G_C * s * grad(div J). The standalone
// coupling_force helper below retains the selected effective ALPHA
// normalization; the production legacy branch uses the opposite sign, and
// the production emergent branch uses grad|J|. See FTD-0467.
inline double coupling_term(const Voxel& v, double divJ) {
    return G_C * v.state * divJ;
}

// Term 3: selected onsite matter-side velocity coupling
//         -g_c * s * (v . J)
// Its point-worldline variation has the usual v x curl(J) structure, but its
// FIELD variation is -g_c*s*v, not phase_read's +g_c*curl(s*v). FTD-0574
// proves the exact coded source instead comes from +g_c<curl J,s*v>, whose
// path variation contains induction and curl-curl terms. This diagnostic term
// and the coded moving source are therefore not one common action. Zero for
// stationary particles (v=0).
inline double velocity_coupling_term(const Voxel& v) {
    double v_dot_J = v.velocity.x * v.flux.x
                   + v.velocity.y * v.flux.y
                   + v.velocity.z * v.flux.z;
    return -G_C * v.state * v_dot_J;
}

// Term 4: selected Gauss penalty  -lambda_G * (div J - rho_charge)^2
// Penalizes violation of the selected div(J)=rho identification. This local
// diagnostic does not prove full-event charge conservation or U(1) gauge
// redundancy; reactions are separately scoped by FTD-0421.
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
//
// Pair-counting convention: PAIRS-ONCE. The gradient sector of the action is
//   S_grad = -½c² Σ_links w_l |J(a)-J(b)|²    (each neighbor link counted once,
//                                              w_face = 1/3, w_edge = 1/6)
// This function returns site v's HALF-SHARE of its 18 incident links,
//   L_grad(v) = -¼c² [Σ_face (1/3)|ΔJ|² + Σ_edge (1/6)|ΔJ|²]
// so that Σ_v L_grad(v) = S_grad exactly (each link split half/half between
// its two endpoint sites). Under this normalization the total variation
//   δS_grad/δJ(v) = c² [(1/3)·Σ_face J(n) + (1/6)·Σ_edge J(n) - 4·J(v)]
// is exactly the 18-point stencil integrated by phase_read(), with the same
// relative normalization as the kinetic term ½|Δ_t J|² (wave speed c).
// A per-site FULL-neighbor sum with prefactor -½c², accumulated over sites,
// would count every link twice: 2× the gradient energy, and an EL equation
// with 2c²∇² against the coded kinetic term. Verified by
// test_action_stationarity.cpp Section 7 (single-spike value + finite-
// difference δS/δJ against laplacian_flux).
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
    return -0.25 * (C_WAVE * C_WAVE) * grad_sq;
}

// ============================================================================
// Composite Lagrangian Densities
// ============================================================================

// Full interaction Lagrangian density (4 terms — excludes field sector)
// Includes velocity coupling (Lorentz/magnetic sector).
inline double lagrangian_density(const Voxel& v, double divJ, double rho_charge) {
    return born_infeld_term(v)
         + coupling_term(v, divJ)
         + velocity_coupling_term(v)
         + gauss_term(divJ, rho_charge);
}

// Alias for backward compatibility
inline double lagrangian_density_full(const Voxel& v, double divJ, double rho_charge) {
    return lagrangian_density(v, divJ, rho_charge);
}

// ============================================================================
// Hamiltonian Density (Legendre transform of Born-Infeld)
// ============================================================================

// H_BI = E_REST·f / sqrt(1 - B), with f=1-L² and
// B=|u|²/C_SPEED²+L² (FTD-0402 raw-lattice contract).
inline double hamiltonian_density(const Voxel& v, double divJ, double rho_charge) {
    const double h_bi = born_infeld_hamiltonian(v.latency, v.velocity.mag2());
    return h_bi - coupling_term(v, divJ) - velocity_coupling_term(v) - gauss_term(divJ, rho_charge);
}

// ============================================================================
// Variational Force Functions
// ============================================================================

// Selected effective-normalization helper with the operator/sign of the
// coupling-term point-probe variation:
//   F_helper = +alpha * s * grad(div J)
// The exact bare variation of the written +G_C*s*div(J) interaction carries
// G_C, not ALPHA. This helper is not called by phase_forces_main_loop; retain
// the distinction established by FTD-0467.
inline Vec3 coupling_force(int8_t state, Vec3 grad_divJ) {
    return grad_divJ * (ALPHA * state);
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
    double field_gradient_sum = 0.0;   // -½c² Σ_links w_l|ΔJ|² (pairs-once; see Term 6)

    // Per-term sums (4 interaction + dissipation)
    double born_infeld_sum = 0.0;
    double coupling_sum = 0.0;
    double velocity_coupling_sum = 0.0;
    double gauss_sum = 0.0;
    double dissipation_sum = 0.0;

    // Totals
    double total_lagrangian = 0.0;     // Complete 6-term Lagrangian
    // Legacy particle-plus-interaction diagnostic. Despite the retained API
    // name, this excludes field kinetic, gradient, and exact tick cross terms.
    // It must not be used as a total wave-energy conservation observable.
    // See FTD-0452 and eft/native_energy_contract.h.
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

    // FTD-0404 append-only spatial-measure metadata.
    double cell_volume = VOXEL_VOLUME;
};

// Legacy-named production field-equation replay residual. After phase_read(),
// delta_j_[i] should equal c²∇²J − g_c∇(s) + g_c∇×(s·v). FTD-0574 proves
// this is the EL equation of the nearest-time-slice free-field action plus
// prescribed interaction +g_c<s,div J>+g_c<curl J,s v>. In the moving-source
// sector it is NOT the EL residual of lagrangian_density(), whose onsite
// velocity term has a different J-variation. The retained API name is legacy.
struct ELResidual {
    double rms = 0.0;       // RMS residual over all sites
    double max_abs = 0.0;   // Maximum absolute residual
};

// Compute Lagrangian diagnostics from a RenderBridge snapshot
LagrangianDiag compute_lagrangian_diagnostics(const RenderBridge& rb);

// Independently recompute the production field EOM and compare against the
// stored delta_j_ buffer. Call after phase_read() for meaningful results.
ELResidual compute_el_residual(const RenderBridge& rb);

// Legacy-named production-force replay residual: verifies that force_diag_[i]
// matches an independent evaluation of the selected force formulas.
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

// Independently recompute selected production forces on manifested particles
// and compare them to force_diag_. Despite the retained API name, this is not
// a proof of a common action; see FTD-0467.
// Call after tick() for meaningful results (force_diag_ is populated by phase_forces).
ParticleELResidual compute_particle_el_residual(const RenderBridge& rb);

}  // namespace ftd
