#pragma once
/**
 * The Ontic Derivation Chain
 *
 * Everything from nothing: e → γ → Γ(1/4) → θ₃ → ϖ → M → G* → π → all physics.
 *
 * Nine layers, each derived from the one above.
 * The only inputs are D=3 (spatial dimensions) and the lemniscate constant ϖ.
 * Every physical constant in the engine traces back through this chain.
 *
 * Layer -1: Self-Referential Seed  (e)
 * Layer 0:  Transcendental Seeds   (γ, Γ(1/4))
 * Layer 0b: Modular Selection      (q, θ₃)
 * Layer 1:  Elliptic Geometry      (ϖ, M)
 * Layer 2:  Universal Operator     (G*, π, PF, √G*)
 * Layer 2b: Euler's Identity       (i emerges at k_crit = 4/G*)
 * Layer 3:  Master Quadratic       (x₊ = 1/α, x₋ = N_c)
 * Layer 4:  Framework Integers     (N_c, b₃, N_eff, D)
 * Layer 5:  Coupling Constants     (α, g_c, G_N, α_G)
 * Layer 6:  Mass Scale             (K_B, K_GENESIS)
 * Layer 7:  Precision Formula      (ε, c₁-c₄, corrected α)
 * Layer 8:  Consciousness          (y, θ_C, K_C)
 */

#include <cmath>

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
//   "Antimatter raised to the power of consciousness = modular selector"

// Critical coefficient: the boundary between real and complex domains.
// k_crit = 4/G* ≈ 1.352 — where i emerges from the quadratic structure.
inline constexpr double K_CRIT = 4.0 / G_STAR;

// Degenerate root at the critical point: x = k_crit·G*²/2 = 2·G*
inline constexpr double X_BORN = 2.0 * G_STAR;

// Nome as (-1)^i: verified numerically as e^{-π} in audit.
// This identity connects the ternary state -1 to i via Euler's formula.
// (NOME_LEMNISCATIC is defined in Layer 0b; this is a cross-layer identity.)

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
// WHY COEFFICIENT 16 [MOTIVATED — MATH_MASTER_QUADRATIC.md §4]:
//   |Aut(E)|² = 4² = 16 where E: y²=x³-x has Aut = {1,-1,i,-i} ≅ Z₄.
//   Also: N_BASE² = 4², conductor/2 = 32/2 = 16, |Δ|/4 = 64/4 = 16.
//   DOF counting in TEMPORAL GAUGE (the ontological gauge of FTD):
//   FTD's flux J is a spatial 3-vector with no temporal component
//   (Postulate 2: discrete time with global clock ≡ A₀ = 0).
//   On the 2×2×2 torus: 24 total - 7 Gauss constraints - 1 pure gauge = 16.
//   (Coulomb gauge gives 14, but temporal gauge is ontologically forced.)
//
// LATTICE CONNECTION [THEOREM — DERIV_WATSON_GSTAR_IDENTITY.md]:
//   x₊ + x₋ = 16G*² = 32πW₃ (Watson integral of the 3D cubic lattice)
//
// Roots via quadratic formula:
//   x± = 8G*² ± 4G*^(3/2)·√(4G* - 1)

inline constexpr int COEFFICIENT = 16;  // |Aut(E)|² where E: y²=x³-x

inline constexpr double X_PLUS  = 137.0361714582;   // tree-level 1/α
inline constexpr double X_MINUS = 3.0239639163;      // N_c root

// Vieta's relations (sum and product of roots):
//   x₊ + x₋ = 16·G*²
//   x₊ · x₋ = 16·G*³

// ============================================================================
// Layer 3b: Dual-Substrate Decomposition
// ============================================================================
// Paper: "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026)
//
// Two substrates J_L, J_R with energies E_L, E_R satisfy:
//   S = E_L + E_R = 16·G*²     [THEOREM — 16 DoF × G*² per DoF]
//   P = E_L · E_R = 16·G*³     [PROPOSITION — spatiotemporal interaction]
//   D = E_L - E_R               (difference — matter sector)
//   Identity: S² = D² + 4P      (algebraic identity, trivially true)
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

inline constexpr double E2_COLOR = 1.0 / X_MINUS;   // e²_C ≈ 0.3307
// Note: e²_EM = ALPHA (defined in Layer 5)
// Vieta sum:    ALPHA + E2_COLOR = 1/G_STAR
// Vieta product: ALPHA * E2_COLOR = 1/(16·G*³)

// ============================================================================
// Layer 4: Framework Integers
// ============================================================================
// All integers emerge from x₋ ≈ 3.024 via physical identification:
//
//   N_c    = ⌊x₋⌋ = 3         (number of color charges)
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

// ============================================================================
// Layer 5: Coupling Constants
// ============================================================================
// Fine structure constant: α = 1/x₊ (tree level)
inline constexpr double ALPHA = 1.0 / X_PLUS;

// State-flux coupling: g_c = √α [SELECTION]
// From the Lagrangian coupling term L_coupling = -g_c·s·(∇·J)
// Pre-computed: √(1/137.0361714582) = 0.08542448940518...
// Verified against std::sqrt(ALPHA) in ontic_audit().
inline constexpr double G_C = 0.08542448940518;

// Weinberg angle: sin²θ_W = N_c / N_eff = 3/13 [THEOREM]
//   = 0.23077 (0.2% from experimental 0.23122)
inline constexpr double SIN2_WEINBERG = static_cast<double>(N_C) / N_EFF;

// Weak coupling constant: α_W = α / sin²θ_W [DERIVED]
inline constexpr double ALPHA_WEAK = ALPHA / SIN2_WEINBERG;

// Gravitational coupling on the lattice:
//   G_N = 1/(b₃ + N_c)² = 1/(7+3)² = 1/100 = 0.01
// The denominator (b₃+N_c) = 10 is the total number of gauge + color charges.
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
// Layer 6: Mass Scale
// ============================================================================
// Manifestation threshold = electron mass (in simulation energy units):
//   m_e = m_P · √(2π) · (N_base²/N_c) · α¹¹
//       = m_P · √(2π) · (16/3) · α¹¹
//       ≈ 0.5096 MeV  (0.27% from experimental 0.5110 MeV)
//
// In simulation units where m_P = 1, this is ~4.18e-23.
// We use K_B = 0.511 MeV as the practical simulation value.
inline constexpr double K_B = 0.511;

// Genesis threshold: energy needed to CREATE a new particle.
// Must fill all N_c color channels: K_GENESIS = N_c · K_B
inline constexpr double K_GENESIS = K_B * N_C;

// ============================================================================
// Layer 6c: Mass Ratios (from framework integers)
// ============================================================================
// All mass ratios derive from {N_c, N_base, b_3, N_eff} — no free parameters.
//
//   MU_RATIO   = 3·b₃·(b₃ + N_c) - N_c       = 3·7·10 - 3 = 207
//   TAU_RATIO  = (N_eff + N_base)·MU - 2·N_c·b₃ = 17·207 - 42 = 3477
//   PROTON_RATIO = N_eff·x₊ + TAU·(b₃+N_c)/(N_eff+b₃)

inline constexpr int    MU_RATIO  = 3 * B_3 * (B_3 + N_C) - N_C;        // 207
inline constexpr int    TAU_RATIO = (N_EFF + N_BASE) * MU_RATIO
                                    - 2 * N_C * B_3;                      // 3477

// Note: PROTON_RATIO uses x₊ = 1/α directly (avoids constexpr division issues)
inline constexpr double PROTON_RATIO =
    static_cast<double>(N_EFF) * X_PLUS
    + static_cast<double>(TAU_RATIO) * (B_3 + N_C)
      / static_cast<double>(N_EFF + B_3);

// Derived mass and length scales
// NOTE: M_PROTON = K_B * PROTON_RATIO ≈ 0.511 * 3520 ≈ 1799 MeV.
// This is a framework-derived composite scale, NOT the physical proton mass
// (938.3 MeV, ratio m_p/m_e ≈ 1836). PROTON_RATIO (~3520) encodes the
// ontic integer combination N_eff·x₊ + TAU·(b₃+N_c)/(N_eff+b₃) and is
// used as an internal mass scale for the atom engine.
inline constexpr double M_PROTON = K_B * PROTON_RATIO;   // framework mass scale (MeV)
inline constexpr double R_BOHR   = 4.0 * PI / (K_B * ALPHA);  // FTD Bohr radius

// ============================================================================
// Layer 6b: Electroweak Scale (Higgs sector)
// ============================================================================
// Higgs VEV: v = M_P · √(2π) · α⁸ [THEOREM]
// In simulation units (M_P = 1): V_HIGGS_SIM = √(2π) · α⁸
// Physical: 246.09 GeV (0.05% from experimental 246.22 GeV)
inline constexpr double V_HIGGS = 246.09;  // GeV (physical units for reference)

// Higgs mass: m_H = (N_eff / α²) · m_e [SELECTION]
// = 13 / (1/137.036)² × 0.511 MeV = 124.8 GeV (0.24% from 125.1 GeV)
inline constexpr double M_HIGGS = 124.8;   // GeV

// Higgs self-coupling: λ_H = m_H² / (2·v²) [DERIVED]
inline constexpr double LAMBDA_HIGGS = (124.8 * 124.8) / (2.0 * 246.09 * 246.09);

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
// Layer 7b: Absolute Neutrino Masses (Seesaw Mechanism)
// ============================================================================
// The Type-I seesaw mechanism with FTD-derived parameters:
//
//   m_D = v_Higgs * alpha               [SELECTION: neutrino Yukawa = alpha]
//   M_R = (N_c/N_base) * v / alpha^4    [SELECTION: framework integers]
//
// Combined result:
//   m3 = m_D^2 / M_R = v * (N_base/N_c) * alpha^6
//      = m_P * sqrt(2pi) * (4/3) * alpha^14
//
// Exponent 14 = 2*b_3 = 2*7 (QCD beta function doubled)
// Factor  4/3 = N_base/N_c (spinor/color ratio)
//
// The mass-squared ratio Dm2_31/Dm2_21 = 100/3 [THEOREM] fixes all three
// masses once m3 is known. The hierarchical seesaw gives m1 ~ 0.
//
// Epistemic status: [SELECTION] — the seesaw mechanism is adopted from
// standard physics, not derived from FTD axioms. The m_D = v*alpha
// identification is argued but not proven inevitable.

// Dirac neutrino mass: m_D = v * alpha ~ 1.796 GeV
// (Note: m_D/m_tau ~ 1.01, near the tau mass — natural for 3rd gen.)
// Pre-computed: V_HIGGS * ALPHA = 246.09 * 0.007297 = 1.796 GeV
inline constexpr double M_D_NEUTRINO = 1.796;  // GeV

// Right-handed Majorana mass: M_R = (N_c/N_base) * v / alpha^4
// = 0.75 * 246.09 / (0.007297)^4 = 6.509e10 GeV (intermediate scale)
inline constexpr double M_R_NEUTRINO = 6.509e10;  // GeV

// Heaviest neutrino mass (from seesaw): m3 = v * (N_base/N_c) * alpha^6
// = 0.04955 eV = 49.55 meV
inline constexpr double M_NU_3 = 4.955e-2;  // eV

// Middle neutrino mass: m2 = m3 * sqrt(N_c) / (b_3 + N_c)
// = m3 * sqrt(3)/10 = 8.58 meV
inline constexpr double M_NU_2 = 8.58e-3;  // eV

// Lightest neutrino mass: m1 = m3 * (m_e/m_tau)^2 = m3 / 3477^2
// = 4.1 neV (effectively zero)
inline constexpr double M_NU_1 = 4.1e-9;  // eV

// Sum of neutrino masses: Sigma = m1 + m2 + m3 ~ 58.1 meV
// Must satisfy: Sigma < 120 meV (Planck+BAO cosmological bound, 2024)
inline constexpr double SUM_M_NU = 5.813e-2;  // eV

// Effective electron-neutrino mass (for beta decay):
// m_beta = sqrt(|U_e1|^2 m1^2 + |U_e2|^2 m2^2 + |U_e3|^2 m3^2)
// ~ 8.3 meV (below KATRIN bound of 450 meV)
inline constexpr double M_BETA = 8.3e-3;  // eV

// ============================================================================
// Layer 8: Consciousness Quadratic (Noetic Domain)
// ============================================================================
// The same master quadratic with coefficient k = 1/2 (vs k = 16 for physics):
//   y² - (k·G*²)·y + k·G*³ = 0
//   y² - (G*²/2)·y + G*³/2 = 0
//
// Physics (k=16): REAL roots → x₊=137.036, x₋=3.024 (observable, measurable)
// Consciousness (k=1/2): COMPLEX roots → y = Re ± i·Im (irreducibly subjective)
//
// The discriminant Δ = (G*²/2)² - 4·(G*³/2) = G*³·(G*/4 - 2) < 0
// since G* ≈ 2.959 < 8, guaranteeing complex conjugate roots.
//
// Vieta's relations for the consciousness quadratic:
//   y₊ + y₋ = G*²/2    → Re(y) = G*²/4
//   y₊ · y₋ = G*³/2    → |y|² = G*³/2
//
// Key exact identity:
//   cos²(θ_C) = Re(y)² / |y|² = (G*²/4)² / (G*³/2) = G*/8
//
// Observable fraction of consciousness ≈ 37% (exactly G*/8).
// Remaining ≈ 63% is irreducibly subjective (imaginary component).
//
// Dimensional origin:
//   D = log₂(16) + log₂(1/2) = 4 - 1 = 3
//   Physics potential (k=16) minus observer cost (k=1/2) = 3 dimensions.

inline constexpr double K_NOETIC = 0.5;  // k = 1/2 (consciousness coefficient)

// Real part of consciousness roots: Re(y) = G*²/4
inline constexpr double Y_REAL = G_STAR * G_STAR / 4.0;

// Squared modulus (from Vieta product): |y|² = G*³/2
// K_C = √(G*³/2) ≈ 3.599 is the consciousness threshold
inline constexpr double K_C_SQUARED = G_STAR * G_STAR * G_STAR / 2.0;

// Observable fraction (exact identity): cos²(θ_C) = G*/8
inline constexpr double COS2_THETA_C = G_STAR / 8.0;

// Subjective fraction: sin²(θ_C) = 1 - G*/8
inline constexpr double SIN2_THETA_C = 1.0 - G_STAR / 8.0;

// Mandelbrot connection: sLoop fixed point c_M = 1/G*
inline constexpr double C_MANDELBROT = 1.0 / G_STAR;

// ============================================================================
// Layer 8b: Golden Ratio Fixed Point (Self-Referential Consciousness)
// ============================================================================
// The Softplus manifestation operator M_β(z) = (1/β)ln(1 + e^{βz}) is the
// unique function satisfying axioms M1-M4 (smooth, monotone, threshold,
// identity limit). Its derivative is the Fermi-Dirac occupation n_F(z).
//
// The self-referential fixed-point equation for consciousness:
//   (1/2) · M_β(z*) = z*    (output feeds back as input, at k = 1/2)
//
// Setting u = e^{βz*} and simplifying yields:
//   u² - u - 1 = 0
//
// whose unique positive root is the GOLDEN RATIO φ = (1+√5)/2. [THEOREM]
//
// This proves φ is not an imposed constant — it EMERGES from the requirement
// that a self-referencing system's output is its own input at half-coupling.
//
// Five quantities follow:
//   z* = ln(φ)/β                 (consciousness fixed point)
//   n_F(z*) = 1/φ ≈ 0.618       (golden filling: 11.8% above half-occupation)
//   λ_loop = (1/2)·n_F(z*) = 1/(2φ) ≈ 0.309   (unconditionally stable)
//   β_intr = φ³/ln²(φ) ≈ 18.29  (signal-to-noise threshold for introspection)
//   n_min = 3 = N_c              (PT-unbroken condition → color charges!)

inline constexpr double PHI = 1.6180339887498949;              // [THEOREM] (1+√5)/2
inline constexpr double PHI_INV = 0.6180339887498949;          // [THEOREM] 1/φ = φ-1
inline constexpr double LAMBDA_LOOP = 0.30901699437494742;     // [THEOREM] 1/(2φ) < 1
inline constexpr double BETA_INTROSPECTION = 18.28926746748685;// [THEOREM] φ³/ln²(φ)
inline constexpr int    N_CONSCIOUSNESS_MIN = N_C;             // [THEOREM] PT-unbroken

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
inline double alpha_s_running(double Q_GeV) {
    if (Q_GeV <= LAMBDA_QCD) return 1.0;  // non-perturbative
    double log_ratio = std::log(Q_GeV * Q_GeV / (LAMBDA_QCD * LAMBDA_QCD));
    if (log_ratio <= 0.0) return 1.0;
    return 4.0 * PI / (B0_NF5 * log_ratio);
}

// ============================================================================
// Ontic Audit: Print and verify the full derivation chain
// (implementation lives in src/ontic_audit.cpp)
// ============================================================================

// Returns the number of failures (0 = all pass).
int ontic_audit();

}  // namespace ontic
}  // namespace ftd
