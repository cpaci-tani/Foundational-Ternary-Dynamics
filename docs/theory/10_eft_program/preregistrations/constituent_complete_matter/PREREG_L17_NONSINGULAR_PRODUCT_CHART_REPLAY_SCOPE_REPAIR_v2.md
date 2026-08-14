# FTD-0834 — FTD-0832 replay scope repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID]`  
**Date:** 2026-08-10  
**Scope:** verifier-launch repair only; no producer rerun and no physics-gate change  
**Production impact:** none

## 1. Disclosed state

The FTD-0832 producer result is known. Its original replay stopped at the
undefined module-scope name `scalar_close`. FTD-0833 injected that helper and
then stopped at the second undefined module-scope name `required`. Neither
attempt produced an artifact verdict or check count.

Inspection of the complete v5-only block shows these are its only two
references to helpers scoped locally inside `main`. This protocol does not
change that block or inspect any result number to choose a gate.

## 2. Locked repair

Leave the FTD-0832 producer, corpus, verifier, original wrapper, and every
threshold unchanged. A new shim injects:

1. the verifier's exact `scalar_close` formula; and
2. the successful-value semantics of its `required` helper: fetch the named
   CSV field, reject empty or nonfinite payloads, and return its float value.

The shim may not catch or suppress exceptions. It then executes the unchanged
hash-checking verifier under the unchanged v5 environment flag.

Locked hashes:

- v2 repair shim SHA-256:
  `4BE5B51357525E46F821978DB82F595503848FF7D31709D39F41494F2C4CC3CF`;
- unchanged verifier SHA-256:
  `10070C6BE8ACE7E2A4A19158932DCC1EC1DA48A8773B9382A063D9989767AA1D`;
- unchanged original v5 wrapper SHA-256:
  `61C66059B7247A3A64EDD9A15E4F04581942C3E57D972EBC6F9F25D129D4CE6E`.

## 3. Execution and outcomes

Copy the hash-matching shim into the source-pinned FTD-0832 worktree and run
exactly once:

```text
python scripts/proofs/proof_l17_complete_tangent_nonsingular_product_chart_v5_replay_repair_v2.py
```

- `REPLAY_SCOPE_REPAIR_VALID_AND_FTD0832_CERTIFIED`: the unchanged verifier
  reaches its terminal report and all checks pass.
- `REPLAY_SCOPE_REPAIR_OR_FTD0832_CERTIFICATE_INVALID`: the shim hash differs,
  any exception remains, or any unchanged verifier check fails.

Only the first outcome permits booking the producer verdict. It licenses no
new chart, tolerance, tangent, localization, recurrence, or clock claim.

## 4. Recorded outcome

The shim and unchanged verifier hashes matched. The replay reached its
terminal report:

```text
FTD-0832 independent tangent certificate: 94/95 checks PASS
execution_valid=true
solve_resolved=false
eligible_candidate_count=1
qualified_candidate_count=0
verdict=L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED
FAIL candidate metric rows replay
```

The sole mismatch is the primary-to-sign `sign_angle`: producer
`7.300048299977713e-08`, replay `0.0`, locked scalar cross-check tolerance
`2e-8`. Outcome `REPLAY_SCOPE_REPAIR_OR_FTD0832_CERTIFICATE_INVALID`
therefore applies. No FTD-0832 physics verdict is certified.
