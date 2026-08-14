# FTD-0880 — Gauss-record canonical-reduction exact-anchor repair v4

**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parents:** FTD-0878 invalid `64/66`; FTD-0879 fail-closed before execution  
**Scope:** verifier-only exact-anchor repair of FTD-0878 C48  
**Production impact:** none

## 1. Frozen defect

FTD-0879 searched the FTD-0878 wrapper for the bare phrase
`18-point isotropic Laplacian`. That phrase occurs twice because the wrapper
contains both the old and replacement C48 source strings. Its uniqueness guard
therefore aborted before any parent certificate execution.

## 2. Frozen inputs

| Input | SHA256 |
|---|---|
| FTD-0878 repair protocol | `6625F17CEC5FA2EF0BD294990FE949E70129B270D0C49D86528677DF3BFB52C9` |
| FTD-0878 wrapper | `226B5D1B417725FD97F3A29A7EF2A7C60536BBB85A61532AD56FA301137F4B76` |
| invalid FTD-0879 protocol | `A7CE50DAC58D3D45E71CEEC8E3708562CABCAB4636052A39F793C81385C96915` |
| fail-closed FTD-0879 wrapper | `4FBF611151D4F7139BCB79C38FE491A01380F3A1EA0C1BC62D67BEDE0E00661A` |

All FTD-0877 source hashes, algebra, witnesses, equations, 66 gates,
tolerances, outcomes, and scope ceilings remain inherited unchanged.

## 3. Only permitted repair

The wrapper must fail closed unless all four hashes match. It may replace one
exact multiline `new_c48` assignment in the FTD-0878 wrapper, changing only:

```text
"18-point isotropic Laplacian" in poisson_cpp
```

to

```text
"isotropic 18-point Poisson stencil" in poisson_cpp
```

The whitespace-normalized equation conjunct and both count repairs remain
byte-for-byte unchanged. All invalid parents remain unchanged on disk.

## 4. Locked implementation and outcome

The implementation is
`scripts/proofs/proof_gauss_record_canonical_reduction_v4.py`. Its SHA256 and
this protocol's SHA256 must be entered in `REF_PREREGISTER_MANIFEST.md` before
first execution.

Run exactly:

```text
python scripts/proofs/proof_gauss_record_canonical_reduction_v4.py
```

- `66/66`: repaired Outcome A;
- any failure: preserve all attempts and book no theorem.
