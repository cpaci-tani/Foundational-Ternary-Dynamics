# FTD-0938 — Phase-gated primitive C4 connection certificate repair v2

**Identifier:** `FTD-0938`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0937, execution-invalid

## 1. Parent record

| parent | protocol SHA-256 | certificate SHA-256 | result |
|---|---|---|---|
| FTD-0937 | `CDDDC452A94938945728571D8677E5CE4F1BD9A0EAEA840A8F4323D22F0E7823` | `FB14DD1CB379C38AAC9A241E4E2373E2CE511F8C988139ED47EBD50F2315057D` | `126/128`; all source, group, connection, Hamiltonian, wake, scale, carry, and scope gates passed; one unevaluated SymPy matrix-factor comparison failed, followed by the dependent Outcome-A flag |

The failed mathematical line constructed

\[
 \Delta K=(P-g\gamma q_1^2u)-(P-g\gamma q_0^2u)
\]

and compared it structurally with

\[
 -g\gamma(q_1^2-q_0^2)u.
\]

SymPy retained different factorizations of the same matrix expression. No
equation, assumption, source, threshold, or physical discriminator failed.
No theorem is booked from the FTD-0937 execution.

## 2. Sole permitted repair

The v2 wrapper may replace exactly this one parent-certificate comparison:

```python
delta_mechanical == -gate * gamma * (q1**2 - q0**2) * u
```

with the algebraically identical zero-residual comparison

```python
sp.simplify(
    delta_mechanical + gate * gamma * (q1**2 - q0**2) * u
) == sp.zeros(3, 1)
```

The wrapper must:

1. verify the frozen parent certificate hash;
2. verify the old form occurs exactly once;
3. verify the new form is absent before repair;
4. apply the substitution once in memory;
5. compile and execute the repaired parent in memory; and
6. require the inherited certificate to exit zero.

It may not modify the parent file or any source hash, symbol, equation,
assumption, inequality, outcome definition, physical gate, or scope ceiling.

## 3. Inherited outcome and firewall

All 128 FTD-0937 gates and its Outcome A/B/C definitions are inherited.

```text
PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=SYMPY_MATRIX_ZERO_RESIDUAL_NORMALIZATION
MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES=UNCHANGED
MINIMUM_REGISTERED_CONNECTION=A_g=g*gamma*q^2*u_live
NAIVE_CLOSED_SOURCE_PLUS_WAKE_COMPOSITION=NOT_ENERGY_CONSERVING
DIRECTION_PLUS_WAKE_IDENTIFIES_REAL_IMPULSE=FALSE
PRODUCTION_INTEGRATION=FORBIDDEN
```

The exact SHA-256 of this repair protocol and wrapper must be entered in the
preregistration manifest before any theorem is booked.
