# Audit — Nine-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0592  
**Status:** `[THEOREM + NUMERICAL FACT]` +
`[CLOSED NEGATIVE — ARBITRARY ONE-TIME REMOVALS FOR N <= 9]` +
`[OPEN — N >= 10]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_9_CLOSED_BY_ORBIT_COHERENCE`

## Finding

The FTD-0590 orbit bound remains subcritical when evaluated at the next
preregistered source count. The uniform nine-source maximum is

\[
 \max_{0\le r\le9}
 \left[C_L\sqrt{9-r}
 +Q_L\sqrt{r+\mu_Lr(r-1)}\right],
\]

and is attained at `r=8` on every registered volume. The largest value is
`1.4801131737725799` at `L=65`, below `K_GENESIS` by
`0.036272885379397879`. First-event induction therefore closes descendant
genesis for all `N<=9` histories in scope.

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
- The numerical evaluation tests 40 removal partitions. It does not test all
  production histories by simulation; the theorem supplies that uniformity.
- The strict margins are finite-volume numerical facts, not an analytic
  all-`L` bound.
- FTD-0593 evaluates `N=10`; its ordinary bound is inconclusive and does not
  demonstrate capability.

## Integrity

- locked preregistration:
  `DDAA7FC084C3F8F146E722F15E1089FDDA83D095EB5C55D2B31823A20BD41DE8`;
- C++ recomputation from the parent orbit analyzer;
- independent Python orbit reconstruction and all-partition comparison;
- verifier result: 126/126 PASS;
- no geometry, polarity, or removal-schedule search;
- no production/default/toggle/scenario change.

## Program consequence

The frozen endogenous autocatalysis branch is now closed negative through
nine original sources on the registered quotients. This does not provide the
missing reciprocal force, localized carrier, particle pole, or genesis
mechanism. FTD-0593 prices `N=10` but does not close or realize it.
