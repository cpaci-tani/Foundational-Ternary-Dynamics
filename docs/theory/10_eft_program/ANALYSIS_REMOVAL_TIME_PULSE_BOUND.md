# Analysis — Removal-Time Pulse Bound

**FTD ID:** FTD-0589  
**Status:** `[ANALYSIS COMPLETE]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_6_CLOSED_NEXT_COUNT_7_UNRESOLVED`

## Purpose

FTD-0588 left one universal five-source history unclosed: the residual field
after every original source had evaporated. Its envelope represented that
field as one common initial step plus five independently bounded negative
steps. That representation is exact before inequalities, but its triangle
bound ignores cancellation of the static part of each finite pulse.

## Decisive calculation

For the exact modal step response `r_n`, a source removed at `T` contributes

\[
 r_n-r_{n-T}
 =2\sec(\theta/2)\sin(T\theta/2)
  \sin((n-(T-1)/2)\theta).
\]

The constant term cancels identically. The corrected one-pulse coefficient is
therefore `P_L`, not the FTD-0586 doubled-step coefficient. At `L=65`,

```text
old doubled-step pulse envelope = 0.38662829804669041
exact finite-pulse envelope     = 0.21062758886981753
```

This is a correction to the sharpness of the bound, not to the production
recurrence.

## Hybrid history structure

At a fixed tick, all sources still present share the common temporal step and
retain the FTD-0588 `sqrt(N-r)` spatial orthogonality gain. Removed sources
carry different pulse durations, so this pass bounds them sourcewise:

\[
 H_L(N,r)=C_L\sqrt{N-r}+rP_L.
\]

The worst integer partition for both `N=6` and `N=7` is one remaining source:
`r=N-1`. On the worst registered volume,

```text
H_65(6,5) = 1.3663808891042910 < K_GENESIS
H_65(7,6) = 1.5770084779741087 > K_GENESIS
```

Thus six closes uniformly and seven is merely not excluded.

## Campaign

The observer used no geometry or schedule selected by field outcome. The
five-source shapes were the two tetrahedral chiralities plus their center; the
six-source shapes were axial octahedral orbits of radius one and two. The four
prescribed histories were permanent, synchronous at tick 16, staggered at
`4(j+1)`, and paired at `(8,8,16,16,24[,24])`. Native-unlocked histories used
the two fixed seeds per cell registered before execution.

All 96 arms passed. They recorded 176 evaporation events, no descendant
genesis, no kinematics, and maximum flux `0.11074116846428322`. Direct kernel
norms agreed with the exact Gram quadratic form, translations, and 24 proper
cubic rotations below `1e-12`.

## Epistemic result

- Exact pulse cancellation and the hybrid bound are theorem-grade.
- The four spectral values are numerical facts.
- The 96 histories are conformance measurements.
- Arbitrary `N<=6` first-descendant genesis is closed negative on the four
  registered quotients by induction.
- `N=7` remained open under this sourcewise pulse bound because it crossed
  threshold. FTD-0590 later closes that boundary by cubic-orbit coherence;
  no positive seven-source mechanism was observed or inferred.
- Reciprocal mobile matter remains closed in the frozen production dynamics.

## Artifacts

- preregistration:
  `preregistrations/PREREG_REMOVAL_TIME_PULSE_BOUND_v1.md`;
- theorem:
  `derivations/THEOREM_REMOVAL_TIME_PULSE_BOUND.md`;
- native observer:
  `engine/src/eft/removal_time_pulse_bound.cpp`;
- CTest:
  `engine/tests/test_removal_time_pulse_bound.cpp`;
- independent verifier:
  `scripts/proofs/proof_removal_time_pulse_bound.py`;
- run records:
  `engine/results/ftd_0589/windows_msvc_cpu.{json,csv}`.
