# EXPLR — (1+i)-Tower Multiplier Uniqueness: Substantive Structural Tightening

**Document type:** Exploratory result (positive)
**Status:** [STRUCTURAL OBSERVATION] — substantially tightens the (m=2, k=4) selection in FTD-0111
**Created:** 2026-05-01 evening
**Provenance:** Q1 follow-up from `THEOREM_HARMONIC_INVARIANT_TOWER.md` (FTD-0111); user request "focus on other open gaps" after α-derivation routes documented as exhausted
**Related:** `THEOREM_HARMONIC_INVARIANT_TOWER.md`; `SPEC_ALGEBRAIC_SPINE.md §8`; `EXPLR_PATHS_TO_ALPHA.md` (which this slightly strengthens)

---

## 0 · Summary

A scan over **58 (multiplier, level) pairs** in the (1+i)-tower-style polynomial family — varying Gaussian-integer multipliers `m = a²+b² ∈ {1, 2, 4, 5, 8, 9, 10, 13, 16, 17, 18, 20}` and tower levels `k ∈ {3, 4, 5, 6, 7}` — finds that **(m=2, k=4) is uniquely close to 1/α = 137.036**, with the next-closest match (m=4, k=3 → x_+ = 186.35) off by 36% relative error vs (m=2, k=4)'s 1.26 ppm.

This is a substantive tightening of FTD-0111. Combined with the structural justifications (m=2 from Z[i]=CM ring; k=4 from N_base = O_h trivial-irrep multiplicity), the (1+i, k=4) selection is **maximally rigid within the natural Gaussian-integer-tower family**.

---

## 1 · Setup

The (1+i)-tower from FTD-0111 (Theorem 8) generalizes to multiplier `m`:

```
M_{m,k}(x) = x² − m^k · G*^(k−2) · x + m^k · G*^(k−1)
```

with `G* = Γ(1/4)/Γ(3/4) ≈ 2.9587`. The harmonic invariant
`1/y_+ + 1/y_- = 1` (with `y = x/G*`) holds **generically** for any
`(m, k)` satisfying this normalization — it's not (1+i)-specific.

What IS (1+i)-specific is the actual root values at each level. The
master quadratic (m=2, k=4) gives `x_+ ≈ 137.036` matching CODATA 1/α
to 1.26 ppm.

The question: **is (m=2, k=4) UNIQUELY close to 137.036 among
structurally-natural (m, k) pairs, or is it just one among many that
happens to match?**

---

## 2 · Scan results

Scan parameters: `m` over the 12 smallest distinct Gaussian-integer
norms (excluding zero); `k ∈ {3, 4, 5, 6, 7}`. Total: 58 (m, k) pairs
with real roots.

**Top 5 closest matches to x_+ = 137.036:**

| Rank | m | (a, b) | k | x_+ | rel ppm to 137.036 |
|---|---|---|---|---|---|
| **1** | **2** | **(1+i)** | **4** | **137.036171** | **1.26** |
| 2 | 4 | (2) | 3 | 186.349 | 359,853 (36%) |
| 3 | 1 | (1) | 6 | 73.546 | 463,312 (46%) |
| 4 | 1 | (1) | 7 | 223.720 | 632,563 (63%) |
| 5 | 1 | (1) | 5 | 22.493 | 835,863 (84%) |

**Specific neighbors of (m=2, k=4):**

| (m, k) | x_+ | rel ppm |
|---|---|---|
| (2, 4) | 137.036 | 1.26 |
| (1, 4) | (no real roots) | — |
| (4, 4) | 2238.0 | 15,331,471 |
| (5, 4) | 5468.1 | 38,902,936 |
| (2, 3) | 20.20 | 852,571 |
| (2, 5) | 825.82 | 5,026,267 |
| (8, 4) | 35852 | 260,627,864 |
| (9, 4) | 57430 | 418,090,245 |

**The gap from (m=2, k=4) at rank 1 (1.26 ppm) to rank 2 (36%) is
~5 orders of magnitude in relative error.** No other (m, k) pair in
the family is anywhere close to physical 1/α.

---

## 3 · Structural justifications

### 3.1 · Why m = 2

- **m = 2 = |1+i|²** is the smallest non-trivial norm of a Gaussian
  integer.
- **Z[i] is the CM ring** of the lemniscatic elliptic curve E: y² = x³ − x
  (Theorem 3 / FTD-0003). Among the 9 class-number-1 imaginary
  quadratic fields, d = −4 is uniquely selected.
- **(1+i) is a prime in Z[i]** with `1+i = √2·e^(iπ/4)` — the smallest
  Gaussian prime (associated to the rational prime 2 which ramifies in
  Z[i]).

So m = 2 is **structurally forced** by the CM uniqueness theorem (which
selects Z[i]) plus the smallest-non-trivial-norm constraint (which is
the natural choice within Z[i]).

### 3.2 · Why k = 4

- **k = 4 = N_base** is the multiplicity of A_{1g} (trivial irrep) of
  the cubic point group O_h on the 27-voxel Moore block (DERIV_K_FROM_OH_A1G_MULTIPLICITY,
  FTD-0110 linear theorem).
- **k = 4** is also the dimension of the natural representation of the
  cyclic group Z_4 = {1, i, −1, −i} = units of Z[i].
- **k = 4** appears across multiple FTD layers (number theory: Z_4
  units; group theory: A_{1g} multiplicity; geometric algebra: Cl(3,0)
  grades; particle ontology: 4 manifested states per generation).

So k = 4 is **structurally forced** by the cubic-lattice symmetry
(O_h) acting on the 27-block, and is the same 4 that appears across
FTD's layers (per `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` §6).

### 3.3 · The combination (m=2, k=4)

Combining the two structural justifications:
- Take the smallest non-trivial Gaussian-integer norm: m = 2.
- Take the structural level given by O_h on the 27-block: k = 4.

**These two structural arguments uniquely select (m=2, k=4) within the
(m, k) family.** The empirical observation is that this combination
gives x_+ = 137.036 matching 1/α to 1.26 ppm.

---

## 4 · What this strengthens

Before this scan, FTD-0111 noted "level k = 4 empirical observation,
why-level-4 [OPEN]" as a follow-up. The current scan tightens this:

**Strengthening 1 (RANK):** Among 58 (m, k) pairs with structurally-
natural multipliers and small levels, (m=2, k=4) is RANK 1 by
closeness to physical 1/α, with a 5-orders-of-magnitude gap to rank 2.

**Strengthening 2 (STRUCTURAL FORCING):** Both selections (m=2 and
k=4) have independent structural justifications (CM uniqueness +
smallest norm; O_h trivial-irrep multiplicity). The combination is
not "one happy accident" but a meeting of two independently-forced
selections.

**Strengthening 3 (NO NEAR-MISSES):** No other (m, k) pair in the
scanned family is "almost there" — the closest non-(2,4) match is
36% off, not 5%, not even 50%. There's no "we could just as easily
have picked (m=4, k=3) instead" alternative.

---

## 5 · What this does NOT establish

- **NOT a derivation of α.** The 1.26 ppm empirical match still requires
  the conjecture x_+ = 1/α. The structural rigidity of (m=2, k=4) within
  the tower family does not promote this to theorem.
- **NOT a proof of uniqueness across all polynomial families.** The
  scan is over `m^k · G*^(k-2)` and `m^k · G*^(k-1)` coefficients with
  Gaussian-integer m. Other polynomial families (e.g., transcendental
  multipliers, higher-degree polynomials, polynomials with non-G*
  coefficients) might contain matches we haven't checked.
- **NOT a falsification of look-elsewhere concerns.** FTD-0097's
  monomial-level scan found the catalog over-rich. This document's
  scan is at the polynomial level (specifically: (1+i)-tower-style
  polynomials), which is a more restrictive search space. A FULL
  look-elsewhere analysis would scan all polynomials of comparable
  complexity, not just the tower family.
- **NOT new mathematics** beyond what FTD-0111 already established.
  The harmonic invariant is generic for the tower normalization; this
  document is checking which specific (m, k) gives the empirical match.

---

## 6 · Status update for FTD-0111

**Before this commit:** FTD-0111 [THEOREM] for the harmonic invariant;
"why level k = 4" listed as [OPEN] follow-up Q1 from the original
filing.

**After this commit:** Q1 has SUBSTANTIALLY PROGRESSED. The level-4
selection is now:
- Structurally justified via N_base = mult(A_{1g}) of O_h
- Empirically uniquely close to 1/α among scanned (m, k) pairs
- Both arguments independent and converging

**FTD-0111's main theorem (the harmonic invariant) is unchanged** —
this document doesn't extend the THEOREM-grade content. But the
DERIVED status of the level-k=4 selection is now better grounded.

The empirical identification x_+ = 1/α (FTD-0013) remains [STRONGLY
MOTIVATED CONJECTURE]. This document strengthens its standing
modestly: x_+ at (m=2, k=4) is rank-1 among 58 structurally-natural
candidates, with no near-misses.

---

## 7 · LEDGER status

This document does NOT introduce a new LEDGER entry. It updates the
status of FTD-0111's Q1 follow-up from [OPEN] to [SUBSTANTIALLY
PROGRESSED — RANK-1 STRUCTURAL UNIQUENESS DEMONSTRATED].

The empirical identification FTD-0013 (x_+ = 1/α) is unchanged in
status (still [STRONGLY MOTIVATED CONJECTURE]) but now has additional
supporting evidence: the (m=2, k=4) selection is rank-1 unique among
the natural Gaussian-integer-tower family.

---

## 8 · What this ENABLES going forward

- **Paper A** (Letters in Mathematical Physics, ~10pp): the
  uniqueness scan strengthens the structural narrative around the
  master quadratic. The Paper A draft can cite this rank-1 result as
  evidence that (m=2, k=4) is structurally forced, not arbitrary.
- **Look-elsewhere refinement** (FTD-0097 follow-up): the scan
  methodology here can be extended to broader polynomial families,
  giving a polynomial-level look-elsewhere analysis to complement
  FTD-0097's monomial-level scan.
- **Spine §14 single-line summary** could be tightened to mention the
  structural uniqueness of (m=2, k=4).

---

## 9 · Single-line summary

**Among 58 (m, k) pairs in the natural (1+i)-tower polynomial family
with Gaussian-integer multipliers and small levels, (m=2, k=4) is
RANK 1 in closeness to physical 1/α with a 5-orders-of-magnitude gap
to rank 2; combined with independent structural justifications for
m=2 (Z[i] CM ring + smallest norm) and k=4 (O_h trivial-irrep
multiplicity), the (1+i)-tower master-quadratic level is structurally
forced — substantially tightening FTD-0111's empirical-selection
component without elevating x_+ = 1/α from [STRONGLY MOTIVATED
CONJECTURE] to [DERIVED].**

Verification: `scripts/proofs/proof_tower_multiplier_uniqueness.py`.

---

*End of exploration.*
