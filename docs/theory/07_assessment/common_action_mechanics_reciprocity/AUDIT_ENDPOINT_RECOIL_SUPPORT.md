# AUDIT — Endpoint recoil support

**Date:** 2026-07-24  
**Identifier:** `FTD-0448`  
**Status:** `[THEOREM — CUBIC COVARIANCE DOES NOT FIX SITE-CENTERED RECOIL SUPPORT]`  
**Verdict:** `CUBIC_SYMMETRY_LEAVES_ENDPOINT_RECOIL_AMBIGUITY`  
**Pre-registration:** [`PREREG_ENDPOINT_RECOIL_SUPPORT_v1.md`](../10_eft_program/preregistrations/PREREG_ENDPOINT_RECOIL_SUPPORT_v1.md)  
**Run of record:** `engine/results/ftd_0448/windows_msvc_cpu.csv`

## 1. Exact counterexample

FTD-0447 fixes the direction of an isolated polar work response. To ask where
equal-and-opposite field recoil lives in the current site-centered ontology,
FTD-0448 registers the exact total

$$
R(d)=\frac{12}{|d|^2}d
$$

for every Moore displacement and compares three endpoint rules:

$$
(R,0),\qquad(0,R),\qquad(R/2,R/2).
$$

All three have exactly the same total recoil. All three are globally cubic
covariant: zero failures across `26*48*3=3744` exact tests.

The source-only and target-only configurations are distinct for every
direction—the minimum squared configuration distance is `96`—but have exactly
equal quadratic norm. Consequently neither cubic covariance, total momentum,
nor that local quadratic diagnostic distinguishes them.

This is a constructive nonuniqueness proof. Spatial symmetry fixes the recoil
axis but not its temporal placement at the departure or arrival endpoint.

## 2. What selects the midpoint

Endpoint exchange swaps source-only and target-only. The half-and-half rule is
the only member of the registered family invariant under that exchange, and it
has lower endpoint quadratic norm than either endpoint-only rule.

Those facts can motivate midpoint recoil, but they require additional
principles:

- endpoint exchange treats the link as unoriented or combines spatial exchange
  with a time-reversal rule;
- minimum endpoint norm is a variational selection;
- half-and-half placement is a time-centering convention.

None follows from spatial cubic covariance alone. The production hop is
oriented, so endpoint exchange is not automatically a symmetry of the event.

## 3. Natural structural continuation

The ambiguity exists because a transaction occurring between ticks is being
forced onto integer-time site variables. A more faithful candidate is a
link-centered recoil stored at the half tick:

$$
\Pi_{a\to b}^{n+1/2}=-\Delta p_{particle}.
$$

Such a variable belongs to the spacetime transition itself rather than either
endpoint. Its reverse transaction can negate the oriented link record and
restore the prior state. This matches the staggered-time logic already used by
the selected matched Maxwell sidecar, whose magnetic field lives at a half
tick, but it is not yet part of native matter dynamics.

## 4. Correct claim boundary

FTD now has:

- exact finite hop work (FTD-0443);
- a symmetry-derived isolated force direction (FTD-0447);
- proof that site-centered recoil support remains nonunique (FTD-0448).

It does not yet have a reversible particle-plus-field transaction. A
link-centered half-tick record is the next observer-only candidate, not an
authorized production change.

## 5. Reproducibility

- campaign SHA256: `1d99da7e96c574f60a92d160515d6f00f120b4c444d043bee1c5486b99f5b20e`
- helper SHA256: `481b677799a2598cb92e80443f5f15034427330206acc8729e793d9c1bd34093`
- record SHA256: `31f0376839c18ea8329bbe2e8ffbca0bdd5a46b26ea3985ba1fb5c8d86460837`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: exact-integer algebraic observer, no production tick
- result: `CUBIC_SYMMETRY_LEAVES_ENDPOINT_RECOIL_AMBIGUITY`

No production dynamics were changed.
