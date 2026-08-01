# FTD-0712 — Resonant internal-gait cancellation v1

**Status:** `[SELECTED KINEMATICS — BOUNDED FAMILY CLOSED NEGATIVE]`  
**Verdict:** `BOUNDED_INTERNAL_GAIT_CANNOT_CANCEL_LOCKED_RESONANCE`  
**Production status:** unchanged

Allowing the 16 constituents to deform at the midpoint while fixing the start,
translated endpoint, and center trajectory reduces the eight-mode null norm:

```text
4.6345148020027714e-4 -> 4.6132275275513028e-5
```

All eight Newton steps are accepted and all Gram pivots remain near
`1.12e-5`. The run stops because one constituent reaches the preregistered
`0.05` coordinate cap, not because the algebra or causal current fails.
Maximum speed is `0.550035 < 1/sqrt(3)`, edge deformation is `0.0544`, center
residual is zero, continuity is `4.44e-16`, and covariance is `7.03e-17`.

The bounded family therefore fails its `1e-10` cancellation gate, while the
monotone direction supplies evidence that the composite's internal current
can screen the rigid resonance. FTD-0713 tests the physically causal bound
without regrading this negative result.

Record: protocol `47BC6C88...E344852`; summary `DB9E76C4...E21FB`; proof
`C214AD0E...E13C2`.

