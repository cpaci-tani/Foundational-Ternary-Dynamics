# FTD-0636 — Connected-block knot-local Hessian v1

**Status:** `[PRE-REGISTRATION — LOCKED AFTER FTD-0635 DIAGNOSIS]`  
**Parent:** FTD-0635 verdict
`CONNECTED_BLOCK_FULL_48D_HESSIAN_EXECUTION_INVALID`  
**Qualified state parent:** FTD-0633 verdict
`CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CONSTRUCTIVE`  
**Scope:** full 48-coordinate Hessian inside one smooth quadratic-B-spline
sector  
**Date:** 2026-07-27

## 1. Structural diagnosis

The selected polarity coat is a tensor quadratic B-spline. Its one-dimensional
kernel is `C1` but not `C2`; the second derivative jumps at half-integer knot
planes. FTD-0634/0635 used `h_H=2e-4`, while the closest frozen constituent
coordinate is only about `9.465e-5` from a knot. Those Hessian stencils cross
different polynomial sectors. Consequently, their positive spectra are
recorded but not qualified Hessians, and the FTD-0633 `h=1e-3` translation
curvatures are finite-amplitude secants rather than local second derivatives.

## 2. Frozen knot-local estimator

Repeat the full FTD-0635 enumeration with:

- first-derivative step `h_g=4e-6`;
- Hessian and rigid-translation step `h_H=4e-5`.

Before evaluating energy, compute the minimum distance `delta_knot` from every
frozen effective coordinate to any half-integer knot. Require
`h_H < delta_knot/2`. Verify every one- and two-coordinate Hessian stencil
stays in the same kernel sector as the center.

The 4,711-evaluation count, cap-eight chart, static functional, eigensolver,
state, graph, orientations, and all non-translation numerical gates remain
unchanged.

## 3. Translation identity

The FTD-0633 coarse secants are retained as diagnostics but are no longer used
as local-Hessian targets. In each orientation and direction require

`|v_T^T H v_T - K_direct/16| <= 1e-5`,

where `v_T` is the normalized 48-coordinate rigid-translation vector and
`K_direct` is recomputed with the same knot-local `h_H`. No fitted target is
introduced.

## 4. Gates and verdicts

Inherit FTD-0635 gates: 48-gradient infinity norm `<=1e-8`, valid redressing,
Gauss `<=1e-11`, multiplicity `<=8`, same-anchor separation `>=0.9`, Hessian
antisymmetry `<=1e-12`, eigensolver residual `<=1e-7`, orthogonality
`<=1e-10`, and cyclic sorted-spectrum covariance `<=1e-6`.

Verdicts:

- `CONNECTED_BLOCK_KNOT_LOCAL_48D_BASIN_CONSTRUCTIVE` if both arms satisfy
  every gate and have minimum eigenvalue `>1e-5`;
- `CONNECTED_BLOCK_KNOT_LOCAL_FALSE_MINIMUM` if a common-gate-valid arm has a
  confirmed eigenvalue `<-1e-5`;
- `CONNECTED_BLOCK_KNOT_LOCAL_HESSIAN_MARGINAL` if no confirmed negative mode
  exists but an eigenvalue lies in `[-1e-5,1e-5]`;
- `CONNECTED_BLOCK_KNOT_LOCAL_HESSIAN_EXECUTION_INVALID` for provenance,
  sector, coverage, redress, translation identity, covariance, eigensolver, or
  output failure.

A constructive verdict establishes a piecewise-smooth local adiabatic basin,
not a globally `C2` matter action. Physical small-oscillation claims must keep
amplitudes inside the measured knot clearance or replace the selected coat
with a smoother action in a separately versioned ontology.

## 5. Artifacts

Produce a distinct CTest and `ftd_0636` JSON/CSV records, independent
certificate, analysis/audit, and synchronized canonical records. Preserve both
invalid predecessor runs and leave production unchanged.
