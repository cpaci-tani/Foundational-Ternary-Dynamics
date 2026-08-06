# FTD-0713 — Causal-bound internal-gait continuation v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parent:** FTD-0712

## Question

FTD-0712 reduced the exact body-diagonal nullspace obstruction by a factor of
ten while maintaining stable Jacobian pivots, but stopped when one midpoint
coordinate reached the auxiliary `0.05` displacement cap. Does the same
existing-variable gait reach exact compatibility when constrained by physical
causal speed and graph deformation rather than that auxiliary cap?

## Frozen continuation

Start from the exact 16 midpoint displacements recorded by FTD-0712 (state CSV
SHA-256 `40CD492F9766FB1DC701CF71CA51B0B91D2B2C5E7464785F6AE2A7433FB84030`).
Retain without change:

- the FTD-0708 start geometry and one-site translated endpoint geometry;
- the exact zero-center parameterization;
- the 32 exact quadratic-coat straight-segment currents;
- the four independent body-diagonal modes and two left-null vectors per mode;
- the `16x45` centered Jacobian with `h=1e-5`;
- the minimum-norm Newton step `-J^T(JJ^T)^-1R`;
- pivoted Gram solve, eight iterations, and backtracking through `1/1024`;
- strict residual decrease on every accepted step;
- endpoint, source, current, field operator, and tolerance.

Remove only `max ||delta_a||_infinity <=0.05`. Continue to require for every
constituent

\[
|\tfrac12\hat x+\delta_a|\le C_{\rm SPEED},
\qquad
|\tfrac12\hat x-\delta_a|\le C_{\rm SPEED},
\]

and maximum fractional graph-edge deformation `<=0.10`. This is a registered
change of candidate family after a closed-negative bounded run, not a
reinterpretation of FTD-0712.

## Gates and verdicts

Require all current, center, conjugacy, covariance, speed, edge, and numerical
gates from FTD-0712, with final 16-component residual and full eight-mode norm
each `<=1e-10`.

- `CAUSAL_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE` if every gate passes;
- `CAUSAL_OR_EDGE_BOUND_PREVENTS_RESONANCE_CANCELLATION` if evaluations and
  algebra remain valid but no accepted state reaches the residual gate;
- `CAUSAL_INTERNAL_GAIT_CONTINUATION_EXECUTION_INVALID` for provenance,
  reconstruction, current, nullspace, Jacobian, conjugacy, or covariance
  failure.

A constructive result remains kinematic. It advances to exact Fourier field
reconstruction and an unchanged reciprocal matter replay. It does not show
that the action dynamically generates the gait.
