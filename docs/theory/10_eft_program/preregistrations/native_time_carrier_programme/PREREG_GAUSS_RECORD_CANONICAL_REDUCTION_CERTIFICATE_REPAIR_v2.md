# FTD-0878 — Gauss-record canonical-reduction certificate repair v2

**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parent:** FTD-0877 invalid `64/66` certificate  
**Scope:** verifier-only repair of C48 and the dependent terminal counts  
**Production impact:** none

## 1. Frozen defects

FTD-0877 passed every algebraic gate and failed only:

1. **C48 source-marker representation.** The verifier searched for the
   one-line substring
   `INV3 * face_sum + INV6 * edge_sum - source[idx]`, while the frozen C++
   source line-wraps that same expression after `(`. Whitespace normalization
   makes the already frozen marker exact; no semantic source requirement
   changes.
2. **C66 terminal count.** The parent contains 65 preterminal checks and 66
   total checks, but its terminal predicate expected 68 preterminal and 69
   total checks. The labels/order/gates themselves are unchanged.

## 2. Frozen parent inputs

| Input | SHA256 |
|---|---|
| FTD-0877 protocol | `4F24779197A2DE93ABB10DCFC0F84D23EB528A80E96CC3D4F1A548A429F27F4A` |
| invalid FTD-0877 certificate | `AC787BADE1050341B47AC5B96C525EB7F871082AFD5DA85BB7361A8CF634D0BF` |

All seven source hashes, incidence matrices, equations, rational witnesses,
Fourier-symbol challenges, tolerances, physical gates, outcomes, and scope
ceilings are inherited unchanged.

## 3. Only permitted repair

The wrapper must fail closed unless both parent hashes match. It may make
exactly three in-memory substitutions:

1. C48 searches the same SOR marker after `" ".join(poisson_cpp.split())`;
2. C66 requires `checks == 65` and labels the preterminal count C65; and
3. the success branch requires `checks == 66`.

The invalid parent certificate remains unchanged on disk. No mathematical or
source gate may be added, removed, weakened, or retuned.

## 4. Locked implementation

```text
scripts/proofs/proof_gauss_record_canonical_reduction_v2.py
```

The wrapper SHA256 and this protocol's pre-run SHA256 must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_gauss_record_canonical_reduction_v2.py
```

## 5. Outcomes

- `66/66`: register FTD-0878 as the repaired exact FTD-0877 certificate and
  retain FTD-0877 Outcome A;
- any failure: repair invalid, book no theorem, and preserve both attempts.

The expected physical verdict is unchanged: exact matched canonical reduction
and static record section; no uniformly local charge conjugate; live
production projector closed negative as an exact projector; dynamic native
preparation and `G*` synchronization open.
