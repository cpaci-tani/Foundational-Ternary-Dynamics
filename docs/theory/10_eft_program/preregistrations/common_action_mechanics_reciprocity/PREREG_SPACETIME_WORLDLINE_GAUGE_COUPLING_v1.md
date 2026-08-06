# PRE-REGISTRATION — Spacetime worldline gauge coupling v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0484`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0478` exact subcell current and the `FTD-0480`
frozen-E/B force-gather closure

## Question

Does the exact `FTD-0478` straight-segment current arise as the spatial part
of a local spacetime worldline coupling to a cubical link connection, with an
exact gauge endpoint identity and no coordinate-route choice?

This campaign is observer-only. It does not reopen `FTD-0480`, license
`common_action_face_dynamics`, alter production state, or identify the
auxiliary connection with the primitive site-centred flux field.

## Frozen discrete complex

For periodic sites `i`, use the production-side matched conventions

```text
(G chi)^a_i = chi_(i+e_a) - chi_i,
(D K)_i = sum_a (K^a_i - K^a_(i-e_a)),
G = -D^T.
```

The stored positive-face current is the Poincare-dual representation of a
primal link cochain. Its exact interpolating form is the cubical Whitney /
Nedelec one-form

```text
W^x_(i,j,k) = 1_[i,i+1](x) lambda_j(y) lambda_k(z) dx,
```

with cyclic expressions for `y,z` and `lambda_i(x)=max(1-|x-i|,0)`.
The already frozen current is

```text
K_f = q integral_gamma W_f.
```

No alternative path decomposition is admitted.

## Frozen spacetime deposits

For the straight path `x(tau)=x_0+tau Delta x`, `0<=tau<=1`, define

```text
K^0_f = q integral_gamma (1-tau) W_f,
K^1_f = q integral_gamma tau W_f,
R_i   = q integral_0^1 Lambda_i(x(tau)) d tau.
```

The deposits must be evaluated analytically after splitting only at crossed
integer coordinate planes. Required algebraic identities are

```text
K^0 + K^1 = K,
sum_i R_i = q.
```

## Frozen auxiliary connection and coupling

Place `A^0,A^1` on the same primal links / stored positive-face arrays as the
current, and place `Phi` on sites for the time slab. For
`lambda_t=C_SPEED*Delta t`, define

```text
S_int / g = <A^0,K^0> + <A^1,K^1> - lambda_t <Phi,R>.
```

Under

```text
A^n -> A^n + G chi^n,
Phi -> Phi - (chi^1-chi^0)/lambda_t,
```

the exact locked identity is

```text
Delta S_int / g
  = <rho^1,chi^1> - <rho^0,chi^0>.
```

The derived field representatives are

```text
E = -(A^1-A^0)/lambda_t - G Phi,
B = C^T A,
```

where `C^T` is the existing `matched_curl_adjoint`; therefore `E` and `B`
must be invariant and `C^T G chi=0` to roundoff.

## Locked arms and gates

Use `L=17` and cover:

- both polarities;
- static paths and all six axis directions;
- two- and three-axis diagonal paths;
- an integer-plane crossing and a periodic-boundary crossing;
- integer translations and proper cubic rotations;
- deterministic affine and trigonometric `A`, `Phi`, and `chi` fixtures.

Every valid arm must satisfy, at `1e-12`:

1. `K^0+K^1=K` componentwise;
2. temporal-charge partition `sum R=q`;
3. the full spacetime gauge endpoint identity;
4. `C^T G chi=0`;
5. gauge invariance of `E` and `B`;
6. locality of every deposited cochain;
7. integer-translation and proper-cubic covariance.

Invalid volume, charge, time scale, or non-finite field input must be rejected.

## Frozen interpretation and consequence

Passing proves only that the `FTD-0478` current is the exact spatial de Rham
image of a straight worldline and admits a selected local gauge-connection
completion. It does not prove microscopic U(1), identify `A` with primitive
`J`, or close reciprocal dynamics.

A one-slab action may define endpoint Legendre maps, but a gauge-invariant
physical update must use either the two-slab discrete Euler--Lagrange equation
or an explicitly added canonical momentum state. If the identity passes,
that unavoidable state/history requirement is recorded rather than hidden.
If any algebraic gate fails, this selected connection completion closes
negative without tolerance changes or an interpolated force repair.

Run-of-record test-source SHA256:
`3EF828E7B13CCEA91FE2EA459E1E3AD8C0FEA36852A28B00EFE88FF83D36D555`.
