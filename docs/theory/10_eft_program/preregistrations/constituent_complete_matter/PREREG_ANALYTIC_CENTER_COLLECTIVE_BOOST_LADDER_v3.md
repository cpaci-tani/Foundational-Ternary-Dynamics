# FTD-0645 — Analytic-center collective boost ladder v3

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Physics parent:** FTD-0642 result SHA-256
`E4DCBC8F3BC0A8AE30986581C7B518F08155C28C5412697DEB01B6BECC782930`  
**Closed predecessor:** FTD-0644
`ANALYTIC_CENTER_V2_DIRECTIONAL_TRANSPORT_CLOSED`  
**Scope:** soft-subspace covariance-observer correction only  
**Date:** 2026-07-27

## 1. Isolated defect

FTD-0644's independent certificate shows that its `5.1404e-6` cubic failure
comes only from evaluating rotated trajectories in the unrotated analytic
eigenbasis. Center, momentum, shape, field energy, dressing, hops, recovery,
static energy, and spectrum all satisfy their locked cubic gates.

## 2. Single correction

In each cyclic arm, compute the analytic Hessian at the already preregistered
whole-state-rotated reference. Use its own mass-normalized eigenvectors to
project that arm's 48 coordinate histories and form

`F_soft=sum_{a=0}^5 Q_a^2/sum_{a=0}^{47}Q_a^2`.

The eigenvalues must still match the canonical spectrum within `1e-9`. Only
the six-dimensional soft-subspace fraction is compared across orientations;
individual eigenvectors inside degenerate eigenspaces are not compared.

## 3. Inherited protocol

Every state, amplitude, direction, arm, tick count, solver setting, observable,
threshold, tolerance, mirror, and physical classification in FTD-0644 is
inherited unchanged. There are exactly 32 arms and 16 forward plus 16
state-only reverse ticks per arm.

Verdict strings replace `ANALYTIC_CENTER_V2_` with `ANALYTIC_CENTER_V3_`:

- `ANALYTIC_CENTER_V3_COHERENT_FINITE_DEPINNING_CONSTRUCTIVE`;
- `ANALYTIC_CENTER_V3_COHERENT_NO_THRESHOLD_AT_LADDER_RESOLUTION`;
- `ANALYTIC_CENTER_V3_COHERENT_MIXED_ONSET`;
- `ANALYTIC_CENTER_V3_DIRECTIONAL_TRANSPORT_CLOSED`;
- `ANALYTIC_CENTER_V3_BOOST_EXECUTION_INVALID`.

No raw predecessor value changes a gate. No verdict establishes a vanishing
continuum threshold, inertial mass, relativistic dispersion, radiation-free
co-moving solution, physical charge, particle pole, common cone, Lorentz
recovery, or production ontology.

