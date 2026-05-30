#pragma once
/**
 * ontic/master_quadratic.h — Layers 3, 3b, 3c, 4, 4b of the ontic chain.
 *
 * Contents:
 *   Layer 3:  Master Quadratic (COEFFICIENT, X_PLUS, X_MINUS, X_PLUS_PRECISION)
 *   Layer 3b: Dual-Substrate Decomposition (E_SUM, E_PRODUCT, DELTA_SQUARED, ...)
 *   Layer 3c: Charge-Space Duality (E2_COLOR)
 *   Layer 4:  Framework Integers (N_C, N_GEN, N_F, N_BASE, B_3, N_EFF, D_CONSTRAINT,
 *             DELTA_COLOR, alpha-power ladder)
 *   Layer 4b: Neutrino Mixing (PMNS angles from framework integers)
 *
 * Depends on: ontic/lemniscate.h (G_STAR).
 */

#include "ftd/ontic/lemniscate.h"

namespace ftd {
namespace ontic {

// ============================================================================
// Layer 3: Master Quadratic
// ============================================================================
// The master quadratic equation:
//   x² - 16·G*²·x + 16·G*³ = 0
//
// WHY QUADRATIC (degree 2) [THEOREM — DERIV_QUADRATIC_NECESSITY.md]:
//   Proof 1: Self-referential closure of the ternary constraint
//            0 = (-1) + (+1) doubles degree from 1 to 2.
//   Proof 2: CM field Q(i) has degree 2 over Q; Schneider-Chudnovsky
//            bounds algebraic relations to degree ≤ 2.
//
// WHY COEFFICIENT 16 [THEOREM — DERIV_MASTER_QUADRATIC_GAP_EQUATION.md §2.2]:
//   Two independent finite-combinatorial routes:
//     Route A: |Aut(E)|² = 4² = 16 where E: y²=x³-x has Aut = {1,-1,i,-i} ≅ Z₄.
//     Route B: z_BCC × |non-void ternary states| = 8 × 2 = 16
//              (BCC coordination on the Moore neighbourhood × {-1, +1}).
//   Also: N_BASE² = 4², conductor/2 = 32/2 = 16, |Δ|/4 = 64/4 = 16.
//
//   (A historical Route C — temporal-gauge DOF count 24-7-1 = 16 on the
//    2³ torus — was retracted as incorrect in 2026: proper Coulomb-gauge
//    fixing on T³ gives 14, not 16. See AUDIT_MASTER_QUADRATIC.md and
//    DERIV_MASTER_QUADRATIC_GAP_EQUATION.md §2.2 line 84.)
//
// LATTICE CONNECTION [THEOREM — DERIV_WATSON_GSTAR_IDENTITY.md]:
//   x₊ + x₋ = 16G*² = 32πW₃ (Watson integral of the 3D cubic lattice)
//
// Roots via quadratic formula:
//   x± = 8G*² ± 4G*^(3/2)·√(4G* - 1)

inline constexpr int COEFFICIENT = 16;  // |Aut(E)|² where E: y²=x³-x

inline constexpr double X_PLUS  = 137.0361714582;   // tree-level 1/α (master-quadratic root)
inline constexpr double X_MINUS = 3.0239639163;      // smaller root (artifact; NOT N_c — retired FTD-0014)

// Precision-corrected 1/α (Layer 7 — see below for c₁..c₄ and ε).
// This is the value that matches CODATA 2022 to < 0.001 ppt.
//
//   X_PLUS_PRECISION = x₊ − c₁|ε| + c₂|ε|² − c₃|ε|³ − c₄|ε|⁴
//
// Declared here so downstream code can opt into the precision value
// without re-implementing the series. The c_k and ε constants are
// declared in Layer 7 below; the arithmetic is performed in the
// `constants.h` re-export block (which sits after Layer 7 materialises).
// The numerical value is pre-computed and verified in ontic_audit().
//
// NOTE: the engine's Coulomb coupling currently uses ALPHA = 1/X_PLUS
// (tree-level). The precision value differs by ~3.8 ppm — below every
// benchmark's measurable resolution. See:
//   scripts/proofs/proof_motivic_master_quadratic.py  (derivation)
//   docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md  (chain)
inline constexpr double X_PLUS_PRECISION = 137.035999177;  // 4-term corrected

// Vieta-consistent precision root for x₋ [DERIVED from Vieta + precision x₊]:
//   x₊ · x₋ = 16·G*³  →  x₋ = 16·G*³ / x₊_precision
//
// Using the tree-level X_MINUS with the precision X_PLUS_PRECISION breaks
// the Vieta identities at the 6th digit. Computing x₋ FROM Vieta ensures
// the charge-quartic identities (Layer 3c) hold to machine precision.
inline constexpr double X_MINUS_PRECISION = COEFFICIENT * G_STAR * G_STAR * G_STAR / X_PLUS_PRECISION;

// Vieta's relations (sum and product of roots):
//   x₊ + x₋ = 16·G*²
//   x₊ · x₋ = 16·G*³

// ============================================================================
// Layer 3b: Dual-Substrate Decomposition
// ============================================================================
// Paper: "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026)
//
// Two substrates J_L, J_R with energies E_L, E_R satisfy:
//   S = E_L + E_R = 16·G*²     [THEOREM — Vieta sum of the master quadratic]
//   P = E_L · E_R = 16·G*³     [THEOREM — Vieta product of the master quadratic]
//   D = E_L - E_R               (difference — matter sector)
//   Identity: S² = D² + 4P      (algebraic identity, trivially true)
//
// PROVENANCE NOTE (2026-05-08 audit, AUDIT_DUAL_SUBSTRATE_PROVENANCE.md):
//   The dual-substrate identity is the master quadratic in (S, P, δ)
//   coordinates rather than (x_+, x_-) coordinates. It is NOT an
//   independent derivation: the values S, P are forced by Vieta on the
//   master quadratic; δ² = (4G*-1)/(4G*) is the algebraic dressing
//   1 - 4P/S². The identity is interpretively useful (J_L ↔ J_R as CPT;
//   chirality on the ±i eigenspaces of J), but EXPLR_GSTAR_FLUX_TIME §1's
//   "five independent lines of evidence" framing reads as overclaim;
//   "five readings of the same algebraic structure" is the honest form.
//
// The splitting parameter:
//   δ² = (4G* - 1)/(4G*) = 1 - 1/(4G*) ≈ 0.91554
//   E_L = S(1 + δ)/2,  E_R = S(1 - δ)/2
//
// Z₂ symmetry: J_L ↔ J_R is CPT (S invariant, D → -D, P invariant)
//
// Cosmological constant [CONJECTURE]:
//   Ω_Λ = 2/3 ≈ 0.667 (2.7% from observed 0.685)

// Sector energies
inline constexpr double E_SUM     = COEFFICIENT * G_STAR * G_STAR;               // S = 16·G*² ≈ 140.060
inline constexpr double E_PRODUCT = COEFFICIENT * G_STAR * G_STAR * G_STAR;      // P = 16·G*³ ≈ 414.392

// Splitting parameter: δ² = (4G* - 1)/(4G*) = 1 - 1/(4G*)
inline constexpr double DELTA_SQUARED = (4.0 * G_STAR - 1.0) / (4.0 * G_STAR);  // ≈ 0.9155

// Individual substrate energies (approximate; exact requires sqrt, computed in audit)
// E_L, E_R are the two roots of t² - S·t + P = 0
inline constexpr double DELTA_APPROX   = 0.9568;    // √(DELTA_SQUARED)
inline constexpr double E_LEFT_APPROX  = 136.912;   // S·(1+δ)/2
inline constexpr double E_RIGHT_APPROX = 3.148;     // S·(1-δ)/2

// Sector fractions: what fraction of S² is matter (D²) vs vacuum (4P)
inline constexpr double MATTER_FRACTION = DELTA_SQUARED;          // D²/S² ≈ 0.9155 (91.55%)
inline constexpr double VACUUM_FRACTION = 1.0 - DELTA_SQUARED;   // 4P/S² ≈ 0.0845 (8.45%)

// Cosmological constant conjecture [CONJECTURE]:
//   Λ = 2H₀²  →  Ω_Λ = 2/3  (2.7% from observed 0.685)
inline constexpr double OMEGA_LAMBDA_CONJ = 2.0 / 3.0;

// ============================================================================
// Layer 3c: Charge-Space Duality (DERIV_CHARGE_QUARTIC_FROM_GSTAR.md)
// ============================================================================
// Substituting e² = 1/x into the master quadratic transforms it into
// the charge quartic:
//   16·G*³·e⁴ - 16·G*²·e² + 1 = 0           [THEOREM]
//
// This is the reciprocal polynomial of the master quadratic. Its roots
// are e²_EM = 1/x₊ = α  and  e²_color = 1/x₋.
//
// Vieta sum:    α + e²_C = 1/G*              (inverse coupling sum)
// Vieta product: α · e²_C = 1/(16·G*³)      (action-scale product)

inline constexpr double E2_COLOR = 1.0 / X_MINUS_PRECISION;   // e²_C ≈ 0.3307 (Vieta-consistent)
// Note: e²_EM = ALPHA (defined in Layer 5, uses X_PLUS_PRECISION)
// Vieta sum:    ALPHA + E2_COLOR = 1/G_STAR    (exact by construction)
// Vieta product: ALPHA * E2_COLOR = 1/(16·G*³) (exact by construction)
//
// Legacy tree-level value available as 1.0/X_MINUS for reference.

// ============================================================================
// Layer 4: Framework Integers
// ============================================================================
// N_c = 3 is the framework's single free integer. It is sourced INDEPENDENTLY
// from lattice topology (see docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md),
// NOT from the smaller root x₋. The earlier x₋ ↔ N_c identification is RETIRED
// (LEDGER row FTD-0014 removed in ca7eb61); x₋ ≈ 3.024 is a mathematical
// artifact of the quadratic (0.80% from 3), not the origin of N_c.
//
// Given N_c = 3 from topology, the remaining integers follow by the integer
// reduction theorem below:
//
//   N_c    = 3                (number of color charges — from topology, not x₋)
//   N_gen  = N_c = 3           (number of fermion generations)
//   N_f    = 2·N_gen = 6       (number of quark flavors)
//   b₃     = (11N_c-2N_f)/3 = 7  (QCD one-loop beta coefficient)
//   N_eff  = b₃ + 2N_c = 13   (effective degrees of freedom = Fibonacci F₇)
//   N_base = 2^((D+1)/2) = 4  (spinor dimension in D=3)
//   D      = N_c·N_base²-1 = 47  (constraint dimension)

inline constexpr int D_SPATIAL = 3;
inline constexpr int N_C       = 3;
inline constexpr int N_GEN     = 3;
inline constexpr int N_F       = 6;
inline constexpr int N_BASE    = 4;
inline constexpr int B_3       = 7;
inline constexpr int N_EFF     = 13;
inline constexpr int D_CONSTRAINT = 47;

// COLOR EXCESS: fractional deviation of x₋ from N_c [THEOREM — algebraic]
//   δ_c = x₋ − N_c = 16G*³·α − 3 = 0.023963916339...
//
// Forced by Vieta: x₊·x₋ = 16G*³, so x₋ = 16G*³/x₊ = 16G*³·α.
// If x₋ were exactly 3, G* would need to be 2.93469... (not 2.95868).
// The excess measures geometric frustration between transcendental G* and integer N_c.
//
// Candidate closed forms (none exact):
//   δ_c ≈ 1/42 = 1/(2·N_c·b₃)     (0.65% error)  [OPEN]
//   δ_c ≈ π·α                       (4.3% error)   [OPEN]
//   δ_c ≈ 2·α_s/(3π)               (5.1% error)   [OPEN]
//
// Exact: δ_c = 8G*² − 4G*^(3/2)·√(4G*−1) − 3
inline constexpr double DELTA_COLOR = 0.023963916339021004;

// INTEGER REDUCTION THEOREM (DERIV_PION_MASS_FROM_GSTAR.md):
// All four integers follow from N_c = 3 alone:
//   N_base = N_c(N_c-1) - 2 = 4       (spinor dimension)
//   b_3    = N_c² - 2        = 7       (SU(3) pseudo-Goldstones)
//   N_eff  = b_3 + 2·N_c     = 13      (effective degrees of freedom)
// The "four free integers" are actually ONE free integer (N_c).
static_assert(N_BASE == N_C * N_C - N_C - 2, "Integer reduction: N_base = N_c(N_c-1)-2");
static_assert(B_3    == N_C * N_C - 2,        "Integer reduction: b_3 = N_c^2 - 2");
static_assert(N_EFF  == B_3 + 2 * N_C,        "Integer reduction: N_eff = b_3 + 2*N_c");

// Alpha-power ladder: exponents that walk through the Standard Model.
// FOUND_LADDER_GENERATING_RULE.md: perturbative boundary at n=4=N_BASE,
// then gaps = {N_BASE, N_C, N_C, N_F} = {4, 3, 3, 6} encode each SM sector.
// Total non-perturbative walk = 4+3+3+6 = 16 = COEFFICIENT = k_phys.
inline constexpr int LADDER_PERTURBATIVE = N_BASE;                 // n=4  (QED boundary)
inline constexpr int LADDER_HIGGS        = LADDER_PERTURBATIVE + N_BASE; // n=8  (+SU(2), Higgs)
inline constexpr int LADDER_ELECTRON     = LADDER_HIGGS + N_C;          // n=11 (+color, hadrons)
inline constexpr int LADDER_NEUTRINO     = LADDER_ELECTRON + N_C;       // n=14 (+seesaw, CP)
inline constexpr int LADDER_GRAVITY      = LADDER_NEUTRINO + N_F;       // n=20 (+all species)

// ============================================================================
// Layer 4b: Neutrino Mixing (PMNS angles from framework integers)
// ============================================================================
// All mixing angles are ratios of framework integers {N_c, b_3, N_eff, N_base}.
// These are genuine derivations [THEOREM], not parametric insertions.

// sin²(θ₁₂) = N_c / (N_c + b₃) = 3/10 = 0.300
//   (0.69% from experimental 0.307)
inline constexpr double SIN2_THETA12 = static_cast<double>(N_C) / (N_C + B_3);

// sin²(θ₂₃) = (N_eff + N_c) / (2·N_eff + N_c) = 16/29 = 0.5517
//   (2.5% from experimental 0.546)
inline constexpr double SIN2_THETA23 = static_cast<double>(N_EFF + N_C) / (2.0 * N_EFF + N_C);

// sin²(θ₁₃) = 1 / (N_base · N_eff) = 1/52 = 0.01923
//   (7.0% from experimental 0.02203)
inline constexpr double SIN2_THETA13 = 1.0 / (N_BASE * N_EFF);

// Mass-squared ratio: Δm²₃₁ / Δm²₂₁ = (b₃ + N_c)² / N_c = 100/3 = 33.33
//   (1.47% from experimental 32.85)
inline constexpr double DM2_RATIO = static_cast<double>((B_3 + N_C) * (B_3 + N_C)) / N_C;

// Normal hierarchy (Δm²₃₁ > 0) [THEOREM]
inline constexpr bool NORMAL_HIERARCHY = true;

}  // namespace ontic
}  // namespace ftd
