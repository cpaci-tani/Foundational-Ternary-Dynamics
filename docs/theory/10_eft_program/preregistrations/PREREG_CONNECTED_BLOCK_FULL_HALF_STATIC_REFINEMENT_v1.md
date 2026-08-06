# FTD-0631 — Connected-block fully-half static refinement v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Qualified parent:** FTD-0629 verdict
`CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE`  
**Candidate-generating observation:** FTD-0630 v1
`CONNECTED_BLOCK_TRANSLATION_CURVATURE_EXECUTION_INVALID`  
**Scope:** observer-only refinement of the connected 16-constituent matter
candidate at half-cell phase on all three lattice axes  
**Date:** 2026-07-27

## 1. Motivation and evidential boundary

FTD-0630 missed its locked cyclic-curvature covariance gate because a
second finite difference amplified orientation-dependent floating-point energy
roundoff. That run is retained as execution-invalid. Its large, independently
repeated sign pattern may generate a new candidate, but it is not a qualified
positive result and its thresholds are not changed here.

This protocol tests the candidate directly. The FTD-0628 relative geometry is
translated by `+1/2` cell along both axes transverse to its body axis, placing
all three one-body translation coordinates at the positive-curvature phase
identified by FTD-0630. Its four symmetry-preserving shape coordinates are
then refined and the resulting complete dressed state is tested dynamically.

## 2. Frozen construction

- `L=17`, width `2`, 16 exact ternary-polarity constituents, and the unchanged
  72-edge Moore graph;
- FTD-0628 starting shape
  `(a,b,t_outer,t_inner) = (1.4993153663084844,
  0.4994670538459639, 0.50006590532229034,
  0.50018096647517352)`;
- the inherited body-axis half-cell construction plus a uniform `+1/2`-cell
  translation along both transverse axes;
- unchanged quadratic polarity coat, minimum-energy longitudinal Gauss
  redressing, measured face normalization, binding functional, production
  relativistic dispersion, and common-action solver;
- `binding_stiffness=1`, shared-anchor chart enabled, production unchanged.

No new constituent, bond, force, damping term, counterterm, stochastic term,
or post-hoc field correction is permitted.

## 3. Refinement and diagnostics

Use the same four-coordinate central-difference Newton refinement as FTD-0628:

- gradient step `2e-5`;
- Hessian step `2e-4`;
- at most 16 Newton iterations and 10 binary backtracks;
- accepted steps must remain in the inherited shape box and strictly lower
  the fully redressed static energy;
- stop when the gradient infinity norm is `<=1e-9`.

Run independent x- and cyclic y-oriented arms. For the final states:

1. require all four reduced Hessian eigenvalues `>1e-6`;
2. measure the three uniform-translation curvatures with a new, locked
   diagnostic step `h_T=1e-3` and require each `K_i>1e-4` and
   `|g_i|<=1e-9`;
3. perform one common-action step and 64 forward plus 64 state-only inverse
   ticks from zero momentum.

The translation estimator is a new qualification diagnostic, not a rerun or
repair of FTD-0630. Its larger locked step prevents the known `O(epsilon/h^2)`
roundoff amplification from deciding covariance.

## 4. Gates and verdicts

Common gates:

- both orientations initialize and refine;
- final static energy is lower than the corresponding unrefined fully-half
  starting energy and lower than the recorded FTD-0628 body-half energy;
- Gauss residual `<=1e-11`, chart multiplicity `<=2`, and shared-anchor
  separation `>=0.9`;
- complete dressed-state cyclic covariance `<=1e-9`;
- maximum one-step constituent impulse, displacement, and total momentum are
  each `<=1e-9`;
- over 64 ticks, center displacement `<=1e-10`, complete-state distance
  `<=1e-8`, energy drift `<=1e-12`, common residual `<=1e-10`, and inverse
  recovery `<=1e-10`.

Verdicts:

- `CONNECTED_BLOCK_FULL_HALF_STATIC_BASIN_CONSTRUCTIVE` if every gate passes;
- `CONNECTED_BLOCK_FULL_HALF_SYMMETRY_STATIONARY_ONLY` if reduced refinement
  and one-step stationarity pass but translation or repeated-state gates fail;
- `CONNECTED_BLOCK_FULL_HALF_STATIC_REFINEMENT_CLOSED_NEGATIVE` if the
  candidate cannot be refined to a positive reduced stationary point;
- `CONNECTED_BLOCK_FULL_HALF_STATIC_REFINEMENT_EXECUTION_INVALID` for missing
  parent/provenance, coverage, numerical-solver, covariance, or output failure.

A constructive verdict licenses the full 48-coordinate adiabatic Hessian. It
does not establish stability outside the four-coordinate ansatz, physical
particle identity, a quantum spectrum, or production ontology.

## 5. Artifacts

Produce one observer CTest, versioned JSON and CSV run-of-record files, an
independent certificate, analysis/audit, and synchronized ledger/index/tracker
entries. Production defaults and the production tick remain unchanged.
