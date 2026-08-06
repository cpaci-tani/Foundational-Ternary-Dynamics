# FTD-0637 — Connected-block analytic envelope Hessian v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0636 verdict
`CONNECTED_BLOCK_KNOT_LOCAL_HESSIAN_EXECUTION_INVALID`  
**Qualified state parent:** FTD-0633 verdict
`CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CONSTRUCTIVE`  
**Scope:** analytic first and second derivatives of the frozen static functional
inside the occupied quadratic-B-spline sectors  
**Date:** 2026-07-27

## 1. Question

Is the FTD-0633 dressed state a stationary positive basin of the complete
48-coordinate static functional when derivatives are evaluated analytically,
rather than by finite differences that either cross coat knots or compete with
the Poisson stopping error?

## 2. Frozen functional

No state, graph, charge, normalization, volume, or energy term changes.  For
constituent positions `x`, use exactly

`E(x) = E_bind(x) + beta E_longitudinal[rho(x)]`,

where `rho` is the tensor quadratic coat, `beta` is the locked face-field
normalization, and `E_longitudinal = (1/2) rho^T phi` with
`(-Delta) phi = rho` on the same `L=17` periodic zero-mean lattice used by
FTD-0633--0636.

## 3. Analytic derivatives

Within each occupied polynomial sector, enumerate the exact one-dimensional
quadratic-B-spline value, first derivative, and second derivative.  Assemble
`rho`, all 48 vectors `rho_i = partial_i rho`, and all same-constituent second
derivatives `rho_ij = partial_i partial_j rho`.  Distinct constituents have
`rho_ij=0`.

Solve

`(-Delta) phi = rho`,  `(-Delta) chi_j = rho_j`

to infinity-norm residual `<=1e-13`.  The locked field derivatives are

`g_i^field = beta rho_i^T phi`,

`H_ij^field = beta (rho_i^T chi_j + phi^T rho_ij)`.

Add the exact spring derivatives for every Moore edge with
`E_e=(k/4)(|d|^2-l_e^2)^2`:

`grad_d E_e = k (|d|^2-l_e^2)d`,

`H_dd E_e = k[2 d d^T + (|d|^2-l_e^2)I]`.

No fitted force, post-hoc correction, or numerical-difference value enters the
analytic result.

## 4. Registered checks

Run the frozen `x` and cyclic `y` arms.  Require:

- charge, derivative-charge, and derivative-dipole sum rules `<=1e-12`;
- Poisson residuals for `phi` and every `chi_j` `<=1e-13`;
- analytic Hessian antisymmetry `<=1e-12`;
- analytic versus knot-local finite-difference gradient agreement `<=5e-8`
  at the already locked FTD-0636 step `4e-6` (comparison only);
- analytic versus knot-local finite-difference Hessian agreement `<=5e-4`
  at the already locked FTD-0636 step `4e-5` (comparison only);
- full gradient infinity norm `<=1e-8` for stationarity;
- Jacobi eigensolver residual `<=1e-7`, orthogonality `<=1e-10`;
- cyclic sorted-spectrum covariance `<=1e-6`;
- minimum analytic eigenvalue `>1e-5` for a positive basin;
- exact rigid-translation Rayleigh/direct analytic contraction identity
  `<=1e-12`.

The finite-difference comparisons cannot override the analytic verdict.  A
comparison failure makes the execution invalid because it indicates an
implementation inconsistency.

## 5. Verdicts

- `CONNECTED_BLOCK_ANALYTIC_48D_BASIN_CONSTRUCTIVE`: all common gates pass,
  both gradients pass stationarity, and both analytic spectra are positive;
- `CONNECTED_BLOCK_ANALYTIC_FALSE_MINIMUM`: common gates pass, stationarity
  passes, and a negative eigenvalue below `-1e-5` exists;
- `CONNECTED_BLOCK_ANALYTIC_NONSTATIONARY`: common analytic and comparison
  gates pass but either gradient exceeds `1e-8`;
- `CONNECTED_BLOCK_ANALYTIC_HESSIAN_MARGINAL`: stationarity passes but an
  eigenvalue lies in `[-1e-5,1e-5]`;
- `CONNECTED_BLOCK_ANALYTIC_HESSIAN_EXECUTION_INVALID`: provenance, sector,
  coverage, Poisson, identity, comparison, covariance, or output failure.

A constructive verdict is a local classical static result for the selected
action and eight-record chart.  It does not establish a particle, mass,
quantum state, pole, continuum translation symmetry, or production ontology.
A nonstationary verdict refutes FTD-0633 as a full 48-coordinate rest state and
requires analytic refinement before any mode language resumes.

## 6. Artifacts

Produce a focused CTest, versioned JSON/CSV matrices and spectra, an independent
certificate, and synchronized analysis/audit/ledger records.  Production
remains unchanged.
