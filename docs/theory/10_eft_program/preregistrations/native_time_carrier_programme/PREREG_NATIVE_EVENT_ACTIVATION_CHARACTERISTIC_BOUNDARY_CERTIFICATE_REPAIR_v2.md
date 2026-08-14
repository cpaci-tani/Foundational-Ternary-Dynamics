# PRE-REGISTRATION — Native event activation/characteristic certificate repair v2

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0858`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 40/40]`  
**Parent:** `FTD-0857` execution-invalid verifier; no theorem  
**Inherited physics protocol:**
[`PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md`](PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md)  
**Parent pre-run protocol SHA-256:**
`6B354D41D8B2324A434758383D6C8B123D17CF813FAAD21CDA84C1A010DA08B1`  
**Frozen invalid verifier SHA-256:**
`6D7B2FC2B6BA432976D359A2C104EAB15FAB175BC5F721B7B6B84BA8D13D17A2`

## 1. Scope

This is a verifier-only repair. It inherits all seven source hashes, equations,
40 gates, outcomes, firewalls, and the expected Outcome B from FTD-0857. No
source, physical claim, tolerance, model parameter, event predicate,
characteristic definition, dispersion relation, or outcome condition may
change.

Exactly four transformations are permitted:

1. replace the nonexistent event-slice end marker
   `// ---- Sequential post-pass` with the existing function boundary
   `void phase_write_assign_pending_ids`;
2. in C32, compare the two time-reversal components after symbolic
   simplification rather than using Python tuple structural equality;
3. in C37, compare the trace-defect formula after symbolic simplification
   rather than requiring one SymPy factor ordering; and
4. in C40, remove the accidental literal `+` from the wrapped expected phrase
   `not thereby\n+derived physical law`.

The repair wrapper must hash-check the invalid parent verifier, require each
old fragment exactly once, apply each transformation in memory exactly once,
and execute the resulting certificate without editing the parent.

## 2. Acceptance

- **Repaired Outcome B:** all inherited 40 gates pass.
- **Still invalid:** wrapper/source mismatch, any repair outside the four
  registered transformations, or any inherited gate failure.

No production code may change in this run. No Born, Bell, `G*`, thermodynamic,
biological, or completeness promotion is permitted.

## 3. Recorded outcome

**Pre-run repair-protocol SHA-256:**
`A43367B3BF46918ED5DBDFDF988E53DE75274170E314B61328AB40EB9BFBE1F3`  
**Repair-wrapper SHA-256:**
`E2A6D22946E0E3BD9A5CE208EB7C440567AA72B97C28F507C099F06E93740204`

The wrapper hash-checked the invalid parent, applied exactly the four frozen
verifier repairs in memory, and passed all inherited gates `40/40`. The
registered verdict is repaired Outcome B:

```text
PRODUCTION_EVENT_ACCEPTANCE_IS_DETERMINISTIC_LOCAL_AND_TARGET_BLIND_GIVEN_FIXED_INPUTS
COMMON_FIELD_TRIGGERS_DO_NOT_DETERMINE_THE_RELATIVE_ON_SHELL_RECORD_PORT
RELATIVE_EDGE_PAIR_HAS_AN_EXACT_INCOMING_OUTGOING_ENERGY_CURRENT_CHART
FROZEN_C18_DISPERSION_IS_NOT_THE_EXACT_ONE_CELL_HISTORY_RAIL
SIGNAL_WORK_CLOSES_ZERO_WHILE_PHYSICAL_CONTROLLER_COST_REMAINS_OPEN
VERDICT=OUTCOME_B_NATIVE_TRIGGER_AND_CHART_PRODUCTION_PORT_INCOMPLETE
```

The theorem of record is
[`THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md).
