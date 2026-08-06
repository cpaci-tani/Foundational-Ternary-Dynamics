# Analysis — Eight-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0591  
**Status:** `[ANALYSIS COMPLETE]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_8_CLOSED_BY_ORBIT_COHERENCE`

## What was tested

FTD-0591 did not invent another inequality. It froze the FTD-0590 coefficients
and asked the next integer question: whether

\[
 C_L\sqrt{8-r}+Q_L\sqrt{r+\mu_Lr(r-1)}
\]

stays below the genesis threshold for every `r=0,...,8`.

The answer is yes on all four registered quotients. The maximizing partition
is `r=7`: one original source remains under the common step history and seven
removed sources contribute bounded pulses.

## Worst registered quotient

At `L=65`, the nine bounds are

| `r` | bound |
|---:|---:|
| 0 | 0.88598484158097957 |
| 1 | 1.0480574061575600 |
| 2 | 1.1293187903985722 |
| 3 | 1.1993657508089304 |
| 4 | 1.2602749780222400 |
| 5 | 1.3102316020671925 |
| 6 | 1.3440258584608986 |
| 7 | **1.3473027423603405** |
| 8 | 1.1668702290173800 |

The decline at `r=8` is not destructive interference inferred from a chosen
history. It is the disappearance of the surviving common-step contribution
in the already-proved hybrid upper bound.

## Epistemic consequence

- The FTD-0590 inequality plus first-event induction makes the `N<=8`
  conclusion theorem-grade once the registered numerical maxima are fixed.
- The orbit constants and finite partition maxima remain numerical facts.
- The result says no frozen-sector descendant can be the event that creates
  the extra source needed for autocatalysis.
- It said nothing positive about nine sources; FTD-0592 subsequently evaluated
  and closed that count under the same inequality.
- It does not reopen reciprocal mobile matter or justify a production change.

## Artifacts

- preregistration:
  `preregistrations/PREREG_EIGHT_SOURCE_ORBIT_COHERENCE_v1.md`;
- theorem:
  `derivations/THEOREM_EIGHT_SOURCE_ORBIT_COHERENCE.md`;
- C++ observer:
  `engine/src/eft/eight_source_orbit_coherence.cpp`;
- CTest:
  `engine/tests/test_eight_source_orbit_coherence.cpp`;
- independent verifier:
  `scripts/proofs/proof_eight_source_orbit_coherence.py`;
- run records:
  `engine/results/ftd_0591/windows_msvc_cpu.{json,csv}`.
