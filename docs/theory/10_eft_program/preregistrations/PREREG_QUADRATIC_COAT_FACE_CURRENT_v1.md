# PRE-REGISTRATION — Quadratic coupling coat and exact face current

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0541`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0484`, `FTD-0539`, `FTD-0540`  
**Scope:** observer-only construction of the smooth, nonnegative,
non-cardinal escape branch priced by FTD-0540. Primitive manifestation remains
one ternary site plus its existing remainder. No production state, default,
toggle, scenario, force, collision law, phase order, field ontology,
normalization, action coefficient, energy definition, or tolerance changes.

## 1. Locked coat

Use the centered quadratic B-spline `B2` and no fitted shape parameter:

```text
B2(u)=3/4-u^2,                    |u|<=1/2,
      (1/2)(3/2-|u|)^2,          1/2<|u|<3/2,
      0,                          otherwise.
```

For effective position `x`, define the signed coupling coat

```text
rho_i(x)=q product_d B2(x_d-i_d),       q in {-1,+1}.
```

Require compact support at no more than 27 sites, nonnegative unsigned
weights, partition of unity, exact first-moment reproduction, integer-
translation covariance, polarity mirror, cubic covariance, and `C1`
coordinate regularity. At an integer position the coat must retain the exact
non-cardinal weights implied by FTD-0540; no re-normalization is allowed.

## 2. Locked oriented-face current

Let `B1(u)=max(1-|u|,0)` and use the exact derivative identity

```text
d/dx B2(x-i)=B1(x-(i-1/2))-B1(x-(i+1/2)).
```

For the straight one-tick segment `x(t)=x0+t Delta x`, deposit on the positive
`d` face at `i_d+1/2`

```text
K^d_(i+1/2)=q Delta x_d integral_0^1
  B1(x_d(t)-(i_d+1/2))
  product_(e!=d) B2(x_e(t)-i_e) dt.                 (1)
```

Equation (1) is the only current. No axis route, current repair, Gauss
projection, force, or endpoint source is permitted. Split the straight path
at every crossed half-integer coordinate plane. On each open piece the
integrand has degree at most five, so three-point Gauss-Legendre quadrature is
polynomial-exact. This quadrature implements an analytic piecewise integral;
it is not an adjustable numerical approximation.

Prove and test

```text
rho_after-rho_before+div K=0,
sum_faces_d K^d=q Delta x_d.                         (2)
```

## 3. Locked orbit and regularity gates

Test both polarities, axial/edge/corner displacements, stationary paths,
translated copies, cubic permutations/inversions, threshold and half-integer
crossings, and a periodic-boundary path on `L=17`. Require:

- partition, first moment, continuity, integrated-current moment, locality,
  translation, polarity, cubic, and reversal residuals below `1e-12`;
- causal component displacement no greater than one cell;
- invalid/nonfinite/over-causal inputs fail closed.

For a fixed nontrivial face connection, evaluate the deposited worldline
coupling across the FTD-0539 inactive-coordinate integer reflection plane.
Use the same fourth-order left/right derivative formulas at `h=2^-12` and
`h/2`. Require same-side convergence and the left/right derivative jump below
`1e-7`. The connection remains fixed during endpoint variation.

## 4. Locked verdicts

- all coat, current, orbit, reversal, and `C1` gates close:
  `QUADRATIC_COAT_EXACT_CURRENT_C1_CONSTRUCTIVE`;
- continuity or current-moment identity fails:
  `QUADRATIC_COAT_CURRENT_CLOSED_NEGATIVE`;
- exact current closes but the reflection-plane derivative remains nonsmooth:
  `QUADRATIC_COAT_C1_ESCAPE_CLOSED_NEGATIVE`;
- only floating-point convergence or orbit gates fail:
  `QUADRATIC_COAT_FACE_CURRENT_UNRESOLVED`.

A constructive result establishes a new selected coupling sidecar only. It
does not reopen FTD-0536, prove exact energy, select a mechanical impulse, or
license `common_action_face_dynamics`.
