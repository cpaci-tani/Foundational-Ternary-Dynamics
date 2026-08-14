# FTD-0879 — Gauss-record canonical-reduction comment-marker repair v3

**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parent:** FTD-0878 invalid `64/66` repair  
**Scope:** verifier-only repair of C48's independent comment marker  
**Production impact:** none

## 1. Frozen defect

FTD-0878 correctly normalized C48's equation marker and corrected the terminal
counts. Its first C48 conjunct still searched for the absent phrase
`18-point isotropic Laplacian`. The frozen source actually says
`isotropic 18-point Poisson stencil`; it separately says
`18-point Laplacian` later in the same function. The normalized equation marker
was independently confirmed present.

FTD-0878 therefore remained `64/66`; C66 failed only because C48 did.

## 2. Frozen parent inputs

| Input | SHA256 |
|---|---|
| FTD-0878 repair protocol | `6625F17CEC5FA2EF0BD294990FE949E70129B270D0C49D86528677DF3BFB52C9` |
| invalid FTD-0878 wrapper | `226B5D1B417725FD97F3A29A7EF2A7C60536BBB85A61532AD56FA301137F4B76` |

The FTD-0877 parent protocol/certificate hashes, seven source hashes, all
equations, all 66 gates, all tolerances, all outcomes, and all scope ceilings
remain inherited unchanged.

## 3. Only permitted repair

The wrapper must fail closed unless both FTD-0878 hashes match. It may replace
exactly one in-memory string in the FTD-0878 wrapper:

```text
18-point isotropic Laplacian
->
isotropic 18-point Poisson stencil
```

No other substitution is permitted. The invalid FTD-0877 and FTD-0878 files
remain unchanged on disk.

## 4. Locked implementation and outcome

The implementation is
`scripts/proofs/proof_gauss_record_canonical_reduction_v3.py`. Its SHA256 and
this protocol's pre-run SHA256 must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution.

Run exactly:

```text
python scripts/proofs/proof_gauss_record_canonical_reduction_v3.py
```

- `66/66`: repaired Outcome A;
- any failure: preserve all three attempts and book no theorem.
