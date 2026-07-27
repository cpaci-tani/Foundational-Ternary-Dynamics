# Analysis — Ten-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0593  
**Status:** `[ANALYSIS COMPLETE — INCONCLUSIVE BOUND]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_ORBIT_BOUND_INCONCLUSIVE`

## Result

The unchanged FTD-0590 inequality ceases to decide the first-event question
at `N=10`. Every registered volume maximizes at `r=9`: one original source
retains its common step history while nine removed sources contribute bounded
pulses.

At the worst registered quotient `L=65`, the eleven bounds are

| `r` | bound |
|---:|---:|
| 0 | 0.99056116640472625 |
| 1 | 1.1590233086553565 |
| 2 | 1.2480182518024923 |
| 3 | 1.3276961646318952 |
| 4 | 1.4010744686889001 |
| 5 | 1.4681114245835336 |
| 6 | 1.5275193271808085 |
| 7 | 1.5766124930336378 |
| 8 | 1.6098626498078699 |
| 9 | **1.6127738812210539** |
| 10 | 1.4320835294833276 |

## Interpretation

- The bound is rigorous and uniform; its inability to exclude an event is
  also rigorous.
- The number `1.6127738812` is an upper bound in an enlarged coefficient
  space, not an attainable engine amplitude.
- No source placement, sign assignment, removal schedule, observation tick,
  or observation site was produced.
- The positive-mechanism question therefore remains unanswered.
- The live mathematical defect is identifiable: temporal factors are still
  relaxed independently on cubic orbits even when distinct orbits have the
  same exact stencil eigenvalue `M`.

## Artifacts

- preregistration:
  `preregistrations/PREREG_TEN_SOURCE_ORBIT_COHERENCE_v1.md`;
- theorem boundary:
  `derivations/THEOREM_TEN_SOURCE_ORBIT_COHERENCE.md`;
- C++ observer/CTest:
  `engine/src/eft/ten_source_orbit_coherence.cpp`,
  `engine/tests/test_ten_source_orbit_coherence.cpp`;
- independent verifier:
  `scripts/proofs/proof_ten_source_orbit_coherence.py`;
- run records:
  `engine/results/ftd_0593/windows_msvc_cpu.{json,csv}`.
