# Audit — FTD-0732 captured-state perturbation survival v1

**Status:** `[AUDIT PASS — UNRESOLVED VERDICT AND SUBORDINATE ROBUSTNESS CERTIFIED]`  
**Date:** 2026-07-29

## Findings

1. The protocol hash remains `1A93899A…0903`; the 5% probe amplitude and
   verdict map were not changed after output.
2. All six inward-compression variants fail the registered initial
   negative-energy condition. Their pair energies are
   `+0.0025364--+0.0040180`.
3. Those variants pass Gauss, momentum-preservation, graph, and causal gates;
   the failure is specifically energy-sector inadmissibility.
4. The locked verdict is therefore
   `CAPTURE_PERTURBATION_TRANSACTION_UNRESOLVED`. It cannot be changed to the
   boundary verdict, which was reserved for valid arms that fail later.
5. All 78 admissible arms execute, remain continuously graph-inside and
   negative through their 256-step continuation, and invert state-only.
6. All 12 unperturbed centers survive through parent tick 384.
7. The hostile `radial_impulse_plus` and `dynamic_field_minus` selectors pass
   in all 18 held-out `L=65` confirmations with zero volume or polarity class
   mismatch.
8. The admissible sub-cross is measured robustness, not a passed aggregate
   campaign, an open neighborhood, or asymptotic stability.
9. The compression boundary belongs to the selected compact potential; it is
   not a physical particle radius derived from the five postulates.

## Correct statement

FTD-0732 is unresolved because its fixed 5% inward-position probe starts
outside the captured negative-energy sector. Conditional on initial
admissibility, every one of the 78 tested center, outward-position,
radial/transverse-momentum, and dynamic-field perturbations survives exactly
through parent tick 384, including the registered held-out volume controls.

## Next admissible inference

The next protocol may derive and use the fixed action's negative-energy radial
interval. It may not choose a new percentage merely because 5% failed.

## Verification

- runner `4D706C2A…5A4E`;
- JSON `508EAB61…2B09`;
- CSV `15926F9E…E2AD`;
- certificate `5AED00E0…04F8`, `1469/1469 PASS`;
- production tick, defaults, toggles, and scenarios unchanged.

