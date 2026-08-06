# FTD-0620 — Balanced-gait internal phase-return test

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CONSTITUENT-HISTORY RUN]`
**Date:** 2026-07-27
**Production status:** unchanged

## 1. Question

The FTD-0618 net-neutral six-constituent state translates under a balanced
internal excitation.  FTD-0619 shows that this motion carries a fixed-lattice
continuous-translation defect.  The next independent question is whether the
motion is a repeatable internal gait or a one-time relaxation.

This protocol does not search for a favorable initial mode.  It reuses exactly
the FTD-0618 rest, positive-gait, and negative-gait initial states.

## 2. Locked parents and dynamics

- FTD-0618 result SHA-256:
  `5F04E64DFD7CBFD10CE3AC779361C4124654C817320DFC81E6D5A482889F54D3`.
- FTD-0619 result SHA-256:
  `0FEE2158E3DCB5EED2F837D74E89127F4B01160335057115F095FDF3C724669D`.
- Volume: the inherited `L=17` CPU observer lattice.
- Rest arm: 128 forward and 128 reverse ticks.
- Active signs: 512 forward and 512 reverse ticks each.
- Common-action options, rest-state reconstruction, excitation amplitude,
  charge-conjugate half-turn, field initialization, binding, and tolerances are
  unchanged from FTD-0618.
- No legacy force, production toggle, scenario, fitted momentum, damping,
  phase reset, favorable-time selection, or post-hoc correction is admitted.

## 3. Locked internal-state observer

For each three-constituent core, unwrap the compact constituent positions about
that core's instantaneous centre using the periodic shortest displacement.
Define

```text
r_a(t)  = x_a(t) - X_core(t),
pi_a(t) = p_a(t) - P_core(t)/3.
```

The matter-internal return residuals are

```text
d_x(t) = max over all six constituents |r_a(t)-r_a(0)|,
d_p(t) = max over all six constituents |pi_a(t)-pi_a(0)|,
D(t)   = max(d_x(t),d_p(t))/A,
```

where `A` is the already fixed FTD-0618 excitation amplitude.  No fitted
metric or field variable enters this matter-internal diagnostic.

The observer also projects core-A internal momentum and shape displacement on
the two locked FTD-0615 rotational patterns.  The momentum-plane phase is

```text
theta(t) = unwrap atan2(P_mode1(t), P_mode0(t)).
```

This angle is descriptive.  A winding is not by itself a return.

## 4. Return and transport rules

A post-launch return event is a temporal local minimum of `D(t)` at `t>=32`
with

```text
d_x <= A/20,
d_p <= A/20.
```

Consecutive sub-threshold samples count as one event, represented by their
minimum.  A sign arm is recurrent only if it has at least two post-launch
events and the two cycle lengths

```text
T1 = t_return,1,
T2 = t_return,2 - t_return,1
```

agree within 10 percent of their mean.

Persistent transport is evaluated without fitting: every fixed 128-tick
window must advance at least `0.5` cell in absolute axial displacement.  A
one-time relaxation requires no recurrent return, final internal-momentum norm
below 10 percent of its initial value, and last-window axial displacement below
`0.1` cell.

The two active signs must mirror at every common tick: pair displacement is
negated and `d_x,d_p` agree, all within `1e-8`.

## 5. Algebraic gates

Every executed step must retain:

- common-action residual at most `1e-12`;
- total-energy drift at most `1e-10`;
- minimum internal distance at least `0.5` and maximum at most `2.0`;
- chart multiplicity at most two;
- state-only forward/reverse recovery at most `1e-8`.

Failure of an algebraic gate yields only
`BALANCED_GAIT_PHASE_RETURN_NUMERICALLY_UNRESOLVED`.

## 6. Locked verdict map

1. Both active signs recurrent, persistently translating, and sign-mirrored:
   `RECURRENT_INTERNAL_GAIT_TRANSLATOR`.
2. Both active signs satisfy the one-time-relaxation rule:
   `BALANCED_GAIT_ONE_TIME_RELAXATION`.
3. Both active signs persistently translate but do not satisfy recurrence:
   `PHASE_RETURN_NOT_OBSERVED_PERSISTENT_GAIT`.
4. Algebra passes but the arms split or satisfy none of the above:
   `BALANCED_GAIT_PHASE_BEHAVIOR_MIXED`.
5. Any algebraic failure:
   `BALANCED_GAIT_PHASE_RETURN_NUMERICALLY_UNRESOLVED`.

Finite non-observation does not prove nonrecurrence.  A positive matter-
internal return does not prove full field-state recurrence, isolated momentum,
a particle pole, Lorentz recovery, or a physical particle.

