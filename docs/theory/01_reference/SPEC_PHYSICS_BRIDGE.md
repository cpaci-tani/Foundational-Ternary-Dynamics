# SPEC — The Physics Bridge: What FTD's Algebraic Spine Says About α

**Document type:** Reference specification (synthesis)
**Status:** [SYNTHESIS] — crystallizes the bridge between FTD's mathematical spine and the Standard Model constant 1/α
**Re-tag note:** The single load-bearing physics identification is `x_+  1/α` (FTD-0013, [STRONGLY MOTIVATED CONJECTURE]).
**Related:** `SPEC_ALGEBRAIC_SPINE.md` (the algebraic spine — nine numbered results: seven theorem-grade + two honestly-tiered, Theorem 3 at its arithmetic core only; see §0 count convention); `SPEC_FQCR.md` (operator-theoretic restatement); `SPEC_ALPHA_READOUT_CONTRACT.md` (MC-T4.3 closure contract); `TRACKER_ONTIC_TRUTH.md` (tier assignments); `SPEC_OPEN_MATH_BY_SECTOR.md` (MC-T4.3); LEDGER FTD-0001/0013 (the conjecture); FTD-0097 (look-elsewhere monomial); FTD-0117 (G\* notational fix)

---

> FTD-0792: the engine never ran on the derived root.

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

These are forced by the spine theorems (no free parameters). The larger
root `x_+` matches the QED fine-structure constant reciprocal `1/α` to
**1.26 ppm**.

The mathematical content (theorems + uniqueness) is established. The
empirical match (the IDENTIFICATION `x_+  1/α`) is conjectural. The
bridge between them is the structural rigidity of the math + the
precision of the empirical match.

This document crystallizes what's established, what's not, and what
the bridge LOGICALLY SAYS.

**Bridge-audit status.** The algebraic layer is strong, while the
physical mechanism problem is narrow. The missing
object is not "more algebraic evidence"; it is a non-action readout
rule that maps the algebraic root/eigenvalue to an operational
electromagnetic coupling without inserting α. This is MC-T4.3 in
`SPEC_OPEN_MATH_BY_SECTOR.md`.

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

This is a mathematical identity between the two roots `y_± = x_±/G*` of
the rescaled polynomial.

### 1.4 · Closed-form for x_+ (Theorem 8 corollary)

```
x_+ = 1/(2G*) − √(4G* − 1) / (4·G*^(3/2))    (= the closed form for "1/α" if conjecture holds)
```

### 1.5 · Q(G\*) field-theoretic structure (Theorem 9 / FTD-0112)

`Q(G*)` is a π-free subfield of `Q(π, Γ(1/4))` (conditional on Chudnovsky 1976),
conditional on Chudnovsky 1976 algebraic independence of π and Γ(1/4). (Maximality
is unproven and false — `Q(Γ(1/4))` is larger.) The master
quadratic's coefficients live in `Z[16]·Q(G*)` — i.e., in the π-free
field extended by the integer 16.

### 1.6 · Polynomial uniqueness

**Tower-scan uniqueness** (`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`):
among 58 (m, k) pairs in the natural Gaussian-integer-tower family

```
M_{m,k}(x) = x² − m^k · G*^(k−2) · x + m^k · G*^(k−1)
```

with m ∈ {1, 2, 4, 5, 8, 9, 10, 13, 16, 17, 18, 20} (smallest distinct
norms) and k ∈ {3, 4, 5, 6, 7}: **(m=2, k=4) is rank 1 in closeness to
1/α with a 5-orders-of-magnitude gap to rank 2**.

This uniqueness result is a theorem within its explicit search space.

---

## 2 · The empirical content (observations)

### 2.1 · The single physics identification

```
CODATA 2022:           1/α = 137.035999177(21)        (~10-digit precision)

FTD master quadratic:  x_+ = 137.036171...              (algebraic)
                        x_- =   3.023964...              (algebraic)

|x_+ − 1/α| / (1/α)  = 1.26 × 10⁻⁶  (1.26 ppm)
```

### 2.2 · Status of the empirical match

- **x_+ ≈ 1/α**: tagged FTD-0013 [STRONGLY MOTIVATED CONJECTURE]
  — the single load-bearing physics identification of the master
  quadratic.

---

## 3 · The bridge — what the math + observation TOGETHER say

### 3.1 · What the bridge LOGICALLY says

**The disciplined statement:**

> *The polynomial form is fixed by the algebraic spine once G*,
> coefficient 16, and degree 2 are accepted. The master quadratic's
> larger root agrees with 1/α to 1.26 ppm; this is arithmetic.
> `x_+ = 1/α` has no numerical-uniqueness support. The physics
> identification `x_+ = 1/α` remains formally [STRONGLY MOTIVATED
> CONJECTURE] because no derivation chain from FTD axioms to physical α
> has been established despite multiple attempts.*

### 3.2 · What the bridge does NOT say

- Does NOT say α is derived from FTD axioms.
- Does NOT say the empirical match `x_+  1/α` is impossible to be
  coincidental.
- Does NOT say FTD's framework is empirically falsified or
  empirically validated as the unique correct theory.

### 3.3 · What would CLOSE the bridge

The bridge would close formally if any of the following were
established:

1. **Structural derivation of α.** A chain from FTD axioms to physical
   α. Multiple routes attempted and closed-negative:
   - EFT recovery R1, R2, R3 (`SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`)
   - Z-factor reading (FTD-0116)
   - RG running (`EXPLR_PATHS_TO_ALPHA.md` §7.5)
   - Algebraic combinations (`EXPLR_PATHS_TO_ALPHA.md` §2)

2. **Decisive look-elsewhere argument.** A decisive structural-uniqueness
   argument over a broad polynomial family. This route would strengthen
   the conjecture, not convert it into a mechanism by itself.

3. **Independent FTD route.** A different derivation of α from FTD
   that converges on x_+. Engine measurements (Rutherford α ≈ 0.042)
   are ~6× off from physical α; not converging.

4. **Non-action readout rule (current best target).** Because the
   action/EFT routes are closed-negative, a closure proof would likely
   have to operate through a boundary condition, observable-selection
   rule, quantization/readout rule, or discrete-native measurement
   protocol. The rule must be stated without α as an input and must say
   why the dominant master-quadratic/FQCR eigenvalue is the physical
   electromagnetic coupling rather than merely a distinguished number.
   This is the precise MC-T4.3 target. The admissibility contract for
   such a rule is now `SPEC_ALPHA_READOUT_CONTRACT.md`: state the
   preparation, observable algebra, electromagnetic measurement
   functional, readout map, and calibration discipline before checking
   the physical target.

None of these is currently in hand. The bridge stays as crystallized
in §3.1.

---

## 4 · What FTD CAN claim externally

### 4.1 · For Paper A (Letters in Mathematical Physics)

The paper can present:

- **Theorem 1**: G* = Γ(1/4)/Γ(3/4) algebraic identity.
- **Theorem 2**: master quadratic with closed-form roots.
- **Theorem 8**: harmonic-invariant tower with anomaly transcendence.
- **Theorem 9**: Q(G\*) field-theoretic characterization (conditional
  on Chudnovsky 1976).

The paper does NOT need to claim α is derived. It can present the
mathematics + the observed empirical regularity (with explicit
[CONJECTURE] tag).

### 4.2 · For external falsification

A reviewer or future researcher could falsify the conjecture by:
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
1. **A mathematical spine** (nine numbered results: seven theorem-grade + two honestly-tiered — Theorem 3 at its arithmetic core only; see `SPEC_ALGEBRAIC_SPINE.md` §0 count convention) producing a specific algebraic
   object whose roots are computable.
2. **An empirical observation** that the larger root matches 1/α at
   high precision.
3. **A conjectural identification** linking 1 and 2, not formally
   derivable from current axioms.

The bridge is **finished at the algebraic layer**.
Further closure is no longer a documentation problem. It requires a new
theoretical mechanism, most plausibly MC-T4.3's non-action readout rule,
or a discrete-native measurement program that bypasses imported
continuous-QFT machinery while still comparing to measured physics.

The framework's standing is:
- **Mathematical core**: established at theorem grade, internally
  consistent, structurally rigid.
- **Empirical match**: precise (1.26 ppm).
- **Physical bridge**: open at the IDENTIFICATION level; closed at
  the structural-rigidity level.

This is an honest, defensible, and externally-publishable position.

---

## 6 · LEDGER status

This document does NOT introduce a new LEDGER entry. It crystallizes
the position synthesized from existing entries:

- FTD-0001 (master quadratic): [THEOREM] — unchanged
- FTD-0013 (x_+ = 1/α identification): [STRONGLY MOTIVATED CONJECTURE], unchanged tag
- FTD-0097 (look-elsewhere monomial): [MEASURED]
- FTD-0111 (harmonic invariant tower): [THEOREM] with Q1 progressed
- FTD-0117 (G\* typo fix): [BUG RESOLVED]

---

## 7 · What this document does NOT claim

- NOT a theorem of α-derivation.
- NOT a falsification of any prior closed-negative route.
- NOT a new spine theorem (spine count unchanged — nine numbered results: seven theorem-grade + two honestly-tiered, Theorem 3 at its arithmetic core only; see `SPEC_ALGEBRAIC_SPINE.md` §0 count convention).
- NOT a promotion of FTD-0013 from [STRONGLY MOTIVATED CONJECTURE].

---

## 8 · Single-line summary

**FTD's physics bridge consists of a mathematically rigid spine (nine
numbered results — seven theorem-grade + two honestly-tiered, Theorem 3
at its arithmetic core only; see `SPEC_ALGEBRAIC_SPINE.md` §0 count
convention — producing the master quadratic with
computable roots ≈ 137.036 and ≈ 3.024) and the empirical observation
that the larger root matches 1/α at 1.26 ppm. The single
physics identification `x_+ = 1/α` remains [STRONGLY MOTIVATED
CONJECTURE] absent a derivation chain from FTD axioms to physical α.
This is the bridge as
currently finished at the algebraic layer — strong enough for external
publication of the mathematical core (Paper A scope), not yet closed
at the formal-derivation level. The next mathematical target is
MC-T4.3: a non-action readout/observable-selection rule that explains
why the distinguished root is the physical electromagnetic coupling.**

---

*End of synthesis.*
