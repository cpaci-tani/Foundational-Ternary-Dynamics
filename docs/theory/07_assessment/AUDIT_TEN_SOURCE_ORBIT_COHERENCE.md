# Audit — Ten-Source Removal-Time Orbit Coherence

**FTD ID:** FTD-0593  
**Status:** `[THEOREM + NUMERICAL FACT]` +
`[INCONCLUSIVE — ARBITRARY ONE-TIME REMOVALS AT N=10]` +
`[OPEN — N >= 10]`  
**Date:** 2026-07-26  
**Verdict:** `TEN_SOURCE_ORBIT_BOUND_INCONCLUSIVE`

## Finding

The FTD-0590 orbit inequality does not close the separately preregistered
ten-source count. All four quotients maximize at `r=9`; the `L=65` upper bound
is `1.6127738812210539`, exceeding the production threshold by
`0.096387822069076146`.

This is a defect in the deciding power of the bound, not a demonstrated
genesis event. The calculation supplies no witness history.

## Adversarial scope controls

- The finite-volume scope remains `L={9,17,33,65}`.
- Sources are distinct, stationary, initially present, and removable once.
- Initial field is zero; movement, forces, reactions, collisions, damping,
  projection, clocks, and baths are excluded.
- The orbit relaxation permits independent bounded temporal coefficients on
  different cubic orbits even where the exact pulse law forces them to agree
  through a shared `M` eigenvalue.
- A super-threshold upper bound cannot be inverted into an existence claim.
- The previously proved `N<=9` closure is unchanged.

## Integrity

- locked preregistration:
  `10EBAFCC24B0589B975BD14E3CD4FD4508942830EA7A4FB541378655F25DC348`;
- 44 exhaustive partition evaluations;
- C++/Python reconstruction: 130/130 PASS;
- no geometry, polarity, schedule, time, or site search;
- production/default/toggle/scenario unchanged.

## Program consequence

`N=10` remains open. The only admissible immediate refinement under the locked
failure consequence is exact shared-`M` eigenshell grouping under a separately
preregistered identifier. Failure of that refinement would leave a witness
search or a different selected dynamics as a distinct, explicitly priced
branch.
