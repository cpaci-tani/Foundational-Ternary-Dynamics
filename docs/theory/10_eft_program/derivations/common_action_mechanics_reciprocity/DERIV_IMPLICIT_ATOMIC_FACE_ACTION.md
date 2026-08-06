# DERIVATION — Minimal implicit atomic face action

**Identifier:** `FTD-0536`  
**Status:** `[DERIVED — CONDITIONAL ON SELECTED FACE-FIELD DYNAMICS]` +
`[CLOSED NEGATIVE — FTD-0531 SCALAR ROOT STATIONARITY]` +
`[CLOSED NEGATIVE BY FTD-0539 — EXACT-ENERGY/UNIQUE-INVERSION MOBILE LAW]`  
**Inputs:** FTD-0478 normalization, FTD-0484 exact spacetime worldline action,
FTD-0490 production-dispersion matter action, FTD-0535 endpoint-current split.

## 1. Coefficient derivation

Write the most economical one-slab local action using only the existing
connection endpoints and no extra route or stage variable:

```text
S_d=S_m+a||A_1-A_0||^2-b||C^T A_1||^2+g S_int^(1).
```

The selected face-field energy normalization fixes `a=beta/(2 lambda^2)`
and `b=beta/2`. Variation of the exact worldline action gives `K^(0)` at
`A_0` and `K^(1)` at `A_1`. Requiring unit current in the physical face-field
update fixes `g=beta/lambda`; no remaining coefficient is free. Therefore

```text
S_d = S_m
    + beta/(2 lambda^2)||A_1-A_0||^2
    - beta/2||C^T A_1||^2
    + (beta/lambda)S_int^(1).
```

This is a selected discrete field action because face/link flux itself remains
the selected research mainline. The coefficient relations are derived once
that field representation and the FTD-0478 normalization are adopted.

## 2. Endpoint field equations

Define

```text
E_slab=-(A_1-A_0)/lambda,  B_1=C^T A_1.
```

Then

```text
(lambda/beta) D_(A_0) S_d = E_slab+K^(0)=E_0,
-(lambda/beta)D_(A_1) S_d
  =E_slab+lambda C B_1-K^(1)=E_1.
```

Subtracting gives the exact atomic Ampere update

```text
E_1-E_0=lambda C B_1-K.
```

Thus FTD-0535's nonzero start deposit is not an obstruction once the
connection and current are solved atomically. It enters the initial canonical
relation and changes `A_1` before `B_1` is formed.

## 3. Particle equations

For each straight carrier segment the same action defines the discrete
Legendre maps

```text
P_0=-D_(x_0)S_d,  P_1=D_(x_1)S_d.
```

Subtracting the signed endpoint connection terms gives gauge-invariant kinetic
momenta. These equations determine the impulse; no `grad|J|`, Poisson force,
or separately imposed Lorentz force is available. A scalar total-energy root
need not satisfy these three-component endpoint equations. FTD-0536 therefore
tests the existing FTD-0531 roots before attempting a new nonlinear solve.

## 4. Scope

The action is local on one temporal slab but implicit: `K^(0)` and `K^(1)`
depend on the unknown endpoint while `A_1` depends on `K^(0)`. It uses the
existing face connection and ternary worldline only; it adds no primitive
variable. It does not prove exact energy conservation, reversibility, a
mobile branch, a stable dressed particle, or infrared recovery.

FTD-0536 evaluated the action on all 240 FTD-0531 diagonal scalar roots. The
field Euler equations close below `4.17e-17`, but every scalar root misses the
kinetic endpoint equations and ordinary quadratic-energy balance. Thus the
action is constructive while the old root family is closed negative as its
solution. FTD-0537--0539 complete the fresh simultaneous solve. The corner
roots are smooth but fail both energies. The edge roots solve their in-plane
equations, but the normal derivative is a converged cusp whose subgradient
interval contains zero without selecting a unique force; both edge energies
also fail. Therefore this minimal action remains a coherent selected
variational object but is closed negative as the exact reciprocal mobile law.
