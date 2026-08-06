# Audit — Removal-Time Cubic-Orbit Coherence

**FTD ID:** FTD-0590  
**Status:** `[THEOREM + NUMERICAL FACT]` +
`[CLOSED NEGATIVE — ARBITRARY ONE-TIME REMOVALS FOR N <= 7]` +
`[BOUNDARY SUPERSEDED BY FTD-0591 — N <= 8 CLOSED]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE`

## Finding

FTD-0589's seven-source opening was again inequality slack, not an observed
causal mechanism. It treated six finite pulses as mutually coherent in every
mode even though the production stencil forces their temporal factors to be
constant on cubic momentum orbits.

The exact orbit relaxation proves

\[
 |J|\le C_L\sqrt{N-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}.
\]

Exhaustive mode/displacement-orbit evaluation gives
`mu_L=0.3610...0.36274`, and the seven-source maximum remains below
`K_GENESIS` by at least `0.30210967672636913` on the registered quotients.
A first-event induction therefore forbids descendant genesis through seven
arbitrary stationary sources and arbitrary one-time removals.

## Scope controls

- The theorem is finite-volume and registered only at odd
  `L={9,17,33,65}`.
- Sources must be distinct, stationary, initially present, and removable at
  most once.
- The result is prior to first descendant genesis and excludes every other
  production mechanism named in the preregistration.
- The cubic-orbit relaxation enlarges the allowed temporal coefficients. It
  cannot underestimate the exact histories.
- No source geometry, polarity assignment, removal schedule, observation
  tick, or field amplitude was searched.
- FTD-0591 subsequently closes `N=8`; this audit never predicted that count
  capable.

## Integrity

- locked preregistration:
  `E7C766CB3AD7062452F6AC1DDD9B3DC854F0DF6BCC6B2D32B1DC402281BD7721`;
- exact nonzero mode coverage at all four volumes;
- every nonzero displacement orbit evaluated;
- C++/Python coefficient agreement within `5e-12`;
- independent proof: 72/72 PASS;
- focused CTest: PASS;
- production/default/toggle/scenario changes: none.

## Program consequence

FTD-0589 remains the canonical exact pulse-cancellation theorem, but its open
`N=7` boundary is superseded. FTD-0591 further closes the frozen causal source
sector through eight sources on the registered quotients. This still does not
produce a reciprocal force, stable localized manifested carrier, particle
pole, or physical scenario.
