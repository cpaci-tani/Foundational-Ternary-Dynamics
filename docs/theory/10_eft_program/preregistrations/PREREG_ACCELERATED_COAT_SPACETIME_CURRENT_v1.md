# PRE-REGISTRATION — Accelerated quadratic-coat spacetime current

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0548`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0541`, `FTD-0542`, `FTD-0547`  
**Scope:** observer-only reparameterization of one quadratic-coat worldline by
the exact uniform-force schedule. Primitive ternary state, production current,
force, tick, toggle, default, and scenarios remain unchanged.

## 1. Locked deposits

For the exact FTD-0547 path `x(tau)` define

```text
rho_i(tau)=q Lambda_i(x(tau)),
K       =q integral W(x(tau)).x'(tau) d tau,
K0      =q integral (1-tau)W(x(tau)).x'(tau) d tau,
K1      =q integral tau W(x(tau)).x'(tau) d tau,
T_i     =q integral Lambda_i(x(tau)) d tau.        (1)
```

Use the same tensor `B2^3` site coat and compatible `B1*B2*B2` oriented-face
basis as FTD-0541/0542. `K` must agree with the exact endpoint-only FTD-0541
line current: total spatial current is invariant under monotone temporal
reparameterization.

The split deposits must obey

```text
K0+K1=K,
div K0+T-rho0=0,
div K1-T+rho1=0.                                  (2)
```

and `sum_i T_i=q`. These are exact integration-by-parts identities for any
schedule.

## 2. Locked quadrature and gauge gate

Split the temporal interval at every half-integer face/basis knot crossed by
the spatial segment. Integrate each smooth piece with 16-point Gauss-Legendre
quadrature. Require total-current, recombination, temporal partition, and both
split-continuity residuals below `5e-12`.

For arbitrary periodic endpoint gauge functions, require

```text
<G chi0,K0>+<G chi1,K1>+<chi1-chi0,T>
=<rho1,chi1>-<rho0,chi0>                          (3)
```

below `5e-12`.

## 3. Schedule discriminator

Compare (1) with FTD-0542's uniform temporal interpolation between the same
spatial endpoints. At zero force, `K0`, `K1`, and `T` must agree below
`5e-12`. For at least one nonzero massive arm, either `T` or an endpoint
current split must differ by more than `1e-8`. Reusing the linear-schedule
deposits is then closed negative.

Under exact time reversal require

```text
K_rev=-K,
K0_rev=-K1,
K1_rev=-K0,
T_rev=T                                             (4)
```

below `5e-12`.

## 4. Locked arms and verdicts

Use `L=17`, `M=0.511`, `h=c=C_SPEED`, momenta `0.1,0.2,0.3`, field
amplitudes `0.04,0.08,0.12`, both charge signs, `beta=1` and native FTD-0478
beta, and directions `<100>,<010>,<001>,<111>`: `144` arms plus zero-force,
gauge, invalid, and reversal controls. Starts use generic non-knot positions.

- all identities close and a schedule-dependent split is measured:
  `ACCELERATED_COAT_CURRENT_EXACT_LINEAR_SPLIT_REJECTED`;
- identities close but no split changes:
  `COAT_TEMPORAL_SPLIT_PARAMETERIZATION_INVARIANT`;
- quadrature or identity gates fail:
  `ACCELERATED_COAT_SPACETIME_CURRENT_UNRESOLVED`.

A constructive result supplies the current required by the FTD-0547 escape
only in the uniform-force integrable sector. It does not yet prove a general
self-consistent matter-field action or exact-energy neutral-pair transaction.
