# FTD-0484 — Spacetime worldline gauge coupling

**Date:** 2026-07-25  
**Status:** `[THEOREM — SELECTED CUBICAL CONNECTION] + [OPEN — RECIPROCAL FIELD/MATTER DYNAMICS]`  
**Verdict:** `EXACT_SELECTED_SPACETIME_GAUGE_COUPLING`

## Result

The `FTD-0478` deposited current is exactly the line integral of the lowest
Cartesian cubical Whitney/Nedelec one-form basis. The arrays called oriented
face current in finite-volume language are primal-link current cochains paired
with their Poincare-dual faces; they are not primal face two-forms.

For a straight subcell segment, the observer now evaluates

```text
K^0_f = q integral (1-tau) W_f.dx,
K^1_f = q integral tau W_f.dx,
R_i   = q integral Lambda_i(x(tau)) d tau
```

analytically. Besides `K^0+K^1=K`, the stronger coefficient-resolved
four-dimensional continuity identities close:

```text
D K^0 + R = rho^0,
D K^1 - R = -rho^1.
```

With a selected link connection `A` and site scalar potential `Phi`,

```text
S_int/g = <A^0,K^0> + <A^1,K^1> - lambda_t<Phi,R>
```

obeys the exact endpoint identity

```text
Delta_chi S_int/g
  = <rho^1,chi^1> - <rho^0,chi^0>.
```

All 27 checks pass. The largest algebraic residual is the split-continuity
residual `5.55e-16`; the gauge endpoint residual is `2.08e-17`. Static paths,
all six axis directions, two- and three-axis diagonals, integer-plane and
periodic crossings, both polarities, integer translations, and a proper cubic
rotation are included.

## What this repairs

`FTD-0480` failed because scalar work cannot determine a transverse force
component when that displacement component is zero. The worldline action has
no such division: varying the entire path determines every endpoint component.
Thus the correct repair is an action variation, not another interpolation of
`E` or `B`.

## What it costs

The repair introduces a real auxiliary spacetime connection: spatial link
potential, temporal site-link potential, and either a second adjacent slab of
position history or an independent canonical momentum. None is determined by
the frozen `(s,J,remainder)` state. The result therefore consumes a selected
ontology extension; it is not microscopic U(1) emergence and `A` is not the
primitive site-centred `J`.

A one-slab action supplies endpoint Legendre maps but not an autonomous
position update. The gauge-invariant interior equation is the two-slab
discrete Euler--Lagrange equation

```text
D_2 L_d(x_(n-1),x_n) + D_1 L_d(x_n,x_(n+1)) = 0.
```

## Normalization obstruction exposed

Completing this interaction with the symmetric matched field action fixes the
relative magnetic term. With matched wave coefficient `c_f=C_SPEED`, exact
Gauss/source normalization and the field update force the particle curvature
term to be proportional to

```text
E + (v cross B)/c_f,
```

not the `E+v cross B` gather used by the frozen `FTD-0479` observer. Those
three normalizations cannot all be retained when `c_f != 1`. This is a new
exact normalization boundary; `FTD-0479` remains a constructive observer, not
a derived action transaction.

## Consequence

No production toggle or scenario is licensed. The next gate is an
observer-only two-slab variational derivative, including stationary transverse
electric response, affine magnetic curvature, pure-gauge null force,
threshold differentiability, and the `1/c_f` normalization test.

Run of record: `engine/results/ftd_0484/windows_msvc_cpu.json`.
