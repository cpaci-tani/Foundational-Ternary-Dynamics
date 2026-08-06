# Analysis — Ten-Source Temporal Product Capacity

**FTD ID:** FTD-0597  
**Status:** `[ANALYSIS COMPLETE — TEN-SOURCE FIRST-EVENT CLOSURE]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_TEMPORAL_PRODUCT_CAPACITY`

## The missing cancellation

The preceding bounds treated each removal-pulse product as if it could have
either sign at unit magnitude on every exact eigenshell. That is impossible
at one observation time. Every pulse value shares the same offset
`-cos((n+1/2)theta)/2`; all values lie in one interval of width one.

The consequence is asymmetric:

\[
 -\frac14\le u_i u_j\le1.
\]

A shell whose spatial character has the unfavorable sign can contribute only
one quarter of the magnitude previously assigned to it. This is why the
temporal kernel falls from approximately `0.363` to `0.273` on the large
quotients.

## Why this succeeds where FTD-0596 did not

FTD-0596 solved the spatial compatibility problem but still used an
unphysical symmetric temporal product box. FTD-0597 leaves the spatial LP
unchanged and corrects only that product projection. The maximum at `L=65`
drops from `1.5932999259` to `1.4577559408`, crossing below threshold with a
certified margin `0.0586301184`.

No source history was found and no near-threshold configuration was selected.
The proof is uniform in distinct source positions, polarities, one-time
removal ticks, observation tick, and observation site within the frozen
sector.

## What the theorem changes

The frozen linear first-event boundary moves from `N<=9` to `N<=10`. Ten
initial stationary sources cannot bootstrap an eleventh manifested source by
their native state-gradient field plus arbitrary one-time disappearance.

This strengthens a negative capacity result. It does not make the original
object mobile, reciprocal, stable, or particle-like. It also does not apply
when movement, repeated reactions, Gauss projection, nonlinear carriers, or
other excluded production phases are restored.

## Remaining boundary

The next source-count question is `N=11`, under a separate lock. The present
proof does not establish a general all-`N` asymptotic theorem. A full
higher-order common-time feasible set may further improve the bound, but it is
not needed for the ten-source closure and remains unclaimed.

## Artifacts

- protocol:
  `preregistrations/PREREG_TEN_SOURCE_TEMPORAL_PRODUCT_CAPACITY_v1.md`;
- theorem:
  `derivations/THEOREM_TEN_SOURCE_TEMPORAL_PRODUCT_CAPACITY.md`;
- C++ verifier/CTest:
  `engine/src/eft/ten_source_temporal_product_capacity.cpp`,
  `engine/tests/test_ten_source_temporal_product_capacity.cpp`;
- deterministic certificate generator:
  `scripts/proofs/generate_ten_source_temporal_product_capacity.py`;
- independent proof:
  `scripts/proofs/proof_ten_source_temporal_product_capacity.py`;
- run record and sparse certificate:
  `engine/results/ftd_0597/`.
