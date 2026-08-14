# FTD-0833 — FTD-0832 replay NameError repair v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPLAY REPAIR INVALID]`  
**Date:** 2026-08-10  
**Scope:** verifier-launch repair only; no producer rerun and no physics-gate change  
**Production impact:** none

## 1. Disclosed observation

The FTD-0832 producer completed and printed
`L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED`. The first independent replay
then stopped before evaluating an artifact because the v5 module-scope
function `harmonic_detail_replay` called `scalar_close`, while the unchanged
verifier declared that helper only as a local function inside `main`.

The exception was:

```text
NameError: name 'scalar_close' is not defined
```

No replay verdict or check count was produced. The producer result is known,
so this is not a blind physics protocol. It is a disclosed fail-closed repair
of verifier name resolution.

## 2. Locked repair

Leave all FTD-0832 files and artifacts unchanged. A new launch shim injects
the verifier's already-declared comparison formula into the `runpy` globals:

```python
def scalar_close(lhs, rhs, tolerance=2e-12):
    return abs(lhs - rhs) <= tolerance * max(1.0, abs(rhs))
```

It then executes the unchanged
`proof_l17_complete_tangent_candidate.py` with the unchanged v5 environment
flag. The old verifier continues to check its own source and artifact hashes.
The shim may not catch, suppress, or translate any exception.

Locked hashes:

- repair shim SHA-256:
  `946875B310CF1FEA591DCB06E738D72AF642C6E6FED56837B8FB13A9D4507D40`;
- unchanged verifier SHA-256:
  `10070C6BE8ACE7E2A4A19158932DCC1EC1DA48A8773B9382A063D9989767AA1D`;
- unchanged original v5 wrapper SHA-256:
  `61C66059B7247A3A64EDD9A15E4F04581942C3E57D972EBC6F9F25D129D4CE6E`.

## 3. Execution

Run exactly once in the FTD-0832 clean source-pinned worktree:

```text
python scripts/proofs/proof_l17_complete_tangent_nonsingular_product_chart_v5_replay_repair.py
```

The shim copied into that worktree must match the locked hash above. The
producer is not rerun. No threshold, denominator, artifact, or source-closure
expectation may change.

## 4. Outcomes

- `REPLAY_REPAIR_VALID_AND_FTD0832_CERTIFIED`: the unchanged verifier reaches
  its terminal report and all checks pass.
- `REPLAY_REPAIR_OR_FTD0832_CERTIFICATE_INVALID`: the shim hash differs, an
  exception remains, or any unchanged verifier check fails.

Only the first outcome permits booking the FTD-0832 producer verdict. It does
not convert an unresolved/negative tangent result into a clock and does not
license a denominator or threshold repair.

## 5. Recorded outcome

The locked shim hash matched and the run resolved `scalar_close`, then stopped
in the same v5 block at a second module/local-scope defect:

```text
NameError: name 'required' is not defined
```

Therefore the outcome is
`REPLAY_REPAIR_OR_FTD0832_CERTIFICATE_INVALID`. No artifact verdict was
produced and FTD-0832 was not booked from this attempt.
