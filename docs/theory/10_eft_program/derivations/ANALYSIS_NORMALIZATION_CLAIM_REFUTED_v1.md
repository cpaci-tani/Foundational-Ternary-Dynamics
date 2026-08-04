# FTD-0793 — The "G* is a Normalization" Claim, Refuted v1

**Status:** `[REFUTATION — CLAIM NEVER REGISTERED, KILLED PRE-REGISTRATION]` +
`[EXACT — 16 G*^2 = 32 pi W_BCC]` +
`[OBSERVATION — THE TRUE STATEMENT IS ABOUT PERIODS, AND IS STANDARD]`
**Verdict:** `NORMALIZATION_THESIS_VACUOUS_AND_FALSIFIED_PERIODS_IS_THE_RIGHT_FRAME`
**Parents:** Theorem A / Theorem B (sidebranch §§30.1–30.3), `FTD-0785`, `FTD-0791`
**Production impact:** none

## 1. What was claimed, and why it is recorded despite never being registered

Proposed (by me, in-session): that `G*` plays exactly **one** role everywhere —
a normalization / total mass, via `G* = B(1/4,1/2)/sqrt(pi)` — with the master
quadratic as the **sole exception**, and that this unifies Theorem A and
Theorem B as "spectra are ratios at fixed measure, moments are ratios of
different normalizations."

It was sent to a hostile referee before registration and **refuted**. It is
recorded here because the *counter*-findings are load-bearing and because the
failure mode is instructive.

## 2. The claim's own partition is empty on its second cell

`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (registered, line 108) already states

```text
16 G*^2  =  32 pi * W_BCC        verified to 25 digits, residual 0.0
```

The claim itself lists `W_BCC` as a normalization. So the master quadratic's
`x`-coefficient **is** that normalization times an elementary factor, and the
"sole exception" is not an exception under the claim's own criterion. The
constant term goes the same way: `16 G*^3 = 16 B(1/4,1/2)^3 pi^{-3/2}`.

This is not a referee's construction — it is FTD's own registered chain, and
it refutes the claim rather than supporting it.

## 3. "Is a total mass" is a vacuous predicate

Every positive real is the total mass of a measure (`c*rho_4` with
`c = target/B`). Explicitly: `1/alpha = 26.131386... x B`,
`1729 = 329.702908... x B`. "1729 is a normalization" is exactly as true as
"`G*` is a normalization." A predicate with no discriminating power cannot do
explanatory work, and "up to elementary factors" — as written, admitting
`sqrt(pi)`, `1/(2pi)`, `32pi`, and *squaring and cubing* — absorbs anything.

Further, `G* = B(1/4,1/2)/sqrt(pi)` is the substitution `u = x^4` in
`int_0^1 (1-x^4)^{-1/2} dx`, already registered as `MATH_MASTER_QUADRATIC.md`
Theorem 2.1. **Restating a definition is not a unification.**

## 4. Four of the six "appearances" are the same integral

```text
partition function of rho_4 : int_{-1}^{1} dx/sqrt(1-x^4) = B/2 = 2.6220575542921198
lemniscate constant varpi   : 2 int_0^1 dx/sqrt(1-x^4)     = B/2 = 2.6220575542921198
```

Character for character the same integral. `K(1/sqrt2) = varpi/sqrt2` is a
rescale of it; both moments are ratios built from it. The "six appearances"
are one change of variables (four items), one genuine theorem (Watson BCC),
and one algebraic coefficient which §2 reduces to the Watson value. **The
multiplicity was an artifact of renaming.**

## 5. The A/B "unification" contradicts both proofs

- **Theorem A contains no measure.** Its proof is a Buckingham nullspace
  argument: `[E] = [I][Omega]`, so `{I, E, Omega}` span rank two across three
  dimensions and **no length is constructible from the spectrum**. The claim
  replaces a proved dimensional-rank mechanism with an unproved measure story.
- **The spectral Beta is a different Beta.** `C` carries `B(1/n, 3/2)`, not
  `B(1/n, 1/2)`; their ratio is exactly `n/(n+2)`. At `n = 4` this gives
  `C ∝ G*^{-4/3} = 0.235434603...`, and **a total mass to the power −4/3 is
  not the total mass of anything canonical.**
- **"Ratios of different normalizations ⟹ Gamma survives" is FALSE, and
  Theorem B says so.** `<|x|^r> = B((r+1)/n,1/2)/B(1/n,1/2)` is always a ratio
  of two different Betas, yet it collapses to rationals when `n | r`, and to
  algebraic numbers in a second family:

```text
n=4, r=4 : 1/3          n=8,  r=2 : sqrt2 - 1
n=6, r=1 : sqrt3/3      n=12, r=4 : 2 - sqrt3
n=4, r=1 : sqrt(pi)/G*   <- survives
```

  §30.3 warns explicitly that the clean invariant is "*not a rational function
  of n*, **not** *carries a Gamma ratio*," and that the older dichotomy "was an
  artifact of only ever testing `n = 4`." **The claim reintroduced precisely
  the error that §30.3 was corrected — earlier in the same session — to
  remove.**
- **The measure framing fails its own invariance test.** `<|x+beta|>` = 0.59907,
  0.60288, 0.69462 for `beta` = 0, 0.1, 0.5. A property "of the measure" would
  be translation-stable; the origin must be pinned separately by the
  potential's minimum.

## 6. The correct frame is periods, and it is standard

Enumerating what was merged: total mass of a probability measure; the real
period `Omega_+` of the CM curve `y^2 = x^3 - x`; the quarter period of the
Weierstrass function; the lemniscate arc length; the reciprocal AGM limit
(`G* = 2 sqrt(pi)/M(1,sqrt2)`, residual 0.0); the lattice Green's-function
resolvent value `W_BCC`; and the **length ratio** `G* = (1/A) sqrt(6 pi I/(mu Omega))`.

These coincide numerically because **`Gamma(1/4)` is a period**, and the
periods of a CM curve lie in one `Qbar`-line (Chowla–Selberg). That is the
real theorem, it is about periods rather than masses, and it explains the
coincidences with none of the claim's machinery. Merging seven roles under
"total mass" takes the least structured description and asserts it as the
essence; roles 2, 3, 5, 6, 7 carry structure (a lattice, a monodromy, a fixed
point, a resolvent, a length ratio) that total mass does not encode, and
role 7 is dimensionally incompatible with it.

Sidebranch §30.2/§30.3 already contain the sharper statement: **`G*` is
shape information — a ratio of two lengths — and symplectic invariants
discard orbit shape.** That is FTD's own best answer to "what is `G*`," and it
is better than the one proposed.

## 7. What survives

All pre-existing, none novel: `G* = B(1/4,1/2)/sqrt(pi)`; `Z[rho_4] = B/2 = varpi`;
`<x^2> = 4/G*^2`; `<|x|> = sqrt(pi)/G*`; `K(m=1/2) = G* sqrt(pi)/(2 sqrt2)`;
`W_BCC = G*^2/(2 pi) = Gamma(1/4)^4/(4 pi^3)` (verified three independent ways
to 30 digits — the only non-trivial member); and `16 G*^2 = 32 pi W_BCC`.

Allowed at most as `[OBSERVATION]`, non-novel:

> `Gamma(1/4)` is a period; the quartic/lemniscatic constants in FTD lie in
> the `Qbar`-line of periods of `y^2 = x^3 - x`, hence are rational-power
> related.

## 8. The failure mode, recorded

Eighth refuted construction in one session. This one is the worst of the set,
because it did not merely fail an external check — **it contradicted a
correction I had made myself, earlier the same day, to the very theorem it
claimed to unify.** The pattern across all eight is now unambiguous: exact
arithmetic, correctly computed, attached to an interpretation that was never
tested against the framework's own registered statements before being
believed.

The operational conclusion is not "try harder at construction." It is that
construction claims in this programme should be routed to adversarial audit
**before** they are written into any document, and that the first thing an
audit should check is whether the claim contradicts something already
registered — which in this case it did, twice, in documents the claim cited.
