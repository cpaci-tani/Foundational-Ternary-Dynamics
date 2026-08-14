# FTD-0901 — common/relative connection gearbox source-marker repair v3

**Identifier:** `FTD-0901`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parents:** FTD-0899 and FTD-0900, both execution-invalid

## 1. Parent record

| parent | protocol SHA256 | certificate/wrapper SHA256 | result |
|---|---|---|---|
| FTD-0899 | `38B7B6C929CC10F3F296FBA56A36478790D5AD648F8F9D2603058EE58F245AA0` | `75426CCCE016C6471583BB65FD2D9C608D27AE871C44643F1A224F2C867176AB` | passed `C01--C41`, then malformed generated symbol range raised `ShapeError` |
| FTD-0900 | `8E179C04DA9DBED8A6974922268126B01E9A94E66A54A6D113227E22D8624D3B` | `7096BB1F01794B0A2812D61EE21F44378B8F4454312F00A8057BD4D9907B6EA9` | symbol separator repaired; inherited `82/87`; only C62--C64 and C66--C67 source-marker comparisons failed |

All mathematical gates `C14--C60`, all other source/scope anchors, and all
terminal firewalls `C70--C87` passed. The remaining failures are exact prose
representation mismatches:

- C62 and C63 compare phrases split by Markdown newlines;
- C64 omits the literal backticks around `` `i` ``;
- C66 removes spaces from the source but retains spaces in the needle instead
  of using the frozen underscore marker; and
- C67 compares a phrase split by a Markdown newline.

No theorem is booked from either invalid parent.

## 2. Sole permitted repairs

The v3 wrapper may apply exactly these six in-memory substitutions to the
frozen FTD-0899 certificate:

1. retain the FTD-0900 symbol separator repair
   `f"{prefix}0:3" -> f"{prefix}_0:3"`;
2. whitespace-normalize the C62 common-relative source before its unchanged
   phrase comparison;
3. whitespace-normalize the C63 odd-pointer source before its unchanged phrase
   comparison;
4. change the C64 needle from `i supplies orientation` to the literal frozen
   Markdown phrase `` `i` supplies orientation ``;
5. change the C66 needle to the frozen terminal marker
   `native_vector_common_action=open`; and
6. whitespace-normalize the C67 phase-boundary source before its unchanged
   phrase comparison.

The wrapper must verify exact old-form counts, apply each replacement once,
and compile the result in memory. It may not modify any source hash, symbol,
equation, assumption, algebraic comparison, inequality, threshold, physical
gate, outcome, or scope ceiling.

## 3. Inherited outcome and firewall

All 87 FTD-0899 gates and its Outcome A/B/C definitions are inherited.

```text
PARENT_PROTOCOLS_AND_CERTIFICATES=PRESERVED
REPAIR_COUNT=EXACTLY_SIX
REPAIR_SCOPE=ONE_SYMBOL_SEPARATOR_PLUS_FIVE_SOURCE_MARKER_NORMALIZATIONS
MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES=UNCHANGED
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
CONTINUOUS_NONZERO_CONNECTION_PRESERVES_CRITICAL_QUARTIC=FALSE_IN_REGISTERED_CLASS
PRODUCTION_INTEGRATION=FORBIDDEN
```

The exact SHA256 of this repair protocol and wrapper must be entered in the
preregistration manifest before first execution.
