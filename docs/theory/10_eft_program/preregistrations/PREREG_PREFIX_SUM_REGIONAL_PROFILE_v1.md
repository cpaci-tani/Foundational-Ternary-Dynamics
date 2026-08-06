# PREREGISTRATION — Prefix-sum regional profile v1

**Identifier:** `FTD-0688`  
**Status:** `[LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-28

FTD-0686 is algebraically correct but still adds each component contribution
to every containing requested radius.  FTD-0687 emitted no checkpoint or
result file before resource termination at approximately thirty minutes.

Replace only the accumulation algorithm: add each local component energy once
to its exact doubled-Chebyshev-radius bin, form one cumulative prefix sum, and
read requested integer radii at index `2R`.  Equations, component positions,
regional energies, transport/source definitions, tolerances, and dynamics do
not change.  Require scalar FTD-0671 equivalence within `2e-12`, equality with
the FTD-0686 implementation within `2e-12`, and the existing 256-mask exact
certificate.
