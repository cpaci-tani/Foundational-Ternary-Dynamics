# FTD-0487 — Gauss/threshold force obstruction

**Date:** 2026-07-25  
**Status:** `[THEOREM — SELECTED COMPACT Q1/NEDELEC + MATCHED GAUSS]`  
**Verdict:** `NONZERO_GAUSS_SOURCE_FORCES_THRESHOLD_MULTIVALUEDNESS`

## Theorem

For the compact `FTD-0484/0485` connection reconstruction, define the axial
right-minus-left field jump at site `i` by

```text
J_a(i) = E_a(i)-E_a(i-e_a).
```

The stationary two-slab variational impulse has the corresponding one-sided
jump

```text
Delta I_a(i) = g q lambda_t J_a(i).
```

But the matched Gauss operator is exactly

```text
D E(i) = sum_a J_a(i) = rho(i).
```

The triangle inequality therefore gives

```text
max_a |J_a(i)| >= |rho(i)|/3,
max_a |Delta I_a(i)|
  >= |g q lambda_t rho(i)|/3.
```

With `g=kappa/C_SPEED`, `lambda_t=C_SPEED*Delta t`, the speed cancels:

```text
max_a |Delta I_a(i)|
  >= kappa Delta t |q rho(i)|/3.
```

Thus a nonzero manifested Gauss source forbids a globally single-valued axial
point force at its lattice site within this compact reconstruction.

## Verification

All ten checks pass on `L=17`. Positive and negative dipoles, translated and
cyclically rotated copies close Gauss and the jump identity exactly. The
pointwise lower-bound violation is zero. For a unit source and unit tick, the
normalized unavoidable impulse lower bound is
`0.0072973525643314245`.

A globally threshold-continuous control has every axial component independent
of its own coordinate and consequently has exactly zero divergence. The
converse fails: an exact matched curl has divergence residual `5.42e-20` while
retaining component jump `1.35056e-4`. Hence `rho=0` is necessary but not
sufficient for point-force continuity.

## Consequence

The `FTD-0485` threshold failure is not a poorly chosen fixture, numerical
tolerance, or gauge artifact. Gauss itself forces it wherever charge is
nonzero. Averaging the sides, choosing the incoming/outgoing cell, introducing
a subgradient, using a smoother wider-support shape, or formulating dynamics
directly as link events are possible new constructions, but each adds a rule,
state/history dependence, or representation beyond the frozen candidate.

No production toggle or scenario is licensed.

Run of record: `engine/results/ftd_0487/windows_msvc_cpu.json`.
