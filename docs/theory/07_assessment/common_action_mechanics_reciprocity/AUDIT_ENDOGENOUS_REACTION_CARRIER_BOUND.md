# AUDIT — Endogenous Reaction-Carrier Bound

**FTD ID:** FTD-0586  
**Status:** `[THEOREM — EXACT MODAL/RECTANGULAR-PULSE BOUND]` +
`[NUMERICAL FACT — REGISTERED FINITE VOLUMES]` +
`[MEASURED — SANITIZED PRODUCTION CONFORMANCE]` +
`[CLOSED NEGATIVE — N <= 3 ENDOGENOUS AUTOCATALYSIS]` +
`[BOUNDARY SUPERSEDED BY FTD-0588/0589 — CLOSED THROUGH N=6]`  
**Date:** 2026-07-26

## Result

A finite-support manifestation pattern is not automatically self-sustaining. In the
causal single-substrate source sector, the exact production step response
gives a pointwise bound for any stationary ternary source that is either kept
or evaporated once. On the registered volumes the worst one-source pulse bound
is

\[
B_{65}=0.38662829804669041.
\]

Three arbitrarily positioned and signed sources therefore obey

\[
|J|\le3B_{65}=1.1598848941400712
<K_{\rm GENESIS}=1.5163860591519780.
\]

The remaining margin is `0.35650116501190654`. By a first-event induction,
one, two, or three sources cannot generate a new manifested site in this
sector.

## Engine result

All 96 locked endogenous arms passed over 12,288 ticks:

- zero accepted genesis events;
- 96 accepted evaporations in the pulse fixtures;
- support always remained a subset of the seed;
- maximum observed flux `0.080678735152695802`;
- zero analytic-bound excess;
- bit-exact zero velocity and remainder;
- exact observer state/RNG neutrality.

All four external magnitude-`100` controls fired genesis. The negative result
is source-mechanism specific, not a disabled threshold path.

## Correction to the externally ignited pattern story

FTD-0474 injected `12`, `20`, or `40` times `K_GENESIS` and measured
finite-support thresholded dynamics. It did not show that a small manifested
seed creates the field required to reproduce itself. FTD-0586 closes that
stronger interpretation for up to three sanitized sources in the causal
coupling sector. FTD-0587 further shows that the qualified dispersal tail has
zero genesis and depends on repeated selected Gauss projection; its current
classification is an externally prepared evaporative remnant, not an
autonomous reaction front.

The first source count not excluded by this bound is four. That statement is a
limit of the sourcewise inequality, not a positive collective mechanism.
FTD-0588 later uses exact source-character orthogonality to close all
asynchronous four-source histories and all common histories through five
sources. No amplitude was tuned and no self-maintaining front was observed.

FTD-0589 subsequently cancels the constant step pieces inside each finite
pulse and closes every arbitrary one-time-removal history through six sources.
Seven is the first count not excluded by the combined bound, not a positive
mechanism.

## Ontological consequence

The engine currently supports three different objects that must not be
conflated:

1. a prescribed ternary source with a retarded field dressing;
2. an externally ignited manifestation/reaction pattern;
3. a transported particle worldline with reciprocal momentum and energy.

The first exists in the restricted native linear sector. The second exists in
the externally seeded FTD-0474 histories. The third remains closed in the
frozen ontology. FTD-0586 supplies no bridge between them.

## Reproducibility

- protocol: `PREREG_ENDOGENOUS_REACTION_CARRIER_BOUND_v1.md`;
- theorem: `THEOREM_ENDOGENOUS_REACTION_CARRIER_BOUND.md`;
- native test: `endogenous_reaction_carrier_bound` PASS;
- independent proof: 72/72 PASS;
- run of record: `engine/results/ftd_0586/windows_msvc_cpu.json`;
- production/default/toggle/scenario changes: none.

## Verdict

`ENDOGENOUS_N_LE_3_AUTOCATALYSIS_CLOSED_BOUND_INCONCLUSIVE_AT_N_GE_4`
