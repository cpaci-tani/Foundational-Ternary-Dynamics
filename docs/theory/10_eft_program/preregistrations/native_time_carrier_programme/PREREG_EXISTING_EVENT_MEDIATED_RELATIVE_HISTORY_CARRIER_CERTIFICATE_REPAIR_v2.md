# FTD-0945 — Existing event-mediated relative-history carrier certificate repair v2

**Identifier:** `FTD-0945`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0944, first immutable execution `136/137`, Outcome D from one
prose-firewall marker mismatch after every source and mathematical gate passed

## 1. Parent record

| parent | protocol SHA-256 | certificate SHA-256 | result |
|---|---|---|---|
| FTD-0944 | `9E2EF3C707A798AD73F7DF1280273F2924B9C7D3B337393000C6175E55811B1D` | `2B7E9AE5427B5EAA680E50433AB343EE4D3315C28081C7832364597FDFAA34B7` | `136/137`; every frozen source, transition-algebra, relative-zero, injectivity, composition, outcome, and other scope gate passed; only the literal prose marker `No new primitive storage type` was absent |

The locked parent protocol states:

```text
It would not force a new primitive storage type,
```

The parent certificate checked the semantically equivalent but textually
different capitalized fragment:

```python
"No new primitive storage type",
```

No mathematical, source, production, or outcome failure occurred.

## 2. Sole permitted repair

The v2 wrapper may replace exactly one parent source literal:

```python
"No new primitive storage type",
```

with the exact locked-protocol fragment:

```python
"not force a new primitive storage type",
```

The wrapper must:

1. verify the frozen parent protocol hash;
2. verify the frozen parent certificate hash;
3. verify the old literal occurs exactly once;
4. verify the new literal is absent from the parent certificate before repair;
5. apply exactly one in-memory substitution;
6. compile and execute the repaired parent in memory; and
7. require the inherited certificate to exit zero with all `137/137` gates.

It may not edit the parent protocol or certificate, change a source hash,
transition map, action class, matrix, invariant, witness, acceptance gate,
Outcome A/B/C/D definition, scope firewall, or production source.

## 3. Inherited outcome and firewall

All FTD-0944 gates and outcome definitions are inherited unchanged.

```text
PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=ONE_PROTOCOL_PROSE_MARKER
MATHEMATICS_SOURCES_OUTCOMES=UNCHANGED
TARGET_HISTORY_READ=FALSE
PRODUCTION_INTEGRATION=FORBIDDEN
```

The repair protocol and wrapper hashes must be recorded before booking the
combined FTD-0944/0945 theorem.
