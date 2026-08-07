# AUDIT — Boundary collision resolution trilemma

**Date:** 2026-07-25  
**Identifier:** `FTD-0505`  
**Status:** `[THEOREM — ZERO-POSTTIME SEPARATION OBSTRUCTION]` +
`[THEOREM — SINGLE-ANCHOR COLLISION-CAPACITY LOWER BOUND]` +
`[CONSTRUCTIVE — SELECTED FINITE-RANGE EXCLUSION FAMILY]` +
`[RETRACTED — UNCONDITIONAL CAPACITY/RANGE/PHASE TRILEMMA; FTD-0507]`  
**Verdict:** `SUPERSEDED_BY_FTD_0507_BOUNDARY_CHART_CAPACITY`  
**Pre-registration:**
[`PREREG_BOUNDARY_COLLISION_RESOLUTION_TRILEMMA_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_BOUNDARY_COLLISION_RESOLUTION_TRILEMMA_v1.md)  
**Run of record:** `engine/results/ftd_0505/windows_msvc_cpu.json`

## 0. Successor correction — FTD-0507

The zero-posttime calculation and the selected finite-radius family below
remain correct. The unconditional storage premise does not. FTD-0505 assumed
that coincident same-sign carriers must be written to one anchor. FTD-0507
proves that the frozen `(site,remainder)` state has `2`, `4`, or `8` distinct
stable anchors at face, edge, or corner collision points and can store two
same-sign carriers without expanding the ternary alphabet. Actual CPU
movement then carries and reverses a manually selected outgoing phase exactly.

The five-symbol lower bound now applies only at a lattice knot, after forced
canonical single-anchor write-back, or whenever multiplicity exceeds the
retained chart count. The former verdict
`BOUNDARY_COLLISION_REQUIRES_CAPACITY_RANGE_OR_PHASE` is retracted. The
surviving open requirement is an explicit collision impulse; FTD-0507 also
shows that aggregate trilinear face current loses the axial relative mode that
would be needed to derive that impulse from the field alone.

## 1. Exact boundary event

Two equal-mass same-sign carriers start at

```text
x_L=c-a*n, x_R=c+a*n,
v_L=+v*n, v_R=-v*n,
a=0.25, v=0.25, dt=1.
```

Their unique intersection time is

```text
tau=a/v=1=dt.
```

The event is therefore not an FTD-0504 interior crossing. It occurs exactly at
the time slice on which the next production state must be representable.

## 2. Separated output cannot occur on the same slice

Suppose either outgoing carrier is required to appear a positive distance
`b` beyond the collision vertex at speed `u<=C_SPEED`. The required time is

```text
T=a/v+b/u=dt+b/u.
```

For every `b>0`, `T>dt`. This is independent of force strength: the collision
has no post-vertex time remaining inside the registered step. The eight locked
attempts used `b={1/64,1/32,1/16,1/8}` and `u={v,C_SPEED}`. Their smallest
temporal causal defect was

```text
1/(64*C_SPEED) = 0.027063293868263782 ticks.
```

Thus an instantaneous output convention cannot write two separated carriers
at the same tick without moving the event earlier, extending the step, or
allowing super-causal propagation.

## 3. A canonical single-anchor endpoint exceeds the ternary alphabet

If zero output distance is additionally forced onto one canonical anchor, both
same-sign unit polarities occupy that site. This requires local charge `+2` or
`-2`, while the primitive site has only `{-1,0,+1}`. The exact single-anchor
defect is one.

Accommodating both signs and both double occupancies in one charge-faithful
site alphabet requires at least

```text
{-2,-1,0,+1,+2}: five symbols.
```

A product extension of the existing ternary state therefore needs at least
one additional binary occupancy flag. This is only a charge-capacity lower
bound. It does not yet store two momenta, colors, spins, flavors, or other
worldline attributes.

The bound is conditional both on the face-current mainline's identification of
each manifested polarity with one transported unit and on canonical
single-anchor write-back. FTD-0507 removes the latter condition for non-knot
boundary points by retaining distinct raw charts. This is not a claim that
native production `sum(s)` is conserved under reaction toggles.

## 4. Finite-range exclusion is constructive but selected

A hard-core encounter can happen before the boundary. For radius `0<r<a`,
each carrier reflects at `c +/- r*n` at

```text
tau_r=(a-r)/v < dt
```

and uses the remaining `r/v` to end at `c +/- 2r*n`. This supplies two
separated ternary endpoints without changing speed. The radii

```text
r={a/4,a/2,3a/4}
```

were tested for both polarities under all 48 signed cubic maps and three
integer translations: 864 exclusion arms. Every arm preserved energy,
momentum, charge, causality, exact continuity, covariance, and reversal:

```text
worst conservation/continuity residual  4.68375338513738e-17
worst reversal residual                 1.73472347597681e-17.
```

This is a valid family of selected dynamics, not a derivation. The three radii
produce distinct exact charge/current histories; the minimum pairwise
signature difference is `0.036830357142856984`. Conservation and the frozen
zero-radius collision data do not choose `r`.

## 5. Moving the event off the tick changes the physical state

With the initial separation fixed, moving the symmetric collision to
`dt-delta` or `dt+delta` requires

```text
v_early=a/(dt-delta),
v_late =a/(dt+delta).
```

For `delta={0.1,0.2,0.3}`, every early speed remained causal, but every early
or late speed changed the production-dispersion pair energy. The smallest
absolute energy change was `0.022044953189828354` in registered energy units.

Therefore a timing shift is not a representation-only repair. It changes the
incoming momenta/energy, moves the time slicing, or requires an additional
subtick phase variable.

## 6. Conditional single-anchor trilemma

For the zero-range fixture plus canonical single-anchor write-back:

1. positive same-tick separation is temporally impossible;
2. zero separation exceeds the ternary charge alphabet;
3. moving contact earlier requires either a finite exclusion radius or changed
   kinematics/time phase.

Under that additional write-back condition, the event requires at least one of:

```text
additional collision-state capacity,
a selected nonzero interaction range,
an additional/changed temporal phase.
```

The result does not choose among them. FTD-0507 proves that the actual frozen
noncanonical site/remainder representation evades the capacity horn at
face/edge/corner points, so this paragraph no longer closes representation-only
boundary storage in general.

## 7. Consequence

An event-native implementation may continue quotienting identical crossings.
At a non-knot stored boundary slice, distinct existing charts supply exact raw
capacity and can carry an outward continuation; at a knot or after
canonicalization, the overload remains. A future constructive branch must
preregister the collision impulse and then re-run the common-action,
exact-current, reversal, and multibody gates. If it chooses canonicalization,
finite exclusion, or temporal staggering, those remain explicit additions.

No production collision code, occupancy field, force, toggle, scenario, or
timing rule was added.

- checks: `7/7 PASS`;
- test SHA256:
  `87AB88DE847A0B84921A480585C2B57B7774DE45A55FEFBFCD0F589CE1669887`;
- header SHA256:
  `48CEE0E44CE19983EEB08BF593AD35F7C1D37F8FBD66E373FE3988470EA7A9C8`;
- implementation SHA256:
  `52FF15A7CEB5B0253F5A8DF2E35254D8E28CBF46FC21B31C6B09BACE19927497`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
