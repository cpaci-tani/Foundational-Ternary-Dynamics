# FTD-0906 — native ternary-dipole/phase-wedge certificate repair v2

**Identifier:** `FTD-0906`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0905, execution-invalid

## 1. Parent record

| parent | protocol SHA256 | certificate SHA256 | result |
|---|---|---|---|
| FTD-0905 | `6FC0C2BAB8A84378F3B88618BA41E16B4C328AFF497446A2A4542990AA20CA4E` | `FAA3CD3635C048AAD95E312AE59D6B725444C7C55571A0913A864F8AC8E038F0` | exact certificate `74/75`; C38 alone failed |

Every frozen source, native-type, dipole, covariance, phase-wedge,
time-parity, symmetric-square, central-memory, stability, clock-separation,
and terminal scope gate passed except C38.

C38 asked whether the frozen FTD-0840 theorem contains the phrase

```text
every nonzero discrete step has one strict orientation
```

The theorem contains those exact words across a Markdown line wrap. The
parent certificate searched the raw source for the single-line string, so it
returned false. Normalizing whitespace before the same literal source-marker
comparison changes no physics or mathematics.

No theorem is booked from FTD-0905.

## 2. Sole permitted repair

The v2 wrapper may make exactly one in-memory replacement in the frozen
FTD-0905 certificate:

```python
"every nonzero discrete step has one strict orientation" in source_text["pair_theorem"]
```

becomes

```python
"every nonzero discrete step has one strict orientation" in " ".join(source_text["pair_theorem"].split())
```

The wrapper must verify:

1. the repair protocol and parent certificate hashes;
2. the old form occurs exactly once;
3. the new form is absent before repair;
4. the repaired source contains the new form exactly once and no old form;
5. the compiled inherited certificate exits zero.

It may not change any frozen source, hash, symbol, equation, assumption,
threshold, transformation, comparison target, outcome, or scope ceiling.

## 3. Inherited outcome and firewall

All 75 FTD-0905 gates and its Outcome A/B/C definitions are inherited.

```text
PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=C38_WHITESPACE_NORMALIZATION_ONLY
MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES=UNCHANGED
NATIVE_TERNARY_DIPOLE_AXIS=CONDITIONAL_EXACT
BILATERAL_PHASE_WEDGE_TIME_PARITY=UNCHANGED
CENTRAL_MEMORY_AND_CLOCK_SEPARATION=UNCHANGED
PRODUCTION_INTEGRATION=FORBIDDEN
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this repair protocol and wrapper must be entered in the
preregistration manifest before first execution.
