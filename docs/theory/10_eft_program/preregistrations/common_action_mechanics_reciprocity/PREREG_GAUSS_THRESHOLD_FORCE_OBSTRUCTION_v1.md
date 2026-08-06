# PRE-REGISTRATION — Gauss/threshold force obstruction v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0487`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0485` and `FTD-0486`

## Question

Is the `FTD-0485` hop-threshold force jump an avoidable fixture artifact, or
is a nonzero jump forced by the matched Gauss source for every compact
Q1/Nedelec point-force reconstruction?

## Frozen identity

At site `i`, define the one-sided normal-field jumps

```text
J_a(i) = E_a(i) - E_a(i-e_a).
```

For a stationary two-slab worldline at the coordinate plane through `i`, the
right-minus-left variational impulse in direction `a` is

```text
Delta I_a(i) = g q lambda_t J_a(i).
```

The existing backward divergence gives identically

```text
sum_a J_a(i) = D E(i) = rho(i).
```

Therefore

```text
max_a |J_a(i)| >= |rho(i)|/3.
```

With the minimal-action normalization `g=kappa/C_SPEED` and
`lambda_t=C_SPEED*Delta t`, at least one axial one-sided impulse jump obeys

```text
max_a |Delta I_a(i)| >= kappa Delta t |q rho(i)|/3.
```

No statistical or continuum assumption enters this bound.

## Locked tests

Use `L=17` and require below `1e-12`:

1. the component jumps sum to the existing `divergence_at` value at every
   site for a deterministic dipole path and translated/cyclically rotated
   copies;
2. the dipole source comparison `D E=rho` is exact;
3. the pointwise triangle-inequality bound has no violation;
4. both source polarities satisfy the bound;
5. a globally threshold-continuous control—each component independent of its
   own coordinate—has exactly zero divergence;
6. a divergence-free curl control may still have nonzero component jumps,
   proving that `rho=0` is necessary but not sufficient for force continuity;
7. the normalized impulse lower bound equals the direct coefficient expression
   below `1e-15`.

## Frozen interpretation

If the tests pass, record a theorem conditional only on the selected compact
Q1/Nedelec reconstruction and matched Gauss law:

> A nonzero manifested Gauss source forbids globally single-valued axial
> point force at its lattice site.

The theorem closes the hypothesis that the `FTD-0485` threshold failure can be
removed by a different allowed field fixture or tighter tolerance. It does not
exclude a link-event action that never requests a point force, a wider smooth
shape, or an explicit branch/subgradient rule. Each is a new dynamics or
state/history selection and is outside this campaign.

No production toggle, scenario, or constant identification follows.

Run-of-record test-source SHA256:
`E6811E8AFA9AD8E568A5B1689F8652B942D13296B09DA1613AB0877FECBFF84C`.
