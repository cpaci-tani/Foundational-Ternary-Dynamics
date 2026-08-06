# FTD-0794 — Link 4 Refuted: the Plaquette Selects the Harmonic Law

**Status:** `[REFUTATION — LINK 4's BRIDGE CLAIM]` +
`[EXACT — C_4 INVARIANCE SELECTS 1-x^2 AND EXCLUDES 1-x^4]` +
`[TAUTOLOGY — NULL-FLATNESS IS THE CONCLUSION]` +
`[RETAG — [CONDITIONAL THEOREM] -> [SELECTION, UNEARNED]]`
**Verdict:** `LATTICE_DOES_NOT_SELECT_THE_QUARTIC_IT_SELECTS_THE_HARMONIC`
**Parents:** chain link 4, `A2` (imported), `FTD-0780`, `FTD-0783`, `FTD-0789`,
`GRAV-001` (retired)
**Production impact:** none

## 1. Why this mattered

After the Part III withdrawal, link 4 was **the last unwithdrawn bridge from
lattice structure to the quartic law** — the only remaining claim that the
substrate *selects* the law whose occupancy normalization is `G*`. Audited
refute-by-default and **independently verified**.

## 2. Conceded: the algebra is valid

Under assumptions 1–6 the solution is unique, verified symbolically:

```text
deg <= 2 : no solution      (F(1)=0 forces c1=-1, so F''(0)=-2 != 0)
deg <= 4 : F = 1 - x^4      UNIQUE, no free parameter
deg <= 6 : one free parameter
deg <= 8 : two free parameters
```

Minimality is correctly applied and degree 4 is forced. The algebra is right.
It is the assumptions that carry no weight.

## 3. FATAL — the plaquette `C_4` selects the HARMONIC law and excludes the quartic

The rotation `(Q,R) -> (-R,Q)` is real: order 4, determinant 1, trace 0. But
identifying `(Q,R)` with the normalized phase pair `(x,y)` — the only way the
plaquette can do any work here — **refutes the conclusion**:

```text
harmonic  x^2 + y^2 - 1   ->   x^2 + y^2 - 1     INVARIANT
quartic   x^4 + y^2 - 1   ->   x^2 + y^4 - 1     NOT invariant
```

and no rescaling repairs it: requiring `a^4 y^4 + b^2 x^2 = lam (a^4 x^4 + b^2 y^2)`
identically has **solution set empty** (verified symbolically).

The ring's own dynamics gives the same answer. The 4-site `C_4` Laplacian has
spectrum `{0, 2, 2, 4}`, and `Q = (phi_0-phi_2)/2`, `R = (phi_1-phi_3)/2` are
**exactly the degenerate eigenvalue-2 pair** (verified: `L Q = 2Q`, `L R = 2R`).
Linear ring dynamics `phi_ddot = -K L phi` therefore gives
`Q_ddot = -2KQ`, `R_ddot = -2KR` — two simple harmonic oscillators, orbit
`x^2 + y^2 = 1`. The `C_4` is a rotation *inside a degenerate eigenspace*,
which is precisely why it forces the circle.

**Taken seriously, the minimum lattice says `F = 1 - x^2`.** The lattice
content of link 4 is not zero; it is negative.

## 4. This was already refuted, in a neighbouring section of the same document

The sidebranch `RELATIVITY_CLOSURE_DERIVATION.md` (§26 refutation, written
2026-08-03) states: *"no constant rescaling of (q,p) renders the orbit
`C_4`-invariant (the harmonic case admits one; the quartic does not) … the
quarter rotation is a **harmonic**-clock symmetry imposed on the very
anharmonicity that is supposed to be the mechanism."* That refutation is
quoted **verbatim in the chain document's own Part III withdrawal banner** and
logged as `GRAV-001 … RETIRED … Do not revive`.

**The package retires this argument in Part III and keeps it in §4.**

## 5. FATAL — null-flatness is the conclusion, not a hypothesis

Two independent proofs.

**(a) It is an iff.** Assumptions 1–4 with degree ≤ 4 leave exactly

```text
F = 1 + c1 x^2 - (1+c1) x^4 ,      F''(0) = 2 c1
=>  F''(0) = 0  <=>  c1 = 0  <=>  F = 1 - x^4
```

A conditional theorem whose hypothesis is logically equivalent to its
conclusion inside the working class is a tautology.

**(b) It is literally "no quadratic term."** From energy conservation at
constant mass, `F(x) = 1 - V(Ax)/E` is an **identity**, so

```text
F''(0) = -(A^2/E) V''(0)
```

Assumption 5 ⟺ `V''(0) = 0` ⟺ zero spring constant ⟺ "the clock is not a
harmonic oscillator." The slogan *"zero distinction has zero linear temporal
stiffness"* is a paraphrase of `V''(0)=0` and nothing more. The theorem reduces
to: *even + no `q^2` term + lowest degree ⇒ `q^4`*.

**The framework says so itself.** `ANALYSIS_COUPLED_QUARTIC_CLOCK_FIELD_v1.md`:
*"The quartic-selection lemma remains conditional on the **unearned premise**
that the signed distinction has no quadratic term. Until the substrate derives
or explicitly adopts that premise, `V = q^4/2` is a coherent candidate, **not a
selected law of physical time**."*

No independent justification for null-flatness exists anywhere in the repo or
the sidebranch — only the boxed slogan, plus three *realization* attempts that
all closed negative (FTD-0783; FTD-0787, itself refuted by FTD-0789). The only
measured native candidate, FTD-0780, returned
`NATIVE_DOUBLET_EXCLUDED_HARMONIC_MODE` — i.e. the substrate **as measured**
has `V''(0) != 0`.

## 6. `G*` is pinned only by the minimality tie-break

The degree-6 family `F_c = 1 - (1+c)x^4 + c x^6` satisfies assumptions 1–5 for a
range of `c`, and gives a **continuum** of clock constants
(`G*(c) = 4 T_4(c)/sqrt(pi)`):

| `c` | quarter period | implied `G*` |
|---|---|---|
| −0.50 | 1.2573392505964 | 2.8375108326875 |
| −0.25 | 1.2825498301619 | 2.8944050182316 |
| **0** | **1.3110287771461** | **2.9586751191886** |
| +0.25 | 1.3436810383880 | 3.0323633818750 |
| +0.50 | 1.3818393432498 | 3.1184774543902 |

`Gamma(1/4)/Gamma(3/4)` survives only by an aesthetic minimal-degree
tie-break, for which no physical argument is stated.

Null-flatness is not even canonical among degree-4 selectors: `F'(1)=0`
(smooth turning point) and minimum integrated curvature both give `(1-x^2)^2`
instead.

## 7. Assumptions 3, 4 are definitions; "analytic" is decorative

From the same identity: `F(±1)=0` ⟺ `V(±A)=E` ⟺ **A is the turning
amplitude**, which is how `A` is defined; `F(0)=1` ⟺ `V(0)=0` ⟺ **v_max is
attained at `q=0`**, which is how `v_max` is defined. Together they assert only
*bounded periodic motion in an even single-well potential*.

Assumptions 1–5 have an infinite-dimensional **analytic** solution set —
e.g. `H(x) = (1-x^4) exp(c x^4)` is even, analytic, nonconstant, satisfies
`H(0)=1`, `H(±1)=0`, `H''(0)=0`, and is not a polynomial. Assumption 6 silently
upgrades "analytic" to "polynomial."

## 8. Tag overclaim, against the framework's own tagging

| location | tag |
|---|---|
| chain doc §4 (**audited claim**) | `[CONDITIONAL THEOREM]` |
| package `START_HERE_COMPLETE_PHYSICS_CHAIN.md` | "Minimum C4 clock support **[SELECTION]**" |
| package `PROJECT_STATUS_LEDGER.json` | listed under `"conditional"` |
| sidebranch, A2 | "**imported** from the temporal-occupancy program" |
| `ANALYSIS_SURD_OBSERVABLE_EXCLUSION_v1.md` | "`G*` (**one selection**: null-flat quartic)" |
| canonical `LEDGER.md` | **no row exists** |

The package's own flowchart says SELECTION; only the chain prose says
CONDITIONAL THEOREM — and its own sentence calls null-flatness "the
substantive **selection**."

## 9. Honest logical content, and the consequence

**Assumed:** 1-D conservative autonomous motion at constant mass; even
polynomial potential; bounded with turning points; **the potential has no
quadratic term**; lowest degree wins.
**Derived:** the potential is `lambda q^4`.
**Lattice used:** none — the derivation goes through end to end without the
plaquette. When the plaquette *is* used, it gives the opposite answer.

**Recommended retag:** `[SELECTION — null-flatness, unearned]` +
`[TRIVIAL COROLLARY given the selection]` +
`[REFUTED PROVENANCE — the C_4/plaquette motivation selects the harmonic law
and is already retired as GRAV-001]`.

**Consequence for the programme.** The last unwithdrawn bridge from lattice
structure to the quartic law **does not exist**. `G*` enters FTD by *choice* at
link 4 — an adopted premise the substrate has never supplied and, where
measured (FTD-0780), contradicts. What survives of the `G*` chain is: Watson's
1939 theorem that the BCC lattice Green's function carries `Gamma(1/4)`, and
the fact that `z = 1/4` is the unique point where the Gamma reflection ratio is
the eigenvalue of the square root of integration. Both are classical
mathematics about `Gamma(1/4)`; neither selects a physical law.
