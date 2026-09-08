# Registered hydrodynamic response campaign

Date: 2026-09-08. Status: **[PREREGISTRATION — LINEAR-RESPONSE VERIFICATION]**.
Law: `phi-v2-staged-candidate-1`. Protocol: `strict-hydro-response-1`.
Prediction source: [H1 dispersion](DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md).
No preparation, horizon, observable, or acceptance band changes after this lock.

## Hypothesis

The linearized Boltzmann period map `P(k)` predicts the stroboscopic evolution
of the seven conserved moments of a small sinusoidal perturbation of the
p = 1/96 counting reference. Failure is retained as "Boltzmann closure fails
for the staged law at this preparation", not corrected.

## Locked matrix

| Factor | Values |
|---|---|
| Lattice | periodic L = 32, single polarity (offset 0), background A9 code `encode(0,+1)` in both slots, s = 0, ell = 0, phase 0 |
| Directions | (1,0,0), (1,1,0), (1,1,1) |
| Wavenumbers | m in {1, 2}; k = 2 pi m n / L |
| Modes | the seven stage-0 conserved modes, equilibrium shapes from the exact proxy V, scaled to max |phi| = 1 |
| Amplitude | epsilon = 1/4 (relative modulation of p) |
| Seeds | 8 per cell, seed = 20260907000 + index; draws are `numpy.random.default_rng(seed).random((L^3,192)) < p(1+eps phi cos(k.x))` |
| Horizon | 23 stroboscopes of 48 microticks (1104 microticks per case); slowest predicted rate 0.01018756463675881 per period, e-folds covered 0.23 (budget-capped) |
| Cases | 3 x 2 x 7 x 8 = 336; total microticks 336 x 23 x 48 = 370944 |
| Throughput basis | [probe](evidence/strict-hydro-throughput-2026-09.json), L = 32: 53.5587681364745 microticks/s |

## Observable and prediction

`m_a(n) = sum_x e^{-i k.x} sum_c w_a(c) N_c(x, 48 n)`, complex, per seed; the
report uses the seed mean and its standard error. Prediction is
`m(n) = W P(k)^n h0`, `h0 = L^3 p eps phi / 2`, computed and hashed into the
lock before execution (`predictions.json`).

## Acceptance (fixed)

Per (k, mode) cell: relative RMS deviation of the seed-mean trajectory from the
locked prediction at most 1/10; declared rate estimator (least-squares slope of
ln|m| versus n) within 2 standard errors of the seed scatter around the
predicted rate. All 42 cells pass: closure verified at this scope. Otherwise
the failing cells are the retained obstruction. No retuning.

## Execution and reproducibility

`recovery_hydro_campaign.prepare_campaign(dir, L, n)` writes preparations,
weights, locked predictions and the SHA256 lock. `run_campaign(dir)` verifies
the complete source closure, runner and inputs before and after the WSL2 GPU
run of `engine/strict/recovery_hydro/hydro_main.cpp`, which calls the accepted
CUDA `advance` unchanged. `summarize_campaign(dir)` checks receipts and writes
`report.json`. Replay directory: `engine/build_strict_hydro/campaign_v1`.
