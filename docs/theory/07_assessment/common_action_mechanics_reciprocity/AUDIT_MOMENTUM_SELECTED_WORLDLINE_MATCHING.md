# AUDIT — Momentum-selected worldline matching

**Date:** 2026-07-25  
**Identifier:** `FTD-0503`  
**Status:** `[THEOREM — DISTINCT-ENDPOINT FREE MATCHING UNIQUENESS]` +
`[CONSTRUCTIVE — PHASE-SPACE-SELECTED CURRENT 1-CHAIN]` +
`[OPEN — COINCIDENT-TARGET COLLISION]`  
**Verdict:** `PHASE_SPACE_SELECTS_FREE_WORLDLINE_CHAIN`  
**Pre-registration:**
[`PREREG_MOMENTUM_SELECTED_WORLDLINE_MATCHING_v1.md`](../10_eft_program/preregistrations/PREREG_MOMENTUM_SELECTED_WORLDLINE_MATCHING_v1.md)  
**Run of record:** `engine/results/ftd_0503/windows_msvc_cpu.json`

## 1. Free phase space determines a target

For the production dispersion, the FTD-0490 free discrete Legendre map is
one-to-one inside the causal cone:

```text
d(p)=lambda c p/sqrt(E_REST^2+c^2|p|^2),
|d|<lambda.
```

Hence every associated start record `(x_i,p_i)` predicts one physical endpoint

```text
y_i=x_i+d(p_i).
```

If the unordered endpoint multiset contains every predicted `y_i` exactly once,
there is one and only one compatible permutation. This is a direct injectivity
theorem: a second exact matching would have to assign some start to a different
endpoint equal to the same predicted point, contradicting distinctness.

No persistent distinguishable-particle label is required. The association of
position and momentum at the start of the tick supplies the necessary local
phase-space record.

## 2. Exact Legendre permutation matcher

The observer does not use nearest distance. For each endpoint permutation it
evaluates the exact zero-field FTD-0490 discrete Legendre segment and scores

```text
R=max_i |p_d(x_i,y_sigma(i))-p_i|_infinity.
```

The four-carrier square has 24 permutations; nine are causal strict-interior
candidates and the rest contain rejected diagonals. In the static, clockwise,
and counterclockwise arms, exactly one candidate has `R=0`:

```text
zero momentum   -> stationary 1-chain,
CW momenta      -> clockwise 1-chain,
CCW momenta     -> counterclockwise 1-chain.
```

The minimum gap to every other valid permutation is
`1.5330000000000001` in the registered momentum units.

## 3. The selected current is exact

After matching, the observer reconstructs signed constituent worldlines and
deposits the FTD-0478/0484 current. Against the intended FTD-0502 histories:

```text
selected current residual       0
aggregate continuity residual   0
causal residual                 0.
```

Thus the large endpoint-current kernel of FTD-0502 is not an obstruction in
the isolated free phase-space sector. Existing pre-movement kinematics selects
one point in that kernel before any information-erasing snapshot update occurs.

## 4. Cubic covariance

Static, CW, and CCW matching were repeated under all 48 signed cubic maps and
three integer translations, for 432 arms. Every arm evaluated all 24
permutations, found one exact assignment, and reconstructed the transformed
current with zero residual.

## 5. Relation to production velocity

`Voxel` stores velocity rather than the observer's canonical momentum. Inside
the causal free sector, the production dispersion maps velocity/displacement
and momentum bijectively. Therefore the matching information is already present
in the associated `(occupied voxel, remainder, velocity)` record; no new
persistent momentum or `particle_id` is needed for this restricted result.

This does not repair the raw `(site,remainder)` write-back obstruction proved
by FTD-0497/0500. It selects the physical path and current. A production matter
representation must still decide how the physical endpoint is manifested.

## 6. Collision is now the sharp boundary

Two carriers starting at `x=8.25` and `x=8.75` were assigned exact opposing
momenta whose free targets are both `x=8.50`. The dispersion inversion reaches
the common target with residual zero. The matcher returns

```text
COLLISION_RULE_REQUIRED
```

before permutation selection because a ternary manifested endpoint cannot hold
two carriers. It does not deduplicate, bounce, annihilate, or discard either
worldline.

Thus free transport is not underdetermined; the next missing law is the local
collision transaction at coincident targets. That law must map the incoming
phase-space/worldline data to valid outgoing manifestation, current, momentum,
and field records while preserving the required invariants.

## 7. Plan consequence and reproducibility

The native-first interface can now be narrowed:

```text
input:   associated position + velocity/momentum + field,
solve:   physical endpoint and oriented current atomically,
write:   only after the transaction is fixed,
branch:  explicit collision/reaction solver when targets coincide.
```

This is constructive evidence for the event-native route, but it remains an
observer. Interacting-field uniqueness, raw manifested write-back, and collision
semantics remain open.

- checks: `6/6 PASS`;
- test SHA256:
  `52D5A79FC4F8ED13716D1791C7ABFB7BC7ECABAD959467D7B224837D299EF507`;
- header SHA256:
  `09EAF99234FFE723897C283B8BDEA75ABA4523C8B1DDE760F43A29CC65C67CC0`;
- implementation SHA256:
  `B802879D40E818AD7040281569C57F505C8DF09B30B9836FC3DFA2022560D90B`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
