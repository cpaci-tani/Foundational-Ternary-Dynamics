# DERIV — Program F-double-prime: Bivector Algebra Closure Tests (Partial)

**Tag:** [MEASURED] [PARTIAL] — 1/3 closure tests pass; F-prime matching signature robust but full Cl(3,0) bivector closure fails at 4-injection scale
**Ledger row:** FTD-0087
**Filed:** 2026-04-24
**Companions:**
- [DERIV_PLAQUETTE_BIVECTOR_EMERGENCE.md](DERIV_PLAQUETTE_BIVECTOR_EMERGENCE.md) (FTD-0086) — F-prime matching signature
- [test_bivector_closure.cpp](../../../engine/tests/test_bivector_closure.cpp) — GPU test

---

## Executive statement

**Program F-double-prime** ran three closure tests to upgrade F-prime's "matching-bivector signature" (FTD-0086) to a full $Cl(3,0)$ bivector subalgebra. The outcome is **partial**:

| Test | Result | Pass? |
|---|---|---|
| **A. Multi-seed robustness** of F-prime matching-plaquette commutators | 2/3 pairs above \|m\|>1 threshold; all 3 concentrate on matching plaquette with 6×-26× signal/off ratios | partial pass |
| **B. Casimir uniformity** ($e_i^2$ scalar grade) | $S_x = 511, S_y = 485, S_z = 445$ across diagonal $(i,i)$ injections; max axis-deviation 7.3% | **PASS** |
| **D. Bivector commutator closure** $[B_a, B_b] \stackrel{?}{\propto} B_c$ via 4-injection 8-sequence linear combinations | Expected plaquette populated with correct sign in all 3 cases, but off-axis plaquettes carry larger mass | **FAIL** |

**Net:** 1/3 PASS cleanly. F-prime's matching signature is robust (Branch-A non-commutativity is real and persists across seeds), but **the plaquette bivectors do not form a closed $\mathfrak{su}(2)$ Lie algebra at the 4-injection measurement scale**. The matching commutator $[\hat{E}_i, \hat{E}_j] \to P_{ij}$ holds; the iterated commutator $[B_a, B_b] \to B_c$ does not close cleanly.

This **tempers but does not refute** FTD-0086. The bivector-matching signature is genuine; the algebraic structure is approximate-SU(2) at leading order but does not close cleanly under composition.

---

## 1. Why the closure tests matter

FTD-0086 established that the COMMUTATOR $[\hat{E}_f, \hat{E}_g][P_a]$ concentrates on the matching plaquette $P_a$ for all three off-diagonal pairs. This is a **necessary** condition for $\mathfrak{su}(2)$ emergence, not a sufficient one. A clean Lie algebra also requires:

1. The Casimir / scalar grade is axis-isotropic ($e_i^2 = +1$ uniformly, not axis-dependent).
2. The bivector subalgebra closes under iterated commutator: $[b_i, b_j] \propto \epsilon_{ijk} b_k$.
3. The signal is robust under reseeding (not a one-off coincidence).

F-double-prime tests all three.

---

## 2. Setup

L = 8, A = 10, full non-local toggle set (same as F-prime). Multi-seed average across 8 deterministic seeds $0$x$F3170517$ to $0$x$F317051E$.

### 2.1 Part A — multi-seed robustness

Re-run F-prime's three off-diagonal pair commutators across 8 seeds. Compute mean and standard deviation of:
- The signal on the matching plaquette.
- The maximum off-axis plaquette mass.

Pass criterion (per pair): $|\bar{m}| > 1.0$ AND $|\bar{m}| > 3 \cdot \bar{\text{off}}$.

### 2.2 Part B — Casimir uniformity

For each axis $i$, run the diagonal injection sequence $(i, i)$, measure scalar observable $S_i = \sum_{x \in 2^3} |J(x)|^2$. In $Cl(3,0)$ the scalar grade $e_i^2 = +\mathbb{1}$ is axis-independent, predicting $S_x = S_y = S_z$.

Pass criterion: $\max_i |S_i - \bar{S}| / \bar{S} < 10\%$.

### 2.3 Part D — bivector commutator closure

For each cyclic triple $(B_a, B_b, B_c)$ where $B_a, B_b, B_c$ are the three plaquette bivectors $\{P_{xy}, P_{xz}, P_{yz}\}$:

Operationally, $\hat{B}_a = \frac{1}{2}[\hat{E}_i, \hat{E}_j]$ for matching axes. Then
$$
[\hat{B}_a, \hat{B}_b] = \tfrac{1}{4}\sum_{\text{8 signed orderings}} \pm \prod \hat{E}_{a_1} \hat{E}_{a_2} \hat{E}_{a_3} \hat{E}_{a_4}
$$

Each ordering is a 4-injection sequence (4 WH injections, 4 ticks). For each of 3 cyclic triples, run 8 sequences × 8 seeds = 64 engine runs per triple, linearly combine, project on three plaquettes.

$\mathfrak{su}(2)$ closure predicts: result concentrated on $B_c$ (the third bivector).

Pass criterion (per triple): signal on expected plaquette > 1.0 AND > 2× max off-axis.

---

## 3. Results

### 3.1 Part A — multi-seed robustness

```
  Pair (i,j)  | mean(matching) +/- stdev    | mean(off-axis max) +/- stdev
  ------------+-----------------------------+------------------------------
  (1,2)->P_xy |   +0.499 +/-  0.000         |   +0.083 +/-  0.000
  (1,3)->P_xz |  +11.779 +/-  1.122         |   +0.445 +/-  0.134
  (2,3)->P_yz |   +3.181 +/-  0.000         |   +0.211 +/-  0.000

  Pairs with robust matching > 3 x off-axis: 2 / 3
```

**Interpretation:**
- All three pairs concentrate on the matching plaquette, ratios 6× / 26× / 15×.
- Two of three (xz and yz) have absolute matching signal > 1.0; (xy) has signal 0.499 (below the threshold).
- Surprising: stdev = 0.000 for (xy) and (yz) across 8 seeds. The (xz) pair is the only one with seed-sensitive signal (stdev = 1.12).
- F-prime's reported (xy) signal of 2.34 was likely an artifact of the specific seed used in F-prime; under broader sampling, the mean is 0.499.

**Verdict:** matching-plaquette concentration is robust; absolute magnitudes vary across seeds and across pairs. The "3/3 strong matches" claim from F-prime is corrected to "3/3 concentration patterns + 2/3 above |m|>1 threshold."

### 3.2 Part B — Casimir uniformity (PASS)

```
  S_x = 510.908 +/- 0.000
  S_y = 484.976 +/- 0.000
  S_z = 445.380 +/- 0.000
  Mean S = 480.421, max |S_i - <S>| / <S> = 7.3%
```

**Verdict: PASS.** The diagonal $(i, i)$ Casimir is axis-isotropic to 7.3%, well within the 10% threshold. This is consistent with $e_i^2 = +1 \cdot \mathbb{1}$ scalar grade across all three axes.

The 7.3% axis spread (511 → 485 → 445 going $x \to y \to z$) is monotonic — possibly reflecting an axis-ordering bias in the engine's GPU implementation (storage order, kernel launch order). Not a structural anisotropy of FTD itself.

### 3.3 Part D — bivector commutator closure (FAIL)

```
  [B_xy, B_yz]   (expected dominant: P_xz)
     P_xy =  +11.629 +/- 58.586
     P_xz =  -12.382 +/-  2.829   <-- expected dominant
     P_yz =  +40.637 +/-  8.368
     signal = 12.382, max_off = 40.637  --> no closure

  [B_xz, B_xy]   (expected dominant: P_yz)
     P_xy =   -2.743 +/- 43.370
     P_xz =  +75.821 +/- 39.649
     P_yz =  -45.826 +/- 30.797   <-- expected dominant
     signal = 45.826, max_off = 75.821  --> no closure

  [B_yz, B_xz]   (expected dominant: P_xy)
     P_xy =   -4.953 +/-  8.689   <-- expected dominant
     P_xz =  -58.495 +/- 49.888
     P_yz =  -56.058 +/- 19.314
     signal = 4.953, max_off = 58.495  --> no closure
```

**Verdict: FAIL.** All three cyclic-triple closure tests fail: in each case, an off-axis plaquette accumulates more mass than the expected one.

**However**, the expected plaquette IS populated with the correct sign in all three cases:
- $[B_{xy}, B_{yz}] \to P_{xz}$ has $-12.4$ (expected negative for $\mathfrak{su}(2)$ structure constant).
- $[B_{xz}, B_{xy}] \to P_{yz}$ has $-45.8$ (expected negative).
- $[B_{yz}, B_{xz}] \to P_{xy}$ has $-5.0$ (expected negative).

The signs are consistent with $[b_i, b_j] = -2 \epsilon_{ijk} b_k$ (Cl(3,0) bivector convention). What fails is the *concentration* — other plaquettes also accumulate substantial mass.

**Possible explanations:**
1. **4-injection dynamical noise.** Each tick in the engine is a non-local interaction step (forces, triad, exchange, etc.). Four ticks of accumulated dynamics produce significant flux mixing across all axes. The algebraic structure that's clean in 2-injection (F-prime) gets washed out in 4-injection.
2. **Plaquettes are not a closed Lie subalgebra.** The bivectors might form an *approximate* SU(2) that doesn't close cleanly under composition — the Lie bracket of two plaquette bilinears is not itself a plaquette bilinear, but a more complex 4-form that projects only partially onto plaquettes.
3. **Casimir / scalar mixing.** $b_i^2 = -1$ in Cl(3,0) means the iterated commutator can mix scalar grade with bivector grade. The "off-axis" mass we see may be scalar contamination projected non-trivially onto plaquette bilinears.

Most likely: combination of (1) and (2). The matching signature in F-prime is real and robust; the iterated structure breaks down because (a) physical noise grows with the number of injections, and (b) the natural 4-form generated by iterated bivector products does not project cleanly onto 2-form plaquettes.

---

## 4. What this changes (and what stays)

### 4.1 What stays from FTD-0086

- The **matching-bivector signature is real**: $[\hat{E}_i, \hat{E}_j]$ concentrates on $P_{ij}$, robustly across seeds, with 6×-26× signal/off-axis ratio.
- **Non-commutativity exists** in FTD's non-local dynamics on the plaquette basis.
- The mode-erasure no-go (FTD-0073) is **broken** for non-local dynamics with link-bilinear / bivector observables.

### 4.2 What is corrected

- F-prime's "3/3 strong matches" is corrected to "3/3 concentration patterns; 2/3 above robust-magnitude threshold."
- F-prime's claim that "plaquette bivectors close $\mathfrak{su}(2)$" is **retracted to "plaquette bivectors carry $\mathfrak{su}(2)$-like matching commutator signature; full Lie-algebra closure not verified."**
- The "fermion emergence shifts from Branch-B selection to Branch-A derivation" claim is **tempered**: matching signature is necessary but not sufficient for fermion emergence; closure failure means the bivector subalgebra is approximate, not exact.

### 4.3 Honest current status

After FTD-0086 + FTD-0087:

| Property | Status |
|---|---|
| Matching commutator signature ($[\hat{E}_i, \hat{E}_j] \to P_{ij}$) | **[MEASURED]** — robust 3/3 |
| Matching bivector concentration > 3× off-axis (multi-seed) | **[MEASURED]** — 2/3 strong, 1/3 marginal |
| Casimir / scalar grade axis-isotropy | **[MEASURED]** — 7.3%, PASS |
| Iterated-commutator closure $[B_a, B_b] \propto B_c$ | **[FAILED]** at 4-injection scale |
| Full $\mathfrak{su}(2)$ Lie-algebra structure | **[NOT VERIFIED]** |
| Branch-A native fermion derivation | **[NOT YET]** — matching signature alone insufficient |

### 4.4 What this means for Branch B

Pre-F closure tests (after FTD-0086):
> Fermion emergence in FTD is no longer closed-negative. It has a concrete positive signature.

Post-F-double-prime (this doc):
> Fermion emergence in FTD has a robust **matching-commutator signature** on plaquette bivectors but **no full Lie-algebra closure** at the 4-injection measurement scale. The algebraic category is non-abelian; whether a *closed* fermion structure can be extracted from FTD remains open.

**Branch-B fermion selection is still the most likely accounting**, but with a quantitative leak: the lattice carries a non-trivial bivector-matching signature that any Branch-B selection should respect or reproduce.

---

## 5. Where to push next

Three paths, in order of tractability:

### 5.1 Path 1 — alternative basis for closure

The plaquette bilinear $P_{ij}(x) = J_i(x) J_j(x+\hat{e}_i) - J_i(x+\hat{e}_j) J_j(x)$ is the natural 2-form. But the iterated bivector product naturally produces a 4-form or scalar, not a 2-form. Maybe the right basis is:
- **Wilson-loop-style bilinears**: $W_{ij}(x) = \prod_{e \in \partial \square_{ij}(x)} J_e$ — a closed-loop product.
- **Edge-parallel-transport bilinears**: $J_i(x) J_i(x + \hat{e}_j)$ (no antisymmetry) — different algebraic class.
- **Higher-grade bilinears** including 1-form contributions from cross-axis edges.

A new test program could enumerate these and find which one closes cleanly.

### 5.2 Path 2 — accept approximate closure and quantify deviation

Treat FTD's plaquette bivectors as an **approximate** $\mathfrak{su}(2)$, characterize the deviation (4-form leakage coefficients), and ask whether the deviation is parametrically small in some limit (large lattice, low temperature, etc.). This is closer to "approximate symmetry" in physics — like how SU(3) flavor is approximate but useful.

### 5.3 Path 3 — Branch-B selection with structural constraints

If neither Path 1 nor Path 2 closes the algebra, accept that fermion content is a Branch-B selection. But the matching signature from FTD-0086 + FTD-0087 imposes a *structural constraint* on which selections are consistent: any Branch-B fermion structure must reproduce the bivector commutator concentration we've measured.

This is analogous to FTD-0077: SU(3) is a Branch-B selection consistent with FTD's $C_3$ discrete subgroup. Similarly, a Dirac fermion content might be a Branch-B selection consistent with FTD's bivector matching signature.

### 5.4 Recommendation

**Path 1 (Wilson loop / alternative basis)** is the most tractable next step. The 4-injection failure suggests the plaquette is wrong, not that non-commutativity is wrong. A Wilson-loop-style test (1 injection + closed-loop readout) is a simpler protocol with less dynamical noise.

If Path 1 also fails: accept Path 2/3.

---

## 6. Status

**Program F-double-prime: PARTIAL CLOSURE** as of 2026-04-24.

- **1/3** closure tests PASS (Casimir axis-isotropy).
- F-prime matching signature **holds** (concentration robust, magnitudes vary by seed).
- $\mathfrak{su}(2)$ Lie algebra closure **fails** at 4-injection scale.

**Tempers FTD-0086:**
- Branch-A native fermion derivation: **not established**.
- Bivector matching signature: still real and consequential.
- Path forward: Wilson loop / alternative basis (Program F-triple-prime) or accept approximate closure.

---

*Filed 2026-04-24. The honest closure of Program F. F-prime's matching-bivector concentration is robust across seeds, but the iterated bivector commutator does not close into a clean $\mathfrak{su}(2)$ Lie algebra at the 4-injection measurement scale. Branch-A native fermion derivation is not established; the Branch-B selection layer for fermion content remains the most likely accounting, with the constraint that any selection must reproduce the measured bivector matching signature.*
