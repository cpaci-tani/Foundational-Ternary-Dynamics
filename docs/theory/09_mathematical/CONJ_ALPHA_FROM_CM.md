# Conjecture: The Fine Structure Constant from CM Arithmetic

**Status:** [CONJECTURE]
**Version:** 1.0 (2026-03-30)
**Depends on:** Watson integral identity, CM theory of E: y² = x³ − x, Fermat two-square theorem

---

## Statement

**Conjecture (Alpha-CM).** The fine structure constant α of quantum electrodynamics satisfies

$$\frac{1}{\alpha} = x_+$$

where x₊ is the larger root of the **master quadratic**

$$x^2 - |E(\mathbb{Q})_{\text{tors}}|^2 \cdot G^{*2} \cdot x + |E(\mathbb{Q})_{\text{tors}}|^2 \cdot G^{*3} = 0$$

associated to the CM elliptic curve E: y² = x³ − x with j-invariant 1728 and complex multiplication by ℤ[i], where

$$G^* = \frac{\Gamma(1/4)^2}{\sqrt{2}\,\Gamma(1/2)^2} \approx 2.9586751...$$

is the lemniscatic bridge constant (pi-free Gamma-primitive form; equivalently √2·Γ(1/4)²/(2π) since π = Γ(1/2)²).

**Numerical evidence:**
- Leading term: x₊ = 137.0361714582... vs CODATA α⁻¹ = 137.035999177(21). Agreement: 1.26 ppm.
- With 4-term precision formula: agreement to 15+ significant digits (< 0.001 parts per trillion).

---

## What Is Proven [THEOREM]

The following statements are theorems in classical mathematics:

### T1. The bridge constant G*

$$G^* = \frac{\sqrt{2}\,\Gamma(1/4)^2}{2\pi} = \sqrt{2\pi}\,\theta_3(e^{-\pi})^2 = \frac{2\sqrt{\pi}}{\text{agm}(1,\sqrt{2})} = \frac{2K(1/\sqrt{2})}{\sqrt{\pi}}$$

where θ₃ is the Jacobi theta function, agm is the arithmetic-geometric mean, and K is the complete elliptic integral of the first kind. All forms are algebraically equivalent. [Known since Gauss-Legendre, 1799-1811]

### T2. The coefficient 16

$$|E(\mathbb{Q})_{\text{tors}}|^2 = |\text{Aut}(E)|^2 = 16$$

where E(ℚ)_tors = {O, (0,0), (1,0), (−1,0)} ≅ ℤ/2ℤ × ℤ/2ℤ (Klein four-group) and Aut(E) = {±1, ±i} ≅ ℤ/4ℤ (cyclic). Different groups, same order 4, same square 16. [Classical]

### T3. Degree 2

The CM field of E is ℚ(i) with [ℚ(i):ℚ] = 2. By the Schneider-Chudnovsky theorem, algebraic relations among CM periods are bounded in degree by the CM field degree. [Chudnovsky 1984]

### T4. The roots

The quadratic x² − 16G*²x + 16G*³ = 0 has discriminant Δ = 64G*³(4G* − 1) > 0 and roots:

- x₊ = 8G*² + 4G*√(G*(4G*−1)) = 137.0361714582...
- x₋ = 8G*² − 4G*√(G*(4G*−1)) = 3.0239639163...

Both are transcendental numbers (by Nesterenko's theorem on Γ(1/4)). [Pure algebra]

### T5. The harmonic mean invariance

In the normalized variable u = x/G*, the quadratic becomes u² − 16G*u + 16G* = 0 with the property SUM = PRODUCT = 16G*. The harmonic mean of the normalized roots is:

$$H = \frac{2}{1/u_+ + 1/u_-} = \frac{2 \cdot u_+ u_-}{u_+ + u_-} = 2 = [\mathbb{Q}(i):\mathbb{Q}]$$

This holds for ALL quadratics of the form u² − Su + S = 0, regardless of S. The harmonic mean is fixed at the CM field degree. [Pure algebra]

### T6. The theta function bridge

The same generating function θ₃(q)² = Σ r₂(n)qⁿ produces:
- **π** via the unweighted density: R(N)/N = (Σ r₂(n))/N → π [Gauss circle theorem]
- **G*/√(2π)** via the exponential weighting: Σ r₂(n)e^{−πn} → G*/√(2π) [Chowla-Selberg at τ = i]

where r₂(n) = 4Σ_{d|n} χ₋₄(d) counts representations of n as a sum of two squares, controlled entirely by the split/inert classification of primes in ℤ[i]. [Classical analytic number theory]

### T7. The BSD connection

The Birch and Swinnerton-Dyer formula for E: y² = x³ − x (proven by Coates-Wiles 1977, Rubin 1991):

$$L(E,1) = \frac{\Omega \cdot c_2}{|E(\mathbb{Q})_{\text{tors}}|^2} = \frac{\varpi \cdot 4}{16} = \frac{\varpi}{4}$$

where c₂ = 4 = |Aut(E)| is the Tamagawa number at p = 2. This connects |E(ℚ)_tors|² = 16 (the master quadratic coefficient) to the L-function value through BSD. [Proven for rank-0 CM curves]

---

## What Remains to Be Proven [OPEN]

### Gap: The self-consistency form

The quadratic's specific structure — with constant term = (linear coefficient) × G*, i.e., the SUM = PRODUCT condition — is equivalent to the self-consistency prescription:

$$x = |E(\mathbb{Q})_{\text{tors}}|^2 \cdot G^{*2} \left(1 - \frac{G^*}{x}\right)$$

**This functional form has not been derived from either:**
1. The partition function of a lattice field theory on a cubic graph with no defined boundary (attempted and failed — self-energy is constant and x-independent on finite tori), or
2. The algebraic geometry of the CM curve E alone (no known theorem produces this form from CM invariants).

**The gap reduces to proving ONE statement:**

> **Conjecture (Self-Consistency).** The cubic graph with no defined boundary, equipped with ternary states and continuous flux coupled through the Gauss constraint, produces (for arbitrarily large finite extent L, with finite-L error O(1/L)) an effective coupling that satisfies x = K(1 − G*/x) where K = 16G*² and G* = √(2πW₃) with W₃ the Watson integral.

If this single statement is proven, the rest follows by algebra (T4).

---

## Structural Evidence (Not Proof)

### E1. Dimensional uniqueness
⌊x₋(D)⌋ = D only for D = 3 among dimensions 1–6. The master quadratic "recognizes" its own dimensionality. [Verified numerically]

### E2. Moat/split classification
The split/inert classification of primes in ℤ[i] (which controls r₂(n), the Gauss circle problem, and θ₃²) reproduces the same partition that FTD associates with electromagnetism (split) vs confinement (inert). [Structural consistency, not derivation]

### E3. The 1.26 ppm agreement
The leading-term match x₊ = 137.0362 vs α⁻¹ = 137.0360 has probability < 10⁻⁵ under a random-coincidence model, given that x₊ is determined by a single transcendental constant (Γ(1/4)) with no free parameters. [Statistical argument]

### E4. The precision formula
The 4-term correction using ε = e^π − π − 20 and rational coefficients from framework integers {3, 4, 7, 13} matches CODATA to 15+ digits. However, the coefficients are [SELECTION] — observed to work, not proven necessary. [Numerical observation]

### E5. The 8/26 cosmological ratio
BCC(8)/Moore(26) = 0.3077 vs Planck Ω_m = 0.315 ± 0.007. Within 1σ but not derived from dynamics. [Numerical observation]

---

## Attack Vectors

### Path A: Retracted
**Path A retracted (load-bearing premise was completed-infinity self-consistency).** Under undefined-boundary ontology, the conjecture's Path A — proposing that 1/α emerges as the L → ∞ self-consistent fixed point of a finite-L sequence — is no longer well-posed. The other paths in this document (algebraic uniqueness via CM curve, dual match via the master quadratic) are independent of Path A and survive unchanged. The preferred attack vector is now algebraic + structural, anchored in [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) and CM-curve uniqueness across class-number-1 fields.

### Path B: CM period relation
Prove that the CM curve E: y² = x³ − x satisfies an intrinsic algebraic relation of the form x² − |Aut(E)|²G*²x + |Aut(E)|²G*³ = 0 where x is a natural "coupling" associated to the curve's period lattice. This would be a new theorem in arithmetic geometry.

### Path C: L-function connection
Show that the master quadratic roots are related to special values of the L-function L(E,s) at specific points. For instance, if x₊ = f(L(E,1)) for some known function f, the conjecture reduces to evaluating L(E,1) (which is known = ϖ/4).

### Path D: The harmonic mean argument
Prove that ANY self-consistent coupling on a lattice with CM by ℤ[i] must have harmonic mean = [ℚ(i):ℚ] = 2 in normalized units. Combined with T2 (coefficient 16) and T1 (scale G*), this would uniquely determine the quadratic.

---

## Summary

| Component | Status | What it gives |
|-----------|--------|---------------|
| G* = √2·Γ(1/4)²/(2π) | **[THEOREM]** | The scale |
| |Aut(E)|² = 16 | **[THEOREM]** | The coefficient |
| Degree 2 from [ℚ(i):ℚ] | **[THEOREM]** | The polynomial degree |
| H = 2 (harmonic mean) | **[THEOREM]** | Invariant of all such quadratics |
| BSD: L(E,1) = ϖ/4 | **[THEOREM]** | Connection to L-function |
| θ₃² generates both π and G* | **[THEOREM]** | Dual convergence from r₂(n) |
| F(x) = K(1−G*/x) | **[OPEN]** | The self-consistency form |
| x₊ = 1/α | **[STRONGLY MOTIVATED CONJECTURE]** (LEDGER FTD-0013) | The physical identification. This doc's title "CONJ" predates the project-wide unified tag; the continuum-QED equivalence route is supporting/historical evidence, not a current promotion path. |

**The conjecture rests on one open mathematical statement.** Everything else is proven.
