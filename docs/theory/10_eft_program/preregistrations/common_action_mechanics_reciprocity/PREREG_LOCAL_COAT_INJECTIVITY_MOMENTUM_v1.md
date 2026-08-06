# PRE-REGISTRATION — Local-coat injectivity and momentum v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0465`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0464`  
**Engine artifact:** `engine/tests/campaign_local_coat_injectivity_momentum.cpp`

**Locked campaign SHA-256:**
`F6CDCDEC4BE9B7F7D09246B512F8736D1C032D844C5047D4FD3F3BC98F34F326`

## 1. Questions

Is FTD-0464's finite partial-history translation injective on the engine's
single `J/W` field, and does the resulting field momentum change supply the
equal-and-opposite recoil required by the particle update?

## 2. Exact kernel test

For each fixed radius `R=1,2,3`, use the exact FTD-0464 map: remove the field
inside the source-centered cube and add it one face step forward while the
outside field remains in place. On every forward boundary site and each of the
six `J/W` vector components, place `+1` on the selected inner-face site and
`-1` on its unselected forward neighbor. These witnesses have disjoint
site/component pivots and are linearly independent. Require their mapped image
to vanish to `1e-12`.

The preregistered nullity lower bounds are

`6(2R+1)^2 = 54, 150, 294`

for `R=1,2,3`. As a control, global periodic translation followed by its exact
inverse must recover a deterministic full-field fixture to `1e-12`.

## 3. Momentum test

Reproduce all 42 FTD-0464 `R=1` event times with initial dressing independently
off and on. Every event must remain kinematically valid and close total event
energy to `1e-12`. Measure

`Delta P_field = P_field(after)-P_field(before)`

using the registered central lattice field-momentum observer and compare it to
the particle update's required field recoil

`P_particle(before)-P_particle(after)`.

Record pass count, RMS/minimum/maximum momentum residual, and worst energy
residual for both arms. No compensating impulse is added.

## 4. Locked classification

- `LOCAL_TRANSLATION_NONINJECTIVE_MOMENTUM_CLOSES`: all exact kernel and global
  controls pass, and all 84 events close momentum to `1e-12`;
- `LOCAL_TRANSLATION_NONINJECTIVE_MOMENTUM_MISMATCH`: the kernel/global controls
  pass, but at least one event fails momentum closure;
- `PROTOCOL_INVALID`: a witness, global inverse, attempt count, kinematic, or
  event-energy gate fails.

## 5. Consequence

Noninjectivity means the local additive translation cannot be reversed from
the post-event engine field because moved and unmoved boundary values are
merged. Momentum mismatch means it also cannot serve as the complete physical
exchange. Either defect blocks sequential promotion of the FTD-0464 map. A
successor must be an injective local map and close both energy and momentum;
stored observer provenance does not count as engine state.

## 6. Execution record

All `54/150/294` registered kernel witnesses mapped exactly to zero, while the
global permutation reversed exactly. All 84 particle updates remained
kinematically valid and closed energy, but momentum closed in `0/84`. The
momentum-residual RMS was `0.00203404` with dressing off and `0.00225716` with
dressing on. Locked verdict:

`LOCAL_TRANSLATION_NONINJECTIVE_MOMENTUM_MISMATCH`.
