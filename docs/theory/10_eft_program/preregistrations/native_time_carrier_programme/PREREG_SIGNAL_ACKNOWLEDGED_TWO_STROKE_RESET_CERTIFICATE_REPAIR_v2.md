# PRE-REGISTRATION — Signal-acknowledged two-stroke reset certificate repair v2

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0869`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED OUTCOME B 44/44]`  
**Parent:** `FTD-0868`, execution-invalid and aborted after C26

## 1. Frozen parent

| Artifact | Frozen pre-run hash |
|---|---|
| FTD-0868 protocol | `7880E72A78261324E97CB3F70B62012C8438BE66F0152F886049FB4923A5C94D` |
| `scripts/proofs/proof_signal_acknowledged_two_stroke_reset.py` | `FAC4A5BA4095742CFCFC8DB4DDE145E942E29254417EB87645648458B643E9FB` |

The parent passed all four source hashes, C5--C20, and C22--C26. C21 failed;
construction of C27 then raised `TypeError`, so C27--C44 were not executed.

## 2. Exact read-only diagnosis

For positive `B,I_0`, SymPy returns

```text
solve_univariate_inequality(I_0-B>0,I_0) == (B < I_0)
```

as a relational Boolean, not `Interval.open(B,oo)`. The two expressions denote
the same strict reserve, but structural equality is false.

For C27, `partial_signal_energy>0` is SymPy `BooleanTrue`. Python 3.13 does not
permit `int(BooleanTrue)` directly in this environment, but
`int(bool(BooleanTrue))` is exactly one. No predicate changes.

## 3. Only permitted repairs

The wrapper must:

1. verify the frozen parent-script hash;
2. replace exactly one C21 comparison target from `Interval.open(B,oo)` to the
   exact returned relation `(B < action0)`;
3. replace exactly one C27 conversion so both SymPy Booleans pass through
   Python `bool` before `int`;
4. execute all inherited 44 checks in memory without modifying the parent; and
5. return success only for inherited `44/44`.

No source, equation, inequality, waveform, coupling, acknowledgement, reset
law, energy account, export rule, outcome, or scope firewall may change.

## 4. Locked implementation and outcomes

The unrun repair wrapper is

```text
scripts/proofs/proof_signal_acknowledged_two_stroke_reset_v2.py
```

- **Repaired Outcome B:** exactly both permitted verifier repairs occur and the
  inherited certificate passes `44/44`.
- **Outcome C:** parent hash mismatch, replacement-count mismatch, or any
  inherited failure. Book no theorem.

Expected result: repaired Outcome B. This expectation is frozen before the
first wrapper execution.

No physics content, tolerance, fit, search, production path, or completeness
claim changes.

## 5. Recorded outcome

The wrapper verified the frozen parent hash, performed exactly the two
permitted in-memory representation repairs, and the inherited certificate
passed `44/44`. FTD-0868 remains execution-invalid and aborted after C26; the
positive theorem is booked only from FTD-0869.
