# DERIVATION — Quadratic-coat discrete-gradient transaction

**Identifier:** `FTD-0551`  
**Status:** `[SELECTED DYNAMICS] + [THEOREM — CONDITIONAL EXACT ENERGY/GAUSS IDENTITIES] + [NUMERICAL FACT — NONLINEAR ROOT CAMPAIGN]`  
**Scope:** observer-only; not the production tick and not a claim of unique
action-derived dynamics.

## 1. Atomic endpoint map

For the production matter Hamiltonian

```text
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2),              (1)
```

define the vector discrete gradient

```text
vbar=C_SPEED^2(p0+p1)/(H0+H1).                    (2)
```

Since `(H1-H0)(H1+H0)=C_SPEED^2(|p1|^2-|p0|^2)`, it obeys

```text
H1-H0=vbar dot (p1-p0).                           (3)
```

Set `x1=x0+h vbar`, deposit the exact FTD-0541 current `K`, and advance the
matched field by

```text
B'=B-lambda C^T E0,
E*=E0+lambda C B',
E1=E*-K.                                          (4)
```

The endpoint momentum is the root of

```text
p1-p0=g h q(Ebar_orbit+vbar cross B'_orbit),      (5)
```

where both orbit gathers are the FTD-0550 adjoint reconstructions and
`Ebar=(E0+E1)/2`. Position, momentum, current, and fields are therefore one
implicit transaction rather than a move followed by inferred recoil.

## 2. Exact work and energy

The electric adjoint identity and `x1-x0=h vbar` give

```text
g h q vbar dot Ebar_orbit=g <Ebar,K>.              (6)
```

The magnetic term is perpendicular to `vbar`. Taking the scalar product of
(5) with `vbar` and using (3) yields

```text
H1-H0=g <Ebar,K>.                                 (7)
```

FTD-0544 proves for (4) that the scaled modified field energy satisfies

```text
U1-U0=-g <Ebar,K>.                                (8)
```

Equations (7) and (8) prove exact total-energy exchange conditional on a
converged root of (5). Taking divergence of (4), using `div C=0`, and using
the exact coat continuity equation propagates Gauss without projection.

## 3. Inversion and status

Given the accepted segment, reverse (4) in the order `+K`, `-lambda C B'`,
`+lambda C^T E0`, and subtract the recorded total impulse. This is an exact
algebraic inverse of the accepted transaction to floating-point residual.

The discrete-gradient map is selected because FTD-0543 already proves that
exact fixed-step energy does not follow automatically from configuration
stationarity. FTD-0549 also prevents this endpoint transaction from being
relabelled as the complete schedule-resolved spacetime action. It is a valid
reciprocal integrator candidate, not a derived unique microscopic law.

## 4. Boundary

One-step closure does not establish stable static dressing, absence of
self-force, ballistic multi-tick transport, packet-caused hopping, history-free
reversal, or a physical pole. Those are the next gates before a default-off
mobile branch can be installed.
