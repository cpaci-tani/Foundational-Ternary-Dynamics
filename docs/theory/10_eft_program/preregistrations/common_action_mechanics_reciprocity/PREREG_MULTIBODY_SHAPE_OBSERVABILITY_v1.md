# PRE-REGISTRATION — Multibody shape observability v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0501`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0484`, `FTD-0498`, `FTD-0500`

## Question

Can the additive trilinear polarity field and its exact oriented face current
serve as a complete quotient state for multiple manifested objects, without
individual worldline records or additional multipole variables?

## Locked algebraic theorem

Restrict particles to one open lattice cell on the x axis with integer y and
z. For charges `q_i` at fractional positions `f_i in [0,1]`, CIC deposition is

```text
rho_0 = sum_i q_i(1-f_i) = Q-M,
rho_1 = sum_i q_i f_i     = M,
Q = sum_i q_i,
M = sum_i q_i f_i.
```

Therefore the complete deposited site field factors through only `(Q,M)`.
Any two configurations with equal signed charge and signed first moment have
identical trilinear polarity. Under matched straight worldlines, sum the exact
FTD-0478 face currents particle by particle and test the resulting aggregate
continuity directly.

## Locked same-polarity kernel

Embed at y=z=8:

```text
A+: x={8.25, 8.75}, q={+1,+1},
B+: x={8.375,8.625}, q={+1,+1}.
```

Both have `Q=2` and the same first moment. Require their deposited density to
match below `1e-12`, while their squared pair separations differ exactly by

```text
(0.50)^2-(0.25)^2=3/16.
```

Translate every particle by `+0.05 e_x`. Require the aggregate endpoint
densities and exact face currents to remain identical and continuous below
`1e-12`.

## Locked neutral-location kernel

Use

```text
A0: (+1 at 8.35, -1 at 8.65),
B0: (+1 at 8.45, -1 at 8.75).
```

Both have `Q=0` and the same signed first moment, but their unsigned centers
differ by `0.10`. Translate each rigid pair by `+0.05 e_x`. Require identical
aggregate `rho/current` histories and test whether the total face current is
zero even though both constituent worldlines move.

## Locked vacuum-kernel control

Compare vacuum with a coincident `(+1,-1)` pair at `x=8.4`. This algebraic
control is not claimed to be an allowed frozen ternary state. It tests whether
the signed additive shape alone can distinguish a pre-reaction neutral pair
from absence.

## Locked covariance and raw-state checks

- Repeat the same-polarity and neutral degeneracies under all 48 signed cubic
  maps about knot `(8,8,8)` and integer translations that keep support inside
  `L=17`.
- Confirm each compared pair has the same primitive ternary anchor pattern but
  different per-voxel remainders. This isolates exactly what shape aggregation
  erases.
- Record charge, signed first moment, unsigned center, squared separation,
  aggregate continuity, and constituent-current norm.

## Frozen verdicts

- `SHAPE_CURRENT_COMPLETE_FOR_MULTIBODY` only if every distinct locked
  configuration is distinguished by total polarity or face-current history.
- `SHAPE_CURRENT_REQUIRES_WORLDLINE_DECOMPOSITION` if exact equal histories
  coexist with different separation, neutral location, or vacuum content.
- `KERNEL_IS_NUMERICAL_OR_CHART_ARTIFACT` if any equality fails at `1e-12`,
  does not survive cubic maps/translations, or arises from invalid individual
  shape/current records.

## Scope ceiling

This is observer-only. It does not authorize a particle list, a multipole
hierarchy, collision rules, reactions, a toggle, or a scenario. The result
qualifies whether the remaining shape/quotient route is already a complete
ontology; it does not reject trilinear shape as a one-particle coupling rule.

## Run-of-record hashes

- test SHA256:
  `CDDAF8B0E272B6D94F1BA29387FA1E27995D907E16D54C8CC5FE571C1B6E2ABA`;
- header SHA256:
  `2B72991B2B5189E957C1811B5AD83391AFEE8CFF14BE007324D7F2825C29B5D4`;
- implementation SHA256:
  `C01C2F7575548D1FEDE7786D78E35EAF6A2657F0C3DD4C75573B720112EBEF7C`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- record: `engine/results/ftd_0501/windows_msvc_cpu.json`.
