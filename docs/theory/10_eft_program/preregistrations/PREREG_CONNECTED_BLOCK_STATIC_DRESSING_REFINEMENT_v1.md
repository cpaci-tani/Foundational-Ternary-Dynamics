# FTD-0628 — Connected-block static dressing refinement v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0627 verdict `CONNECTED_BLOCK_BOUNDED_IRREGULAR_REST_OPEN`  
**Scope:** observer-only search for a genuine static dressed configuration of
the selected connected 16-constituent block  
**Date:** 2026-07-27

## 1. Question

FTD-0627 shows that the rigid exact-half initialization is centre-stationary,
bounded, and reversible for 256 ticks, but carries small non-recurrent internal
motion. This campaign asks whether that motion is merely relaxation from a
nonstationary rigid starting geometry:

> Does the unchanged selected action possess a nearby symmetry-preserving,
> minimum-energy-Gauss-dressed configuration that is stationary against all 48
> constituent coordinates and remains fixed under repeated common-action
> evolution?

No frequency, clock, mass, particle, quantum, or ground-state interpretation is
permitted unless the full fixed-point gate passes.

## 2. Frozen parent model

- `L=17`, `width=2`, 16 ternary-polarity constituents and the unchanged 72-edge
  Moore-local reference graph;
- exact body-axis half phase and zero constituent momentum;
- unchanged production dispersion, quadratic polarity coats, straight face
  current, matched face/edge field update, binding action, `kappa=1`, `dt=1`,
  `C_SPEED=1/sqrt(3)`, normalization, common-action solve, and state-only inverse;
- `allow_shared_anchor_chart=true`, with observed multiplicity no greater than
  two;
- no production tick, toggle, scenario, GPU, WASM, force branch, reaction,
  threshold, or ontology change.

## 3. Symmetry-reduced geometry

Write centred body-axis coordinates as four axial layers
`{-a,-b,+b,+a}`. Cubic transverse symmetry fixes each layer to the four points
`(+-t,+-t)` in its transverse plane. Charge conjugation plus body reflection
fixes the negative-charge half as the reflected positive-charge half. The only
registered shape coordinates are therefore

`theta = (a,b,t_outer,t_inner)`

with rigid start `(1.5,0.5,0.5,0.5)`. The admissible box is

- `1.25 <= a <= 1.75`;
- `0.25 <= b <= 0.75` and `a-b >= 0.50`;
- `0.25 <= t_outer,t_inner <= 0.75`.

The centre, charges, graph metadata, edge rest lengths, and zero momentum are
held fixed. Cyclic x/y copies use the identical four coordinates.

## 4. Static functional and deterministic refinement

For every admissible `theta`, discard the inherited fields and solve the
periodic minimum-energy longitudinal Gauss field to `1e-13`; set the magnetic
half-field to zero. Minimize the unchanged static energy

`U(theta) = U_binding(theta) + beta U_field(theta)`,

where `beta` is the independently measured face-field work normalization.
Rest energy is constant and omitted.

Use damped Newton iteration from the rigid start only:

- central gradient step `h_g=2e-5`;
- central Hessian step `h_H=2e-4`;
- at most 16 Newton iterations;
- symmetric Jacobi eigensolve for the 4x4 Hessian;
- if the Hessian is not positive definite, replace eigenvalues below `1e-6`
  by `1e-6` for the search step only;
- backtracking factors `1,1/2,...,1/1024`, accepting the first admissible point
  with lower `U`;
- stop only when `||grad U||_inf <= 1e-9`; otherwise classify optimization
  negative.

No additional initial point, random restart, parameter scan, changed box,
tolerance change, or post-result fit is allowed in v1.

## 5. Gates

Run the registered x-oriented solution and its cyclic y copy.

1. **Initialization:** every Gauss redress is valid with residual `<=1e-11`;
   graph metadata and charge assignment are unchanged.
2. **Stationarity in the ansatz:** final `||grad U||_inf <=1e-9`; the
   unmodified final Hessian has minimum eigenvalue `>1e-6`; energy is lower
   than the rigid start.
3. **Full-space stationarity:** one unchanged common-action step executes, and
   the maximum norm of all 16 total constituent impulses is `<=1e-9`; maximum
   constituent displacement and momentum are each `<=1e-9`.
4. **Repeated fixed point:** 64 forward steps and 64 state-only inverse steps
   execute. Maximum centre displacement is `<=1e-10`; maximum internal state
   distance from the refined initial state is `<=1e-8`; energy drift is
   `<=1e-12`; final recovery is `<=1e-10`; multiplicity is `<=2` and any
   shared-anchor effective-position separation is `>=0.9`.
5. **Common action:** every forward/inverse residual remains `<=1e-10`.
6. **Cubic covariance:** refined coordinates and all scalar diagnostics agree
   under the x-to-y cyclic copy to relative/absolute `1e-9`; rotated states
   agree to maximum component `1e-9`.

## 6. Locked verdicts

- `CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE` only if all six
  gates pass.
- `CONNECTED_BLOCK_SYMMETRY_STATIONARY_ONLY` if Gates 1–2 pass but either the
  full-space or repeated fixed-point gate fails.
- `CONNECTED_BLOCK_STATIC_REFINEMENT_CLOSED_NEGATIVE` if no registered
  positive-Hessian stationary point is found.
- `EXECUTION_INVALID` if the parent fingerprint, protocol lock, normalization,
  coverage, output, or instrumentation checks fail.

A constructive result establishes a selected classical dressed fixed point of
the research action only. A negative result closes this four-coordinate
symmetry ansatz, not every constituent geometry or binding action.

## 7. Artifacts

- observer redressing API under `connected_moore_block_action`;
- `test_connected_block_static_dressing_refinement.cpp` and focused CTest;
- versioned JSON plus optimization, arm, and tick CSV records;
- independent Python recomputation of recorded energy, stationarity, Hessian,
  covariance, repeated-state, and verdict gates;
- analysis, audit, ledger, tracker, manifest, indexes, changelog, and engine
  specification updated together.
