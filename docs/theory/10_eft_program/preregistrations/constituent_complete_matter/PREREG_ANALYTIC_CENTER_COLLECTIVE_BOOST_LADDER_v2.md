# FTD-0644 — Analytic-center collective boost ladder v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Physics parent:** FTD-0642 result SHA-256
`E4DCBC8F3BC0A8AE30986581C7B518F08155C28C5412697DEB01B6BECC782930`  
**Invalid predecessor:** FTD-0643
`ANALYTIC_CENTER_BOOST_EXECUTION_INVALID`  
**Scope:** arithmetic/covariance correction only; amplitudes, observables,
tolerances, and physical classifications remain those locked in v1  
**Date:** 2026-07-27

## 1. Corrections

FTD-0643 is invalid because its explicitly listed arms total 32 while its
prose and runner coverage gate state 29. This version freezes the correct
count:

- one rest arm;
- 21 positive ladder arms: seven amplitudes in each of `<100>`, `<110>`, and
  `<111>`;
- six negative mirrors at `p=0.03,0.12`;
- four high-amplitude cyclic controls.

Total: 32 arms.

FTD-0643's cyclic implementation changed only the launch direction. In this
version a cyclic control first applies `(x,y,z)->(z,x,y)` once or twice to
every constituent position about the periodic coordinate origin, preserves
constituent identity, charge, momentum labels, and graph incidence, and then
recomputes the minimum-Gauss dressing. It applies the same cyclic map to the
launch momentum. The state rotation must reproduce the FTD-0638 static energy
and analytic spectrum within `1e-9` before that arm can run.

## 2. Inherited locked protocol

All remaining content of
`PREREG_ANALYTIC_CENTER_COLLECTIVE_BOOST_LADDER_v1.md` is inherited unchanged:

- `L=17`, orientation-zero analytic center, 16 forward plus 16 state-only
  reverse common-action ticks;
- positive momentum ladder
  `{0.001875,0.00375,0.0075,0.015,0.03,0.06,0.12}` in the three canonical
  direction families;
- minimum-Gauss longitudinal dressing and zero initial magnetic field;
- the same center, mobility, hop, shape, strain, mode, soft-fraction,
  instantaneous-dressing, energy, chart/fibre, and inverse observables;
- exact/coherence gates `1e-10`, recovery `1e-9`, shape/strain `0.05`;
- high-boost gates `D_parallel>=0.75`, mobility `>=0.75`, transverse
  displacement `<=0.10`, at least 16 hops, positive final velocity,
  `F_soft>=0.95`, and `R_dress<=0.50`;
- monotonicity, sign-mirror, and whole-state cubic tolerances;
- `mobile` at mobility `>=0.5`, `pinned` at `|mobility|<=0.1`, and
  `transition` otherwise.

The verdict strings are renamed only by replacing `ANALYTIC_CENTER_` with
`ANALYTIC_CENTER_V2_` to prevent an invalid v1 result from being mistaken for
this campaign:

- `ANALYTIC_CENTER_V2_COHERENT_FINITE_DEPINNING_CONSTRUCTIVE`;
- `ANALYTIC_CENTER_V2_COHERENT_NO_THRESHOLD_AT_LADDER_RESOLUTION`;
- `ANALYTIC_CENTER_V2_COHERENT_MIXED_ONSET`;
- `ANALYTIC_CENTER_V2_DIRECTIONAL_TRANSPORT_CLOSED`;
- `ANALYTIC_CENTER_V2_BOOST_EXECUTION_INVALID`.

No raw FTD-0643 measurement changes a gate or amplitude here. No verdict
establishes a vanishing continuum threshold, inertial mass, relativistic
dispersion, radiation-free co-moving solution, physical charge, particle
pole, common cone, Lorentz recovery, or production ontology.

