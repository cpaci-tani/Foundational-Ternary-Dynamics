#pragma once
/**
 * FTD Render-Bridge Constants
 *
 * Engine-facing interface to the ontic derivation chain.
 * All values derive from {D=3, ϖ} via ontic.h.
 *
 * This header re-exports the ontic constants into the ftd:: namespace
 * so existing engine code (render_bridge.cpp, lagrangian.h, etc.) can
 * use them without qualification. The derivation logic lives in ontic.h.
 *
 * See docs/theory/SPEC_FTD_LAGRANGIAN.md v2.0.
 */

#include "ontic.h"
#include "ftd/constants_gpu.cuh"
#include <algorithm>
#include <cmath>


namespace ftd {

// ============================================================================
// Re-export ontic constants into ftd:: namespace for engine use
// ============================================================================

// Layer -1: Self-referential seed
using ontic::EULER_E;

// Layer 0: Transcendental seeds
using ontic::EULER_GAMMA;
using ontic::GAMMA_QUARTER;

// Layer 0b: Modular selection
using ontic::NOME_LEMNISCATIC;
using ontic::THETA_LEMNISCATIC;

// Layer 1: Elliptic geometry
using ontic::GAUSS_CONSTANT_M;
using ontic::PI;
using ontic::VARPI;

// Layer 2: Universal operator
using ontic::G_STAR;
using ontic::GSTAR_ACTION;
using ontic::GSTAR_FLUX;
using ontic::GSTAR_TIME;
using ontic::PF;
using ontic::SQRT_GSTAR;

// Layer 2b: Euler's identity / emergence of i
using ontic::K_CRIT;
using ontic::X_BORN;

// Layer 3: Master quadratic
using ontic::COEFFICIENT;
using ontic::X_MINUS;
using ontic::X_PLUS;
using ontic::X_PLUS_PRECISION;

// Layer 3c: Charge-space duality
using ontic::E2_COLOR;

// Layer 3b: Dual-substrate decomposition
using ontic::DELTA_APPROX;
using ontic::DELTA_SQUARED;
using ontic::E_LEFT_APPROX;
using ontic::E_PRODUCT;
using ontic::E_RIGHT_APPROX;
using ontic::E_SUM;
using ontic::MATTER_FRACTION;
using ontic::OMEGA_LAMBDA_CONJ;
using ontic::VACUUM_FRACTION;

// Layer 4: Framework integers
using ontic::B_3;
using ontic::D_CONSTRAINT;
using ontic::D_SPATIAL;
using ontic::LADDER_ELECTRON;
using ontic::LADDER_GRAVITY;
using ontic::LADDER_HIGGS;
using ontic::LADDER_NEUTRINO;
using ontic::LADDER_PERTURBATIVE;
using ontic::N_BASE;
using ontic::N_C;
using ontic::N_EFF;
using ontic::N_F;
using ontic::N_GEN;

// Layer 4b: Neutrino mixing
using ontic::DM2_RATIO;
using ontic::NORMAL_HIERARCHY;
using ontic::SIN2_THETA12;
using ontic::SIN2_THETA13;
using ontic::SIN2_THETA23;

// Layer 5: Coupling constants
using ontic::ALPHA;             // = 1/X_PLUS_PRECISION (CODATA match; 2026-04-17)
using ontic::ALPHA_G_APPROX;
using ontic::ALPHA_PRECISION;   // alias to ALPHA (same value)
using ontic::ALPHA_TREE;        // = 1/X_PLUS (tree-level, reference only)
using ontic::ALPHA_WEAK;
using ontic::G_C;
using ontic::G_N;
using ontic::SIN2_WEINBERG;

// ──────────────────────────────────────────────────────────────────────
// ALPHA_EFT: two-vertex coupling used at every force-computation site.
//
// HONEST FRAMING:
//   G_C was DEFINED as √α in ontic.h Layer 5 (the state-flux coupling
//   in the Born-Infeld Lagrangian). So ALPHA_EFT = G_C² = α is an
//   algebraic identity by construction — this is a CONSISTENCY CHECK,
//   not a derivation. The static_assert below catches drift between
//   the two definitions, it does NOT prove new physics.
//
//   The actual derivation of α is the master quadratic x² − 16G*²x + 16G*³ = 0
//   whose tree root is X_PLUS. The 4-term corrected root
//   X_PLUS_PRECISION matches CODATA 2022 to < 0.001 ppt; the engine's
//   ALPHA has used that precision value since 2026-04-17 (TRACKER §1.5).
//
// WHAT THIS MEANS FOR THE ENGINE:
//   Force code may use either ALPHA or ALPHA_EFT — they are equal by
//   construction. Prefer ALPHA_EFT in code paths where the two-vertex
//   picture is pedagogically helpful (e.g., the EFT emergent force mode
//   where flux field + state probe each contribute one vertex).
inline constexpr double ALPHA_EFT = G_C * G_C;
static_assert(ALPHA_EFT > 0.00729 && ALPHA_EFT < 0.00731,
              "Consistency: G_C^2 must equal alpha (G_C was defined as sqrt(alpha))");
static_assert(ALPHA_EFT > 0.99999999 * ontic::ALPHA &&
              ALPHA_EFT < 1.00000001 * ontic::ALPHA,
              "Consistency: G_C hardcoded value must match sqrt(ALPHA) to 1e-8");

// Layer 5b: QCD sector
using ontic::ALPHA_S_MZ;
using ontic::alpha_s_running;
using ontic::B0_NF5;
using ontic::B0_NF6;
using ontic::LAMBDA_QCD;
using ontic::M_Z;

// Layer 6: Mass scale
using ontic::K_B;
using ontic::K_GENESIS;

// Layer 6c: Mass ratios
using ontic::M_PROTON;
using ontic::MU_RATIO;
using ontic::PROTON_RATIO;
using ontic::R_BOHR;
using ontic::TAU_RATIO;

// Layer 6b: Electroweak scale (Higgs)
using ontic::LAMBDA_HIGGS;
using ontic::M_HIGGS;
using ontic::V_HIGGS;

// Lattice Representation of VEV and W/Z Mass
// V_HIGGS is 246.09 GeV. K_B is 0.511 MeV.
// Therefore V_HIGGS_LATTICE = V_HIGGS * 1000.0 / K_B (scaled by the electron mass amplitude)
inline constexpr double HIGGS_VEV_LATTICE = V_HIGGS * 1000.0 / K_B;
inline constexpr double WZ_MIXING_ANGLE_COS = 0.881; // cos(theta_W) ~ 80.4/91.2

// Layer 7: Precision formula
using ontic::C1;
using ontic::C2;
using ontic::C3;
using ontic::C4;
using ontic::EPSILON;
using ontic::EPSILON_ABS;

// Layer 8: Consciousness quadratic (noetic domain)
using ontic::C_MANDELBROT;
using ontic::COS2_THETA_C;
using ontic::K_C_SQUARED;
using ontic::K_NOETIC;
using ontic::SIN2_THETA_C;
using ontic::Y_REAL;

// Layer 8b: Golden ratio fixed point (self-referential consciousness)
using ontic::BETA_INTROSPECTION;
using ontic::LAMBDA_LOOP;
using ontic::N_CONSCIOUSNESS_MIN;
using ontic::PHI;
using ontic::PHI_INV;

// Simulation parameters
using ontic::C_SPEED;
using ontic::C_WAVE;
using ontic::DAMPING;
using ontic::DRAG_PER_AXIS;

// ============================================================================
// Engine-specific constants (not part of ontic chain)
// ============================================================================

// Drag values for specific particle types
inline constexpr double DRAG_ELECTRON = 0.25; // 1D drag
inline constexpr double DRAG_TOP = 0.75;      // 3D drag

// Default effective radius for Scale 1 particles (from Phase 6 engine data)
inline constexpr double R_EFF_DEFAULT = 2.48;

// ============================================================================
// SM Sector Constants (Phase 2 — phenomenological insertions)
// ============================================================================

// Strong coupling at lattice (Planck) scale [IMPOSED]
// At the Planck scale, QCD is strongly coupled: α_s ~ O(1).
// F_strong(r) = ALPHA_S * exp(-M_YUKAWA*r) / r² * (1 + M_YUKAWA*r)
inline constexpr double ALPHA_S = 1.0;

// Yukawa range parameter (inverse meson mass in lattice units) [IMPOSED]
// 1/M_YUKAWA sets the effective range of the strong force (~1-2 voxels).
inline constexpr double M_YUKAWA = 1.0;

// Energy scale per lattice unit for QCD running (GeV) [IMPOSED]
// Maps lattice separation r to energy scale: Q(r) = Q_LATTICE / r
// At Q_LATTICE = 2.0: r=1 → α_s≈0.37, r=√2 → α_s≈0.44, r≥5 → α_s=1.0
inline constexpr double Q_LATTICE = 2.0;

// Running strong coupling as a function of lattice separation.
// Maps r (voxels) → Q (GeV) via Q = Q_LATTICE / r, then evaluates running.
// Clamped to [0, ALPHA_S] to avoid Landau pole artifacts.
inline double alpha_s_lattice(double r_voxels) {
  if (r_voxels <= 0.0)
    return ALPHA_S;
  double Q = Q_LATTICE / r_voxels;
  double as = alpha_s_running(Q);
  return std::min(as, ALPHA_S);
}

// Strong force string tension [DERIVED from ALPHA_S and K_B]
// V(r) = SIGMA_STRING * r (linear confinement potential)
// F = -dV/dr = -SIGMA_STRING (constant force at long range)
// Motivated by lattice QCD: sigma ~ alpha_s * K_B^2
inline constexpr double SIGMA_STRING = ALPHA_S * K_B * K_B;

// Asymptotic freedom crossover radius [IMPOSED]
// Below this radius, coulombic 1/r^2 term dominates (asymptotic freedom).
// Above this radius, linear confinement dominates.
// Set to 1/M_YUKAWA for consistency with former Yukawa range.
inline constexpr double R_CONFINEMENT = 1.0 / M_YUKAWA;

// Weak transmutation threshold [IMPOSED]
// Polarity flips when field stress |∇·J| + |∇×J| + |∇ρ| exceeds this.
inline constexpr double WEAK_THRESHOLD = K_GENESIS;

// Triad binding energy: K_B × φ [DERIVED from stability analysis]
// PHI now in ontic.h Layer 8b as [THEOREM] (emerges from Softplus fixed point)
inline constexpr double BINDING_ENERGY = K_B * PHI;

// Exchange repulsion strength scale (Fermi pressure)
// From DERIV_SPIN_STATISTICS_BRIDGE: the discriminant trichotomy produces
// an antisymmetric exchange term between identical fermions.
// The 1/r^4 exchange force uses α² as its natural scale.
inline constexpr double ALPHA_EXCHANGE = ALPHA * ALPHA; // ≈ 5.3e-5

// Larmor radiation: flux damping proportional to acceleration²
// From classical Larmor formula P = (2α/3)·a², normalized by K_B.
// K_LARMOR * a² gives the fraction of max damping to apply.
//
// The coefficient must exceed the coupling injection rate g_c ≈ 0.085
// per site visited, otherwise accelerating particles GAIN net field
// energy (coupling pumps faster than Larmor damps). Scaling by N_EFF
// (the effective DoF count) ensures Larmor dominates at moderate a.
inline constexpr double K_LARMOR = 4.0 * N_EFF / (3.0 * K_B); // ≈ 33.9

// Minimum damping floor: 1% of full damping even at a=0.
// Ensures thermodynamic dissipation is never completely off.
inline constexpr double LARMOR_FLOOR = 0.01;

// Triad binding detection (CPU port of GPU triad_detection_kernel)
// Three same-sign particles within TRIAD_RADIUS with near-equilateral geometry.
inline constexpr double TRIAD_RADIUS = 3.0;          // max pairwise distance
inline constexpr double TRIAD_RATIO_THRESHOLD = 0.8; // min(r)/max(r) threshold

// Exchange (Pauli) force range (CPU port of GPU exchange_force_kernel)
// Same-spin repulsion falls off as exp(-r²/EXCHANGE_RANGE²).
inline constexpr double EXCHANGE_RANGE = 3.0; // voxels
inline constexpr double EXCHANGE_RANGE_SQ = EXCHANGE_RANGE * EXCHANGE_RANGE;

// ============================================================================
// Engine tuning constants (extracted from render_bridge.cpp)
// ============================================================================

// SOR solver parameters (used by gauss_project and solve_coulomb_poisson).
//
// The Coulomb solver is warm-started from the previous tick, so post-tick
// drift converges in a handful of iterations. Per-iteration SOR cost
// dominates the tick at L>=64 (~70% of total), so iteration count has
// direct linear FPS impact. Empirical convergence at warm-start:
//
//   iters | gauss_violation @ L=64 | tick cost (rel)
//   ------+------------------------+-----------------
//     30  |  0.00012               |  ~3.0x
//     10  |  0.00040               |  ~1.0x
//      6  |  0.00080               |  ~0.7x   ← chosen
//      4  |  0.0018                |  ~0.5x
//
// 6 iters keeps gauss_violation < 1e-3 for typical scenarios and gives
// ~30% faster ticks than 10. If you observe drift in physics tests at
// the new value, bump back to 8.
inline constexpr int SOR_ITERATIONS = 6;
inline constexpr double SOR_OMEGA = 1.75;

// Evaporation: particle dies when 7-site neighborhood energy < K_B² × this
inline constexpr double EVAP_THRESHOLD = 1e-6;

// Numerical underflow guards
inline constexpr double EPSILON_FLUX_SQ = 1e-30; // guard for |J|² divisions
inline constexpr double EPSILON_MAG = 1e-15; // guard for magnitude divisions

// RF-8 (2026-04-25): bandwidth budget floor used by accumulate_proper_time()
// to keep the effective speed-of-light denominator finite when L² approaches
// 1 (near-horizon limit) and to clamp the kinetic budget below 1.0. Was
// previously a bare `1e-6` literal at three call sites in render_bridge.cpp.
inline constexpr double BANDWIDTH_FLOOR = 1e-6;

// Wavepacket injection: Gaussian truncated at this many sigma
inline constexpr double GAUSSIAN_CUTOFF_SIGMA = 3.0;

// Tier-2 gravity gradient scale: 1/(2×2) for r=2 stencil
inline constexpr double GRAD_TIER2_SCALE = 0.25;

// Color force regime boundaries — now defined in ftd/constants_gpu.cuh (shared with GPU).
// Using declarations bring them into the ftd:: namespace so existing callers are unchanged.
using ::COLOR_COULOMB_RADIUS;
using ::COLOR_TRANSITION_RADIUS;
using ::COLOR_TRANSITION_DENOM;
using ::COLOR_LINEAR_DENOM;

// Latency / horizon clamps used by the GR sector
inline constexpr double LATENCY_HORIZON_CLAMP = 0.998;   // f = 1 - L² floor

// ============================================================================
// Scale 2 Phase 3 Constants — Inter-atomic forces
// ============================================================================

// H-bond parameters [DERIVED from α perturbation theory]
// H-bonds are ~10× weaker than covalent, ~comparable to vdW for small atoms.
// LJ 10-12 form: V = eps * [5*(sig/r)^12 - 6*(sig/r)^10] * cos^n(theta)
inline constexpr double H_BOND_EPSILON = K_B * ALPHA * ALPHA * ALPHA; // ~1.98e-7
inline constexpr double H_BOND_COS_POWER = 2.0; // angular dependence exponent

// Angle strain (VSEPR) [DERIVED from α × K_B]
// V = K_ANGLE * (theta - theta_eq)^2 / 2
inline constexpr double K_ANGLE = ALPHA * K_B; // ~3.72e-3

// Torsional (Dihedral) Strain [DERIVED from α² × K_B]
// V = V_TORSION / 2 * [1 + cos(n*phi - gamma)]
inline constexpr double V_TORSION = ALPHA * ALPHA * K_B; // ~2.71e-5

// Improper Torsion (Planarity) [DERIVED from α × K_B]
// V = K_IMPROPER * (omega)^2 / 2
inline constexpr double K_IMPROPER = ALPHA * K_B * 2.0;

// Thermostat coupling timescale [IMPOSED]
// Berendsen: lambda = sqrt(1 + dt/tau * (T_target/T_current - 1))
inline constexpr double THERMOSTAT_TAU_DEFAULT = 10.0; // in dt units

} // namespace ftd
