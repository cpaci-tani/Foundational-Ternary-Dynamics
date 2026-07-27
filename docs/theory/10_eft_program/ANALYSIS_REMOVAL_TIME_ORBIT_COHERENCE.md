# Analysis — Removal-Time Cubic-Orbit Coherence

**FTD ID:** FTD-0590  
**Status:** `[ANALYSIS COMPLETE]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE`

## What changed

FTD-0589 bounded each removed source separately by `P_L`, producing

\[
 C_L\sqrt{N-r}+rP_L.
\]

At `N=7`, the worst partition `r=6` crossed the genesis threshold by
`0.034`--`0.061`. That did not show a mechanism; it showed that all spatial
and temporal coherence among the six pulses had been discarded.

The exact pulse has the form

\[
 p_{n,T}(k)=B(k)u_{n,T}(M(k)),\qquad |u|\le1.
\]

The temporal factor is identical across every signed-permutation orbit of
the production stencil. Retaining the exact character cancellation inside
each orbit and relaxing only between orbits gives the pairwise coherence
constant `mu_L`.

## Decisive bound

For `r` removed sources,

\[
 |J_R|\le Q_L\sqrt{r+\mu_Lr(r-1)}.
\]

The measured coherence converges near `0.36274`, far below the sourcewise
value `1`. For six removed sources the effective squared count is therefore

\[
 6+30\mu_{65}=16.882098839184336,
\]

rather than `36`. Including the one surviving common-step source gives

```text
old FTD-0589 bound at L=65 = 1.5770084779741087
orbit-coherence bound       = 1.2142763824256086
genesis threshold           = 1.5163860591519780
strict margin               = 0.3021096767263691
```

## Why this is not a schedule search

The computation enumerates only irreducible mode and displacement orbits of
the finite translation group. The maximum is a universal pairwise operator
constant for all distinct source positions. Pulse histories are enlarged to
independent bounded coefficients on each orbit; no actual removal time or
source layout is optimized.

The maximizing displacement is nearest-neighbour axial for `L>=17`; the small
`L=9` quotient instead maximizes at `(4,4,4)`. These are witnesses for the
universal coefficient, not selected seven-source geometries.

## Epistemic result

- The orbit Gram inequality and first-event induction are theorem-grade.
- The four exhaustive orbit norms are numerical facts.
- The result closes first descendant genesis for arbitrary one-time-removal
  histories through seven sources on the registered quotients.
- FTD-0591 subsequently proves the same bound remains subcritical at eight
  sources; neither result shows a positive genesis mechanism.
- It does not repair the frozen reciprocal mobile-matter failure.

## Artifacts

- preregistration:
  `preregistrations/PREREG_REMOVAL_TIME_ORBIT_COHERENCE_v1.md`;
- theorem:
  `derivations/THEOREM_REMOVAL_TIME_ORBIT_COHERENCE.md`;
- C++ observer:
  `engine/src/eft/removal_time_orbit_coherence.cpp`;
- CTest:
  `engine/tests/test_removal_time_orbit_coherence.cpp`;
- independent verifier:
  `scripts/proofs/proof_removal_time_orbit_coherence.py`;
- run records:
  `engine/results/ftd_0590/windows_msvc_cpu.{json,csv}`.

The open count stated by this analysis is superseded by FTD-0591 and FTD-0592.
The next unevaluated integer is `N=10`.
