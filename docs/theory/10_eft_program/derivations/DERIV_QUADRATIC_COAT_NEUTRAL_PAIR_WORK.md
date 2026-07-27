# DERIVATION — Quadratic-coat neutral self-consistent pair work

**Identifier:** `FTD-0546`  
**Status:** `[DERIVED — EXACT NEUTRAL FIELD ALGEBRA] + [NUMERICAL FACT —
NONZERO ENERGY DEFECT] + [CLOSED NEGATIVE — FROZEN MINIMAL QUADRATIC-COAT
COMMON ACTION]`  
**Inputs:** FTD-0542 endpoint/temporal deposits, the FTD-0536 minimal field
action, FTD-0478 normalization, and the production dispersion.

## 1. Why the temporal split cannot be replaced by a midpoint source

For a neutral pair, sum both coat worldlines:

```text
K=K0+K1,
div K0+T-rho0=0,
div K1-T+rho1=0.                                  (1)
```

The scalar-potential variation of the common action sources `T`, not
`(rho0+rho1)/2`. On the mean-zero periodic sector, solve

```text
-div G Phi=T,          E_star=-G Phi.             (2)
```

In the longitudinal subfamily `A0=A1=0`, hence `B0=B1=0`. The vector-potential
endpoint equations are then

```text
E0=E_star+K0,
E1=E_star-K1.                                     (3)
```

Using (1)--(2),

```text
div E0=T+rho0-T=rho0,
div E1=T-(T-rho1)=rho1,
E1-E0=-K.                                         (4)
```

Thus the construction is neutral, Gauss-realizable, and self-consistent with
the exact endpoint source split. No projection or external harmonic field is
used.

The arithmetic endpoint field is

```text
Ebar=(E0+E1)/2=E_star+(K0-K1)/2.                  (5)
```

Consequently `E_star` is not generally the FTD-0544 work field. The difference
is an exact current-split term. Likewise `T` is not generally the endpoint
density average. These are structural distinctions, not integration errors.

## 2. Exact field-energy identity

With the FTD-0478 coefficient `beta`, ordinary field energy in this
longitudinal subfamily is `U=beta||E||^2/2`. Equations (3) and (5) give

```text
Delta U
=beta[||E_star-K1||^2-||E_star+K0||^2]/2
=-beta<Ebar,K>.                                   (6)
```

The field half of the transaction is therefore exact even after respecting
the temporal split.

## 3. Matter discriminator

Each particle uses the same slab `(A0,A1,Phi)` and the analytic FTD-0545
Legendre map. Define

```text
D_pair=sum_i [H(pi1_i)-H(pi0_i)]-beta<Ebar,K>.    (7)
```

If the fixed-step common action were an exact-energy reciprocal law, (7)
would vanish on every solution of (1)--(4). The locked 128-arm neutral campaign
instead finds

```text
max |D_pair| = 9.6808436326516136e-09,             (8)
```

while Poisson/Gauss/source algebra closes below `9.98e-14`, direct/deposited
action agreement below `8.68e-19`, exact field work below `2.90e-18`, and
gauge covariance below `6.94e-18`.

The energy defect is therefore more than four orders above the worst field
algebra residual and more than nine orders above the action/work roundoff.
The frozen minimal quadratic-coat common action is not an exact-energy mobile
law.

## 4. Boundary

This negative result leaves the exact coat current, spacetime gauge action,
endpoint Gauss transport, and field Poynting identities intact. It closes the
specific frozen-variable route that requires all of them plus exact production
matter energy from the same minimal fixed-step action.

Removing smooth self-interaction, adding a lapse, changing the kinetic action,
or selecting a discrete-gradient impulse would be a new registered dynamics.
None is implied or licensed here. Under the FTD-0479/0481 gate, no production
mobile toggle or dashboard scenario follows.
