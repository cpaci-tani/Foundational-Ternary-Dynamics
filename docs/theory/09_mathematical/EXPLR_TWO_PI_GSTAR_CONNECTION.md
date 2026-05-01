# EXPLR — Connection Between 1/(2π) and G*: An Honest Assessment

**Document type:** Exploratory note (line examined, reopened, tested, closed-negative)
**Status:** **[CLOSED NEGATIVE]** as of 2026-05-01 — Q4a numerical measurement falsified the FTD-0116 [HYPOTHESIS] reopening. See §11 for the falsification result; the rest of the document records the full epistemic trajectory.
**Created:** 2026-04-30, with §11 falsification update 2026-05-01
**Provenance:** Q4 follow-up from `DERIV_RETARDED_GREEN_LATTICE.md` (FTD-0113, 2026-04-30 earlier same day)
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
G*            ≈ 2.9586751...   PROJECT CANONICAL G_STAR
                               = Γ(1/4)/Γ(3/4) = Γ(1/4)²/(√2·π)
G*²           ≈ 8.7538...
G*²/(2π)      ≈ 1.3932...      Watson-identity value W₃ (BCC normalization)
```

**IMPORTANT NOTATIONAL CLARIFICATION (added 2026-04-30 after
collaborator catch).** The project's canonical `G_STAR` (per
`scripts/constants.py`) is the lemniscate ratio `Γ(1/4)/Γ(3/4) ≈
2.9587`, **NOT** the Bernoulli/Gauss lemniscate constant
`ϖ = Γ(1/4)²/(2√(2π)) ≈ 2.6221`. The two are related by
`G_STAR = ϖ · √(2/π)` ≈ ϖ · 0.7979 but they are distinct constants.

The spine document `SPEC_ALGEBRAIC_SPINE.md §1` currently has a
typo: it states `G* = Γ(1/4)²/(2√(2π)·Γ(1/2)) ≈ 2.622057554`, but
(a) the stated formula evaluates to 1.4793 not 2.622, and (b) the
master quadratic check `x² − 16G*²x + 16G*³ = 0` produces
x_+ = 137.036 (= 1/α numerically) ONLY at G* = 2.9587, not at
G* = 2.622 (which would give x_+ = 107.3). Filed as separate audit
item against `SPEC_ALGEBRAIC_SPINE.md` for correction.

Throughout this document, **G\* refers to the project canonical
2.9587** unless otherwise noted.

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

## 7 · Decision (REVISED 2026-04-30, same day)

**Initial decision was over-aggressive.** On further examination
prompted by collaborator pushback ("this sounds VERY interesting"),
the structural observation in §3 (`G*² = self-energy / radiation
amplitude ratio`) carries a *new physical interpretation* that the
algebra-only reading missed. Q4 is **NOT fully closed**; it is
**REOPENED** with three concrete sub-questions (Q4a/b/c, §10 below) and
a sharpened physical interpretation (§9 below).

**What is closed:** the search for a *new mathematical derivation* of
`G*` from `1/(2π)` (or vice versa) at the level of Γ-function or
modular-form identity. The Watson identity is the only such
connection, and it is already a theorem.

**What is OPEN:** the *physical interpretation* of `G*²` as a
UV-IR matching constant / lattice Z-factor analog. This carries
falsifiable predictions that have not been tested.

This document records the assessment so that:
- Future contributors see both the original cautious assessment AND
  the revised re-opening
- The structural observation (`G*² = self-energy/radiation ratio`)
  is preserved with its physical interpretation made explicit
- The lemniscate-circle analogy is recorded as a structural curiosity
  with explicit non-promotion
- The CLAUDE.md anti-target discipline is honoured: numerical
  proximity (`1/(2π)` and `G*` both have `2π` factors) was examined
  for *structural mechanism*, not pattern-matched for closure
- The discipline is preserved by NOT promoting any conjecture, while
  the productive content (engine-testable predictions) is preserved
  by opening proper [OPEN] sub-questions

---

## 8 · LEDGER status

**Initial assessment (this section, original 2026-04-30):** this
document does not introduce a new LEDGER entry; it updates the
status of FTD-0113's Q4 follow-up from [OPEN] to [EXAMINED —
closed-line, no promotion].

**Revised (this section, same day, after collaborator pushback):**
the §9 physical-interpretation reading and §10 sub-questions warrant
a fresh LEDGER entry **FTD-0116** at the [HYPOTHESIS] tag (Z-factor
reading + 3D-specificity prediction; not [DERIVED] because the
Z-factor identification is interpretive and the 3D-specificity claim
requires the §10 Q4c calculation to confirm).

The original structural observation that `G*² = W₃ / α_r(continuum
amplitude)` remains a one-line restatement of the Watson identity
(FTD-0001 sub). What is new in FTD-0116 is the interpretation of
this ratio as the FTD lattice Z-factor analog and the associated
falsifiable predictions.

---

## 9 · The physical-interpretation reading (added 2026-04-30, revised)

Beyond the Watson-identity-rearrangement reading of §3, the structural
observation `G*² = G_L(0) / [continuum 2r·G_L(r) on light cone]` admits
a non-trivial *physical* interpretation:

### 9.1 · G*² as the FTD lattice Z-factor

In QED, the field-strength renormalization constant `Z` relates the
bare (UV-cutoff-dependent) field amplitude to the physical (IR,
measured) field amplitude:

```
e_phys = √Z · e_bare,         A_phys = √Z · A_bare
```

The constant `Z` is determined by the regularization scheme. With
the FTD lattice as natural UV cutoff, the analogous Z is exactly:

```
Z_FTD := G_L(0) / [continuum 2r·G_L(r) on light cone]
       = (G*²/(2π)) / (1/(2π))
       = G*²                                                          (★)
```

If this reading is correct, then `G* = √Z_FTD ≈ 2.9587` is the
**FTD lattice renormalization constant**. The relationship between
the source coupling `g_s` (FTD-native bare coupling) and the
far-field Coulomb amplitude (FTD-native physical coupling) would be
governed by `G*` in the standard Z-factor sense.

This is a falsifiable claim. The EFT recovery program closed three
routes (R1, R2, R3) for deriving `α` from `g_s`, but those tested
*different* relationships. The Z-factor relationship in (★) has not
been specifically tested.

### 9.2 · Three independent paths to G*, with new fourth angle

`G*` already appears via three independent derivation paths:

| Path | Mechanism | Documented in |
|---|---|---|
| (a) Master quadratic | BCC eigenvalue structure | `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` |
| (b) Watson identity | Lattice Poisson Green's function (Glasser-Zucker) | `SPEC_ALGEBRAIC_SPINE.md §5` |
| (c) Lemniscatic constant | Gauss-Schwarz formula `Γ(1/4)²/(2√(2π))` | `DERIV_LFUNCTION_GSTAR_CONNECTION.md` |

The new content of this document adds a **fourth angle**:

| (d) Phase G UV/IR ratio | Lattice-vs-continuum mismatch in radiation amplitude | this document, §3 |

This is **not a new derivation** of `G*` (it just rearranges the
Watson identity). But it gives `G*` a physical interpretation it
did not previously carry: `G*² = lattice Z-factor analog` in FTD's
electrodynamics.

### 9.3 · 3D-specificity prediction

If `G*²` truly is the UV/IR mismatch constant in 3D lattice ED,
then the analogous constant in *other* spatial dimensions should be
DIFFERENT. The 4D cubic-lattice Green's function at origin is also
finite (4D Watson integral converges) but evaluates to a different
closed form NOT involving `G*`.

This is a **falsifiable structural prediction**:
- 3D lattice: Z-factor analog = `G*²` ≈ 8.754
- 4D lattice: Z-factor analog = (different closed form, not involving G*)

If FTD's lattice geometry forces 3D specifically (the spine claims
this via `|Aut(E)|² = 2^D · (D-1)!` at D=3), then `G*` IS the
dimension-3-specific UV-IR ratio. **`G*` becomes a structural marker
of three-dimensionality**, not just an arbitrary closed-form
constant.

---

## 10 · NEW open follow-ups (Q4 reopened)

Three concrete sub-questions emerge from §9.5, each with bounded
scope and falsifiable content:

### Q4a — Engine measurement of Z_FTD = G*²

Measure `g_s` (source coupling, FTD-native) and `α_r(r → ∞, L = large)`
(far-field Phase G amplitude) on the engine. Compute the ratio.
Predicted: it should equal `G*` (or `G*²`, depending on convention).

**Status:** [OPEN]
**Effort:** Bounded engine work (~1-2 days, C++ benchmark).
**Falsifiable:** Yes — agreement with `G*` to engine precision is a
PASS; deviation by O(1) is a FAIL.
**Risk:** Engine convention for `g_s` may not literally be the
"bare Coulomb coupling" in the QED Z-factor sense; needs careful
definition setup before measurement.

### Q4b — Field-theoretic Z-factor calculation from FTD action

Does the FTD action, expanded around the projected-EFT description,
produce `Z = G*²` in the standard one-loop Z-factor sense (matching
of UV-divergent self-energy diagrams with a physical regulator)?

This is a direct calculation that connects to the closed-negative
EFT routes (R1, R2, R3 in `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`).
Those routes tested the relationship `x_+ = (something derived from
FTD action)`. The Z-factor relationship is structurally distinct:
`Z = G*²` rather than `α = G*² × (something)`.

**Status:** [OPEN]
**Effort:** Medium pencil-and-paper EFT calculation (~1 week).
**Falsifiable:** Yes — direct calculation either matches or
doesn't.
**Risk:** May produce a derivation that has an `O(1)` numerical
factor different from `G*²`; would need careful interpretation of
"matching scheme" choices.

### Q4c — Dimensional Z-factor scan

Compute the analogous Z-factor (lattice Green's function at origin
÷ continuum-limit Phase-G amplitude) for cubic lattices in
D = 2, 3, 4, 5 spatial dimensions. If the structural reading of
`G*²` as "3D-specific UV-IR ratio" is correct, the value should be
a dimension-specific closed-form constant with `G*` appearing
specifically at D=3.

**Status:** [OPEN]
**Effort:** Pure number theory / lattice Green's function
literature (~3-5 days).
**Falsifiable:** Yes — D-dependent Z(D) is computable in closed
form for cubic lattices; the prediction is verifiable.
**Risk:** Low — even a NEGATIVE result (i.e., D=3 Z-factor is
"just" `G*²` without 3D being specially marked) is informative.
A POSITIVE result (3D is uniquely structurally selected at the
Z-factor level) would strengthen the algebraic-spine claim that
D=3 is forced.

---

## 11 · Q4a result: FTD-0116 hypothesis FALSIFIED (2026-05-01)

The Q4a sub-question was tested numerically in
`scripts/proofs/proof_z_factor_q4a.py`. Method: compute `G_L(0)` for
the actual FTD lattice stencils (SC and G18) at `L ∈ {8, 16, 32, 64,
96, 128}`, extrapolate to `L → ∞` via Richardson, divide by the
continuum amplitude `1/(2π)`. Compare to predicted `G*² ≈ 8.754`.

**Result:**

| Stencil | Measured Z_FTD = G_∞(0) · 2π | Predicted G*² | Verdict |
|---|---|---|---|
| Simple cubic (SC) | **1.5879...** | 8.754 | ≠ G*² (off by 5.5×) |
| G18 (engine canonical) | **1.9917...** | 8.754 | ≠ G*² (off by 4.4×) |

The naive Z-factor reading **fails by a factor of ~4.4×** for the
engine's actual stencil.

### 11.1 · What went wrong with the hypothesis

The reading `Z_FTD = G*²` conflated two different Watson constants.
The spine's `W₃ = G*²/(2π) ≈ 1.393` refers to the **BCC sublattice**
Watson integral (per `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` §5). The
engine's gauss-projection runs on **G18** (face + edge weights, no
corner), which is a different lattice with a different Watson integral
value (≈ 0.317, giving Z ≈ 1.99). These two Watson constants were
equated wrongly in the original Q4 reading.

In retrospect this conflation should have been caught earlier:
- Spine §5 / FTD-0001-sub: `W₃` is a BCC-sublattice quantity
- Engine: gauss-projection runs on cubic G18, NOT on BCC sublattice
- Cubic-Watson and BCC-Watson are different integrals

### 11.2 · What's actually true

Two clean structural numbers emerged from the Q4a measurement:

**For SC stencil:**

```
Z_SC = G_∞^SC(0) · 2π = π · W_cubic_standard ≈ 1.5879
```

where `W_cubic_standard = 0.5054620...` is the standard Watson 1939
cubic-lattice integral. This is a clean closed form.

**For G18 stencil:**

```
Z_G18 = G_∞^G18(0) · 2π ≈ 1.9917
```

This does NOT equal `π · W_cubic_standard` (which would give 1.588) —
the G18 stencil has its own Watson integral. The numerical value
1.9917 has no obvious closed form; it is a stencil-specific quantity.
Notably, it is close to but does not converge to 2 (residual ≈ 0.4%
even at L=128).

### 11.3 · Status of Q4 sub-questions after Q4a falsification

**Q4a (engine measurement of Z_FTD):** Tested. **FALSIFIED.** Z_FTD
≠ G*² for either SC or G18.

**Q4b (EFT calculation of Z = G*² from FTD action):** No longer
worth pursuing as posed. The naive Z-factor reading is wrong;
without a candidate mechanism that could produce G*² rather than
the cubic-Watson value, Q4b is a directed search for a result that
Q4a has already ruled out.

**Q4c (dimensional Z-factor scan):** Status downgraded but not
necessarily closed. The Q4a result shows that the cubic-G18 Watson
integral is the relevant Z-factor on the FTD lattice. Whether the
*dimensional* Watson integrals on cubic lattices form a structurally
significant family (e.g., are forced to take certain values at
D=3 specifically) is an independent question that can still be
investigated. But it is no longer a Z-factor question; it is a
cubic-lattice-Watson-integral question.

### 11.4 · Updated honest verdict

**FTD-0116 hypothesis FALSIFIED.** The structural connection between
`1/(2π)` and `G*` reverts to the original §5 verdict: the only
mathematical connection is the Watson identity (already a theorem),
and the only correct Watson identity is the BCC-sublattice one
(`W_BCC = G*²/(2π)`). The cubic-G18 lattice that the engine actually
implements has a different Watson value (≈ 1.99/(2π) ≈ 0.317), which
does not connect to G* in any clean way the present analysis can
identify.

The lemniscate-circle analogy (§4) and the U(1)/S¹ rotation
structure observation (§2) remain as structural curiosities without
predictive content. **No new theorem candidate. No new derivation
direction. Q4 line CLOSED-NEGATIVE.**

This is the correct outcome of the discipline: hypothesis was
floated, sub-questions were opened with falsifiable tests, the
test was run, the hypothesis failed, the line is closed negative
**with the failure documented honestly**. CLAUDE.md anti-target
discipline preserved through every step.

### 11.5 · What we keep from this examination

Even though Q4 closed negative, three artifacts have lasting value:

1. **The catch of FTD-0117** (spine document G* typo). Discovered
   during Q4 examination, fixed across all 5 canonical-tier surface
   areas in commits 1fcd519 + 0ad116e.

2. **The clean separation of cubic-Watson vs BCC-Watson.** The
   engine's gauss-projection works on cubic-G18. The spine's
   Watson identity uses BCC-sublattice. These are different
   integrals; conflating them was the source of the falsified
   hypothesis. Future work should keep them clearly distinct.

3. **The numerical value Z_G18 ≈ 1.9917** is itself an engine-
   measurable structural constant of FTD's lattice. It has no
   closed form connecting to G*, but it is a real number that the
   engine produces. Filed for the record in case a future
   theoretical development connects it to other FTD constants.

---

## 12 · What this document does NOT claim

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
