# FTD-0884 — Finite port rail and positive source-battery certificate repair v2

**Identifier:** `FTD-0884`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0883`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0883 execution reported `54/56`. All five source hashes,
the protocol hash, every finite-bank algebra/reversal gate, every positive-
battery energy/inverse gate, and every scope gate except C3 passed. C3 searched
the literal contiguous substring

```text
explicit one-vector-per-port cyclic representation
```

but the frozen protocol wraps that phrase across a Markdown newline as
`explicit one-vector-per-port\ncyclic representation`. C56 then failed only
because it correctly depends on C3.

This is a verifier representation defect. It changes no source, equation,
`L=4` probe, capacity, exact arithmetic, battery reserve, outcome, terminal
marker, or scope ceiling.

## 2. Frozen parent hashes

| Artifact | SHA-256 |
|---|---|
| `PREREG_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_BOUNDARY_v1.md` | `0B6ACD3C1E41B4D1EE60CCA9A5E04E91E84FC96F06A3725B1F41DDDFD79E8C0B` |
| `scripts/proofs/proof_finite_port_rail_positive_source_battery_boundary.py` | `9596738C5FA23964CDEE234BD73E1A48B658516D931B5E92CC085118D90DD02B` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Sole permitted substitution

The wrapper must find exactly one occurrence of

```python
    "explicit one-vector-per-port cyclic representation" in protocol_text,
```

and replace it in memory with

```python
    "explicit one-vector-per-port cyclic representation"
    in " ".join(protocol_text.split()),
```

No other source text may change. The wrapper must verify that the old anchor
occurs exactly once, that the replacement occurs exactly once afterward, and
that both parent hashes match section 2 before executing the repaired in-memory
certificate.

## 4. Inherited gates and outcome

All 56 FTD-0883 gates, their order, exact arithmetic, capacity, reserve,
terminal markers, and outcome rule are inherited unchanged. The only expected
mechanical effect is that C3 recognizes the already frozen line-wrapped class
statement and C56 passes if C1--C55 all pass.

## 5. Scope firewall

```text
REPAIR_SCOPE=C3_WHITESPACE_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
EQUATIONS_PROBES_CAPACITY_RESERVE_UNCHANGED=TRUE
EXACT_REAL_MEMORY_NO_GO=NOT_CLAIMED
BATTERY_LAW_STATUS=IMPOSED_REFERENCE
CANONICAL_HAMILTONIAN_RESERVOIR=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA-256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.

