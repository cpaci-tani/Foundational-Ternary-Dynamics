# Pre-registration — Local work-port/factor certificate repair v3

**Identifier:** `FTD-0982`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Expected classifier:** inherited FTD-0981 `Outcome B`

## 1. Immutable provenance

- FTD-0981 protocol SHA-256:
  `7CF3DC6239200CF1B773ADEC0633F0B30CD5735C7FF8BDA1360F730888C5EDE3`.
- FTD-0981 proof SHA-256:
  `BDD16E3D4AB8BF0E0D4C72E5520638AB712D64E113725145B27F919B620F0C69`.
- First execution: `76/79`; one source-marker spelling and two missing
  inequality substitutions failed.
- v2 repair protocol SHA-256:
  `4FD4AAE506BF96B890C020FEB3E798F12558AC271A8EEACE5D26722FDA8BCD9E`.
- v2 wrapper SHA-256:
  `39F0287B56EB4FC62BF04CEB0A40FFFCD8B3B06455229068321812C7CA984B09`.
- v2 execution: `78/79`; both inequality repairs passed, but the source marker
  still failed because the theorem wraps `A local` and `quarter-turn` across
  a Markdown newline.

All substantive exact gates passed in both executions. Parent and v2 files
remain byte frozen.

## 2. Authorized repairs against the original parent

Exactly three in-memory substitutions are authorized.

1. Replace the raw source-text predicate

   ```python
   "local quarter-turn is still a legitimate symplectic event" in trilemma_text
   ```

   with the whitespace-normalized predicate

   ```python
   "local quarter-turn is still a legitimate symplectic event" in " ".join(trilemma_text.split())
   ```

   This accepts the frozen `A local\nquarter-turn` line without changing its
   required substantive phrase.

2. Add the exact lower-branch witness `k=kappa**2/2` to the
   `positive_q_defect` limit, exactly as locked in v2.

3. Add the exact upper-branch witness `k=2*kappa**2` to the
   `positive_p_defect` limit, exactly as locked in v2.

No other source substitution, assertion change, gate waiver, expected-value
change, production mutation, numerical search, or scope promotion is allowed.

## 3. Integrity requirements

The wrapper must verify:

- the original parent protocol and proof hashes;
- both earlier repair hashes;
- each old anchor occurs exactly once in the original parent;
- each repaired anchor occurs exactly once in memory;
- the original parent and both v2 files remain byte frozen; and
- all `79/79` inherited gates and the inherited Outcome B execute.

## 4. Classifier

- `79/79`, inherited Outcome B, and all integrity gates pass: verifier repair
  succeeds;
- otherwise: Outcome D.
