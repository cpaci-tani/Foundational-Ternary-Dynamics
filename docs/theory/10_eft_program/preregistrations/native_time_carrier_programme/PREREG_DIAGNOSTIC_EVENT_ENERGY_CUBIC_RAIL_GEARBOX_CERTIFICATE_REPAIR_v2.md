# FTD-0855 — Diagnostic event energy/cubic rail certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 32/32]`  
**Date:** 2026-08-10  
**Parent:** FTD-0854 invalid certificate  
**Scope:** verifier-only repair of C13, C25, and C26  
**Production impact:** none

## 1. Frozen defect

FTD-0854 returned `29/32`. C13 sliced `phase_write.cpp` from the evaporation
heading through end-of-file, so it included a later maintenance transformation
outside the evaporation event. C25 and C26 declared the event label merely
nonzero even though the protocol fixed `s in {-1,+1}`; SymPy therefore did not
replace `s^2` by one. Every source hash and every other gate passed. No frozen
physical or algebraic identity failed.

## 2. Frozen parent inputs

| Input | SHA256 |
|---|---|
| post-run FTD-0854 protocol | `5397975BC0A6FE0312088B2E741D2F972894AE14EB4711E1BF465488541A6F2A` |
| invalid FTD-0854 script | `60A0EE0CA003737ADD2B57FEDC82A6118B7543FABF6EA007A9D01BB7ECD280A8` |

All seven FTD-0854 source hashes, equations, 32 check labels/order, outcome
definitions, and expected Outcome B are inherited unchanged.

## 3. Only permitted repairs

The wrapper must fail closed unless both parent hashes match. It may make only
these in-memory substitutions:

1. C13's inspected source slice starts immediately after the unique
   evaporation-event counter increment and ends at the following history-
   journal block. This contains the record/label assignment and excludes later
   functions.
2. C25 and C26 simplify their exact energy expressions after applying the
   already registered ternary-event identity `s^2=1`.

No source, equation, coefficient, type, energy definition, tolerance, check
count, label, outcome, or interpretation may change. The invalid parent remains
unchanged on disk.

## 4. Locked implementation

```text
scripts/proofs/proof_diagnostic_event_energy_cubic_rail_gearbox_v2.py
```

Frozen wrapper SHA256:

```text
8953357829B0814BE60D6855FF2DE9A167256E757C2431C2BA997FCB9E26C647
```

Run exactly:

```text
python scripts/proofs/proof_diagnostic_event_energy_cubic_rail_gearbox_v2.py
```

## 5. Outcomes

- `32/32`: register FTD-0855 as the repaired scoped theorem;
- any failure: repair invalid, book no theorem, and preserve both attempts.

The expected result remains FTD-0854 Outcome B: the adopted diagnostic supplies
a positive local event energy and the cubic radial mode is exactly the causal
odd history rail, while production dual-energy accounting, a reserved directed
rail, the reciprocal barrier, and full-state lift remain open.

## 6. Recorded outcome

The first locked repair execution returned `32/32 PASS`. Only the registered
C13 source-slice boundary and C25/C26 application of `s^2=1` changed in memory.
Outcome B is booked in
[`THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md).
