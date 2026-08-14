# FTD-0896 — Bloch-quasimomentum lift source-marker repair v3

**Identifier:** `FTD-0896`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0895`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0895 repair execution reported `80/81`. C16, C34, C35,
C37, and C39 were repaired exactly as registered. C73 remained false because
the normalized Doxygen line in the frozen header is

```text
observer-only research * instrumentation
```

The asterisk is the retained leading Doxygen marker from the wrapped second
line. FTD-0895 incorrectly attributed the mismatch to a Unicode hyphen and
searched the wrong representation. The independent interaction-scale
conjunct passed.

This is a one-marker certificate repair. No source, equation, theorem
statement, outcome, or scope ceiling may change.

## 2. Frozen parent hashes

| artifact | SHA256 |
|---|---|
| `PREREG_BLOCH_QUASIMOMENTUM_LIFT_CERTIFICATE_REPAIR_v2.md` | `79D31FA87C3F9DC5F59C09C57748B94B149336E325B3DC47019C20729EED5E88` |
| `scripts/proofs/proof_bloch_quasimomentum_lift_local_momentum_map_trilemma_v2.py` | `C4FCFD2BABF29FA09811BA68D5B9B96D6AA1B4B5CAF6E87C94A0CC512832E4FD` |

The inherited FTD-0894 protocol and certificate hashes remain frozen inside
the parent wrapper. Any mismatch invalidates the repair.

## 3. Exactly permitted in-memory substitution

The v3 wrapper must find exactly one occurrence of the FTD-0895 R4 replacement
phrase

```text
observer‑only research instrumentation
```

and replace it with the actual normalized frozen-source phrase

```text
observer-only research * instrumentation
```

No other text may change. The wrapper must verify the old anchor occurs once,
the new anchor is absent initially, and the substitution is unique before
executing the repaired FTD-0895 wrapper in memory.

## 4. Inherited gates and outcome

All 81 FTD-0894 checks and the three successful FTD-0895 representation
normalizations are inherited. The only expected effect is that C73 recognizes
the frozen Doxygen line; all other gates must replay identically.

## 5. Scope firewall

```text
REPAIR_SCOPE=C73_DOXYGEN_LINE_MARKER_NORMALIZATION_ONLY
FTD0894_PROTOCOL_UNCHANGED=TRUE
FTD0894_CERTIFICATE_UNCHANGED=TRUE
FTD0895_REPAIRS_R1_R2_R3_UNCHANGED=TRUE
QUASIMOMENTUM_ADDITION=EXACT_MODULO_RECIPROCAL_LATTICE
GLOBAL_CONTINUOUS_HOMOMORPHIC_T3_TO_R3_SECTION=IMPOSSIBLE
FINITE_RANGE_GLOBAL_UNWRAPPED_GENERATOR=IMPOSSIBLE
WINDING_HISTORY_TYPE=OPEN_CANDIDATE_NOT_SELECTED
LOCAL_STRESS_ROUTE=NOT_RULED_OUT
PHYSICAL_MOMENTUM_SCALE=OPEN
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA256 of this repair protocol and its wrapper must be recorded in
the preregistration manifest before first execution.
