# THEOREM — Endpoint schedule underdetermination

**Identifier:** `FTD-0549`  
**Status:** `[THEOREM — ENDPOINT/MIDPOINT INSUFFICIENCY]`  
**Inputs:** exact quadratic-coat partition and first-moment reproduction from
FTD-0541, and the schedule-dependent spacetime deposits of FTD-0542/0548.

## Theorem

Endpoint position, endpoint velocity, midpoint velocity, duration, total
displacement, endpoint kinetic energies, and total oriented face current do
not determine the exact endpoint-weighted currents `K0,K1` and temporal coat
`T`.

## Proof

Consider the same oriented segment

```text
x(tau)=x0+d n s(tau),       0<=tau<=1,
```

and the schedules

```text
s0(tau)=tau,
se(tau)=tau+epsilon f(tau),
f(tau)=tau^2(1-tau)^2.                            (1)
```

Because `f(0)=f(1)=f'(0)=f'(1)=f'(1/2)=0`, the two paths have identical
positions and velocities at both endpoints and identical midpoint velocity.
Writing `u=1-2tau`,

```text
f'(tau)=u(1-u^2)/2,
max |f'|=1/(3 sqrt(3)).                           (2)
```

Hence `se` is strictly monotone for `|epsilon|<3 sqrt(3)`, in particular for
the registered `|epsilon|<=1/2`.

The total face current is the oriented spatial line integral

```text
K=q integral W(x) dx,                             (3)
```

so it is identical for both parameterizations. However,

```text
integral_0^1 f(tau) d tau=Beta(3,3)=1/30.         (4)
```

The quadratic coat reproduces the first moment, so the first spatial moment
of its temporal occupation is

```text
mu_T=q integral x(tau) d tau.
```

Equation (4) therefore gives

```text
Delta mu_T=q d n epsilon/30.                      (5)
```

Partition of the compatible face basis gives

```text
sum K0=q d n integral (1-tau)s'(tau)d tau
      =q d n integral s(tau)d tau,

sum K1=q d n integral tau s'(tau)d tau
      =q d n[1-integral s(tau)d tau].             (6)
```

Thus

```text
Delta sum K0= q d n epsilon/30,
Delta sum K1=-q d n epsilon/30,
Delta sum K =0.                                   (7)
```

For any nonzero `q,d,epsilon`, the exact spacetime deposits differ despite all
listed endpoint and midpoint data agreeing. This proves the claim.

Under time reversal, `1-se(1-tau)=s_{-epsilon}(tau)`, so the witness family is
closed under reversal and the endpoint-current differences exchange with the
required signs.

## Consequence and boundary

An endpoint-only or endpoint-plus-midpoint transaction cannot reconstruct the
gauge-required spacetime source. A viable common-action step must solve or
otherwise carry enough internal stages to determine the within-tick history.

This is not a no-go theorem for self-consistent dynamics and does not require
a new ontological primitive. The missing history may be a derived internal
stage of one atomic implicit solve. The theorem closes only algorithms that
try to infer `K0,K1,T` after solving endpoints alone.
