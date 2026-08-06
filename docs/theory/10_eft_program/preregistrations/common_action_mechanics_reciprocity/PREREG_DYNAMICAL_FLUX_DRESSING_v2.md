# FTD-0476 — Dynamical Flux Dressing / Wake / Release Probe v2

**Date locked:** 2026-07-25  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; ALL STRONG LABELS FAILED]`  
**Supersedes:** v1 movement-event classification only

## 1. Repair scope

Version 1 sampled the immutable production history journal only after the full
24-tick moving arm.  The bridge clears that journal at the start of every tick,
so completed movement events from earlier ticks were no longer present.  This
was an observer-harness defect: the primitive source position still showed six
face hops, while the terminal journal count incorrectly showed zero.

Version 2 makes exactly one repair: after every completed tick in the moving
arm, it accumulates that tick's movement and reaction events before the next
tick clears the journal.  The observer remains read-only.

## 2. Frozen carry-forward

Every other clause of
`PREREG_DYNAMICAL_FLUX_DRESSING_v1.md` is incorporated unchanged:

- volumes, boundaries, initial conditions, active terms, ticks, polarities,
  source-off intervention, movement velocity, and samples;
- activity, radius, near-field, alignment, divergence, mirror, support,
  energy, and neutrality estimators;
- all numerical thresholds and all classification names;
- the production scenario, admission test, observer, and frozen engine tick.

No v1 result is used to change a threshold.  In particular, the locked radial,
wake, release-radius, and release-near-fraction cutoffs remain `0.75`, `0.15`,
`2.0`, and `0.20` respectively.

## 3. Run-of-record

Write `dynamical_flux_dressing_v2.csv` and `verdict_v2.txt` under
`engine/results/ftd_0476/`.  Preserve v1 unchanged.  A valid moving history must
now reconcile its net displacement with the accumulated production movement
events and must contain no reaction events.

## 4. Pre-run source lock

SHA-256 values are recorded after successful compilation and before the first
v2 execution.

| artifact | SHA-256 |
|---|---|
| v2 campaign source | `D24A9EC9051B98313E1D5BC3645A5635151DAFB1A365665A04D6D89F28A33BCE` |
| scenario admission test | `824EDF8A12ADE1D6EFE5DD4A242DEAA80243B561B55D734BE15BC051E4643FC4` |
| read-only observer | `0B6219344470E61EBB9008BBD41D74D56EDF3A70F742296538B01B7B69C3D31E` |
| C++ scenario source | `A5FE3FD6F56E269F831E7C6806E1919D588F83EA52B64EFE0B49BA6BF52FAF6A` |
| JavaScript scenario mirror | `F9FEDF2D89E8027D6879726C9FCEF145B695E4E69B9004FBB769CD2096A266BE` |
| this preregistration, pre-lock-table | `C375EEE81C558DD97D05A28CE13C6284BEFA6D6CD1504254AFF0B58C04E58510` |
