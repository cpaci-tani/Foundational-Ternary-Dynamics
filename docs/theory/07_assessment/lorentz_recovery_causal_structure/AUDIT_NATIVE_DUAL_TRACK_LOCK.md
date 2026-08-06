# FTD-0420 — Native-first dual-track production lock

**Date:** 2026-07-22  
**Status:** `[PRE-REGISTRATION — FROZEN CORE]` + `[THEOREM — cryptographic source contract]`  
**Verdict:** `SIX-MILESTONE-LOCK-INSTALLED; PRODUCTION-TICK-FROZEN; TARGET-FITTING-PROHIBITED`

FTD-0420 reserves `FTD-0420` through `FTD-0425` for the native charge, native
pole, native marginal-flow, auxiliary pole/counterterm, and unitarity gates.
The controlling protocol is
`PREREG_NATIVE_FIRST_DUAL_TRACK_RECOVERY.md`.

The machine-readable lock hashes the production tick, phase rules, toggle
catalog, constants, and native continuity interface. Blocks delimited by
`FTD-HISTORY-BEGIN/END` are excluded because they are the newly admitted
read-only observer. Any other source change breaks the verifier and requires a
new preregistration rather than silently changing the tested theory.

The lock also independently evaluates the exact native charge transition
matrix over the rational numbers. It performs no physical-constant search and
contains no empirical target.

**Artifacts**

- `scripts/proofs/native_dual_track_lock.json`
- `scripts/proofs/proof_native_dual_track_lock.py`
- `engine/include/ftd/eft/history_event_journal.h`

The program is 70% native and 30% auxiliary. Exact Lorentz invariance is not a
gate; control of every dimension-four preferred-frame coefficient is.
