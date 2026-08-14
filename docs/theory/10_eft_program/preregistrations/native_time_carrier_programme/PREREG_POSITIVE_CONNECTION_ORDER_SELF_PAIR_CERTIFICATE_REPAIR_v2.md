# FTD-0903 — positive connection order/self-pair certificate repair v2

**Identifier:** `FTD-0903`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN REPAIR]`  
**Date:** 2026-08-11  
**Parent:** FTD-0902, execution-invalid

## 1. Parent record

| parent | protocol SHA256 | certificate SHA256 | result |
|---|---|---|---|
| FTD-0902 | `568F98C7AF01FC48DEAFEDC773FF33A129D089AFC606511C2D3C9F1C45D37061` | `C56907311B93942ABD7CD3DA96882CDC811EA333526C11C30F9C7BE004EB107C` | exact certificate `80/81`; C32 alone failed |

All source locks and substantive gates passed. C32 tested the registered rest
sector identity

\[
\left.\frac{(P-\gamma u)^2}{2M}\right|_{P=0,\,u^2=D^4}
=\frac{\gamma^2D^4}{2M}.                                \tag{1}
\]

The frozen certificate supplied `P: 0` and `u**2: D**4` in one SymPy
dictionary substitution. Simultaneous replacement did not expose `u**2`
inside the square after replacing `P`, so the comparison retained `u**2` and
failed. Sequential substitution of `P=0` followed by `u**2=D**4` evaluates
the same registered expression to exact zero. This is a representation-order
defect, not a change to (1).

No theorem is booked from FTD-0902.

## 2. Sole permitted repair

The v2 wrapper may make exactly one in-memory substitution in the frozen
FTD-0902 certificate:

```python
H_expected.subs({P: 0, u**2: D**4})
```

becomes

```python
H_expected.subs(P, 0).subs(u**2, D**4)
```

The wrapper must verify:

1. the repair protocol and parent certificate hashes;
2. the old form occurs exactly once;
3. the new form is absent before repair;
4. the repaired source contains the new form exactly once and no old form;
5. the compiled inherited certificate exits zero.

It may not modify any source hash, symbol, equation, assumption, comparison,
threshold, physical gate, outcome, or scope ceiling.

## 3. Inherited outcome and firewall

All 81 FTD-0902 gates and its Outcome A/B/C definitions are inherited.

```text
PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED
REPAIR_COUNT=EXACTLY_ONE
REPAIR_SCOPE=C32_SEQUENTIAL_SUBSTITUTION_ORDER_ONLY
MATHEMATICS_THRESHOLDS_SOURCES_OUTCOMES=UNCHANGED
SIGNED_SELF_PAIR_CONNECTION=IMPOSED_REFERENCE_LAW
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
MOVING_SECTOR_EXACT_QUARTIC=FALSE_GENERICALLY
PRODUCTION_INTEGRATION=FORBIDDEN
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this repair protocol and wrapper must be entered in the
preregistration manifest before first execution.
