# PRE-REGISTRATION — Endpoint recoil support v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0448`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0447` cubic hop work response  
**Engine artifact:** `engine/tests/campaign_endpoint_recoil_support.cpp`  
**Campaign SHA256:** `1d99da7e96c574f60a92d160515d6f00f120b4c444d043bee1c5486b99f5b20e`  
**Helper SHA256:** `481b677799a2598cb92e80443f5f15034427330206acc8729e793d9c1bd34093`

## 1. Question

FTD-0447 fixes the direction of an isolated longitudinal exchange. For the
current site-centered field ontology, FTD-0448 asks:

> Does global cubic covariance also determine whether compensating field
> recoil is stored at the departure endpoint, arrival endpoint, or both?

## 2. Frozen counterexample family

For each Moore displacement `d`, use the exact nonzero registered total

$$
R(d)=\frac{12}{|d|^2}d,
$$

whose components are even integers on all three shells. Compare:

- source-only: `(R,0)`;
- target-only: `(0,R)`;
- midpoint split: `(R/2,R/2)`.

All are rules on labeled source/target endpoints. Apply every one of the 48
signed coordinate permutations to all 26 displacements.

## 3. Locked exact gates

- all three rules have total recoil `R` exactly;
- source-only and target-only are distinct but have identical quadratic norm;
- all three rules are globally cubic covariant across `26*48*3=3744` tests;
- endpoint exchange swaps source-only with target-only;
- midpoint split alone is invariant under endpoint exchange within the
  registered family;
- midpoint split has lower endpoint quadratic norm than either endpoint-only
  rule.

All checks use exact integers.

## 4. Locked outcomes

- `CUBIC_SYMMETRY_LEAVES_ENDPOINT_RECOIL_AMBIGUITY`: every gate passes.
- `PROTOCOL_INVALID`: any gate fails.

## 5. Interpretation boundary

The campaign proves nonuniqueness only for site-centered endpoint deposits.
It does not prove that any registered rule is physical. Endpoint exchange or a
minimum endpoint norm can select the midpoint split, but each is an additional
time-centering/variational principle, not a consequence of spatial cubic
covariance.

A link-centered half-tick recoil may encode that extra principle more naturally
than either endpoint. That candidate requires a separate reversible-event
contract.

## 6. Banned moves

- No total scale, rule family, group, norm, exchange operation, or gate may
  change after first execution.
- No midpoint rule may be called derived from cubic symmetry alone.
- No production tick changes.
