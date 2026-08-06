# PRE-REGISTRATION — Centered-fiber knot transaction v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0496`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0490`, `FTD-0492`--`FTD-0495`

## Question

Does the existing momentum, together with the explicitly new FTD-0495
dressing fiber, determine a unique causal and reversible centered transaction
from a manifested lattice knot?  This is an observer-only test of the minimal
nonholonomic extension.  It is not an ordinary-common-action claim.

## Exact discrete-gradient matter step

Use the production dispersion

```text
H(p)=sqrt(E_REST^2+c^2 |p|^2)
```

and define

```text
p_1-p_0=dt g q E_center^(1/2),
d=dt c^2(p_0+p_1)/(H(p_0)+H(p_1)).
```

Then algebraically

```text
H(p_1)-H(p_0)=g q E_center^(1/2) dot d.
```

The speed is causal because

```text
|d|/|dt| <= c(|p_0|+|p_1|)/(H_0/c+H_1/c) <= c.
```

## Exact centered trace of the deposited current

For a straight segment from a knot to `d`, with every `|d_a|<1`, exact
FTD-0478 deposition has centered knot trace

```text
C(K)_a=(q d_a/2) I_bc(d),
I_bc=1-(|d_b|+|d_c|)/2+|d_b d_c|/3.
```

This follows by analytically integrating the two transverse anchor hats,
`(1-|d_b|t)(1-|d_c|t)`, over `t in [0,1]`.

With

```text
E_mid=E_0-gK/2,
```

the momentum equation reduces to the three-dimensional fixed point

```text
p_1=T(p_1)
T_a=p_0,a+dt gq C(E_0)_a-(dt g^2/4)d_a I_bc(d).
```

No branch label or route variable appears.

## Locked uniqueness certificate

On `c|dt|<1`, use the conservative bounds

```text
||D_p d||_2 <= |dt| c^2/E_REST,
||D_d(d_a I_bc)||_2 <= 1+c|dt|.
```

Therefore

```text
Lip(T) <= g^2 dt^2 c^2(1+c|dt|)/(4 E_REST).
```

The production-scale arm uses `c=1/sqrt(3)`, `E_REST=0.511`, `dt=1`, and
`g=0.73`.  Require the bound to be below one, deterministic convergence from
three distinct initial guesses to the same root below `1e-12`, and an impulse
residual below `1e-12`.

## Matched field and fiber update

Deposit the exact current `K`, then set

```text
E_1=E_0-gK,
E_mid=(E_0+E_1)/2,
Delta D=g<K,E_mid>-gq E_center^(1/2) dot d.
```

Require below `1e-12`:

- exact current continuity and relative Gauss transport;
- field midpoint and centered-current trace formulas;
- matter discrete-gradient work;
- total `field + matter + D` energy;
- causal speed and locality;
- algebraic reversal of position, momentum, current, field, and fiber.

## Locked arms

1. zero field, zero momentum: unique exact rest;
2. symmetric Gauss source with zero centered bias: unique exact rest;
3. uniform and anisotropic external centered biases: nonzero uniquely signed
   displacement;
4. nonzero initial momentum with zero centered bias;
5. both polarities, translations, all 48 signed cubic maps, and three field
   amplitudes inside the contraction domain;
6. three distinct fixed-point initial guesses per representative arm.

## Frozen verdicts

- `UNIQUE_CENTERED_FIBER_KNOT_STEP` if every algebraic, uniqueness, symmetry,
  causal, energy, Gauss, and inverse gate passes.
- `KNOT_STEP_EXISTS_UNIQUENESS_UNPROVED` if identities close but contraction
  or multi-start agreement fails.
- `CENTERED_FIBER_KNOT_STEP_CLOSED_NEGATIVE` if any exact identity or inverse
  gate fails.

## Scope ceiling

Even a positive result is one knot-to-subcell step with an explicit new
history fiber.  It does not establish arbitrary-remainder evolution, integer
hops, ballistic dressed motion, magnetic exchange, reactions, an ordinary
action, a production toggle, a scenario, or infrared recovery.  Those remain
separate gates.  No production source or default may change.

Run-of-record SHA256 values:

- test: `634F5357403709C84587AC767828AB797673AE9565395C9003A6BF79E51B6A63`;
- header: `C656647420FAACC06E65AC275D2E3A8A5A0C9DE320ABF2A16EBA8F888669A073`;
- implementation:
  `0E2BEF9427E9DB131D2DB86988DEC3CC91E863BBD9B468A27D36885564884835`.
