# FTD-0727 — Bound-dressing persistence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0727`  
**Date:** 2026-07-29  
**Parent:** `FTD-0726`  
**Scope:** extend the qualified FTD-0726 energetic-trapping histories to 96
ticks and distinguish persistent localized dressing from delayed escape; no
production state, default, toggle, scenario, interaction coefficient, field
normalization, physical initial state, or ontology promotion.

## 1. Question locked before validation

Does the complete constituent-plus-field state that is energetically trapped
at tick 48 remain graph-connected, negative, reciprocal, invertible, and
localized through tick 96, or does the near field return energy and release
the pair?

## 2. Parent lock

- connected-action header SHA-256:
  `DAC2DC83A7366EB5856B008613079E2FB8100A05D4C38ACC23B8C145DD03D65E`;
- connected-action source SHA-256:
  `0B64BB431DCA847AE03321BF983D1023AD40CDE33D5B49DED0E2A14B6664337C`;
- FTD-0726 preregistration SHA-256:
  `8C484A05DC94F4099687757660F6D0873E614A7D55FAE40637539BECEFF4A335`;
- FTD-0726 result JSON SHA-256:
  `FE73C4FBCBB3D1FB796D0BB2A758FF8EC3A867915A1711E91713C7FC407D697D`;
- FTD-0726 verdict:
  `COVARIANT_ENERGETIC_TRAPPING_WITHOUT_DETACHED_FIELD`;
- numerical realization: solve tolerance `2e-12`, at most 96 iterations.

The selected action, compact well, current, field update, and normalization
remain frozen.

## 3. Frozen campaign

- `L=33`, periodic field, `dt=1/4`, 96 forward and 96 state-only reverse
  steps;
- unbound separation `1.30` at:
  - deep trapped parent momentum `p=0.0060`;
  - shallow trapped parent momentum `p=0.0095`;
  - escaping parent control `p=0.0120`;
- bound-control separation `1.00`, momentum `0.015`;
- all 13 unoriented Moore rays, both polarity orders, and origin/translated
  `(4,-3,2)` copies;
- minimum-energy periodic longitudinal face dress, `B=0`, initialization CG
  tolerance `1e-13`, at most 4096 iterations;
- canonical interaction normalization, exact quadratic-coat current, matched
  face/edge update, compact-well depth `0.01`, squared cutoff `1.5`;
- common-action gate `1e-10`, inverse gate `1e-8`, scalar-history covariance
  gate `1e-9`, recoil gate `1e-9`, pair/field balance gate `1e-8`.

Total: 156 unbound plus 52 bound-control complete histories.

## 4. Locked persistence classifiers

For each history record graph membership, pair internal energy, and field
energy at every tick. Record the dynamic-field norm, magnetic energy, and
doubled median radius relative to the instantaneous minimum-energy static
dress at ticks 48 and 96.

`tail_persistent` requires graph membership and pair internal energy below
`-1e-6` at every tick from 49 through 96, with no graph exit after entry.

`localized_dressing_96` additionally requires at tick 96:

- dynamic-field norm `>1e-8`;
- magnetic energy `>1e-10`;
- doubled dynamic-field median radius `<=4`.

This radius condition classifies localization only. It does not make
streamlines literal strands or establish that all binding energy is carried
inside that radius.

The `p=0.0120` escape control must remain outside the graph with positive pair
energy at every tick from 49 through 96. The pre-bound control must satisfy
`tail_persistent` in all arms.

## 5. Finite-volume limitation locked before output

This is not called a pre-wrap or infinite-volume test. The exact local stencil
can expand support by one lattice site per tick, and the longitudinal initial
dress is already periodic. Therefore a 96-tick `L=33` history may contain
finite-volume recurrence even when the physical group-speed distance
`C_SPEED*dt*96` is below `L/2`.

A positive result licenses a separate volume-stability campaign. It cannot by
itself establish asymptotic stability.

## 6. Locked verdict map

- Every algebra, inverse, covariance, recoil, and control gate passes; all 104
  parent trapped arms are `tail_persistent` and
  `localized_dressing_96`:
  `FINITE_VOLUME_BOUND_DRESSING_PERSISTS_TO_96_TICKS`.
- All 104 parent trapped arms persist, but at least one fails the locked
  localization condition:
  `FINITE_VOLUME_TRAPPING_PERSISTS_WITH_EXTENDED_FIELD`.
- Some but not all parent trapped arms persist:
  `DIRECTIONAL_OR_POLARITY_PERSISTENCE_SPLIT`.
- No parent trapped arm persists, or every parent family exits:
  `TRANSIENT_TRAPPING_REEMITS_BY_96_TICKS`.
- The escaping parent control re-enters or fails its positive outside tail:
  `FINITE_VOLUME_OR_DYNAMICAL_RECURRENCE_CONTAMINATES_ESCAPE_CONTROL`.
- A pre-bound control fails:
  `DERIVED_PAIR_BOUND_CONTROL_UNSTABLE_BY_96_TICKS`.
- Any root, current, Gauss, common-action, energy, inverse, covariance, or
  recoil gate fails:
  `BOUND_DRESSING_PERSISTENCE_TRANSACTION_UNRESOLVED`.

Even the strongest verdict is a selected finite-volume classical persistence
witness. Volume stability, perturbative stability, a physical particle pole,
quantum binding, mass, spin, statistics, and production adoption remain
separate gates.

