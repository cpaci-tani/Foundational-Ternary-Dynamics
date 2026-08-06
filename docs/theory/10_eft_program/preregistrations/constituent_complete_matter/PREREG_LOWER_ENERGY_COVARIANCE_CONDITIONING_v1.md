# FTD-0725 — Lower-energy covariance conditioning v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE VALIDATION RUN]`  
**Identifier:** `FTD-0725`  
**Date:** 2026-07-29  
**Parent:** `FTD-0724`  
**Scope:** observer-only localization and numerical-conditioning test for the
failed FTD-0724 translation-covariance gate; no change to the mathematical
action, physical initial state, production state, default, toggle, scenario,
field normalization, interaction, or ontology.

## 1. Question locked before validation

Is the FTD-0724 scalar-history translation defect caused by finite nonlinear
root-solver accuracy, or does the unchanged long-interaction action select
different complete histories for lattice-translated initial states?

This protocol diagnoses the failed gate. It cannot retroactively promote the
FTD-0724 raw `208/260` negative-sector pattern.

## 2. Parent lock

- connected-action header SHA-256:
  `DAC2DC83A7366EB5856B008613079E2FB8100A05D4C38ACC23B8C145DD03D65E`;
- connected-action source SHA-256:
  `0B64BB431DCA847AE03321BF983D1023AD40CDE33D5B49DED0E2A14B6664337C`;
- FTD-0724 runner SHA-256:
  `98C3D1B572B695B901C11555765CD4B5BC33F7FFCAEB5C6F638DB96113801208`;
- FTD-0724 JSON SHA-256:
  `068AAFAE5029C3993BF0D7ECA3A9F681A3830B0A6B9E1191011EE6C0B53CD91F`;
- FTD-0724 CSV SHA-256:
  `3D8BFFC1E03559CA7B14E9FA79B1EB8542B117484D0AD6C7E9BA4AC4ACB06D20`;
- FTD-0724 failed scalar-history gate:
  `1.0680766715509549e-8 > 1e-9`.

## 3. Frozen physical campaign

Retain the FTD-0724 physical problem exactly:

- `L=33`, `dt=1/4`, 48 forward steps;
- unbound separation `1.30` and momenta
  `{0.0060,0.0075,0.0085,0.0095,0.0120}`;
- bound-control separation `1.00`, momentum `0.015`;
- all 13 unoriented Moore rays;
- `plus_minus` polarity order at origin and translated `(4,-3,2)` copies;
- minimum-energy periodic longitudinal face field, `B=0`, initialization CG
  tolerance `1e-13`, at most 4096 iterations;
- canonical interaction normalization, exact quadratic-coat currents,
  matched face/edge update, selected compact well depth `0.01`, squared cutoff
  `1.5`, common gate `1e-10`, recoil gate `1e-9`.

FTD-0724 emitted polarity-mirror scalar records identically, so this diagnostic
removes the duplicate conjugate order and isolates translation. There are 65
unbound and 13 bound origin/translated pairs per solver condition: 156 forward
histories per condition, 312 total. Reverse replay is not repeated because
FTD-0724 passed all 312 locked reverse gates; the target here is forward
translation covariance.

## 4. Frozen numerical conditions

Run the identical exact residual with separate caches for each arm:

| label | solve tolerance | maximum iterations |
|---|---:|---:|
| `baseline` | `2e-11` | 48 |
| `tight` | `2e-12` | 96 |

The tighter condition changes only termination accuracy and iteration budget.
It does not change an equation, residual, action coefficient, seed, physical
state, timestep, or acceptance gate.

## 5. Per-tick localization

At ticks `0..48`, translate the origin complete state by `(4,-3,2)` and compare
it directly with the shifted state. Record:

- separation, pair-internal-energy, and field-energy differences;
- translated electric-face, magnetic-edge, constituent position/momentum, and
  complete-state maximum differences;
- maximum root/common-action residual for the preceding step;
- both arms' graph membership and pair-energy sign.

Report the maximum and its family, momentum, direction, tick, and component for
each numerical condition. Also report negative-sector and bound-control counts
using the unchanged final-eight-tick energy/graph classifier.

## 6. Locked gates and verdicts

The baseline run must reproduce the FTD-0724 maximum scalar spread within
`1e-12` absolute. Both conditions must execute all 156 histories, pass every
rowwise common-action and recoil gate, and select identical energy-sign/graph
classifications between translated pairs.

- Baseline reproduces; tight scalar and complete-state defects are each
  `<=1e-9`; each is at most one fifth its baseline value; raw sign classes are
  unchanged (`104/130` unbound arms negative, 26/26 bound controls retained):
  `COVARIANCE_DEFECT_NUMERICAL_CONDITIONING_CONFIRMED`.
- Baseline reproduces and tight defects improve by at least fivefold, but one
  remains above `1e-9`:
  `COVARIANCE_CONVERGENCE_INCOMPLETE`.
- Baseline reproduces but either defect improves by less than fivefold, a
  translated pair changes graph/energy-sign class, or tight complete-state
  covariance remains `>1e-8`:
  `LONG_INTERACTION_COVARIANCE_DEFECT_PERSISTS`.
- Baseline does not reproduce, any history/root/common-action/recoil gate
  fails, or the two numerical conditions do not cover the frozen matrix:
  `CONDITIONING_DIAGNOSTIC_UNRESOLVED`.

A numerical-conditioning verdict licenses a separately preregistered full
formation/stability rerun at tighter tolerance. It does not validate FTD-0724,
does not qualify detached-field capture, and does not establish matter.
