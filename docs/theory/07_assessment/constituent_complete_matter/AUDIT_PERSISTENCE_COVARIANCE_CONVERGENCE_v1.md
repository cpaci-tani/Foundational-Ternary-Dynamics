# Audit — FTD-0728 persistence covariance convergence v1

**Status:** `[AUDIT PASS — ABSOLUTE COVARIANCE PASS / FIVEFOLD
CONVERGENCE FAIL]`  
**Date:** 2026-07-29

## Findings

1. **The absolute covariance gate closes.** Scalar-history spread is
   `5.6798055148021831e-10 < 1e-9`.

2. **The stronger registered convergence gate fails.** The tight/parent ratio
   is `0.5133`, above the required `0.2`.

3. **Every physical class is solver-stable.** Parent trapping `104/104`,
   localized dressing `0/104`, escape-control pass `0/52`, escape final
   negative `12/52`, and bound control `52/52` are unchanged.

4. **The remaining covariance sensitivity is late matter separation.** The
   maximum occurs at tick 92 in the shifted `p=0.0120`, direction `0_1_-1`
   arm.

5. **No stable-matter or volume claim advances.** Absolute covariance at this
   numerical realization does not waive the failed fivefold gate or the
   periodic-volume ambiguity.

## Correct statement

At a ten-times tighter root termination, the selected 96-tick matrix is
translation/polarity covariant below its absolute tolerance and reproduces
all physical classes, but its observed covariance defect improves only by a
factor `1.948`, not the preregistered factor five.

## Verification

- protocol `F2C1D17A…6412`;
- runner `F2294329…ABB5`;
- JSON `3E9723FE…0F7D`;
- CSV `72621EA4…6F42`;
- independent certificate `4E83074F…809F`, `103/103 PASS`;
- production defaults, tick, toggles, and scenarios unchanged.

