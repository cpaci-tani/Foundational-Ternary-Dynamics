# PREREGISTRATION — Causal excitation separation L113 v1

**Identifier:** `FTD-0690`  
**Status:** `[LOCKED BEFORE IMPLEMENTATION AND EXECUTION]`  
**Date:** 2026-07-28  
**Production changes:** forbidden

## Scope

Run the first executable extension beyond the existing tick-80 causal record.
This is not a replacement for the resource-invalid `L=129` campaign and does
not establish an infinite-volume limit.

## Frozen changes from FTD-0689

- volume `L=113`;
- fixed integer origin `(56,56,56)`;
- horizon `T=96` and conservative image-contact tick `113-2*8=97`;
- late-local window `80..96`;
- print one nonphysical progress checkpoint after every completed forward and
  reverse tick.  Do not write physical tick data until the complete verdict is
  written.

## Frozen inheritance

Keep the exact FTD-0689 rest state, internal modes `{6,7}`, both polarities
plus control, fresh `p_max=1.25e-7`, FTD-0683 component geometry, FTD-0688
prefix-sum regional observer, radii `{8,16,24,32,40,48}`, outward-arrival
threshold `0.001`, center tolerance `1e-12`, bitwise initial fields, target
schema, exact gates, polarity gates, state-only inversion, and factorized
spatial/late classifiers.

The late plateau definition is unchanged except for the explicitly shortened
window: mean at least `0.01`, coefficient of variation at most `0.10`, and
relative slope at most `0.001` for both core and radius-eight field.

Output goes to `engine/results/ftd_0690/`.  Runner, embedded source, observer,
and binary hashes must be recorded before invocation.  Failure to complete is
execution-invalid and yields no physical classification.
