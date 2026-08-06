# FTD-0730 — Persistence/re-entry volume discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0730`  
**Date:** 2026-07-29  
**Parents:** `FTD-0728`, `FTD-0729`  
**Scope:** distinguish local persistence/recapture from periodic-volume
dependence using the unchanged selected action; no coefficient, state type,
classifier, production default, toggle, scenario, or ontology change.

## 1. Questions locked before validation

1. Do the `p=0.0060/0.0095` negative cores and their extended-field
   morphology persist from `L=33` to `L=65`?
2. Does the third `p=0.0120` graph transition occur at the same time on both
   volumes, move with volume, or disappear on `L=65`?

## 2. Parent lock

- FTD-0728 physical-class JSON SHA-256:
  `3E9723FE36E23D07E23685BDEF20C0F07A491ED0C525897D774963C71A080F7D`;
- FTD-0729 protocol SHA-256:
  `96751A97197E6F52625FFECD53CF7B66752960290968530196E9A8F9A52AD384`;
- FTD-0729 selected root realization: tolerance `2e-14`, at most 384
  iterations;
- FTD-0729 targeted scalar/complete maxima:
  `6.2501e-12` / `2.9612e-12`.

## 3. Frozen matrix

Use volumes `L={33,65}`, periodic fields, `dt=1/4`, and 96 forward plus 96
state-only reverse steps.

At each volume run:

- `p=0.0120`, separation `1.30`: all 13 unoriented Moore rays and both
  polarity orders (`26` histories);
- `p={0.0060,0.0095}`, separation `1.30`: representative face `0_0_1`, edge
  `0_1_-1`, and body-diagonal `1_1_1` rays, both polarities (`12` histories);
- pre-bound separation `1.00`, `p=0.015`: the same three representative rays
  and both polarities (`6` histories).

Total: 44 histories per volume, 88 complete histories.

Retain the exact FTD-0728 action, initial minimum-energy periodic dress,
current, face/edge update, normalization, well depth `0.01`, squared cutoff
`1.5`, root tolerance `2e-14`, common-action gate `1e-10`, recoil gate `1e-9`,
inverse gate `1e-8`, and pair-plus-field balance gate `1e-8`.

## 4. Locked observables

For each arm record:

- every graph-transition tick (first three explicitly);
- tail persistence for ticks 49--96;
- final-eight-tick energy/sign class;
- pair-to-field energy export and balance;
- dynamic-field norm, magnetic energy, and doubled median radius at ticks 48
  and 96 relative to the instantaneous static dress;
- state-only inverse recovery and rowwise residual maxima.

Match volumes by family, momentum, direction, and polarity. For `p=0.0120`,
record the absolute third-transition-tick difference where both volumes
re-enter. For persistent parents, record the tick-96 radius difference.

## 5. Locked controls

- `L=33` must reproduce:
  - one graph transition and tail persistence for all 12 lower-energy
    representative arms;
  - radius three at tick 48 and radius five or six at tick 96;
  - three graph transitions in all 26 `p=0.0120` arms;
  - all six pre-bound controls persistent.
- all 88 histories must pass action, energy, recoil, and inverse gates;
- all six `L=65` pre-bound controls must persist.

## 6. Locked verdict map

- Any execution/algebra/inverse/control or `L=33` reproduction gate fails:
  `VOLUME_DISCRIMINATOR_UNRESOLVED`.
- Any `L=65` lower-energy representative loses tail persistence:
  `LOWER_ENERGY_PERSISTENCE_VOLUME_SENSITIVE`.
- Lower-energy cores persist but any matched tick-96 field radius differs by
  more than one doubled-radius unit:
  `PERSISTENT_CORE_FIELD_MORPHOLOGY_VOLUME_SENSITIVE`.
- All `L=65` `p=0.0120` arms lack a third transition through tick 96:
  `P012_REENTRY_FINITE_VOLUME_RECURRENCE`.
- All `L=65` arms re-enter and every matched third-transition tick differs by
  at most two ticks:
  `P012_REENTRY_LOCAL_DYNAMICS_VOLUME_STABLE`.
- All `L=65` arms re-enter, but at least one third-transition time differs by
  more than two ticks:
  `P012_REENTRY_VOLUME_DEPENDENT_TIMING`.
- Only some `L=65` arms re-enter:
  `P012_REENTRY_DIRECTIONAL_VOLUME_SPLIT`.

A volume-stable re-entry is still a selected finite-volume classical result,
not proof of asymptotic capture. Absence at `L=65` identifies volume
sensitivity but does not by itself determine the infinite-volume limit.

