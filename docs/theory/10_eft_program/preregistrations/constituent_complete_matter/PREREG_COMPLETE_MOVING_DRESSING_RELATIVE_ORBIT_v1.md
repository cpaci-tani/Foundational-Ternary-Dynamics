# FTD-0706 — Complete moving-dressing relative-orbit test v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0705

## Question

Is the current “static dressing plus `v=1/2` constituent momentum” preparation
already a native uniformly moving complete state, in the exact discrete sense

\[
F^2(X)=T_{(1,0,0)}X,
\]

or does only the constituent core translate while the complete field fails the
relative-periodic-orbit condition?

## Frozen setup

- FTD-0638 orientation-0 complete dressed state remapped to periodic `L=33`;
- assign all 16 constituents `production_flat_momentum((1/2,0,0))`;
- execute exactly two complete common-action ticks;
- construct `T_(1,0,0) X` by integer-shifting every constituent anchor and
  every matched face/edge carrier, leaving remainders, momenta, charges,
  binding graph, and edge rest data unchanged;
- reverse the two ticks to the initial complete state;
- run a zero-momentum `m=0,q=2` fixed-point control;
- run an integer-translation covariance control by shifting the entire moving
  initial state by `(3,0,0)`, evolving it two ticks, and comparing it with the
  shifted unshifted final state.

No force, damping, external drive, field correction, or root search is added.

## Frozen observables and gates

Record maximum residuals between `F^2(X)` and `T_1 X` separately for:

- effective constituent position;
- constituent momentum;
- electric face field;
- magnetic edge field;
- complete state.

All execution, common-action, energy, and reverse gates must pass at `1e-10`
or better; complete inverse distance must be `<=1e-9`; the rest fixed-point
and translated-covariance controls must be `<=1e-9`.

Verdicts:

- `COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_CANDIDATE` if every relative-orbit
  residual is `<=1e-9`;
- `CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING` if position and momentum
  residuals are `<=0.05` but either field residual is `>1e-6`;
- `NO_RELATIVE_ORBIT_FOR_STATIC_BOOST_PREPARATION` if execution is valid but
  the matter residual also exceeds `0.05`;
- `MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID` if any algebra,
  covariance, rest, energy, or inverse gate fails.

The second or third verdict rejects only this preparation. It converts the
next problem into a state-only shooting equation for a complete relative orbit;
it does not authorize post-hoc damping or field replacement.
