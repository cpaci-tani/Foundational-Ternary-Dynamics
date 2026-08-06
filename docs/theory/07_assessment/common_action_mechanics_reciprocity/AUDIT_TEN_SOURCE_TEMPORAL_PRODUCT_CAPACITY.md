# Audit — Ten-Source Temporal Product Capacity

**FTD ID:** FTD-0597  
**Status:** `[THEOREM — EXACT TEMPORAL PAIR-PRODUCT BOUND]` +
`[THEOREM — TEN-SOURCE FIRST-EVENT COROLLARY]` +
`[NUMERICALLY CERTIFIED FACT]` +
`[CLOSED NEGATIVE — ENDOGENOUS AUTOCATALYSIS FOR N<=10 IN FROZEN SECTOR]`  
**Date:** 2026-07-26  
**Verdict:**
`ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_TEMPORAL_PRODUCT_CAPACITY`

## Finding

The FTD-0596 temporal relaxation was unnecessarily symmetric. At a common
observation time, two normalized removal pulses can have product as low as
`-1/4`, not `-1`. Correcting that defect makes all four ten-source bounds
strictly subcritical. The smallest certified margin is
`0.05863011837924281` at `L=65,r=8`.

## Theorem scope

The result excludes a first descendant genesis event from at most ten
distinct stationary original sources with arbitrary signs and arbitrary
one-time removal ticks, in the frozen wave-plus-native-source sector on the
four registered periodic quotients.

It does not exclude genesis in the full engine, repeated source creation,
moving sources, selected Gauss dynamics, reactions, collisions, external
drives, or a separately adopted nonlinear carrier. It does not construct or
qualify mobile matter.

## Adversarial checks

- The `-1/4` endpoint is not numerical: it is the sharp maximum of `ac` under
  `a,c>=0` and `a+c<=1`.
- Source polarities do not evade the bound; their pair product merely reverses
  the total signed shell sum, already covered by the outer absolute value.
- Shell signs are retained before the temporal envelope; no cancellation is
  claimed between unequal exact `M` shells unless licensed by the product
  interval.
- The spatial feasible set is exactly the preregistered FTD-0596 Delsarte
  polytope; no post-result spatial cut was introduced.
- The all-removed `r=10` value is inherited unchanged, so closure does not
  rely on an unregistered refinement of that partition.

## Integrity

- protocol locked before evaluation at SHA-256
  `7FF1D85959CE80932C3F60FBC0E39BEBC09E7567EF39724B166879F41843801D`;
- all signed-shell kernel tables independently reconstructed in C++;
- 32 sparse dual certificates independently reconstructed at 90 decimal
  digits;
- 413/413 proof checks pass;
- no configuration, polarity, history, time, or threshold search;
- no production/default/toggle/scenario change.

## Program consequence

The strongest frozen first-event closure is now `N<=10`, superseding the
FTD-0592 `N<=9` boundary. This is a negative source-capacity theorem. The
reciprocal mobile-matter candidate remains closed at its earlier algebraic
gates, and no particle or infrared milestone is reopened by this result.
