# Analysis — Nine-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0592  
**Status:** `[ANALYSIS COMPLETE]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_9_CLOSED_BY_ORBIT_COHERENCE`

## What was tested

FTD-0592 froze the FTD-0590 coefficients and asked only whether

\[
 C_L\sqrt{9-r}+Q_L\sqrt{r+\mu_Lr(r-1)}
\]

stays below the genesis threshold for every `r=0,...,9`.

The answer is yes on all four registered quotients. The maximizing partition
is `r=8`: one original source remains under the common step history and eight
removed sources contribute bounded pulses.

## Worst registered quotient

At `L=65`, the ten bounds are

| `r` | bound |
|---:|---:|
| 0 | 0.93972883426559939 |
| 1 | 1.1052793159707366 |
| 2 | 1.1907963419893157 |
| 3 | 1.2662186130411519 |
| 4 | 1.3342216064566785 |
| 5 | 1.3941647961490951 |
| 6 | 1.4435861330989059 |
| 7 | 1.4770522183956305 |
| 8 | **1.4801131737725799** |
| 9 | 1.2995309364658540 |

The decline at `r=9` is not an observed destructive-interference history. It
is the disappearance of the surviving common-step contribution in the
already-proved hybrid upper bound.

## Epistemic consequence

- The FTD-0590 inequality plus first-event induction makes the `N=9`
  conclusion theorem-grade once the registered numerical maxima are fixed.
- FTD-0591 supplies the smaller-count closure, giving the combined `N<=9`
  statement.
- The orbit constants and finite partition maxima remain numerical facts.
- The result says no frozen-sector descendant can be the event that creates
  the extra source needed for autocatalysis.
- It says nothing positive about ten sources, because `N=10` was not part of
  the locked evaluation. FTD-0593 subsequently finds the ten-source ordinary
  bound inconclusive, not positive.
- It does not reopen reciprocal mobile matter or justify a production change.

## Artifacts

- preregistration:
  `preregistrations/PREREG_NINE_SOURCE_ORBIT_COHERENCE_v1.md`;
- theorem:
  `derivations/THEOREM_NINE_SOURCE_ORBIT_COHERENCE.md`;
- C++ observer:
  `engine/src/eft/nine_source_orbit_coherence.cpp`;
- CTest:
  `engine/tests/test_nine_source_orbit_coherence.cpp`;
- independent verifier:
  `scripts/proofs/proof_nine_source_orbit_coherence.py`;
- run records:
  `engine/results/ftd_0592/windows_msvc_cpu.{json,csv}`.
