# FTD-0488 — Self-field subtraction locality

**Date:** 2026-07-25  
**Status:** `[THEOREM — LOCAL ISOLATED SELF-FIELD IMPOSSIBLE] + [CONSTRUCTIVE — GLOBAL NEUTRAL HODGE DECOMPOSITION] + [UNDERDETERMINED — SOURCE PROVENANCE]`  
**Verdict:** `LOCAL_PER_PARTICLE_SELF_FIELD_SUBTRACTION_UNAVAILABLE`

## Locality theorem

For every periodic face field,

```text
sum_i D E(i)=0
```

by exact telescoping. The same proof holds for every finite-support field on
the uncontained lattice because the shifted finite sums cancel. Consequently,

```text
D E_self = q delta_i, q!=0
```

has neither a periodic solution nor a finite-support uncontained solution. An
isolated-charge self-field requires a compensating background/partner or an
infinite tail reaching the environment/asymptotic boundary.

## Global neutral construction

For a neutral source, the minimum-norm longitudinal field

```text
E_L=D^T(DD^T)^+rho
```

exists, and any matched field decomposes as `E=E_L+E_T` with `D E_T=0` and
`<E_L,E_T>=0`. The `L=17` observer closes Gauss to `3.55e-13`, longitudinal
curl to `3.47e-17`, transverse divergence to `1.73e-17`, orthogonality to
`2.24e-19`, and the quadratic-energy split to `1.95e-15`.

The construction is explicitly global: all `14,739=3L^3` face components of
the minimum longitudinal dipole exceed the locked `1e-12` support threshold.
A lone periodic `+1` source is rejected as nonneutral.

## Provenance obstruction

Even when attributed source fields are individually neutral, total field and
Gauss data do not determine the attribution. For any `T in ker D`,

```text
E_1 -> E_1+T,
E_2 -> E_2-T
```

preserves the total field and both attributed divergences. The deterministic
control changes each attribution by `5.50e-4` while the total-field residual
and attributed Gauss residual remain below `1.12e-16`.

Minimum norm can select one global longitudinal decomposition, but applying it
per particle first requires a neutralization/partner convention. Retaining the
actual dynamical source provenance requires additional history labels absent
from the total frozen field.

## Consequence

“Subtract the particle's own field” is not a local repair of FTD-0487. It
requires a background, pair assignment, infinite/global solve, or provenance
history. No force subtraction, production toggle, or scenario is introduced.

Run of record: `engine/results/ftd_0488/windows_msvc_cpu.json`.
