# Audit — Ten-Source Exact Shared-M Coherence

**FTD ID:** FTD-0594  
**Status:** `[THEOREM + NUMERICAL FACT]` +
`[INCONCLUSIVE — N=10]` +
`[CLOSED NEGATIVE — SHARED-M REFINEMENT AS A DECIDER]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_SHARED_M_BOUND_INCONCLUSIVE`

## Finding

Exact shared-stencil-eigenvalue grouping does not rescue the ten-source bound.
At the decisive `L=65` quotient, every one of the 6,544 cubic mode orbits has
a distinct exact `M` key. The refined upper bound is therefore identical to
FTD-0593 and exceeds threshold by `0.096387822069076146`.

## Defect closed

The result closes one suspected bookkeeping defect: the FTD-0590 calculation
did discard cancellation between distinct cubic orbits sharing `M`. Exact
cyclotomic grouping restores that cancellation. It is too small to decide the
question and is absent entirely at `L=65`.

## Claims not licensed

- No ten-source configuration was constructed.
- No polarity or removal schedule was selected.
- No bound attainment was shown.
- No genesis, particle, reciprocal force, pole, scenario, or infrared claim
  follows.
- Super-threshold upper bounds remain logically one-sided.

## Integrity

- pre-evaluation lock:
  `F7E04AA0E1B417CC856C58C2B60A4AEABF8D81CA0B766DF5756AC4CEF8A83E25`;
- arbitrary-precision C++ cyclotomic reduction;
- independent Python integer-polynomial reduction;
- full key/multiplicity comparison, not a floating digest;
- 172/172 independent checks;
- production/default/toggle/scenario unchanged.

## Program consequence

The uniform norm path now requires a genuinely new constraint. The most local
next candidate is the simultaneous pair-distance spectrum of a finite set of
distinct sources; the exact temporal feasible set is the harder alternative.
Either requires a separate preregistration and neither may be inferred from
this inconclusive result.
