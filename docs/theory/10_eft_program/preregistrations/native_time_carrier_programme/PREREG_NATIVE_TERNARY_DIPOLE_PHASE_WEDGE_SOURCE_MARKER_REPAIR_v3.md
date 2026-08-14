# FTD-0907 — native ternary-dipole/phase-wedge source-marker repair v3

**Identifier:** `FTD-0907`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0906, execution-invalid

## 1. Parent record

| run | protocol SHA256 | certificate/wrapper SHA256 | result |
|---|---|---|---|
| FTD-0905 | `6FC0C2BAB8A84378F3B88618BA41E16B4C328AFF497446A2A4542990AA20CA4E` | `FAA3CD3635C048AAD95E312AE59D6B725444C7C55571A0913A864F8AC8E038F0` | exact certificate `74/75`; C38 alone failed |
| FTD-0906 | `F3758EECECACFD92CB35DFD501868F0C72CE3AAA7ADB77AA8826029B2C1F1340` | `4608E92745BCB047AA18BBB8B5EE8DDB7C825E9D2B4DCD0A2148F7B0EBD53E8B` | repair integrity passed; inherited certificate remained `74/75` |

FTD-0906 normalized whitespace around a phrase the frozen FTD-0840 theorem
does not state verbatim. The theorem's actual exact orientation conclusion is

```text
\boxed{\chi_h<0}
```

followed by

```text
on every nonzero step.
```

The mathematical content is the same registered strict-orientation gate, but
the original certificate's prose marker added the words `discrete`, `has one`,
and `strict orientation`. FTD-0906 could not repair a marker whose target
phrase was not present.

No theorem is booked from FTD-0905 or FTD-0906.

## 2. Sole permitted repair

The v3 wrapper may make exactly one in-memory replacement in the frozen
FTD-0906 wrapper. Its proposed repaired parent expression

```python
"every nonzero discrete step has one strict orientation" in " ".join(source_text["pair_theorem"].split())
```

becomes

```python
"on every nonzero step" in " ".join(source_text["pair_theorem"].split())
```

The v3 wrapper must verify both frozen repair hashes, the old proposal occurs
exactly once, the corrected proposal is initially absent, and the compiled
FTD-0906 wrapper exits zero after exactly this replacement.

No source hash, equation, symbol, threshold, mathematical comparison,
transformation, outcome, or scope ceiling may change.

## 3. Inherited outcome and firewall

All 75 FTD-0905 gates and all FTD-0906 repair-integrity requirements are
inherited.

```text
PARENT_PROTOCOLS_CERTIFICATES_AND_WRAPPER=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=C38_ACTUAL_SOURCE_MARKER_ONLY
MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES=UNCHANGED
NATIVE_TERNARY_DIPOLE_AXIS=CONDITIONAL_EXACT
BILATERAL_PHASE_WEDGE_TIME_PARITY=UNCHANGED
CENTRAL_MEMORY_AND_CLOCK_SEPARATION=UNCHANGED
PRODUCTION_INTEGRATION=FORBIDDEN
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this repair protocol and wrapper must be entered in the
preregistration manifest before first execution.
