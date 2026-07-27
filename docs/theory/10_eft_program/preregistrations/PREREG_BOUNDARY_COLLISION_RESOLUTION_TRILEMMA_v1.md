# PRE-REGISTRATION — Boundary collision resolution trilemma v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0505`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0497`, `FTD-0499`, `FTD-0503`, `FTD-0504`

## Question

Can a same-sign two-carrier collision that occurs exactly at a stored tick be
resolved by the frozen ternary site/remainder/velocity representation while
preserving exact charge, kinematics, locality, causality, covariance, and
reversal, or must one add capacity, an interaction range, or a time phase?

## Locked boundary fixture

Use `L=17`, center `(8.5,8.5,8.5)`, unit direction proportional to `(1,2,3)`,
half-separation `a=0.25`, `dt=1`, and speed `v=a/dt=0.25`. Two equal-mass,
same-sign carriers start at `c +/- a n` with inward momenta from the production
dispersion. Their unique intersection is `c` at `tau=dt`.

Run both polarities, all 48 signed cubic maps, and three integer translations.
The tolerance is `1e-12`.

## Locked zero-time separation theorem

For an output a distance `b>0` beyond the vertex at speed `u<=C_SPEED`, the
required time is

```text
tau_out = a/v + b/u = dt + b/u > dt.
```

Thus no positive separated output belongs to the same tick. Test
`b in {1/64,1/32,1/16,1/8}` and `u in {v,C_SPEED}`. The exact temporal causal
defect is `b/u`. At `b=0`, both carriers occupy the common endpoint and the
FTD-0504 same-sign ternary charge defect is exactly one.

## Locked storage lower bound

One primitive ternary site stores charges `{-1,0,+1}`. Representing one
collision snapshot for both polarities additionally requires `-2` and `+2`,
so a charge-faithful single-site alphabet needs at least five symbols. A
product extension of the existing ternary state therefore needs at least one
additional binary occupancy flag. This charge bound does not yet store two
independent momenta or intrinsic-attribute records.

## Locked pre-contact exclusion family

Test hard-core radii

```text
r in {a/4,a/2,3a/4}.
```

At radius `r`, the carriers reverse at `c +/- r n` at time `(a-r)/v`, then
end at `c +/- 2r n`. Require exact energy, momentum, charge, causality,
continuity, cubic covariance, and reversal below `1e-12`. Require the exact
piecewise face-current signatures for distinct radii to differ. Passing this
arm is constructive only for a selected finite-range exclusion law; it does
not derive `r` from zero-radius contact or from conservation.

## Locked timing-shift family

Keep the same starting separation and move the symmetric collision off the
tick by `delta in {0.1,0.2,0.3}`. The required early/late speeds are

```text
v_early=a/(dt-delta),
v_late =a/(dt+delta).
```

Compute their production-dispersion energy shifts from the registered
boundary fixture. Require every nonzero `delta` to change the kinetic energy;
`v_early` must remain causal in the registered range. Therefore timing
staggering is not a representation-only repair: it changes momenta/energy, or
equivalently introduces a new temporal phase/slicing datum.

## Locked verdicts

- `BOUNDARY_COLLISION_REQUIRES_CAPACITY_RANGE_OR_PHASE` if same-tick separation
  is impossible, the common endpoint exceeds ternary capacity, and the two
  constructive escape families require respectively a nonzero interaction
  radius or changed timing/energy.
- `FROZEN_TERNARY_BOUNDARY_COLLISION_RESOLVED` only if one registered arm
  supplies separated same-tick output with zero causal defect and no extra
  parameter/state.
- `BOUNDARY_COLLISION_INCONSISTENT` if even the selected pre-contact exclusion
  controls fail conservation, continuity, covariance, or reversal.

## Scope ceiling

This is observer-only. It does not authorize multi-occupancy, hard-core force,
substepping, variable time, collision code, a toggle, or a scenario. It tests
the minimal exact alternatives exposed by FTD-0504.

## Run-of-record hashes

- test SHA256:
  `87AB88DE847A0B84921A480585C2B57B7774DE45A55FEFBFCD0F589CE1669887`;
- header SHA256:
  `48CEE0E44CE19983EEB08BF593AD35F7C1D37F8FBD66E373FE3988470EA7A9C8`;
- implementation SHA256:
  `52FF15A7CEB5B0253F5A8DF2E35254D8E28CBF46FC21B31C6B09BACE19927497`.
