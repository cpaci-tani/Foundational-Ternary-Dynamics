# FTD-0726 — Covariant lower-energy formation v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0726`  
**Date:** 2026-07-29  
**Parents:** `FTD-0724`, `FTD-0725`  
**Scope:** fresh full qualification of the lower-energy basin under the
FTD-0725 tighter numerical realization of the unchanged common action; no
production state, default, toggle, scenario, interaction, field normalization,
physical initial state, acceptance tolerance, or ontology promotion.

## 1. Question locked before validation

Does the complete FTD-0724 formation matrix retain its lower-energy negative
pair basin when every history is solved at the FTD-0725 covariance-qualified
root tolerance, including both polarity orders, state-only reverse replay,
exact energy exchange, and the unchanged detached-field classifier?

## 2. Parent lock

- connected-action header SHA-256:
  `DAC2DC83A7366EB5856B008613079E2FB8100A05D4C38ACC23B8C145DD03D65E`;
- connected-action source SHA-256:
  `0B64BB431DCA847AE03321BF983D1023AD40CDE33D5B49DED0E2A14B6664337C`;
- FTD-0724 unresolved runner SHA-256:
  `98C3D1B572B695B901C11555765CD4B5BC33F7FFCAEB5C6F638DB96113801208`;
- FTD-0725 conditioning JSON SHA-256:
  `829A76E2187F389318D71C8D3035957FD29E106D61D1DFBF0220006463E9E89E`;
- FTD-0725 verdict:
  `COVARIANCE_DEFECT_NUMERICAL_CONDITIONING_CONFIRMED`;
- qualified numerical realization: solve tolerance `2e-12`, at most 96
  iterations.

FTD-0724 is not reused as a positive result. This is a fresh run of record.

## 3. Frozen campaign

Use the unchanged FTD-0724 physical matrix:

- `L=33`, `dt=1/4`, 48 forward and 48 state-only reverse steps;
- unbound separation `1.30` and momenta
  `{0.0060,0.0075,0.0085,0.0095,0.0120}`;
- bound-control separation `1.00`, momentum `0.015`;
- all 13 unoriented Moore rays;
- both polarity orders;
- origin and translated `(4,-3,2)` copies;
- minimum-energy periodic longitudinal face field, `B=0`, initialization CG
  tolerance `1e-13`, at most 4096 iterations;
- canonical interaction normalization, exact quadratic-coat current, matched
  face/edge update, compact-well depth `0.01`, squared cutoff `1.5`;
- common-action gate `1e-10`, inverse gate `1e-8`, scalar-history covariance
  gate `1e-9`, recoil gate `1e-9`, pair/field balance gate `1e-8`.

Total: 260 unbound plus 52 bound-control complete histories.

## 4. Locked physical expectations

FTD-0725 preserved the following raw classes at both solver conditions. The
fresh full replay must reproduce them before any basin claim advances:

- `p=0.0060`, `0.0075`, `0.0085`, and `0.0095`: negative sector in 52/52 arms
  per momentum;
- `p=0.0120`: positive escape in 52/52 arms;
- bound controls: retained in 52/52 arms.

These are replication targets, not proof of stable formation.

## 5. Unchanged classifiers

`negative_sector` requires graph membership and pair internal energy below
`-1e-6` for the final eight ticks. `captured` additionally requires an outside
positive start, graph entry without later exit, pair/field balance `<=1e-8`,
dynamic-field norm `>1e-8`, magnetic energy `>1e-10`, and dynamic-field median
doubled radius at least four.

Report graph transitions, active ticks, final pair energy, field export,
dynamic norm, magnetic energy, and morphology by momentum. Negative-sector and
capture fractions must be nonincreasing with momentum.

## 6. Locked verdict map

- All algebra, inverse, covariance, recoil, and controls pass; the locked
  `208/260` negative-sector pattern reproduces; all 208 negative arms also pass
  the detached-field classifier:
  `COVARIANT_LOWER_ENERGY_CAPTURE_CONSTRUCTIVE`.
- All gates and controls pass; the locked negative-sector pattern reproduces;
  at least one but not all negative arms passes the detached-field classifier:
  `COVARIANT_CAPTURE_MORPHOLOGY_DIRECTIONAL`.
- All gates and controls pass; the locked negative-sector pattern reproduces;
  zero negative arms passes the detached-field classifier:
  `COVARIANT_ENERGETIC_TRAPPING_WITHOUT_DETACHED_FIELD`.
- All gates and controls pass; some negative sector exists monotonically but
  the locked sign pattern does not reproduce:
  `COVARIANT_TRAPPING_BOUNDARY_SHIFTED`.
- Fractions are nonmonotone: `NONMONOTONE_COVARIANT_FORMATION_RESPONSE`.
- No arm reaches the negative sector:
  `NO_COVARIANT_LOWER_ENERGY_BASIN_OBSERVED`.
- A bound control fails: `DERIVED_PAIR_BOUND_STATE_UNSTABLE_TIGHT_ROOT`.
- Any root, current, Gauss, common-action, energy, inverse, covariance, or
  recoil gate fails: `TIGHT_ROOT_FORMATION_TRANSACTION_UNRESOLVED`.

Even the strongest verdict is a selected finite-volume classical formation
witness. Persistence, perturbative stability, a physical particle pole,
quantum binding, and production adoption remain separate gates.
