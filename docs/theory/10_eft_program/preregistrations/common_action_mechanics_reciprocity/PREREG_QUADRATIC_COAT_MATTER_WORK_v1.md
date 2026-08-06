# PRE-REGISTRATION — Quadratic-coat matter-work identity

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0545`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0490`, `FTD-0542`, `FTD-0543`, `FTD-0544`  
**Scope:** observer-only endpoint variation of the FTD-0542 interaction and
comparison with the production dispersion. No endpoint solve, field repair,
energy projection, lapse, force, toggle, default, or scenario is allowed.

## 1. Locked normalization and endpoint maps

Use exactly

```text
h=C_SPEED,
g=beta/h,
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2),
S_m=-(E_REST h/C_SPEED)sqrt(1-|d|^2/h^2).          (1)
```

For the FTD-0542 interaction `S_int(x0,x1;A0,A1,Phi)`, derive its endpoint
gradients analytically from the continuous straight segment. With
`x(t)=x0+t d`, `A(t,x)=(1-t)A0(x)+t A1(x)`, and
`F=A(t,x).d-h Phi(x)`, require

```text
D_x0 S_int=qg integral[-A+(1-t)(grad A)^T d
                       -h(1-t)grad Phi]dt,
D_x1 S_int=qg integral[ A+t(grad A)^T d
                       -h t grad Phi]dt.            (2)
```

Use the same `B1/B2` face interpolation and `B2^3` scalar interpolation as
FTD-0542. Split at half-integer knots and use four-point Gauss-Legendre
integration. Reject a sample that lands exactly on a `B1` derivative kink.

Let `p` be the free momentum from (1). Define

```text
P0=p-D_x0 S_int,
P1=p+D_x1 S_int,
pi0=P0-qg A0(x0),
pi1=P1-qg A1(x1).                                  (3)
```

The matter/work defect is

```text
D=H(pi1)-H(pi0)-beta<E,K>.                         (4)
```

Require the direct line action to agree with the independently deposited
FTD-0542 action below `1e-12`. Under a nontrivial gauge transformation, require
`pi0`, `pi1`, `E`, `<E,K>`, and `D` invariant below `1e-10`.

## 2. Locked uniform harmonic witness

Take constant face connections with `Phi=0` and

```text
A1-A0=-h E,
B0=B1=0.
```

Partition and first-moment reproduction force

```text
pi0=p-a,
pi1=p+a,
a=beta q E/2,
beta<E,K>=2 a.C_SPEED^2 p/H(p).                    (5)
```

For a collinear positive `p,a`, the exact defect is

```text
D=H(p+a)-H(p-a)-2a C_SPEED^2 p/H(p).               (6)
```

Its small-`a` leading term is

```text
D=-(C_SPEED^4 E_REST^2 p/H(p)^5)a^3+O(a^5).        (7)
```

Use `E_REST=0.511`, `beta=1`, free momenta `0.1,0.2,0.3`, field amplitudes
`0.04,0.08,0.12`, both polarities, all three axes, and one proper-cubic
diagonal. Require (5) and (6) below `1e-11`, zero/pure-gauge controls below
`1e-12`, and at least one registered nonzero defect above `1e-8` with the sign
of (7).

## 3. Locked scope and verdicts

The uniform mode is a harmonic external-field sector. It proves or disproves
a universal fixed-step matter-work identity of the action, but it is not a
Gauss-realizable self-field solution for one net periodic charge. A negative
result must therefore trigger a neutral, self-consistent coupled-pair gate
before closing all fixed-step reciprocal dynamics.

- every endpoint, action, gauge, and analytic gate closes and a nonzero defect
  is present: `COAT_FIXED_STEP_MATTER_WORK_NOT_IDENTITY`;
- every registered defect vanishes: `COAT_MATTER_WORK_IDENTITY_CONSTRUCTIVE`;
- endpoint/action equivalence fails: `COAT_ENDPOINT_VARIATION_INVALID`;
- only floating/knot gates fail: `COAT_MATTER_WORK_UNRESOLVED`.

No result licenses production dynamics.
