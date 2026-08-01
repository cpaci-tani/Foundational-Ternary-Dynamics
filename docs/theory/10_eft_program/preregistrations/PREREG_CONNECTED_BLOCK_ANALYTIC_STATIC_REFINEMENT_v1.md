# FTD-0638 — Connected-block analytic static refinement v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0637 verdict `CONNECTED_BLOCK_ANALYTIC_NONSTATIONARY`  
**State parent:** FTD-0633 eight-fibre dressed geometry  
**Scope:** deterministic full-coordinate Newton refinement within the unchanged
quadratic-coat sector and static functional  
**Date:** 2026-07-27

## 1. Question

Does the positive analytic basin found by FTD-0637 contain a genuine
48-coordinate stationary state, or does the residual analytic force persist
when the state is refined without changing the action, chart, or ontology?

## 2. Locked refinement

For each frozen `x` and cyclic `y` arm, recompute the FTD-0637 analytic
gradient `g` and Hessian `H`.  Solve `H delta = -g` by pivoted Gaussian
elimination.  Try step multipliers in the fixed order

`alpha in {1, 1/2, 1/4, 1/8, 1/16, 1/32, 1/64}`.

Accept the first candidate that:

- keeps every constituent in its original quadratic-B-spline polynomial
  sector;
- remains valid under the cap-eight chart and Poisson redressing;
- satisfies the Armijo condition
  `E_new <= E_old + 10^-4 alpha g.delta`.

Use at most four accepted Newton iterations.  Stop early only when the
analytic gradient infinity norm is `<=1e-12`.  No coordinate is projected
back into the four-parameter family, and no force or energy term is added.

## 3. Gates

Require in both orientations:

- FTD-0637 parent fingerprint and normalization valid;
- every analytic derivative and Poisson gate inherited from FTD-0637;
- at least one accepted Newton step and at most four;
- no spline-sector crossing and no anchor-multiplicity above eight;
- strictly lower static energy;
- final analytic gradient infinity norm `<=1e-12`;
- final minimum analytic Hessian eigenvalue `>1e-5`;
- eigensolver residual `<=1e-7` and orthogonality `<=1e-10`;
- cyclic final-energy, sorted-spectrum, and displacement-pattern covariance
  residuals `<=1e-9`;
- analytic gradient checked against the locked `4e-6` central difference to
  `<=5e-8` at the final state.

## 4. Verdicts

- `CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE` if all gates pass;
- `CONNECTED_BLOCK_ANALYTIC_REFINEMENT_NONSTATIONARY` if the analytic
  machinery remains valid but the four-iteration state exceeds `1e-12`;
- `CONNECTED_BLOCK_ANALYTIC_REFINEMENT_LEFT_SECTOR` if no registered step can
  remain in the original sector;
- `CONNECTED_BLOCK_ANALYTIC_REFINEMENT_EXECUTION_INVALID` for provenance,
  solve, identity, comparison, covariance, or output failure.

The constructive verdict would establish an observer-level local rest state
of the selected constituent action.  It would not yet establish dynamical
stability, a propagating pole, a quantum particle, or production adoption.

## 5. Artifacts

Produce a focused CTest, versioned JSON/CSV records, independent certificate,
analysis/audit, and synchronized canonical records.  Preserve FTD-0637's
nonstationary parent result and leave production unchanged.
