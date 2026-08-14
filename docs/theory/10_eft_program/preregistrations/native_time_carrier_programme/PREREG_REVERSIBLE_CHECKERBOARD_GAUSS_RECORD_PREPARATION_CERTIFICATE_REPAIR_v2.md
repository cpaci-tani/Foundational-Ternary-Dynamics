# FTD-0882 — Reversible checkerboard Gauss-preparation certificate repair v2

**Identifier:** `FTD-0882`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** `FTD-0881`  
**Production status:** unchanged

## 1. Failure being repaired

The first locked FTD-0881 execution reported `58/60`. Provenance C1--C8 and
every substantive mathematical, locality, reversibility, convergence-rate,
energy, information, and scope gate other than C34 passed. C34 searched the
literal contiguous substring

```text
common affine intersection
```

but the frozen protocol wraps that phrase across a Markdown newline as
`common\n  affine intersection`. C60 then failed only because it correctly
depends on C34.

This is a verifier representation defect. It does not change the theorem,
equations, source set, `L=4` probe, eight-sweep exact-arithmetic witness,
`4/9` contraction ceiling, energy ledger, scope, or outcome rule.

## 2. Frozen parent hashes

| Artifact | SHA-256 |
|---|---|
| `PREREG_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_v1.md` | `50816F74F87D6120C871031D25EF704479B3E4873EB4F108080516C74E298942` |
| `scripts/proofs/proof_reversible_checkerboard_gauss_record_preparation.py` | `99B570E8E8CFD8FB7474060F3B0114281F2C2F02E92F47BA77E33139414EB634` |

Both parent artifacts remain byte-frozen. Any mismatch invalidates the repair.

## 3. Sole permitted substitution

The wrapper must find exactly one occurrence of

```python
      and "common affine intersection" in protocol_text)
```

and replace it in memory with

```python
      and "common affine intersection" in " ".join(protocol_text.split()))
```

No other source text may change. The wrapper must verify that the old anchor
occurs exactly once, that the replacement occurs exactly once afterward, and
that the parent protocol and certificate hashes match section 2 before
executing the repaired in-memory certificate.

## 4. Inherited gates and outcome

All sixty FTD-0881 gates, their order, exact arithmetic, thresholds, terminal
markers, and outcome rule are inherited unchanged. The expected mechanical
effect is:

- C34 recognizes the already frozen line-wrapped protocol statement; and
- C60 passes automatically if C1--C59 then all pass.

The permitted successful result remains exactly FTD-0881 Outcome A. Any other
failure is preserved and requires a new preregistered repair.

## 5. Scope firewall

```text
REPAIR_SCOPE=C34_WHITESPACE_NORMALIZATION_ONLY
PARENT_PROTOCOL_UNCHANGED=TRUE
PARENT_CERTIFICATE_UNCHANGED=TRUE
EQUATIONS_PROBES_THRESHOLDS_UNCHANGED=TRUE
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

## 6. Pre-run lock

The exact SHA-256 of this repair protocol and the wrapper must be recorded in
the preregistration manifest before first execution.
