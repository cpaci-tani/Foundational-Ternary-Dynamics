# PRE-REGISTRATION — Knot Legendre branch v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0491`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0487`, `FTD-0490`

## Question

Given one gauge-invariant initial kinetic momentum at a manifested lattice
knot, does the FTD-0490 discrete Legendre equation select a unique incident
cubical-cell endpoint, or can multiple gauge-covariant outgoing histories
solve the same equation?

## Locked exact fixture

At a central site `i`, define a locally reflection-symmetric matched electric
field by

```text
E_a(i)       = +q/6,
E_a(i-e_a)   = -q/6,
```

extended uniformly across the two adjacent coordinate slabs for each axis.
Then

```text
D E(i)=sum_a [E_a(i)-E_a(i-e_a)]=q.
```

The eight incident cells carry constant electric vectors

```text
E_sigma = q (sigma_x,sigma_y,sigma_z)/6,
sigma_a in {-1,+1}.
```

Represent the field on one time slab by `A_0=0`, `A_1=-lambda E`, `Phi=0`.
For a straight history inside one incident cell the exact interaction is

```text
S_int = -(g q lambda/2) E_sigma dot d.
```

The zero-input initial Legendre equation is therefore

```text
P_0 = p(d) - (g q lambda/2) E_sigma = 0.
```

It has the analytic solution

```text
p_sigma = (g q lambda/2) E_sigma
        = (g lambda/12) sigma,
d_sigma = lambda c p_sigma
          /sqrt(E_REST^2+c^2|p_sigma|^2).
```

Every `d_sigma` points into its own incident cell. Thus eight distinct
outgoing endpoints solve one identical physical input if the exact evaluator
confirms the limit.

## Locked tests

Use `L=17`, central knot `(8,8,8)`, `lambda=c=C_SPEED`,
`E_REST=0.511`, `g=0.73`, both polarities, and tolerance `1e-12`.

1. Verify `D E(i)=q` exactly and the divergence-free status of any added
   uniform bias.
2. Approach the knot from each incident cell with
   `epsilon in {1e-4,1e-6,1e-8}` and evaluate the analytic endpoint.
3. Require all eight symmetric branches to have initial kinetic/canonical
   residual below `1e-12`, equal displacement magnitude, and exact polarity
   mirror/cubic orbit behavior.
4. Apply a nonzero arbitrary gauge. Require every branch endpoint and kinetic
   residual to remain unchanged below `1e-12`.
5. Add the frozen uniform external bias `E_bias=(0.4,0.5,0.6)`. Require exactly
   one sign-consistent incident-cell branch for each polarity. This is a
   control showing that generic physical asymmetry can select a branch; it
   does not repair the symmetric allowed state.

## Frozen verdicts

- `UNIQUE_KNOT_LEGENDRE_BRANCH` only if exactly one symmetric branch solves
  the zero-input equation for both polarities.
- `SYMMETRIC_KNOT_HAS_EIGHT_LEGENDRE_BRANCHES` if all eight exact branches
  solve it and remain gauge invariant.
- `IMPLEMENTATION_INVALID` if Gauss, analytic momentum, gauge, polarity, or
  cubic controls miss `1e-12`.

## Failure consequence

Eight branches close unconditional algebraic inversion for the frozen knot
variables. The field and initial kinetic momentum do not contain a branch bit.
A centered weak derivative, rest-preserving rule, random choice, prior-history
direction, smoother shape, or quantum amplitude sum is a new explicit
selection/ontology cycle. No such rule may be inferred from the existence of
the eight solutions.

No production toggle or scenario is authorized.

Run-of-record test-source SHA256:
`5662BA3AA3308B447ED9E6C2BC78C1D382599391EBDFAA1F00C9D3302F95ACAE`.
