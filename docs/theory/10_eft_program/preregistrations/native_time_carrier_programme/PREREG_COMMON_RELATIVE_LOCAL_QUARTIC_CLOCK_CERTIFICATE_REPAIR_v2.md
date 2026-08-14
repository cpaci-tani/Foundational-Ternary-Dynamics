# FTD-0844 — Common/relative local quartic clock certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; REPAIRED EXACT CERTIFICATE 28/28]`  
**Date:** 2026-08-10  
**Parent:** FTD-0843 invalid `26/28` certificate  
**Scope:** exact verifier-only repair of C14 and dependent C28  
**Production impact:** none

## 1. Frozen defect

FTD-0843 C14 evaluated

```text
U.T * G * U == G
```

using SymPy structural matrix equality. The two matrices are algebraically
equal but not stored in the same unsimplified expression form. The exact
diagnostic

```text
simplify(U.T * G * U - G)
```

is the zero matrix. C28 failed only because it required `failed == 0` and
repeated the same structural comparison.

## 2. Frozen parent inputs

| Input | SHA-256 |
|---|---|
| post-run FTD-0843 protocol | `050EAC8DB2BDC0A7AA2116874F7F43A4F08D6246703004BB8C4573A0795A6F79` |
| invalid FTD-0843 script | `D5CCC53504E162D9999AAAE7F0142F7FD8EA98DBE153328059A6672C79B68076` |

All seven source hashes, all equations, all 28 check labels/order, all outcome
definitions, and the expected Outcome B are inherited unchanged.

## 3. Only permitted repair

The repair wrapper must fail closed unless both parent hashes match. It may
make exactly two in-memory substitutions:

1. C14 compares every entry of `simplify(U.T*G*U-G)` with exact zero.
2. C28 repeats that exact simplified-difference comparison.

No source file, equation, coefficient, tolerance, physical gate, or outcome
may change. The invalid parent script remains unchanged on disk.

## 4. Locked implementation

```text
scripts/proofs/proof_common_relative_local_quartic_clock_v2.py
```

Wrapper SHA-256:
`A2AEF445E7C9260CE0A546A0041A87744007F3A747CCD907EC5DBF92578EAC41`

Pre-run protocol SHA-256:
`B3045E5CDD0DEA22F6AFE9CA7379D1D5A458EA9A99CCCD89A1A4E2FC12B16FED`

The wrapper hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_common_relative_local_quartic_clock_v2.py
```

## 5. Outcomes

- `28/28`: register FTD-0844 as the repaired exact selected-carrier theorem;
- any failure: repair invalid, book no theorem, and preserve both attempts.

The expected result remains Outcome B from FTD-0843: an exact positive,
P4-local selected carrier with production cross-gradient, formation, readout,
maintenance, and finite-tick cadence open.

## 6. Recorded outcome

The first locked repair execution returned `28/28 PASS`. Only C14 and C28
changed, exactly as registered. Outcome B is booked in
[`THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md).
