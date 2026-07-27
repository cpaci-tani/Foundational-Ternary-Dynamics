# PRE-REGISTRATION — Implicit atomic face action

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0536`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0484`, `FTD-0490`, `FTD-0531`, `FTD-0533`,
`FTD-0535`  
**Scope:** observer-only reconstruction of the minimal one-slab phase-space
action forced by the exact FTD-0484 endpoint-current split. The registered
test asks whether the already locked FTD-0531 scalar energy roots are also
stationary endpoints of that action. No production state, default, toggle,
scenario, force, collision law, phase order, field ontology, normalization,
or tolerance changes.

## 1. Locked action

Let `lambda=C_SPEED`, let `beta` be the FTD-0478 mapped field-work
coefficient, and let `S_int^(1)` denote the unit-coupling FTD-0484 spacetime
worldline action including the signed ternary polarity. The candidate is

```text
S_d = S_m
    + beta/(2 lambda^2) ||A_1-A_0||^2
    - beta/2 ||C^T A_1||^2
    + (beta/lambda) S_int^(1).
```

The coefficients are not fitted. With

```text
E_slab=-(A_1-A_0)/lambda,
K^(0)+K^(1)=K,
B_1=C^T A_1,
```

the two connection variations must give

```text
E_0= E_slab+K^(0),
E_1= E_slab+lambda C B_1-K^(1),
E_1-E_0=lambda C B_1-K.
```

Given `(A_0,E_0)` and a trial worldline, the first equation fixes

```text
A_1=A_0-lambda(E_0-K^(0)).
```

For the FTD-0531 fixture use its locked `E_0`, choose the gauge representative
`A_0=lambda E_0`, and reconstruct `A_1`, `B_1`, and `E_1` from these equations.

## 2. Registered arms and estimators

Use all 240 FTD-0531 edge/corner roots: 20 signed diagonal directions, two
speeds `{0.125,0.25}`, two polarities, and three translations on `L=17`.
For each of the two carriers:

1. rebuild the exact FTD-0484 total, start, and end current deposits;
2. evaluate the complete deposited interaction action on the reconstructed
   slab with coupling `g=beta/lambda`;
3. differentiate both particle endpoints by five-point centered differences
   at `h=2^-12` and `h/2`, rebuilding every internal crossing on every probe;
4. combine those derivatives with the analytic FTD-0490 matter action;
5. form gauge-invariant endpoint kinetic Legendre momenta by subtracting the
   interpolated endpoint connection term;
6. compare them to the FTD-0531 registered incoming and outgoing momenta.

The following quantities are frozen before execution:

- split recombination and continuity residuals;
- both connection Euler-equation residuals and the endpoint field-update
  identity;
- fine/coarse endpoint derivative convergence;
- start and end kinetic Legendre residuals, decomposed into longitudinal and
  transverse parts;
- the ordinary quadratic field-energy defect
  `2(H_1-H_0)+beta[(U_E+U_B)_1-(U_E+U_B)_0]`;
- translation, polarity-mirror, and signed-cubic residuals of scalar metrics.

Algebraic field and current identities must close below `1e-12`. Numerical
endpoint derivatives must converge below `1e-7`. A scalar root counts as
stationary only if both endpoint kinetic residuals are below `1e-7` and its
quadratic total-energy defect is below `1e-12`. Invalid inputs must fail
closed.

## 3. Locked verdicts

- If the field identities close and all 240 scalar roots pass both particle
  stationarity and energy gates:
  `IMPLICIT_ATOMIC_ACTION_CLOSES_DIAGONAL_ENDPOINT`.
- If the field identities close but any registered scalar root fails a
  particle stationarity or energy gate:
  `ATOMIC_FACE_ACTION_CONSTRUCTIVE_SCALAR_ROOT_NOT_STATIONARY`.
- If the current split, field Euler equations, derivative convergence, or
  covariance checks fail:
  `IMPLICIT_ATOMIC_FACE_ACTION_UNRESOLVED`.

The second verdict does not reject the action. It rejects only the inference
that the independently energy-tuned FTD-0531 endpoint solves it. The next
registered task would be the actual simultaneous nonlinear solve of particle
endpoints and connection variables, with energy and reversal audited on the
resulting root rather than imposed afterward.

## 4. Execution record

Executed 2026-07-25 without changing the locked action, estimators, gates, or
verdicts. All `6/6` classification checks pass over 240 arms. The field action
and exact endpoint deposits are coherent, but none of the FTD-0531 scalar roots
passes the particle stationarity and ordinary-energy gates. Locked verdict:

```text
ATOMIC_FACE_ACTION_CONSTRUCTIVE_SCALAR_ROOT_NOT_STATIONARY
```

Canonical audit:
[`AUDIT_IMPLICIT_ATOMIC_FACE_ACTION.md`](../../07_assessment/AUDIT_IMPLICIT_ATOMIC_FACE_ACTION.md).
The SHA256 of this preregistration before this execution annotation was
`8CB27E8232FF65C43A74B2A6A24B407AA86AA969DE0042419E7B630424403CC7`.
