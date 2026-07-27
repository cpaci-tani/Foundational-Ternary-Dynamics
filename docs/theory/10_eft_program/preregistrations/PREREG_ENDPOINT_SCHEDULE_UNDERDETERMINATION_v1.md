# PRE-REGISTRATION — Endpoint schedule underdetermination

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0549`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0542`, `FTD-0547`, `FTD-0548`  
**Scope:** observer-only theorem witness. Production state, tick, current,
force, energy, toggle, default, and scenarios remain unchanged.

## 1. Locked schedules

For the same oriented spatial segment `x=x0+d n s(tau)`, compare

```text
s0(tau)=tau,
se(tau)=tau+epsilon tau^2(1-tau)^2.               (1)
```

Use `0<|epsilon|<=1/2`. Both schedules must have identical start, end,
start derivative, midpoint derivative, and end derivative. Monotonicity must
follow from

```text
min se'(tau)=1-|epsilon|/(3 sqrt(3))>0.           (2)
```

Thus the schedules share endpoint positions, endpoint velocities, midpoint
velocity, duration, displacement, endpoint kinetic energies, and total
oriented face current.

## 2. Locked exact moments

Using partition and first-moment reproduction of the quadratic coat, prove

```text
integral_0^1 tau^2(1-tau)^2 d tau=1/30,

Delta mu_T  =q d n epsilon/30,
Delta sum K0=q d n epsilon/30,
Delta sum K1=-q d n epsilon/30,
Delta sum K =0.                                   (3)
```

Require endpoint/derivative, recombination, total-current, and analytic
moment residuals below `1e-14`. At least one split difference must exceed
`1e-8`.

Under time reversal, the schedule family must map `epsilon -> -epsilon`, with
the moment differences transforming accordingly, below `1e-14`.

## 3. Locked arms and verdicts

Use displacements `0.05,0.10,0.20`, epsilon values
`-0.5,-0.25,+0.25,+0.5`, both polarities, and directions
`<100>,<010>,<001>,<111>`: `96` arms plus invalid controls.

- all identities close and a nonzero split is proved:
  `ENDPOINT_DATA_DO_NOT_DETERMINE_SPACETIME_CURRENT`;
- any exact identity fails:
  `ENDPOINT_SCHEDULE_UNDERDETERMINATION_UNRESOLVED`.

A positive theorem closes only endpoint-only or endpoint-plus-midpoint
reconstruction of `K0,K1,T`. It does not prove that a self-consistent atomic
solve is impossible: a force history or internal stage solution may determine
the schedule without adding a new ontological primitive.
