# FTD-0640 — Connected-block analytic matter modes v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0639 verdict
`CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_CONSTRUCTIVE`  
**Static parent:** FTD-0638 exact analytic center  
**Scope:** complete 48-coordinate infinitesimal matter spectrum and
small-amplitude common-action response inside the occupied coat sectors  
**Date:** 2026-07-27

## 1. Question

Does the analytically centered connected object carry genuine internal matter
degrees of freedom whose measured common-action phases follow the complete
analytic Hessian and the unchanged production inertia?

## 2. Locked linear prediction

At each FTD-0638 cyclic center, recompute the FTD-0637 analytic Hessian `H`.
Every constituent coordinate uses the unchanged small-momentum production
inertia `M_INERTIAL`; no fitted mass matrix is allowed. Diagonalize

`H v_m = lambda_m M_INERTIAL v_m`,

with `v_m^T M_INERTIAL v_n = delta_mn`. Predict

`omega_m = sqrt(lambda_m)`,

`Omega_m = 2 atan(omega_m/2)`

for the unit-step implicit common-action map. Modes whose analytic eigenvalues
agree within `1e-10` are evaluated as one degenerate purity subspace; their
individual basis orientation carries no claim.

## 3. Locked perturbations

For every one of the 48 sorted `x`-arm modes, displace the center along the
mass-normalized eigenvector so its largest constituent-coordinate displacement
is exactly `8e-6`. Set all momenta to zero and rebuild the longitudinal field.

For representative sorted indices

`{0,1,3,5,6,8,12,18,24,31,39,45,47}`,

also run:

- an `x`-arm half-amplitude control at `4e-6`;
- an `x`-arm sign mirror at `-8e-6`;
- the cyclic `y`-arm mode at `8e-6`.

This gives 87 registered arms. Run each for 256 forward ticks and then 256
state-only reverse ticks. Every trajectory must remain in the initial spline
sector; no history is supplied to inversion.

## 4. Estimators and gates

Project the full 48-coordinate displacement at every tick onto the analytic
mass-orthonormal basis. Estimate each phase from the locked three-point
recurrence over ticks `1..254`.

Require:

- all 87 arms initialize, complete, and invert;
- initial maximum displacement equals its target within `1e-12`;
- every trajectory stays in its starting spline sector, has zero site hops,
  anchor multiplicity `<=8`, and same-anchor separation `>=0.9`;
- common-action residual `<=1e-10`, energy drift `<=1e-12`, and state-only
  recovery `<=1e-10`;
- center displacement `<=1e-4` and total state excursion `<=1e-3`;
- each primary mode phase differs from `Omega_m` by `<=2%`;
- RMS leakage outside the analytic degenerate eigenspace is `<=10%`;
- representative half/full phase difference `<=0.5%` and excess-energy ratio
  lies in `[3.9,4.1]`;
- representative sign-mirror phase difference `<=0.5%` and normalized signed
  trajectory residual `<=5%`;
- representative cyclic phase difference `<=0.5%`;
- analytic `x/y` sorted-spectrum covariance `<=1e-9`.

## 5. Verdicts

- `CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE` if all gates pass;
- `CONNECTED_BLOCK_ANALYTIC_MATTER_MODE_STABILITY_CLOSED_NEGATIVE` if a valid
  arm leaves the registered bounded/sector envelope or fails inversion;
- `CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_MIXED` if evolution is bounded and
  reversible but any frequency, purity, amplitude, sign, or cyclic gate fails;
- `CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_EXECUTION_INVALID` for provenance,
  eigensystem, coverage, initialization, solver, or output failure.

A constructive verdict establishes classical linear-response degrees of
freedom of the selected finite object. Frequencies remain dimensionless engine
frequencies; they are not particle masses, quantum levels, or physical poles.
The independent face/edge field spectrum remains outside this campaign.

## 6. Artifacts

Produce a focused CTest, mode/arm/tick CSV and JSON summary, independent
certificate, analysis/audit, and synchronized canonical records. Production
remains unchanged.
