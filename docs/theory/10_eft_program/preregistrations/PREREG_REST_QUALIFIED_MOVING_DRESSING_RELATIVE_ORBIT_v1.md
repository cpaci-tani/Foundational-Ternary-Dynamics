# FTD-0709 — Rest-qualified moving-dressing relative orbit v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0708

## Question

After replacing FTD-0706's drifting control by the qualified `L=33` rest
fixed point from FTD-0708, does uniform constituent momentum `v=1/2` already
produce a complete relative orbit

\[
F^2(X)=T_{(1,0,0)}X?
\]

## Frozen setup

- reconstruct the FTD-0708 state from its 16 recorded constituent positions
  and unchanged charge/graph data, then apply the unchanged fibre-cap-8
  minimum-energy longitudinal redressing;
- verify the reconstructed rest state fingerprint and two-tick fixed-point
  residual before interpreting motion;
- assign every constituent `production_flat_momentum((1/2,0,0))`;
- execute exactly two complete common-action ticks;
- compare the result with exact integer translation of all constituents,
  momenta, face electric carriers, and edge magnetic carriers;
- reverse both ticks and run a shift-`(3,0,0)` covariance control.

No moving-field solve, force, damping, trajectory fit, or post-hoc redressing
is applied.

## Frozen gates and verdicts

All common-action and energy residuals must be `<=1e-10`; inverse, rest, and
translation covariance must be `<=1e-9`. Record position, momentum, electric,
magnetic, and complete relative-orbit residuals separately.

- `REST_QUALIFIED_COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_CANDIDATE` if every
  component residual is `<=1e-9`;
- `REST_QUALIFIED_CORE_TRANSLATES_WITHOUT_COMPLETE_MOVING_DRESSING` if
  position and momentum residuals are `<=0.05` but either field residual is
  `>1e-6`;
- `REST_QUALIFIED_STATIC_BOOST_HAS_NO_RELATIVE_ORBIT` if the matter residual
  also exceeds `0.05`;
- `REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID` if any
  provenance, reconstruction, action, energy, inverse, rest, or covariance
  gate fails.

The second or third verdict rejects only instantaneous static boosting. The
next admissible candidate is a state-only moving-field shooting solve or a
causal acceleration/formation history, not a new primitive by default.

