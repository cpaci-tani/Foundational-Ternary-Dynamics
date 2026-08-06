# PRE-REGISTRATION — Quadratic-coat neutral self-consistent pair work

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0546`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0535`, `FTD-0536`, `FTD-0542`, `FTD-0544`, `FTD-0545`  
**Scope:** observer-only longitudinal two-worldline subfamily of the selected
quadratic-coat common action. No production state, force, phase, projection,
lapse, discrete-gradient repair, toggle, default, or scenario is allowed.

## 1. Locked self-consistent field construction

Use two straight quadratic-coat worldlines with charges `(+1,-1)`. Sum their
FTD-0542 deposits:

```text
K=K0+K1,
div K0+T-rho0=0,
div K1-T+rho1=0.                                  (1)
```

On a periodic neutral lattice solve the mean-zero scalar equation

```text
-div G Phi=T,
E_star=-G Phi.                                    (2)
```

Set `A0=A1=0`, so `B0=B1=0`. The endpoint field equations of the minimal
common action then give

```text
E0=E_star+K0,
E1=E_star-K1.                                     (3)
```

Equations (1)--(3) must prove, not project,

```text
div E0=rho0,
div E1=rho1,
E1-E0=-K.                                         (4)
```

The Poisson, temporal-source, split-continuity, endpoint-Gauss, and update
residuals must each be below `1e-12`. The scalar solve must reach max residual
below `1e-13` without altering the neutral source.

## 2. Locked energy discriminator

Use the FTD-0478 native normalization

```text
beta=C_WAVE^2 (G_C/C_WAVE^2)^2,
h=C_SPEED,
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2),
E_REST=0.511.                                     (5)
```

Evaluate each matter endpoint with the analytic FTD-0545 Legendre map on the
same slab `(A0,A1,Phi)`. Define

```text
Ebar=(E0+E1)/2,
W=beta<Ebar,K>,
Delta U_field=beta[||E1||^2-||E0||^2]/2,
D_pair=sum_i[H(pi1_i)-H(pi0_i)]-W.                (6)
```

Require the exact field identity

```text
Delta U_field+W=0                                 (7)
```

below `1e-12`. Direct and deposited interaction actions must agree below
`1e-12`; gauge-transformed kinetic endpoints and `D_pair` must agree below
`1e-10`.

Also record

```text
Ebar-E_star=(K0-K1)/2                             (8)
```

and the difference between `T` and `(rho0+rho1)/2`. Neither difference may be
silently discarded or renamed numerical error.

## 3. Locked arms

Use `L=17,19`; pair separations `3,4`; inward displacements `0.02,0.05` and
their outward reversals; both charge orderings; and directions
`<100>,<010>,<001>,<111>`. The pair center and every endpoint use generic
non-knot fractional coordinates. This gives `128` registered arms.

Require both charge orders and all directions to remain valid. A zero-motion
control must have zero current, field-energy change, matter-energy change, and
pair defect below `1e-12`. Invalid/non-neutral inputs fail closed.

## 4. Locked verdicts

- all algebraic and gauge gates pass, with at least one
  `|D_pair|>1e-10`:
  `SELF_CONSISTENT_COAT_PAIR_ENERGY_CLOSES_NEGATIVE`;
- all algebraic and gauge gates pass and every `|D_pair|<=1e-12`:
  `SELF_CONSISTENT_COAT_PAIR_ENERGY_CONSTRUCTIVE`;
- only scalar-solver convergence fails:
  `SELF_CONSISTENT_COAT_PAIR_POISSON_UNRESOLVED`;
- any action/source/field identity fails:
  `SELF_CONSISTENT_COAT_PAIR_ALGEBRA_INVALID`.

A negative result closes the frozen quadratic-coat minimal common action as an
exact-energy reciprocal mobile law. It does not retract the coat current,
gauge action, Gauss transport, or field Poynting theorems, and it does not
license a lapse or selected non-variational repair.
