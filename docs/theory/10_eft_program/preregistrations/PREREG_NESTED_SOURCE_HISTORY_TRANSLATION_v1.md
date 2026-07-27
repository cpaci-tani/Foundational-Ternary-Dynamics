# PRE-REGISTRATION — Nested source-history translation v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0464`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0462`, `FTD-0463`  
**Engine artifact:** `engine/tests/campaign_nested_source_history_translation.cpp`

**Locked campaign SHA-256:**
`AFF552AECFEE2CF1CECC561F943752F546090A61C26C377C4121CC68FA529330`

## 1. Question

Can a fixed local portion of the polarity-generated `J/W` history move one
face step with the manifested polarity while preserving event locality and
making all 42 registered particle updates kinematically admissible? Is the
selected one-time longitudinal dressing necessary for that result?

## 2. Frozen protocol

Use the exact `L=33`, 48-tick, speed `0.15`, packet amplitude `0.02`, and
one-time dressing work `1e-4` of FTD-0459 through FTD-0463. Evolve the packet,
dressing, and stationary-polarity source history separately. At every one of
the 42 inherited attempt ticks, evaluate eight arms:

- initial dressing `off` and `on`;
- translated source-history radii `R=1,2,3,global`.

For finite `R`, the selected history is the fixed periodic Chebyshev cube
centered on the source site. Remove its snapshot `J/W` values from their old
sites and add them one face step in `+x`; the event support is the union of the
old and translated cubes. Fields outside that union remain exactly unchanged.
For `global`, translate every source-history site by one periodic face step.
Move the ternary manifestation from source to target in every arm. No field
amplitude, phase, speed, radius, tolerance, or event time is fitted or scanned.

## 3. Exact energy identity

For each event define the isolated source-history observer change
`Delta H_self`, external/source cross-energy change `Delta X`, external
endpoint work `W_ext`, and complete event change `Delta H_event`. Require

`Delta H_event = Delta H_self + Delta X - W_ext`

to `1e-12` at every attempt. Require zero squared event-difference norm outside
the registered support to `1e-12`. The `global+dressing-on` arm must reproduce
FTD-0462's required-work RMS `0.00023156579861414742` to `1e-12` and remain
kinematically admissible at `42/42` attempts.

## 4. Recorded estimators

For every arm record attempt count, kinematically valid count, required-work
RMS and maximum absolute value, isolated-self-change RMS, cross-energy-change
RMS, mean/minimum/maximum selected source-history `J/W` norm fraction, worst
identity residual, and worst outside-support fraction.

## 5. Locked classification

- `LOCAL_TRANSLATION_SUFFICIENT_DRESSING_INDEPENDENT`: at least one of
  `R=1,2,3` passes `42/42` both with dressing off and on;
- `LOCAL_TRANSLATION_SUFFICIENT_DRESSING_DEPENDENT`: a local radius passes
  `42/42` with dressing on but none passes with dressing off;
- `LOCAL_TRANSLATION_SUFFICIENT_DRESSING_OBSTRUCTS_REGISTERED_ARM`: a local
  radius passes `42/42` with dressing off but none passes with dressing on;
- `GLOBAL_TRANSLATION_ONLY`: no local arm passes `42/42`, while at least one
  global arm does;
- `NO_FULL_RECOVERY`: no arm passes `42/42`;
- `PROTOCOL_INVALID`: any execution, identity, locality, or parent-reproduction
  gate fails.

## 6. Interpretation boundary

A successful local arm proves only that a selected finite translation of the
existing history is sufficient for registered kinematics. It does not prove
that the history is a native bound mode, that production selects the event, or
that the field autonomously follows a polarity. Failure of local arms means
the frozen history cannot yet be partitioned into a causal particle dressing
by these fixed geometric supports. Dressing-on/off dependence diagnoses the
selected initial condition; it does not derive a physical dressing.

## 7. Execution record

All eight arms completed 42 attempts, closed the exact identity below
`3.80e-18`, and changed no field norm outside their registered support. Every
arm was kinematically admissible at `42/42`. The `R=1` arms remained fully
admissible with dressing both off and on, while moving a mean `41.10%` of the
source-history `J/W` norm. The global dressing-on RMS reproduced FTD-0462 to
`2.98e-19`. Locked verdict:

`LOCAL_TRANSLATION_SUFFICIENT_DRESSING_INDEPENDENT`.
