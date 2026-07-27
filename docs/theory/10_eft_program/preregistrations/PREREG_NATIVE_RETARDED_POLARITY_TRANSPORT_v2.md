# PRE-REGISTRATION — Native retarded polarity transport v2

**Date locked:** 2026-07-23  
**Identifier:** `FTD-0430`  
**Status:** `[PRE-REGISTRATION — VERSIONED AFTER INVALID v1 ANALYSIS SPECIFICATION]`  
**Supersedes for execution:** `PREREG_NATIVE_RETARDED_POLARITY_TRANSPORT_v1.md`

## 1. Reason for versioning

All three v1 execution files completed before the infrared regression was
evaluated. The local mode, causal-cone, exact-pole, residue, mirror, and backend
checks passed. During evaluation, the v1 infrared section was found to contain
an internally contradictory feature definition:

- it says the fit is "the same predeclared pair used by FTD-0429";
- it then defines `h4=sum_a k_a^4`;
- the locked FTD-0429 v1 preregistration and result verifier define
  `h4=(sum_a k_a^4)/q2`.

The literal unnormalized v1 feature gives RMS `2.0448e-3`, above the locked
`10^-4` gate, and a continuum intercept differing from FTD-0429 by `6.44e-3`,
above the locked `2e-3` gate. V1 is therefore outcome D at the analysis-
specification layer. Its completed files are preserved as invalid-run
provenance and may not enter the v2 decision fit.

V2 corrects the transcription to the already-published, source-locked FTD-0429
feature definition. Because the v1 outputs were already visible, v2 uses new,
non-overlapping volumes. No v1 scalar measurement is reused for the v2 verdict.

## 2. Normative inheritance

Sections 1–5 and 7 of v1 remain normative except for the execution-matrix
amendment below. In particular v2 retains exactly:

- the production one-cell hop and stationary counterfactual;
- the frozen `wave_propagation + coupling + movement` sector;
- both Gauss mechanisms OFF;
- the nine direction/harmonic modes;
- the read-only Fourier and causal-support estimators;
- the exact native pole, susceptibility, and step-residue predictions;
- every local `10^-8`, `10^-7`, `10^-6`, `10^-5`, and `10^-11` gate;
- the four outcome interpretations and all explicit non-claims.

## 3. Corrected locked infrared model

For the positive-orientation WSL2 v2 rows, define

\[
 q^2=\sum_a k_a^2,
 \qquad
 h_4=\frac{\sum_a k_a^4}{q^2}.
\]

The locked models are exactly the FTD-0429 models:

\[
 M_0: Z=Z_0+Aq^2+Bh_4+Cq^4,
 \qquad
 M_\emptyset: Z=Aq^2+Bh_4+Cq^4.
\]

Advancement retains the v1 thresholds without change:

- `Delta BIC = BIC_empty-BIC_0 >= 10`;
- RMS residual of `M_0 <= 10^-4`;
- `|Z_0-3G_C|/(3G_C) <= 0.01`;
- `|Z_0-Z_0^(FTD-0429)|/Z_0^(FTD-0429) <= 0.002`.

The correction is not selected from a menu of successful regressions. It is
the single feature definition locked in
`PREREG_NATIVE_DYNAMIC_POLARITY_RESPONSE_v1.md` section 6 and implemented in
`proof_native_dynamic_polarity_response_results.py` before FTD-0430 existed.

## 4. New execution matrix

- `L=48`, profile `full`: orientations `+1` and `-1`, Windows/MSVC CPU and
  WSL2 CUDA/GCC.
- `L=96`, profile `infrared`: orientation `+1`, WSL2 CUDA/GCC.

The exact same source fractions `A=(L/4,L/2,L/2)` and
`B=(5L/8,L/2,L/2)` are used. Both sizes are divisible by eight. The Windows
and WSL2 `L=48` results must agree mode by mode within `10^-5`; polarity
orientations must agree within `10^-5`.

The campaign accepts only the locked pairs `L=48/full` and
`L=96/infrared`. A profile that omits or adds an arm is invalid. V1 files at
`L=32,64` are provenance only and are excluded from every v2 fit and verdict.

## 5. Lock consequence

The v1 source lock and its completed CSV hashes are preserved. The amended
campaign volume/profile guard, this document, and all unchanged upstream
sources receive v2 hashes before the first `L=48` or `L=96` execution.
