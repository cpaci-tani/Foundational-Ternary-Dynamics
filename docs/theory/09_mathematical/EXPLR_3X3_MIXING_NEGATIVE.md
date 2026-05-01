# EXPLR — 3×3 Mixing Matrix Generalization: Honest Negative Result

**Document type:** Exploratory brainstorm (negative result)
**Status:** [STRUCTURAL OBSERVATION — NEGATIVE] — the 2×2 master-quadratic-as-mixing reading does NOT extend cleanly to 3×3; FTD's mode count for EM-color appears to be specifically 2
**Created:** 2026-05-01 evening (continuing harmonic-conjugacy brainstorm)
**Provenance:** User request "let's look at 3×3" following the 2×2 mixing matrix interpretation
**Related:** `EXPLR_MASTER_QUADRATIC_AS_MIXING_MATRIX.md` (the 2×2 reading); `SPEC_PHYSICS_BRIDGE.md`

---

## 0 · Question and result

**Question:** the 2×2 master quadratic gives `(1/α, N_c)` as symmetric/antisymmetric eigenmodes. Does a natural 3×3 generalization yield a triple of SM constants (e.g., lepton masses, gauge couplings)?

**Result:** **No.** Within the natural FTD structures examined, no 3×3 mixing matrix produces a clean SM-matching triple. The 2×2 master quadratic appears to be **structurally specific to a 2-mode system**, not extensible to higher-rank mixings via the same template.

This is an honest negative result, recorded for future reference so the same exploration isn't repeated.

---

## 1 · 3×3 candidates explored

### 1.1 · Sub-blocks of the 4×4 A_{1g} matrix

The natural 4×4 matrix from the linear theorem (`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`) acts on the (center, SC, FCC, BCC) orbit basis. Dropping one orbit gives a 3×3:

| Drop | Eigenvalues |
|---|---|
| CENTER | (-1.92, -4.19, -4.55) |
| SC | (-2.61, -4.00, -4.72) |
| FCC | (-2.61, -4.00, -4.72) |
| BCC | (-1.92, -4.19, -4.55) |

**Observation:** the CENTER↔BCC and SC↔FCC pairing reflects the structural duality between innermost/outermost and middle orbits in the 27-block (they have the same eigenvalue spectrum). This is itself a structural fact worth noting.

**SM match:** none of these eigenvalue triples match any obvious SM constant or ratio. The numerical values are O(1) and concentrated in [-1.9, -4.7]; SM ratios span 10²-10³.

### 1.2 · Fully-symmetric 3×3 with FTD-natural entries

A matrix `[[d, c, c], [c, d, c], [c, c, d]]` has eigenvalues `(d + 2c, d - c, d - c)` — only **2 distinct eigenvalues** by S_3 symmetry. The "antisymmetric" eigenvalue is doubly degenerate.

With diagonal `d = 8G*²` and off-diagonal `c = √(64G*⁴ − 16G*³)` (matching the 2×2 reading):

```
eigenvalues = (3.024, 3.024, 204.04)
              \____ degenerate ____/  (= N_c, doubled)
                                     (no SM match)
```

The N_c eigenvalue appears twice (degenerate), and the third eigenvalue is 204.04 — not a recognized SM constant.

**Verdict:** the master-quadratic-style symmetric coupling, applied to a 3×3 system, gives a "1+2 splitting" with N_c doubled — not a 3-mode system mapping to 3 distinct SM constants.

### 1.3 · Searches over off-diagonal value (varying c)

Diagonal fixed at `8G*²`, varied off-diagonal:

| Off-diagonal | Eigenvalues |
|---|---|
| 1 | (69.03, 69.03, 72.03) — all close to diagonal |
| G* (2.96) | (67.07, 67.07, 75.95) |
| G*² (8.75) | (61.28, 61.28, 87.54) |
| G*³ (25.90) | (44.13, 44.13, 121.83) |
| G*⁴ (76.63) | (-6.60, -6.60, 223.29) |
| √(64G*⁴ − 16G*³) (67.01) | (3.02, 3.02, 204.04) |

None of these triples match any obvious SM constant pattern.

### 1.4 · 3×3 corresponding to specific SM triples

**Lepton mass ratios** (1, 207, 3477):
- Trace = 3685, det = 719,439, e₁e₂+e₁e₃+e₂e₃ = 723,423
- No clean 3×3 mixing matrix structure with FTD-natural entries reproduces these eigenvalues
- A post-hoc scan finds: trace ≈ 48·G*⁴ at 0.2% — but this is **fishing** (single-parameter scan over p in 16·G*^p), not derivation. Per CLAUDE.md anti-target: not promoted.

**Gauge coupling reciprocals at m_Z** (128, 30, 8.5):
- Trace ≈ 167, no clean FTD-structural form
- Different energy scale than other SM constants, complicating matching

---

## 2 · Why the 2×2 reading is structurally special

The 2×2 master-quadratic-as-mixing reading has these special features that don't extend to 3×3:

### 2.1 · The harmonic invariant is a 2-eigenvalue identity

`1/y_+ + 1/y_- = 1` (Theorem 8, FTD-0111) is specifically a 2-eigenvalue relation. It corresponds to the Vieta identity:

```
sum_of_inverse_roots = (sum_of_roots) / (product_of_roots)
                      = (linear coeff) / (constant term)
```

For a degree-3 polynomial, the analog would be:

```
1/y_1 + 1/y_2 + 1/y_3 = (e_1·e_2 + e_1·e_3 + e_2·e_3) / (e_1·e_2·e_3)
                       = (quadratic coeff) / (constant term)
```

This is a 3-eigenvalue identity but doesn't have the same Kirchhoff parallel-equivalent form. The "harmonic conjugacy" interpretation specifically requires 2 modes.

### 2.2 · The (1+i)-tower is degree-2 at every level

FTD-0111's tower `M_k(x) = x² − 2^k·G*^(k−2)·x + 2^k·G*^(k−1) = 0` is **degree-2 at every level k**. Higher levels don't give higher-degree polynomials — they give different degree-2 polynomials. The tower doesn't naturally produce degree-3 or higher polynomials.

### 2.3 · 2-mode systems are physically distinguished

In physics, 2-state systems (qubits, two-level atoms, isospin doublets, etc.) are fundamentally simpler than N-state systems. The pair (1/α, N_c) being mapped to a 2-mode system is consistent with the SM's 2-sector (EM, color) decomposition at this level. Adding a 3rd sector (weak force) requires bringing in α_W which wasn't part of the master quadratic dual prediction.

**Maybe the master quadratic specifically describes EM-color mixing**, not a higher-rank gauge structure. The 2-mode reading is then the correct one, and 3×3 generalizations are NOT structurally meaningful.

---

## 3 · What this NEGATIVE result tells us

### 3.1 · 2-mode mixing is fundamental, not extensible

The harmonic-conjugacy reading of the master quadratic (commit `09a1569`) is a **structurally specific 2-mode interpretation**. Attempting to generalize to N-mode (e.g., 3×3, 4×4) doesn't yield clean structure within FTD's natural search space.

This is itself a fact about FTD's structure: the dual-prediction is specifically a 2-component object, not a slice of a higher-rank system.

### 3.2 · Open question: is there a 3-mode FTD constant pair?

Maybe FTD has multiple INDEPENDENT 2-mode systems, each describing a different physical pairing. E.g.:
- (1/α, N_c) — EM ↔ color
- Some other pair — weak ↔ ?
- Another pair — Higgs ↔ ?

If true, FTD's full structure is a collection of 2×2 mixing matrices, not a single higher-rank one. This would be consistent with the spine theorems but would require identifying additional master-quadratic-like polynomials for other physical pairings.

This is a research direction, not a session-scale task. The (1+i)-tower's other levels (k=3, 5, 6, ...) give different polynomials with different roots; whether any of them maps to a physically meaningful pair is an open question.

### 3.3 · Implication for Paper A

Paper A should present the master-quadratic-as-2×2-mixing reading **as specific to the 2-mode case**, not claim it as a template for higher-rank gauge structures. This honesty is important: overclaiming "FTD's master quadratic predicts all gauge couplings via a higher-rank mixing matrix" would be fishing.

The 2×2 reading is the right scope. It's structurally rich enough on its own.

---

## 4 · The post-hoc lepton-mass observation (anti-target flagged)

For completeness: scanning `trace_lepton / (16·G*^p)` over p revealed:

```
p=4:  3685 / (16·G*⁴) = 3.0057 ≈ N_c (within 0.2%)
```

i.e., **m_e + m_μ + m_τ ≈ 16·N_c·G*⁴ = 48·G*⁴ at 0.2% precision** (in m_e units).

**Status:** anti-target flagged. This is a single-parameter post-hoc scan, not a derivation. Per FTD-0097 / CLAUDE.md discipline:

- Promotion requires: pre-registered structural argument for why p=4 specifically
- Look-elsewhere analysis on the candidate scan space
- Independent FTD-structural derivation

**Without these, this observation is FISHING and is recorded here ONLY so future "I think I've found it" claims can cross-reference it as already-explored.**

It is NOT promoted. The 0.2% match could be coincidental given the scan over p. Using FTD-0097's monomial-level findings, the catalog is over-rich at this level of structural complexity, so individual matches are not significant without further controls.

---

## 5 · Summary

The 3×3 generalization brainstorm yielded:

1. **Sub-blocks of A_{1g}**: structural duality observed (CENTER↔BCC, SC↔FCC) but no SM match
2. **Fully-symmetric 3×3**: cannot give 3 distinct eigenvalues
3. **Various 3×3 forms**: no clean SM triple emerges
4. **Lepton mass triple**: not naturally produced; post-hoc near-match flagged as fishing

**The harmonic-conjugacy 2×2 reading is structurally specific to 2-mode systems and does not extend cleanly to higher rank.** This is itself a structural observation: FTD's master quadratic describes specifically EM-color mixing, not a slice of higher gauge structure.

For Paper A: present the 2×2 reading at its proper scope. Don't overclaim N-mode generalization.

---

## 6 · LEDGER status

This document does NOT introduce a new LEDGER entry. It records a structurally-informative negative result for the 3×3 generalization question.

The post-hoc lepton-mass observation `trace ≈ 48·G*⁴` is **explicitly NOT promoted** per CLAUDE.md anti-target discipline. Recorded only for cross-reference if future "lepton masses from FTD" claims arise.

---

## 7 · Single-line summary

**The 2×2 master-quadratic-as-mixing reading does NOT extend cleanly to 3×3 within natural FTD structures: sub-blocks of the 4×4 A_{1g} matrix don't match SM triples; fully-symmetric 3×3 forms can't give 3 distinct eigenvalues; specific SM triples (lepton masses, gauge couplings) don't emerge from natural FTD constructions. The harmonic-conjugacy 2-mode reading is structurally specific to (1/α, N_c) and does not template to higher-rank gauge structures. A post-hoc scan finds `m_e+m_μ+m_τ ≈ 48·G*⁴` at 0.2% but is explicitly flagged as fishing per CLAUDE.md anti-target discipline and not promoted. The negative result is itself informative: FTD's master quadratic describes specifically a 2-component pair, with the (1+i)-tower's harmonic invariant being a 2-eigenvalue identity that doesn't generalize to 3-mode systems via the same template.**

---

*End of brainstorm.*
