# AUDIT — Fixed-step variational energy scope

**Date:** 2026-07-26  
**Identifier:** `FTD-0543`  
**Status:** `[THEOREM — VARIATIONAL SCOPE]` +
`[CONSTRUCTIVE — EXACT COUNTEREXAMPLE AND DISCRETE-GRADIENT PRICE]`  
**Verdict:** `FIXED_STEP_ACTION_ENERGY_NOT_AUTOMATIC`  
**Pre-registration:**
[`PREREG_FIXED_STEP_ENERGY_SCOPE_v1.md`](../10_eft_program/preregistrations/PREREG_FIXED_STEP_ENERGY_SCOPE_v1.md)  
**Theorem:**
[`THEOREM_FIXED_STEP_ENERGY_SCOPE.md`](../10_eft_program/derivations/THEOREM_FIXED_STEP_ENERGY_SCOPE.md)  
**Run of record:** `engine/results/ftd_0543/windows_msvc_cpu.json`

## Result

The fixed-step configuration equations and the time-node equation are
independent variations. The latter conserves `E_d=-D_h L_d`; it is absent
when the tick duration is frozen.

The registered quartic midpoint witness closes exactly:

```text
p0                                  1.0625 = 17/16
p1                                  0.9375 = 15/16
endpoint energy defect              0.125 = 1/8
discrete Lagrangian energy          0.515625 = 33/64
worst analytic-identity residual    5.6898930012039273e-16
```

Thus a valid fixed-step Legendre map need not preserve its endpoint
Hamiltonian. The exact discrete-gradient comparison preserves energy to
`4.4408920985006262e-16`, but its registered phase-area determinant is
`9/11`, with defect `-2/11`. Exact energy can be bought, but this simple route
is not symplectic/area preserving.

## Consequence

FTD-0542 is a genuine common interaction, but it is not yet a mobile law. The
next candidate must explicitly test a special invariant of the full
coat-Maxwell map before adding a new temporal primitive. If that fails, the
remaining honest routes are a variable lapse or a selected energy-preserving
discrete gradient with an independent reversibility/spectral audit.

No production state, default, tick phase, force, toggle, scenario, energy
definition, or tolerance changed.

## Reproducibility

- test: `test_fixed_step_energy_scope`, failures `0`;
- preregistration SHA256:
  `3842AF60024B1D9E749FF5EDB203BFC35772F40604CA568BCAE481DDF51B5450`;
- test SHA256:
  `79D3840599C891D82A7472CB282B6F11A3C6028F5DBF7B9DE9C99C3ED46FEE9A`;
- header SHA256:
  `4C47A80ED17E29DDFF970C449ADF230A8AC1860BB6977E74A32A7AB8604E5200`;
- source SHA256:
  `D897A27DEC7D0DD9AE3AD05E72EBAA230B859F372CE1362BB211D0C38E1B4D38`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU theorem observer.
