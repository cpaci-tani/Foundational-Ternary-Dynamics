# Look-Elsewhere Effect Analysis for FTD Constant Matches

**Purpose:** Quantify the search space explored and the probability of chance agreement for FTD's numerical predictions.

**Version:** 1.0
**Date:** January 2026

---

## 1. The Problem

When a numerical match is found (e.g., 1/α = 137.036 to 1.26 ppm), a fair evaluation requires asking:

1. **What was the search space?** How many constructions were tried before finding this one?
2. **What is the null model?** What is the expected distribution of matches under random construction?
3. **What is the look-elsewhere correction?** How does the effective p-value change when accounting for multiple comparisons?

This appendix provides honest answers to these questions.

---

## 2. The α Match (1.26 ppm)

### 2.1 The Claim

The FTD framework produces a quadratic equation:

$$x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$$

where G* = √2 · Γ(1/4)² / (2π) ≈ 2.9586751192 is the lemniscatic constant.

The larger root is:

$$x_+ = 137.036171...$$

This matches the experimental value 1/α = 137.035999177(21) to **1.26 ppm**.

### 2.2 Allowed Construction Class (A Priori)

The following elements are **fixed by the axioms** before any search:

| Element | Source | Status |
|---------|--------|--------|
| 3D cubic lattice | Axiom A1 | Fixed |
| Gauss constraint ∇·J = ρ | Axiom A3 | Fixed |
| 2×2×2 minimal lattice | Minimal non-trivial | Fixed |
| 16 DoF from constraint counting | Theorem T2 | Derived |
| CM curves (maximal automorphism) | Selection S1 | Argued |
| j ∈ {0, 1728} | Only two CM curves with enhanced symmetry | Mathematical fact |
| j = 1728 (cubic compatible) | Selection S2 | Argued |

**Derived from these choices (not searched):**

- G* from lemniscate period
- Quadratic form from dual constraint structure
- Coefficient 16 from lattice DoF

### 2.3 Effective Search Space

The construction class is **extremely constrained**:

| Choice Point | Options | Justification |
|--------------|---------|---------------|
| CM curves | 2 (j = 0 or j = 1728) | Only two with enhanced automorphism |
| Cubic-compatible | 1 (j = 1728 only) | Z/6Z doesn't embed in octahedral group |
| Coefficient | 1 (= 16) | Fixed by lattice DoF count |
| Polynomial degree | 1 (quadratic) | Minimal for two constraints |

**Effective search space: 1 curve, 1 polynomial form.**

This is not a search over thousands of possibilities. The construction is highly constrained.

### 2.4 What Was Actually Explored

To be transparent, the following alternatives **were considered** during framework development:

| Alternative | Why Rejected |
|-------------|--------------|
| j = 0 curve | Not compatible with cubic symmetry |
| Cubic polynomial | No second constraint to justify third root |
| Coefficient 24 (Leech lattice) | Wrong lattice dimension |
| Different normalization of G* | Breaks period integral identity |

**Estimated exploration factor: ~10 alternatives tried.**

### 2.5 Null Model

Under the null hypothesis (no underlying structure), the quadratic root is uniformly distributed in a plausible range [100, 200].

Probability of matching 137.036 within 1.26 ppm:

$$P(\text{match}) = \frac{137.036 \times 1.26 \times 10^{-6} \times 2}{100} \approx 3.5 \times 10^{-6}$$

### 2.6 Look-Elsewhere Correction

Accounting for the exploration factor (~10 alternatives) and normalization choices (~5):

$$P_{\text{corrected}} \approx 3.5 \times 10^{-6} \times 50 \approx 1.7 \times 10^{-4}$$

**Interpretation:** A 1.26 ppm match has probability ~0.02% under the null model with generous look-elsewhere correction.

---

## 3. The N_c Match (0.8%)

### 3.1 The Claim

The smaller quadratic root is:

$$x_- = 3.023964...$$

This lies within 0.8% of N_c = 3 (the number of color charges in QCD).

### 3.2 Null Model

Under random quadratic with roots in [100, 200] for x₊:

- The sum x₊ + x₋ = 16(G*)² ≈ 140
- So x₋ = 140 - x₊ is uniformly distributed near 3 when x₊ ≈ 137

Probability of x₋ ∈ [2.9, 3.1]:

$$P(x_- \in [2.9, 3.1]) \approx \frac{0.2}{10} = 0.02$$

**Interpretation:** A second root near 3 is not highly improbable (~2%), but noteworthy when combined with the α match.

---

## 4. Combined Assessment

### 4.1 Joint Probability

If the two matches were independent:

$$P_{\text{combined}} \approx 1.7 \times 10^{-4} \times 0.02 \approx 3 \times 10^{-6}$$

However, the matches are **not independent**—they come from the same quadratic, so x₊ constrains x₋.

### 4.2 Honest Summary

| Statement | Assessment |
|-----------|------------|
| The α match is unlikely under null model | **True** (p ~ 10⁻⁴ corrected) |
| The α match *proves* underlying structure | **False** (alternative constructions may exist) |
| The construction is heavily searched | **False** (very constrained class) |
| The agreement is *predicted* from first principles | **Overclaim** (selection principles involved) |

**Recommended language:**

> "The framework is *consistent* with the observed value of α to 1.26 ppm. The construction class is highly constrained (single CM curve, fixed coefficient), limiting the look-elsewhere effect. However, we do not claim this constitutes proof of underlying structure—only that the numerical agreement is noteworthy and warrants further investigation."

---

## 5. Monte Carlo Verification

### 5.1 Purpose

To rigorously quantify the look-elsewhere effect, we provide a Monte Carlo script that:

1. Samples alternative constructions within allowed variations
2. Computes resulting "α-like" values for each
3. Reports the fraction achieving ≤ 1.26 ppm match with 137.036

### 5.2 Script

See `simulations/look_elsewhere_monte_carlo.py` for implementation.

### 5.3 Preliminary Results

**Status: To be computed**

Expected output format:

```
Look-Elsewhere Monte Carlo Analysis
===================================
Samples: 100,000
Variations: j ∈ [1700, 1756], coefficient ∈ [14, 18]

Results:
  Matches ≤ 1.26 ppm: N
  Fraction: N / 100,000
  Implied p-value: X.XX × 10⁻Y

Conclusion: [interpretation]
```

---

## 6. What Would Strengthen the Claim

The α match would be more compelling if:

1. **Multiple independent predictions** from the same G* matched experiment
2. **Novel prediction** (not known when framework was developed) subsequently confirmed
3. **Alternative constructions** were exhaustively ruled out
4. **The discrepancy** (1.26 ppm) was explained by a computable QED correction

Current status:

- (1) Partial: electron mass (0.27%) and weak mixing (0.19%) also match, but may not be independent
- (2) Not yet: predictions (4th generation, tensor mode) are post-dictions or untested
- (3) Not done: exhaustive search of alternatives not performed
- (4) Not done: specific QED diagram calculation not performed

---

## 7. Conclusion

The FTD α match is:

- **Noteworthy:** 1.26 ppm is unlikely under naive null model
- **Constrained:** The construction class is small, limiting look-elsewhere
- **Not proof:** Alternative explanations cannot be excluded

We report this match as an **observation requiring explanation**, not as proof that the framework captures fundamental physics.

---

*Appendix prepared for peer review transparency.*
