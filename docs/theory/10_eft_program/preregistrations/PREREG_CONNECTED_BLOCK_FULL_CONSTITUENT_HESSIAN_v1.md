# FTD-0634 — Connected-block full constituent Hessian v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0633 verdict
`CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CONSTRUCTIVE`  
**Scope:** complete 48-coordinate adiabatic static stability of the refined
16-constituent dressed state  
**Date:** 2026-07-27

## 1. Question

FTD-0633 established stationarity and positive curvature only in three rigid
translations and four symmetry-preserving shape coordinates. Does the same
state remain a local energy minimum when every constituent is displaced
independently in all three Cartesian directions?

## 2. Frozen state and functional

Use the recorded FTD-0633 refined states at `L=17`:

- x arm `(a,b,t_outer,t_inner) = (1.4992742199186664,
  0.49947120868980366, 0.50009465475929205,
  0.50018755308199814)`;
- y arm `(1.4992742199191138, 0.49947120868992617,
  0.50009465475922343, 0.5001875530819222)`.

Use the unchanged 16 polarities, 72-edge graph, cap-eight chart, minimum-energy
longitudinal redressing, measured face normalization, and static energy
`U_binding + beta U_field`. Momentum and magnetic field are zero. Production
remains unchanged.

## 3. Estimator

Flatten the 16 effective positions in constituent-record order into 48
coordinates. With the locked step `h=2e-4`, compute:

- all 48 central first derivatives;
- all 48 diagonal second derivatives;
- all 1,128 off-diagonal second derivatives using the four-corner central
  stencil.

Diagonalize the explicitly symmetrized 48 by 48 Hessian. No projection,
regularization, optimizer, deleted mode, or fitted tolerance is allowed.

For each Cartesian unit-translation vector normalized in the 48-dimensional
Euclidean metric, compute its Rayleigh quotient and compare it with the
FTD-0633 uniform-translation curvature divided by 16. Record overlaps of every
eigenvector with the three normalized translations; these labels are
diagnostic only.

If an eigenvalue is below `-1e-5`, probe that recorded eigenvector at amplitudes
`+/-h` and `+/-2h`. A negative-mode defect is confirmed only if the symmetric
energy decrement has the registered negative sign at both amplitudes.

## 4. Gates and verdicts

Common gates:

- both x/y arms complete all `4,615` redress evaluations (center, cached
  gradient/diagonal-Hessian stencil, off-diagonal Hessian, and the six
  translation comparison evaluations; negative-mode
  probes are additional only when triggered);
- every redressing is valid with multiplicity `<=8`, same-anchor separation
  `>=0.9`, and Gauss residual `<=1e-11`;
- gradient infinity norm `<=1e-8`;
- raw Hessian antisymmetry residual `<=1e-12`;
- cyclic sorted-eigenvalue covariance `<=1e-6`;
- translation Rayleigh quotients agree with the corresponding recorded
  FTD-0633 curvature divided by 16 within `1e-5` absolute.

Verdicts:

- `CONNECTED_BLOCK_FULL_48D_ADIABATIC_BASIN_CONSTRUCTIVE` if every common gate
  passes and the minimum eigenvalue is `>1e-5` in both arms;
- `CONNECTED_BLOCK_SYMMETRY_REDUCED_FALSE_MINIMUM` if a common-gate-valid arm
  has an eigenvalue `<-1e-5` and its line probes confirm negative curvature;
- `CONNECTED_BLOCK_FULL_48D_HESSIAN_MARGINAL` if no confirmed negative mode
  exists but at least one eigenvalue is in `[-1e-5,1e-5]`;
- `CONNECTED_BLOCK_FULL_48D_HESSIAN_EXECUTION_INVALID` for provenance,
  coverage, redress, covariance, translation-consistency, eigensolver, or
  output failure.

Even a constructive result establishes only adiabatic local stability for the
selected graph/action/chart. It does not derive the graph, binding stiffness,
physical mass, particle identity, or quantum spectrum.

## 5. Artifacts

Produce one observer CTest, JSON summary, arm/eigenvalue/eigenvector/Hessian
CSV records, independent numerical certificate, analysis/audit, and
synchronized canonical records. No production or default toggle changes.
