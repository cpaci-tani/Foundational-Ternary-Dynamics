# FTD-0951 — Preregistration: causal work-booked C18 relaxation certificate repair v2

**Identifier:** `FTD-0951`  
**Date:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Parent:** `FTD-0950`

## 1. Failure record

The first immutable FTD-0950 execution passed `78/80` and returned Outcome D.
All source hashes, controller equations, causal-support checks, residual
identities, work/charge ledgers, inverse-port checks, and scope gates passed.

The sole primary failure was `G3 iterate-difference induction`. The verifier
compared

```python
next_increment_bound == b*c**(n+1)
```

by SymPy structural equality. The left side had already expanded one rational
factor while the right side retained a power. Their difference simplifies
exactly to zero. The second reported failure was only the deliberately chained
Outcome-A classifier responding to that primary false result.

This is a certificate-normalization defect, not evidence for or against the
registered mathematics.

## 2. Frozen parent hashes

| Artifact | SHA-256 |
|---|---|
| `PREREG_CAUSAL_WORK_BOOKED_C18_FINITE_RADIUS_RELAXATION_v1.md` | `12C21B138BCFFB0F8613194620F8D75A287E6DDD9E25EC40DF50E14B78220988` |
| `proof_causal_work_booked_c18_finite_radius_relaxation.py` | `A2690CAEAEA7363C5E14D492844B250874545EABC8AF029415B3671E69D45071` |

Both parent files remain byte-for-byte unchanged.

## 3. Sole authorized transformation

The wrapper may replace exactly one occurrence of

```python
next_increment_bound == b*c**(n+1), next_increment_bound
```

with

```python
sp.simplify(next_increment_bound - b*c**(n+1)) == 0, next_increment_bound
```

in memory before executing the parent certificate.

No other source line, hash, equation, constant, inequality, check, outcome,
ontology statement, or production file may change. The wrapper must fail
closed unless the old fragment occurs exactly once, the new fragment occurs
zero times in the frozen parent, exactly one substitution is performed, both
parent hashes match, and the inherited certificate exits zero.

## 4. Repair interpretation

If the repaired inherited run passes `80/80`, FTD-0950 earns its frozen
Outcome A through FTD-0951 verifier repair. If it does not, the branch remains
Outcome D and no theorem may be written.

This repair authorizes no tolerance, numerical search, parameter change,
empirical target, proof weakening, engine mutation, or production promotion.
