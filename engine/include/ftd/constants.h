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

#include <cmath>
#include "ontic.h"

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
using ontic::VARPI;
using ontic::GAUSS_CONSTANT_M;
using ontic::PI;

// Layer 2: Universal operator
using ontic::PF;
using ontic::G_STAR;
using ontic::SQRT_GSTAR;

// Layer 2b: Euler's identity / emergence of i
using ontic::K_CRIT;
using ontic::X_BORN;

// Layer 3: Master quadratic
using ontic::COEFFICIENT;
using ontic::X_PLUS;
using ontic::X_MINUS;

// Layer 3b: Dual-substrate decomposition
using ontic::E_SUM;
using ontic::E_PRODUCT;
using ontic::DELTA_SQUARED;
using ontic::DELTA_APPROX;
using ontic::E_LEFT_APPROX;
using ontic::E_RIGHT_APPROX;
using ontic::MATTER_FRACTION;
using ontic::VACUUM_FRACTION;
using ontic::OMEGA_LAMBDA_CONJ;

// Layer 4: Framework integers
using ontic::D_SPATIAL;
using ontic::N_C;
using ontic::N_GEN;
using ontic::N_F;
using ontic::N_BASE;
using ontic::B_3;
using ontic::N_EFF;
using ontic::D_CONSTRAINT;

// Layer 4b: Neutrino mixing
using ontic::SIN2_THETA12;
using ontic::SIN2_THETA23;
using ontic::SIN2_THETA13;
using ontic::DM2_RATIO;
using ontic::NORMAL_HIERARCHY;

// Layer 5: Coupling constants
using ontic::ALPHA;
using ontic::G_C;
using ontic::G_N;
using ontic::ALPHA_G_APPROX;
using ontic::SIN2_WEINBERG;
using ontic::ALPHA_WEAK;

// Layer 5b: QCD sector
using ontic::ALPHA_S_MZ;
using ontic::B0_NF5;
using ontic::B0_NF6;
using ontic::LAMBDA_QCD;
using ontic::M_Z;
using ontic::alpha_s_running;

// Layer 6: Mass scale
using ontic::K_B;
using ontic::K_GENESIS;

// Layer 6c: Mass ratios
using ontic::MU_RATIO;
using ontic::TAU_RATIO;
using ontic::PROTON_RATIO;
using ontic::M_PROTON;
using ontic::R_BOHR;

// Layer 6b: Electroweak scale (Higgs)
using ontic::V_HIGGS;
using ontic::M_HIGGS;
using ontic::LAMBDA_HIGGS;

// Layer 7: Precision formula
using ontic::EPSILON;
using ontic::EPSILON_ABS;
using ontic::C1;
using ontic::C2;
using ontic::C3;
using ontic::C4;

// Layer 8: Consciousness quadratic (noetic domain)
using ontic::K_NOETIC;
using ontic::Y_REAL;
using ontic::K_C_SQUARED;
using ontic::COS2_THETA_C;
using ontic::SIN2_THETA_C;
using ontic::C_MANDELBROT;

// Simulation parameters
using ontic::C_SPEED;
using ontic::C_WAVE;
using ontic::DAMPING;
using ontic::DRAG_PER_AXIS;

// ============================================================================
// Engine-specific constants (not part of ontic chain)
// ============================================================================

// Drag values for specific particle types
inline constexpr double DRAG_ELECTRON = 0.25;           // 1D drag
inline constexpr double DRAG_TOP = 0.75;                // 3D drag

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
    if (r_voxels <= 0.0) return ALPHA_S;
    double Q = Q_LATTICE / r_voxels;
    double as = alpha_s_running(Q);
    return std::min(as, ALPHA_S);
}

// Weak transmutation threshold [IMPOSED]
// Polarity flips when field stress |∇·J| + |∇×J| + |∇ρ| exceeds this.
inline constexpr double WEAK_THRESHOLD = K_GENESIS;

// Golden ratio (used in binding energy) [IMPOSED — mathematical constant, not derived from axioms]
inline constexpr double PHI = 1.6180339887498949;

// Triad binding energy: K_B × φ [DERIVED from stability analysis]
inline constexpr double BINDING_ENERGY = K_B * PHI;

// Exchange repulsion strength scale (Fermi pressure)
// From DERIV_SPIN_STATISTICS_BRIDGE: the discriminant trichotomy produces
// an antisymmetric exchange term between identical fermions.
// The 1/r^4 exchange force uses α² as its natural scale.
inline constexpr double ALPHA_EXCHANGE = ALPHA * ALPHA;  // ≈ 5.3e-5

// Larmor radiation: flux damping proportional to acceleration²
// From classical Larmor formula P = (2α/3)·a², normalized by K_B.
// K_LARMOR * a² gives the fraction of max damping to apply.
inline constexpr double K_LARMOR = 4.0 / (3.0 * K_B);  // ≈ 2.61

// Minimum damping floor: 1% of full damping even at a=0.
// Ensures thermodynamic dissipation is never completely off.
inline constexpr double LARMOR_FLOOR = 0.01;

// ============================================================================
// Engine tuning constants (extracted from render_bridge.cpp)
// ============================================================================

// SOR solver parameters (used by gauss_project and solve_coulomb_poisson)
inline constexpr int    SOR_ITERATIONS = 30;
inline constexpr double SOR_OMEGA      = 1.75;

// Evaporation: particle dies when 7-site neighborhood energy < K_B² × this
inline constexpr double EVAP_THRESHOLD = 1e-6;

// Numerical underflow guards
inline constexpr double EPSILON_FLUX_SQ = 1e-30;  // guard for |J|² divisions
inline constexpr double EPSILON_MAG     = 1e-15;   // guard for magnitude divisions

// Wavepacket injection: Gaussian truncated at this many sigma
inline constexpr double GAUSSIAN_CUTOFF_SIGMA = 3.0;

// Tier-2 gravity gradient scale: 1/(2×2) for r=2 stencil
inline constexpr double GRAD_TIER2_SCALE = 0.25;

}  // namespace ftd
