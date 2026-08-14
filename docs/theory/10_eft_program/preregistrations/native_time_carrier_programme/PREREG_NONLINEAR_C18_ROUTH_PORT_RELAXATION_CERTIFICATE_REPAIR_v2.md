# FTD-0953 — Preregistration: nonlinear C18 Routh-port certificate repair v2

**Identifier:** `FTD-0953`  
**Date:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — VERIFIER-ONLY REPAIR LOCKED BEFORE EXECUTION]`  
**Parent:** `FTD-0952`

## 1. Failure record

The first immutable FTD-0952 execution halted at the finite-horizon reserve
check before printing a verdict. The certificate had declared

```python
sigma, omega = sp.symbols("sigma omega", nonzero=True, real=True)
```

and later asked SymPy to decide whether the positive-capacity expression
`2*N*qmax`, proportional to `omega`, was greater than zero. SymPy correctly
refused because `nonzero` permits negative values.

The preregistered source fixes `omega^2=26 Lambda/25` and uses the positive
frequency branch. The failure is therefore a missing verifier assumption, not
a change to the mathematics or physical scope.

## 2. Frozen parent hashes

| Artifact | SHA-256 |
|---|---|
| `PREREG_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md` | `0326481C47902DBD3EBD9442D904BD37CE014CF551135FC50D1F6CEF953246F5` |
| `proof_nonlinear_c18_routh_port_relaxation_charge_reservoir_boundary.py` | `0E4C35A5C0B616A091B44906F10F1431086E88A0C1F19041DF2FA96E5496CFD5` |

Both parent files remain byte-for-byte unchanged.

## 3. Sole authorized transformation

The wrapper may replace exactly one occurrence of

```python
sigma, omega = sp.symbols("sigma omega", nonzero=True, real=True)
```

with

```python
sigma = sp.symbols("sigma", nonzero=True, real=True)
omega = sp.symbols("omega", positive=True, real=True)
```

in memory before executing the parent certificate.

The wrapper must fail closed unless both parent hashes match, the old fragment
occurs exactly once, the new fragment is absent, exactly one substitution is
performed, and the inherited certificate exits zero. It may not alter any
source hash, equation, constant, inequality, convergence argument, symplectic
test, outcome, scope statement, or production file.

## 4. Repair interpretation

If the repaired inherited certificate passes, FTD-0952 earns only its frozen
Outcome B: positive nonlinear Routh-port relaxation closes, while the
phase-blind physical charge/action reservoir remains noncanonical. If it does
not pass, the branch remains invalid and no theorem may be written.

No tolerance, search, fit, empirical substitution, proof weakening, engine
mutation, or production promotion is authorized.
