# Pre-registration — Global work-pair/local-batch concurrency certificate repair v2

**Identifier:** `FTD-0984`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The immutable FTD-0983 parent execution passed every source-lock,
aggregate-pair, shared-phase degeneracy, and local-product symplectic gate
reached before Python stopped at the first local-energy check.  The sole
execution defect is that two verifier expressions call the nonexistent
`sympy.simpl` attribute instead of `sympy.simplify`.  This is a verifier typo;
it changes no equation, source, physical classifier, or scope.

The parent protocol and proof remain unchanged.

## 2. Frozen parents

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_GLOBAL_WORK_PAIR_VERSUS_LOCAL_BATCH_CONCURRENCY_v1.md` | `4D47C48793A591A54168B4A24EFFBB537EA8F11F6F226C0B52049A3E7CBD8C6C` |
| `proof_global_work_pair_local_batch_concurrency.py` | `E985B8EE6952AC494963F0B7DD1A4BD81FEBBD8D72881BCDA8E4C6D8DC733F0D` |

All four theoretical source hashes inherited by FTD-0983 remain frozen.

## 3. Sole permitted repair

The wrapper may replace, in memory, exactly the two occurrences of

```text
sp.simpl(
```

with

```text
sp.simplify(
```

The repaired calls occur only in the two `G4 local energy` residual checks.
No source file may be modified by the wrapper.  No other substitution is
permitted.

## 4. Repair integrity gates

- both parent hashes and this repair-protocol hash must match;
- the old token must occur exactly twice; each of the two repaired full-line
  anchors must be absent in the immutable parent before repair;
- exactly two in-memory substitutions must occur;
- the repaired inherited run must pass `59/59` and retain Outcome B;
- the parent protocol and parent proof hashes must be unchanged afterward.

Any failed integrity gate gives Outcome D.  Repaired Outcome B licenses only
the FTD-0983 aggregate-versus-local ownership theorem.  It does not license a
native work-port formation mechanism, production integration, `G*`, Born,
Bell, mass, Hilbert-space recovery, or framework completeness.
