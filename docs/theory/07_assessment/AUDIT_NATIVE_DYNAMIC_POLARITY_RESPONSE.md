# FTD-0429 — Native dynamical polarity response

**Date:** 2026-07-23  
**Status:** `[DERIVED + MEASURED — RESTRICTED NATIVE LINEAR SECTOR]`  
**Verdict:** `A_FINITE_NATIVE_IR_POLARITY_SUSCEPTIBILITY`

## 1. Correction to the charge question

FTD-0421 closed one precise proposition: no nontrivial exact additive charge
survives every production event over its preregistered local discrete feature
basis. That result does not decide whether charge is an effective dynamical
property of long-wavelength polarity/flux histories.

FTD-0429 tests the latter proposition directly. It does not add a charge
variable, Gauss projector, face-field sidecar, force, or counterterm.

## 2. Frozen native sector

The initial flux and wave velocity are zero. A globally neutral ternary square
mode is held fixed by disabling movement and reactions. The only active
production terms are

$$
\partial_t^2 J=C_{\rm wave}^2\Delta_{18}J-G_C\nabla_c s.
$$

`gauss_projection=false`, `matched_gauss_dynamics=false`, and damping, forces,
genesis, evaporation, pair production, and weak transmutation are all off.
The field response is therefore produced by the existing native
wave/state-coupling tick rather than by a constraint solve.

## 3. Independent mode derivation

For a Fourier mode `k`, let `M(k)` be the positive symbol of `-Delta_18`:

$$
M(k)=4-\frac23\sum_a\cos k_a
-\frac23\sum_{a<b}\cos k_a\cos k_b.
$$

The central gradient and divergence both have symbol `i sin(k_a)`. The static
offset of the dynamically forced oscillator is therefore

$$
Z(k)\equiv\frac{(\nabla_c\cdot J)_k}{s_k}
=\frac{G_C}{C_{\rm wave}^2}
\frac{\sum_a\sin^2 k_a}{M(k)}.
$$

Since `M(k)=|k|^2+O(|k|^4)` and
`sum sin^2(k_a)=|k|^2+O(|k|^4)`,

$$
\boxed{\lim_{k\to0}Z(k)=\frac{G_C}{C_{\rm wave}^2}=3G_C.}
$$

This is a theorem within the stated linear engine sector. The measurement is
still necessary to verify that the production tick, initialization, Fourier
convention, and time-domain pole fit realize that derivation.

## 4. Locked campaign and execution repair

Directions `<100>`, `<110>`, and `<111>`, harmonics `n=1,2,3`, both polarity
signs, and three source densities were preregistered. The time response was
fit to a constant plus sine/cosine at the exact discrete pole. No empirical
constant was targeted.

V1 completed both `L=32` full matrices, but full-volume host projections made
the `L=64` process exceed the execution ceiling. No closed `L=64` record was
produced. V2 was locked before the rerun and retained every estimator and
threshold while moving the already reproduced mirror/density controls to
`L=32`; all nine primary lower-momentum points remained at `L=64`.

Two timed-out Windows wrappers also left WSL writers alive, contaminating one
shared path with identical overlapping rows. That file is preserved as
`invalid_v1_v2_overlapping_writers.csv` and excluded from all fits. The v2 run
was repeated to an isolated path and closed with exactly nine unique rows.

## 5. Results

All 57 admitted rows pass the source-neutrality, state-immutability, forbidden-
toggle, complex-pole-fit, operator-prediction, and backend gates. The result
verifier passes `31/31` checks.

| quantity | result | locked gate |
|---|---:|---:|
| infrared fit rows | `18` | exact row contract |
| fitted `Z0` | `0.256247622955862` | positive |
| derived `3 G_C` | `0.256273629308563` | comparison value |
| relative intercept difference | `1.01479e-4` | `<= 0.01` |
| constant-model RMS | `5.90795e-5` | `<= 1e-4` |
| `Delta BIC` against `Z0=0` | `279.141656` | `>= 10` |
| worst single-mode operator error | `< 3.1e-13` | `<= 1e-7` |
| worst normalized time-fit residual | `< 1.8e-12` | `<= 1e-8` |

Polarity mirrors, source-density controls, and MSVC/CUDA-GCC reproduction pass
at `L=32`. All `L=64` direction/harmonic arms pass. The zero-intercept model is
not a viable description of these data.

## 6. What is established

Within the frozen reaction-free native linear sector,

$$
s\quad\xrightarrow{\text{native time evolution}}\quad
\nabla_c\cdot J\simeq (3G_C)s\qquad(k\to0).
$$

Primitive polarity therefore has a dynamically generated, finite infrared
closed-flux normalization. Combined with exact reaction-free transport of
signed polarity by production movement histories, this licenses the phrase
**coarse-scale emergent charge in the restricted native sector**.

This supersedes the broader wording that “native charge is absent.” The
correct closed-negative statement is only that an exact microscopic additive
generator was not found in the FTD-0421 basis across the full reaction set.

## 7. Successor result and remaining open scope

FTD-0430 executed the moving-source successor. An actual one-cell production
hop carries the same infrared coefficient, with causal support and the exact
native discrete pole/residue. The retarded transported-response sub-gate is
therefore closed positive in the reaction-free moving-source sector.

FTD-0429/0430 together still do not establish:

- exact conservation through genesis, evaporation, annihilation, or weak
  transmutation;
- a microscopic or gauge-theoretic `U(1)` generator;
- gauge redundancy or a Ward identity;
- radiation from an accelerating or continuously moving source;
- a positive photon pole or quantization;
- force feedback or the empirical normalization of electric charge;
- a common matter/flux limiting cone.

The next native charge gate is a reaction-aware slow-mode campaign testing
whether coarse blocked polarity/flux has a decay rate tending to zero in the
infrared despite the exact full-event nullspace closure of FTD-0421.

## 8. Artifacts

- v1 preregistration: `docs/theory/10_eft_program/preregistrations/PREREG_NATIVE_DYNAMIC_POLARITY_RESPONSE_v1.md`
- v2 preregistration: `docs/theory/10_eft_program/preregistrations/PREREG_NATIVE_DYNAMIC_POLARITY_RESPONSE_v2.md`
- observer: `engine/include/ftd/eft/native_dynamic_polarity_response.h`
- campaign: `engine/tests/campaign_native_dynamic_polarity_response.cpp`
- source lock: `scripts/proofs/native_dynamic_polarity_response_lock.json`
- v1 provenance lock: `scripts/proofs/native_dynamic_polarity_response_lock_v1.json`
- lock verifier: `scripts/proofs/proof_native_dynamic_polarity_response_lock.py` (`26/26`)
- result verifier: `scripts/proofs/proof_native_dynamic_polarity_response_results.py` (`31/31`)
- run manifest: `engine/results/ftd_0429/manifest.json`
