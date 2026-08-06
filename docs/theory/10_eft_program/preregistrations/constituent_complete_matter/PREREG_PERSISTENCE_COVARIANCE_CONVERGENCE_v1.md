# FTD-0728 — Persistence covariance convergence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0728`  
**Date:** 2026-07-29  
**Parent:** `FTD-0727`  
**Scope:** fresh full tighter-root replay of the unresolved 96-tick
persistence matrix; no action, physical initial state, field normalization,
classifier, production state, default, toggle, scenario, or ontology change.

## 1. Question locked before validation

Does a ten-times tighter termination of the same exact common-action residual
reduce the FTD-0727 scalar-history covariance miss below the locked gate while
preserving its persistence, field-extension, escape-control re-entry, and
bound-control classes?

## 2. Parent lock

- connected-action header SHA-256:
  `DAC2DC83A7366EB5856B008613079E2FB8100A05D4C38ACC23B8C145DD03D65E`;
- connected-action source SHA-256:
  `0B64BB431DCA847AE03321BF983D1023AD40CDE33D5B49DED0E2A14B6664337C`;
- FTD-0727 protocol SHA-256:
  `49941B346EFF02394C381E6661D47E1A519FD333A087A88A25825D817457312F`;
- FTD-0727 runner SHA-256:
  `5640D53F956D8E3B9610A9B2269F16A4DAAFF79CDE436252734BA5EE3F085B68`;
- FTD-0727 result JSON SHA-256:
  `52C9537C4E40AD33683070081EA1D4160BFF3C1101AF9237FC36B4EBA95E3F95`;
- parent scalar spread:
  `1.1065308669344631e-9`;
- parent verdict:
  `BOUND_DRESSING_PERSISTENCE_TRANSACTION_UNRESOLVED`.

## 3. Frozen physical matrix

Repeat FTD-0727 exactly:

- `L=33`, periodic field, `dt=1/4`, 96 forward and 96 state-only reverse
  steps;
- unbound separation `1.30`, momenta `{0.0060,0.0095,0.0120}`;
- bound-control separation `1.00`, momentum `0.015`;
- all 13 unoriented Moore rays, both polarity orders, origin and translated
  `(4,-3,2)` copies;
- the same minimum-energy initial dress, current, field update, compact well,
  normalization, and all FTD-0727 tail/morphology classifiers;
- common-action gate `1e-10`, inverse gate `1e-8`, scalar-history covariance
  gate `1e-9`, recoil gate `1e-9`, pair/field balance gate `1e-8`.

Change only nonlinear root termination from `2e-12` to `2e-13` and the maximum
iteration budget from 96 to 192.

Total: the same 208 complete histories.

## 4. Locked convergence and class requirements

Conditioning is confirmed only if:

1. scalar-history spread is `<=1e-9` and at most `0.2` times the parent spread;
2. every rowwise action, energy, recoil, inverse, and bound-control gate passes;
3. all 104 `p=0.0060/0.0095` arms remain tail-persistent;
4. all 104 parent trapped arms retain the extended-field class
   (`localized_dressing_96=0`);
5. all 52 `p=0.0120` arms retain three graph transitions and fail the clean
   escape control, with 12/52 negative at the final eight ticks;
6. all 52 pre-bound controls remain persistent and localized.

Record the family, momentum, direction, polarity, translation, tick, and
scalar component of the maximum tight-root covariance difference.

## 5. Locked verdict map

- All convergence and class requirements pass:
  `FINITE_VOLUME_PERSISTENCE_WITH_EXTENDED_FIELD_AND_RECURRENCE_QUALIFIED`.
- Covariance passes `1e-9` but does not improve fivefold:
  `PERSISTENCE_COVARIANCE_PASSES_WITH_INCOMPLETE_CONVERGENCE`.
- Covariance remains above `1e-9` or fails to improve:
  `PERSISTENCE_COVARIANCE_DEFECT_PERSISTS`.
- Any locked persistence, extension, recurrence, final-sign, or bound-control
  class changes:
  `PERSISTENCE_CLASS_SOLVER_SENSITIVE`.
- Any root, current, Gauss, common-action, energy, inverse, or recoil gate
  fails:
  `PERSISTENCE_TIGHT_ROOT_TRANSACTION_UNRESOLVED`.

Even the strongest verdict qualifies only the selected finite-volume
classical histories. It does not retroactively promote FTD-0727 and does not
establish infinite-volume stability, radiation, a particle pole, mass, spin,
statistics, or production dynamics.

