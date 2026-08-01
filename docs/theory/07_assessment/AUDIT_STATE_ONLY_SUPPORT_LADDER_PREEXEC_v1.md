# FTD-0754C — State-only support-ladder pre-execution audit v1

**Status:** `[PRE-EXECUTION AUDIT — EXISTING DISCOVERY CORPUS ONLY]`  
**Date:** 2026-07-30  
**Protocol:**
`docs/theory/10_eft_program/preregistrations/PREREG_STATE_ONLY_SUPPORT_LADDER_v1.md`

## Frozen scope

The executable may replay only the already-seen FTD-0753/0754 face, edge, and
body histories and observe support half-widths `{4,6,8}` at the eight existing
observer ticks. It may not create a new perturbation, volume, direction,
duration, field phase, or negative-control history. Therefore the run is an
analytic/discovery addendum and cannot count toward FTD-0755 validation.

## Frozen identities and gates

The protocol hash is
`F1E8A18631D923040607128D34CCC6C2FF17D6D9D0BA594CBF57C7A9157BD48A`.
It freezes primitive energy reconstruction, nested-projection orthogonality,
the Pythagorean decrement, monotonic bound energy, exact old scalar replay,
and `1e-12` relative gates before output inspection.

Frozen source identities are:

- interface:
  `F180DAE14DF62244E9F091F68670EA1EEA192881D87BAE86D43BE633C09CC696`;
- implementation:
  `10BF768DC480C5A0699A18B097E44AC685A27D13BF2C90C95758EC1FF3D3FB2F`;
- locked runner:
  `9DE1A9B26C7033B31F215179766BBE2913198CFA780A9FF86BDCA780EBA20A3C`;
- WSL2 executable:
  `3088EFAC09FB70121F7B0B04E968AB02A53AE2ED67C082BF720EFB623DD7D4E2`.

The native state-only observer and covariance CTests pass 2/2 after adding the
ladder algebra. The ladder records are observer-only; no result feeds the
field pipeline, nonlinear root, current deposition, or accepted state.

## Artifact-absence check

Immediately before registered replay,
`engine/results/ftd_0754_support_ladder/` did not exist. No result value was
available when the radii, identities, tolerances, hashes, or consequence map
were frozen.

Production defaults, established CUDA, scenarios, and ontology are unchanged.
