#pragma once
/**
 * ontic/gauge_couplings.h — Layers 5, 5b, 7 and simulation parameters.
 *
 * Contents:
 *   Layer 5:  Coupling Constants (ALPHA, ALPHA_TREE, ALPHA_PRECISION,
 *             G_C, SIN2_WEINBERG, ALPHA_WEAK, G_N, ALPHA_G_APPROX)
 *   Layer 5b: QCD Sector (ALPHA_S_MZ, B0_NF5/6, LAMBDA_QCD, M_Z)
 *   Layer 7:  Precision Formula (EPSILON, C1-C4)
 *   Simulation Parameters: C_SPEED, C_WAVE, DAMPING, DRAG_PER_AXIS
 *   QCD running coupling: alpha_s_running() (declared; defined in
 *                         src/eft/qcd_one_loop_perturbative.cpp)
 *                         [IMPOSED] — perturbative-QCD formula, NOT
 *                         lattice-measured; Phase-2 of the EFT Recovery
 *                         Program will measure β(g) directly from blocking.
 *   ontic_audit(): declared; defined in src/ontic_audit.cpp
 *
 * Depends on: ontic/lemniscate.h (PI), ontic/master_quadratic.h (X_PLUS,
 *             X_PLUS_PRECISION, N_C, N_EFF, N_BASE, N_F, B_3).
 */

#include "ftd/ontic/lemniscate.h"
#include "ftd/ontic/master_quadratic.h"

namespace ftd {
namespace ontic {

// ============================================================================
// Layer 5: Coupling Constants
// ============================================================================
// Fine structure constant [THEOREM]: α = 1 / X_PLUS_PRECISION.
//
// As of the 2026-04-17 precision rollout (TRACKER §1.5), the engine
// uses the 4-term-corrected value from the master quadratic, which
// matches CODATA 2022 (137.035999177) to < 0.001 ppt. Tree-level
// x₊ remains available via ALPHA_TREE for reference comparisons.
//
//   ALPHA           = 1 / X_PLUS_PRECISION = 1/137.035999177   (engine)
//   ALPHA_TREE      = 1 / X_PLUS           = 1/137.0361714582  (reference)
//   ALPHA_PRECISION = alias to ALPHA (kept for docs/scripts that
//                     reference the old name explicitly)
//
// All downstream constants that depend on ALPHA (G_C, DAMPING,
// ALPHA_EFT, ALPHA_EXCHANGE, H_BOND_EPSILON, K_ANGLE, V_TORSION,
// K_IMPROPER) inherit the precision value automatically via their
// constexpr derivations.
inline constexpr double ALPHA           = 1.0 / X_PLUS_PRECISION;
inline constexpr double ALPHA_TREE      = 1.0 / X_PLUS;
inline constexpr double ALPHA_PRECISION = ALPHA;

// State-flux coupling g_c — derivation chain (honestly traced):
//
//   STEP 1 [THEOREM, algebraic]:
//     The master quadratic x² − 16G*²x + 16G*³ = 0 has root
//     x_+ = X_PLUS_PRECISION (4-term-corrected).
//     This is pure algebra over the lemniscatic ring.
//
//   STEP 2 [STRONGLY MOTIVATED CONJECTURE, physical match]:
//     Identify x_+ with the inverse fine-structure constant: α ≡ 1/x_+.
//     Evidence: CM-curve uniqueness at class number 1 selects this
//     λ-modular fixed point, and the adversarial look-elsewhere scan
//     (FTD-0189) finds the master quadratic to be the unique dual-matcher.
//     (The smaller root x_- ≈ 3.024 is a mathematical artifact; its
//     identification with N_c is RETIRED — FTD-0014 removed in ca7eb61.
//     N_c = 3 comes independently from topology, see DERIV_NC_FROM_TOPOLOGY.md.)
//     See LEDGER FTD-0013.
//
//   STEP 3 [SELECTION, Lagrangian ansatz]:
//     Adopt the coupling term L_coupling = −g_c · s · (∇·J).
//     The Wilsonian matching condition for this ansatz at the
//     classical / tree level forces g_c² = α.
//     Therefore G_C ≡ √α = √(1/X_PLUS_PRECISION).
//
//   STEP 4 [OPEN, Mechanism B]:
//     A first-principles derivation of g_c from lattice→continuum
//     matching of the static charge–charge potential (without
//     assuming step 3's ansatz) is the load-bearing open problem
//     for upgrading FTD from "Wilsonian-shaped" to "Wilsonian EFT".
//     See SPEC_EFT_RECOVERY_PROGRAM.md Phase 2 and FTD-0031.
//
// Numerical value: √(1/137.035999177) = 0.085424543102854...
// Hardcoded as a literal because C++17 forbids constexpr std::sqrt.
// Runtime equality with std::sqrt(ALPHA) is asserted in ontic_audit();
// the compile-time static_assert in constants.h verifies G_C² ≈ ALPHA.
inline constexpr double G_C = 0.0854245431028543695;

// Weinberg angle: sin²θ_W = N_c / N_eff = 3/13 [THEOREM]
//   = 0.23077 (0.2% from experimental 0.23122)
inline constexpr double SIN2_WEINBERG = static_cast<double>(N_C) / N_EFF;

// Weak coupling constant: α_W = α / sin²θ_W [DERIVED]
inline constexpr double ALPHA_WEAK = ALPHA / SIN2_WEINBERG;

// ══════════════════════════════════════════════════════════════════════
// GRAVITY REGIME BANNER
// ══════════════════════════════════════════════════════════════════════
// The engine runs in a TOY-GRAVITY REGIME where G_N ≈ 0.01 — roughly
// 37 orders of magnitude stronger than physical gravity (α_G ≈ 5.9 × 10⁻³⁹).
//
// This is a deliberate simulation choice: on a 32³–128³ lattice, physical
// gravity is so weak that nothing gravitational would ever be visible
// within the observable tick budget. By running at EM-comparable strength
// we can see wells, tidal effects, and orbital motion form in tens of ticks
// instead of 10³⁷ ticks.
//
// IMPLICATIONS for anyone citing an engine "gravity" result:
//   1. Shapes (wells, horizons, geodesics) are QUALITATIVELY correct.
//   2. Quantitative scales are NOT physical. "Time dilation X%" is
//      time dilation at strong-field lattice parameters, not Earth-surface
//      or cosmological parameters.
//   3. α_G (ALPHA_G_APPROX ≈ 5.9e-39) is the PHYSICAL coupling — cite
//      this when the derivation is what matters. G_N (0.01) is the
//      TOY coupling — cite this only with "lattice toy" framing.
//   4. Benchmarks comparing engine to real GR must either (a) state
//      "weak-field, strong-coupling toy regime" or (b) use α_G and accept
//      the sim won't visibly move.
//
//   G_N = 1 / (b₃ + N_c)² = 1/(7+3)² = 1/100 = 0.01
//
// The ratio (b₃+N_c) = 10 is the total number of gauge + colour charges;
// why this particular ratio is the lattice gravity coupling is argued
// from dimensional analysis in EXPLR_LATTICE_GRAVITY_SCALE.md. The
// derivation is more like [SELECTION] than [THEOREM].
// ══════════════════════════════════════════════════════════════════════
inline constexpr double G_N = 1.0 / ((B_3 + N_C) * (B_3 + N_C));

// Physical gravitational coupling (dimensionless):
//   α_G = 2π·(16/3)²·(N_eff + 3/b₃)²·α²⁰ ≈ 5.91 × 10⁻³⁹
//
// The α²⁰ exponent (20 = N_eff + b₃ = 13 + 7) is the cross-domain penalty:
//   EM/Strong: couple spatial→spatial (same domain)     → strength ~ α
//   Gravity:   couples spatial→temporal (cross-domain)   → strength ~ α²⁰
// This explains why G_N(lattice) = 0.01 > α = 0.0073:
//   On the lattice, both forces are same-domain. The physical hierarchy
//   α_G/α ~ 10⁻³⁷ only appears after the α²⁰ bridge to physical units.
//
// See: FOUND_SPACETIME_EMERGENCE.md §10.2, AUDIT_NOVEL_PREDICTIONS.md §E2
//
// Cannot use constexpr with std::pow, so compute in ontic_audit().
// Approximate value for reference:
inline constexpr double ALPHA_G_APPROX = 5.91e-39;

// ============================================================================
// Layer 5b: QCD Sector
// ============================================================================
// QCD coupling at M_Z scale [THEOREM]:
//   α_s(M_Z) = b₃ / (b₃ + 4·N_eff) = 7 / 59 = 0.11864
//   (0.6% from experimental 0.1179)
inline constexpr double ALPHA_S_MZ = static_cast<double>(B_3) / (B_3 + 4.0 * N_EFF);

// QCD beta function one-loop coefficient: b₀ = (11·N_c - 2·n_f) / 3
// For 5 active flavors at M_Z: b₀ = (33 - 10)/3 = 23/3
inline constexpr double B0_NF5 = (11.0 * N_C - 2.0 * 5) / 3.0;   // 23/3 ≈ 7.667

// For all 6 flavors: b₀ = (33 - 12)/3 = 7 (= b₃, by construction)
inline constexpr double B0_NF6 = (11.0 * N_C - 2.0 * N_F) / 3.0;  // 7

// Λ_QCD (from 2-loop matching at M_Z) [SELECTION]
inline constexpr double LAMBDA_QCD = 0.215;  // GeV

// M_Z for scale reference [EXTERNAL INPUT]
inline constexpr double M_Z = 91.1876;  // GeV

// ============================================================================
// Layer 7: Precision Formula (radiative corrections)
// ============================================================================
// The modular deviation ε connects the lemniscate nome to framework integers:
//   ε = e^π - π - (b₃ + N_eff) = e^π - π - 20 ≈ -0.000900021
//
// The 4-term corrected inverse fine structure constant:
//   1/α = x₊ - c₁|ε| + c₂|ε|² - c₃|ε|³ - c₄|ε|⁴
//
// where each coefficient is a ratio of framework integers:
//   c₁ = N_c²/D         = 9/47
//   c₂ = (N_eff-2N_base)/N_base³ = 5/64
//   c₃ = N_base/(N_c·D) = 4/141
//   c₄ = (N_c·D)/(b₃+N_base) = 141/11
//
// Result: 137.035999177... matches CODATA 2022 to < 0.001 ppt.

inline constexpr double EPSILON = -0.0009000208;
inline constexpr double EPSILON_ABS = 0.0009000208;

inline constexpr double C1 = 9.0 / 47.0;     // N_c²/D
inline constexpr double C2 = 5.0 / 64.0;     // (N_eff-2N_base)/N_base³
inline constexpr double C3 = 4.0 / 141.0;    // N_base/(N_c·D)
inline constexpr double C4 = 141.0 / 11.0;   // (N_c·D)/(b₃+N_base)

// ============================================================================
// Simulation Parameters (discretization + imposed)
// ============================================================================

// Speed limit: nothing outruns light [DERIVED]
// Previously C_SPEED = 1.0 (axiomatic). Now unified with C_WAVE:
// particles and waves share the same causal speed limit c = 1/√3.
inline constexpr double C_SPEED = 0.57735026918962576451;  // = C_WAVE = 1/sqrt(3)

// Speed of light: maximum stable wave propagation speed on the 3D cubic lattice.
// DERIVED from CFL stability for d²J/dt² = c²∇²J with 6-neighbor Laplacian:
//
//   c² · (2D/h²) ≤ 2/dt²   (von Neumann stability)
//   c² ≤ 1/D                (with h = dt = 1)
//   c = 1/√D = 1/√3         (for D = 3 spatial dimensions)
//
// Not a free parameter: uniquely determined by {D=3, cubic lattice, leapfrog}.
// This is the CFL limit — the same constraint as in FDTD electromagnetics.
inline constexpr double C_WAVE = 0.57735026918962576451;  // 1/sqrt(3) [DERIVED]

// Damping rate: γ = α [IMPOSED — identification γ = α is a parameter choice (ASSUMP.6)]
//
// The dissipation rate is set equal to the fine structure constant.
// Motivation (not derivation):
//   - Manifested particles "negotiate" discrete lattice geometry each tick
//   - Energy loss = geometric mismatch between continuous flux and discrete lattice
//   - The coupling strength g_c = √α governs the state-flux interaction
//   - Self-consistency of lattice thermal equilibrium suggests γ = α
//
// Per CLAUDE.md ASSUMP.6: this identification is motivated by the observation
// that EM coupling governs irreversible transitions, but it is NOT derived
// from first principles — it is imposed.
// See: EXPLR_VACUUM_DRAG_DERIVATION.md, SPEC_SIX_ALGORITHMS.md (Algorithm 5)
inline constexpr double DAMPING = ALPHA;  // γ = α = 0.00729... [IMPOSED]

// Drag: rounding cost per axis = 1/N_BASE [DERIVED]
inline constexpr double DRAG_PER_AXIS = 1.0 / N_BASE;

// ============================================================================
// QCD Running Coupling Function
// ============================================================================
// α_s(Q) = 4π / (b₀ · ln(Q²/Λ²))  [one-loop running]
// Valid for 5 active flavors: m_b < Q < m_t
// Returns 1.0 in the non-perturbative regime (Q ≤ Λ_QCD).
//
// Definition lives in src/eft/qcd_one_loop_perturbative.cpp (renamed
// 2026-04-19 from src/ontic_running_coupling.cpp — ticket O2 pure-header +
// EFT Recovery Program Phase 0 honest-tagging rename). [IMPOSED].
double alpha_s_running(double Q_GeV);

// ============================================================================
// Ontic Audit: Print and verify the full derivation chain
// (implementation lives in src/ontic_audit.cpp)
// ============================================================================

// Returns the number of failures (0 = all pass).
int ontic_audit();

}  // namespace ontic
}  // namespace ftd
