# FTD-0717 — Period-three common-action preflight v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parents:** FTD-0715 and FTD-0716

## Question

Do the independently compatible FTD-0715 matter momenta and minimum-norm
FTD-0716 co-moving field already satisfy the fixed common-action energy and
translation-recoil identities, or must the coupled solve select a different
field within the allowed solution family?

## Frozen replay

Load the FTD-0716 minimum-norm face/edge correction and the 48 FTD-0715
momentum segments. Reconstruct the same three exact quadratic-coat currents
and advance the field for three ticks in the unchanged matched order. Do not
modify the trajectory, momenta, current, field normalization, homogeneous
field content, or interaction scale.

Use the already selected face-flux normalization

\[
\beta=C_{\rm WAVE}^2\left(G_C/C_{\rm WAVE}^2\right)^2
\]

and measure on every tick:

1. matter energy change from the frozen production dispersion;
2. `beta` times the exact matched modified field-energy change;
3. matter impulse plus `beta` times the exact local translation
   pseudomomentum change;
4. matter impulse plus the FTD-0619 spline-Poynting momentum change;
5. the complete three-tick translated field residual.
6. absolute face-Gauss residual against the quadratic constituent density at
   all four matter phases.

The exact local pseudomomentum and spline-Poynting observer are both reported;
neither may be chosen post-result as the sole favorable definition.

## Gates and verdicts

Require parent hashes, 48 segment reconstruction, exact field source replay,
finite normalization, complete translated field residual `<=1e-10`, and
maximum absolute Gauss residual `<=1e-10`.

- `PERIOD_THREE_MINIMUM_NORM_COMMON_ACTION_PREFLIGHT_CONSTRUCTIVE` requires
  maximum per-tick total-energy residual, local translation-momentum defect,
  and spline-Poynting momentum defect all `<=1e-10`;
- `PERIOD_THREE_MINIMUM_NORM_FIELD_REQUIRES_COUPLED_SELECTION` applies when the
  replay is valid and translated return passes but Gauss or one or more
  common-action defects exceeds `1e-10`;
- `PERIOD_THREE_COMMON_ACTION_PREFLIGHT_EXECUTION_INVALID` applies to parent,
  reconstruction, normalization, non-finite, or translated-return failure.

A negative result closes only the independently chosen minimum-norm field as
the common-action solution. It does not authorize rescaling the field or
fitting a null mode after inspection. The next admissible candidate is a fresh
coupled solve in which field, momenta, recoil, and energy are solved
simultaneously under a new lock.
