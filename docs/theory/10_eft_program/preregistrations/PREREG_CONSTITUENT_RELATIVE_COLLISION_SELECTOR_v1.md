# PRE-REGISTRATION — Constituent-relative collision selector

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0512`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only audit of whether the already-associated carrier phase
space and the FTD-0507 boundary charts can support and uniquely solve a
restricted reciprocal collision transaction. No production rule, default,
toggle, scenario, tolerance, field normalization, or ontology changes.

## 1. Question and scope

FTD-0507 proves that a face/edge/corner boundary collision can be stored in
distinct existing `(site,remainder)` charts, but it manually supplies the
outgoing momenta. This campaign asks two separate questions:

1. does the existing chart normal plus constituent momentum admit an exact,
   local, reversible elastic collision map without a new hidden phase; and
2. can that map be derived from the aggregate trilinear face density/current
   used by the selected face-field coupling?

Only equal-mass, same-polarity, isolated, reaction-free, zero-COM-normal
collisions are in scope. General impact parameters, unequal masses,
collisions with reactions, and production integration are excluded.

## 2. Frozen candidate map

Let the retained collision charts have anchors `a1,a2` and common effective
position `x`. Define the oriented chart normal and phase-space variables

```text
n = (a2-a1)/|a2-a1|,
P = p1+p2,
q = (p1-p2)/2.
```

The registered collision inputs obey `P dot n = 0` and `q dot n > 0`
(incoming). The only admitted impulse class is an equal-and-opposite central
impulse,

```text
p1' = p1-lambda n,
p2' = p2+lambda n.
```

The nontrivial elastic/outgoing solution is frozen as

```text
lambda = 2(q dot n),
q' = q-2(q dot n)n.
```

This is a selected hard-contact premise, not a claimed consequence of the
face-field action. The null solution `lambda=0` is rejected only by the
registered outgoing/nonpenetration condition. The audit must report this
selection explicitly.

## 3. Registered fixtures

Use `L=17`, rest energy `0.511`, `C_SPEED=1/sqrt(3)`, both polarities,
three integer translations, every nonzero Moore chart direction, and speeds
`1/8` and `1/4`. Collision points are the FTD-0507 half-coordinate fixtures.

```text
26 directions x 2 polarities x 3 translations x 2 speeds = 312 arms.
```

The outgoing observation segment has length `1/4` along the chart normal.
All tolerances are fixed at `1e-12`.

## 4. Algebraic and geometric gates

For every arm require:

1. both retained charts represent the same effective collision position;
2. impulse sum, total momentum, total relativistic matter energy, and polarity
   residuals are below `1e-12`;
3. the impulse is parallel to the chart normal and the nontrivial algebraic
   solution has residual below `1e-12`;
4. relative normal momentum reverses sign while tangential relative momentum
   is unchanged;
5. the map is an involution, is time-reversal covariant, is local to the two
   charts, and is covariant under integer translations and the signed cubic
   group;
6. outgoing speeds remain below `C_SPEED` and exact face-current continuity
   stays below `1e-12`.

These gates establish a mathematically admissible selected collision map.
They do not establish that the native action selects it.

## 5. Aggregate face-action discriminator

For the six face-normal directions, compare:

```text
H0: two coincident static carriers,
H1: the same two carriers separating symmetrically along n.
```

Record the complete aggregate `(rho_before,rho_after,J_face)` signature and
the sum of the two constituent current norms. The face-action derivation gate
is negative if all of the following hold:

- aggregate H0/H1 signature residual is below `1e-12`;
- aggregate H1 current norm is below `1e-12`;
- constituent H1 current norm is strictly positive;
- H1 matter kinetic energy exceeds H0 by more than `1e-6`.

That counterexample proves that no deterministic interaction functional of
the aggregate trilinear density/current alone can distinguish the static and
relative modes or supply their energy difference. It does not prohibit a
constituent-level contact functional using already-associated phase space.

Edge and corner results are reported but do not decide this no-go because
their tensor-product shapes retain even cross terms.

## 6. Locked verdicts

- If the collision-map gates pass and the face discriminator is negative:
  `SELECTED_REFLECTION_EXISTS_FACE_ACTION_CANNOT_DERIVE_IT`.
- If the collision-map gates pass and the aggregate signature distinguishes
  every registered face arm:
  `FACE_ACTION_DERIVATION_REMAINS_OPEN`.
- If conservation, covariance, reversal, or causality fails:
  `EXISTING_PHASE_SPACE_CANNOT_SUPPORT_RESTRICTED_COLLISION_MAP`.

No successful verdict authorizes a production collision rule. A production
branch would require a separately registered common interaction functional
and would remain default-off.

## 7. Execution record

The locked body above had SHA256
`2DA607D150683A082F98730BABECEDC11B4B1F713996007AE6EDA7DFD44179E0`
before this execution section and status transition were appended. No gate,
fixture, tolerance, or verdict cell changed after the lock.

- test SHA256:
  `F973E0389807ABD92E8517F5041458AD606C78D9672CE1861ABD0E07984459F5`;
- header SHA256:
  `5CB254B04DA58BC330D049556433C2F78C2E62EC35A665D8908ABD38B313E15A`;
- implementation SHA256:
  `9B97E2F7DB5FF27D3D336B915796650851D1851ECCAC6A3209C1FD0BC549D8B7`;
- result: `5/5 PASS` across `312` registered arms;
- verdict:
  `SELECTED_REFLECTION_EXISTS_FACE_ACTION_CANNOT_DERIVE_IT`.
