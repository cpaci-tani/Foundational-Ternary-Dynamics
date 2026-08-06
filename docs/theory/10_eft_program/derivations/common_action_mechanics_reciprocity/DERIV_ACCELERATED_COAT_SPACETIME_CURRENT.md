# DERIVATION — Accelerated quadratic-coat spacetime current

**Identifier:** `FTD-0548`  
**Status:** `[DERIVED — EXACT DEPOSIT IDENTITIES] + [NUMERICAL FACT — LOCKED
QUADRATURE]`  
**Inputs:** FTD-0541 quadratic-coat face current, FTD-0542 spacetime split,
and the FTD-0547 exact constant-force schedule.

## 1. Schedule-dependent deposits

For the accelerated path `x(tau)`, define

```text
rho_i(tau)=q Lambda_i(x(tau)),
K =q integral W(x(tau)).x'(tau) d tau,
K0=q integral (1-tau)W(x(tau)).x'(tau) d tau,
K1=q integral tau W(x(tau)).x'(tau) d tau,
T_i=q integral Lambda_i(x(tau)) d tau.            (1)
```

The compatible coat bases obey `div[W.x']=-d Lambda/dtau`. Therefore
integration by parts gives

```text
K0+K1=K,
div K0+T-rho0=0,
div K1-T+rho1=0,
sum_i T_i=q.                                      (2)
```

These identities do not assume uniform temporal motion.

## 2. What is and is not parameterization invariant

The total current `K` is the line integral of the face one-form and depends
only on the oriented spatial path. Any monotone reparameterization of the
same segment leaves it unchanged. The factors `tau` and `1-tau`, and the
temporal occupation `T`, do depend on the within-tick schedule. Consequently
the endpoint current split is physical data of the discrete spacetime action,
not reconstructible from the spatial endpoints alone.

For endpoint gauge functions `chi0,chi1`, (2) implies

```text
<G chi0,K0>+<G chi1,K1>+<chi1-chi0,T>
=<rho1,chi1>-<rho0,chi0>.                         (3)
```

Under exact reversal, `K` changes sign, `K0` and `K1` exchange with a minus
sign, and `T` is invariant.

## 3. Registered observer result

Sixteen-point Gauss-Legendre integration split at every coat knot was tested
on 144 locked arms. Total-current, recombination, partition, continuity,
gauge, and reversal residuals all remain below `2.19e-14`, against the
`5e-12` gate. The accelerated split differs from the endpoint-linear split by
as much as `0.0016542884780729739`, while the zero-force limit agrees below
the gate.

The exact accelerated current therefore exists, but FTD-0542's linear-time
`K0,K1,T` cannot be reused for accelerating matter.

## 4. Boundary

The construction inherits FTD-0547's constant-force schedule. It supplies
the correct gauge-covariant current for that schedule only. A spatially
varying self-consistent field must determine its own within-tick force history
and schedule before these deposits can be evaluated. No production mobile
law follows from this observer result. The observer fails closed if momentum
changes sign within the tick: the registered proof covers a monotone segment,
whereas a turning path requires a separately split multi-leg quadrature.
