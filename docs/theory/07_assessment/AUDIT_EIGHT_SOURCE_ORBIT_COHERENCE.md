# Audit — Eight-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0591  
**Status:** `[THEOREM + NUMERICAL FACT]` +
`[CLOSED NEGATIVE — ARBITRARY ONE-TIME REMOVALS FOR N <= 8]` +
`[BOUNDARY SUPERSEDED BY FTD-0592]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_8_CLOSED_BY_ORBIT_COHERENCE`

## Finding

The FTD-0590 orbit bound remains subcritical when evaluated at the next
preregistered source count. The uniform eight-source maximum is

\[
 \max_{0\le r\le8}
 \left[C_L\sqrt{8-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}\right],
\]

and is attained at `r=7` on every registered volume. The largest value is
`1.3473027423603405` at `L=65`, below `K_GENESIS` by
`0.16908331679163724`. First-event induction therefore closes descendant
genesis for all `N<=8` histories in scope.

## Adversarial scope controls

- The result inherits the finite-volume restriction `L={9,17,33,65}`.
- It assumes distinct, stationary, initially present sources, each removable
  at most once.
- It assumes zero initial field and excludes movement, forces, reactions,
  collisions, damping, projection, clocks, and baths.
- It concerns only the interval before a first descendant event.
- The orbit inequality enlarges allowed temporal coefficients and therefore
  cannot understate the exact history norm, but it is not an exact attainable
  source configuration.
- The numerical evaluation tests 36 removal partitions. It does not test all
  production histories by simulation; the theorem supplies that uniformity.
- The strict margins are finite-volume numerical facts, not an analytic
  all-`L` lower bound.
- FTD-0592 subsequently evaluates and closes `N=9`; this audit did not.

## Integrity

- locked preregistration:
  `F6ED8183765BCCC29427DFFBCA6074D916FEDBF7D97B557F38DD3405721D4F70`;
- C++ recomputation from the parent orbit analyzer;
- independent Python orbit reconstruction and all-partition comparison;
- verifier result: 122/122 PASS;
- no geometry, polarity, or removal-schedule search;
- no production/default/toggle/scenario change.

## Program consequence

The frozen endogenous autocatalysis branch is closed negative through eight
original sources on the registered quotients. This does not provide the
missing reciprocal force, localized carrier, particle pole, or genesis
mechanism. FTD-0592 subsequently closes `N=9`; the live boundary is `N=10`.
