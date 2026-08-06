# FTD-0639 — Connected-block analytic dynamical rest v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0638 verdict
`CONNECTED_BLOCK_ANALYTIC_STATIC_BASIN_CONSTRUCTIVE`  
**Scope:** state-only common-action evolution of the analytically centered
48-coordinate dressed state  
**Date:** 2026-07-27

## 1. Question

Does the FTD-0638 local minimum remain a bounded, reversible rest state under
the already selected constituent common-action tick, or is it only a static
energy construction?

## 2. Frozen state and dynamics

Load the 17-digit FTD-0638 final positions for the `x` and cyclic `y` arms,
restore their unchanged charges and Moore graph, set every constituent
momentum to zero, and rebuild the longitudinal electric dressing with the
cap-eight chart.  Use the existing `solve_connected_moore_block_forward` and
`solve_connected_moore_block_reverse` with production dispersion, wave speed,
binding stiffness, timestep, finite-difference scale, iteration limit, and
default solve tolerances.  The only enabled research option is the already
selected shared-anchor chart.

For each arm run 128 forward ticks, followed by 128 state-only reverse ticks.
No history, force correction, field reset, damping, reaction, collision,
legacy force, or production toggle is available to the reverse solve.

## 3. Gates

Require:

- parent fingerprint, longitudinal redressing, graph, neutrality, and cap-eight
  chart valid;
- every forward and reverse step valid with every common-action residual
  `<=1e-10`;
- no anchor multiplicity above eight, no spline-sector crossing, and no site
  hop;
- maximum constituent impulse `<=1e-9`;
- maximum full-state distance from the initial rest state `<=1e-8`;
- maximum centre displacement `<=1e-10`;
- maximum total-energy drift `<=1e-12`;
- final state-only inverse recovery `<=1e-10`;
- cyclic covariance of impulse, state excursion, centre drift, energy drift,
  and recovery `<=1e-9`.

## 4. Verdicts

- `CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_CONSTRUCTIVE` if all gates pass;
- `CONNECTED_BLOCK_ANALYTIC_STATIC_ONLY` if valid evolution leaves the locked
  rest envelope or fails recovery;
- `CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_EXECUTION_INVALID` for provenance,
  initialization, solver, coverage, covariance, or output failure.

A constructive verdict establishes bounded reversible classical rest only for
the selected action and finite campaign.  It does not establish indefinite
stability, a propagating matter pole, quantum behavior, or production
adoption.

## 5. Artifacts

Produce a focused CTest, tick CSV and JSON/CSV summary, independent certificate,
analysis/audit, and synchronized canonical records.  Production remains
unchanged.
