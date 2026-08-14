# FTD-0940 — Phase-gated neutral-C4 Hodge-chord certificate repair v2

**Identifier:** `FTD-0940`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0939, execution-invalid before any mathematical gate

## 1. Parent record

| parent | protocol SHA-256 | certificate SHA-256 | result |
|---|---|---|---|
| FTD-0939 | `53C09B0F862B8C6DBE9B8E92CCDFCF6A0C2AB0671A5C7D4DCD8780397A15BDF3` | `F5266CC6219A7C1D81729A977FD9727045A7E4610E1364D2629D1FB6CC89463C` | abort before gate reporting; the input guard treated every zero component as outside the Moore-step alphabet and therefore rejected all registered planar edge directions |

The frozen live directions are planar Moore edges such as `(-1,1,0)`. The
parent intended to reject components outside `{-1,0,+1}` but encoded

```python
if any(abs(value) != 1 for value in displacement):
```

which rejects the mandatory zero component. No source, equation, theorem,
outcome, or physical discriminator was executed.

## 2. Sole permitted repair

The v2 wrapper may replace exactly the parent guard

```python
if any(abs(value) != 1 for value in displacement):
```

with

```python
if any(value not in (-1, 0, 1) for value in displacement):
```

The wrapper must:

1. verify the frozen parent certificate hash;
2. verify the old guard occurs exactly once;
3. verify the repaired guard is absent before substitution;
4. apply the substitution once in memory;
5. compile and execute the repaired parent in memory; and
6. require the inherited certificate to exit zero.

It may not modify the parent file, protocol, source hashes, direction set,
current construction, equations, thresholds, outcomes, scope ceilings, or
production firewall.

## 3. Inherited outcome and firewall

All FTD-0939 gates and Outcome A/B/C definitions are inherited.

```text
PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=MOORE_COMPONENT_ALPHABET_GUARD
MATHEMATICS_SOURCES_OUTCOMES=UNCHANGED
TARGET_DIRECTION_OR_WAKE_READ=FALSE
PRODUCTION_INTEGRATION=FORBIDDEN
```

The exact SHA-256 of this repair protocol and wrapper must be entered in the
preregistration manifest before any theorem is booked.
