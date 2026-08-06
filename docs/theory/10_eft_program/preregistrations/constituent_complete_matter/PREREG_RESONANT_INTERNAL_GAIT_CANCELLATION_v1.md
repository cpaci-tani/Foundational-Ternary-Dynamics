# FTD-0712 — Resonant internal-gait cancellation v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parents:** FTD-0708, FTD-0711

## Question

Can the existing 16-constituent composite cancel the exact body-diagonal
co-moving field obstruction by a small internal deformation during the
half-tick, while preserving its center trajectory and returning to the same
translated geometry after two ticks?

## Frozen endpoints and internal variables

Use the corrected FTD-0708 `L=33` geometry. For constituent `a`, fix

\[
x_a^{(0)}=x_a,
\qquad x_a^{(2)}=x_a+\hat x,
\]

and vary only the midpoint

\[
x_a^{(1)}=x_a+\tfrac12\hat x+\delta_a.
\]

Enforce `sum_a delta_a=0` exactly by using the first 15 constituent vectors as
45 independent coordinates and setting `delta_15=-sum_{a<15}delta_a`.
Therefore the center advances exactly one half site on each tick. Charges,
graph, endpoint geometry, coat, field stencil, and volume remain unchanged.

Each of the 32 currents is the existing exact quadratic-coat straight-segment
current. Both segment speeds must satisfy

\[
|\tfrac12\hat x+\delta_a|\le C_{\rm SPEED},
\qquad
|\tfrac12\hat x-\delta_a|\le C_{\rm SPEED}.
\]

Also require `max_a ||delta_a||_infinity <=0.05` and maximum fractional change
of any graph-edge length at the midpoint `<=0.10`.

## Frozen resonant residual

FTD-0711 found incompatibility only at the eight exact modes

\[
(k_x,k_y,k_z)=(\pm2\pi/3,\pm2\pi/3,\pm2\pi/3).
\]

Use the four representatives with `k_x=+2*pi/3`; the other four are their
reality conjugates. At each representative, construct the exact `6x6`
co-moving field block from FTD-0711 and an orthonormal basis for its
two-dimensional left nullspace. Project the complete affine field RHS—static
FTD-0708 field plus both deposited currents—onto those two vectors. The real
and imaginary parts give 16 real residual components.

The rigid `delta=0` residual must reproduce the FTD-0711 full eight-mode
nullspace norm `4.6345148020027714e-4` within `1e-12` before any solve is
accepted.

## Frozen solve

- full centered-difference `16x45` Jacobian with `h=1e-5`;
- minimum-coordinate-norm Newton step
  `Delta=-J^T (J J^T)^-1 R`;
- pivoted Gaussian elimination of the `16x16` Gram system;
- at most eight Newton iterations;
- backtracking scales `1,1/2,...,1/1024`;
- accept only a valid causal trial with strictly smaller residual infinity
  norm;
- no regularizer, force term, current rescaling, mode deletion, or endpoint
  adjustment.

## Gates and verdicts

Require:

- every current partition, first moment, current moment, continuity, locality,
  and causal residual `<=1e-12`;
- center constraint `<=1e-14`;
- all speed, displacement, and edge-deformation bounds above;
- final 16-component null residual infinity norm `<=1e-10`;
- conjugate eight-mode reconstruction residual `<=1e-10`;
- translation-by-three covariance residual `<=1e-10`.

Verdicts:

- `RESONANT_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE` if every gate passes;
- `BOUNDED_INTERNAL_GAIT_CANNOT_CANCEL_LOCKED_RESONANCE` if all evaluations and
  linear algebra are valid but no accepted state reaches the residual gate;
- `RESONANT_INTERNAL_GAIT_CANCELLATION_EXECUTION_INVALID` for failed parent
  provenance, rigid cross-check, current construction, nullspace algebra,
  Jacobian, center, conjugacy, or covariance evaluation.

A constructive result is only a kinematic/source compatibility witness. It
advances to a full field reconstruction and unchanged reciprocal matter
replay. It does not establish that the common action dynamically selects the
gait. A negative result closes only this bounded two-tick midpoint family.
