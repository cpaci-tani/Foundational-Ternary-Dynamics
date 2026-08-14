# PRE-REGISTRATION — Phase-referenced action export rail certificate repair v2

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0862`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; 36/36 PASS — OUTCOME B]`  
**Parent:** `FTD-0861`  
**Parent protocol:**
[`PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md`](PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md)

## 1. Invalid parent execution

The source-locked parent returned `35/36 FAIL`. All seven source hashes and
every exact mathematical gate passed. Only C35 failed because its verifier
looked for the absent prose marker

```text
does not actuate the relative pair
```

The frozen FTD-0860 source states the intended production boundary as

```text
its common-field trigger cannot determine the relative port
```

This is a verifier/source-vocabulary mismatch. It changes no equation,
threshold, source hash, physical claim, or expected Outcome B.

## 2. Frozen repair

Execute the frozen parent verifier in memory after exactly one replacement:

```text
and "does not actuate the relative pair" in transducer_boundary
```

becomes

```text
and "its common-field trigger cannot determine the relative port" in transducer_boundary
```

The wrapper must verify the frozen parent script SHA256 before applying the
replacement and must require exactly one matching fragment. The parent file
must remain unchanged.

## 3. Frozen hashes and command

- Parent verifier:
  `scripts/proofs/proof_phase_referenced_action_export_rail.py`
- Parent verifier SHA256:
  `098FA1885B72D60DD0B8DAE547CEAD73B96A8977D92EB11DD896EC4311840F09`
- Repair wrapper:
  `scripts/proofs/proof_phase_referenced_action_export_rail_v2.py`
- Repair wrapper SHA256:
  `DC38CF600E1A2500DF53E7A9090C79239E04595065C94794B07A766486D3D4C6`
- Required command:
  `python scripts/proofs/proof_phase_referenced_action_export_rail_v2.py`
- Required denominator: exactly `36/36`.

## 4. Frozen outcomes

- **Outcome B — repaired exact selected reference rail:** all `36/36` parent
  gates pass after the sole registered vocabulary repair.
- **Outcome C — invalid:** parent hash mismatch, repair multiplicity other than
  one, or any check failure.

No Outcome A production promotion is available through a verifier-only repair.

## 5. Scope ceiling

The repair cannot derive or promote any physical result. The parent scope
ceiling remains unchanged: no persistent vacuum carrier, phase-calendar
origin, `G*` cadence, Born rule, production C18 integration, Hilbert recovery,
Bell mechanism, thermodynamic arrow, biological identification,
CM/substrate gearbox, Lorentz hiding, or completeness follows.

## 6. Execution record

- Pre-run repair protocol SHA256:
  `6DF12ECB3299614D568B8DA26B165209E1C9F2DF27EF8707AF3849D44AE49CE0`.
- Execution: `36/36 PASS`.
- Frozen outcome: **Outcome B — repaired exact selected reference rail**.
- Result:
  [`THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md).
