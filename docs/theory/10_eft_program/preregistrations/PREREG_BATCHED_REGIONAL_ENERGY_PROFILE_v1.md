# PREREGISTRATION — Batched regional-energy profile v1

**Identifier:** `FTD-0686`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date locked:** 2026-07-28  
**Production changes:** forbidden; exact observer optimization only

## Purpose and identity

FTD-0671 evaluates one regional matched-field ledger by constructing masked
face/edge fields repeatedly.  FTD-0685 showed that repeating this operation at
six radii on `L=129` is not operationally viable within the registered run
limit.  Derive a batched observer that is algebraically identical at every
radius but accumulates all regions in one component pass.

For a component mask `P_R`, adjointness gives

```text
<B,C^T P_R E> = <C B,P_R E>.
```

Therefore the FTD-0671 symmetric regional modified energy equals the sum of
local component contributions

```text
u(E_a) = E_a^2/2 - (lambda/4) E_a (C B)_a,
u(B_a) = B_a^2/2 - (lambda/4) B_a (C^T E)_a.
```

Bin these contributions at the same actual face/edge positions used by
FTD-0671, cumulatively sum them once, and read every requested radius from the
cumulative arrays.  Apply the construction to `(E0,B0)`, `(E*,B1)`, and
`(E1,B1)`; define boundary transport, source exchange, and ledger residual
exactly as FTD-0671.

## Exact qualification

- Compare batched and scalar FTD-0671 outputs at radii
  `{0,2,4,8,(L-1)/2}` for asymmetric dense fields and a local current.
- Require every energy, transport, source, update, partition, and ledger value
  to agree within `2e-12`.
- Require integer translation and proper cyclic cubic covariance within
  `2e-12`.
- Require source-free, full-volume, invalid-size, nonfinite, unordered-radius,
  noninteger-origin, and nonpositive-scale controls.
- An exact-rational certificate must prove the masked-inner-product identity
  for at least 256 masks and verify batched cumulative extraction.

No tolerance, regional definition, physical classifier, or dynamics changes.
