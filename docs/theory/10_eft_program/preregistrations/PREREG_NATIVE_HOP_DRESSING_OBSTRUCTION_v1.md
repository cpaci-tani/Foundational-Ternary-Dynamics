# FTD-0560 — Native Periodic-Hop Dressing Obstruction v1

**Status:** [PRE-REGISTRATION — LOCKED/RUN; POSITIVE POINT-HOP OBSTRUCTION]
**Date locked:** 2026-07-26
**Scope:** observer-only theorem campaign for the isolated production
`FULL`-stencil wave plus native state-coupling sector
**Production changes:** none

## 1. Question

Can a single manifested polarity that advances by one lattice site every
finite `T` ticks carry a square-summable, exactly co-moving native field
dressing without radiating into the production wave band?

This protocol addresses the prescribed one-site schedule only.  It does not
exclude extended, neutral, internally deforming, nonlinear, defect-bound, or
topological carriers.

## 2. Frozen symbols

For central differences define

\[
 \mathbf q(\mathbf k)=(\sin k_x,\sin k_y,\sin k_z).
\]

The production state source in `phase_read` is

\[
 f=iG_C[-\mathbf q\,S+\mathbf q\times\mathbf j].
\]

For a rigid point polarity with velocity `v`, `j=vS`, so

\[
 |f|^2=G_C^2|S|^2
 (|\mathbf q|^2+|\mathbf q\times\mathbf v|^2).
\]

The equality is locked as an exact orthogonality theorem: the longitudinal
gradient source and transverse curl source cannot destructively interfere.

For one hop `d` every `T` ticks, use the FTD-0558 spectrum

\[
 \Omega_l=(\mathbf k\cdot\mathbf d+2\pi l)/T,
\qquad
 c_l=\frac{1-e^{i\mathbf k\cdot\mathbf d}}
 {T[1-e^{i(\mathbf k\cdot\mathbf d+2\pi l)/T}]}.
\]

The production pole is

\[
 D_l(\mathbf k)=C_{\rm WAVE}^2M(\mathbf k)
 -4\sin^2(\Omega_l/2).
\]

## 3. Locked existence proof

For an axial momentum `u`, the `FULL` symbol reduces exactly to

\[
 M(u,0,0)=4\sin^2(u/2),
\qquad
 \theta_a(u)=2\arcsin(\sin(u/2)/\sqrt3),\quad 0<u<\pi.
\]

The following roots are locked before numerical execution.

1. `T=1`: fix `k_parallel=0.1`.  Vary the cyclic transverse component over
   `[0,0.2]`.  The principal harmonic changes sign between the endpoints and
   therefore has an interior oblique root.
2. `T=2`: the principal axial harmonic has a unique root of
   `u/2=theta_a(u)` in `(0,pi)`; in fact `u=2pi/3`.
3. Every `T>=3`: harmonic `l=1` has a unique axial root `u_T in (0,pi)` of

   \[
    (2\pi-u_T)/T=\theta_a(u_T).
   \]

   Existence follows because the left-minus-right expression is positive at
   `u=0` and negative at `u=pi`; uniqueness follows from strict monotonicity
   of `T theta_a(u)+u`.

At every axial root for `T>=2`, resonance and the axial dispersion imply the
exact coefficient identity

\[
 |c_l|=\frac{\sqrt3}{T}.
\]

The effective longitudinal forcing is therefore

\[
 |f_l|=G_C\frac{\sqrt3}{T}\sin u_T>0.
\]

As `T -> infinity`,

\[
 u_T=\frac{2\pi\sqrt3}{T+\sqrt3}+O(T^{-3}),
\qquad
 |f_l|=\frac{6\pi G_C}{T^2}+O(T^{-3}).
\]

Thus slow hopping suppresses the resonant forcing but never makes it exactly
zero at finite period.

## 4. Square-summability consequence

A Floquet co-moving field would satisfy mode by mode

\[
 J_l(\mathbf k)=f_l(\mathbf k)/D_l(\mathbf k).
\]

At a regular point of a resonance surface, choose a normal coordinate `n`.
If `f_l` is nonzero there, `D_l=lambda n+O(n^2)` and
`|J_l|^2 >= C/n^2`.  Its local three-dimensional integral diverges.
Therefore no finite-energy/square-summable exactly co-moving linear dressing
exists for the registered point-hop schedule.

This does not assert that every finite periodic grid contains an exactly
resonant grid point.  It is an infinite-lattice/continuum-BZ solvability
obstruction; finite boxes approach the resonant surface through increasingly
near-resonant modes.

## 5. Locked observer campaign

Run periods `T=1,...,16`, all three positive coordinate hop axes, and both
polarities: exactly 96 arms.

For every arm require:

- bisection/root residual at or below `1e-12`;
- root strictly inside its registered bracket;
- production denominator residual at or below `1e-12`;
- direct source-norm versus orthogonal decomposition residual at or below
  `1e-12`;
- effective forcing divided by `G_C` strictly above `0.05`;
- polarity reversal changes the complex source sign and preserves its norm
  within `1e-12`;
- coordinate rotations preserve root, phase, coefficient norm, and source
  norm within `1e-12`;
- for every `T>=2`, `||c_l|-sqrt(3)/T|<=1e-12`.

Additionally drive the exact production modal map at each registered root for
`N=128` ticks and require the normalized energy coefficient to satisfy the
FTD-0559 analytic resonant bound around `1/2`.  This is a field-response
check, not a particle-radiation measurement.

## 6. Verdicts

- `POINT_HOP_DRESSING_OBSTRUCTED`: all analytic identities, root
  cardinalities, covariance arms, and resonant response bounds pass.  This
  closes an exactly co-moving square-summable dressing for a single periodic
  point polarity in the frozen linear native coupling sector.
- `POINT_HOP_DRESSING_OBSTRUCTION_FAILED`: any locked gate fails.  No carrier
  claim advances.

Passing does not close the general nonlinear carrier gate and does not license
a toggle, force, scenario, damping term, source subtraction, or ontology
change.

## 7. Execution record

The protocol was locked before compilation and execution with SHA-256
`4056580CFEDFC2E0A638FE9DE0B3D8D5B609B2CBECFB294033BE67A571047477`.
All 96 arms passed under pinned MSVC 14.44.  Maximum pole, source
orthogonality, coefficient, polarity-mirror, and cubic-covariance residuals
were respectively `4.44e-16`, `8.67e-19`, `2.22e-16`, zero, and `7.77e-16`.
The minimum normalized effective forcing was `0.0628777`, above the locked
`0.05` gate.  Verdict: `POINT_HOP_DRESSING_OBSTRUCTED`.
