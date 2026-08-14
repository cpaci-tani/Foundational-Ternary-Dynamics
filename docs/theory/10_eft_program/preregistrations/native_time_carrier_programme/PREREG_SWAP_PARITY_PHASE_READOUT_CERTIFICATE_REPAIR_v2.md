# FTD-0846 — Swap-parity phase-readout certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 32/32]`  
**Date:** 2026-08-10  
**Parent:** FTD-0845 invalid `31/32` certificate  
**Scope:** exact verifier-only repair of C9  
**Production impact:** none

## 1. Frozen defect

FTD-0845 C9 evaluated

```text
factor(kappa*(a-q**2)**2/2) == kappa*(a-q**2)**2/2
```

by SymPy structural expression equality. SymPy returned the left expression
as `kappa*(-a+q**2)**2/2`; the two squared forms are algebraically identical
but structurally different. Their exact simplified difference is zero.

All five source hashes and C6--C8/C10--C32 passed. No theorem is booked from
the invalid parent.

## 2. Frozen parent inputs

| Input | SHA-256 |
|---|---|
| post-run FTD-0845 protocol | `0AACC3A6E33CB65DD045CBA82E6BF3ED8F6C522EBCA1B5DD5A217ADCDBDC6054` |
| invalid FTD-0845 script | `41E1D1E9043620D20E71A2B18EC72041D5BBC7298133F6C082F9FB877F58FB66` |

All sources, equations, coefficients, 32 check labels/order, outcome
definitions, and expected Outcome B are inherited unchanged.

## 3. Only permitted repair

The wrapper must fail closed unless both parent hashes match. It may make
exactly one in-memory substitution: C9 compares

```text
simplify(factor(W_plus) - kappa*(a-q**2)**2/2)
```

with exact zero. No source file, mathematical class, physical gate,
tolerance, coefficient, check count, or outcome may change. The invalid
parent remains unchanged on disk.

## 4. Locked implementation

```text
scripts/proofs/proof_swap_parity_phase_readout_v2.py
```

Wrapper SHA-256:
`F7E7C7D3C901F3CF80F2FB8B4A222DBE2897320664300EC5F868465A46B56C5C`

Pre-run protocol SHA-256:
`C769DFBF4125CCFE864D85B4CD604A8793FE51DF8F03B9672D07E67F0DB025AF`

The wrapper hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_swap_parity_phase_readout_v2.py
```

## 5. Outcomes

- `32/32`: register FTD-0846 as the repaired scoped readout theorem;
- any failure: repair invalid, book no theorem, and preserve both attempts.

The expected result remains FTD-0845 Outcome B: common/even readout sees only
the symmetric-square quotient; positive bilinear faithful readout destroys
criticality; the selected quartic odd pointer is the scoped degree-minimum
local conservative bridge with explicit backreaction.

## 6. Recorded outcome

The first locked repair execution returned `32/32 PASS`. Only C9 changed,
exactly as registered. Outcome B is booked in
[`THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md).
