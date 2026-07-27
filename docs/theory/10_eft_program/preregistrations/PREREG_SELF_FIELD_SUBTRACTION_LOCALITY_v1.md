# PRE-REGISTRATION — Self-field subtraction locality v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0488`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0487`

## Question

Can the Gauss-forced threshold jump be removed by subtracting each manifested
particle's own longitudinal field using only local frozen state, without a
background, partner assignment, or field-provenance history?

## Exact solvability statements

For a periodic field, telescoping gives

```text
sum_i D E(i) = 0.
```

The same identity holds on the uncontained lattice for every finite-support
field because all shifted finite sums cancel. Therefore no periodic field and
no finite-support uncontained field can solve

```text
D E_self = q delta_i
```

for `q!=0`. An individual-charge self-field requires a compensating
background/partner or an infinite tail reaching the environment/asymptotic
boundary.

For a neutral source, freeze the global minimum-norm longitudinal solution

```text
E_L = D^T (D D^T)^+ rho.
```

Every total field with `D E=rho` then decomposes as

```text
E = E_L + E_T,
D E_T = 0,
<E_L,E_T> = 0.
```

The decomposition is a global Hodge choice, not a local event rule.

## Locked tests

Use `L=17` and require:

1. periodic divergence telescopes below `1e-12` on deterministic fields;
2. a lone `+1` source is rejected as nonneutral by the matched minimum-energy
   solver;
3. a neutral dipole has a converged minimum-norm longitudinal field with Gauss
   and curl-adjoint residuals below `1e-10`;
4. adding a deterministic matched curl and decomposing it recovers a
   divergence-free transverse remainder, longitudinal/transverse inner product,
   and quadratic-energy split below `1e-10`;
5. the minimum-norm field has lower energy than a routed string field;
6. source attribution is nonunique: adding `T in ker D` to one attributed
   neutral source field and `-T` to another preserves the total field and both
   source divergences while changing both attributed fields;
7. translated and polarity-mirrored neutral sources preserve the result.

## Frozen interpretation

A passing result closes local per-particle self-field subtraction for the
frozen variables. It does not forbid a global Poisson/Hodge observer, an
explicit neutralizing background, an infinite dressing with retained source
provenance, or pair-labelled fields. Those are new nonlocal structures or
state/history variables and cannot be silently used to repair FTD-0487.

No production toggle, scenario, or force subtraction follows.

Run-of-record test-source SHA256:
`90429696010497152E799F740E3ED6561CDA144A8C7B58F2119F8CD236DC7793`.
