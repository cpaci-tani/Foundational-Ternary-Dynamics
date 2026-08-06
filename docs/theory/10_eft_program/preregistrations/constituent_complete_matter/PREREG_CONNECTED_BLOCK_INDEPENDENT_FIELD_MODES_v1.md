# FTD-0641 — Connected-block independent field modes v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** FTD-0640 verdict
`CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE`  
**Scope:** source-free transverse face/edge tangent spectrum on the FTD-0638
dressed background; matter coordinates held fixed by definition  
**Date:** 2026-07-27

## 1. Question

Does the matched oriented-face electric / oriented-edge magnetic state possess
its own divergence-free propagating modes, with phases fixed by the discrete
curl complex and `C_SPEED`, independently of the 48 matter-coordinate modes?

This campaign classifies the bare field tangent operator only. It does not
test scattering, field-induced matter motion, a dressed pole, photon ontology,
or a common matter/field cone.

## 2. Locked analytic prediction

For the source-free matched update

\[
B_{t+1/2}=B_{t-1/2}-\lambda C^TE_t,
\qquad
E_{t+1}=E_t+\lambda C B_{t+1/2},
\]

where `lambda=C_SPEED`, a transverse Fourier mode with integer wavevector
`n=(n_x,n_y,n_z)` has

\[
\sigma(n)^2=4\sum_a\sin^2\!\left(\frac{\pi n_a}{L}\right),
\]

and the exact temporal phase

\[
\Omega(n)=2\arcsin\!\left(\frac{\lambda\sigma(n)}{2}\right).
\]

The electric perturbation is constructed as `delta E=C A` from a real
single-wavevector edge potential, so discrete Gauss continuity is algebraic.
Set `delta B=0`; this is a standing oscillator with the same unique phase.

## 3. Locked arms

Use `L=17` and the FTD-0638 orientation-zero dressed field as the background.
For `n=1,2,3`, use:

- all three axis permutations of `<100>`;
- all three zero-axis permutations of `<110>`;
- `<111>`;
- two nonvanishing edge-potential polarizations per wavevector.

This gives 42 primary arms at maximum electric-face amplitude `1e-7`.
For `n=1`, the canonical `<100>`, `<110>`, and `<111>` wavevectors and both
polarizations also receive half-amplitude (`5e-8`) and sign-mirror (`-1e-7`)
controls, giving 54 arms total.

Evolve the dressed background control and the background-plus-perturbation
state separately for 256 source-free forward ticks. Subtract them at every
tick before estimating the perturbation. Then apply 256 algebraic reverse
ticks to the perturbed state. Matter records remain unchanged and are not
passed through the common-action solver.

## 4. Estimators and gates

Project each electric perturbation onto its initial normalized face mode.
Estimate the phase by the registered three-point recurrence over ticks
`1..254`. Require:

- all 54 arms initialize, complete, and reverse;
- maximum seeded face amplitude equals its target within `1e-14`;
- initial and every evolved perturbation has divergence `<=1e-12`;
- modified-energy drift of each full state and background control is
  `<=1e-12`;
- state-only field recovery is `<=1e-11`;
- primary relative phase error is `<=1e-8`;
- maximum normalized recurrence residual is `<=1e-8`;
- half/full and sign-mirror phase differences are `<=1e-8`;
- signed normalized electric trajectories agree within `1e-8`;
- the two polarizations at fixed wavevector agree within `1e-10`;
- cubic permutations at fixed family and `n` agree within `1e-10`;
- measured phase is strictly increasing from `n=1` to `n=3` inside each
  registered direction family.

## 5. Verdicts

- `CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CONSTRUCTIVE` if every gate passes;
- `CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_MIXED` if all arms remain exact,
  divergence-free, energy-preserving, and reversible but a phase, recurrence,
  amplitude, sign, polarization, cubic, or monotonicity gate fails;
- `CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CLOSED_NEGATIVE` if a valid mode
  violates bounded source-free evolution or inversion;
- `CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_EXECUTION_INVALID` for provenance,
  initialization, coverage, nonfinite output, or runner failure.

A constructive verdict establishes independent classical transverse field
modes of the selected matched discretization. It does not establish coupled
normal modes, radiation emitted by matter, photons, quantum occupation,
physical frequency units, or a continuum pole.

## 6. Artifacts

Produce one focused CTest, arm/tick CSV and JSON summary, an independent
certificate, analysis/audit, and synchronized canonical records. Production
remains unchanged.
