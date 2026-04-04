# G* as Universal Bridging Constant: The ϖ/√(PF) Decomposition

## How PF = π/4 Enters and Exits Every Sector of Physics

**Document Version:** 1.0
**Framework Version:** FTD v5.26
**Date:** February 19, 2026
**Standard:** Domain-by-domain algebraic reduction with three-layer epistemic separation

**Depends on:**
- [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) — Constant chain γ → ϖ → M → π → G*
- [MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md) — Master quadratic and G* definition (Layer 1: pure mathematics)
- [BRIDGE_QUADRATIC_PHYSICS.md](../01_reference/BRIDGE_QUADRATIC_PHYSICS.md) — Selection principles SP1-SP6 (Layer 2)
- [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) — Schwarzschild metric from lattice principles

---

## Abstract

The FTD master constant G* = Γ(1/4)/Γ(3/4) ≈ 2.9587 (equivalently √2 · Γ(1/4)²/(2π)) decomposes exactly as:

$$G^* = \frac{\varpi}{\sqrt{\text{PF}}}$$

where ϖ ≈ 2.622 is the lemniscate half-period (continuous geometry) and PF ≡ π/4 ≈ 0.785 is the circle-in-square packing fraction (discrete lattice geometry). When this decomposition is applied domain-by-domain — black hole thermodynamics, loop quantum gravity, vacuum energy, QFT loops — a universal pattern emerges: **PF cancels in every dimensionless physical observable** and survives only in absolute scale-setting quantities.

The deepest consequence is in LQG: the minimal area quantum reduces to A_min = N_base · ln(2) · l_P², a **topological** quantity independent of packing geometry, determined entirely by the division algebra tower and information theory.

Three epistemic layers are cleanly separated: algebraic identities [THEOREM], lattice interpretations [SELECTION], and deeper structural conjectures [CONJECTURE].

---

## Preface: Epistemic Framework

| Tag | Meaning | Standard |
|-----|---------|----------|
| **[AXIOM]** | Primitive FTD postulate | Cannot be derived; foundational |
| **[DEFINITION]** | Formal naming | No truth claim; establishes notation |
| **[THEOREM]** | Rigorously proven | Complete derivation; check proof |
| **[SELECTION]** | Argued choice | Not unique; justified by criteria |
| **[CONJECTURE]** | Unproven claim | Evidence but no proof |
| **[VERIFIED]** | Confirmed numerically | All cases checked |

### Three-Layer Structure

| Layer | Content | Tags |
|-------|---------|------|
| **A: Algebra** | G* = ϖ/√(PF), PF cancellation identities, Stefan-Boltzmann decomposition | [THEOREM], [VERIFIED] |
| **B: Lattice Interpretation** | PF as discretization cost, Immirzi decomposition, division algebra denominator | [SELECTION] |
| **C: Structural Claims** | Three irreducible constants, PF cancellation rule, ontic significance | [CONJECTURE] |

### Honesty Note

The identity G* = ϖ/√(PF) is **algebraically exact** — it follows from the definitions of ϖ and G*. The *interpretation* of PF = π/4 as a packing fraction, and the claim that PF cancellation reflects deep physics, are [SELECTION] arguments. The Immirzi parameter decomposition (§4) assumes a specific identification that is not independently proven — it is a proposed correspondence between FTD and LQG.

### Disambiguation: PF in This Document

Throughout this document, **PF ≡ π/4 ≈ 0.7854** denotes the circle-in-square packing fraction. This is distinct from uses of "PF" as informal shorthand for "Planck Frequency" that appear in other FTD documents (e.g., DERIV_LATTICE_SCHWARZSCHILD.md Part III). The constant PF = π/4 is the canonical definition.

---

# PART I: THE DECOMPOSITION

---

## §1. Definitions and Algebraic Identity

### 1.1 The Packing Fraction [DEFINITION]

$$\text{PF} \equiv \frac{\pi}{4} \approx 0.78540$$

This is the **circle-in-square packing fraction**: the ratio of a circle's area to its circumscribing square. On each face of the cubic lattice, PF measures the fraction of area covered by the maximal inscribed circle.

### 1.2 The Lemniscate Half-Period [DEFINITION]

$$\varpi = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}} \approx 2.62206$$

This is the half-period of the Bernoulli lemniscate r² = cos(2θ) — the "π of the figure-eight." It encodes continuous analytical geometry: elliptic integrals, period lattices, complex multiplication.

### 1.3 The Bridge Identity [THEOREM]

**Theorem 1.1** (G* Decomposition): *The FTD master constant satisfies:*

$$\boxed{G^* = \frac{\varpi}{\sqrt{\text{PF}}} = \frac{\varpi}{\sqrt{\pi/4}} = \frac{2\varpi}{\sqrt{\pi}}}$$

**Proof:** Starting from the definitions:

$$\frac{\varpi}{\sqrt{\pi/4}} = \varpi \cdot \frac{2}{\sqrt{\pi}} = \frac{\Gamma(1/4)^2}{2\sqrt{2\pi}} \cdot \frac{2}{\sqrt{\pi}} = \frac{\Gamma(1/4)^2}{\sqrt{2\pi} \cdot \sqrt{\pi}} = \frac{\Gamma(1/4)^2}{\pi\sqrt{2}}$$

Meanwhile:

$$G^* = \frac{\sqrt{2} \cdot \Gamma(1/4)^2}{2\pi} = \frac{\Gamma(1/4)^2}{2\pi/\sqrt{2}} = \frac{\Gamma(1/4)^2}{\pi\sqrt{2}} \quad \blacksquare$$

**Numerical verification:**
- ϖ/√(π/4) = 2.62206 / √(0.78540) = 2.62206 / 0.88623 = 2.95868
- G* = √2 · Γ(1/4)² / (2π) = 2.95868
- Agreement to machine precision. [VERIFIED]

### 1.4 Relationship to Existing FTD Formula

The existing formula in `scripts/constants.py` states G* = 2ϖ/√π. This is the same identity:

$$\frac{\varpi}{\sqrt{\pi/4}} = \varpi \cdot \frac{2}{\sqrt{\pi}} = \frac{2\varpi}{\sqrt{\pi}}$$

The PF form makes the lattice interpretation explicit.

---

## §2. Physical Interpretation [SELECTION]

### 2.1 Two Computational Domains

| Domain | Governed by | Character |
|--------|-------------|-----------|
| **Continuous** | ϖ (lemniscate half-period) | Analytic functions, elliptic curves, complex multiplication, period integrals |
| **Discrete** | PF = π/4 (packing fraction) | Cubic lattice faces, circle-in-square geometry, digital quantization |

### 2.2 G* as Exchange Rate

G* converts between these domains: it is the **fixed ratio** at which continuous analytical potential (measured in ϖ units) translates into discrete lattice state (measured in PF units). The speed limit C = 1 is the throughput constraint — one exchange per tick per node.

### 2.3 Why √(PF), Not PF

The square root appears because the exchange involves an **area-to-length** reduction. PF is an area ratio (circle area / square area). The physical exchange rate operates on lengths (lattice spacing), so the relevant conversion is √(PF) — the fraction of the lattice face that a circle's *diameter* covers in one dimension, normalized to the face width.

---

# PART II: DOMAIN-BY-DOMAIN PF REDUCTION

---

## §3. Black Hole Thermodynamics

### 3.1 Entropy in PF Notation [THEOREM]

The Bekenstein-Hawking entropy (in Planck units, G = c = ℏ = k_B = 1):

$$S_{BH} = \frac{A}{4\ell_P^2} = \frac{4\pi r_s^2}{4} = \pi r_s^2 = \pi(2M)^2 = 4\pi M^2$$

Express 4π through FTD integers:

$$4\pi = N_{\text{base}}^2 \cdot \text{PF} = 16 \cdot \frac{\pi}{4} = 4\pi$$

Therefore:

$$S_{BH} = N_{\text{base}}^2 \cdot \text{PF} \cdot M^2$$

### 3.2 Hawking Temperature in PF Notation [THEOREM]

$$T_H = \frac{1}{8\pi M}$$

Express 8π:

$$8\pi = 2 \cdot 4\pi = 2 \cdot N_{\text{base}}^2 \cdot \text{PF}$$

Therefore:

$$T_H = \frac{1}{2 \cdot N_{\text{base}}^2 \cdot \text{PF} \cdot M}$$

### 3.3 The PF Cancellation [THEOREM]

**Theorem 3.1** (Entropy-Temperature Product): *The product S_BH × T_H is independent of PF:*

$$S_{BH} \times T_H = N_{\text{base}}^2 \cdot \text{PF} \cdot M^2 \times \frac{1}{2 \cdot N_{\text{base}}^2 \cdot \text{PF} \cdot M} = \frac{M}{2}$$

PF cancels. $N_{\text{base}}^2$ cancels. The result $M/2$ depends on **nothing** except the mass. $\blacksquare$

### 3.4 Physical Interpretation [SELECTION]

$S_{BH} \times T_H = M/2$ is the thermodynamic expression of **total computational saturation** at the horizon. PF (the geometric packing cost) appears in both S and T individually — entropy scales with the number of lattice faces (involving PF), and temperature scales inversely with the processing cycle time (also involving PF). But the *physics* — total energy throughput at saturation — depends only on mass.

The factor $1/2$ reflects the equal partition between left-moving and right-moving modes at the horizon.

---

## §4. LQG Minimal Area

### 4.1 Standard LQG Area Spectrum [DEFINITION]

In loop quantum gravity, the area spectrum for a surface punctured by spin-network edges is:

$$A = 8\pi\gamma_I \ell_P^2 \sum_p \sqrt{j_p(j_p + 1)}$$

where $\gamma_I$ is the Barbero-Immirzi parameter and $j_p$ are spin labels. The minimal area corresponds to a single puncture with $j = 1/2$:

$$A_{\min} = 8\pi\gamma_I \cdot \frac{\sqrt{3}}{2} \cdot \ell_P^2 = 4\pi\sqrt{3} \cdot \gamma_I \cdot \ell_P^2$$

### 4.2 The Immirzi Parameter Decomposition [SELECTION]

We propose the following decomposition of the Barbero-Immirzi parameter:

$$\boxed{\gamma_I = \frac{\ln 2}{N_{\text{base}} \cdot \text{PF} \cdot \sqrt{N_c}} = \frac{\ln 2}{\pi\sqrt{3}}}$$

**Verification:** $N_{\text{base}} \cdot \text{PF} \cdot \sqrt{N_c} = 4 \cdot \frac{\pi}{4} \cdot \sqrt{3} = \pi\sqrt{3}$

$$\gamma_I = \frac{\ln 2}{\pi\sqrt{3}} \approx \frac{0.6931}{5.4414} \approx 0.12738$$

This matches the Domagala-Lewandowski / Meissner (2004) value for the Immirzi parameter derived from Bekenstein-Hawking entropy matching.

**Epistemic status [SELECTION]:** This decomposition is *proposed*, not derived from FTD axioms. The identification of $N_{\text{base}} \cdot \text{PF} = \pi$ and $\sqrt{N_c} = \sqrt{3}$ in the denominator is a pattern observation. The **result** (γ_I = ln(2)/(π√3)) is standard LQG; the **decomposition** into FTD integers is the new claim.

### 4.3 PF Cancellation in the Minimal Area [THEOREM]

**Theorem 4.1** (Topological Minimal Area): *The minimal area quantum is independent of PF:*

$$A_{\min} = 4\pi\sqrt{3} \cdot \gamma_I \cdot \ell_P^2 = 4\pi\sqrt{3} \cdot \frac{\ln 2}{\pi\sqrt{3}} \cdot \ell_P^2 = 4\ln 2 \cdot \ell_P^2$$

$$\boxed{A_{\min} = N_{\text{base}} \cdot \ln 2 \cdot \ell_P^2}$$

**Proof:** The π√3 from the area formula coefficient exactly cancels the π√3 in the Immirzi parameter denominator:

$$\underbrace{4\pi\sqrt{3}}_{\text{from area spectrum}} \times \underbrace{\frac{\ln 2}{\pi\sqrt{3}}}_{\gamma_I} = 4\ln 2 = N_{\text{base}} \cdot \ln 2 \quad \blacksquare$$

**Numerical value:** $A_{\min} = 4 \times 0.6931 \times \ell_P^2 \approx 2.773 \, \ell_P^2$

### 4.4 Why This Is "The Showstopper" [SELECTION]

The final expression $A_{\min} = N_{\text{base}} \cdot \ln 2 \cdot \ell_P^2$ contains:

| Factor | Origin | Character |
|--------|--------|-----------|
| $N_{\text{base}} = 4$ | Division algebra base dimension (ℝ, ℂ, ℍ, 𝕆) | Topological — from algebraic structure |
| $\ln 2$ | Information content of one binary choice | Information-theoretic — universal |
| $\ell_P^2$ | Planck area (lattice face area) | Dimensional — scale-setting |

The packing geometry (PF) has cancelled completely. The area quantum depends on:
- **What kinds of algebras exist** (division algebras → N_base = 4)
- **How much information one bit carries** (ln 2)
- **How large the lattice faces are** (l_P²)

It does **not** depend on how circles pack into squares, or on any continuous geometric factor. The holographic data cap is topological, not geometric.

---

## §5. Vacuum Energy Denominators

### 5.1 Stefan-Boltzmann Decomposition [THEOREM + SELECTION]

The Stefan-Boltzmann constant in natural units:

$$\sigma = \frac{\pi^2}{60}$$

**Theorem 5.1** (Division Algebra Denominator): *The denominator 60 decomposes as:*

$$60 = N_{\text{base}} \cdot D_\Sigma = 4 \cdot 15$$

*where $D_\Sigma = 1 + 2 + 4 + 8 = 15$ is the sum of dimensions of the four normed division algebras.* $\blacksquare$

### 5.2 The Division Algebra Tower [SELECTION]

| Algebra | Symbol | Dimension | Properties |
|---------|--------|-----------|------------|
| Real numbers | ℝ | 1 | Ordered, commutative, associative |
| Complex numbers | ℂ | 2 | Commutative, associative (not ordered) |
| Quaternions | ℍ | 4 | Associative (not commutative) |
| Octonions | 𝕆 | 8 | Alternative (not associative) |
| **Total** | | **D_Σ = 15** | |

By the Hurwitz theorem, these are the **only** normed division algebras over ℝ. The sum 15 = 1 + 2 + 4 + 8 is therefore a universal algebraic constant.

### 5.3 Stefan-Boltzmann in FTD Notation [SELECTION]

Expressing π = N_base · PF = 4 · (π/4):

$$\sigma = \frac{(N_{\text{base}} \cdot \text{PF})^2}{N_{\text{base}} \cdot D_\Sigma} = \frac{N_{\text{base}} \cdot \text{PF}^2}{D_\Sigma}$$

The interpretation: vacuum thermal radiation involves $\text{PF}^2$ (area packing on two lattice faces) divided by $D_\Sigma$ (the 15 algebraic degrees of freedom that distribute the energy). The factor N_base converts between lattice units and physical units.

### 5.4 Casimir Energy Connection [SELECTION]

The Casimir energy density between parallel plates (separation $a$) in natural units:

$$\rho_{\text{Casimir}} = -\frac{\pi^2}{720 \, a^4} = -\frac{(N_{\text{base}} \cdot \text{PF})^2}{12 \cdot N_{\text{base}} \cdot D_\Sigma \cdot a^4}$$

The factor 720 = 12 × 60 = 12 × N_base × D_Σ, where 12 = 3 × 4 = N_c × N_base is the kissing number K(3) of the FCC lattice (number of nearest neighbors).

---

## §6. QFT Loop Expansion

### 6.1 One-Loop Expansion Parameter [SELECTION]

The standard QED one-loop correction factor:

$$\frac{\alpha}{2\pi}$$

Express 2π:

$$2\pi = 2^D \cdot \text{PF} \quad \text{where } D = 3 \text{ (spatial dimensions)}$$

**Verification:** $2^3 \cdot \frac{\pi}{4} = 8 \cdot 0.7854 = 2\pi$ ✓

### 6.2 Interpretation [SELECTION]

The one-loop expansion parameter is:

$$\frac{\alpha}{2^D \cdot \text{PF}}$$

Each virtual loop "sees" $2^D$ lattice octants (the corners of the 3D cube), and each octant is PF-efficient (fraction π/4 of the area is accessible to circular modes). The product $2^D \cdot \text{PF}$ is the total accessible phase space for one virtual exchange.

Higher-loop corrections involve additional factors of this parameter, providing geometric suppression from the lattice structure.

---

# PART III: THE THREE IRREDUCIBLE CONSTANTS

---

## §7. Irreducible Constant Set [SELECTION]

### 7.1 The Claim

All FTD physics can be expressed through **four transcendental constants** and **four integers**:

**Constants:**

| Constant | Value | Domain | Role |
|----------|-------|--------|------|
| ϖ (varpi) | 2.62206 | Continuous geometry | Lemniscate half-period; analytic structure |
| PF = π/4 | 0.78540 | Discrete geometry | Circle-in-square packing; lattice cost |
| ln(2) | 0.69315 | Information theory | Entropy of one bit; binary choice content |
| √2 | 1.41421 | Metric geometry | FCC nearest-neighbor distance; critical coupling |

**Integers:**

| Integer | Value | Origin |
|---------|-------|--------|
| N_c | 3 | Color charges (from master quadratic x_-) |
| N_base | 4 | Base dimension (lattice geometry, division algebra) |
| b_3 | 7 | QCD beta coefficient (11 - 4N_c/3 at N_f = 0) |
| N_eff | 13 | Fibonacci F_7 (self-referential closure) |

### 7.2 Derived Quantities

From these eight primitives, all FTD physical constants are assembled:

| Derived quantity | Expression | Value |
|------------------|------------|-------|
| π | N_base · PF = 4 · π/4 | 3.14159 |
| G* | ϖ/√(PF) = 2ϖ/√π | 2.95868 |
| α | 1/x_+ from master quadratic | 1/137.036 |
| A_min/l_P² | N_base · ln(2) | 2.77259 |
| γ_I | ln(2)/(N_base · PF · √N_c) | 0.12738 |
| D_Σ | 1 + 2 + 4 + 8 = 2⁰ + 2¹ + 2² + 2³ | 15 |
| σ (Stefan-Boltzmann) | (N_base · PF)² / (N_base · D_Σ) | π²/60 |

### 7.3 Independence [SELECTION]

The four constants are functionally independent:
- **ϖ** is not a rational function of π (both are transcendental, but algebraically independent — a consequence of Nesterenko's theorem on Γ(1/4))
- **PF = π/4** is a rational multiple of π (but π is not independent of ϖ; rather, π = M · ϖ where M = AGM(1,√2))
- **ln(2)** is transcendental and algebraically independent of ϖ and π (no known algebraic relation)
- **√2** is algebraic (irrational but a root of x² - 2 = 0)

The minimal irreducible set is arguably {ϖ, ln(2), √2}, since π = M · ϖ and PF = π/4 = Mϖ/4. However, PF is retained as a separate entry because it encodes a distinct physical concept (discrete packing) even though it is mathematically derivable from ϖ.

---

## §8. The Cancellation Rule [SELECTION]

### 8.1 Statement

**PF Cancellation Rule:** *Within any single physics sector, PF cancels in every dimensionless ratio or physically observable product. PF survives only in quantities that set absolute scales.*

### 8.2 Evidence

| Domain | PF-dependent quantities | PF-free observable |
|--------|------------------------|-------------------|
| **BH thermo** | S_BH (has PF), T_H (has PF) | S × T = M/2 |
| **LQG area** | γ_I (has PF), area coefficient (has PF) | A_min = N_base · ln(2) · l_P² |
| **Vacuum energy** | σ numerator (has PF²), denominator (has N_base · D_Σ) | Relative energy ratios |
| **Relativity** | Metric components (f = 1 - r_s/r) | Time dilation ratios dτ₁/dτ₂ |

### 8.3 Interpretation [SELECTION]

PF = π/4 encodes the **geometric cost of discretization** — the fraction of each lattice face that is accessible to smooth (circular) modes. This cost is an artifact of embedding continuous physics on a discrete substrate. The fact that it cancels in observables suggests:

> **Observable physics is independent of the lattice's packing geometry.** The lattice is scaffolding; the physics is the circle, not the square.

This is analogous to how lattice QCD results become lattice-spacing-independent in the continuum limit. Here, PF-independence occurs at the level of dimensionless observables even at finite lattice spacing.

### 8.4 Formalization [CONJECTURE]

For any observable $O$ expressible as:

$$O = \text{PF}^a \cdot f(\varpi, \ln 2, \sqrt{2}, N_c, N_{\text{base}}, b_3, N_{\text{eff}})$$

if $O$ is a **dimensionless ratio within a single physics sector**, then $a = 0$.

This is a **conjecture** — it has been verified in the domains above, but a general proof would require a more precise definition of "physics sector" and a systematic survey of all FTD observables.

---

# PART IV: DEEPER CONNECTIONS

> **Epistemic Status:** All claims in Part IV are **[CONJECTURE]**. The algebraic results of Parts I–III stand independently.

---

## §9. Why PF = π/4 Is Not Arbitrary [CONJECTURE]

### 9.1 The Geometric Content

PF = π/4 is not a free parameter — it is the unique answer to: "What fraction of a square lattice face is covered by the maximal inscribed circle?"

This is **the** geometric cost of discretization: the lattice is square, but physical fields (wave equations, flux propagation) have circular symmetry. The mismatch is exactly 1 - PF ≈ 0.215 — the "corner waste" where the circle doesn't reach.

### 9.2 Connection to the Lattice Speed Limit

The speed of causality C = 1 means one lattice node per tick. A wave propagating isotropically on a square lattice effectively "fills" a circle inscribed in the square lattice face at each step. The fraction of the face accessed per tick is PF. The inaccessible corners (fraction 1 - PF) represent the anisotropy cost of discrete propagation.

### 9.3 Why √(PF), Not PF, Appears in G*

G* operates on **lengths** (exchange rate per lattice spacing), not areas (exchange rate per lattice face). The length-scale version of the packing fraction is √(PF) = √(π/4) = √π/2 ≈ 0.886 — the fraction of a lattice edge that a diameter covers. G* = ϖ/√(PF) is a length-domain bridge.

---

## §10. Connection to the Master Quadratic [CONJECTURE]

### 10.1 The Quadratic in PF Notation

The master quadratic $x^2 - 16G^{*2}x + 16G^{*3} = 0$ in PF notation:

$$x^2 - \frac{16\varpi^2}{\text{PF}} \cdot x + \frac{16\varpi^3}{\text{PF}^{3/2}} = 0$$

### 10.2 The Coefficient 16 [SELECTION]

The coefficient 16 = N_base² = 4² appears because the master quadratic counts **physical degrees of freedom on the minimal 2×2×2 lattice cell**: 24 components - 7 constraints - 1 gauge = 16 (see SPEC_THE_MASTER_QUADRATIC_UNIFIED.md §3).

In PF notation: 16 = N_base² = (4/PF) · π. The factor 4/PF = 4/(π/4) = 16/π ≈ 5.09 is the lattice face area divided by the inscribed circle area — the inverse of the packing efficiency.

### 10.3 Roots and PF [CONJECTURE]

The quadratic's discriminant involves:

$$\Delta = 256G^{*4} - 64G^{*3} = 64G^{*3}(4G^* - 1)$$

In PF form:

$$\Delta = \frac{64\varpi^3}{\text{PF}^{3/2}} \left(\frac{4\varpi}{\sqrt{\text{PF}}} - 1\right)$$

The roots $x_+ \approx 137.036$ (→ 1/α) and $x_- \approx 3.024$ (→ N_c) are dimensionless — consistent with the PF cancellation rule (§8), where dimensionless quantities from the master quadratic should be PF-free. However, the roots are functions of G* = ϖ/√(PF), so PF enters implicitly. The cancellation, if it holds, would require $x_\pm$ to depend only on the combination ϖ/√(PF) (which they do, since they depend only on G*). This is the trivial sense of PF-independence: the roots are functions of G* alone, and G* happens to decompose as ϖ/√(PF).

---

# PART V: CLAIMS TABLE AND CROSS-REFERENCES

---

## §11. Claims Summary

| ID | Claim | Tag | Depends On | Falsification |
|----|-------|-----|------------|---------------|
| **PF-1** | G* = ϖ/√(PF) | [THEOREM] | Definitions of G*, ϖ, PF | Algebraic identity; unfalsifiable |
| **PF-2** | S_BH × T_H = M/2 (PF cancels) | [THEOREM] | Standard BH thermodynamics | Refuted if S×T ≠ M/2 in any BH |
| **PF-3** | A_min = N_base · ln(2) · l_P² | [THEOREM] + [SELECTION] | LQG area spectrum + Immirzi decomposition PF-4 | Refuted if LQG gives different A_min |
| **PF-4** | γ_I = ln(2)/(N_base · PF · √N_c) | [SELECTION] | DL/Meissner Immirzi value | Refuted if correct γ_I differs from ln(2)/(π√3) |
| **PF-5** | 60 = N_base · D_Σ = 4 · (1+2+4+8) | [THEOREM] | Arithmetic | Algebraic identity; unfalsifiable |
| **PF-6** | QFT loop parameter = α/(2^D · PF) | [SELECTION] | Standard QED | Refuted if one-loop parameter ≠ α/(2π) |
| **PF-7** | PF cancels in all dimensionless sector ratios | [SELECTION] | Pattern across 4 domains | Refuted by a dimensionless observable containing PF |
| **PF-8** | Physics expressible through {ϖ, PF, ln(2), √2} + {3,4,7,13} | [CONJECTURE] | Complete framework survey | Refuted by an irreducible constant outside this set |

---

## §12. Cross-References

| Document | Relevant Content |
|----------|-----------------|
| [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) | Constant chain γ → ϖ → M → π → G*; ontological ordering |
| [SPEC_THE_MASTER_QUADRATIC_UNIFIED.md](../archive/ARCH_SPEC_THE_MASTER_QUADRATIC_UNIFIED.md) | Master quadratic definition, coefficient 16, G* properties |
| [DERIV_LATTICE_SCHWARZSCHILD.md](../archive/ARCH_DERIV_LATTICE_SCHWARZSCHILD.md) | Lattice availability f = 1 - r_s/r, holographic bound A_min, PF cancellation in §§12–13 |
| [DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md](DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md) | G* and four forces, 137-lobe structure |
| [EXPLR_CUBOCTAHEDRAL_GEOMETRY.md](../08_structural/EXPLR_CUBOCTAHEDRAL_GEOMETRY.md) | FCC packing fraction (74% — distinct from PF = π/4), kissing number K(3) = 12 |
| [EXPLR_TRIT_INFORMATION_THEORY.md](../08_structural/EXPLR_TRIT_INFORMATION_THEORY.md) | Information-theoretic perspective on G* |

---

## Appendix A: Quick Reference Table

| Quantity | Standard Form | FTD Decomposition | PF Status |
|----------|---------------|-------------------|-----------|
| G* | √2·Γ(1/4)²/(2π) | ϖ/√(PF) | Contains √(PF) |
| S_BH | 4πM² | N_base²·PF·M² | Contains PF |
| T_H | 1/(8πM) | 1/(2·N_base²·PF·M) | Contains PF |
| S×T | M/2 | M/2 | **PF-free** |
| γ_I | ln(2)/(π√3) | ln(2)/(N_base·PF·√N_c) | Contains PF |
| A_min | 4ln(2)·l_P² | N_base·ln(2)·l_P² | **PF-free** |
| σ | π²/60 | N_base·PF²/D_Σ | Contains PF² |
| α/(2π) | α/(2π) | α/(2^D·PF) | Contains PF |

## Appendix B: Disambiguation of "PF" Across FTD Documents

| Document | "PF" Usage | Meaning |
|----------|------------|---------|
| **This document** (canonical) | PF = π/4 | Circle-in-square packing fraction |
| DERIV_LATTICE_SCHWARZSCHILD.md §§11–13 | "PF" (informal) | Planck Frequency context |
| EXPLR_CUBOCTAHEDRAL_GEOMETRY.md | η ≈ 0.7405 | FCC sphere packing fraction (distinct from PF) |

**Convention:** Throughout FTD, **PF ≡ π/4** unless explicitly stated otherwise.
