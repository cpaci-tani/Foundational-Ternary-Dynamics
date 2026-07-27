# PRE-REGISTRATION — Discrete Legendre worldline v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0490`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0485`, `FTD-0489`

## Question

Does the correct classical endpoint equation—the discrete Legendre transform
of the full matter-plus-worldline action—remain gauge covariant and locally
invert the production dispersion in a cubical-cell interior, despite
FTD-0489's proof that bare open action values cannot be ordered across
different endpoints?

## Frozen action

For one time slab with `lambda=c dt` and straight displacement
`d=x_1-x_0`, freeze the matter principal action implied by the existing
production dispersion:

```text
S_m(d) = -(E_REST lambda/c) sqrt(1-|d|^2/lambda^2),
|d| < lambda.
```

Its endpoint derivative is

```text
p = D_2 S_m = E_REST d/(c lambda sqrt(1-|d|^2/lambda^2)),
E(p)^2 = E_REST^2+c^2|p|^2,
d = lambda c p/E(p).
```

Add exactly the FTD-0484 interaction action, with no new force gather:

```text
S_d(x_0,x_1)=S_m(x_1-x_0)+S_int(x_0,x_1).
```

The discrete endpoint momenta are

```text
P_0=-D_1 S_d,
P_1= D_2 S_d.
```

Under `S_int' = S_int+gq(chi_1(x_1)-chi_0(x_0))`, they must obey

```text
P_0'=P_0+gq grad chi_0(x_0),
P_1'=P_1+gq grad chi_1(x_1).
```

The cubical Q1/Nedelec commuting identity must simultaneously give

```text
A_n'(x_n)=A_n(x_n)+grad chi_n(x_n),
Pi_n=P_n-gq A_n(x_n),
Pi_n'=Pi_n.
```

Thus the correct endpoint residual is gauge covariant even though the action
value is not gauge invariant.

## Locked scope and tests

This first pass is restricted to a straight segment strictly inside one
spatial cell. Use `L=17`, `lambda=C_SPEED`, `E_REST=0.511`, `c=C_SPEED`,
both polarities, fixed analytic fields, and tolerance `1e-12`.

1. Automatic differentiation of `S_int` must reproduce the exact FTD-0484
   deposited action below `1e-12`.
2. The free analytic inverse `d=lambda c p/E(p)` must return the supplied
   momentum and close the production dispersion below `1e-12` for axial and
   diagonal momenta.
3. General gauge transformations must shift both canonical endpoint momenta
   exactly as the interpolated connection shifts, leaving `Pi_0,Pi_1`
   invariant below `1e-12`.
4. A nonzero pure gauge must reproduce the free kinetic endpoint momenta.
5. Polarity reversal, integer translation, and proper cubic rotation must
   close below `1e-12`.
6. Noncausal `|d|>=lambda`, cross-cell, zero-charge, nonfinite, and mismatched
   inputs must fail closed.

## Frozen interpretation

A pass proves that FTD-0489 does **not** forbid a classical common-action
update in cell interiors: canonical endpoint matching, not open-action
minimization, is the gauge-covariant equation. It does not prove a unique
endpoint, threshold crossing, coupled field update, or production dynamics.

The knot/cell-interface branch exposed by FTD-0487 remains a separate required
test. No production toggle or scenario follows from an interior pass.

Run-of-record test-source SHA256:
`057C63E6794F5AD274A3027BDE3F126223B7F9E6C6EB79A488816EA0D22936F5`.
