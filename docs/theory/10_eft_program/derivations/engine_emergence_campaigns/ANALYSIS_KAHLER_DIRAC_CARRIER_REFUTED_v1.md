# FTD-0798 — The Massive Kähler–Dirac Carrier, Refuted; and the Born Density the Flux Already Has

**Status:** `[CLOSED NEGATIVE — KD CARRIER, FAILS C1 AND C3 AT SIGHT]` +
`[THEOREM — NEW: THE SECOND-ORDER FLUX ALREADY CARRIES A BORN-VALUED POSITIVE
CONSERVED DENSITY]` + `[CONSTRUCTIVE — NEW: M18 ADMITS A LOCAL 3-SQUARE
DECOMPOSITION]` + `[CORRECTION — C2 WAS MISREAD BY ITS OWN AUTHOR]`
**Verdict:** `KD_CARRIER_ASSUMES_ITS_CONCLUSION_AND_ITS_MOTIVATION_IS_FALSE`
**Parents:** `FTD-0089`, `FTD-0199`, `FTD-0200`, `FTD-0357` (four walls),
`FTD-0379`, `FTD-0781`, `SPEC_CARRIER_CONSTRAINTS_v1`
**Production impact:** none — the proposal was killed before registration

## 1. The proposal

A massive Kähler–Dirac field `H = (d + d*) + m*Gamma`, `Gamma = (-1)^p`, on the
cubic de Rham complex (8 components, `1+3+3+1 = 2^3`), offered as a
"first-order clock operator carrier": first order so `rho = psi^dag psi` is
positive and conserved (Born-compatible), gapped at `omega_0 = m` (the de
Broglie clock), and with C2 satisfiable because `m` is a free parameter.

**Refuted on six independent grounds.** The operator identities are all
correct — and none of them help.

## 2. The motivating premise is FALSE (the most important finding)

The proposal was motivated by: *"the second-order flux's only conserved
positive density is the energy, which weights modes by `omega^2` and therefore
cannot be Born."* **Wrong.** The positive-frequency number density

```text
phi_plus = (1/2)( w^{1/2} phi + i w^{-1/2} phidot ),      n = |phi_plus|^2
```

is pointwise non-negative, conserved (drift `1.05e-2` over 600 steps;
referee measured `7.1e-16` in his normalisation), obeys the Hermitian
first-order law `i d_t phi_plus = sqrt(-grad^2 + m^2) phi_plus`, and is
**Born-valued**: at equal *mode occupation* two modes split **0.5000 : 0.5000**
(independently verified here).

The original "factor 23.7" compared equal **field** amplitudes, which is not
equal Born amplitude — the standard relativistic mode normalisation is
`A ~ 1/sqrt(2 omega)`. Comparing equal field amplitudes gives `0.3838:0.6162`;
comparing equal occupation gives exactly `0.5:0.5`.

**Consequence, and it is general:** the existing second-order flux field
*already* carries a positive, conserved, Born-valued density, at the cost only
of quasi-locality (the `w^{-1/2}` kernel decays as `~1/m`). This **forecloses
every future argument of the form "FTD needs a first-order sector to obtain a
Born measure."** That argument is now closed as a class.

## 3. C2 was misread by the person who wrote it

`SPEC_CARRIER_CONSTRAINTS_v1` (registered 2026-08-03) states verbatim that
**C2 cannot be repaired by driving amplitude, because the doublet is a linear
normal mode of a positive-definite Hessian**, and that **C3 is the mechanism by
which C2 could be satisfied**. C2 is MacKay–Aubry non-resonance for a
**nonlinear breather**; the whole gate exists to require that the frequency lift
come from a *native nonlinear mechanism*.

**Inserting a free parameter `m` is precisely the move C2 was written to
exclude.** The proposal inverted its own constraint's meaning.

It also fails two Tier-1 constraints **at sight, without measurement**:

- **C1** requires a *nonlinear conservative sector*. The KD field is exactly
  linear. **FAILS.**
- **C3** requires `n = 4`, `V''(0) = 0` with quartic growth. The KD field is a
  harmonic normal mode of a positive-definite operator, `n = 2`. **FAILS.**

**Quantitative bonus kill.** The carrier's maximum group velocity falls as
`1/m`, dropping below the flux cone `1/sqrt(3)` at `m* = 2/sqrt(3) = 1.154701`
— which is **below** the C2 band threshold `2 arcsin(1/sqrt 3) = 1.230959`.
**Every `m` that satisfies C2 makes the carrier slower than the flux it is
supposed to clock.**

## 4. `rho = psi^dag psi` does not survive FTD's coupling

The production coupling is `delta_J = C_WAVE^2 Lap18(J) - G_C grad(s) + G_C curl(s v)`
— **additive sources**, not a potential or a `U(1)` connection. For a
first-order unitary law, `i psi_t = H psi + f` gives
`d||psi||^2/dt = 2 Im<psi, f> != 0`. Measured: norm `1.000 -> 1.234` in 6 time
units at source strength 0.05 (free case drift `5.5e-10`). Admissible couplings
*do* preserve it — real scalar potential (`2.7e-15`), `U(1)` link connection
(`4.4e-14`) — and **FTD has neither**: `engine/CMakeLists.txt:1212` records
"J is R^3-valued, not U(1)"; `term_toggles.h:96` records gauge links as
"write-only — no substrate feedback"; `Voxel` is entirely real.

This is already a registered theorem: `THEOREM_FOUR_WALLS_v1` (FTD-0357)
Lemma C — a many-to-one readout "cannot carry a globally invertible dynamical
pairing." And genesis is a threshold projection, not unitary: **37% norm loss**
in one application.

## 5. FTD-0379 is a direct hit

FTD-0379 closed Dirac–Kähler **NEGATIVE at v1.1 using the true DK operator with
`delta = d*`** — the exact object proposed here: *"the true DK operator
contributes nothing at the tested protocol; fitted speed `a* ~ 0`"*, with
per-grade Klein–Gordon winning 4/4. The only escape is "this is new dynamics,
not a description," which concedes §6.

## 6. Ontological cost: P3 is abandoned, not extended

P3 states `J` is primary and `s` is *the action of J via the genesis threshold
rule* — not an independent field — with its value set grounded in Axiom 0.
An 8-component complex field is (i) independent, (ii) not a threshold
projection of `J`, (iii) not Axiom-0 grounded, (iv) not ternary. **All four
clauses fail.** This is a different theory that keeps the lattice.

## 7. Taste breaks the clock

Eight components are **4 exactly degenerate tastes** (splitting `1.3e-15`). Any
Hermitian non-scalar perturbation splits them at `O(epsilon)`: `1e-3 -> 4.4e-3`,
`1e-2 -> 4.4e-2`, `0.1 -> 0.44`. Density conservation survives; **the degeneracy
does not.** Four species, four clock rates, no selection principle. FTD-0089 §A2
already registers that `Cl(3,0)` supports a single mass scale by Schur.

## 8. The reduction that actually kills it

A **one-component** complex field with any Hermitian generator
(`i psi_t = (-grad^2 + m) psi`) delivers every advertised property — positive,
conserved (drift `3.3e-14`), gapped, tunable. **The Kähler–Dirac structure
contributes nothing to the Born argument.** All the work is done by "complex
field + Hermitian generator" — which is postulating quantum mechanics.

**The proposal assumes its conclusion**, and the eight components are decoration
that costs four unwanted tastes.

## 9. Salvage — two new results, both against the proposal

1. **The positive-frequency Born density** (§2) — `[THEOREM — NEW]`. Forecloses
   the "first-order sector needed for Born" argument as a class.
2. **`M18` admits a local, cubic-covariant 3-square decomposition** —
   `[CONSTRUCTIVE — NEW]`, reproducing `M18` coefficient-for-coefficient across
   all 512 Fourier modes to `1.9e-13`. So a *local* Kähler–Dirac operator with
   `D^2 = M18 . I` exists (the naive one gives `M6`, reintroducing a
   dimension-four anisotropy the production stencil does not have). Currently
   purposeless, but real, and found by the referee against his own interest.

## 10. Record

Eleventh refuted construction of the session. Distinctive failure modes worth
keeping: the author **contradicted a constraint he had written the previous
day**, and the motivating premise was an artifact of comparing the wrong
amplitudes. Both were catchable by reading the framework's own documents.

**Side item flagged:** `ANALYSIS_DE_BROGLIE_CLOCK_v1.md:16` still carries the
pre-2026-07-18 `+G_C grad(s)` sign, contradicting the code.
