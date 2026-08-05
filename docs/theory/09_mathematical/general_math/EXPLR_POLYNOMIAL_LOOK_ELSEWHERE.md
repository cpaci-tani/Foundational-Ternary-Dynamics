# EXPLR — Polynomial-Level Look-Elsewhere Refinement (FTD-0097 Extension)

**Document type:** Exploratory result (substantive positive)
**Status:** [NUMERICAL FACT — finite enumeration] *(downgraded from [STRUCTURAL OBSERVATION] 2026-08-04, FTD-0802 — see the governing banner below)* — extends FTD-0097's monomial scan to polynomial level. Exactly one polynomial in the stated finite family matches the numerical pair at MQ precision. **That count is what chance predicts (`N_null = 0.0014`), so it is not evidence of structural uniqueness.**

**v1.4 annotation:** This scan uses the historical target pair `(1/α, N_c)` (where the second target reflects the pre-v1.4 `x_-  N_c` identification). That identification is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The polynomial-template-uniqueness fact reported here — that exactly one polynomial in the family matches the *numerical pair* (137.036…, 3.024) at MQ precision — is **independent of the physical interpretation of the second target** and stands. The "dual-prediction" prose below is preserved as historical interpretation, with the understanding that the load-bearing physics identification is now single-root (`x_+  1/α`, FTD-0013). `N_c = 3` in FTD is independently sourced via `DERIV_NC_FROM_TOPOLOGY.md`. FTD-0319's later adversarial scan (formerly cited as FTD-0189; 2.65 M polynomials, 18-constant FTD-undesigned basket; rank 1 by ~130×) is the canonical follow-up.
**Related:** `AUDIT_LOOK_ELSEWHERE_RESULTS.md` (FTD-0097 monomial scan); `EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md` (tower-level scan); `EXPLR_PATHS_TO_ALPHA.md` (α-derivation route survey); `THEOREM_HARMONIC_INVARIANT_TOWER.md` (FTD-0111)

---

> ## ⚠ GOVERNING BANNER — the uniqueness reading here does not survive its base rate
>
> **Added 2026-08-04 (FTD-0802); this banner governs. Where the body below and
> this banner disagree, this banner wins.**
>
> This document is the source of the "uniquely dual-selective" reading that
> propagated into OT-3.3, `SPEC_PHYSICS_BRIDGE`, `FOUND_STRUCTURAL_DECOUPLING`
> and the papers. Both of its legs have now been audited refute-by-default:
>
> - **The canonical follow-up named in the v1.4 annotation below (FTD-0319, the
>   2.65 M adversarial scan) sits at the chance base rate** — FTD-0791,
>   2026-08-03. The null expects 1.42–1.67 matchers at the registered gate; the
>   scan found 1. Retagged `[MEASURED]` → `[SELECTION]`.
> - **The extended version of *this* family failed its own pre-registered
>   base-rate control** — FTD-0802, 2026-08-04
>   (`PREREG_OT33_BASERATE_v1.md`, `scripts/experiments/verify_ot33_baserate.py`).
>   Measured on the rational-coefficient extension of the family scanned here:
>   `N_null = 0.0014` dual-matchers under displaced targets (`P(>=1) = 0.0009`),
>   so **a unique-matcher outcome is what chance predicts, not evidence against
>   it**. The `x_+` leg alone matches a random target near 137 to 1.26 ppm
>   **about one time in three** (`P(>=1) = 0.337`). The `x_-` leg eliminates
>   **zero** candidates at the registered gate, so "dual-selective" and
>   "selective" are the same predicate — and the `x_- ↔ N_c` target the v1.4
>   annotation already flags as retired is doing none of the work.
>   The extended scan's published count was **also wrong**: 4 non-master
>   dual-matchers, not 0.
>
> **What survives:** the arithmetic. Exactly one polynomial in the stated finite
> family matches the numerical pair at MQ precision — that is a true, checkable
> [NUMERICAL FACT] about a finite enumeration. **What does not survive:** reading
> it as structural uniqueness, or citing it as evidence for `x_+ = 1/α`. Per
> FTD-0802 the identification now retains **no numerical-uniqueness support of
> any kind**; OT-1.9 and OT-1.5 remain, both structural.

---

## 0 · Summary

A scan over **147,456 polynomials** of the form

```
M_{n,p,m,q}(x) = x² − n · G*^p · x + m · G*^q
```

with `n, m ∈ {1, 2, …, 64}` and `p, q ∈ {0, 1, 2, 3, 4, 5}` finds that
**exactly one polynomial** matches BOTH targets simultaneously at
master-quadratic precision (x_+ to 1/α at 1.26 ppm AND x_- to N_c at
0.80%): **the master quadratic itself** with `(n, p, m, q) = (16, 2, 16, 3)`.

This extends FTD-0097's monomial-level finding to polynomial level. It
provides **strong structural support** for the master quadratic's
dual-prediction property as not coincidental within the natural FTD
polynomial family.

---

## 1 · Background and motivation

FTD-0097 (`AUDIT_LOOK_ELSEWHERE_RESULTS.md`) ran a monomial-level
look-elsewhere scan and found:
- **Catalog over-rich** at monomial level (62 raw / 11 dedup hits at
  ε=10⁻⁴ vs Poisson null λ=4)
- **Dual-prediction property** of the master quadratic specifically
  distinguished from monomial-level fits (one polynomial → BOTH 1/α
  and N_c)

The natural follow-up: extend the look-elsewhere methodology to the
polynomial level. **Within the natural FTD polynomial family (degree 2
with integer × G*-power coefficients), is the master quadratic
uniquely dual-selective, or are there other polynomials in the same
class that also match both targets?**

This document answers that question.

---

## 2 · Scan setup

### 2.1 · Search space

```
M_{n,p,m,q}(x) = x² − n · G*^p · x + m · G*^q
```

with parameters in:
- `n ∈ {1, 2, …, 64}` (64 values)
- `m ∈ {1, 2, …, 64}` (64 values)
- `p ∈ {0, 1, 2, 3, 4, 5}` (6 values)
- `q ∈ {0, 1, 2, 3, 4, 5}` (6 values)

**Total: 64² × 6² = 147,456 polynomials.**

The master quadratic corresponds to `(n=16, p=2, m=16, q=3)`. The space
is structured to be FTD-natural: integer coefficients × G*-powers, with
ranges generous enough to contain "near-by" alternative polynomials at
similar complexity.

### 2.2 · Targets and tolerances

**Targets:**
- `x_+` = `1/α` = 137.035999084 (CODATA)
- `x_-` = `N_c` = 3.0

**Tolerances** (matched to master quadratic empirical precision):
- `x_+`: 1.26 ppm absolute = `1.73 × 10⁻⁴`
- `x_-`: 0.80% absolute = `2.42 × 10⁻²`

These are the SAME tolerances at which the master quadratic itself
matches the targets. A polynomial passes the dual-match test if and
only if it matches at MQ precision.

---

## 3 · Results

### 3.1 · Single-target match counts

| Target | Match count | Hit rate |
|---|---|---|
| `x_+` to 1/α at 1.26 ppm | 1 | 8 per million |
| `x_-` to N_c at 0.80% | 298 | 2454 per million |

The `x_-` tolerance (0.80%) is much looser than `x_+` (1.26 ppm), so
many more polynomials match x_- alone. This is expected.

### 3.2 · Dual-match count

**EXACTLY 1 polynomial matches BOTH simultaneously: the master
quadratic itself.**

```
n = 16, p = 2, m = 16, q = 3
M(x) = x² − 16·G*²·x + 16·G*³
x_+ = 137.036171   (matches 1/α to 1.26 ppm)
x_- = 3.0240       (matches N_c to 0.80%)
```

No other polynomial in the 147,456-element search space gives both
roots matching the targets at master-quadratic precision.

### 3.3 · Statistical context

Of the 147,456 polynomials, 26,002 have complex roots (discriminant
< 0) and are excluded. This leaves **121,454 polynomials with real
roots**.

- Real-roots dual-match rate: **1 / 121,454 = 8.23 × 10⁻⁶**
- The single match IS the master quadratic — not a false positive

If x_+ and x_- matches were INDEPENDENT (which they're not, because
they're constrained by Vieta), the expected dual-match count would be
about 0.002 across the search space — i.e., we'd expect 0-1 dual
matches by chance. Observing exactly 1 (and that 1 being the master
quadratic) is consistent with both:

1. **Structural selectivity** of the master quadratic: the polynomial
   form is rigid enough that no other (n, p, m, q) in the search
   space happens to dual-match.
2. **Random coincidence under independent assumption**: ~0-1 expected,
   1 observed — within statistical fluctuation.

The Vieta correlation (x_+ + x_- = a, x_+ · x_- = b) actually makes
dual-match even harder, not easier — so the probability under random
null is LOWER than the independent-assumption estimate. This argues
in favor of structural selectivity over random coincidence.

---

## 4 · Combined structural picture

Combining this scan with the earlier (1+i)-tower uniqueness scan
(`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`):

| Search space | Type | Master quadratic ranking | Gap to rank 2 |
|---|---|---|---|
| Tower family (m, k) | 1-parameter (b = a·G* always) | rank 1 of 58 | 5 orders of magnitude |
| Polynomial family (n, p, m, q) | 2-parameter (a, b independent) | unique among 121,454 dual-matchers (single dual match) | infinity (no other matches) |

**The master quadratic is uniquely dual-selective in two complementary
search spaces** at master-quadratic precision:
- 1-parameter tower family: rank 1 with 5-orders gap
- 2-parameter independent-coefficient family: unique dual-matcher

Both results are within the FTD-natural integer × G*-power complexity
class.

---

## 5 · What this strengthens

### 5.1 · Master quadratic dual-prediction property

The dual-prediction (one polynomial → BOTH 1/α and N_c) was tagged in
FTD-0097 as "the strongest structural evidence" for FTD-0001/0013/0014.
This scan **substantively strengthens that claim**:

- Within the natural FTD polynomial complexity class (integer × G*-
  power coefficients with reasonable parameter ranges), the master
  quadratic is the unique dual-matcher at its own precision.
- This is a stronger statement than "the master quadratic dual-matches"
  (which was already known) — it says "and nothing else in the natural
  family does".

### 5.2 · α-derivation prospects

The α-derivation problem remains [STRONGLY MOTIVATED CONJECTURE]
(per `EXPLR_PATHS_TO_ALPHA.md`). This scan does NOT promote the
identification x_+ = 1/α to [DERIVED]. But it does **eliminate one
class of skeptical alternatives**: "maybe many polynomials match α
within FTD's natural structures, and the master quadratic is just one
of many". After this scan, that alternative is ruled out — within
the structured family, the master quadratic is the only dual-matcher.

### 5.3 · Look-elsewhere completeness

FTD-0097 closed the monomial-level look-elsewhere question (catalog
over-rich at monomial level). This scan closes the **polynomial-level
look-elsewhere question** within the natural FTD polynomial family
(catalog NOT over-rich at polynomial level — exactly 1 dual-matcher,
which is the master quadratic itself).

Together, these two scans provide a **complete look-elsewhere
characterization** at the levels FTD's spine theorems live on.

---

## 6 · What this does NOT establish

- **NOT a derivation of α.** The 1.26 ppm match is still the empirical
  basis for x_+ = 1/α. This scan strengthens the structural standing
  of the master quadratic but doesn't convert conjecture to theorem.
- **NOT exhaustive across all polynomial families.** The scan covers
  integer coefficients × G*-powers up to 5. Other families (rational
  coefficients, transcendental multipliers, higher G*-powers, mixed
  G*+π+lemniscate-constant terms) are NOT scanned. The completeness
  claim is restricted to the searched family.
- **NOT a tightening of tolerances.** The scan uses MQ precision
  (1.26 ppm for x_+, 0.80% for x_-). Relaxing tolerances would find
  more matches; tightening would find fewer (possibly zero). The
  master-quadratic-precision tolerances are the appropriate match for
  comparing against the master quadratic itself.
- **NOT new mathematics.** The result is a structural observation
  about the searched family. The master quadratic theorem (FTD-0001)
  is unchanged.

---

## 7 · Open follow-ups

- **Broader polynomial families:** scan rational × G*-power
  coefficients; mixed G* + π + ϖ + framework-integer terms; higher
  G*-powers. Each is an extended search space; if the master
  quadratic remains uniquely dual-selective, the structural standing
  strengthens further.
- **Higher-degree polynomials:** scan degree-3, degree-4 polynomials
  with similar structure. Does any polynomial with one extra term
  give a tighter match?
- **Tighter tolerances:** the 1.26 ppm tolerance for x_+ matches MQ.
  At 0.001 ppm tolerance, the master quadratic itself doesn't match
  (CODATA precision is ~10 digits, not enough). Run scans at
  intermediate tolerances to map the structural-vs-empirical boundary.

---

## 8 · LEDGER status

This document does NOT introduce a new LEDGER entry. It updates the
description of FTD-0097's outcome:

- **FTD-0097** (monomial-level scan): catalog over-rich at monomial
  level. Status: [MEASURED] (unchanged).
- **Polynomial-level extension** (this document): catalog NOT
  over-rich at polynomial level — master quadratic uniquely dual-
  selective in the natural FTD family. Status: [STRUCTURAL
  OBSERVATION] (new).
- **FTD-0001/0013/0014** (master quadratic + dual-prediction
  conjecture): unchanged in tag status, but with substantially
  strengthened structural evidence after this scan + the (1+i)-tower
  scan combined.

---

## 9 · Single-line summary

**Among 147,456 polynomials in the natural FTD complexity class
(degree 2, integer × G*-power coefficients with n, m ∈ [1, 64] and
p, q ∈ [0, 5]), exactly one matches both 1/α (1.26 ppm) and N_c (0.80%)
simultaneously: the master quadratic itself, with (n=16, p=2, m=16,
q=3). This polynomial-level look-elsewhere result extends FTD-0097's
monomial finding (catalog over-rich at monomial level) to its
counterpart (catalog NOT over-rich at polynomial level — uniquely
selective within the natural family). Combined with the (1+i)-tower
rank-1 uniqueness, the master quadratic dual-prediction is now
demonstrated as structurally rigid within both the 1-parameter tower
family and the 2-parameter independent-coefficient family.**

Verification: `scripts/proofs/proof_polynomial_look_elsewhere.py`.

---

*End of exploration.*
