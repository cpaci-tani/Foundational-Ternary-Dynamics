# EXPLR — Connection Between 1/(2π) and G*: An Honest Assessment

**Document type:** Exploratory note (line examined and closed)
**Status:** [OBSERVATION] — records the structural connection that exists; explicitly NOT a new derivation
**Created:** 2026-04-30
**Provenance:** Q4 follow-up from `DERIV_RETARDED_GREEN_LATTICE.md` (FTD-0113, 2026-04-30 earlier today)
**Related:** `SPEC_ALGEBRAIC_SPINE.md §5` (Watson identity, the actual connection);
`DERIV_RETARDED_GREEN_LATTICE.md` (origin of `1/(2π)` as Phase G continuum amplitude);
`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (Watson-identity proof via BCC eigenvalue);
`DERIV_LFUNCTION_GSTAR_CONNECTION.md` (parallel L-function identities involving G*)

---

## 0 · Why this document exists

In `DERIV_RETARDED_GREEN_LATTICE.md` (FTD-0113), the universal
continuum-limit amplitude of the Phase G geometric coupling on the
light cone was identified as `1/(2π)`. The CLAUDE.md anti-target rule
forbids fishing for a numerical match between this `1/(2π)` and the
lemniscatic constant `G*`. The follow-up Q4 was: **examine whether
any structural mechanism connects `1/(2π)` to `G*`** — purely as a
structural inquiry, not as a numerical hunt.

This document records the result of that examination: the connection
exists, it is exactly the Watson identity (already a spine theorem),
and there is no *additional* derivation in either direction waiting
to be discovered.

---

## 1 · The two numbers

```
1/(2π)        ≈ 0.159154943    Phase G continuum amplitude (light cone)
G*            ≈ 2.6220575...   lemniscatic constant Γ(1/4)²/(2√(2π))
G*²           ≈ 6.8751854...
G*²/(2π)      ≈ 1.0944...      Watson-identity value W₃
```

---

## 2 · Structural observation 1 — both numbers carry the same `2π`

The `1/(2π)` in the Phase G continuum amplitude:

```
α_r(r, t, ∞) = 2r · δ(t − r/c) / (4π r)  =  δ(t − r/c) / (2π).
```

Source: 3D Coulomb potential `1/(4π r)` ÷ Phase G prefactor's `2r` → `1/(2π)`.

The `2π` in the Watson identity (`SPEC_ALGEBRAIC_SPINE.md §5`):

```
W₃ = (1/(2π)³) · ∫_{[−π,π]³} d³k · 1 / (2 · (3 − cos kₓ − cos k_y − cos k_z)).
```

Source: Brillouin-zone measure `(2π)³` (one `2π` per spatial dimension)
absorbed into the Glasser-Zucker closed-form `G*²/(2π)`.

**Both `2π` factors descend from the underlying U(1) ≅ S¹ rotation
structure.** Position-space `4π` solid angle and momentum-space `(2π)³`
measure are different manifestations of the same 1D circle period.
Calling them "the same 2π" is structurally accurate.

---

## 3 · Structural observation 2 — G*² is the "self-energy / radiation-amplitude" ratio

Take the ratio of the Watson lattice self-energy and the Phase G
continuum radiation amplitude:

```
G*² = W₃ / [α_r(continuum amplitude)]
    = (G*²/(2π)) / (1/(2π))
    = (lattice self-energy at origin) / (continuum radiation amplitude on light cone).
```

This is a clean dimensionless ratio with structural meaning. `G*²`
quantifies "how much larger the static self-interaction at the origin
is than the radiation amplitude on the light cone, in lattice
electrodynamics." It is calibration-independent and does not change
under any choice of physical-unit anchor.

This is itself worth recording as a structural fact, but it is not
a *derivation* of `G*` — it is just the Watson identity rearranged.

---

## 4 · Structural observation 3 — lemniscate-circle analogy

The lemniscatic constant `G* = ϖ` plays for the lemniscate the role
that `π` plays for the circle (half-arc-length, period of the
inverse function, etc.). One can write a naïve analogy:

```
Circle period: 2π           ↔  Phase G "circular" amplitude: 1/(2π)
Lemniscate period: 2G*      ↔  Phase G "lemniscatic" amplitude: 1/(2G*)?
```

The candidate `1/(2G*) ≈ 0.1907` is the lemniscate-period analog of
`1/(2π) ≈ 0.1592`. **However**, there is no lemniscate-symmetric
lattice in FTD — the engine's lattice has cubic symmetry, not
lemniscate. So this analogy cannot promote to a derivation. It is a
structural curiosity, not a mechanism.

The lemniscate constant appears in FTD via the master quadratic and
Watson identity (number-theoretic origin), not via any geometric
realisation of lemniscate symmetry on the lattice itself.

---

## 5 · The honest verdict

**The structural connection between `1/(2π)` and `G*` already exists
in the spine — it is exactly the Watson identity:**

```
W₃ = G*² / (2π)        [SPEC_ALGEBRAIC_SPINE.md §5, FTD-0001 sub]
```

This theorem says: the static lattice self-energy at the origin
equals `G*²` times the inverse of the (twice-) circle period. In this
identity, the role of `1/(2π)` is "what `G*²` is being normalized
against." The role of `G*²` is "the closed-form Γ-function content
of the lattice Green's function at the origin."

There is no *additional* derivation of `G*` from `1/(2π)` (or vice
versa) waiting to be discovered. The Watson identity is the
connection. Its proof goes via Γ-function evaluation of the
Brillouin-zone integral (Glasser-Zucker 1980), not via any
structural argument that "makes 2π force G*."

---

## 6 · What would have to happen for a NEW result

For a genuinely new derivation of `G*` from `1/(2π)`, one of the
following would need to land:

1. **A first-principles proof** that the lattice self-energy at the
   origin can be computed from purely continuum data (the `1/(2π)`
   amplitude) plus lattice-stencil structure alone, recovering the
   Glasser-Zucker closed form independently of Γ-function machinery.
   No candidate mechanism is on the table.

2. **A symmetry-uniqueness argument** that the only stencil
   compatible with some lattice-symmetry constraint produces
   `W₃ = (1/(2π)) · (specific expression)` where the "specific
   expression" is forced to equal `G*²`. No candidate mechanism is
   on the table.

3. **A modular-forms or L-function identity** linking `G*` to a
   Phase-G-style coupling-amplitude integral with a new closed form.
   The L-function neighbourhood is `DERIV_LFUNCTION_GSTAR_CONNECTION.md`
   (Catalan constant, `L(1, χ_{−4}) = π/4`); this is parallel
   territory but does not yield a `1/(2π) → G*` derivation.

Neither (1) nor (2) nor (3) has a candidate route under current
project state. The Watson identity is the only structural connection,
and it is already a theorem.

---

## 7 · Decision

**Q4 line closed.** No promotion. The connection is real, it is the
Watson identity, and the Watson identity is already a [THEOREM] in
the spine.

This document records the assessment so that:
- Future contributors do not re-open Q4 expecting a new result
- The structural observation (`G*² = self-energy/radiation ratio`)
  is preserved
- The lemniscate-circle analogy is recorded as a structural curiosity
  with explicit non-promotion
- The CLAUDE.md anti-target discipline is honoured: numerical
  proximity (`1/(2π)` and `G*` both have `2π` factors) was examined
  for *structural mechanism*, not pattern-matched for closure.

---

## 8 · LEDGER status

This document does not introduce a new LEDGER entry. It updates the
status of FTD-0113's Q4 follow-up from [OPEN] to **[EXAMINED — closed-line, no promotion]**.

The structural observation that `G*² = W₃ / α_r(continuum amplitude)`
is a one-line restatement of the Watson identity (FTD-0001 sub) and
needs no new entry.

---

## 9 · What this document does NOT claim

- **NOT a new derivation of `G*`.** The Watson identity (already a
  theorem) is the only mechanism connecting `1/(2π)` and `G*`.
- **NOT a derivation of `1/(2π)` from `G*`.** The Phase G continuum
  amplitude follows from 3D Coulomb geometry, independently of any
  number-theoretic content.
- **NOT a uniqueness argument for the master quadratic.** The
  observation `G*² = self-energy/radiation` is structural, not
  selective.
- **NOT a promotion to spine theorem.** Spine count remains 9.
- **NOT a closure of the broader Maxwell-exploit thread.** Q3 (engine
  cross-check), Q5–Q8 (lattice Larmor, Cherenkov rate, bound-state
  LW, source-half consistency) remain open.

---

*End of exploration.*
