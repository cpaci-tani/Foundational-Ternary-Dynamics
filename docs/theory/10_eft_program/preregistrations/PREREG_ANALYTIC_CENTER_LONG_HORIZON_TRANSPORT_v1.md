# FTD-0646 — Analytic-center long-horizon transport v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0645
`ANALYTIC_CENTER_V3_COHERENT_NO_THRESHOLD_AT_LADDER_RESOLUTION`, result
SHA-256 `694D46A2EBA1D5ABC96A6525B253737359BCD43F442277F2231150DBEBE8CFD4`  
**Scope:** distinguish secular low-energy transport from long-period bounded
collective oscillation  
**Date:** 2026-07-27

## 1. Frozen dynamics and arms

Use the unchanged FTD-0645 analytic-center state, covariant modal observer,
minimum-Gauss initial dressing, common action, and exact-residual cache at
`L=17`. Run 256 forward ticks and 256 state-only reverse ticks.

Canonical positive ladders use

`p={0.001875,0.00375,0.0075,0.015}`

in `<100>`, `<110>`, and `<111>`: 12 arms. Add:

- one zero-momentum rest control;
- negative mirrors at `p=0.001875` and `p=0.0075` in all three families: six
  arms;
- whole-state/modal-basis cyclic controls at `p=0.0075`: `<010>`, `<001>`,
  `<011>`, and `<101>`: four arms.

Total: 23 arms. No amplitude, duration, or arm may be added after execution.
No reaction, collision, graph change, force branch, damping, neutralizer,
external packet, post-hoc recoil, or production toggle is admitted.

## 2. Observables

Inherit FTD-0645 center, momentum, mean velocity, hop, shape, edge strain,
energy, residual, chart/fibre, 48-mode, soft-fraction, instantaneous
minimum-Gauss dressing, sign, and cubic observables.

For each nonzero canonical arm record projected center displacement at
`t={64,128,192,256}` and the four increments `Delta_j`. Fit

`D_parallel(t)=a+v_fit*t`

over ticks `65..256` by ordinary least squares and record `R^2`. Define
`v_free` from the unchanged production dispersion and long-horizon mobility
`mu_256=D_parallel(256)/(256*v_free)`.

## 3. Exactness and coherence

Every arm must initialize, complete, remain chart/graph/fibre valid, and invert
with:

- common-action residual `<=1e-10`;
- energy drift `<=1e-9`;
- recovery `<=1e-8`;
- center-subtracted shape RMS `<=0.05`;
- squared-edge strain `<=0.05`;
- integrated soft fraction `>=0.95` for nonzero arms;
- maximum longitudinal dressing residual `<=0.50` for nonzero arms.

The rest control must keep center displacement and mean speed `<=1e-9` and
have zero hops.

## 4. Persistent-transport discriminator

A canonical nonzero arm is `persistent` only if:

- every 64-tick increment has the launch sign and magnitude at least
  `0.25*64*v_free`;
- `v_fit>=0.5*v_free`;
- `R^2>=0.98`;
- `mu_256>=0.5`;
- the final 64-tick increment has the launch sign.

It is `bounded_reversal` if any 64-tick increment has the wrong sign by more
than `0.01*64*v_free` and the final displacement magnitude is below the
maximum earlier checkpoint displacement. Other valid arms are `mixed`.

Mirror and whole-state cubic histories must agree in checkpoint displacement,
fit velocity, shape, field energy, dressing, soft fraction, hops, and recovery
within `1e-6`.

## 5. Verdicts

- `ANALYTIC_CENTER_LONG_HORIZON_TRANSPORT_CONSTRUCTIVE`: every canonical
  nonzero arm is persistent and all exact, rest, mirror, and cubic gates pass.
- `ANALYTIC_CENTER_LONG_HORIZON_MIXED`: exact/rest/mirror/cubic gates pass and
  at least one but not every canonical arm is persistent.
- `ANALYTIC_CENTER_LONG_HORIZON_BOUNDED_OSCILLATION`: exact/rest/mirror/cubic
  gates pass and every canonical arm is `bounded_reversal`.
- `ANALYTIC_CENTER_LONG_HORIZON_COHERENCE_CLOSED`: a valid trajectory fails
  shape, dressing, soft-subspace, graph/fibre, mirror, or cubic coherence.
- `ANALYTIC_CENTER_LONG_HORIZON_EXECUTION_INVALID`: provenance, coverage,
  initialization, solver, energy, inverse, fit, or output failure.

No verdict proves infinite-time transport, a zero momentum threshold, inertial
mass, a pole, continuum isotropy, relativistic dispersion, physical charge,
Lorentz recovery, or production ontology.

