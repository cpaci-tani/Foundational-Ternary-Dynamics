# FTD-0784 — Surd Observable Exclusion v1

**Status:** `[CONDITIONAL — TRANSPORT OF FTD-0314 C3'S ARGUMENT TO THE
OBSERVABLE FIELD; NOT AN INDEPENDENT THEOREM]` +
`[EXACT — MOMENT EXPRESSIBILITY OF THE MASTER-QUADRATIC INVARIANTS]` +
`[STRUCTURAL COROLLARY — THE Z/2 GAP AND THE DEGREE-2 ESCAPE]`
**Verdict:** `SURD_UNREACHABLE_BY_RATIONAL_SINGLE_CLOCK_OBSERVABLES`
**Parents:** `FTD-0314`, `FTD-0242/0243/0244` (spine), Theorem A / Theorem B
(spectral rigidity / position-space survival, sidebranch §30)
**Condition:** Chudnovsky 1976 — algebraic independence of `pi` and `Gamma(1/4)`
(already a standing spine condition; nothing new is assumed)
**Production impact:** none; pure algebra, no engine execution

## 1. Result in one sentence

No rational function of single-clock occupancy moments — and no spectral
invariant at all — can equal the FC-W surd `delta = sqrt(G*(4G*-1))`; the
substrate can reach `delta^2` exactly but never `delta`, and the gap between
them is precisely one sign choice, which is the `Z/2` twist FC-W adopts
externally — so FTD-0314's carrier-narrowing theorem acquires a dynamical
extension: even a fully successful Gate-C carrier would not pay for W.

## 2. The objects, exactly

With `G* = Gamma(1/4)/Gamma(3/4) = 2.9586751...`:

- the surd: `delta = sqrt(G*(4G*-1)) = 5.6618335126...`; note
  `delta^2 = 4G*^2 - G*`, an element of `Q(G*)`.
- master-quadratic roots: `x_pm = 8G*^2 pm 4G* delta`, with
  `x_+ = 137.036171...`, `x_- = 3.023964...`; splitting `x_+ - x_- = 8G* delta`.
- trace and determinant: `tr = 16G*^2 = 140.0605...`,
  `det = 16G*^3 = 414.392...` (Vieta; verified to 30 digits).

**Moment expressibility [EXACT].** For the quartic clock (`n = 4`), the
normalized occupancy moments are `<x^2> = 4/G*^2` and `<abs(x)> = sqrt(pi)/G*`
(Theorem B machinery). Therefore

```text
tr  = 16 G*^2 = 64 / <x^2>
det = 16 G*^3 = 64 sqrt(pi) / ( <x^2> <abs(x)> )
```

Both invariants of the master quadratic are rational-coefficient monomials in
measured single-clock moments (one of them with a single factor `sqrt(pi)`).
The quadratic's *coefficients* are native-measurable. Its *roots* are not —
that is the exclusion, and it is the one genuinely new [EXACT] content here
(see §3b: the exclusion *argument* is FTD-0314 C3's, transported).

## 3. The exclusion, restated over the observable field

**Setting.** Single-clock observables built from occupancy statistics are
rational functions of the moments; every moment lies in the field
`F = Q(G*, sqrt(pi))` (Theorem B: each normalized moment is a rational
multiple of a power product of `G*` and `sqrt(pi)`). Spectral invariants are
rational in `n` outright (Theorem A) and carry no transcendental at all.

**Statement.** `delta` is not an element of `Q(G*, pi, sqrt(pi))` — a field
strictly containing `F`. Hence no rational function of moments equals
`delta`, nor `8G* delta` (the splitting), nor either root `x_pm`.

**Argument** (FTD-0314 C3's technique, run over a different field). By Chudnovsky 1976, `pi` and `Gamma(1/4)` are algebraically
independent; since `G* = Gamma(1/4)^2/(pi sqrt(2))` (reflection), `G*` and
`pi` are algebraically independent too, so `Q(G*, pi)` is isomorphic to the
rational function field `Q(X, Y)` under `X -> G*`, `Y -> pi`.

(i) *`sqrt(pi)` is quadratic over `Q(X,Y)`:* `Y` has odd valuation (namely 1)
at the prime `Y`, so it is not a square; `Q(G*, pi, sqrt(pi))` is a degree-2
extension, with general element `a + b sqrt(pi)`, `a, b` in `Q(X, Y)`.

(ii) Suppose `delta = a + b sqrt(pi)`. Squaring:
`delta^2 = a^2 + b^2 Y + 2ab sqrt(pi)`. But `delta^2 = X(4X - 1)` lies in
`Q(X, Y)`, so `ab = 0`.

(iii) If `a = 0`: `b^2 = X(4X-1)/Y`. The `Y`-adic valuation of the right side
is `-1`, odd — not a square in `Q(X, Y)`. Contradiction.

(iv) If `b = 0`: `delta` in `Q(X, Y)` requires `X(4X-1)` to be a square
there. Its `X`-adic valuation is `1`, odd — not a square. Contradiction. ∎

The same valuation count kills the splitting: `(8G* delta)^2 = 64 X^3 (4X-1)`
has `X`-valuation 3, odd. And each root `x_pm = 8X^2 pm (1/2)sqrt(64X^3(4X-1))`
is in the field iff the splitting is. Nothing rational in moments reaches any
of the three.

**Condition audit.** The only input beyond exact clock algebra is Chudnovsky
1976. In particular the earlier informal route ("`G*(4G*-1)` not a square in
`Q(G*)`", which leaned on a separate spine result) is *not needed*: inside
`Q(X,Y)` non-squareness is a one-line valuation fact. This inherits
exactly the spine's existing conditionality, no more.

## 3b. Honest relation to FTD-0314 C3 — this is a transport, not a new theorem

**Added 2026-08-03 under the no-promotion rule (FTD-0785).** FTD-0314's C3
already excludes the surd from the CM-period field by *the same technique*:
`surd² = (4Γ(1/4)⁴ − √2πΓ(1/4)²)/(2π²)`, whose numerator factors as
`v²(4v² − √2 u)` with squarefree part of degree 1 in `u = π` — a
non-square. §3 above runs the identical non-squareness argument, over
`Q(G*, π, √π)` instead, using valuations rather than factorization. **The
exclusion method is not new and must not be counted as a second, independent
result.**

What *is* new, and all this document may claim:

1. `[EXACT]` **Moment expressibility** (§2) — that both master-quadratic
   invariants equal rational-coefficient monomials in measured single-clock
   moments. There is no analogue of this in FTD-0314; it is what makes the
   exclusion bite on *data* rather than on periods.
2. `[SCOPE TRANSPORT]` **A different domain.** C3 excludes periods of the
   lemniscatic curve under its forced order-2 maps. This document excludes
   *measured observables* — Theorem A spectra and Theorem B occupancy moments
   — which is the domain Gates A/B/C actually deliver. Neither field contains
   the other (`√π` lies outside `Q̄(π, Γ(1/4))` by the same valuation count),
   so this is a genuine extension of reach, but of an existing argument.
3. `[SYNTHESIS]` The dynamical consequence in §4 and the one-bit framing in §5.

**A finding about C3 itself, recorded here for FTD-0314's next revision:**
C3's period field `F = Q̄(π, Γ(1/4))` is stated inconsistently. It asserts
every forced period lies in `F` and cites `Ω = Γ(1/4)²/√(2π)` with
`Ω/G* = √π` — but `√π ∉ Q̄(π, Γ(1/4))` (π has odd valuation). So either `Ω`
is not in `F` (and "every period lies in `F`" is false as written), or `F`
tacitly means `Q̄(π, Γ(1/4), √π)` and should say so. The conclusion survives
either way — the enlarged field is exactly the one §3 handles — but the
definition needs repair.

## 4. What this does to FTD-0314

FTD-0314 narrowed the binding law `W` to an external `Z/2` adoption with one
`[OPEN]` loophole. This result extends it dynamically, in two directions:

1. **Spectral door closed:** Theorem A says every dimensionless spectral
   monomial of single-orbit data is rational in `n` — nothing transcendental
   survives, so no spectrum ever exhibits `delta`. The coupled-pair
   "exchange-`Z/2`" route (find `delta` in a doublet spectrum) is excluded at
   the level of what spectra can express.
2. **Occupancy door closed:** by §3, position-space statistics —
   the one place `G*` *does* survive — still cannot reach `delta`. Even a
   carrier that passes Gates A, B, and C, delivering `G*` as a measured
   output, delivers only elements of `F`. **Total Gate success leaves W
   unpaid.** The surd's externality is not an artifact of missing dynamics;
   it is field-theoretically robust to every dynamical success the program
   could still have.

## 5. The `Z/2` gap and the degree-2 escape [STRUCTURAL COROLLARY]

`delta^2 = 4G*^2 - G*` **is** in `Q(G*)` — reachable, since Gate C's premise
is bare `G*` as a measured output (the length identity
`G* = (1/A) sqrt(6 pi I/(mu Omega))` delivers it from action-angle plus
amplitude data; occupancy moments alone deliver only the even part
`Q(G*^2, G* sqrt(pi))`, a subtlety recorded here for completeness). The
magnitude is native. What is unreachable is the *signed* object `delta` itself: the field
extension `F(delta)/F` has Galois group exactly `Z/2`, and choosing which
square root is `+delta` is literally the branch datum. **The distance between
what the substrate can measure and what FC-W needs is one bit — the bit FC-W
adopts.** This is the sharpest form of "W is external" the program has
produced.

The one dynamical shape that steps outside the theorem's scope: a genuinely
**two-mode** native object whose symmetric response matrix has
moment-valued invariants (`tr = 64/<x^2>`, `det = 64 sqrt(pi)/(<x^2><abs(x)>)`).
Its eigenvalue problem is degree 2 — the *physics* takes the square root, and
the eigenfrequency splitting would equal `8G* delta` with the `Z/2` realized
as mode labeling (root swap = mode swap). This does not violate the theorem
(eigenvalues are algebraic, not rational, functions of matrix entries); it is
FTD-0314's `[OPEN]` loophole given concrete dynamical form. It is a **shape,
not a candidate**: it presupposes a full Gate-A/B/C carrier *plus* a forced
two-mode structure with exactly those invariants, and the one construction
attempted in this direction (sidebranch §26 and the coupled-clock
master-quadratic assembly) is already refuted/no-go (the pair operator is
traceless). No candidate is registered.

## 6. Scope

Static algebra over exact clock quantities. Nothing here bears on whether a
carrier exists (currently negative at every identified door, FTD-0781/0783);
it bounds what any carrier could ever deliver. The frontier hierarchy stands:
`pi` (forced by geometry) ⊂ `G*` (one selection: null-flat quartic) ⊂ `delta`
(two selections: the quadratic *and* a branch). Numerical verification:
`scripts/experiments/verify_surd_exclusion.py` (30-digit identity checks).
