# DERIVATION — Quadratic-coat spacetime action

**Identifier:** `FTD-0542`  
**Status:** `[SELECTION — SMOOTH NON-CARDINAL COUPLING COAT]` +
`[THEOREM — EXACT SPACETIME CONTINUITY AND GAUGE-ENDPOINT IDENTITY]`  
**Inputs:** the FTD-0541 `B2/B1` coat-current pair and the selected matched
face/edge auxiliary gauge complex. Primitive ternary manifestation is
unchanged.

## 1. The endpoint split is forced by the worldline

Let

```text
rho_i(t)=q product_d B2(x_d(t)-i_d),
div j(t)=-d rho(t)/dt,
x(t)=x0+t Delta x.
```

Multiplying continuity by the two temporal hat functions gives

```text
d[(1-t)rho]/dt=-rho-(1-t)div j,
d[t rho]/dt= rho-t div j.
```

Integration from zero to one therefore proves

```text
div K0=rho0-T,
div K1=T-rho1,                                     (1)
```

where

```text
K0=integral (1-t)j dt,
K1=integral t j dt,
T =integral rho dt.
```

Adding (1) recovers the FTD-0541 continuity equation and gives `K0+K1=K`.
Partition of the coat at every `t` gives `sum_i T_i=q`. Thus the temporal
deposit is derived rather than inserted.

## 2. Polynomial-exact evaluation

On a half-integer knot interval, the FTD-0541 spatial integrand is degree at
most five. The factors `t` and `1-t` raise that to six. The temporal tensor
coat is also degree six. Four-point Gauss-Legendre quadrature is exact through
degree seven, so the implementation evaluates the piecewise-polynomial
integrals without a tuned convergence order.

## 3. The common interaction

For the one-slab connection `(A0,A1,Phi)` and duration `h`, define

```text
S_int=g[<A0,K0>+<A1,K1>-h<Phi,T>].                 (2)
```

Linearity immediately gives

```text
D_A0 S_int=g K0,
D_A1 S_int=g K1,
D_Phi S_int=-g h T.                                (3)
```

Current deposition and field sourcing are therefore derivatives of one
functional. This is the piece that FTD-0479 lacked.

## 4. Exact open-worldline gauge identity

Under

```text
A0 -> A0+G chi0,
A1 -> A1+G chi1,
Phi -> Phi-(chi1-chi0)/h,
```

periodic summation by parts gives `<G chi,K>=-<chi,div K>`. Substituting (1)
into the change of (2), all interior `T` terms cancel:

```text
Delta S_int
=g[-<chi0,rho0-T>-<chi1,T-rho1>
    +<chi1-chi0,T>]
=g[<rho1,chi1>-<rho0,chi0>].                       (4)
```

Equation (4) is the correct covariance of an open charged worldline. A closed
worldline or a product with endpoint matter phases is invariant. The matched
electric field and both magnetic endpoint fields are separately invariant.

## 5. What this repairs—and what it does not

FTD-0542 repairs the missing common origin of the smooth-coat current: the
same functional now supplies both endpoint face sources and the eventual
endpoint impulse through variations of the worldline. It does not reuse or
reopen the closed trilinear action of FTD-0536.

No particle endpoint equation has yet been solved. Fixed duration supplies no
time-node variation, so (2) alone does not prove the required endpoint
matter-plus-field energy ledger. FTD-0543 isolates that logical gap. No force,
toggle, scenario, production update, pole, Lorentz, photon, or particle claim
follows.
