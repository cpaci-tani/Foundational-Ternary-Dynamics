# FTD-0900 — common/relative connection gearbox certificate repair v2

**Identifier:** `FTD-0900`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0899, execution-invalid after `C41`

## 1. Parent record

The frozen FTD-0899 protocol has SHA-256
`38B7B6C929CC10F3F296FBA56A36478790D5AD648F8F9D2603058EE58F245AA0`.
Its frozen certificate has SHA-256
`75426CCCE016C6471583BB65FD2D9C608D27AE871C44643F1A224F2C867176AB`.

The first immutable execution passed every gate through `C41`, then raised

```text
sympy.matrices.exceptions.ShapeError:
Matrix size mismatch: (3, 1) + (0, 0)
```

before `C42`. The helper generated component names with

```python
sp.symbols(f"{prefix}0:3", real=True)
```

When `prefix="d1"`, SymPy reads `d10:3` as an empty descending range. This is
a certificate representation defect. No source hash, equation, inequality,
physical gate, outcome, or scope marker failed.

## 2. Sole permitted repair

The repair wrapper may replace exactly one occurrence of

```python
return sp.Matrix(sp.symbols(f"{prefix}0:3", real=True))
```

with

```python
return sp.Matrix(sp.symbols(f"{prefix}_0:3", real=True))
```

in memory before executing the parent certificate. The underscore changes
only the generated symbol spelling. It changes no algebraic object, dimension,
assumption, source, equation, test, threshold, outcome, or scope ceiling.

Any other textual substitution invalidates the repair.

## 3. Inherited gates and outcome

All FTD-0899 gates and Outcome A/B/C definitions are inherited unchanged.
The wrapper must verify both parent hashes, the exact one-occurrence old/new
replacement contract, compile the repaired source in memory, execute every
parent gate, and accept only a zero parent exit.

```text
PARENT_PROTOCOL_HASH=PRESERVED
PARENT_CERTIFICATE_HASH=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=SYMPY_COMPONENT_SYMBOL_SEPARATOR_ONLY
EQUATIONS_THRESHOLDS_SOURCES_OUTCOMES=UNCHANGED
COMMON_RELATIVE_CONNECTION_ACTION=IMPOSED_REFERENCE_LAW
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
CONTINUOUS_NONZERO_CONNECTION_PRESERVES_CRITICAL_QUARTIC=FALSE_IN_REGISTERED_CLASS
PRODUCTION_INTEGRATION=FORBIDDEN
```

The exact SHA256 of this repair protocol and wrapper must be entered in the
preregistration manifest before first execution.
