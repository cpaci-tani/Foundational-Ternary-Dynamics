# PRE-REGISTRATION — Momentum-selected worldline matching v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0503`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0490`, `FTD-0501`, `FTD-0502`

## Question

Can the existing per-manifestation position and velocity/momentum select the
otherwise-underdetermined transport 1-chain uniquely in the isolated free
multibody sector, without persistent distinguishable-particle labels?

## Locked matching rule

Use the exact free FTD-0490 discrete Legendre relation with

```text
E_REST=0.511,
C_SPEED=1/sqrt(3),
lambda=C_SPEED*dt,
dt=1.
```

For starts `(x_i,p_i)` and an unordered set of distinct endpoints `{y_j}`,
enumerate every permutation `sigma`. For each valid causal, strict-interior
segment evaluate the exact FTD-0490 kinetic start momentum

```text
p_d(x_i,y_sigma(i)).
```

Score the candidate by

```text
R(sigma)=max_i |p_d-p_i|_infinity.
```

A matching is exact when `R<=1e-12`. No distance-only fallback or fitted
penalty is allowed.

## Locked square histories

Reuse the four FTD-0502 square positions and its static, CW, and CCW endpoint
permutations. For each arm, obtain incoming momenta from the exact Legendre map
of that intended history, then discard its pairing and supply only:

- the associated start position-momentum records;
- the common unordered endpoint set `{A,B,C,D}`.

Require all 24 permutations to be evaluated and exactly one admissible match:

```text
static -> identity,
CW     -> (A->B,B->C,C->D,D->A),
CCW    -> reverse cycle.
```

Reconstruct each selected aggregate current and require exact agreement with
the corresponding FTD-0502 history below `1e-12`.

## Locked covariance arms

Repeat all three selections under all 48 signed cubic maps and the three
integer translations from FTD-0502, giving 432 matched arms. Require:

- one exact matching per arm;
- transformed assignment equal to the transformed intended history;
- selected-current residual, Legendre residual, and causal residual below
  `1e-12`;
- a strictly positive gap between the exact match and every other valid
  permutation.

## Locked collision control

Use two starts at `x=8.25` and `x=8.75` with exact free momenta that send both
to `x=8.50`. The intended endpoint multiset contains the same position twice.
Record that the phase-space map predicts two worldlines but a ternary site
snapshot cannot represent their coincident manifested endpoint. This arm must
return `COLLISION_RULE_REQUIRED`, not silently deduplicate, annihilate, bounce,
or choose one carrier.

## Frozen verdicts

- `PHASE_SPACE_SELECTS_FREE_WORLDLINE_CHAIN` if every distinct-endpoint arm
  has one exact match and its current is recovered, while the coincident arm
  fails explicitly into the collision gate.
- `FREE_MATCHING_REMAINS_AMBIGUOUS` if more than one distinct-endpoint
  permutation is exact for any locked arm.
- `MOMENTUM_MATCHING_INCOMPATIBLE_WITH_FACE_CURRENT` if the selected match
  fails to reconstruct the registered current or cubic covariance.

## Scope ceiling

This is observer-only. It does not authorize a production matcher, particle
labels, collision behavior, an interacting field branch, a toggle, or a
scenario. The claim is restricted to free, isolated, strict-interior segments
with distinct endpoints. Existing `particle_id` is not used as matching input.

## Run-of-record hashes

- test SHA256:
  `52D5A79FC4F8ED13716D1791C7ABFB7BC7ECABAD959467D7B224837D299EF507`;
- header SHA256:
  `09EAF99234FFE723897C283B8BDEA75ABA4523C8B1DDE760F43A29CC65C67CC0`;
- implementation SHA256:
  `B802879D40E818AD7040281569C57F505C8DF09B30B9836FC3DFA2022560D90B`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- record: `engine/results/ftd_0503/windows_msvc_cpu.json`.
