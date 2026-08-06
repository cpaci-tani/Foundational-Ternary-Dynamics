# FTD-0624 — Connected-block translation stability v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0623 result SHA-256
`4E86C850BB1354EC1A9C738FF1C50B94D558528966FED2F0EE40B26B67D69926`  
**Scope:** centre-stability correction and dynamical Peierls discriminator  
**Date:** 2026-07-27

## 1. Correction being tested

FTD-0623 showed that an exactly integer-centred `w=2` object remains centred
when launched with exactly zero centre momentum. That is stationarity, not
stability. The exact FTD-0553/0555 law for the frozen quadratic coat is

\[
U_i(f)=U_0+C_iQ(f),\qquad Q(f)=f^4-\frac12 f^2,
\]

with `C_i>0`. Hence `f=0` is a local maximum and `f=+/-1/2` are chart-edge
minima. This campaign tests whether the repeated connected dynamics follows
that analytic classification under explicit centre perturbations.

## 2. Frozen object and action

Use the unchanged FTD-0622/0623 selected action:

- `L=17`, `w=2`, body orientation `x`;
- 16 exact `+1/-1` constituents and 72 frozen reference-Moore bonds;
- `kappa=1`, `dt=1`, `C_SPEED=1/sqrt(3)`;
- minimum-energy initial Gauss field, zero magnetic field and momenta;
- unchanged quadratic coat, matched face/edge update, production dispersion,
  interaction normalization, action gate `1e-10`, solve tolerance `2e-11`,
  and 48-iteration limit.

No damping, boost, reaction, graph rewiring, external force, neutralizer,
legacy force, fitted coefficient, or post-hoc correction is admitted.

## 3. Locked phase arms

Set `epsilon=1/64`. For each translation axis `i in {x,y}`, run:

1. exact integer extremum `f=0`;
2. integer-maximum perturbations `f=+epsilon,-epsilon`;
3. exact positive half-cell extremum `f=+1/2`;
4. half-cell-minimum perturbations
   `f=+(1/2-epsilon),-(1/2-epsilon)`.

There are 12 arms. Each receives eight forward steps followed by eight
state-only reverse steps. No failed arm may be replaced.

## 4. Exact static-law gate

For every initial state, independently evaluate the FTD-0621 spectral result
for `(L=17,w=2,orientation=x)`. Require

\[
|U_{\rm field}(f)-[U_0+C_iQ(f)]|\le10^{-10},
\]

where `C_i` is the registered Peierls coefficient. Also require `C_i>0`,
`U(0)>U(+/-1/2)`, and the direct half-cell difference to agree with
`C_i/16` within `1e-10`.

## 5. Per-step and trajectory gates

Every forward and reverse step must pass all unchanged common-action gates.
Across each forward trajectory require:

- total-energy drift `<=1e-9`;
- centre-subtracted shape RMS `<=0.05` cell;
- maximum squared-edge strain `<=0.10`;
- unique site projection and connected registered graph at every step;
- final state-only recovery `<=1e-8`.

Exact extrema require centre displacement and total matter momentum at every
tick `<=1e-8`. Legitimate chart relabelling at `|f|=1/2` is recorded and is
not itself failure.

## 6. Dynamical classification gates

Let `delta_i(t)` be centre displacement from the initialized phase and let
`P_i(1)` be total matter momentum after the first step.

For both `f=+/-epsilon` maximum perturbations require:

- `sign(P_i(1))=sign(f)`;
- `f*delta_i(8)>0`;
- `|delta_i(8)|>=1e-6`;
- distance from the maximum grows:
  `|f+delta_i(8)|>|f|`.

For both half-cell-minimum perturbations, with
`f_target=sign(f)/2`, require:

- `P_i(1)` points toward `f_target`;
- first-step distance to `f_target` is smaller than `epsilon`;
- distance to `f_target` never exceeds `2 epsilon` over eight ticks.

Positive/negative partners must mirror their centre-displacement and
matter-momentum histories within `1e-8`. Parallel and transverse scalar
classifications need not have equal magnitudes; the anisotropy is recorded.
Cyclic covariance is tested by rotating the parallel `+epsilon` and
`+(1/2-epsilon)` arms to body orientation `y`, adding two locked eight-step
forward/reverse controls. Total registered arms: 14.

## 7. Verdicts

- `INTEGER_MAXIMUM_UNSTABLE_HALF_CELL_MINIMUM_RESTORING`: all exactness,
  inverse, static-law, mirror, covariance, runaway, and restoring gates pass.
- `CONNECTED_TRANSLATION_STABILITY_DYNAMICS_INCONCLUSIVE`: the exact action and
  static law pass, but at least one registered dynamical classification fails.
- `CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID`: any action, energy,
  coherence, static-law, coverage, or inverse gate fails.

The first verdict corrects FTD-0623's stable-rest wording and licenses normal-
mode analysis around the half-cell-centred dressed trajectory. It does not
establish a gapless mode, fixed mass, particle pole, continuous momentum,
native graph formation, or production adoption.
