# AUDIT — Quadratic-coat neutral self-consistent pair work

**Date:** 2026-07-26  
**Identifier:** `FTD-0546`  
**Status:** `[DERIVED — EXACT NEUTRAL FIELD ALGEBRA] + [NUMERICAL FACT —
NONZERO ENERGY DEFECT] + [CLOSED NEGATIVE — FROZEN MINIMAL QUADRATIC-COAT
COMMON ACTION]`  
**Verdict:** `SELF_CONSISTENT_COAT_PAIR_ENERGY_CLOSES_NEGATIVE`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_NEUTRAL_PAIR_WORK_v1.md`](../10_eft_program/preregistrations/PREREG_QUADRATIC_COAT_NEUTRAL_PAIR_WORK_v1.md)  
**Derivation:**
[`DERIV_QUADRATIC_COAT_NEUTRAL_PAIR_WORK.md`](../10_eft_program/derivations/DERIV_QUADRATIC_COAT_NEUTRAL_PAIR_WORK.md)  
**Run of record:** `engine/results/ftd_0546/windows_msvc_cpu.json`

## Result

The neutral self-consistent longitudinal subfamily closes every source and
field identity but not total energy:

```text
registered arms                         128
native beta                             0.021892057692994273
worst Poisson residual                  9.9746831116148649e-14
worst algebra residual                  9.9747358810187148e-14
worst direct/deposited action residual  8.6736173798840355e-19
worst exact field-work residual         2.8968526796097072e-18
worst gauge residual                    6.9388939039072284e-18
largest T versus endpoint-average gap   2.2842481537704851e-04
largest pair energy defect              9.6808436326516136e-09
failures                                0
```

The exact temporal source `T` and endpoint currents construct

```text
E0=E_star+K0,
E1=E_star-K1,
div E0=rho0,
div E1=rho1.
```

The resulting field energy obeys `Delta U=-beta<Ebar,K>` exactly. The
action-derived matter endpoint energies do not gain the opposite amount. The
defect is separated from solver/algebra error by at least four orders of
magnitude and from action/work roundoff by nine orders.

## Verdict and scope

This completes the follow-up required by FTD-0545. The external harmonic
witness was not responsible for the failure: it survives on a neutral,
periodic, Gauss-realizable field sourced by the same coat worldlines.

The frozen minimal quadratic-coat common action is closed negative as an
exact-energy reciprocal mobile law. Therefore `common_action_face_dynamics`,
FTD-0481 mobile campaigns, FTD-0482 reciprocal dressing scenarios, and
FTD-0483 pole/IR claims remain unlicensed. The observer records remain useful
negative evidence.

No production state, default, force, phase, toggle, scenario, energy
projection, self-field subtraction, or tolerance changed.

## Reproducibility

- test: `test_quadratic_coat_neutral_pair_work`, `128` registered arms,
  failures `0`;
- preregistration SHA256:
  `C014A07639406963ED5343ADFF69F2B968105850D9B23D0C32B7C77728AADBDB`;
- test SHA256:
  `BEA6AC3D1AEEBDA8B02C7B3D27C8080925A49479C3F8204A350EBB3AFAF025D3`;
- header SHA256:
  `733AC99446E308DC91297D8B6F78CEFC6C946BDFF45A8B8C81E0C92C54C478C1`;
- source SHA256:
  `CF27301A153E47519F85B9DA33DC22E29ED2E9019D211ECFA90335738B7CBA35`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
