#pragma once
/**
 * ontic/lemniscate.h — Layers -1 through 2b of the ontic chain.
 *
 * Contents:
 *   Layer -1: Self-Referential Seed (EULER_E)
 *   Layer 0:  Transcendental Seeds (EULER_GAMMA, GAMMA_QUARTER)
 *   Layer 0b: Modular Selection (NOME_LEMNISCATIC, THETA_LEMNISCATIC)
 *   Layer 1:  Elliptic Geometry (VARPI, GAUSS_CONSTANT_M)
 *   Layer 2:  Lemniscatic identities (G_STAR, PI, PF, I_1_BCC/W_3,
 *             SQRT_GSTAR, and legacy power aliases)
 *   Layer 2b: Generalized-quadratic discriminant (K_CRIT, X_BORN)
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
// Layer 2: Lemniscatic identities
// ============================================================================
// The lemniscatic gamma ratio G*:
//   G* = 2√(ϖ·M)               (π-free: scaled geometric mean)
//   G* = 2ϖ/√π                 (equivalent, using π = ϖ/M)
//
// G* bridges the elliptic (ϖ) and the arithmetic-geometric (M).
// Its exact dynamical role established by the temporal-interior programme is
// as the dimensionless period coefficient of the critical quartic clock:
//   T A = √π G* √(m/(2λ)).
//
// The historical assignments G*→flux, G*²→time/energy, and G*³→action are
// [SELECTIONS], not dimensional consequences of these pure numbers. The
// constants below retain their names for source compatibility only. Likewise,
// P/S=G* and the harmonic-mean formula are algebraic facts about the master-
// quadratic roots; their identification with physical α or N_c is separate.
//
// See: AUDIT_GSTAR_CLOCK_DEEP_DIVE_v1.md
//
// Pre-computed from G* = 2√(ϖ·M). Verified in ontic_audit().
inline constexpr double G_STAR = 2.958675119188639;

// Exact reparameterization identity:
//   G* = 2ϖ/√π  →  √π = 2ϖ/G*  →  π = 4ϖ²/G*²
//
// Because the definitions of ϖ, M, and G* already contain π through gamma
// reflection, this identity does not derive π from π-free axioms and does not
// establish an ontological priority relation.
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

// Positive square root of G*. No time-operator or read/write-subtick role has
// been derived; the name is retained as a convenient algebraic constant.
inline constexpr double SQRT_GSTAR = 1.720079974649039;

// Legacy power aliases. They are exact numerical powers, but their historical
// FLUX/TIME/ACTION labels are not physical dimensions or native-clock results.
inline constexpr double GSTAR_FLUX   = G_STAR;                      // legacy: G*¹
inline constexpr double GSTAR_TIME   = G_STAR * G_STAR;             // legacy: G*²
inline constexpr double GSTAR_ACTION = G_STAR * G_STAR * G_STAR;    // legacy: G*³

// ============================================================================
// Layer 2b: Generalized-quadratic discriminant
// ============================================================================
// Euler's identity:  e^{iπ} + 1 = 0
//
//   x² - k·G*²·x + k·G*³ = 0
//   Discriminant Δ = k·G*³·(k·G* - 4)
//
// Exact trichotomy: kG*>4 gives two real roots, kG*=4 a repeated real
// root, and kG*<4 a complex-conjugate pair. This elementary discriminant
// fact does not derive complex numbers, fermions, the Dirac equation,
// measurement, or the Born rule. Those physical identifications are retired
// legacy interpretations under the v2 contextual-actualization programme.
//
// A selected geometric encoding maps +1 to e^{i0}, -1 to e^{iπ}, and 0
// to the origin. Euler's identity then mirrors the ternary arithmetic
// (-1)+(+1)=0. The matter/antimatter interpretation is not inferred by the
// identity itself.
//
// On the principal logarithm branch, the lemniscatic nome obeys:
//   q = (-1)^i = (e^{iπ})^i = e^{i²π} = e^{-π}
// This is a branch-dependent complex-power identity, not a physical mapping.

// Critical coefficient where the generalized quadratic has zero discriminant.
inline constexpr double K_CRIT = 4.0 / G_STAR;

// Repeated root x=2G*. X_BORN is a legacy compatibility name; this identity
// neither supplies Born weights nor derives a probability pushforward.
inline constexpr double X_BORN = 2.0 * G_STAR;

// NOME_LEMNISCATIC is defined in Layer 0b.

}  // namespace ontic
}  // namespace ftd
