# PRE-REGISTRATION — Quadratic-coat discrete-gradient transaction

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0551`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0479`, `FTD-0544`, `FTD-0549`, `FTD-0550`  
**Scope:** observer-only selected discrete-gradient matter/field transaction.
Production state, force, tick, toggle, default, and scenarios remain unchanged.

## 1. Locked transaction

For endpoints `p0,p1`, use the production dispersion and its exact discrete
gradient

```text
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2),
vbar=C_SPEED^2 (p0+p1)/(H(p0)+H(p1)),
x1=x0+h vbar.                                    (1)
```

Deposit the FTD-0541 quadratic current `K[x0,x1]`. Advance the matched field
by the unchanged FTD-0544 staggered step

```text
B1/2'=B1/2-lambda C^T E0,
E*=E0+lambda C B1/2',
E1=E*-K,                lambda=C_SPEED h.         (2)
```

Gather the midpoint face field `Ebar=(E0+E1)/2` and updated half-step edge
field along the orbit with FTD-0550. With the single FTD-0478 normalization
coefficient `g`, solve

```text
p1-p0=g h q[Ebar_orbit+vbar cross B_orbit].       (3)
```

No `grad|J|`, Poisson force, legacy Coulomb/Lorentz force, force amplifier,
self-field subtraction, or post-hoc energy correction is permitted.

## 2. Locked identities and closure gates

Equation (1) must satisfy

```text
H(p1)-H(p0)=vbar dot (p1-p0).                     (4)
```

FTD-0550 and magnetic antisymmetry then require

```text
Delta H_matter=g <Ebar,K>,
Delta H_field=-g <Ebar,K>,
vbar dot Delta p_B=0.                             (5)
```

Require nonlinear solve, continuity, both endpoint Gauss laws, force balance,
discrete-gradient identity, electric work, field work, total energy,
kinematics, magnetic zero work, causality, and recorded algebraic inverse
below `1e-12`. Nonconvergence and invalid inputs fail closed.

## 3. Locked campaign and verdicts

Use `L=17`, both polarities, six axial/diagonal initial velocities, three
deterministic transverse-field amplitudes, and two integer translations: 72
neutral periodic arms. Add an independently transformed cyclic-rotation arm
and a deliberately impossible solver budget.

- all algebraic, covariance, inverse, and fail-closed gates pass:
  `QUADRATIC_COAT_DG_TRANSACTION_CONSTRUCTIVE`;
- energy/Gauss/continuity can close only after a new correction or independent
  force rule:
  `QUADRATIC_COAT_DG_TRANSACTION_CLOSED_NEGATIVE`;
- solver roots are not stable enough to classify:
  `QUADRATIC_COAT_DG_TRANSACTION_UNRESOLVED`.

A constructive result licenses only an observer transaction. FTD-0549 means
it is a selected discrete-gradient integrator, not yet the exact fixed-time
spacetime gauge action. A default-off mobile branch still requires multi-tick
static, ballistic, packet-force, hop, and forward/reverse gates.

## 4. Run disposition

Run 2026-07-26 on the pinned MSVC CPU observer: all 72 nonlinear roots and
every locked gate passed. The registered verdict is
`QUADRATIC_COAT_DG_TRANSACTION_CONSTRUCTIVE`. See
[`AUDIT_QUADRATIC_COAT_DISCRETE_GRADIENT_TRANSACTION.md`](../../07_assessment/AUDIT_QUADRATIC_COAT_DISCRETE_GRADIENT_TRANSACTION.md).
