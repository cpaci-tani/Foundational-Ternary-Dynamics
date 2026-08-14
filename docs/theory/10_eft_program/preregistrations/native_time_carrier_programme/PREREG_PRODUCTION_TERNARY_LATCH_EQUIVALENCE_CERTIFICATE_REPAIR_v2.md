# FTD-0850 — Production ternary-latch equivalence certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 30/30]`  
**Date:** 2026-08-10  
**Parent:** FTD-0849 invalid `28/30` certificate  
**Scope:** exact verifier-only repair of C19  
**Production impact:** none

## 1. Frozen defect

FTD-0849 passed all nine source hashes and every independent gate. C19 asked
SymPy whether the unsimplified difference-of-squares
`field_withdrawal` was positive. SymPy returned undecided, despite C16 having
already proved

```text
field_withdrawal = k_g*x + k_g^2/2 = k_g*(x+k_g/2).
```

Both factors are strictly positive under the frozen declarations. C30 failed
only because it requires every previous gate.

## 2. Frozen parent inputs

| Input | SHA-256 |
|---|---|
| post-run FTD-0849 protocol | `26FBECC8E52DB8D523AB5B6EB889D7F6679AE93541BB9DC209F901E96AB3BD51` |
| invalid FTD-0849 script | `BABEB15BEB639D947F664D05972D38E9246CAFBDDB5908FD79479D5894A491B9` |

All sources, formulas, source-semantic anchors, 30 labels/order, equivalence
requirements, outcomes, and expected Outcome B are inherited unchanged.

## 3. Only permitted repair

The wrapper must fail closed unless both parent hashes match. It may replace
only C19's undecided positivity query with:

1. the exact C16 factorization check
   `field_withdrawal-k_g*(x+k_g/2)=0`; and
2. direct positivity-property checks for the two factors `k_g` and
   `x+k_g/2`.

C30 may change only because the repaired C19 Boolean enters its inherited
all-gates conjunction. No mathematical class, source, coefficient, physical
gate, label, count, outcome, or interpretation may change.

## 4. Locked implementation

```text
scripts/proofs/proof_production_ternary_latch_equivalence_v2.py
```

Frozen wrapper SHA-256:

```text
376606CAB83B9B7A35B324054F4958AB94DAFFB076A7A699711AB7A596095391
```

The wrapper hash and this protocol's pre-run hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_production_ternary_latch_equivalence_v2.py
```

## 5. Outcomes

- `30/30`: register FTD-0850 as the repaired scoped production boundary;
- any failure: repair invalid, book no production verdict, preserve both runs.

The expected result remains Outcome B: production supplies ternary/sign/loss
fragments but not strict unlocked basin persistence or an exact event-level
bath/controller ledger.

## 6. Recorded outcome

The first locked repair execution returned `30/30 PASS`. Only C19 changed,
exactly as registered. Outcome B is booked in
[`THEOREM_PRODUCTION_TERNARY_LATCH_BOUNDARY_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_PRODUCTION_TERNARY_LATCH_BOUNDARY_v1.md).
