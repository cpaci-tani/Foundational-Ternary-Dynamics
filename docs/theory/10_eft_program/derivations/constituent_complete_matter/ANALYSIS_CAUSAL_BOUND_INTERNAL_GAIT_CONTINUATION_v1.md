# FTD-0713 — Causal-bound internal-gait continuation v1

**Status:** `[SELECTED KINEMATICS — SOURCE COMPATIBILITY CONSTRUCTIVE]`  
**Verdict:** `CAUSAL_INTERNAL_GAIT_CANCELLATION_CONSTRUCTIVE`  
**Production status:** unchanged

Starting from the locked FTD-0712 state and removing only its auxiliary `0.05`
coordinate cap, two full Newton steps reduce the resonance residual to:

```text
16-component max: 1.4076158628294926e-5 -> 9.8729439337516545e-13
eight-mode norm:  4.6132275275513028e-5 -> 2.3755150329194885e-12
```

The resulting gait is small and physically admissible within the selected
current model:

| gate | value |
|---|---:|
| maximum midpoint displacement | `0.0550894` |
| maximum segment speed | `0.555132 < 1/sqrt(3)` |
| maximum edge deformation | `0.059943` |
| center residual | `0` |
| continuity residual | `5.27e-16` |
| covariance residual | `7.16e-17` |

This proves that the existing extended constituent geometry has enough
current degrees of freedom to cancel the rigid field resonance. It is only a
kinematic compatibility witness. FTD-0714 proves that this unequal two-step
gait cannot be selected by the frozen two-tick momentum-return kinematics.

Record: protocol `901F2F2F...724BF`; summary `E32B5378...FE051`; proof
`491C2F0C...C566`.

