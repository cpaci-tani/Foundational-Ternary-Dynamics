# PRE-REGISTRATION — Matched contact-energy obstruction

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0529`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only energy and Gauss audit of the FTD-0527 identical-contact
rebase inside the isolated matched-history field sector. No production state,
default, toggle, scenario, force, collision law, phase order, field ontology,
normalization, or tolerance change.

## 1. Registered question

FTD-0527 preserves the incoming relativistic matter-energy multiset exactly.
FTD-0528 proves that its complete physical worldline current `K` is
representation-independent and updates the selected face field as

```text
E_after = E_star - K.
```

With the frozen FTD-0478 normalization, the physical matched-field energy and
current work carry the common positive coefficient

```text
beta = C_WAVE^2 (G_C/C_WAVE^2)^2 = G_C^2/C_WAVE^2.
```

The exact electric energy change at the current-deposition substep is

```text
Delta H_field(E_star)
  = beta/2 (||E_star-K||^2-||E_star||^2)
  = -beta <K,E_star-K/2>.
```

The fixed elastic rebase has `Delta H_matter=0`. It can therefore close total
energy only on the codimension-one field set satisfying
`<K,E_star-K/2>=0`, unless another matter/dressing variable changes.

## 2. Baseline-independent transverse discriminator

Let `D` be the matched backward face divergence and `C` the matched edge-to-face
curl. Define

```text
F = C C^T K.
```

The selected complex gives

```text
D F = D C C^T K = 0,
<K,F> = <C^T K,C^T K>.
```

Thus `E_star` and `E_star+aF` have the same Gauss source, while

```text
Delta H_field(E_star+aF)-Delta H_field(E_star)
  = -beta a ||C^T K||^2.
```

Use the locked challenge amplitude `a=1/8`. Embed either deposition state in
the complete staggered source-free step by choosing

```text
B_before = lambda C^T E_star,
lambda = C_SPEED,
```

so the Faraday substep lands on `B_half=0`, the Ampere substep leaves
`E_star` unchanged, and the exact modified field energy reduces to the
quadratic expression above. This prevents the discriminator from depending on
setting the field propagation speed to zero.

## 3. Registered field existence and arms

For each contact history, neutralize its fractional periodic density with a
uniform stationary background. Construct one deterministic compatible face
field by routing each non-root source to a fixed root through shortest periodic
`x/y/z` paths. The route is only an existence witness; the challenged-minus-
baseline energy split above is independent of it.

Run both polarities, three translations, all 26 nonzero Moore directions, and
speeds `1/8` and `1/4` (`312` arms). Require:

1. the crossing and bounce exact histories agree below `1e-12`;
2. exact continuity and before/after absolute Gauss residuals below `1e-12`;
3. `D F` below `1e-12`;
4. the adjoint identity `<K,F>=||C^T K||^2` below `1e-12`;
5. the complete staggered embedding residual below `1e-12`;
6. baseline and challenged field-energy identities below `1e-12`;
7. the measured energy split agrees with
   `beta a ||C^T K||^2` below `1e-12`;
8. for every arm with `||C^T K||^2>1e-12`, at least one of the two elastic
   total-energy residuals is at least half the predicted split, up to
   `1e-12` roundoff;
9. translation, polarity-mirror magnitude, and cubic-orbit magnitude residuals
   below `1e-12` for the baseline-independent quantities;
10. invalid inputs fail closed.

## 4. Locked verdicts

- If every algebraic gate passes and at least one registered arm has
  `||C^T K||^2>1e-12` with predicted split above `1e-10`:
  `ELASTIC_CONTACT_CANNOT_COUPLE_RECIPROCALLY_WITHOUT_FIELD_DEPENDENT_MATTER_OR_DRESSING`.
- If `C^T K=0` on every arm and both compatible fields close:
  `CONTACT_CURRENT_IS_TRANSVERSE_BLIND_ELASTIC_ENERGY_OBSTRUCTION_ABSENT`.
- If Gauss, adjointness, the staggered embedding, or the exact energy formula
  fails:
  `MATCHED_CONTACT_ENERGY_AUDIT_UNRESOLVED`.

The first verdict is not a no-go against reciprocal mobile matter. It is a
no-go against composing the unchanged FTD-0527 elastic output with arbitrary
admissible matched fields. The next admissible construction must solve the
collision impulse, exact current, and field update simultaneously, or assign
the work to an explicit dressing/history degree of freedom.

## 5. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 before this execution annotation was
`6821AE535D777495621145CE9D8E5D33E34B0AB07C005B59F23F5F5AE2A24AFD`.

All `9/9` checks passed over 312 arms. `C^T K` vanished on all 72 axial arms
and was nonzero on all 240 edge/corner arms. The latter produced a compatible-
field energy split of `1.50305e-4..6.24141e-4`, while matter energy remained
exactly unchanged. The locked pass verdict applies:

```text
ELASTIC_CONTACT_CANNOT_COUPLE_RECIPROCALLY_WITHOUT_FIELD_DEPENDENT_MATTER_OR_DRESSING
```

Canonical result:
[`AUDIT_MATCHED_CONTACT_ENERGY_OBSTRUCTION.md`](../../07_assessment/AUDIT_MATCHED_CONTACT_ENERGY_OBSTRUCTION.md).
