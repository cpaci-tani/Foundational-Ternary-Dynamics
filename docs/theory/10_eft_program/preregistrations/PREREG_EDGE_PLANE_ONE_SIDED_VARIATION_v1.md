# PRE-REGISTRATION — Edge-plane one-sided variation

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0539`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0533`, `FTD-0536`, `FTD-0538`  
**Scope:** observer-only one-sided variational audit of the unchanged FTD-0536
action on the exact shell-2 reflection plane. No production state, default,
toggle, scenario, force, collision law, phase order, field ontology,
normalization, action coefficient, smoothing, transverse perturbation, energy
definition, or tolerance changes.

## 1. Frozen edge-plane problem

Use the FTD-0537 shell-2 initial fixtures at speeds `0.125` and `0.25`.
Identify the unique inactive coordinate `a` of the edge direction and constrain
both endpoint coordinates `x_1^a` to their free integer plane values.

Solve only the four active-coordinate start Legendre equations of the same
reduced action,

```text
R_parallel(X_1)=0,
A_1(X_1)=A_0-lambda(E_0-K^(0)(X_1)),
```

using damped Newton, a centered 4x4 Jacobian at `2^-18`, partial pivoting, the
FTD-0537 backtracking sequence, at most 20 iterations, and root gate `1e-8`.
Compare active residuals evaluated with deposited-action derivative base steps
`h=2^-12` and `h/2`; require agreement below `1e-7`.

## 2. Locked one-sided normal derivatives

At the in-plane root, hold the solved connection slab fixed as required by the
field equation/envelope relation. For every carrier and both its start and end
endpoint, evaluate the interaction action normal derivative from the two
adjacent charts with the fourth-order five-sample formulas

```text
D_+ f(0)=(-25f(0)+48f(h)-36f(2h)+16f(3h)-3f(4h))/(12h),
D_- f(0)=( 25f(0)-48f(-h)+36f(-2h)-16f(-3h)+3f(-4h))/(12h).
```

Use `h=2^-12` and `h/2`; require same-side convergence below `1e-7`. Record
the left/right derivative jump and the corresponding incoming normal kinetic
residuals. No average, smoothing, or selected side enters the root equation.

Classify the incoming boundary relation carrier by carrier:

- differentiable if left/right residuals agree below `1e-7`;
- nonsmooth stationary inclusion if their closed interval contains zero;
- nonstationary if the interval excludes zero.

Also record the ordering of the two residuals; interval inclusion alone is not
called a minimum or a stable collision law.

## 3. Orbit and algebra gates

Transport both canonical edge results by signed-cubic rotations, polarity
mirror, and the three registered integer translations to all 144 shell-2 arms.
Reevaluate rather than assume:

- four active root components below `1e-8`;
- active and one-sided derivative convergence below `1e-7`;
- current split, continuity, both field equations, field update, Gauss, and
  causality below `1e-12`;
- translation, polarity, and cubic scalar covariance below `1e-7`;
- invalid inputs fail closed.

Record ordinary and staggered-modified energy defects only after the in-plane
root. Energy is not a root equation.

## 4. Locked verdicts

- Differentiable normal roots and both energy ledgers close:
  `EDGE_PLANE_DIFFERENTIABLE_STATIONARY_ENERGY_CONSTRUCTIVE`.
- Differentiable normal roots exist but either energy ledger fails:
  `EDGE_PLANE_DIFFERENTIABLE_STATIONARY_ENERGY_GATE_CLOSED_NEGATIVE`.
- Normal derivatives disagree, but every incoming residual interval contains
  zero:
  `EDGE_PLANE_NONSMOOTH_STATIONARY_REQUIRES_SUBGRADIENT_SELECTION`.
- Any converged in-plane root has a normal residual interval excluding zero:
  `EDGE_PLANE_STATIONARITY_CLOSED_NEGATIVE`.
- In-plane root, algebra, convergence, or orbit diagnostics fail:
  `EDGE_PLANE_ONE_SIDED_VARIATION_UNRESOLVED`.

The third verdict is not a constructive mobile law. A set-valued subgradient
does not satisfy FTD-0479's algebraic-inversion gate without a separately
derived selection variable or rule.
