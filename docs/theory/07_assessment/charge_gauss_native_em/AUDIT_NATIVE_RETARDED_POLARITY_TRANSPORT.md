# FTD-0430 — Native retarded polarity transport

**Date:** 2026-07-23  
**Status:** `[DERIVED + MEASURED — REACTION-FREE MOVING-SOURCE SECTOR]`  
**Verdict:** `A_RETARDED_NATIVE_COARSE_POLARITY_RESPONSE`

## 1. Question

FTD-0429 established a finite long-wavelength susceptibility for a stationary
ternary source in the native wave/coupling sector. It did not show that an
actual moving source carries that response with it.

FTD-0430 moves a sparse neutral polarity pair through the production
`phase_movement` and compares it with an otherwise identical locked pair. It
asks whether the difference field is retarded, follows the unmodified native
wave pole, and has the same infrared coefficient. It introduces no field,
Gauss solve, matched-field extension, force, or counterterm.

## 2. Frozen production experiment

Only `wave_propagation`, `coupling`, and `movement` are enabled. Both
`gauss_projection` and `matched_gauss_dynamics` are off, as are damping,
reactions, forces, stochastic terms, and alternate wave integrators. Flux and
wave velocity start at zero.

The moving and stationary arms begin with the same neutral pair. In the moving
arm both polarities are primed to hop one cell in `+x` during the real movement
phase at the end of tick 1. They are then locked. The CPU journal records
exactly two movement events and zero reaction events. The CUDA paths reproduce
the exact state displacement and unchanged total signed state.

For every Fourier mode the measured quantities are

$$
\Delta s_k=s_{k,\mathrm{moving}}-s_{k,\mathrm{stationary}},
\qquad
\Delta D_k=(\nabla_c\cdot J_{\mathrm{moving}})_k
          -(\nabla_c\cdot J_{\mathrm{stationary}})_k,
$$

and `R_k(tau)=Delta D_k/Delta s_k`, where `tau=0` is immediately after the
hop. The nine locked modes use directions `<100>`, `<110>`, `<111>` and
harmonics `n=1,2,3`.

## 3. Exact moving-step prediction

After the hop, the difference source is constant. In each Fourier mode the
native kick-drift update obeys

$$
q_{\tau+1}-2q_\tau+q_{\tau-1}+\Omega_k^2q_\tau=F_k,
\qquad
\Omega_k^2=C_{\rm wave}^2M_{18}(k),
$$

with `cos omega_k=1-Omega_k^2/2`. The static particular solution is therefore
the same coefficient derived in FTD-0429,

$$
Z_{\rm exact}(k)=\frac{G_C}{C_{\rm wave}^2}
\frac{\sum_a\sin^2 k_a}{M_{18}(k)}.
$$

The zero-field initial conditions fix the complete step response to

$$
R_k(\tau)=Z_k-Z_k\cos(\omega_k\tau)
 +Z_k\tan(\omega_k/2)\sin(\omega_k\tau),
$$

so its oscillatory amplitude has the independent residue identity

$$
\frac{\sqrt{|B_k|^2+|C_k|^2}}{|Z_k|}
=\frac{1}{\cos(\omega_k/2)}.
$$

This is an exact result for the stated linear difference sector. The campaign
checks that the production hop, phase ordering, CPU/CUDA implementations, and
read-only observer realize it.

## 4. Causal-support result

Movement occurs after field evolution on tick 1. Accordingly, the divergence
difference at `tau=0` is exactly zero. At `tau=1` its maximum magnitude is
`0.128136814654282`.

The source gradient has range one and the measured central divergence adds one
cell. Each later 18-point Laplacian step adds at most one Moore shell. The
locked support bound is therefore `r_infinity <= tau+1`. Across every sampled
tick and every admitted arm:

- `max |Delta div J|(tau=0) = 0`;
- `max` signal outside the dependency cone is `0` at the `10^-13` support
  threshold;
- the response expands to the periodic half-box rather than remaining a local
  dressing.

This is the exact dependency cone of the production map. It is not an
empirical identification with the measured speed of light.

## 5. Preregistration defect and clean rerun

V1 completed at `L=32,64`, but its infrared section contained a contradictory
feature definition. It said it reused the FTD-0429 regression while writing
`h4=sum k_a^4`; the locked FTD-0429 model defines
`h4=(sum k_a^4)/q2`. The literal v1 feature gives RMS `2.0448e-3`, failing the
locked `10^-4` gate. V1 is booked `D_INVALID_ANALYSIS_SPECIFICATION`; all three
CSV hashes and the v1 source lock are preserved.

V2 restored the single already-locked FTD-0429 definition and used entirely
new volumes: full mirror/backend controls at `L=48` and infrared CUDA data at
`L=96`. No v1 scalar entered the v2 fit or verdict.

After the run, the campaign's no-argument CTest default was changed from the
obsolete v1 `L=32` to the locked v2 `L=48`. The v2 lock records this exact
hash as a qualified harness-only successor; all explicit production runs,
estimators, thresholds, and admitted CSVs predate and are unaffected by it.

## 6. V2 results

The result verifier passes `38/38` checks.

| quantity | result | locked gate |
|---|---:|---:|
| admitted v2 rows | `45` | exact row contract |
| fitted `Z0` | `0.256268547570661` | positive |
| derived `3G_C` | `0.256273629308563` | comparison value |
| relative to `3G_C` | `1.98297e-5` | `<= 0.01` |
| FTD-0429 `Z0` | `0.256247622955862` | independent comparison |
| relative to FTD-0429 | `8.16511e-5` | `<= 0.002` |
| constant-model RMS | `1.18846e-5` | `<= 1e-4` |
| `Delta BIC` against `Z0=0` | `336.875805` | `>= 10` |
| worst single-mode `Z_exact` error | `< 5.90e-14` | `<= 1e-6` |
| worst normalized time-fit residual | `< 1.11e-13` | `<= 1e-7` |
| worst step-residue error | `< 5.56e-14` | `<= 1e-5` |
| worst MSVC/CUDA relative difference | `< 3.1e-15` | `<= 1e-5` |
| polarity-mirror difference | `0` | `<= 1e-5` |
| signal outside dependency cone | `0` | `<= 1e-11` |

## 7. What is established

Within the frozen reaction-free native sector, actual movement of primitive
polarity transports the source of the same coarse long-wavelength field
response measured for a stationary source:

$$
\Delta s_{\rm moved}
\quad\xrightarrow{\text{production tick}}\quad
\Delta(\nabla_c\cdot J),
\qquad
Z(k\to0)=3G_C.
$$

The response begins only after the movement phase has changed the source,
remains inside the local dependency cone, and carries the exact native pole
and step residue. The result upgrades “static susceptibility” to
**retarded transported coarse polarity response** in this restricted sector.

## 8. What is not established

FTD-0430 does not establish:

- an exact microscopic additive charge across production reactions;
- conservation through genesis, evaporation, annihilation, or weak
  transmutation;
- a microscopic `U(1)` generator, gauge redundancy, or a Ward identity;
- a freely propagating quantized photon or positive photon spectral density;
- radiation from an accelerating source;
- a force law or back-reaction on matter;
- a charged matter pole or a common matter/flux limiting cone;
- Lorentz invariance or empirical signal-speed normalization.

The source is locked after one hop. Consequently the result establishes causal
transportability of the native polarity susceptibility, not electromagnetic
radiation or a complete dynamical charge theory.

## 9. Next gate

The exact full-event additive generator remains closed negative by FTD-0421,
but that does not exclude a long-lived coarse reaction-sector mode. The next
native charge gate is therefore reaction-aware and dynamical: measure whether
blocked polarity/flux histories under genesis, evaporation, annihilation, and
weak transmutation possess a volume-stable slow mode whose decay rate tends to
zero as `k -> 0`. A nonzero infrared decay intercept closes emergent conserved
charge across reactions even if the reaction-free response survives.

## 10. Artifacts

- v1 preregistration: `docs/theory/10_eft_program/preregistrations/PREREG_NATIVE_RETARDED_POLARITY_TRANSPORT_v1.md`
- v2 preregistration: `docs/theory/10_eft_program/preregistrations/PREREG_NATIVE_RETARDED_POLARITY_TRANSPORT_v2.md`
- observer: `engine/include/ftd/eft/native_retarded_polarity_response.h`
- unit: `engine/tests/test_native_retarded_polarity_response.cpp`
- campaign: `engine/tests/campaign_native_retarded_polarity_transport.cpp`
- v1 lock: `scripts/proofs/native_retarded_polarity_transport_lock_v1.json`
- v2 lock: `scripts/proofs/native_retarded_polarity_transport_lock.json`
- lock verifier: `scripts/proofs/proof_native_retarded_polarity_transport_lock.py` (`31/31`)
- result verifier: `scripts/proofs/proof_native_retarded_polarity_transport_results.py` (`38/38`)
- run manifest: `engine/results/ftd_0430/manifest.json`
