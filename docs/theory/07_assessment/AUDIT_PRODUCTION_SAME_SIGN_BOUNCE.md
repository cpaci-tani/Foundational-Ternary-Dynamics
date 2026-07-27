# AUDIT — Production same-sign bounce reciprocity

**Date:** 2026-07-25  
**Identifier:** `FTD-0506`  
**Status:** `[MEASURED — EXACT PRODUCTION AXIS-FLIP/RESET CONTRACT]` +
`[CLOSED NEGATIVE — RECIPROCAL ELASTIC COLLISION]` +
`[CLOSED NEGATIVE — FACECURRENT-COMPATIBLE FINITE-RANGE EXCLUSION]`  
**Verdict:**
`PRODUCTION_BOUNCE_IS_FIXED_TARGET_RESET_NOT_RECIPROCAL_COLLISION`  
**Pre-registration:**
[`PREREG_PRODUCTION_SAME_SIGN_BOUNCE_RECIPROCITY_v1.md`](../10_eft_program/preregistrations/PREREG_PRODUCTION_SAME_SIGN_BOUNCE_RECIPROCITY_v1.md)  
**Run of record:** `engine/results/ftd_0506/windows_msvc_cpu.json`

**Successor note (FTD-0507):** FTD-0505's unconditional capacity/range/phase
trilemma is retracted. Existing chart multiplicity stores face/edge/corner
collision slices exactly and can carry a manually selected reciprocal outgoing
phase. This does not rescue the production bounce measured here: it still
reflects only the mover, erases remainder, omits target/field recoil and
current, and fails inversion.

## 1. Measured rule

The actual CPU `phase_movement` branch was executed with movement as the only
enabled term. A moving manifested voxel attempted to enter an already occupied
same-sign Moore target. Across all 26 directions, both polarities, and three
integer translations (156 arms), production did exactly this:

```text
source state       unchanged
target state       unchanged
source velocity    flip every attempted-hop axis
source remainder   set to zero
target phase space unchanged
field state        unchanged
history events     zero.
```

The maximum discrepancy from this source-level contract was zero. Therefore
the comments correctly describe the implementation. The defect is the physics
classification "elastic bounce," not comment/code drift.

## 2. It is not the finite-range specular escape of FTD-0505

The locked source begins at subcell remainder `0.80d` with velocity `0.25d`.
Its proposed endpoint is `1.05d`. Reflection at the occupied target position
`d` gives

```text
r_spec=2d-1.05d=0.95d.
```

Production instead sets `r_after=0`. The minimum componentwise mismatch is
exactly `0.95`. Under the face-flux mainline's effective position
`x_eff=site+remainder`, the reset produces a net displacement of magnitude at
least `0.8` in one tick. Even the axial arm therefore exceeds `C_SPEED` by

```text
0.8-C_SPEED = 0.22264973081037409.
```

The correct specular broken path

```text
x+0.80d -> x+d -> x+0.95d
```

uses exactly the available travel time and has zero causal residual in all
face, edge, and corner arms. Hence the production reset is not a coordinate
encoding of specular reflection. It is a physical discontinuity under the
selected `site+remainder` interpretation.

## 3. Energy is preserved but pair momentum is not

Axis sign flip preserves `|v|`, so the production-dispersion pair-energy
residual is exactly zero. The stationary occupied target is unchanged,
however. The total matter momentum therefore changes by

```text
Delta P = p_after-p_before = -2 p_mover
```

for the locked collinear mover. The smallest measured defect, in the axial
arms, is `0.28345180027109212`; edge and corner defects are larger. Every field
component remains exactly zero, so no field recoil balances the change.

This is the momentum law of reflection from an infinitely massive external
wall. Production does not contain such a wall primitive: the target is another
finite manifested voxel with its own velocity and attributes. Therefore the
branch is not a reciprocal two-body elastic collision.

## 4. Site continuity hides a missing subcell current

Site occupancy is unchanged, so the existing snapshot continuity extractor
classifies same-sign bounce as a no-op. That statement is true only at anchor
resolution.

The FTD-0478 shape changes from remainder `0.80d` to production remainder
zero. Exact continuity therefore requires a nonzero oriented face current. If
the journal supplies zero current, the minimum local continuity defect is
`0.8`. The exact production-endpoint current closes continuity to
`2.22e-16`, but it differs from the exact specular-bounce current by at least
`0.95`. No same-sign event is currently journaled, so neither current is
available to a reciprocal field transaction.

Thus "no site transport" does not imply "no coupling current" after subcell
position is taken seriously.

## 5. The unchanged tick is not an inverse

After the first tick the source has zero remainder and reversed velocity. On
the next unchanged tick it simply accumulates remainder away from the target;
it does not re-enter the collision branch. The minimum raw phase-space
distance from the registered initial source is `1.05`.

Energy preservation is therefore not evidence of a reversible collision map.
The remainder reset erases the collision phase required by the inverse.

## 6. Consequence

The current production same-sign rule is a selected exclusion heuristic with
fixed-target reflection and subcell erasure. It cannot serve as the FTD-0481
atomic matter-field transaction, the FTD-0505 conservative finite-range
escape, or evidence for emergent reciprocal mobile matter.

A future repair must be separately preregistered because each admissible
choice changes dynamics: specularly retain the subcell phase, exchange target
momentum, deposit field recoil/current, or introduce a different collision
state. FTD-0506 does not authorize any one of those changes.

No production code, default, toggle, or scenario was modified.

- checks: `5/5 PASS`;
- test SHA256:
  `1BE727708850748B91D9699A3C967F8462B1D1C3584F92746575DF1F6E1AAE95`;
- header SHA256:
  `D6C36B535298FDD2ADC0AE49FA44230FE9E51109A35FB02106BD5D61AD522845`;
- observer implementation SHA256:
  `2D1211069DE48E40846135F813CDF162618796E270A73C84DF60FB942C271E3E`;
- audited production phase SHA256:
  `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU engine measurement;
- production state and defaults: unchanged.
