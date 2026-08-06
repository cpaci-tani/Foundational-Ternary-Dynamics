# FTD-0767 — Dynamic-response and spatial-clearing audit v1

**Status:** `[CERTIFIED DERIVED OBSERVER ALGEBRA + POST-HOC NUMERICAL FACT]`

**Date:** 2026-07-31

**Engine mutation:** none

## Verdict

FTD-0766 descriptively observed a local velocity-aligned residual-field
deformation, but it did not test a spatially detached wake.  Its 16 valid
moving arms displaced the relational core by only `0.190822--0.875652` site.
The selected bound observer has half-width four and its near window has radius
eight, so disjoint initial/final windows require travel greater than eight and
sixteen sites respectively.

The maximum registered travel reaches only `10.95%` and `5.47%` of those two
clearing distances.  The trailing/leading statistic was measured entirely
inside overlapping initial and final neighborhoods.

## Observer correction

After swapping the rest arm's trailing and leading halves in the negative
velocity chart, the signed dynamic response is

```text
delta W_pair
 = [(w_+ - w_0) + (w_- + w_0)]/2
 = (w_+ + w_-)/2.
```

The preparation anisotropy therefore cancels from this directed numerator.
The locked artifact contains a positive local dynamic response in every
evaluable pair.  The response decreases with boost at ages 64 and 128, so it
does not have the registered accumulation ordering expected of a trail.

This correction does not rehabilitate the invalid FTD-0766 execution.  It
changes the descriptive interpretation from “possible wake” to “local
velocity-aligned deformation; spatial wake not tested.”

## Energy caveat

For residual fields `R_q=R_0+delta R_q`,

```text
U(R_q)-U(R_0)
 = <R_0,delta R_q>_H + (1/2)||delta R_q||_H^2.
```

Regional energy subtraction includes an interference term and is not the
nonnegative energy of the difference field.  A successor must record both
terms, actual boundary flux, and matter work before calling any remaining
signal deposited energy.

## Certificate

```text
python scripts/proofs/proof_dynamic_response_spatial_clearing.py
FTD-0767 dynamic-response/clearing certificate: 113/113 checks
minimum_travel=0.19082199688148194
maximum_travel=0.87565230209105493
support_clearing_fraction=0.10945653776138187
near_clearing_fraction=0.054728268880690933
local_velocity_aligned_deformation=OBSERVED_DESCRIPTIVELY
spatially_detached_wake=NOT_TESTED
```

The certificate hash-locks the FTD-0766 artifact, confirms the frozen support
and horizon parameters from source, reconstructs all valid-arm travel,
performs the signed rest cancellation, and reproduces the local asymmetry.
It performs no numerical search and no new engine evolution.

## Correct matter statement

The current mobile object is a manifested relational kernel moving through a
mostly unentrained environmental field.  A rigid co-moving dressing is closed
negative at the registered scope.  A flame-like recruit/shed process remains
open because the kernel has not yet cleared its own selected observer support.
