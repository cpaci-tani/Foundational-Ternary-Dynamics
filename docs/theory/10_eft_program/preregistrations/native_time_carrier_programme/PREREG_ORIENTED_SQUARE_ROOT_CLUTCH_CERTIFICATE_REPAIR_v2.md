# Pre-registration — Oriented square-root clutch certificate repair v2

**Identifier:** `FTD-0980`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Expected classifier:** inherited FTD-0979 `Outcome B`

## 1. Immutable parent record

- FTD-0979 protocol SHA-256:
  `5747E0991BD6984B86B8A9522AD3F9B2927E8AADEDEF0D50C2C826DF7EA185C4`.
- FTD-0979 proof SHA-256:
  `814B2B6760E29129BA6616AE1BC6CC047D6DCFD20BCFDCDA8BCC054D9A3D2C92`.
- First execution: 44 displayed substantive passes, then Python
  `AttributeError: 'tuple' object has no attribute 'free_symbols'` in the G6
  mass-independence reporter. No displayed gate failed and no final classifier
  was reached.

The parent files remain byte frozen.

## 2. Authorized repair

Exactly one in-memory source substitution is authorized. Replace

```python
mu2 not in (sp.expand(k_laurent).coeff(z, 1), sp.expand(k_laurent * z).coeff(z, 0)).free_symbols
```

with the logically identical elementwise predicate

```python
mu2 not in sp.expand(k_laurent).coeff(z, 1).free_symbols
and mu2 not in sp.expand(k_laurent * z).coeff(z, 0).free_symbols
```

The old expression attempted to read `.free_symbols` from the containing
Python tuple rather than from its two SymPy elements. The new expression
changes no algebra, expected value, source hash, classifier, or physical gate.

## 3. Forbidden changes

No other source substitution, assertion change, gate waiver, expected-value
change, production mutation, numerical search, or scope promotion is allowed.
The wrapper must verify that the old anchor occurs exactly once, the repaired
anchor occurs exactly once in memory, the parent remains byte frozen, and all
FTD-0979 gates run to completion.

## 4. Classifier

- inherited all gates pass and FTD-0979 prints Outcome B: verifier repair
  succeeds;
- any inherited failure, anchor mismatch, hash mismatch, or other outcome:
  Outcome D.
