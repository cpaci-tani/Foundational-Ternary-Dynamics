# PRE-REGISTRATION — Implicit atomic endpoint solve

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0537`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0490`, `FTD-0533`, `FTD-0536`  
**Scope:** observer-only nonlinear solve of the FTD-0536 action. No production
state, default, toggle, scenario, force, collision law, phase order, field
ontology, normalization, or tolerance changes.

## 1. Initial-value problem

Use the FTD-0531 registered diagonal fixture only as initial data:

- its two overshoot-preserving carrier start positions;
- incoming kinetic momenta `p_0 u_i` from the production dispersion;
- `E_0=K_ref/2+(1/8)C C^T K_ref`;
- gauge representative `A_0=lambda E_0`.

Do not use the FTD-0531 outgoing momentum or endpoint as an equation. For six
unknown endpoint coordinates `X_1=(x_1^(a),x_1^(b))`, rebuild the exact
FTD-0484 deposits and eliminate the connection endpoint through

```text
A_1(X_1)=A_0-lambda(E_0-K^(0)(X_1)).
```

The nonlinear residual is the pair of gauge-invariant start Legendre maps:

```text
R(X_1)=P_kin,0(X_1)-P_kin,input.
```

This is the initial-value equation of the action. No total-energy equation,
outgoing-direction constraint, force projection, or endpoint symmetry is
added.

## 2. Locked solver

Use damped Newton in the six endpoint coordinates:

- initial endpoint: the free `speed` displacement along each incoming unit
  direction;
- action endpoint derivatives: complete deposited five-point differences at
  `h=2^-12` and `h/2`;
- residual Jacobian: centered differences at `2^-6 h`;
- partial-pivot 6x6 linear solve;
- backtracking factors `1,1/2,...,1/512`, accepting only a strict residual
  decrease and causal segments;
- at most 20 Newton iterations;
- convergence gate `||R||_inf <= 1e-8`.

First solve the four canonical arms `(shell,speed)` in
`{2,3} x {0.125,0.25}` at positive polarity and zero translation. If all four
converge, transport their two displacement vectors by signed cubic rotations,
polarity mirror, and integer translation to all 240 registered arms and
reevaluate the full equations rather than assuming covariance.

## 3. Acceptance gates

1. Every canonical solve and every transported arm must have start Legendre
   residual below `1e-8` and endpoint derivative convergence below `1e-7`.
2. Current split, continuity, both field endpoint equations, atomic field
   update, and Gauss evolution stay below `1e-12`.
3. Every segment remains strictly causal and the Newton Jacobian remains
   algebraically invertible at each accepted iteration.
4. Translation, polarity mirror, and signed-cubic scalar metrics agree below
   `1e-7`.
5. Record, but do not fit, both ordinary quadratic and exact staggered-modified
   total-energy defects. Energy is a post-root gate, never a root equation.
6. Invalid inputs fail closed.

Exact reversal is deferred to a separate locked campaign only if a stationary
root exists.

## 4. Locked verdicts

- If all stationary roots exist and both energy defects close below `1e-12`:
  `IMPLICIT_ATOMIC_STATIONARY_ROOT_ENERGY_CONSTRUCTIVE`.
- If all stationary roots exist but either registered energy defect is nonzero:
  `IMPLICIT_ATOMIC_STATIONARY_ROOT_ENERGY_GATE_CLOSED_NEGATIVE`.
- If any canonical root or its transported covariance orbit fails:
  `IMPLICIT_ATOMIC_STATIONARY_ROOT_NOT_CONSTRUCTED`.
- If algebraic/derivative diagnostics fail:
  `IMPLICIT_ATOMIC_ENDPOINT_SOLVE_UNRESOLVED`.

The second verdict closes this FTD-0536 action as the exact-energy transaction
requested by FTD-0479, while retaining it as a coherent variational selected
dynamics. It does not authorize energy projection or a new fitted term.
