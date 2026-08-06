# FTD-0633 — Connected-block eight-fibre static basin v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0632 verdict
`CUBIC_EIGHT_FIBRE_NECESSARY_AND_SUFFICIENT_FOR_LOCKED_CHART`  
**Scope:** repeat the fully-half static refinement with only the derived
eight-record observer chart  
**Date:** 2026-07-27

## 1. Question

Does replacing the provisional cap-two chart with the independently derived
cap-eight cubic chart remove the sole FTD-0631 obstruction and produce a
stationary, positive-translation, reversible dressed matter basin?

## 2. Frozen protocol

Repeat FTD-0631 exactly, including its two orientations, starting geometry,
four-coordinate shape box, finite-difference steps, Newton/backtracking rules,
energy functional, face normalization, common-action step, 64 forward ticks,
64 state-only inverse ticks, and every numerical gate.

The sole change is:

- minimum-energy longitudinal redressing accepts at most eight distinct
  constituent chart records at one nearest-site anchor.

The cap is fixed by FTD-0632. No term may depend on record index or
multiplicity, and no force, bond, damping, counterterm, stochastic term, or
production rule is added.

## 3. Additional fibre gates

- measured multiplicity never exceeds eight during refinement, translation
  probes, or repeated dynamics;
- any same-anchor effective separation remains `>=0.9`;
- effective positions remain pairwise distinct;
- complete cyclic state covariance remains `<=1e-9`.

All FTD-0631 gates remain unchanged: reduced gradient `<=1e-9`, all four
reduced Hessian eigenvalues `>1e-6`, all three translation curvatures
`>1e-4`, translation gradients `<=1e-9`, one-step impulse/displacement/total
momentum `<=1e-9`, 64-tick center displacement `<=1e-10`, complete-state
distance `<=1e-8`, energy drift `<=1e-12`, common residual `<=1e-10`, and
inverse recovery `<=1e-10`.

## 4. Verdicts

- `CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CONSTRUCTIVE` if every inherited
  and fibre gate passes;
- `CONNECTED_BLOCK_EIGHT_FIBRE_SYMMETRY_STATIONARY_ONLY` if reduced
  stationarity passes but translation or repeated-state gates fail;
- `CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CLOSED_NEGATIVE` if cap eight
  does not produce a positive reduced stationary point;
- `CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_EXECUTION_INVALID` for parent,
  coverage, covariance, solver, or output failure.

A constructive result licenses the full 48-coordinate adiabatic Hessian. It
does not promote the fibre into the production ontology or prove a physical
particle.

## 5. Artifacts

Produce one observer CTest, versioned JSON/CSV records, independent
certificate, analysis/audit, and synchronized canonical records. Preserve the
FTD-0631 closed-negative result and production defaults unchanged.
