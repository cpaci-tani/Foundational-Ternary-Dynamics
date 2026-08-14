# Pre-registration — Local work-port/factor certificate repair v2

**Identifier:** `FTD-0982`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Expected classifier:** inherited FTD-0981 `Outcome B`

## 1. Immutable parent record

- FTD-0981 protocol SHA-256:
  `7CF3DC6239200CF1B773ADEC0633F0B30CD5735C7FF8BDA1360F730888C5EDE3`.
- FTD-0981 proof SHA-256:
  `BDD16E3D4AB8BF0E0D4C72E5520638AB712D64E113725145B27F919B620F0C69`.
- First execution: `76/79` displayed checks passed and the final classifier
  was `Outcome D` because three verifier-only predicates failed.

The failed predicates were:

1. a case-sensitive source phrase expected `local quarter-turn` while the
   frozen theorem reads `A local quarter-turn`;
2. the `k<kappa^2` reserve limit was evaluated with no inequality
   substitution, so SymPy correctly refused to choose its sign; and
3. the `k>kappa^2` reserve limit had the same missing assumption.

All incidence, Dirac-factor, Laurent-inverse, symplectic-lift, inverse,
energy, and four-cycle identities passed. The parent files remain byte
frozen.

## 2. Authorized repairs

Exactly three in-memory source substitutions are authorized.

### R1 — source-marker capitalization

Replace

```python
"local quarter-turn is still a legitimate symplectic event" in trilemma_text
```

with

```python
"A local quarter-turn is still a legitimate symplectic event" in trilemma_text
```

### R2 — registered lower-stiffness branch

Replace

```python
sp.limit(positive_q_defect, amplitude, sp.oo) == sp.oo,
```

with

```python
sp.limit(positive_q_defect.subs(k_symbol, kappa**2 / 2), amplitude, sp.oo) == sp.oo,
```

The substitution is an exact positive witness to `0<k<kappa^2`; the defect
becomes `kappa^2 amplitude^2/4`.

### R3 — registered higher-stiffness branch

Replace

```python
sp.limit(positive_p_defect, amplitude, sp.oo) == sp.oo,
```

with

```python
sp.limit(positive_p_defect.subs(k_symbol, 2 * kappa**2), amplitude, sp.oo) == sp.oo,
```

The substitution is an exact positive witness to `k>kappa^2`; the defect
becomes `amplitude^2/2`.

These repairs add the inequalities already stated in the assertion labels.
They change no equation, source hash, gate, expected value, classifier, or
physical conclusion.

## 3. Forbidden changes

No other substitution, assertion change, gate waiver, expected-value change,
production mutation, numerical search, or scope promotion is allowed. The
wrapper must verify that each old anchor occurs exactly once, each repaired
anchor occurs exactly once in memory, the parent remains byte frozen, and all
FTD-0981 gates run to completion.

## 4. Classifier

- all inherited gates pass and FTD-0981 prints Outcome B: verifier repair
  succeeds;
- any inherited failure, anchor mismatch, hash mismatch, or other outcome:
  Outcome D.
