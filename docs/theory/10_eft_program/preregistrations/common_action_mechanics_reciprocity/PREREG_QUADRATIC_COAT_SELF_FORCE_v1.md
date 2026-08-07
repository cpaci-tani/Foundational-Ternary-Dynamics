# PRE-REGISTRATION — Quadratic-coat self-force discriminator

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0552`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0551`  
**Scope:** observer-only static and multi-tick self-force test. No production
state, force, toggle, default, or scenario changes.

## 1. Locked isolated periodic control

Represent one quadratic charge coat `rho_q(x;r)` at rest and add the uniform
neutralizing background `-q/L^3`. Solve the unique zero-mean periodic
minimum-energy longitudinal field

```text
div E=rho_q-q/L^3,     C^T E=0                   (1)
```

by conjugate gradients to residual below `1e-13`. Set the half-step magnetic
field and initial momentum to zero. Advance only the FTD-0551 transaction;
the uniform background remains stationary.

No external field, force, self-field subtraction, pinning rule, damping, or
post-hoc recentering is permitted.

## 2. Locked positions and gates

Use `L=17,33`, both polarities, and the three subcell positions

```text
r0=(0,0,0),
r1=(0.5,0,0),
r2=(0.173,-0.219,0.287).                          (2)
```

Run 64 ticks. Require every nonlinear solve, continuity, Gauss, one-step
energy, total accumulated energy, and inverse residual below `1e-12`.

The physical static gate is

```text
max_t |x(t)-x(0)| < 1e-12,
max_t |p(t)|      < 1e-12.                        (3)
```

Require polarity mirror, integer translation, and cyclic rotation residuals
below `1e-12`.

## 3. Locked verdicts

- all generic and symmetric positions satisfy (3):
  `QUADRATIC_COAT_SELF_FORCE_ABSENT`;
- algebraic identities close but any generic position violates (3):
  `UNSUBTRACTED_QUADRATIC_SELF_FORCE_PRESENT`;
- Gauss, energy, or nonlinear solves fail first:
  `QUADRATIC_COAT_MULTITICK_ALGEBRA_FAILS`.

The second verdict closes the unmodified FTD-0551 transaction as an isolated
mobile-particle law. It does not authorize self-field subtraction; that would
be a separately selected interaction requiring its own local energy and
gauge derivation. Symmetric lattice positions passing while generic positions
fail counts as lattice pinning, not static qualification.

## 4. Run disposition

Run 2026-07-26 on the pinned MSVC CPU observer. Eight symmetry-position arms
remain static, but all four generic-subcell arms move while the algebraic
energy ledger remains exact. The registered verdict is
`UNSUBTRACTED_QUADRATIC_SELF_FORCE_PRESENT`. See
[`AUDIT_QUADRATIC_COAT_SELF_FORCE.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_QUADRATIC_COAT_SELF_FORCE.md).
