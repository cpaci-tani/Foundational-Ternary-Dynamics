# FTD-0431 — Native Reaction Polarity Slow-Mode Audit

**Date:** 2026-07-23  
**Status:** `[MEASUREMENT — OUTCOME D: INVALID ANALYSIS MODEL]` +
`[DYNAMIC FEEDBACK OBSERVED — DESCRIPTIVE ONLY]`  
**Scope:** frozen production evaporation acting on the FTD-0429/0430 coarse
polarity source, with movement, genesis, pair production, annihilation, weak
transmutation, both Gauss mechanisms, damping, and forces off.

## 1. Question and locked discriminator

FTD-0429 and FTD-0430 establish a finite retarded polarity/flux response only
when reactions are disabled. FTD-0431 asked whether the same source-bearing
mode remains hydrodynamically slow when native evaporation is enabled.

The v1 preregistration fixed an isolated-evaporation calibration, a coupled
wave/coupling/evaporation arm, and a locked control. Its primary estimator was
the ordinary-least-squares slope

\[
  \log A(t)=c-\gamma_k t,\qquad t=0,\ldots,6,
\]

with a normalized RMS gate of `0.02`. Only if every ensemble-mean time fit
passed that gate could the `L=32,64` infrared intercept comparison be used.

## 2. Instrumentation and execution

`native_reaction_polarity_slow_mode.h` is a read-only Fourier observer. The
campaign initializes a globally neutral dense square-wave source, records
`S_k`, `div J_k`, occupancy, and signed state for ticks `0,...,16`, and uses
the immutable CPU event journal to require exact equality between accepted
evaporation events and occupancy loss. No production equation, RNG call, event
order, toggle default, or ontology type was changed.

| Record | Matrix | Result |
|---|---:|---|
| Windows/MSVC CPU, `L=32` full | 153 arms / 2,601 rows | execution valid |
| WSL2 CUDA/GCC, `L=32` full | 153 arms / 2,601 rows | execution valid |
| WSL2 CUDA/GCC, `L=64` infrared | partial only | invalid/excluded |

The source-locked `L=64` implementation constructed and host-observed one
fresh `64^3` bridge per arm. A clean single-process attempt had flushed only
13 complete arms plus 12 ticks of the next arm after 34 minutes. Because the
mandatory `L=32` analysis gate had already failed, the run was stopped and its
233-row partial CSV retained only as invalid provenance. Earlier wrapper
timeouts and one detected orphan/overlapping-writer attempt supplied no
admitted record. The two admitted files and the excluded partial file are
hash-locked in `engine/results/ftd_0431/manifest.json`.

## 3. Controls that passed

- Every admitted arm has the exact registered ticks, modes, seeds, backend,
  profile, and toggle contract.
- CPU history counts equal occupancy loss event by event; no other reaction
  event appears.
- Locked controls lose no sites and retain the source mode below `1e-14`.
- Isolated arms have zero divergence below `1e-14`.
- Coupled arms activate the field above `1e-8` by tick 2.
- The exact production recurrence

  \[
  D_{t+1}=(2-C_{\rm WAVE}^2M_{18})D_t-D_{t-1}
          +G_C\sum_a\sin^2(k_a)S_t
  \]

  closes with maximum normalized residual `2.41e-13` on both backends.
- The isolated calibration gives ensemble
  `gamma=0.105685...` to `0.106113...`, at most `0.715%` from
  `-log(0.9)=0.1053605...`, inside the locked 2% tolerance.
- Windows CPU and WSL2 CUDA coupled early-time slopes agree exactly at the
  recorded precision for all nine modes.

## 4. Decisive failure

The coupled source is not a single exponential over the preregistered window.
Its normalized RMS residual ranges from `0.04288` to `0.22273`, while the
locked maximum is `0.02`. Therefore the primary time-fit is invalid and the
infrared models `M_0` and `M_cons` are inadmissible. No decay intercept, BIC,
or conservation verdict may be calculated from FTD-0431.

The behavior causing the failure is reproducible but descriptive. For the
lowest axial `L=32` mode, the phase-referenced source falls from `1` to
`0.473561` by tick 16 while its successive survival ratios rise toward one.
Across the nine modes the forced early exponential slopes range from
`0.01928` to `0.08460`; these are not valid asymptotic decay rates. The native
mechanism is visible in the frozen equations: generated flux contributes to
the local energy used by evaporation, and the probability contains
`exp(-local_energy/K_MANIFEST^2)`. Field dressing therefore suppresses later
evaporation and bends the source history away from one-rate decay.

## 5. Adjudication

**Outcome D — INVALID ANALYSIS MODEL.** FTD-0431 does not close emergent charge
negative. It also does not establish a conserved or asymptotically stable
charge. It demonstrates that reaction kinetics cannot be assessed by applying
the isolated evaporation rate to an undressed source: native source/field
feedback materially changes the history.

The correct successor is a separately preregistered hazard measurement. It
must observe the production evaporation probability or expected source-mode
loss after field dressing, include late-time/censoring controls, and test its
momentum and volume scaling without calling a finite-time plateau conserved.
Genesis, annihilation, and weak transmutation remain separate later event
classes; they are not inferred from this evaporation-only result.

## 6. Reproducibility

- preregistration:
  `PREREG_NATIVE_REACTION_POLARITY_SLOW_MODE_v1.md`
- source lock: `native_reaction_polarity_slow_mode_lock.json`
- lock verifier: `proof_native_reaction_polarity_slow_mode_lock.py` — 30/30
- result verifier: `proof_native_reaction_polarity_slow_mode_results.py` —
  27/27
- admitted records: Windows/MSVC CPU and WSL2 CUDA/GCC `L=32` full CSVs
- invalid provenance: partial WSL2 CUDA/GCC `L=64` CSV

This audit changes no FTD-0421, FTD-0429, or FTD-0430 theorem/measurement
status. It narrows the next reaction-aware question to the native dressed
hazard rather than a presumed exponential source mode.
