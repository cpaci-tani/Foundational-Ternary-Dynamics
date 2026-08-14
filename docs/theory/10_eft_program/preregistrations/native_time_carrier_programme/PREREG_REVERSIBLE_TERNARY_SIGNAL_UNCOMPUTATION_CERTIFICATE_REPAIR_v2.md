# PRE-REGISTRATION — Reversible ternary signal uncomputation certificate repair v2

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0871`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED OUTCOME A 40/40]`  
**Parent:** `FTD-0870`, execution-invalid at `39/40`

## 1. Frozen parent

| Artifact | Frozen pre-run hash |
|---|---|
| FTD-0870 protocol | `B7CC6479ADCABDEB1A781C1F1FEF16735E31FAB40C1961FDC4DE6D358964E4FF` |
| `scripts/proofs/proof_reversible_ternary_signal_uncomputation.py` | `EF83868D6120D97E0F99C1D7B049CD4A22AB481FF1FD402FE6C777045FE2ECCD` |

The parent passed 39 checks. Only C35 failed. Its protocol marker spans a
single Markdown line break:

```text
it does not
derive a zero-work physical trajectory
```

The verifier searched for the same words separated by one ordinary space.

## 2. Only permitted repair

The wrapper must:

1. verify the frozen parent-script hash;
2. replace exactly one C35 membership test so it applies
   `" ".join(protocol_text.split())` before searching for the unchanged marker;
3. execute all inherited 40 checks in memory without modifying the parent; and
4. return success only for inherited `40/40`.

No source hash, ternary encoding, equation, reset map, inverse, information
bound, energy statement, handoff, rail capacity, outcome, or scope firewall
may change.

## 3. Locked implementation and outcomes

The unrun repair wrapper is

```text
scripts/proofs/proof_reversible_ternary_signal_uncomputation_v2.py
```

- **Repaired Outcome A:** exactly the permitted C35 whitespace normalization
  occurs and all inherited checks pass `40/40`.
- **Outcome C:** parent hash mismatch, replacement-count mismatch, or any
  inherited failure. Book no theorem.

Expected result: repaired Outcome A. This expectation is frozen before first
execution.

No physics content, fit, tolerance, production path, or completeness claim
changes.

## 4. Recorded outcome

The wrapper verified the frozen parent hash, performed exactly the permitted
C35 in-memory whitespace normalization, and the inherited certificate passed
`40/40`. FTD-0870 remains execution-invalid at `39/40`; the positive theorem
is booked only from FTD-0871.
