# Audit — FTD-0734 energy-adapted mixed capture corners v1

**Status:** `[AUDIT PASS — FINITE MIXED-CORNER ROBUSTNESS CERTIFIED]`  
**Date:** 2026-07-29

## Findings

1. The protocol hash remains `E2F4F928…D2251C3`; momentum magnitude,
   half-margin construction, field scales, horizons, tolerances, and verdict
   map were not changed after output.
2. All 204 registered mixed corners satisfy the exact FTD-0733 initial
   negative-energy interval. All 12 center controls also initialize.
3. All 216 histories execute 256 forward ticks and 256 state-only inverse
   ticks. Every stored forward state remains graph-inside, below `-1e-6`, and
   nonnegative in matched field energy.
4. Common action, recoil, pair-plus-field energy, causal, Gauss, and inverse
   gates pass in every row. The largest common residual is `9.772e-14`; the
   largest inverse defect is `4.157e-11`.
5. The six hostile Stage-B selector pairs are independently reproduced from
   Stage A. All 18 `L=65` confirmations share their `L=33` classes and graph
   transition histories.
6. Matched polarity orders share all 99 Stage-A classes. Aggregate polarity
   and volume mismatches are zero.
7. The independent certificate reconstructs the two energy roots, nearest
   shell margin, half-margin target, initial pair energy, all 257-state
   histories, all selectors, and all summary extrema: `6299/6299 PASS`.
8. During pre-record execution, an unconverged sparse local residual candidate
   was incorrectly passed to physical finalization. Its intentionally empty
   scratch fields have `L=0`, causing integer division by zero in continuity
   diagnostics. Finalization now fails closed before all physical diagnostics
   unless the nonlinear root has converged. A focused regression proves the
   stopped scratch state remains nonphysical.
9. The same pre-record trace exposed a stale acceleration-cache miss:
   `3.10099e-14` residual against the locked `2e-14` gate, while the unchanged
   uncached Newton solve reached `7.89646e-15`. Cache failure now resets and
   retries the canonical equation once, recorded by explicit diagnostics. A
   poisoned-cache regression reproduces the uncached endpoint exactly.
10. Those engine corrections change neither the action, state variables,
    initial conditions, tolerances, nor acceptance gates. No run-of-record
    output existed before the fail-closed and cache-fallback corrections.
11. Finite corner survival is not an open-basin theorem, an asymptotic
    stability result, or evidence that the compact pair is a physical particle.

## Correct statement

For the selected compact-pair common action, every registered center and
energy-adapted simultaneous radial/transverse-momentum/radial-position/dynamic-
field corner remains a reversible captured complete-state history through
parent tick 384 on `L=33`, and all hostile selected classes reproduce on
`L=65`. This is certified finite local robustness inside the exact selected
energy shell.

## Verification

- protocol `E2F4F928…D2251C3`;
- runner `3F29678D…FF936F4`;
- JSON `41E0FB2E…6998889`;
- CSV `FCB930BE…8BEF947`;
- certificate `3F46ADE2…5AC2C07`, `6299/6299 PASS`;
- focused local-residual fail-closed/cache-fallback regression passes.

