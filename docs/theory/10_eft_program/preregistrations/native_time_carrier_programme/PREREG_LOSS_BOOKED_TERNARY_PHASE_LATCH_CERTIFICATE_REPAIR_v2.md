# FTD-0848 — Loss-booked ternary phase-latch certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 30/30]`  
**Date:** 2026-08-10  
**Parent:** FTD-0847 invalid certificate  
**Scope:** exact verifier-only repair of the C25/C28 sample comparisons  
**Production impact:** none

## 1. Frozen defect

FTD-0847 passed all three source hashes and C4--C24, then its sample
implementation of the ternary quotient evaluated

```text
bool(-2*A < -A/sqrt(3))
```

with `A>0`. SymPy left the relational expression undecided and raised a
`TypeError`, so C25--C30 were not evaluated and the parent certificate is
invalid. No mathematical equality, inequality, coefficient, or registered
gate failed.

## 2. Frozen parent inputs

| Input | SHA-256 |
|---|---|
| post-run FTD-0847 protocol | `B559BD68C7FF3E8D20431A753433A9C431A68F3363623D74660A25E5AEE0D6CD` |
| invalid FTD-0847 script | `8C0D60C2B0624FC58BA00B9B4A76DA1B641C37D9E4873D991D9ED89CD30103CE` |

All FTD-0847 sources, equations, coefficients, 30 check labels/order, outcome
definitions, and expected Outcome B are inherited unchanged.

## 3. Only permitted repair

The wrapper must fail closed unless both parent hashes match. It may make
exactly one in-memory substitution: inside `rho`, divide the sample by the
strictly positive threshold `theta=A/sqrt(3)` and ask SymPy for the exact sign
of `value/theta+1` and `value/theta-1`. This cancels the positive symbolic
scale before ordering. The same function is used by C25 and C28.

No source file, mathematical class, physical gate, coefficient, tolerance,
check count, label, outcome, or interpretation may change. The invalid parent
remains unchanged on disk.

## 4. Locked implementation

```text
scripts/proofs/proof_loss_booked_ternary_phase_latch_v2.py
```

Frozen wrapper SHA-256:

```text
53BD66C2E8674169790766E7CEC149739C324673B6F0609A5F984F4F3F60377F
```

The wrapper SHA-256 and this protocol's pre-run SHA-256 must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_loss_booked_ternary_phase_latch_v2.py
```

## 5. Outcomes

- `30/30`: register FTD-0848 as the repaired scoped ternary-latch theorem;
- any failure: repair invalid, book no theorem, and preserve both attempts.

The expected result remains FTD-0847 Outcome B: a degree-minimum selected
sextic latch, an exact local damped/work-booked transaction, and an explicit
many-to-one ternary record quotient. Production realization, Born/selector
coupling, microscopic bath information, thermal cost, and `G*` cadence remain
open.

## 6. Recorded outcome

The first locked repair execution returned `30/30 PASS`. Only the comparison
inside `rho` changed, exactly as registered. Outcome B is booked in
[`THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md).
