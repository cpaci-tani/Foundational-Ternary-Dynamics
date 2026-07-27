# Analysis — Ten-Source Pair-Distance Capacity

**FTD ID:** FTD-0595  
**Status:** `[ANALYSIS COMPLETE — TWO-CLASS REFINEMENT INCONCLUSIVE]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_PAIR_DISTANCE_BOUND_INCONCLUSIVE`

## What the refinement removed

FTD-0594 allowed every removed-source pair to attain the global nonzero
displacement coherence independently. FTD-0595 enforces one simultaneous
geometric fact: among nine distinct cubic sites, at most 13 of the 36 pairs
can be axial nearest neighbors. The exact capacities are obtained without
choosing a source history or optimizing against `K_GENESIS`.

## Why it was not decisive

The complement class remains broad. On the largest quotient,

\[
 \kappa_1-\kappa_2=0.00139166242971528,
\]

so replacing at least 23 of the 36 pair coefficients by `kappa_2` barely
changes the Gram norm. The `L=65,r=9` bound moves only from
`1.6127738812210539` to `1.6115888533818610`, while closure requires it below
`1.5163860591519780`.

The nonaxial maximizer is the antipodal body-diagonal class on `L=17,33,65`.
Nine distinct sites cannot generally place all nonaxial pairs at that same
displacement either. FTD-0595 intentionally does not exploit this further:
doing so would require the complete simultaneous distance histogram, not a
post-evaluation third class.

## Remaining mathematical relaxations

Two genuine enlargements remain:

1. the two-class bound assigns `kappa_2` independently to every nonaxial pair,
   ignoring the complete distance-spectrum constraints of a finite site set;
2. the shared-shell pulse coefficients are bounded independently, ignoring
   the common integer-time trigonometric orbit that generates them.

The first is an exact finite extremal problem over distance histograms. The
second is a simultaneous phase-feasibility problem. FTD-0595 closes the
two-class axial-capacity branch as a decider; neither remaining problem is
resolved by this result.

## Artifacts

- locked protocol:
  `preregistrations/PREREG_TEN_SOURCE_PAIR_DISTANCE_CAPACITY_v1.md`;
- theorem boundary:
  `derivations/THEOREM_TEN_SOURCE_PAIR_DISTANCE_CAPACITY.md`;
- observer/CTest:
  `engine/src/eft/ten_source_pair_distance_capacity.cpp`,
  `engine/tests/test_ten_source_pair_distance_capacity.cpp`;
- independent proof:
  `scripts/proofs/proof_ten_source_pair_distance_capacity.py`;
- run record:
  `engine/results/ftd_0595/windows_msvc_cpu.{json,csv}`.
