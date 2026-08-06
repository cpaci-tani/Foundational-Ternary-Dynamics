# FTD-0729 — Late-reentry covariance convergence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0729`  
**Date:** 2026-07-29  
**Parent:** `FTD-0728`  
**Scope:** targeted numerical convergence of the unique recorded worst
FTD-0728 history; no action, physical state, classifier, field normalization,
production default, toggle, scenario, or ontology change.

## 1. Question locked before validation

Does the late translated-separation defect in the `p=0.0120`, direction
`0_1_-1` re-entry history continue to shrink under one additional decade of
exact-root tolerance, including the complete electric, magnetic, and matter
state rather than scalar observables alone?

## 2. Parent lock

- FTD-0728 protocol SHA-256:
  `F2C1D17AE3DF79557E784D25C38904241719EB020E5818565FC37BBC4DA76412`;
- FTD-0728 runner SHA-256:
  `F2294329F3DF1F45C8F5F8104A88E55384C76510FAE64D128825F294BAE1ABB5`;
- FTD-0728 JSON SHA-256:
  `3E9723FE36E23D07E23685BDEF20C0F07A491ED0C525897D774963C71A080F7D`;
- parent tight scalar maximum:
  `5.6798055148021831e-10`;
- locked worst coordinates: unbound, `p=0.0120`, direction `0_1_-1`,
  plus-minus, shifted, tick 92, separation.

## 3. Frozen targeted matrix

- `L=33`, periodic field, `dt=1/4`, 96 forward steps;
- separation `1.30`, momentum `p=0.0120`, direction `(0,1,-1)`;
- both plus-minus and minus-plus polarity orders;
- origin and translated `(4,-3,2)` copies advanced in paired lockstep;
- unchanged initial dress, compact-pair action, current, field update,
  normalization, common-action gate, and recoil gate;
- root conditions:
  - `parent`: tolerance `2e-12`, at most 96 iterations;
  - `tight`: tolerance `2e-13`, at most 192 iterations;
  - `ultra`: tolerance `2e-14`, at most 384 iterations.

At every tick record separation, pair-energy, and field-energy differences,
plus translated maximum-norm electric, magnetic, matter, and complete-state
differences. Record graph-transition counts and final energy class for all
four histories in each condition.

## 4. Locked gates

1. The `tight` plus-minus scalar maximum must reproduce the FTD-0728 value
   within `1e-12` absolute.
2. All roots, common-action identities, recoil checks, and paired graph classes
   must agree within each condition.
3. `ultra` scalar and complete-state maxima must remain below `1e-9`.
4. Conditioning is confirmed only if both `ultra/tight` scalar and complete
   ratios are `<=0.2`.
5. A numerical plateau is recorded if both ultra maxima remain below `1e-9`
   but either ratio exceeds `0.2` without increasing above its tight value.

## 5. Locked verdict map

- Reproduction and all gates pass; both ratios `<=0.2`:
  `LATE_REENTRY_ROOT_CONDITIONING_CONFIRMED`.
- Reproduction and class gates pass; ultra remains below `1e-9`, does not
  increase, but at least one ratio exceeds `0.2`:
  `LATE_REENTRY_COVARIANCE_PLATEAU_BELOW_GATE`.
- Ultra increases or exceeds `1e-9`:
  `LATE_REENTRY_COVARIANCE_DEFECT_PERSISTS`.
- A graph/final-sign class changes with root tolerance:
  `LATE_REENTRY_CLASS_SOLVER_SENSITIVE`.
- Reproduction, root, common-action, or recoil gate fails:
  `LATE_REENTRY_CONVERGENCE_DIAGNOSTIC_UNRESOLVED`.

This diagnostic can close numerical conditioning only for the recorded worst
history. It cannot promote FTD-0727, establish full-matrix fivefold
convergence, or answer the finite-volume recurrence question.

