# The Discrete-Continuous Bridge: The Master Quadratic as Domain Connector

## How x² - 16G*²x + 16G*³ = 0 Unifies Lattice Arithmetic and Lemniscate Analysis

**Date:** February 19, 2026
**Framework:** Foundational Ternary Dynamics v5.26
**Status:** Formal synthesis of bridge structure

---

## Depends On

- [MATH_MASTER_QUADRATIC.md](../01_reference/MATH_MASTER_QUADRATIC.md) — Master quadratic: pure mathematics (Layer 1, claims M-1 through M-15)
- [SPEC_QUADRATIC_PHYSICS_BRIDGE.md](../01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md) — Selection principles SP1-SP6 (Layer 2) and physical correspondences (Layer 3, claims P-1 through P-15)
- [DERIV_GSTAR_PF_BRIDGE.md](DERIV_GSTAR_PF_BRIDGE.md) — G* = ϖ/√(PF) decomposition, PF cancellation rule PF-1 through PF-8
- [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) — Historical/interpretive constant atlas γ → ϖ → M → π → G*
- [DERIV_ALPHA_PRECISION_FORMULA.md](DERIV_ALPHA_PRECISION_FORMULA.md) — 4-term precision formula
- [EXPLR_LOOP_GRID_DUALITY.md](../08_structural/EXPLR_LOOP_GRID_DUALITY.md) — Two-layer ontology (Loop vs Grid)
- [EXPLR_TRIT_INFORMATION_THEORY.md](../08_structural/EXPLR_TRIT_INFORMATION_THEORY.md) — Theta function identity TRIT-1

---

## Honesty Note

This document synthesizes algebraic identities from six existing documents into a unified bridge formalism. The algebraic manipulations are [THEOREM]-level (provable identities). The interpretation of these identities as encoding a "discrete-to-continuous bridge" is [SELECTION] — argued from structural patterns, not uniquely proven. One claim (Fourier self-duality as the bridge equivalence) is [CONJECTURE].

---

## Abstract

The master quadratic x² - 16G*²x + 16G*³ = 0 produces the fine structure constant (x₊ = 137.036) and color charge count (x₋ = 3.024) from a single algebraic equation. Using the decomposition G* = ϖ/√(PF), we rewrite this equation in **PF notation**, revealing that each coefficient factors into a **discrete** component (lattice integers, packing fraction) and a **continuous** component (lemniscate period). We show that the Vieta relations, the discriminant, and the precision formula all admit this factored form. The root ratio x₊/x₋ retains PF dependence — consistent with the PF cancellation rule, since it spans two physics sectors. The theta function identity G* = √(2π) · θ₃(e^{-π})² and the AGM form G* = 2√(ϖ/M) provide independent confirmations that G* is the mathematical object encoding both lattice sums and wave integrals simultaneously.

---

## Part I: The PF Decomposition of the Master Quadratic

### §1.1 The Master Quadratic in PF Notation [THEOREM]

**Theorem 1.1.** *The master quadratic x² - 16G*²x + 16G*³ = 0, under the substitution G* = ϖ/√(PF), becomes:*

$$x^2 - \frac{16\varpi^2}{\text{PF}} \, x + \frac{16\varpi^3}{\text{PF}^{3/2}} = 0$$

*where ϖ ≈ 2.6221 is the lemniscate half-period and PF = π/4 ≈ 0.7854.*

**Proof.** Substitute G* = ϖ/√(PF) directly:

- G*² = ϖ²/PF, so 16G*² = 16ϖ²/PF
- G*³ = ϖ³/PF^{3/2}, so 16G*³ = 16ϖ³/PF^{3/2}

The quadratic x² - 16G*²x + 16G*³ = 0 becomes x² - (16ϖ²/PF)x + (16ϖ³/PF^{3/2}) = 0. ∎

This is an algebraic identity — no approximation is involved.

### §1.2 Discrete and Continuous Factors [SELECTION]

Each coefficient of the master quadratic separates into factors with distinct character:

| Coefficient | Standard Form | PF Form | Discrete Factor | Continuous Factor |
|-------------|---------------|---------|-----------------|-------------------|
| Leading (x²) | 1 | 1 | — | — |
| Linear (x) | 16G*² | 16ϖ²/PF | N_base²/PF | ϖ² |
| Constant | 16G*³ | 16ϖ³/PF^{3/2} | N_base²/PF^{3/2} | ϖ³ |

**Identification of character:**

- **16 = N_base² = 4²**: Counts physical degrees of freedom on the minimal 2×2×2 lattice (24 - 7 - 1 = 16). Also L₃² where L₃ = 4 is the unique non-trivial Lucas perfect square [MQ-22].
- **PF = π/4**: The circle-in-square packing fraction — the fraction of a lattice face occupied by the maximal inscribed circle. Encodes the geometric cost of discretization [PF-1].
- **ϖ ≈ 2.6221**: The lemniscate half-period — a period integral of the self-crossing curve y² = x⁴ - x². Encodes continuous self-referential geometry [MQ-5].

The discrete factors (integers, packing fraction) come from the lattice. The continuous factor (ϖ) comes from elliptic analysis. G* = ϖ/√(PF) is the algebraic object that carries both.

### §1.3 The Coefficient Ratio [THEOREM]

**Theorem 1.2.** *The ratio of constant term to linear coefficient equals G*:*

$$\frac{16G^{*3}}{16G^{*2}} = G^* = \frac{\varpi}{\sqrt{\text{PF}}}$$

**Proof.** Direct cancellation: 16G*³/(16G*²) = G*. ∎

This means the linear and constant coefficients are not independent — they are linked by a single bridge constant G*.

---

## Part II: Vieta Relations as Discrete × Continuous Products

### §2.1 Sum of Roots [THEOREM]

**Theorem 2.1.** *By Vieta's formulas, the sum of roots of x² - 16G*²x + 16G*³ = 0 is:*

$$x_+ + x_- = 16G^{*2} = \frac{16\varpi^2}{\text{PF}} = \frac{N_{\text{base}}^2 \cdot \varpi^2}{\pi/4} = \frac{4N_{\text{base}}^2 \cdot \varpi^2}{\pi}$$

**Numerical verification:** x₊ + x₋ = 137.036 + 3.024 = 140.060 = 16 × (2.9587)² = 16 × 8.754 ✓

The sum factorizes as (discrete integer squared / discrete packing fraction) × (continuous period squared).

### §2.2 Product of Roots [THEOREM]

**Theorem 2.2.** *The product of roots is:*

$$x_+ \times x_- = 16G^{*3} = \frac{16\varpi^3}{\text{PF}^{3/2}}$$

**Numerical verification:** x₊ × x₋ = 137.036 × 3.024 = 414.397 = 16 × (2.9587)³ = 16 × 25.900 ✓

### §2.3 The Root Ratio and Cross-Sector PF Behavior [THEOREM + SELECTION]

**Theorem 2.3.** *The root ratio is:*

$$\frac{x_+}{x_-} = \frac{1 + \sqrt{1 - 1/(4G^*)}}{1 - \sqrt{1 - 1/(4G^*)}} = \frac{1 + \sqrt{1 - \sqrt{\text{PF}}/(4\varpi)}}{1 - \sqrt{1 - \sqrt{\text{PF}}/(4\varpi)}}$$

**Proof.** From the quadratic formula, x± = 8G*²[1 ± √(1 - 1/(4G*))]. Their ratio is [1 + √(1 - 1/(4G*))] / [1 - √(1 - 1/(4G*))]. Substituting 1/G* = √(PF)/ϖ gives the PF form. ∎

**Numerical verification:** x₊/x₋ = 137.036/3.024 = 45.315

**Key observation [SELECTION] (historical, partially superseded by v1.4):** PF does **not** cancel from the root ratio. This is consistent with the PF cancellation rule [PF-7], which states that PF cancels in dimensionless observables *within a single physics sector*. Under the historical paired identification, the root ratio x₊/x₋ was read as spanning two sectors — electromagnetic (x₊ = 1/α) and strong (x₋ ≈ N_c). *(The `x_- ↔ N_c` identification is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`; the "two-sector" reading depended on it.)* The ratio's PF-non-cancellation remains an algebraic statement about the polynomial's coefficients; the physics-sector interpretation post-v1.4 is single-root (`x_+ ↔ 1/α`) plus a mathematical artifact (`x_-`).

PF survival in the cross-sector ratio was historically taken to confirm that PF plays a structural role determining how G* distributes content across the two physics sectors — that reading depends on the now-retired paired identification.

---

## Part III: The Discriminant as Bridge Indicator

### §3.1 Discriminant in PF Form [THEOREM]

**Theorem 3.1.** *The discriminant of the master quadratic, in PF notation, is:*

$$\Delta = (16G^{*2})^2 - 4(16G^{*3}) = 64G^{*3}(4G^* - 1) = \frac{64\varpi^3}{\text{PF}^{3/2}} \left(\frac{4\varpi}{\sqrt{\text{PF}}} - 1\right)$$

**Proof.** Standard discriminant b² - 4ac = (16G*²)² - 4(16G*³) = 256G*⁴ - 64G*³ = 64G*³(4G* - 1). Substitute G* = ϖ/√(PF). ∎

**Numerical verification:** Δ = 64 × 25.900 × (4 × 2.9587 - 1) = 1658 × 10.835 = 17,963 ✓

### §3.2 Critical Point [THEOREM]

**Theorem 3.2.** *The discriminant vanishes when G* = 1/4, equivalently when ϖ/√(PF) = 1/4.*

For the physical values ϖ ≈ 2.622 and PF = π/4, we have G* ≈ 2.959 >> 1/4, placing us deep in the real-root regime (Δ >> 0). The critical PF value where roots would merge is PF_crit = (4ϖ)² ≈ 109.9 — far above the physical PF = 0.785.

### §3.3 Domain Classification [SELECTION]

In the parametric form z² - kG*²z + kG*³ = 0 [MQ-7], the discriminant is Δ(k) = kG*³(kG* - 4). In PF notation:

$$\Delta(k) = \frac{k\varpi^3}{\text{PF}^{3/2}} \left(\frac{k\varpi}{\sqrt{\text{PF}}} - 4\right)$$

The sign of Δ classifies domains:

| Condition | PF Form | Roots | Domain |
|-----------|---------|-------|--------|
| kϖ/√(PF) > 4 | k > 4√(PF)/ϖ | Real | Physics |
| kϖ/√(PF) = 4 | k = 4√(PF)/ϖ | Degenerate | Measurement interface |
| kϖ/√(PF) < 4 | k < 4√(PF)/ϖ | Complex | Reference frame context |

The domain boundary is controlled by the ratio √(PF)/ϖ — a dimensionless number comparing discrete packing geometry to continuous period length. For PF = π/4: k_crit = 4√(π/4)/ϖ = 4/(2ϖ/√π) = 4/G* ≈ 1.352 [MQ-8].

---

## Part IV: The Precision Formula as Bridge

### §4.1 Decomposition of the Bridge Gap ε [THEOREM]

The precision parameter ε = e^π - π - 20 admits a term-by-term decomposition into continuous and discrete components:

**Theorem 4.1.** *The precision parameter can be written as:*

$$\varepsilon = \frac{1}{q_{\text{lem}}} - N_{\text{base}} \cdot \text{PF} - (b_3 + N_{\text{eff}})$$

*where q_lem = e^{-π} is the self-dual lemniscate nome.*

**Proof.** Direct substitution:
- 1/q_lem = e^π (reciprocal of the lemniscate nome — continuous)
- N_base · PF = 4 × π/4 = π (integer × packing fraction — discrete × discrete)
- b₃ + N_eff = 7 + 13 = 20 (framework integers — discrete)

Therefore ε = e^π - π - 20 ✓ ∎

**Structure of ε:**

| Term | Value | Domain | Character |
|------|-------|--------|-----------|
| e^π = 1/q_lem | 23.1407 | Continuous | Reciprocal of the unique Fourier self-dual nome |
| π = N_base · PF | 3.1416 | Discrete × Discrete | Lattice integer × packing fraction |
| 20 = b₃ + N_eff | 20 | Discrete | Sum of framework integers |

The bridge gap ε ≈ 0.00089 is the residual mismatch between the continuous world (e^π) and the discrete world (π + 20). It is small — the two domains nearly agree — but not zero.

### §4.2 The Correction Coefficients [THEOREM]

The 4-term precision formula is:

$$\frac{1}{\alpha} = x_+ - c_1|\varepsilon| + c_2|\varepsilon|^2 - c_3|\varepsilon|^3 - c_4|\varepsilon|^4$$

All correction coefficients are exact rational combinations of framework integers {3, 4, 7, 13}:

| Order | Coefficient | Expression | Numerical |
|-------|-------------|------------|-----------|
| 1st | c₁ = 9/47 | N_c² / (N_c·N_base² - 1) | 0.19149 |
| 2nd | c₂ = 5/64 | (N_eff - 2N_base) / N_base³ | 0.07813 |
| 3rd | c₃ = 4/141 | N_base / (N_c · D) | 0.02837 |
| 4th | c₄ = 141/11 | (N_c · D) / (b₃ + N_base) | 12.8182 |

where D = N_c · N_base² - 1 = 47 is the constraint dimension.

**These coefficients are purely discrete.** They involve only framework integers and their algebraic combinations — no transcendentals, no continuous quantities.

### §4.3 The Precision Formula as Bridge Correction [SELECTION]

The precision formula has the structure:

$$\frac{1}{\alpha} = \underbrace{x_+(G^*)}_{\text{tree level}} - \underbrace{\sum_{n=1}^{4} c_n \cdot |\varepsilon|^n}_{\text{bridge corrections}}$$

where:
- The **tree level** x₊ is a function of G* = ϖ/√(PF), carrying both continuous and discrete content
- The **bridge gap** ε = (continuous nome) - (discrete lattice + integers) measures the mismatch between domains
- The **correction coefficients** c₁...c₄ are ratios of framework integers (purely discrete)

The formula thus reads: *start with the continuous-discrete hybrid (tree level), then apply discrete-coefficient corrections powered by the bridge gap*. Each additional power of |ε| ≈ 0.0009 adds roughly 3 digits of precision.

---

## Part V: The Theta Function Self-Duality Connection

### §5.1 G* as Self-Dual Theta Value [THEOREM]

**Theorem 5.1 (TRIT-1).** *The master quadratic coefficient admits an exact theta function representation:*

$$G^* = \sqrt{2\pi} \cdot \vartheta_3(e^{-\pi})^2$$

*where θ₃(q) = 1 + 2Σ_{n=1}^∞ q^{n²} is the Jacobi theta function of the third kind.*

This is a known identity in the theory of elliptic functions.

**The self-dual nome:** The evaluation point q = e^{-π} is the unique value where θ₃ is its own Fourier transform. From the Jacobi identity:

$$\vartheta_3(e^{-\pi t}) = \frac{1}{\sqrt{t}} \cdot \vartheta_3(e^{-\pi/t})$$

At t = 1: θ₃(e^{-π}) = θ₃(e^{-π}) ✓ — the function equals its own Fourier transform.

### §5.2 Dual Representation [SELECTION]

The theta function θ₃(q) = 1 + 2Σ q^{n²} can be viewed from two perspectives:

| Perspective | Form | Domain |
|-------------|------|--------|
| **Lattice sum** | 1 + 2q + 2q⁴ + 2q⁹ + ... | Discrete: sum over integer indices n² |
| **Fourier integral** | ∫ (Gaussian kernel) dω | Continuous: integral over frequencies |

At q = e^{-π}, these two representations give **identical** values. G* = √(2π) · θ₃² inherits this dual character: it can be computed equally well as a lattice sum (Grid) or as a wave integral (Loop).

This connects directly to the Loop-Grid duality of FTD. The Loop constant G* ≈ 2.959 and Grid constant G = 1/AGM(1,√2) ≈ 0.835 satisfy G*/G = √2 × (2ϖ/√π) × (ϖ/π) ... — but the deeper point is that G* itself, through the theta function, encodes both the discrete and continuous representations simultaneously.

### §5.3 Self-Duality as Bridge Equivalence [CONJECTURE]

Fourier self-duality at q = e^{-π} is the mathematical statement that the discrete representation (lattice sum) and the continuous representation (Fourier integral) of G* are indistinguishable. The lattice IS the continuum at this special point.

If this interpretation holds, then G*'s role as the master quadratic coefficient is not accidental — it is the unique constant that "sees no difference" between discrete and continuous geometry. The master quadratic inherits this bridging property: its coefficients (16G*², 16G*³) carry the self-dual bridge into the physics of coupling constants.

---

## Part VI: The AGM as Convergence Rate

### §6.1 The Arithmetic-Geometric Mean [THEOREM]

The arithmetic-geometric mean M = AGM(1, √2) is the common limit of the iteration:

- a₀ = 1, g₀ = √2
- a_{n+1} = (a_n + g_n)/2 (arithmetic mean — additive, linear)
- g_{n+1} = √(a_n · g_n) (geometric mean — multiplicative, scaling)

The AGM converges to M = π/ϖ ≈ 1.1981. This is a classical result (Gauss, Lagrange).

The two operations have distinct characters:

| Operation | Character | FTD Layer |
|-----------|-----------|-----------|
| Arithmetic mean | Additive, counting, accumulation | Grid (discrete states) |
| Geometric mean | Multiplicative, scaling, interference | Loop (continuous flux) |

### §6.2 G* as AGM Bridge Form [THEOREM]

**Theorem 6.1.** *The bridge constant satisfies:*

$$G^* = 2\sqrt{\frac{\varpi}{M}}$$

*where M = AGM(1, √2) = π/ϖ.*

**Proof.** From G* = 2ϖ/√π and π = ϖ · M:

$$G^* = \frac{2\varpi}{\sqrt{\pi}} = \frac{2\varpi}{\sqrt{\varpi \cdot M}} = \frac{2\varpi}{\sqrt{\varpi} \cdot \sqrt{M}} = \frac{2\sqrt{\varpi}}{\sqrt{M}} = 2\sqrt{\frac{\varpi}{M}} \quad \square$$

**Numerical verification:** 2√(2.6221/1.1981) = 2√(2.1886) = 2 × 1.4794 = 2.9587 = G* ✓

The bridge constant is twice the square root of the ratio of lemniscate period to AGM convergence rate. This form makes explicit that G* measures *how much larger* the self-referential geometry (ϖ) is compared to the arithmetic-geometric reconciliation rate (M).

### §6.3 AGM Convergence and Precision Formula [SELECTION]

The AGM converges quadratically — each iteration doubles the number of correct digits. After n iterations:

| n | a_n | g_n | Agreement |
|---|-----|-----|-----------|
| 0 | 1.000000 | 1.414214 | 0 digits |
| 1 | 1.207107 | 1.189207 | 2 digits |
| 2 | 1.198157 | 1.198154 | 5 digits |
| 3 | 1.198156 | 1.198156 | 11 digits |

This mirrors the precision formula's behavior: each correction term (powered by |ε| ≈ 0.0009) adds roughly 3 digits of precision. The AGM reconciles arithmetic and geometric means; the precision formula reconciles the tree-level G* output with the exact coupling constant.

---

## Part VII: The Complete Bridge Path

### §7.1 The Path from Integers to Physics

```
DISCRETE DOMAIN                       CONTINUOUS DOMAIN
────────────────                      ─────────────────
N_base = 4 (Lucas L_3)               I_4 = int_0^1 dx/sqrt(1-x^4) = 1.311
N_c = 3, b_3 = 7, N_eff = 13        varpi = 2*I_4 = 2.622
PF = pi/4 = 0.785                    G* = varpi/sqrt(PF) = 2.959
16 = N_base^2                        G*^2 = varpi^2/PF = 8.754
                                      G*^3 = varpi^3/PF^{3/2} = 25.900
      |                                    |
      +------------------------------------+
      |  x^2 - 16G*^2 x + 16G*^3 = 0     |
      |  = x^2 - (16*varpi^2/PF) x        |
      |        + (16*varpi^3/PF^{3/2}) = 0 |
      +------------------------------------+
      |                                    |
 x_+ = 137.036 = 1/alpha            x_- = 3.024 (math artifact;
 (EM coupling, FTD-0013 [SMC])      x_- ↔ N_c RETIRED v1.4 §5)

      |                                    |
      +-------- PRECISION FORMULA ---------+
      |                                    |
 epsilon = e^pi - pi - 20 = 0.00089       |
 = (continuous nome) - (discrete)          |
 c_1..c_4 = integer ratios                |
      |                                    |
 1/alpha = x_+ - c_1|eps| + ...     CODATA match < 0.001 ppt
```

### §7.2 Epistemic Status of Each Step [SELECTION]

| Step | From → To | Status | Reference |
|------|-----------|--------|-----------|
| Integration | 4 → I₄ | [THEOREM] (classical analysis) | MQ-4 |
| Doubling | I₄ → ϖ | [THEOREM] (definition) | MQ-5 |
| PF bridge | ϖ, PF → G* | [THEOREM] (algebraic identity) | PF-1 |
| Quadratic | G*, 16 → x₊, x₋ | [THEOREM] (algebra) | MQ-7 |
| EM identification | x₊ → 1/α | [OBSERVED] (1.26 ppm) | MQ-O1 |
| ~~Color identification | floor(x₋) → N_c = 3~~ | **RETIRED** per v1.4 §5 (LEDGER FTD-0014 removed in commit `ca7eb61`); `N_c = 3` independently sourced via `DERIV_NC_FROM_TOPOLOGY.md` | MQ-O2 (retired) |
| Bridge gap | e^π - π - 20 → ε | [THEOREM] (arithmetic) | DCB-6 |
| Precision | x₊, ε, c_i → 1/α | [THEOREM] for the formula inside its ansatz; physical α identification/precision fit remains conjectural/post-hoc | ALPHAP-1 |
| Theta identity | θ₃(e^{-π}) → G* | [THEOREM] (classical) | TRIT-1 |
| AGM form | ϖ/M → G* | [THEOREM] (algebraic) | DCB-10 |

The complete bridge path is composed entirely of [THEOREM]-level steps, with two [OBSERVED] identifications at the final step (roots → physical couplings).

---

## Part VIII: Claims Table and Cross-References

### §8.1 Claims Summary

| ID | Claim | Tag | Depends On | Falsification |
|----|-------|-----|------------|---------------|
| **DCB-1** | Master quadratic in PF form: x² - (16ϖ²/PF)x + (16ϖ³/PF^{3/2}) = 0 | [THEOREM] | PF-1, MQ-7 | Algebraic identity — unfalsifiable |
| **DCB-2** | Vieta sum: x₊ + x₋ = 16ϖ²/PF | [THEOREM] | DCB-1, MQ-12 | Algebraic identity |
| **DCB-3** | Vieta product: x₊ × x₋ = 16ϖ³/PF^{3/2} | [THEOREM] | DCB-1, MQ-13 | Algebraic identity |
| **DCB-4** | PF does NOT cancel in x₊/x₋ (cross-sector) | [THEOREM] | DCB-1 | Verify: root ratio changes under PF variation |
| **DCB-5** | Discriminant: Δ = 64ϖ³/PF^{3/2} × (4ϖ/√PF - 1) | [THEOREM] | DCB-1 | Algebraic identity |
| **DCB-6** | ε = 1/q_lem - N_base·PF - (b₃ + N_eff) | [THEOREM] | ALPHAP-6 | Algebraic identity |
| **DCB-7** | Precision formula = tree level - discrete corrections × bridge gap | [SELECTION] | ALPHAP-1..7 | Interpretation of structure |
| **DCB-8** | G* = √(2π) · θ₃(e^{-π})² encodes both discrete and continuous | [THEOREM] | TRIT-1 | Classical theta identity |
| **DCB-9** | Fourier self-duality IS discrete-continuous equivalence | [CONJECTURE] | DCB-8 | Requires deeper proof |
| **DCB-10** | G* = 2√(ϖ/M) where M = AGM(1,√2) | [THEOREM] | π = ϖM | Algebraic identity |
| **DCB-11** | PF cancels intra-sector but survives inter-sector | [SELECTION] | PF-7, DCB-4 | Test in additional cross-sector ratios |

### §8.2 Cross-References

| Document | Sections Referenced | Key Claims Used |
|----------|-------------------|-----------------|
| [DERIV_GSTAR_PF_BRIDGE.md](DERIV_GSTAR_PF_BRIDGE.md) | §§1-2, §8 | PF-1, PF-7 |
| [FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md](../02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md) | §§3-4 | Historical constant atlas; minimal generating set is definitional/derived, not the canonical α chain |
| [DERIV_ALPHA_PRECISION_FORMULA.md](DERIV_ALPHA_PRECISION_FORMULA.md) | Parts I-III | ALPHAP-1 through ALPHAP-7 |
| [EXPLR_LOOP_GRID_DUALITY.md](../08_structural/EXPLR_LOOP_GRID_DUALITY.md) | §§2-4, §7 | Loop-Grid ontology, AGM reconciliation |
| [EXPLR_TRIT_INFORMATION_THEORY.md](../08_structural/EXPLR_TRIT_INFORMATION_THEORY.md) | §§1-2 | TRIT-1 (theta function identity) |

---

*Document created: February 19, 2026*
*Framework: Foundational Ternary Dynamics v5.26*
*Topic: The master quadratic as the formal bridge between discrete lattice arithmetic and continuous lemniscate analysis*
