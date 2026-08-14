# PRE-REGISTRATION — Clock-gated Hamiltonian exchange certificate repair v2

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0865`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED OUTCOME B 40/40]`  
**Parent:** `FTD-0864`, execution-invalid `39/40`

## 1. Frozen parent

The invalid parent is preserved unchanged:

| Artifact | Frozen hash |
|---|---|
| FTD-0864 pre-run protocol | `967A41A751ADC1DF4BFB7AAB257C2ADA8DEE05A76AB6FD3D19046DC54D9FF454` |
| `scripts/proofs/proof_clock_gated_hamiltonian_exchange.py` | `8B6192E2EBA9D9C2F9B121BA5E34C8141D2456C723781DCD7D515DB5BD740374` |

The first run passed all six source hashes and 33 of 34 subsequent exact gates.
The sole failure was C34.

## 2. Read-only diagnosis

Under `theta -> -theta`, SymPy returned the interaction term in the equivalent
forms

```text
 I_r*chi*epsilon*(1-cos(theta))
-I_r*chi*epsilon*(cos(theta)-1)
```

Direct structural equality is false, while

```text
simplify(H_reversed-H) == 0
```

is exact. This is the same verifier class already encountered in earlier
source-locked certificates: no equation, source, parameter, outcome, or scope
changes.

## 3. Sole permitted repair

The repair wrapper must:

1. verify the frozen parent script hash;
2. find exactly one occurrence of
   `reversed_hamiltonian == hamiltonian`;
3. replace it in memory with
   `sp.simplify(reversed_hamiltonian - hamiltonian) == 0`;
4. execute all inherited 40 checks without modifying the parent file; and
5. return success only if the inherited parent returns `40/40`.

No other replacement is permitted.

## 4. Locked implementation and outcomes

The unrun wrapper is

```text
scripts/proofs/proof_clock_gated_hamiltonian_exchange_v2.py
```

- **Repaired Outcome B:** the wrapper performs exactly one permitted
  replacement and the inherited certificate passes `40/40`.
- **Outcome C:** parent hash mismatch, replacement-count mismatch, or any
  inherited failure. Book no theorem.

Expected result: repaired Outcome B. This expectation is frozen before the
first wrapper execution.

No source, equation, coupling, parameter, tolerance, scope, or expected
physical verdict is changed.

## 5. Recorded outcome

The wrapper verified the frozen parent hash, performed exactly the one
permitted in-memory replacement, and the inherited certificate passed
`40/40`. FTD-0864 remains preserved execution-invalid at `39/40`; the theorem
is booked only from this repaired certificate.
