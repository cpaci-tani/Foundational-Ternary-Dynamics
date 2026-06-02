# DERIV — Mechanism B: g_c from Lattice → Continuum Matching

**Tag:** [CLOSED NEGATIVE] — the matching procedure does not produce a first-principles value of `g_c`. The obstruction is not the no-go theorem (`g_c` is dimensionless), but the **circularity** of the matching reference: every continuum target available to the matching is itself either (i) outside Axiom-Zero `R`, or (ii) the master quadratic root `x_+`, which is what `g_c` was supposed to predict independently.
**Date:** 2026-04-25
**LEDGER row:** FTD-0031 (was OPEN; this document recommends → CLOSED NEGATIVE pending owner sign-off).
**Dependencies:** FTD-0001 (master quadratic), FTD-0013 (`x_+ ↔ 1/α`), FTD-0030/0041 (`a_phys ≡ ℓ_P` calibration), FTD-0050 (engine BCC-orthogonality, Link 8 closure), FTD-0059 (no-go theorem for dimensional Axiom-Zero outputs), Phase G (`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`), Phase H (Phase G §7), `OPEN_GC_FROM_FIRST_PRINCIPLES.md`.
**Supersedes:** the implicit "Mechanism B is partially started" status in `OPEN_GC_FROM_FIRST_PRINCIPLES.md` §2.2.

---

## 1 · Problem statement

The engine's Gauss law (Phase G, `poisson_solvers.cpp:123`) is

```
  ∇·J = g_c · s,        s ∈ {−1, 0, +1},
```

with `g_c` a dimensionless real number. The Phase H scaling theorem
established

```
  α_r(r, L; g_c)  =  g_c² · 2 r G_L(r),
```

verified to better than 10⁻⁴ relative precision against engine output.
For the small-r regime (`G_L(r) ≈ 1/(4π r)`) this gives

```
  α_r  →  g_c² / (2π)        (engine convention).
```

Setting `α_r = α_ref = 1/137.036` requires

```
  g_c  =  √(2π · α_ref)  ≈  0.21413                                 (★)
```

(or `√(4π · α_ref)` in the classical ½-convention; the convention is a
field redefinition and does not alter the question that follows).

The current implementation hardcodes `G_C ≡ √α_ref`, which is target
calibration, not derivation. **Mechanism B asks whether `g_c` can be
fixed by a lattice → continuum matching procedure of the form**

```
  1/g_R²(μ)  =  1/g_c²  +  b_0 log(μ a)  +  O(g_c²)                  (♦)
```

without using `α_ref` (or any quantity equivalent to it) as an external
input on the right-hand side.

This document is the post-mortem.

---

## 2 · What "matching" requires

A bare-to-renormalised matching of the form (♦) requires three
structural ingredients:

| Ingredient | What it must be | Where it lives in FTD |
|---|---|---|
| (i) A bare lattice action `S[J]` whose tree-level coupling is `g_c`. | Dimensionless action with a tunable coupling. | The Phase G derivation gives the kinematics: `∇·J = g_c · s` and `field_energy = Σ \|J\|²` (no ½). |
| (ii) A continuum theory at scale μ with renormalised coupling `g_R(μ)` to match against. | Standard Wilsonian: pick a continuum target. | **Open.** No continuum theory has been *derived* from FTD; one can be *posited* (continuum QED is the obvious candidate). |
| (iii) A matching coefficient (one-loop or higher) computable on both sides. | Loop integrals on lattice (with cutoff `1/a`) and continuum (with regulator μ). | Phase-G partition function (`DERIV_PARTITION_FUNCTION_L2.md`) is **ultralocal** in s under the Gauss constraint — no fluctuations to integrate. |

The audit below shows that each of (ii) and (iii) breaks in a way that
is not fixable without supplying `α_ref` (or its master-quadratic-root
proxy `x_+`) on the right-hand side.

---

## 3 · The Phase-G constraint: g_c does not enter the propagator

The Phase G theorem (`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`) is the
load-bearing structural fact for Mechanism B. Restated:

> The engine's static V(r) measurement is the periodic lattice Poisson
> Green's function `2 · r · G_L(r)`, with **zero fine-structure
> content**. The factor `g_c²` rescales the source amplitude; it does
> **not** enter the propagator's kernel, the dispersion relation, or
> the spectrum of `D(k) = 2(3 − cos k_x − cos k_y − cos k_z)`.

Consequence: in any matching procedure, `g_c` enters only as a
multiplicative source normalisation. The lattice "Feynman rules" for
the Phase-G/Phase-H theory are:

- **Propagator:** `1/D(k)` — independent of `g_c`. No coupling renormalisation comes from here.
- **Source vertex:** factor `g_c` — the only place coupling enters.
- **Self-energy / vacuum polarisation:** *zero at tree level and at one loop*, because the action is quadratic in `J` once `s` is fixed (Gauss constraint), and `J`-loops with no `J`-self-interaction give a Gaussian determinant that depends on `D(k)` only.

So **the "1-loop matching coefficient `b_0`" of equation (♦) is zero
on the FTD side of the match**. There is nothing for the lattice
β-function to pick up at one loop in the Phase-G/Phase-H reduced
theory: the only dimension-4 operator with non-trivial coefficient is
`|∂J|²`, and its coefficient is fixed kinematically (the wave equation
plus the energy accumulator's no-½ convention), not dynamically.

The matched continuum coupling is therefore

```
  g_R²(μ)  =  g_c²        (independent of μ)                       (3.1)
```

at one loop. Higher-loop diagrams require non-Gaussian interactions in
J, which the engine's reduced action does not contain (the "L_coupling
= −g_c · s · (∇·J)" Lagrangian ansatz is bilinear in J and linear in
s; under the Gauss constraint it collapses to ultralocal energy in s
alone — `DERIV_PARTITION_FUNCTION_L2.md`, also FTD-0050).

**This is structurally equivalent to the FTD-0050 closure**: the
engine's stencil does not generate a non-trivial RG flow on the
master-quadratic-relevant sector. Phase-G/Phase-H likewise does not
generate a non-trivial RG flow on the `g_c`-relevant sector. Both
are consequences of the same kinematic fact: the engine's bilinear
+ Gauss-constraint action has no relevant or marginal interactions to
run.

---

## 4 · The matching reference: where the circularity enters

If the lattice β-function is zero at every loop accessible to the
classical engine (§3), then matching becomes trivial: the bare
coupling equals the renormalised coupling at every scale. **The
matching condition (♦) reduces to a *boundary* condition, not a flow
equation:**

```
  g_c  =  g_R(μ_match)   for some chosen scale μ_match.            (4.1)
```

The first-principles question is therefore not "what does flow to
what" but: **what fixes `g_R(μ_match)`?**

Three candidate references:

### 4.1 · Reference A: an external physical observable

The standard QED matching uses the Thomson-limit observable `α(0) =
1/137.036…`. This is a measured number, not derivable from Axiom Zero.
By the no-go theorem (FTD-0059), Axiom-Zero invariants cannot produce
any dimensional quantity, but `α(0)` is dimensionless — so the no-go
theorem does *not* forbid it from being in `R`. However:

- Axiom Zero does not contain `α(0)` directly. It contains `{D=3, ϖ, integers, c_lat = 1/√3}`.
- Whatever Axiom-Zero-derivable dimensionless number is identified with `α(0)` is, by construction, an **algebraic invariant** of the lattice. The candidate is `1/x_+` from the master quadratic (FTD-0013). 

If one takes `g_R(μ_match) = √(2π / x_+)` (engine convention), then
`g_c = √(2π · 1/x_+)`. The numerical value `≈ 0.21413` is reproduced
exactly, but the derivation **uses `x_+`**, which is the very thing
the chain `{D=3, ϖ} → x_+ → α` was supposed to deliver
*independently* of `g_c`.

**The matching is therefore not independent of the master quadratic.**
Using `α_ref` as the matching target is equivalent to using `x_+`.

### 4.2 · Reference B: an internal lattice scale

Lattice gauge theory often matches at the lattice cutoff `μ = 1/a`,
where `g_c = g_R(1/a)` by construction. In FTD this gives

```
  g_c  =  g_R(1/a_phys)
```

But by FTD-0030/0041, `a_phys ≡ ℓ_P` is a calibration, not derived. The
matched coupling at the Planck scale is whatever value `g_R(1/ℓ_P)`
takes in continuum QED — which requires running α from the
electroweak scale up by ~50 e-folds, which requires the entire QED
β-function as input. This is matching to the Standard Model, not
deriving from FTD.

### 4.3 · Reference C: an Axiom-Zero invariant directly

Could `g_c²` itself be an element of `R`? Candidates:

| Candidate | Numerical | Match to (★) `g_c² ≈ 0.04585` | Status |
|---|---:|---:|---|
| `1/(2π · x_+)` | 0.04585… | exact (by construction) | uses `x_+` — circular per §4.1 |
| `1/(2π · 16 G*²)` (via Vieta `x_+ + x_− = 16 G*²`) | 0.000537 | wrong by factor ~85 | rejected |
| `1/(16 G*²)` | 0.003373 | wrong by factor ~13.6 | rejected |
| `(G* − 1)/(16 G*²)` | 0.002237 | wrong | rejected |
| `1 / (b_3 + N_c)²` (the engine's `G_N`) | 0.01 | wrong by factor ~4.6 | rejected |
| `α_ref` directly | 0.007297 | factor 2π low | rejected (also: `α_ref ∉ R` by hypothesis) |
| `2π / x_+` | 0.04585… | exact | uses `x_+` — circular |

Every Axiom-Zero combination that produces the right numerical value
factors through `x_+`. The two structural reasons:

(a) The Phase-G geometric Coulomb already supplies the `1/(2π)` factor
("zero-parameter lattice Coulomb at small r"). Whatever `g_c²`
multiplies it must therefore equal `2π α_ref` exactly to recover QED.

(b) The chain `{D=3, ϖ} → x_+` is the only Axiom-Zero route to a
number near `1/α`. Any other algebraic combination of `{G*, ϖ, π,
integers}` that hits 137 to ppm precision is, by the CM-curve
uniqueness theorem (FTD-0014), the master quadratic root in disguise.

---

## 5 · Diagnostic of the obstruction

The matching procedure (♦) requires *two* dimensionless numbers on the
right-hand side: the lattice cutoff coupling `1/g_c²` and the continuum
target `1/g_R²(μ)`. Mechanism B succeeds if and only if **both** are
specifiable from Axiom-Zero invariants without using each other.

For FTD as currently constituted:

- The lattice side (left of ♦) is what we want to derive.
- The continuum side (right of ♦) has no Axiom-Zero target other than `1/x_+`.
- The Phase-G theorem forces `b_0 = 0` at the level the engine can compute, so there is no flow to break the circularity (no scale-dependent term to match against an experimental scale-dependent observable).

**The diagnosis:** Mechanism B does not produce a first-principles
`g_c` because the "matching" reduces to a tautology

```
  g_c  =  √(2π · 1/x_+)   ⟺   x_+ ↔ 1/α   (FTD-0013).            (5.1)
```

The two statements (5.1) carry the same information. If `x_+ ↔ 1/α` is
[STRONGLY MOTIVATED CONJECTURE] (current LEDGER status of FTD-0013),
then `g_c = √(2π α_ref)` is **also** [STRONGLY MOTIVATED CONJECTURE],
with no independent derivation.

This is not a no-go in the strong sense (FTD-0059) — `g_c` is
dimensionless and could in principle live in `R`. It is a no-go in a
weaker sense: **`g_c` cannot be derived independently of FTD-0013 via
Mechanism B**.

---

## 6 · What would unlock Mechanism B

For Mechanism B to give a genuinely independent derivation, one of the
following must change:

1. **A non-trivial lattice β-function.** The engine's reduced action
   would need a marginal or relevant interaction beyond the bilinear
   `|∂J|² + g_c · s · (∇·J)` form, so that `b_0 ≠ 0` and the matching
   becomes a flow equation rather than a boundary condition. This
   requires either (i) elevating `s` from a Gauss-constrained
   manifestation field to an independent dynamical field with its own
   kinetic term and self-interaction, or (ii) introducing a non-Abelian
   structure in `J` (a fibre-bundle reformulation). Both are major
   engine redesigns and were not pursued under the current Axiom Zero.

2. **An independent algebraic route to `g_c`.** A first-principles
   identity of the form `g_c² = f(G*, π, integers)` that does **not**
   factor through `x_+`. The §4.3 scan rules out the obvious
   candidates; a deeper search would have to either find a number-
   theoretic relation between the Coulomb-tail prefactor and the
   lemniscatic period that bypasses the master quadratic, or accept
   that none exists.

3. **An external physical anchor for `g_c`.** Treat `g_c` as a *third*
   calibration on top of `a_phys` and `K_B`. This is honest but
   weakens the falsifiable spine: the SM-comparison count
   becomes "FTD has 3 calibrations vs SM's ~20" rather than 2 vs 20
   (per FTD-0059 §5).

None of (1)–(3) closes the [STRONGLY MOTIVATED CONJECTURE] gap of
FTD-0013 directly; they relocate it.

---

## 7 · Verdict

**Mechanism B is closed as a candidate for first-principles `g_c`
derivation in the current Axiom-Zero engine**, on three independent
structural grounds:

(a) The Phase-G geometric Coulomb forces `g_c` to enter only as a
    source-side multiplicative factor, with no propagator
    renormalisation (§3).
(b) The Phase-G/Phase-H reduced action has zero one-loop β
    coefficient, collapsing the matching equation (♦) to a tautology
    (§4).
(c) The only Axiom-Zero-derivable dimensionless number that hits the
    required value of `g_c²` is `2π / x_+`, making Mechanism B
    informationally equivalent to FTD-0013 rather than independent of
    it (§5).

The recommended LEDGER update for FTD-0031:

```
status:  OPEN  →  CLOSED NEGATIVE (2026-04-25)
note:    Mechanism B (lattice → continuum matching) does not produce
         an independent g_c. Mechanisms A (topological) and C
         (classical fixed point) are also closed (see
         OPEN_GC_FROM_FIRST_PRINCIPLES.md §2.1, §2.3). A first-
         principles g_c either requires a non-trivial RG flow that the
         current engine does not generate, or accepting g_c as a
         third calibration. The α claim's status is unchanged: FTD-
         0013 remains [STRONGLY MOTIVATED CONJECTURE] on dual-match
         + CM-uniqueness evidence; it is not promotable to [THEOREM]
         via g_c-derivation under Axiom Zero.
```

This does **not** affect FTD-0001 (master quadratic as algebraic
identity, [THEOREM]) or the Phase-G theorem itself. It also does not
affect dimensionless calibration-independent predictions (`N_c = 3`,
mass ratios, mixing angles) — those remain falsifiable directly.

What it does affect: any narrative that claims FTD has *derived* α as
an emergent dynamical outcome must be retracted or sharpened to "α is
identified with `1/x_+` via the master quadratic (algebraic
[THEOREM]) and the source coupling `g_c = √(2π/x_+)` is set
consistently with this identification (one calibration, not two)." The
engine reproduces QED Coulomb at small r if and only if `x_+ ↔ 1/α`
holds — which is the FTD-0013 conjecture, restated.

---

## 8 · Pointers

```
docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md       # Phase G theorem (§§1-2, §7)
docs/theory/10_eft_program/OPEN_GC_FROM_FIRST_PRINCIPLES.md          # earlier scoping; this doc supersedes §2.2
docs/theory/10_eft_program/THEOREM_A_PHYS_NO_GO.md                   # FTD-0059
docs/theory/10_eft_program/AUDIT_LINK8_CLOSURE.md                    # FTD-0050 (BCC-orthogonality)
docs/theory/10_eft_program/DERIV_PARTITION_FUNCTION_L2.md            # ultralocality of S_E in s
docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md    # FTD-0001 / FTD-0013
docs/theory/07_assessment/core_ledgers/LEDGER.md                                  # FTD-0031 row
engine/include/ftd/ontic/gauge_couplings.h:74-77                     # current G_C ≡ √α hardcode
engine/src/poisson_solvers.cpp:123                                   # Gauss law source
engine/include/ftd/term_toggles.h                                    # coulomb_charge_coupling default 1.0
```
