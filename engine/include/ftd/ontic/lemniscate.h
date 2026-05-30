#pragma once
/**
 * ontic/lemniscate.h — Layers -1 through 2b of the ontic chain.
 *
 * Contents:
 *   Layer -1: Self-Referential Seed (EULER_E)
 *   Layer 0:  Transcendental Seeds (EULER_GAMMA, GAMMA_QUARTER)
 *   Layer 0b: Modular Selection (NOME_LEMNISCATIC, THETA_LEMNISCATIC)
 *   Layer 1:  Elliptic Geometry (VARPI, GAUSS_CONSTANT_M)
 *   Layer 2:  Universal Operator (G_STAR, PI, PF, I_1_BCC/W_3, SQRT_GSTAR, GSTAR_FLUX/TIME/ACTION)
 *   Layer 2b: Euler's Identity and emergence of i (K_CRIT, X_BORN)
 *
 * No dependencies on other ontic/ headers.
 */

namespace ftd {
namespace ontic {

// ============================================================================
// Layer -1: Self-Referential Seed
// ============================================================================
// Euler's number e is the eigenvalue of differentiation: d/dx(e^x) = e^x.
// It is the unique real number whose growth rate equals its current value —
// the fixed point of the operation "grow proportionally to what exists."
//
// All subsequent transcendentals require e for their definition:
//   γ = lim(n→∞) [∑1/k - ln(n)]   (ln uses e)
//   Γ(1/4) via Weierstrass product  (uses e^{γz})
//   q = e^{-ϖ/M}                   (nome, the modular selector)

inline constexpr double EULER_E = 2.718281828459045235360;

// ============================================================================
// Layer 0: Transcendental Seeds
// ============================================================================
// The Euler-Mascheroni constant γ is the regularized harmonic sum:
//   γ = lim(n→∞) [∑(k=1..n) 1/k - ln(n)] = 0.5772156649...
//
// It connects to the gamma function via the Weierstrass product:
//   1/Γ(z) = z·e^(γz) · ∏(n=1..∞) [(1+z/n)·e^(-z/n)]
//
// At z=1/4, this gives Γ(1/4) = 3.6256099082...
// γ is the seed from which all elliptic structure grows.

inline constexpr double EULER_GAMMA = 0.57721566490153286;

// Gamma(1/4): the gateway from arithmetic (γ) to geometry (ϖ)
inline constexpr double GAMMA_QUARTER = 3.6256099082219083;

// ============================================================================
// Layer 0b: Modular Selection
// ============================================================================
// The lemniscatic nome q = e^{-ϖ/M} = e^{-π} ≈ 0.04321 selects the
// lemniscatic curve (k = 1/√2, the self-dual point) from the continuous
// family of elliptic curves parameterized by the nome.
//
// Pre-computed; verified as e^{-ϖ/M} in ontic_audit().
inline constexpr double NOME_LEMNISCATIC = 0.04321391826377225;

// Jacobi theta-null at the lemniscatic point:
//   θ₃(0, q) = 1 + 2q + 2q⁴ + 2q⁹ + ...   (lattice counting function)
//
// Exact identities:
//   θ₃² = √2·M                    (connects theta to Gauss's constant)
//   θ₃  = π^{1/4} / Γ(3/4)       (connects theta to gamma function)
//   θ₃  = π^{1/4}·Γ(1/4)/(π√2)  (via reflection: Γ(1/4)·Γ(3/4) = π√2)
//
// Pre-computed; verified against both series and exact formula in ontic_audit().
inline constexpr double THETA_LEMNISCATIC = 1.08643481121331;

// ============================================================================
// Layer 1: Elliptic Geometry
// ============================================================================
// The lemniscate constant ϖ (varpi) is the half-period of the lemniscate
// of Bernoulli r² = cos(2θ). It is to the lemniscate what π is to the circle.
//
//   ϖ = Γ(1/4)² / (2√(2π))
//
// This single constant encodes the geometry of self-intersection — the
// figure-8 curve that is the simplest closed curve crossing itself.

inline constexpr double VARPI = 2.622057554292119810;

// Gauss's constant M = 1/AGM(1, √2)
// where AGM is the arithmetic-geometric mean.
// ϖ and M are related: ϖ = π·M  (verified in audit)
inline constexpr double GAUSS_CONSTANT_M = 0.8346268416740731;

// ============================================================================
// Layer 2: Universal Operator
// ============================================================================
// The Universal Render Bridge constant G*:
//   G* = 2√(ϖ·M)               (π-free: scaled geometric mean)
//   G* = 2ϖ/√π                 (equivalent, using π = ϖ/M)
//
// G* bridges the elliptic (ϖ) and the arithmetic-geometric (M).
// It is the fundamental constant of the render bridge: the geometric
// mean of the lemniscate period and Gauss's constant, scaled by 2.
//
// THE DIMENSIONAL TRIAD [SELECTION]:
//   G*¹ = 2.959  →  FLUX      (J: spatial amplitude per DoF)
//   G*² = 8.754  →  ENERGY    (E: temporal amplitude per DoF, = time)
//   G*³ = 25.90  →  ACTION    (S: spatiotemporal record per DoF)
//
// From Vieta: Sum/16 = G*² (energy), Product/16 = G*³ (action),
// P/S = G* = action/energy = time per DoF. G* is simultaneously
// the natural flux amplitude AND the natural time unit, with G*²
// being their shared energy. The observable ψ = J_L + J_R = G*
// exactly (from dual substrate), confirming G* IS the flux.
//
// G* = HM(1/α, N_c)/2 — half the harmonic mean of the physics roots.
//
// See: EXPLR_GSTAR_FLUX_TIME.md
//
// Pre-computed from G* = 2√(ϖ·M). Verified in ontic_audit().
inline constexpr double G_STAR = 2.958675119188639;

// π DERIVED from the ontic chain [THEOREM]:
//   G* = 2ϖ/√π  →  √π = 2ϖ/G*  →  π = 4ϖ²/G*²
//
// This makes ϖ ontologically prior to π. The lemniscate constant
// (encoding self-intersection geometry) is more fundamental than
// the circle constant (encoding rotational symmetry).
inline constexpr double PI = 4.0 * VARPI * VARPI / (G_STAR * G_STAR);

// Packing fraction: the ratio of inscribed circle to enclosing square
// on each face of the cubic lattice. PF = π/4 = ϖ²/G*²
inline constexpr double PF = PI / 4.0;

// THE WATSON-G* IDENTITY [THEOREM — DERIV_WATSON_GSTAR_IDENTITY.md]:
//   I₁ = G*²/(2π) = Γ(1/4)⁴/(4π³)
//
// I₁ is Watson's first triple integral (Watson, 1939): the self-energy
// of the BCC (body-centered cubic) lattice. In FTD's 26-neighbor Moore
// neighborhood, this corresponds to the 8 CORNER neighbors at (±1,±1,±1).
//
// The Moore neighborhood decomposes into three sublattices:
//   SC  (6 face neighbors):   I₃ ≈ 0.506, CM field Q(√-6), Γ(n/24)
//   FCC (12 edge neighbors):  I₂ ≈ 0.446, CM field Q(√-3), Γ(1/3)
//   BCC (8 corner neighbors): I₁ = 1.393, CM field Q(i),   Γ(1/4) = G*
//
// G* connects to the BCC component because the 8 cube vertices have
// Z₄ rotational symmetry matching the lemniscate curve Aut(E) = {1,-1,i,-i}.
//
// Key consequence: x₊ + x₋ = 16G*² = 32πI₁
//
inline constexpr double I_1_BCC = G_STAR * G_STAR / (2.0 * PI);
// Legacy alias (documents may reference W_3):
inline constexpr double W_3 = I_1_BCC;

// The time operator: √G*
// Each G*-tick divides into two √G* sub-ticks (Read and Write phases).
inline constexpr double SQRT_GSTAR = 1.720079974649039;

// G* Dimensional Triad (EXPLR_GSTAR_FLUX_TIME.md):
//   G*¹ = 2.959  → FLUX:   spatial amplitude per DoF
//   G*² = 8.754  → TIME:   energy = temporal amplitude per DoF
//   G*³ = 25.90  → ACTION: spatiotemporal record per DoF
//
// Key identity: P/S = (x₊·x₋)/(x₊+x₋) = G*³·16/(G*²·16) = G*
// G* is half the harmonic mean of 1/α and N_c.
inline constexpr double GSTAR_FLUX   = G_STAR;                      // G*¹ per DoF
inline constexpr double GSTAR_TIME   = G_STAR * G_STAR;             // G*² per DoF
inline constexpr double GSTAR_ACTION = G_STAR * G_STAR * G_STAR;    // G*³ per DoF

// ============================================================================
// Layer 2b: Euler's Identity and the Emergence of i
// ============================================================================
// Euler's identity:  e^{iπ} + 1 = 0
//
// Every symbol is in the ontic chain except i. The imaginary unit is not
// postulated — it EMERGES from G* via the generalized master quadratic:
//
//   x² - k·G*²·x + k·G*³ = 0
//   Discriminant Δ = k·G*³·(k·G* - 4)
//
// DISCRIMINANT TRICHOTOMY — one quadratic, three regimes:
//   k·G* > 4  (k=16):    Δ > 0  →  REAL roots     (bosons: coupling constants α, N_c)
//   k·G* = 4  (k=4/G*):  Δ = 0  →  degenerate     (measurement / Born rule)
//   k·G* < 4  (k=1/2):   Δ < 0  →  COMPLEX roots  (fermions: Dirac equation from e^{ibt})
//
// The fermion sector is DERIVED, not imported: complex roots oscillate as
// e^{ibt}, which IS the spinor wavefunction evolution (Dirac equation).
// Bosons, fermions, and measurement all emerge from the same quadratic.
//
// The critical coefficient k_crit = 4/G* is the boundary where i appears.
// Below this threshold, self-reference forces the algebra out of R into C.
//
// NULL CONE GEOMETRY [THEOREM — FOUND_BORN_RULE_NULL_CONE.md]:
//   At the critical point, the Born rule emerges from the null-cone
//   equation i² + a² + b² = 0. This single equation simultaneously
//   encodes: the unit circle (U(1) phase), the Pythagorean theorem,
//   the Riemann sphere (spinors), and the Wick rotation between
//   Euclidean (probability) and Lorentzian (causality) signatures.
//   P = |ψ|² is quadratic because it IS the null-cone's quadratic form.
//
// The ternary states {-1, 0, +1} map to complex geometry:
//   +1  =  e^{i·0}     (zero rotation: matter)
//   -1  =  e^{iπ}      (half rotation: antimatter)
//    0  =  origin       (center of rotation: void)
//
// Euler's identity is the ANNIHILATION equation:
//   e^{iπ} + 1 = 0  ↔  (-1) + (+1) = 0  ↔  antimatter + matter = void
//
// The lemniscatic nome encodes a striking corollary:
//   q = (-1)^i = (e^{iπ})^i = e^{i²π} = e^{-π}
//   "Antimatter raised to the power of reference frame context = modular selector"

// Critical coefficient: the boundary between real and complex domains.
// k_crit = 4/G* ≈ 1.352 — where i emerges from the quadratic structure.
inline constexpr double K_CRIT = 4.0 / G_STAR;

// Degenerate root at the critical point: x = k_crit·G*²/2 = 2·G*
inline constexpr double X_BORN = 2.0 * G_STAR;

// Nome as (-1)^i: verified numerically as e^{-π} in audit.
// This identity connects the ternary state -1 to i via Euler's formula.
// (NOME_LEMNISCATIC is defined in Layer 0b; this is a cross-layer identity.)

}  // namespace ontic
}  // namespace ftd
