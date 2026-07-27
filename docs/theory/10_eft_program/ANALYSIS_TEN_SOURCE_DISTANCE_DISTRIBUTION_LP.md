# Analysis — Ten-Source Distance-Distribution LP

**FTD ID:** FTD-0596  
**Status:** `[ANALYSIS COMPLETE — FULL DISTANCE-SPECTRUM RELAXATION INCONCLUSIVE]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE`

## What the refinement removed

FTD-0595 retained only the exact number of axial nearest-neighbor pairs and
assigned one worst coefficient to every remaining pair. FTD-0596 instead
retains the complete cubic-orbit distance distribution and requires its
symmetrized autocorrelation to have nonnegative Fourier transform at every
momentum orbit. This enforces thousands of simultaneous spectral
compatibility inequalities without selecting a source configuration.

The improvement is material:

| `L` | FTD-0595 | FTD-0596 | reduction |
|---:|---:|---:|---:|
| 9 | 1.5596603322901916 | 1.5218539833164362 | 0.0378063489737554 |
| 17 | 1.5926213057370728 | 1.5741191331652207 | 0.0185021725718521 |
| 33 | 1.6030014362387295 | 1.5852789946030676 | 0.0177224416356619 |
| 65 | 1.6115888533818610 | 1.5932999259156457 | 0.0182889274662153 |

The active worst partition changes from nine removed sources to eight. Thus
the old two-class overcount was real, not merely formal.

## Why it still does not decide ten sources

Fourier positivity is necessary but not sufficient for a distance vector to
come from an actual unweighted `r`-point subset. The LP admits fractional
orbit counts and does not impose all higher-order intersection, integrality,
or configuration-realizability constraints. Its optimum is an upper bound,
not a candidate source geometry.

At `L=9` the remaining bound misses closure by only
`0.005467924164458182`; that proximity has no statistical or physical status.
No tolerance may be enlarged and no cut may be selected after seeing it. At
larger volumes the miss is between `0.0577` and `0.0769`, so finite-volume
proximity is not a uniform closure argument.

## What remains mathematically open

The spatial relaxation is now substantially tighter, but the FTD-0594 pulse
envelope still treats shell amplitudes independently. In an actual frozen
history, all shell phases arise from the same integer observation time and
the same integer removal times. The exact temporal feasible set is therefore
smaller than the product envelope used here.

That common-time problem was the remaining registered uniform route at the
FTD-0596 boundary. FTD-0597 subsequently locks and resolves its exact pairwise
projection, closing `N=10`. A configuration search, integral LP,
semidefinite strengthening, or new graph cut would instead be a different
branch and cannot be appended to FTD-0596.

## Artifacts

- locked protocol:
  `preregistrations/PREREG_TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_v1.md`;
- theorem boundary:
  `derivations/THEOREM_TEN_SOURCE_DISTANCE_DISTRIBUTION_LP.md`;
- observer/CTest:
  `engine/src/eft/ten_source_distance_distribution_lp.cpp`,
  `engine/tests/test_ten_source_distance_distribution_lp.cpp`;
- deterministic certificate generator:
  `scripts/proofs/generate_ten_source_distance_distribution_lp.py`;
- independent proof:
  `scripts/proofs/proof_ten_source_distance_distribution_lp.py`;
- run record and sparse certificate:
  `engine/results/ftd_0596/`.
