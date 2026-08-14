# Pre-registration — Global work-pair/local-batch concurrency certificate repair v3

**Identifier:** `FTD-0985`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent dispositions

The immutable FTD-0983 parent stopped on exactly two misspelled
`sympy.simpl` calls.  The immutable FTD-0984 v2 wrapper corrected only those
calls in memory.  Its inherited run then passed the complete certificate at
`62/62`, Outcome B, but the wrapper's integrity marker expected `59/59`.

The count error is exact: the static count omitted three additional executions
of the source-hash check inside its four-source loop.  The physical proof,
classifier, equations, hashes, and scope all passed unchanged.  Both earlier
artifacts remain preserved.

## 2. Frozen chain

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_GLOBAL_WORK_PAIR_VERSUS_LOCAL_BATCH_CONCURRENCY_v1.md` | `4D47C48793A591A54168B4A24EFFBB537EA8F11F6F226C0B52049A3E7CBD8C6C` |
| `proof_global_work_pair_local_batch_concurrency.py` | `E985B8EE6952AC494963F0B7DD1A4BD81FEBBD8D72881BCDA8E4C6D8DC733F0D` |
| `PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v2.md` | `4557C4DDAF9D9A987F84C3779A5659AEAE83A3960DC3119C2A46572FB116FD18` |
| `proof_global_work_pair_local_batch_concurrency_v2.py` | `16E950F3F92C588864E41981F3FA29B19D98FB220BD499394F476DC1344120B0` |

## 3. Sole permitted repair

The v3 wrapper may replace in memory exactly the one v2 integrity line

```text
integrity.append(("inherited 59/59 pass marker", "59/59 checks passed" in inherited))
```

with the otherwise identical `62/62` line.  It may make no other change.

## 4. Integrity and disposition

- all four frozen-chain hashes and this v3 protocol hash must match;
- the old full-line anchor must occur once and the new one must be absent;
- exactly one in-memory replacement is allowed;
- the inherited physical certificate must report `62/62`, Outcome B;
- the repaired v2 integrity layer must report `14/14`, Outcome B;
- every frozen artifact must retain its pre-run hash.

Any failure gives Outcome D.  Passing licenses only the aggregate-work versus
local-concurrency ownership theorem.  Native formation, production
integration, `G*`, Born/Bell, mass, Hilbert-space recovery, and framework
completeness remain outside scope.
