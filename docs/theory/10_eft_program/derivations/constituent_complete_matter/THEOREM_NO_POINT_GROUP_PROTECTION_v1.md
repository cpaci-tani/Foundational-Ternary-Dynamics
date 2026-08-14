# THEOREM — No point-group selection rule protects an embedded carrier mode

**Identifier:** `FTD-1003`
**Date:** 2026-08-14
**Status:** `[THEOREM — NO-GO, conditional on the FTD-0551/0641 selected matched-field dynamics; scoped to linearized dynamics and to spatial point-group symmetry]` + `[CORRECTION — the operative band-clearance edge is the band top, not the axis top]` + `[OPEN — topological mechanism not closed]`
**Parent:** `FTD-0663` (`THEOREM_INTERNAL_MODE_FIELD_BAND_EMBEDDING.md`), whose fourth caveat this partially discharges
**Certificate:** [`scripts/proofs/proof_symmetry_protection_no_go.py`](../../../../../scripts/proofs/proof_symmetry_protection_no_go.py), SHA-256 `5A0531162DB8C5BE04E7B96FF9F74D260A30074557227CA066F82B72C2F77A83` — **17/17 computational checks passed; 11 disclosure/scope assertions logged (cannot fail)**. Read-only mathematics: no engine contact, no numerical search, no production change.
**Adversarial review:** provisional AI red-team pass (ftd-math-redteam) run against the first draft; five defects found and repaired before booking (see §6). That pass is not external human review and does not substitute for it.

---

## 0 · What this closes, and what it does not

FTD-0663 excluded *frequency-gap* protection exactly and excluded complete
decoupling for a prepared finite-volume excitation, then left four
mechanisms open verbatim: **"symmetry, destructive interference, a bound
state in the continuum, or a topological invariant."**

This document closes the **first**, in its spatial point-group sense, and
places the second and third under a sharpened accidental-cancellation
account. The **fourth is not closed** and is left explicitly open.

> **Theorem (no point-group protection).** For the linearized matched
> face/edge field sector with dispersion
> `Omega(k) = 2 asin(sqrt(sum_a sin^2(k_a/2)/3))`, no spatial point-group
> selection rule — under `O_h` or any subgroup, at any site symmetry — can
> force the on-shell coupling of a localized internal mode to vanish
> identically on the isofrequency surface `S(omega)`, for **any** `omega`
> strictly inside `(0, pi)`.

Consequently a non-decaying embedded carrier at such a frequency requires
either accidental on-shell vanishing (tuning, not protection — and here
achievable at codimension *exactly* one, §4) or a mechanism outside spatial
point-group symmetry.

## 1 · Why this matters: it is the last escape on the composite-boost row

`ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md` §5 records the C2
band-clearance requirement as the **sole identified blocker** on the
composite-boost item of the two-body Lorentz obligation, after the
"nodes are Newtonian" issue was correctly reclassified as a modelling error
fixable by substitution. C2 says: a carrier whose internal frequency lies
inside the propagating band couples to travelling modes and radiates, so it
is a resonance rather than a clock.

Symmetry protection was the one route by which a carrier could sit inside
the band and *not* radiate — which would have voided C2 without paying its
cost. This theorem closes that route.

## 2 · The channel space is a rank-2 bundle, not scalar `L^2(S)`

This distinction is load-bearing and was the principal defect of the first
draft. Per FTD-0641 the propagating sector is divergence-free with **two
polarizations per k**; longitudinal modes are slaved by the constraint and
do not propagate. The channel space at frequency `omega` is therefore the
rank-2 **transverse sub-bundle** over `S(omega)`, not scalar `L^2(S)`.

Why the difference decides the theorem rather than decorating it: **the
continuum `SO(3)` analogue of this no-go is FALSE.** A spherically pulsating
source does not radiate (Birkhoff), and it does not radiate precisely
because the momentum sphere is a **non-free** `SO(3)` orbit whose transverse
bundle carries no `l = 0` sections. An argument phrased on scalar `L^2(S)`
would reach the correct lattice conclusion by reasoning that yields a false
theorem one limit away. The lattice conclusion survives because its orbits
are free — which is exactly what §3 establishes.

## 3 · The argument

Let a localized mode carry irrep `Gamma` of the site group, and let the
coupling `V` be invariant. The on-shell coupling is then an equivariant
section of the transverse channel bundle over `S(omega)`. Symmetry forces it
to vanish identically iff `Gamma` is **absent** from the channel
representation.

1. **`S` is a regular level set.** `grad Omega = 0` requires `sin(k_a) = 0`
   for every `a`, i.e. every `k_a in {0, pi}`, giving
   `s = sum_a sin^2(k_a/2) in {0,1,2,3}` exactly. For the first internal
   doublet, `s* = 3 sin^2(omega_0/2) = 0.8078216321246` avoids all four, so
   `S(omega_0)` is a smooth 2-manifold. (Analytic, not sampled.)
2. **Generic points of `S` have trivial stabilizer.** The fixed-point set of
   any non-identity element of `O_h` is a union of flats of dimension `<= 2`;
   a real-analytic level set of the non-constant `Omega` cannot contain a
   2-dimensional piece of such a flat without `Omega` being constant on an
   open subset of it. Hence the free part of `S` is **open and dense**. The
   certificate additionally exhibits 30 free orbits constructively (orbit
   size exactly 48).
3. **Sections over a free orbit carry every irrep.** For a rank-`d`
   equivariant bundle over a free orbit,
   `Ind_e^{O_h}(fiber) = d x (regular representation)`, whose multiplicity
   for each irrep is `d * dim(Gamma) > 0`. With `d = 2`: every one of the ten
   `O_h` irreps appears, with multiplicities
   `A1g:2, A2g:2, Eg:4, T1g:6, T2g:6, A1u:2, A2u:2, Eu:4, T1u:6, T2u:6`.
4. **Therefore** no irrep assignment can forbid coupling by symmetry. ∎

**Criterion used.** The load-bearing statement is not the leading-order
Fermi golden rule but the **Friedrichs-model necessary condition**: a genuine
embedded eigenvalue of the linear coupled system requires the on-shell
coupling to vanish identically on `S` at the *renormalized* frequency. Since
the argument covers every `omega in (0, pi)`, frequency renormalization
cannot escape it.

**Lower site symmetry only strengthens this.** If the true invariance group
is `H <= O_h`, then `Stab_H(k) subset Stab_{O_h}(k) = {e}` at the exhibited
points; a free `O_h`-orbit splits into free `H`-orbits, each giving the
`H`-regular representation, which again contains every `H`-irrep.

## 4 · The escapes that remain, priced honestly

| escape | status |
|---|---|
| (i) `S` degenerates | Only **at** the band endpoint: `Omega(pi,pi,pi) = pi` with orbit size 1 (full `O_h` stabilizer). A measure-zero endpoint, not an operating regime. |
| (ii) accidental on-shell vanishing | **Codimension exactly 1, constructible** — sharper than the generic Friedrich–Wintgen expectation. `sigma^2 = 4s` is *constant* on `S` (verified: `3.2312865285` across all witnesses), so any coupling factorizing as `(c1 + c2 sigma^2(k)) M_bare` is killed on the entire surface by the single tuning `c1 = -4 c2 s*`. This is **tuning, not protection**: it protects one frequency and is unstable to any perturbation, so it cannot underwrite a carrier without its own forcing argument. |
| (iii) strong coupling expelling the mode | That is band clearance — i.e. **C2 satisfied**, not a protected embedded mode. Not an escape from C2; it *is* C2. |
| (iv) topological mechanism | **NOT CLOSED.** FTD-0663's fourth caveat stands. No candidate mechanism is identified here for a genuinely localized mode (which lacks the conserved transverse-momentum label the photonic-crystal construction requires), but absence of a candidate is not a proof. |

## 5 · Correction of record: the operative band edge is `pi`, not `2 asin C`

The corpus records the C2 clearance edge as
`omega_B = 2 asin(1/sqrt 3) = 1.2310` — see `SPEC_CARRIER_CONSTRAINTS_v1.md`
§C2, `ANALYSIS_MOVEMENT_SECTOR_C2_WALL_v1.md`, and
`ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md` §5. That value is the
**`<100>` axis-branch maximum**, not the band top.

FTD-0663 and this theorem both give the transverse band as `[0, pi]`, and the
no-go's argument applies at **every** `omega in (0, pi)` — the certificate
exhibits a free orbit at `omega = 1.5`, above the recorded edge, still
embedded and still unprotected. A carrier at `omega in (1.2310, pi)` is
therefore still a resonance.

**Consequence, which cuts against the programme:** true band clearance
requires `omega > pi`, roughly `2.55x` higher in frequency than recorded, and
the recorded cost `epsilon >= 4.22` (already flagged unaffordable) is a
**lower bound computed against a too-low edge**. This makes the
composite-boost blocker *harsher*, not milder. Owner ruling is invited on
whether any recorded reason exists for treating the axis top as the operative
edge; absent one, the C2 cost line needs recomputation.

## 6 · Defects found by adversarial review and repaired before booking

| # | defect | repair |
|---|---|---|
| 1 | Channel space modelled as scalar `L^2(S)`; would give a false theorem in the continuum limit | Replaced by the rank-2 transverse bundle with the induced-representation lemma (§2, §3.3) |
| 2 | Headline claimed all symmetry protection; FTD-0663 lists four mechanisms | Retitled to the point-group selection-rule form; mechanism (iv) explicitly left open |
| 3 | Fermi-golden-rule framing is leading-order only | Replaced by the Friedrichs embedded-eigenvalue necessary condition; scoped to linearized dynamics |
| 4 | Genericity rested on 30 sampled points | Analytic backstop added (fixed-point flats vs. real-analytic level set); samples demoted to witnesses |
| 5 | Accidental escape stated as "codimension `>= 1`" | Sharpened to **codimension exactly 1, constructible**, via `sigma^2` constant on `S` |

A torus-boundary defect (`-pi` and `+pi` returned as distinct floats by
`math.remainder`, which would have made the corner read as an 8-point orbit
rather than a fixed point) was found and fixed during construction.

## 7 · Scope

Conditional on the FTD-0551/0641 **selected** matched-field dynamics — this
is not postulate-forced content and the no-go inherits that conditionality.
Scoped to linearized dynamics and to spatial point-group symmetry. It proves
symmetry cannot *force* the coupling to vanish; it does not prove the
coupling is nonzero (FTD-0676 separately *measures* `Gamma_E = 0.00653712`
per tick for this mode, so the actual coupling is nonzero). Harmonics also
lie in band (`2 omega_0 = 2.182 < pi`), and MacKay–Aubry — C2's own citation
— requires harmonic clearance too.

**Nothing is promoted.** `x+ = 1/alpha` stays `[SMC]`; MC-T4.3 stays a
`[FOUNDATIONAL OBSTRUCTION]`; `C_SPEED` remains a `[SELECTION]`; FTD-0663
stands; FTD-0208 stands; no `alpha` is derived anywhere. The composite-boost
row remains `[OPEN]` — this theorem removes an escape from its blocker, it
does not deliver the carrier.
