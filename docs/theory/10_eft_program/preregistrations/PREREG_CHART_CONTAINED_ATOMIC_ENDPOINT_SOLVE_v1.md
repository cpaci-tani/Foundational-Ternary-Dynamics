# PRE-REGISTRATION — Chart-contained atomic endpoint solve

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0538`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0533`, `FTD-0536`, `FTD-0537`  
**Scope:** observer-only re-evaluation of the unchanged FTD-0536 action with
endpoint derivative stencils contained in one current chart. No production
state, default, toggle, scenario, force, collision law, phase order, field
ontology, normalization, action coefficient, initial datum, root equation,
energy definition, or tolerance changes.

## 1. Frozen problem

Use exactly the FTD-0537 initial fixture and eliminate the connection endpoint
by

```text
A_1(X_1)=A_0-lambda(E_0-K^(0)(X_1)).
```

Solve the same six start-Legendre equations

```text
R(X_1)=P_kin,0(X_1)-P_kin,input=0.
```

Energy remains a post-root test and is not included in `R`.

## 2. Chart-contained derivative

For each varied endpoint coordinate `x`, define its clearance from the nearest
integer endpoint face by

```text
d(x)=|x-round(x)|.
```

Let `h_0=2^-12`. The coordinate-specific coarse five-point step is

```text
h_c(x)=min(h_0,d(x)/4).
```

The coarse samples extend through `x+-2h_c`, hence stay at least `d/2` from the
nearest endpoint face. Compare that derivative with the same five-point rule
at `h_c/2`. If `d<=2^-30`, the derivative is unresolved rather than sampled
one-sidedly. Record the minimum clearance, minimum selected step, and the
coarse/fine maximum disagreement.

This rule changes no action value. It only prevents the numerical derivative
from comparing two different endpoint charts. Internal segment knots continue
to use the complete FTD-0484 deposited current; FTD-0533 already established
their global variation.

## 3. Locked nonlinear solve

Use the FTD-0537 damped Newton solver unchanged except that every action
endpoint derivative uses the chart-contained rule above:

- free endpoint as initial guess;
- centered 6x6 residual Jacobian at `2^-18`;
- partial pivoting and the same strict-decrease backtracking sequence;
- at most 20 iterations;
- root gate `||R||_inf<=1e-8`;
- derivative-convergence gate `1e-7`;
- algebraic gates `1e-12`.

Solve the four canonical `(shell,speed)` arms in
`{2,3} x {0.125,0.25}` and transport/re-evaluate all 240 signed-cubic,
polarity, and integer-translation arms. Compare each chart-contained endpoint
with the FTD-0537 fine-stencil endpoint and record the maximum displacement.

## 4. Acceptance and energy gates

1. All canonical and transported roots pass the root and chart-contained
   derivative gates.
2. Current split, continuity, both field endpoint equations, field update,
   Gauss evolution, causality, and covariance pass the unchanged FTD-0537
   gates.
3. The action value at a shared endpoint agrees with the ordinary FTD-0537
   evaluator below `1e-12`.
4. Record both the ordinary quadratic and staggered-modified total-energy
   defects without fitting or projection.
5. Invalid inputs and endpoint coordinates with insufficient chart clearance
   fail closed.

## 5. Locked verdicts

- Roots, derivative gates, and both energy gates close below `1e-12`:
  `CHART_CONTAINED_STATIONARY_ROOT_ENERGY_CONSTRUCTIVE`.
- Roots and derivative gates close, but either energy remains nonzero:
  `CHART_CONTAINED_STATIONARY_ROOT_ENERGY_GATE_CLOSED_NEGATIVE`.
- Any canonical root or transported covariance arm fails despite valid
  chart-contained derivatives:
  `CHART_CONTAINED_STATIONARY_ROOT_NOT_CONSTRUCTED`.
- Algebra, chart clearance, or derivative diagnostics fail:
  `CHART_CONTAINED_ENDPOINT_SOLVE_UNRESOLVED`.

The second verdict closes the unchanged FTD-0536 action as the exact-energy
transaction requested by FTD-0479. It does not close the face-current
representation or forbid a separately preregistered action with a derived
conserved energy. Reversal is run only after the constructive energy verdict.
