# FTD-0999 — Preregistration: cumulative clock-growth resource certificate repair v2

**Identifier:** `FTD-0999`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY REPAIR]`  
**Parent:** `FTD-0998`, first execution `89/91`, Outcome D

## 1. Immutable parent record

- Parent protocol SHA-256:
  `6E0B28E7487B7E285EE05F7A16CDAC58984077D2964CC1042931996FFB884052`.
- Parent proof SHA-256:
  `E8257678700C732214D1A44E69FF5FCBEB31696BB86E6A2F5DB8F611534CD6F0`.
- All source hashes and all conservation, admission, cumulative,
  finite-resource, average-power, concurrency, overlap, causality, inverse,
  and interpretation gates passed.
- Two dependent source-census predicates failed because source normalization
  retains the C++ line-comment token between the frozen words
  `accounted` and `channels`.

The parent protocol and proof remain byte-preserved and execution-invalid.

## 2. Authorized in-memory repair

The wrapper may apply exactly one marker-only source substitution at both of
its two occurrences:

- replace `rest-offset-free accounted channels` with the exact normalized
  frozen-source fragment `rest-offset-free accounted // channels`.

No expression being tested, expected value, source hash, classifier,
physical claim, or scope statement may change. The repaired source exists
only in memory during wrapper execution.

## 3. Integrity gates

The wrapper must verify:

- both parent hashes and this repair-protocol hash before execution;
- the old fragment occurs exactly twice and the replacement is absent;
- exactly two marker occurrences are repaired;
- the repaired inherited certificate exits zero with `91/91` and Outcome B;
- the parent protocol and proof bytes remain unchanged; and
- the wrapper reports its own integrity count and fails closed.

## 4. Classifier

- **Outcome B:** every integrity gate passes and the inherited result remains
  the exact causal resource law with the phase-complete native reservoir open.
- **Outcome D:** any hash, occurrence, inherited check, byte-preservation, or
  scope gate fails.

No engine mutation, numerical search, parameter scan, fit, or formula
substitution is authorized.
