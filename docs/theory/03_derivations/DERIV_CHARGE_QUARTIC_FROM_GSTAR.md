# The Charge Quartic: EM and Color Charges from the Master Quadratic

**Date:** March 6, 2026
**Framework:** Foundational Ternary Dynamics v5.27
**Status:** 6 theorems, 1 selection principle
**Category:** 3 (Core Physics Derivations), Entry 3.27

---

## Abstract

The FTD master quadratic x^2 - 16G\*^2 x + 16G\*^3 = 0 has roots x+ ~ 137.036 (= 1/alpha) and x- ~ 3.024 (~ N_c). We show that the substitution e^2 = 1/x transforms this into the **charge quartic**

> 16G\*^3 e^4 - 16G\*^2 e^2 + 1 = 0

whose two roots are the electromagnetic and color charge scales:

| Root | Value | Physical identification |
|------|-------|------------------------|
| e^2_EM = 1/x+ | 0.007297 | Fine structure constant alpha |
| e^2_C = 1/x- | 0.33069 | Color charge scale squared |

This is pure algebra — the quartic IS the master quadratic viewed from charge space rather than coupling space. The leading and constant coefficients swap (reciprocal polynomial duality), while the linear coefficient is shared.

**Key identities (all [THEOREM]):**
- Vieta sum: e^2_EM + e^2_C = 1/G\*
- Vieta product: e^2_EM * e^2_C = 1/(16G\*^3)
- Product identity: e_EM * e_C = 1/(4G\*^(3/2))
- Splitting parameter: e^2_C / e^2_EM = x+/x- = (1+d)/(1-d), d = sqrt(1 - 1/(4G\*))

**Verification:** All identities confirmed to machine epsilon (~1e-16) in `scripts/verification/verify_charge_quartic.py`.

---

## 1. The Substitution **[THEOREM]**

### 1.1 From Couplings to Charges

The master quadratic's roots x+ and x- live in **coupling space**: x+ ≈ 137.036 and x- ≈ 3.024. The readings x+ ↔ 1/alpha and x- ↔ N_c are [STRONGLY MOTIVATED CONJECTURE]. Conditional on that bridge, the natural physical quantity is the **charge squared** e^2 = 1/x, which appears directly in interaction vertices.

The substitution x = 1/e^2 transforms from coupling space (where quantities are large: 137, 3) to charge space (where quantities are small: 0.0073, 0.33).

### 1.2 Algebraic Derivation

Starting from the master quadratic:

> x^2 - 16G\*^2 x + 16G\*^3 = 0

Substitute x = 1/e^2:

> (1/e^2)^2 - 16G\*^2 (1/e^2) + 16G\*^3 = 0
>
> 1/e^4 - 16G\*^2 / e^2 + 16G\*^3 = 0

Multiply through by e^4:

> **1 - 16G\*^2 e^2 + 16G\*^3 e^4 = 0**

Rearranging in standard form (highest power first):

> **16G\*^3 e^4 - 16G\*^2 e^2 + 1 = 0**

This is a quartic in e but a **quadratic in u = e^2**:

> 16G\*^3 u^2 - 16G\*^2 u + 1 = 0

### 1.3 Status

**[THEOREM]:** The charge quartic follows from the master quadratic by the substitution e^2 = 1/x and multiplication by e^4. No new physics assumptions are needed. Every root of the master quadratic maps to a root of the charge quartic via e^2 = 1/x, and vice versa.

---

## 2. The Charge-Space Roots **[THEOREM]**

### 2.1 Solving for e^2

The charge quartic as a quadratic in u = e^2:

> 16G\*^3 u^2 - 16G\*^2 u + 1 = 0

By the quadratic formula:

> u = (16G\*^2 +/- sqrt(256G\*^4 - 64G\*^3)) / (2 * 16G\*^3)
>
> u = (16G\*^2 +/- 8G\*^(3/2) sqrt(4G\* - 1)) / (32G\*^3)

### 2.2 Numerical Values

| Root | Expression | Numerical value | Physical ID |
|------|-----------|-----------------|-------------|
| u+ = e^2_C | 1/x- | 0.330691776643504 | Color charge scale squared |
| u- = e^2_EM | 1/x+ | 0.007297343390138 | EM charge squared (tree-level alpha) |

Note: The tree-level value e^2_EM = 1/x+ = 0.00729734... differs from the precision-corrected alpha = 0.00729735... by ~9e-9 (the 4-term correction series; see MATH_MASTER_QUADRATIC.md Section 11).

### 2.3 Physical Identification

- **e^2_EM = alpha** (in Gaussian/natural units where e^2 = 4*pi*alpha_SI): This is the electromagnetic fine structure constant, governing the strength of photon-charged-particle interactions.

- **e^2_C = 1/x-**: This is the color charge scale. While the strong coupling alpha_s runs with energy, the tree-level color charge scale 1/x- ~ 0.33 represents the bare coupling at the lemniscatic unification point.

### 2.4 Residual Cross-Check

Substituting each root back into the quartic:

| Root | Residual |
|------|----------|
| e^2_C = 0.33069... | 0.00e+00 |
| e^2_EM = 0.00730... | -2.44e-15 |

Both residuals are at or below machine epsilon, confirming exact algebraic identity.

---

## 3. Charge-Space Vieta Relations **[THEOREM]**

### 3.1 Sum and Product

For the quadratic 16G\*^3 u^2 - 16G\*^2 u + 1 = 0, Vieta's formulas give:

**Sum of roots:**
> e^2_EM + e^2_C = -(-16G\*^2) / (16G\*^3) = **1/G\***

**Product of roots:**
> e^2_EM * e^2_C = 1 / (16G\*^3)

### 3.2 Numerical Verification

| Identity | LHS | RHS | Difference |
|----------|-----|-----|------------|
| Sum = 1/G\* | 0.337989120033642 | 0.337989120033642 | 7.77e-16 |
| Product = 1/(16G\*^3) | 0.002413171450462 | 0.002413171450462 | 5.64e-18 |

Both hold to machine epsilon — these are exact algebraic consequences.

### 3.3 Derived Identities

**Product identity (square root of Vieta product):**

> e_EM * e_C = sqrt(e^2_EM * e^2_C) = sqrt(1/(16G\*^3)) = **1/(4G\*^(3/2))**

Numerical verification: product = 0.049124041471183, expected = 0.049124041471183, diff = 5.55e-17.

**Inverse charge sum (from coupling-space Vieta):**

In coupling space: x+ + x- = 16G\*^2, x+ * x- = 16G\*^3.
In charge space: 1/x+ + 1/x- = (x+ + x-)/(x+ * x-) = 16G\*^2 / (16G\*^3) = 1/G\*.

This provides an independent derivation of the charge-space Vieta sum.

### 3.4 Physical Significance

The Vieta relations establish a deep connection between coupling space and charge space:

| Domain | Sum | Product |
|--------|-----|---------|
| **Coupling space** | x+ + x- = 16G\*^2 | x+ * x- = 16G\*^3 |
| **Charge space** | e^2_EM + e^2_C = 1/G\* | e^2_EM * e^2_C = 1/(16G\*^3) |

The coupling-space product equals the charge-space product's inverse: the action scale 16G\*^3 appears as product in coupling space and as 1/product in charge space. Meanwhile:

> (coupling product) * (charge sum) = 16G\*^3 * 1/G\* = 16G\*^2 = coupling sum

This chain links all four Vieta quantities through a single factor of G\*.

---

## 4. Inside-Out Duality **[SELECTION]**

### 4.1 The Duality Table

| | Leading coeff | Linear coeff | Constant |
|-|---------------|-------------|----------|
| **Coupling space** | 1 | -16G\*^2 | 16G\*^3 |
| **Charge space** | 16G\*^3 | -16G\*^2 | 1 |

The leading and constant coefficients **swap**. The linear coefficient (-16G\*^2) is **shared** — it is the self-dual element of the duality.

### 4.2 Reciprocal Polynomial

This is the **reciprocal polynomial** (or palindromic) relationship. For any polynomial P(x) = a_n x^n + ... + a_0, the reciprocal polynomial is P\*(x) = a_0 x^n + ... + a_n. If r is a root of P, then 1/r is a root of P\*.

For the master quadratic P(x) = x^2 - 16G\*^2 x + 16G\*^3:
- P\*(x) = 16G\*^3 x^2 - 16G\*^2 x + 1

This is exactly the charge quartic (in u = e^2). The duality is:

> **x^2 P(1/x) = 16G\*^3 x^2 - 16G\*^2 x + 1 = 0** (charge quartic)

### 4.3 Interpretation

The "inside-out" metaphor: coupling space views physics from the perspective of large-scale structure (1/alpha ~ 137 lattice sites per EM wavelength), while charge space views the same physics from the perspective of interaction vertices (alpha ~ 0.0073 coupling per event). These are not different theories — they are the same quadratic equation read in two directions.

**Status:** The algebra (coefficient swap, reciprocal polynomial) is **[THEOREM]**. The "inside-out" physical interpretation is **[SELECTION]** — it is one way to read the algebraic duality, not a unique or proven interpretation.

---

## 5. Relation to Dual Substrate **[THEOREM]**

### 5.1 Connection to Psi = J_L + J_R = G\* per DoF

The dual substrate framework (DERIV_DUAL_SUBSTRATE_IDENTITY.md) establishes that the observable field psi = J_L + J_R = G\* per degree of freedom. The charge-space Vieta sum

> e^2_EM + e^2_C = 1/G\*

states that the squared charges sum to the **inverse** of the per-DoF flux amplitude. In other words: the total charge-squared budget available per DoF is exactly 1/G\* = 1/psi.

### 5.2 Splitting Parameter

The splitting parameter d measures how asymmetrically the master quadratic's roots are distributed:

> d = (x+ - x-)/(x+ + x-) = sqrt(1 - 1/(4G\*))

This gives:

> x+/x- = (1+d)/(1-d) = e^2_C / e^2_EM

Numerically: d = 0.956819..., ratio = 45.317.

The high asymmetry (d close to 1) reflects the enormous hierarchy between EM and color charges: the color charge squared is ~45 times larger than the EM charge squared, or equivalently, the EM coupling is ~45 times weaker than the color charge scale.

**Derivation of d:**

Define x+/- = 8G\*^2(1 +/- d). Then:
- x+ * x- = 64G\*^4(1 - d^2) = 16G\*^3 (Vieta product)
- Solving: 1 - d^2 = 1/(4G\*)
- Therefore: **d^2 = 1 - 1/(4G\*)** = (4G\* - 1)/(4G\*)

Numerical check: d from formula = 0.956819063350846, d from roots = 0.956819063350845, diff = 1.11e-16.

**Connection to FOUND_LADDER_GENERATING_RULE.md:** The identity x+/x- = (1+d)/(1-d) was independently proven in that document using the elegant form d = (x+ - x-)/(x+ + x-). The charge quartic provides the charge-space reading: the ratio of squared charges equals the splitting ratio.

---

## 6. Epistemic Summary

| # | Claim | Tag | Rationale |
|---|-------|-----|-----------|
| 1 | Charge quartic 16G\*^3 e^4 - 16G\*^2 e^2 + 1 = 0 | **[THEOREM]** | Pure algebra: substitute e^2 = 1/x into master quadratic |
| 2 | Roots e^2_EM = 1/x+, e^2_C = 1/x- | **[THEOREM]** | Direct evaluation from quadratic formula |
| 3 | Vieta sum: e^2_EM + e^2_C = 1/G\* | **[THEOREM]** | Vieta's formulas applied to charge quartic |
| 4 | Vieta product: e^2_EM * e^2_C = 1/(16G\*^3) | **[THEOREM]** | Vieta's formulas applied to charge quartic |
| 5 | Product identity: e_EM * e_C = 1/(4G\*^(3/2)) | **[THEOREM]** | Square root of Vieta product |
| 6 | Splitting: e^2_C/e^2_EM = (1+d)/(1-d) | **[THEOREM]** | Algebraic identity from d definition |
| 7 | "Inside-out" duality interpretation | **[SELECTION]** | Physical reading of reciprocal polynomial structure |

**Simulation verification:** All 6 algebraic identities confirmed in `scripts/verification/verify_charge_quartic.py` (6/6 tests pass, all residuals < 1e-12).

---

## Cross-References

| Document | Relationship |
|----------|-------------|
| MATH_MASTER_QUADRATIC.md | Layer 1 mathematics; this document is the charge-space dual |
| FOUND_LADDER_GENERATING_RULE.md | Independent proof of x+/x- = (1+d)/(1-d) |
| DERIV_DUAL_SUBSTRATE_IDENTITY.md | psi = G\* per DoF; charge sum = 1/psi |
| EXPLR_GSTAR_FLUX_TIME.md | G\*^3 as action scale; appears as 1/(charge product) |
| FOUND_FORCE_STRUCTURE.md | Four forces from master quadratic; charge quartic adds charge-space view |
| PAPER_CONSCIOUSNESS_QFT_GR_BRIDGE.md | Section 2.6 integrates this result |
| scripts/verification/verify_charge_quartic.py | Numerical verification (6/6 tests pass) |
