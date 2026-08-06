# FTD-0635 — Connected-block full constituent Hessian v2

**Status:** `[PRE-REGISTRATION — LOCKED AFTER v1 DIAGNOSIS, BEFORE v2 IMPLEMENTATION/RUN]`  
**Parent:** FTD-0634 verdict
`CONNECTED_BLOCK_FULL_48D_HESSIAN_EXECUTION_INVALID`  
**Qualified state parent:** FTD-0633 verdict
`CONNECTED_BLOCK_EIGHT_FIBRE_STATIC_BASIN_CONSTRUCTIVE`  
**Scope:** numerical-conditioning repair of the full 48-coordinate gradient;
the v1 positive spectrum is disclosed and this run is not independent evidence  
**Date:** 2026-07-27

## 1. Locked diagnosis

FTD-0634 completed its entire Hessian. Both orientations had 48 positive
eigenvalues with minima near `1.676e-3`, but the run was execution-invalid:
the first derivative reused the Hessian step `2e-4` and measured a maximum
gradient near `2.292e-7`, above the locked `1e-8` gate. This scale is
consistent with the `O(h^2)` truncation bias of a central first derivative and
does not diagnose a negative Hessian mode.

## 2. Sole protocol repair

Repeat FTD-0634 exactly with one change:

- compute the 48 first derivatives with `h_g=2e-5`, the gradient step already
  qualified in FTD-0628/0633;
- retain `h_H=2e-4` for every Hessian and translation-curvature evaluation.

The gradient and Hessian stencils are therefore evaluated separately. Each arm
must complete 4,711 base evaluations: one center, 96 fine-gradient, 96
diagonal-Hessian, 4,512 off-diagonal-Hessian, and six translation evaluations.

No state, field, chart cap, action term, eigensolver, tolerance, eigenvalue
gate, covariance gate, or negative-mode discriminator changes.

## 3. Gates and verdicts

All FTD-0634 common gates and verdict thresholds are inherited verbatim. In
particular, gradient infinity norm remains `<=1e-8`, all redressings must pass,
translation Rayleigh consistency remains `<=1e-5`, cyclic eigenvalue
covariance remains `<=1e-6`, and a constructive basin still requires minimum
eigenvalue `>1e-5` in both orientations.

Verdicts use the same four strings as FTD-0634. A constructive verdict is a
numerically repaired qualification of the already disclosed v1 spectrum, not
a blind confirmation.

## 4. Artifacts

Produce a distinct observer CTest and `ftd_0635` JSON/CSV records, independent
certificate, analysis/audit, and synchronized canonical records. Preserve the
FTD-0634 invalid run. Production remains unchanged.
