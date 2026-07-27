# FTD-0432 — Native Dressed Evaporation-Hazard Audit

**Date:** 2026-07-23  
**Status:** `[DERIVED — EXACT CONDITIONAL OBSERVER]` +
`[MEASURED — OUTCOME A: DRESSED HAZARD EXPLAINS NON-EXPONENTIALITY]`  
**Scope:** mechanism validation at `L=32`; no conservation or infrared claim.

## 1. Result

FTD-0431 failed because the coupled polarity source was not a single
exponential. FTD-0432 replaces that invalid fit with the exact production
conditional evaporation probability evaluated immediately before the RNG
decision. The observer predicts both the expected Fourier-source loss and the
expected occupancy loss from the actual field-dressed state.

All locked gates pass. The conditional hazard quantitatively accounts for the
non-exponential source history under both Windows/MSVC CPU and WSL2 CUDA/GCC.

## 2. Exact observer identity

For the frozen single-substrate unit tick, the observer calls the existing
diagnostic `prepare_delta_j()` and constructs in scratch memory

\[
 \widetilde v_i=v_i+\Delta J_i,
 \qquad \widetilde J_i=J_i+\widetilde v_i.
\]

It then evaluates the exact seven-site energy and production probability

\[
 p_i=K_{\rm EVAP\_RATE}\,d\tau_i
     \exp[-E_i/K_{\rm MANIFEST}^2].
\]

No RNG value is read or advanced. The resulting source expectation is

\[
 E[S_k(t+1)|X_t]=S_k(t)-N^{-1}\sum_i s_i p_i e^{-ikx_i}.
\]

Unit tests pin the bare `p=h_k=0.1` identity, zero locked hazard, probability
bounds, and exact 32-tick state/RNG neutrality.

## 3. Execution and conditional-expectation gates

Each backend ran 51 arms and 1,632 registered transitions over the low
`<100>,n=1`, middle `<110>,n=2`, and high `<111>,n=3` modes. Isolated and
coupled arms use eight fixed seeds; locked controls use seed zero.

| Diagnostic | Locked maximum | Observed |
|---|---:|---:|
| source standardized residual, max | 6 | 2.6997 |
| source standardized residual, RMS | 2.5 | 1.0428 |
| occupancy standardized residual, max | 6 | 2.5327 |
| occupancy standardized residual, RMS | 2.5 | 1.0986 |

The standardized scale is the preregistered Bernoulli variance proxy; it is a
diagnostic, not a claim of independent sites. CPU event-journal evaporation
counts equal actual removals exactly. CPU/CUDA source and hazard records agree
at `1.1e-16` scale for complex source fields; maximum expected-removal
difference is `6.82e-13`.

## 4. Native feedback

| Mode | `min h_k` | `max h_k` | range | min mean site `p` | max mean local energy |
|---|---:|---:|---:|---:|---:|
| `<100>, n=1` | 0.0012248 | 0.0991265 | 0.0979017 | 0.0006539 | 15.5496 |
| `<110>, n=2` | 0.0019992 | 0.0922594 | 0.0902602 | 0.0012361 | 3.5722 |
| `<111>, n=3` | 0.0024860 | 0.0735984 | 0.0711124 | 0.0017307 | 1.3541 |

The hazard is neither constant nor monotone. It falls as the source builds a
field, then oscillates with the undamped native wave response. This directly
explains why FTD-0431's one-rate model failed. The effect is native to the
production equations: field energy enters the already-existing exponential
evaporation rule.

## 5. Correct scope

**Outcome A — DRESSED HAZARD EXPLAINS NON-EXPONENTIALITY.** This is a positive
mechanism result. It establishes that polarity/flux feedback dynamically
suppresses and modulates evaporation, and that the exact production hazard
predicts the stochastic one-step source changes.

It does not establish exact charge conservation, an asymptotic plateau, a
zero-momentum decay rate, a hydrodynamic pole, `U(1)`, or a common cone. The
three representative modes differ in direction and harmonic, so their hazard
ordering is not an infrared scaling result. The next campaign must hold the
mode family fixed and vary volume/momentum at a theory-defined phase of the
native pole.

## 6. Reproducibility

- preregistration:
  `PREREG_NATIVE_DRESSED_EVAPORATION_HAZARD_v1.md`
- source lock: `native_dressed_evaporation_hazard_lock.json`
- lock verifier: `proof_native_dressed_evaporation_hazard_lock.py` — 28/28
- result verifier: `proof_native_dressed_evaporation_hazard_results.py` —
  30/30
- records: `engine/results/ftd_0432/windows_msvc_cpu_L32.csv` and
  `engine/results/ftd_0432/wsl2_cuda_L32.csv`

No production source, toggle default, constant, event order, or RNG stream was
changed.
