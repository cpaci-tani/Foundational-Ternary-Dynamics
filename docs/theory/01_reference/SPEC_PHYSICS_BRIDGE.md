# SPEC — The Physics Bridge: What FTD's Algebraic Spine Says About α and N_c

**Document type:** Reference specification (synthesis)
**Status:** [SYNTHESIS] — crystallizes the bridge between FTD's mathematical spine and the Standard Model constants (1/α, N_c) as it currently stands
**Created:** 2026-05-01 evening (closing session on tractable open gaps)
**Provenance:** Closes the "physics bridge" by making its logical structure crisp; consolidates `EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md` + `EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md` + `EXPLR_PATHS_TO_ALPHA.md` into a single reference position
**Related:** `SPEC_ALGEBRAIC_SPINE.md` (the 9 theorems); LEDGER FTD-0001/0013/0014 (the conjecture); FTD-0097 (look-elsewhere monomial); FTD-0117 (G\* notational fix)

---

## 0 · Position summary

FTD's mathematical spine produces a specific algebraic object — the master
quadratic with coefficients in `Z[16]·Q(G*)` where `G* = Γ(1/4)/Γ(3/4) ≈
2.9587` (NOT the Bernoulli/Gauss lemniscate constant ϖ ≈ 2.622). The
polynomial's two roots are:

```
x_+ ≈ 137.036171
x_- ≈   3.023964
```

These are forced by the spine theorems (no free parameters). They match
two unrelated Standard Model constants — the QED fine-structure constant
reciprocal `1/α` and the QCD color number `N_c = 3` — to **1.26 ppm and
0.80%** respectively.

The mathematical content (theorems + uniqueness) is established. The
empirical match (the IDENTIFICATION) is conjectural. The bridge between
them is the structural rigidity of the math + the precision of the
empirical match + the uniqueness of the master quadratic in two natural
search spaces.

This document crystallizes what's established, what's not, and what
the bridge LOGICALLY SAYS.

---

## 1 · The mathematical content (theorems, no physics)

### 1.1 · Definitions

**G\* := Γ(1/4) / Γ(3/4) ≈ 2.95867512.** The FTD master constant. Distinct
from the Bernoulli/Gauss lemniscate constant ϖ ≈ 2.622 (per FTD-0117 fix).
Equivalent forms via Γ-reflection:

```
G* = Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π·√2) = √2·Γ(1/4)²/(2π)
```

### 1.2 · The master quadratic (Theorem 2 / FTD-0001)

The polynomial

```
M(x) = x² − 16·G*²·x + 16·G*³ = 0
```

has real roots:

```
x_± = 8·G*² ± √(64·G*⁴ − 16·G*³)
x_+ = 137.036171...
x_- =   3.023964...
```

The polynomial form is forced by:
- Theorem 1: `G*` is the specific Γ-function ratio (algebraic identity).
- Theorem 3: CM uniqueness selects `d = −4` from the 9 class-number-1
  imaginary quadratic discriminants.
- Theorem 4: `|Aut(E)|² = 16` for E: y² = x³ − x is the smallest non-trivial
  automorphism count.

### 1.3 · The harmonic-conjugate identity (Theorem 8 / FTD-0111)

Setting `y = x/G*`, the master quadratic gives:

```
1/y_+ + 1/y_- = 1
```

Equivalently in physics notation (assuming x_+ = 1/α, x_- = N_c):

```
α + 1/N_c = 1/G*       (slogan form, Kirchhoff-style harmonic conjugacy)
```

Same conjecture, different framing.

### 1.4 · Closed-form for x_+ (Theorem 8 corollary)

```
x_+ = 1/(2G*) − √(4G* − 1) / (4·G*^(3/2))    (= the closed form for "1/α" if conjecture holds)
```

### 1.5 · Q(G\*) field-theoretic structure (Theorem 9 / FTD-0112)

`Q(G*)` is a maximal π-free subfield of `Q(π, Γ(1/4))`, conditional on
Chudnovsky 1976 algebraic independence of π and Γ(1/4). The master
quadratic's coefficients live in `Z[16]·Q(G*)` — i.e., in the π-free
field extended by the integer 16.

### 1.6 · Polynomial uniqueness (this session's results)

**Tower-scan uniqueness** (`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`):
among 58 (m, k) pairs in the natural Gaussian-integer-tower family

```
M_{m,k}(x) = x² − m^k · G*^(k−2) · x + m^k · G*^(k−1)
```

with m ∈ {1, 2, 4, 5, 8, 9, 10, 13, 16, 17, 18, 20} (smallest distinct
norms) and k ∈ {3, 4, 5, 6, 7}: **(m=2, k=4) is rank 1 in closeness to
1/α with a 5-orders-of-magnitude gap to rank 2**.

**Polynomial-scan uniqueness** (`EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md`):
among 147,456 polynomials of the form

```
M_{n,p,m,q}(x) = x² − n · G*^p · x + m · G*^q
```

with n, m ∈ [1, 64] and p, q ∈ [0, 5]: **exactly one matches BOTH x_+
and x_- targets at MQ precision — the master quadratic itself**.

These uniqueness results are theorems within their explicit search
spaces. Their interpretation is structural significance — see §3.

---

## 2 · The empirical content (observations)

### 2.1 · The dual prediction

```
CODATA 2022:           1/α = 137.035999084(21)        (~10-digit precision)
PDG (Standard Model):  N_c = 3                          (exact integer)

FTD master quadratic:  x_+ = 137.036171...              (algebraic)
                        x_- =   3.023964...              (algebraic)

|x_+ − 1/α| / (1/α)  = 1.26 × 10⁻⁶  (1.26 ppm)
|x_- − N_c| / N_c    = 8.0 × 10⁻³   (0.80%)
```

### 2.2 · Status of the empirical match

- **x_+ ≈ 1/α**: tagged FTD-0013 [STRONGLY MOTIVATED CONJECTURE]
- **x_- ≈ N_c**: tagged FTD-0014 [STRONGLY MOTIVATED CONJECTURE]
- **Both simultaneously** (the dual prediction): the central conjecture

The 0.80% deviation in x_- (= 3.024 vs N_c = 3) is NOT a measurement
error — it's structural. The polynomial gives 3.024 exactly; the
empirical N_c is exactly 3. The 0.80% IS the conjecture's residual.

---

## 3 · The bridge — what the math + observation TOGETHER say

### 3.1 · Bayesian strength estimate

Combining the two uniqueness scans (this session, commits `0074f92` +
`f36b741`):

| Search space | Bayes factor (structural / coincidence) |
|---|---|
| Tower scan (1-parameter, 58 candidates) | ~50× |
| Polynomial scan (2-parameter, 147,456 candidates) | ~409× |
| Combined (treating as independent search axes) | **~20,000×** |

In log scale: **~4.3 decimal orders** of evidence for the structural
reading over a random-coincidence null, **WITHIN THE NATURAL FTD
POLYNOMIAL FAMILY**.

This is reasonably strong but not overwhelming. To exceed 10⁶× evidence
(decisive), broader polynomial families would need to be scanned.

### 3.2 · What the bridge LOGICALLY says

**The disciplined statement:**

> *Within FTD's natural polynomial complexity class (degree-2 polynomials
> with coefficients in Z[1..64]·Q(G*)^[0..5]), the master quadratic is
> the UNIQUE polynomial whose roots dual-match (1/α, N_c) at master-
> quadratic precision. The polynomial form itself is forced by the spine
> theorems (no free parameters). The empirical match is at 1.26 ppm and
> 0.80%. The Bayesian strength of evidence for the structural reading
> over random coincidence is ~20,000:1 within this family. The
> identification x_+ = 1/α and x_- = N_c remains formally [STRONGLY
> MOTIVATED CONJECTURE] because no derivation chain from FTD axioms to
> physical constants has been established despite multiple session-scale
> attempts.*

### 3.3 · What the bridge does NOT say

- Does NOT say α is derived from FTD axioms.
- Does NOT say N_c = 3 exactly is derived.
- Does NOT say the dual match is impossible to be coincidental.
- Does NOT say FTD's framework is empirically falsified or
  empirically validated as the unique correct theory.

### 3.4 · What would CLOSE the bridge

The bridge would close formally if any of the following were
established:

1. **Structural derivation of α.** A chain from FTD axioms to physical
   α. Multiple routes attempted and closed-negative:
   - EFT recovery R1, R2, R3 (`SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`)
   - Z-factor reading (FTD-0116)
   - RG running (`EXPLR_PATHS_TO_ALPHA.md` §7.5)
   - Algebraic combinations (`EXPLR_PATHS_TO_ALPHA.md` §2)

2. **Decisive look-elsewhere argument.** Bayes factor exceeding 10⁶:1
   in favor of structural reading. Current ~20,000:1 is informative
   but not decisive. Broader polynomial families and tighter
   tolerances would push this further.

3. **Independent FTD route.** A different derivation of α from FTD
   that converges on x_+. Engine measurements (Rutherford α ≈ 0.042)
   are ~6× off from physical α; not converging.

None of these is currently in hand. The bridge stays as crystallized
in §3.2.

---

## 4 · What FTD CAN claim externally

### 4.1 · For Paper A (Letters in Mathematical Physics)

The paper can present:

- **Theorem 1**: G* = Γ(1/4)/Γ(3/4) algebraic identity.
- **Theorem 2**: master quadratic with closed-form roots.
- **Theorem 8**: harmonic-invariant tower with anomaly transcendence.
- **Theorem 9**: Q(G\*) field-theoretic characterization (conditional
  on Chudnovsky 1976).
- **Uniqueness scans**: the master quadratic is uniquely dual-selective
  in two natural search spaces at master-quadratic precision.

The paper does NOT need to claim α is derived. It can present the
mathematics + the observed empirical regularity (with explicit
[CONJECTURE] tag) + the Bayesian structural-uniqueness argument.

This is enough for a Letters-in-Mathematical-Physics-grade contribution
without overclaiming.

### 4.2 · For external falsification

A reviewer or future researcher could falsify the conjecture by:
- Finding an alternative polynomial in a broader natural family that
  also dual-matches at MQ precision (would weaken structural-uniqueness)
- Finding a derivation of α from QED or another framework that is
  inconsistent with x_+ (would refute the IDENTIFICATION at theory
  level)
- Improved measurement of α revealing a deviation outside 1.26 ppm
  band (would refute the empirical match)

Until one of these occurs, the conjecture stands at its current
strength.

---

## 5 · Honest meta-statement

The "physics bridge" of FTD is:
1. **A mathematical spine** (9 theorems) producing a specific algebraic
   object whose roots are computable.
2. **An empirical observation** that these roots match two unrelated
   Standard Model constants at high precision.
3. **A structural-uniqueness argument** showing the master quadratic
   is the unique dual-matcher in two natural search spaces.
4. **A conjectural identification** linking 1 and 2, supported by 3
   but not formally derivable from current axioms.

The bridge is **finished as much as current methods allow**. Further
closure requires either a new theoretical mechanism (research program,
not session task) or substantially broader statistical analysis (open
follow-up).

The framework's standing is:
- **Mathematical core**: established at theorem grade, internally
  consistent, structurally rigid.
- **Empirical match**: precise (1.26 ppm + 0.80%) and uniquely
  selective within the natural family.
- **Physical bridge**: open at the IDENTIFICATION level; closed at
  the structural-rigidity level.

This is an honest, defensible, and externally-publishable position.

---

## 6 · LEDGER status

This document does NOT introduce a new LEDGER entry. It crystallizes
the position synthesized from existing entries:

- FTD-0001 (master quadratic): [THEOREM] — unchanged
- FTD-0013 (x_+ = 1/α identification): [STRONGLY MOTIVATED CONJECTURE] — unchanged tag, **strengthened** by 2026-05-01 uniqueness results
- FTD-0014 (x_- = N_c identification): [STRONGLY MOTIVATED CONJECTURE] — same
- FTD-0097 (look-elsewhere monomial): [MEASURED] — extended to polynomial level via this session
- FTD-0111 (harmonic invariant tower): [THEOREM] with Q1 progressed
- FTD-0117 (G\* typo fix): [BUG RESOLVED]

---

## 7 · What this document does NOT claim

- NOT a theorem of α-derivation.
- NOT a theorem of N_c-derivation.
- NOT a falsification of any prior closed-negative route.
- NOT a new spine theorem (count remains 9).
- NOT a promotion of FTD-0013/0014 from [STRONGLY MOTIVATED CONJECTURE].

---

## 8 · Single-line summary

**FTD's physics bridge consists of a mathematically rigid spine (9
theorems producing the master quadratic with computable roots ≈ 137.036
and ≈ 3.024), an empirical observation (these roots match 1/α at 1.26
ppm and N_c at 0.80%), a structural-uniqueness argument (the master
quadratic is the unique dual-matcher in two natural search spaces with
combined ~20,000:1 Bayes factor for structural over coincidence
reading), and a conjectural identification linking the math to physics
that remains [STRONGLY MOTIVATED CONJECTURE] absent a derivation chain
from FTD axioms to physical constants. This is the bridge as currently
finished — strong enough for external publication of the mathematical
core (Paper A scope), not yet closed at the formal-derivation level
(Paper A would frame it as conjecture, not theorem).**

---

*End of synthesis.*
