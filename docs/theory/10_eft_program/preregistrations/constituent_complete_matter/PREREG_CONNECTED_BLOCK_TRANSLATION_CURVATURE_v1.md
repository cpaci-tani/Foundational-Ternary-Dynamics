# FTD-0630 — Connected-block translation curvature v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0629 verdict
`CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE`  
**Scope:** three-axis Peierls curvature of the FTD-0628 dressed fixed state and
a fully-half-phased control  
**Date:** 2026-07-27

## 1. Question

The FTD-0628 object is globally half-phased only along its body axis. Its two
transverse center phases remain at integer lattice phase, which earlier Peierls
campaigns identify as stationary maxima. Is the fixed state therefore a
translation saddle, and does a uniform half shift along the two transverse
axes put the same dressed geometry in a positive three-axis translation basin?

## 2. Frozen states

Use the unchanged FTD-0628 refined coordinates, graph, action, normalization,
and minimum-energy longitudinal Gauss redressing at `L=17`.

- `body_half`: the recorded x/y cyclic FTD-0628 geometry;
- `full_half`: the identical relative constituent geometry shifted uniformly
  by `+1/2` cell along both axes transverse to the body axis.

The shift changes no relative coordinate, charge, edge, momentum, binding
energy, action, tolerance, or ontology. It selects another global phase of the
same finite lattice object.

## 3. Estimator

For each state and each Cartesian translation axis, redress the state at
uniform offsets `0,+h,-h`, with `h=2e-4`, and compute

`g_i=[U(+h)-U(-h)]/(2h)`,

`K_i=[U(+h)-2U(0)+U(-h)]/h^2`,

where `U=U_binding+beta U_field`. The inherited fields are discarded at every
evaluation. No fit, phase scan, alternate step, or optimizer is allowed.

Run x-oriented and cyclic y-oriented copies independently.

## 4. Gates and verdicts

Common gates: all 28 redress evaluations are valid; Gauss residuals are
`<=1e-11`; chart multiplicity is `<=2`; any shared-anchor effective separation
is `>=0.9`; cyclic scalar covariance is `<=1e-8`.

The preregistered sign discriminator is:

- `body_half`: body-axis `K>1e-6`, both transverse `K<-1e-6`, and
  `|g_i|<=1e-8`;
- `full_half`: all three `K>1e-6`, `|g_i|<=1e-8`, and lower static energy than
  `body_half`.

Verdicts:

- `BODY_HALF_TRANSLATION_SADDLE_FULL_HALF_BASIN_CONSTRUCTIVE` if the sign
  discriminator and every common gate pass;
- `BODY_HALF_FULL_TRANSLATION_STABLE` if all three body-half curvatures are
  positive and common gates pass;
- `FULL_HALF_TRANSLATION_REPAIR_CLOSED_NEGATIVE` if body-half is a saddle but
  the fully-half control does not have three positive curvatures;
- `CONNECTED_BLOCK_TRANSLATION_CURVATURE_EXECUTION_INVALID` for provenance,
  coverage, redress, chart, covariance, or output failure.

A constructive fully-half result licenses a new static refinement at that
global phase. It is not itself a fixed-point, full-Hessian, mobile-particle, or
production claim.

## 5. Artifacts

Add one observer CTest, JSON and curvature/state CSV records, an independent
finite-difference/sign certificate, analysis/audit, and synchronized canonical
status records. Production remains unchanged.
