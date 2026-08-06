# FTD-0693 — Causal excitation separation with qualified local roots v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production impact:** none

## Question

Can the previously resource-invalid `L=113` causal-separation campaign finish
under the FTD-0692 local residual evaluator while preserving every physical
equation, accepted complete state, observer, and discriminator?

## Frozen design

Inherit FTD-0691 exactly:

- `L=113`, 96 forward ticks and 96 from-state reverse ticks;
- source radius 8 and registered radii `8,16,24,32,40,48`;
- the same control and signed internal-mode excitations;
- spatial observation every four ticks, with exact tickwise global source
  exchange accumulated inside each block;
- late window ticks 80 through 96;
- three concurrent control/sign paths and the established caches; and
- all FTD-0684/0688 physical and algebraic classification gates.

The only licensed engineering change is
`use_local_residual_evaluation=true`. Every accepted root must materialize one
complete candidate, pass the existing common-action gates, and have local-to-
materialized residual difference at most `1e-14`. No tolerance, volume,
momentum, mode, observer, interaction coefficient, or verdict label changes.

## Verdict logic

- A complete exact run receives the existing morphology plus late-time class.
- Failure of a physical/algebraic gate is a valid negative result at the
  corresponding established scope.
- Failure to complete or emit complete records is execution-invalid and makes
  no matter-dynamics claim.

This campaign can distinguish localized dressing, ordered outward transport,
and distributed mixed field behavior before first periodic contact. It cannot
by itself identify a photon, particle pole, literal flux strand, or Lorentz
recovery.

