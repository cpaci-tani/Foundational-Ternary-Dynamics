# FTD-0710 — Prescribed-trajectory co-moving field shooting v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parents:** FTD-0708, FTD-0709

## Question

Can the matched face/edge field itself be made exactly stationary in the
co-moving frame of the qualified `L=33` composite when the constituents are
prescribed to translate rigidly by one axial site in two ticks? If so, does
that field also make the same trajectory a solution of the unchanged
reciprocal matter action?

This is a preconditioner and obstruction-localization campaign. A prescribed
trajectory is not promoted to matter dynamics merely because its field
shooting equation is soluble.

## Frozen state and trajectory

1. Reconstruct the FTD-0708 corrected 16-constituent rest geometry from its
   run-of-record state CSV and rebuild its minimum-energy longitudinal field.
2. Set the axial schedule

   \[
   x_a^{(0)}=x_a,\qquad
   x_a^{(1)}=x_a+\tfrac12\hat x,\qquad
   x_a^{(2)}=x_a+\hat x
   \]

   for every constituent `a`, with charges and graph unchanged.
3. Deposit each tick's current with the existing exact quadratic-coat straight
   segment. Require every segment to be valid, causal, and continuous at
   `1e-12`.

No force, recoil, or endpoint is fitted in the field-only solve.

## Frozen co-moving field equation

For `lambda=C_SPEED`, apply the existing matched leapfrog update twice:

\[
B_{n+1/2}=B_{n-1/2}-\lambda C^T E_n,
\qquad
E_{n+1}=E_n+\lambda C B_{n+1/2}-j_n.
\]

Let `H` be these two sourced ticks followed by translation by `-1` site along
`x`. Solve

\[
\boxed{H(E,B)-(E,B)=0.}
\]

Write the initial field as the FTD-0708 longitudinal field plus a source-free
correction. The correction is solved from the exact affine residual using
matrix-free restarted GMRES:

- restart length `48`;
- at most `480` operator applications;
- modified Gram-Schmidt with Givens rotations;
- zero initial correction;
- stop when the Euclidean residual is at most
  `max(1e-11, 1e-11*||b||_2)`;
- no regularization, damping, spectral deletion, source alteration, or
  post-solve Gauss projection.

Because the right-hand side is the difference of two fields sourced by the
same translated density, the Krylov range is divergence-free. Record the
actual Gauss, harmonic-mean, and infinity-norm residuals rather than assuming
this numerically.

## Frozen reciprocal replay

If the field solve reaches a co-moving infinity residual `<=1e-9`, place it on
the unchanged FTD-0708 geometry, assign every constituent the production
momentum corresponding to `v=(1/2,0,0)`, and execute two unchanged connected
common-action ticks. Compare the result to the exact one-site translation of
the complete initial state. Then reverse both ticks state-only.

Also translate the solved initial state by three sites and require its
field-only residual and reciprocal two-tick result to translate covariantly.

## Locked gates and verdicts

Field gates:

- all 32 current segments valid, causal, and continuous at `1e-12`;
- GMRES completes without non-finite algebra;
- co-moving electric and magnetic infinity residuals each `<=1e-9`;
- Gauss residual before and after `<=1e-10`;
- absolute mean of each electric and magnetic component `<=1e-12`;
- shifted field-only covariance residual `<=1e-9`.

Reciprocal gates:

- every common-action residual and total-energy drift `<=1e-10`;
- two-tick inverse recovery `<=1e-9`;
- shifted reciprocal covariance `<=1e-9`;
- complete relative-orbit residual `<=1e-9` for a constructive complete
  orbit.

Verdicts:

- `PRESCRIBED_TRAJECTORY_COMPLETE_RELATIVE_ORBIT_CANDIDATE` if both field and
  reciprocal gates pass and the complete relative-orbit residual passes;
- `COMOVING_FIELD_SOLVED_RIGID_MATTER_NOT_SELF_CONSISTENT` if every field gate
  passes but the reciprocal complete orbit does not;
- `PRESCRIBED_TRAJECTORY_FIELD_SHOOTING_NOT_RESOLVED` if the locked GMRES run
  is valid but any field residual gate fails;
- `PRESCRIBED_TRAJECTORY_COMOVING_FIELD_EXECUTION_INVALID` for failed parent
  provenance, reconstruction, current, non-finite algebra, common action,
  inverse, or covariance evaluation.

The second verdict advances to a coupled matter-coordinate/momentum shooting
solve. The third advances to a causal acceleration/formation history and a
finite-volume resonance analysis. Neither negative verdict licenses a new
primitive or a modified interaction by itself.
