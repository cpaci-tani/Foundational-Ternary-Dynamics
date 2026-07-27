# DERIVATION — Accelerated-worldline energy escape

**Identifier:** `FTD-0547`  
**Status:** `[DERIVED — EXACT UNIFORM-FORCE SUBSECTOR]`  
**Inputs:** the production dispersion
`H(p)=sqrt(M^2+c^2 p^2)`, a constant collinear force during one tick, and the
FTD-0545/0546 fixed-step work defect.

## 1. The hidden straight-time assumption

Let `p` be the midpoint momentum and `a` the half-impulse, so

```text
p0=p-a,       p1=p+a,       F=2a/h.              (1)
```

FTD-0545 used the midpoint velocity for the whole tick,

```text
d_mid=h H'(p),
D_mid=H(p+a)-H(p-a)-2a H'(p).                    (2)
```

For the nonlinear relativistic dispersion, the symmetric secant is not the
midpoint derivative. Expanding (2),

```text
D_mid=(a^3/3)H'''(p)+O(a^5)
     =-c^4 M^2 p a^3/H(p)^5+O(a^5).              (3)
```

The measured FTD-0545 defect is therefore the expected error from freezing
the within-tick velocity while momentum changes. It is not by itself a
contradiction between relativistic energy and interaction work.

## 2. Exact constant-force worldline

For constant collinear force, the momentum history is fixed:

```text
p(tau)=p-a+2a tau,       0<=tau<=1.               (4)
```

Hamilton's equation gives

```text
x(tau)-x0
 =h [H(p(tau))-H(p-a)]/(2a),                     (5)

d_exact=x(1)-x(0)
 =h [H(p+a)-H(p-a)]/(2a).                        (6)
```

The `a->0` limit is `h H'(p)`. Since `|H'(p)|<c` for `M>0`, the trajectory is
causal. It remains a straight spatial segment in the collinear sector, but
its temporal parameterization is nonuniform.

The interaction work is now exactly

```text
F d_exact
 =[2a/h] h[H(p+a)-H(p-a)]/(2a)
 =H(p1)-H(p0).                                   (7)
```

Thus the matter-work defect vanishes identically without changing the
production dispersion.

## 3. Registered observer result

Across 144 preregistered arms, the exact-work residual is
`2.7105054312137611e-20`; endpoint, derivative, reversal, and causal tests
close at or below `1.60e-15`. The old frozen-velocity defect reaches
`4.1017724139693484e-05`, while the nonuniform schedule differs from the
linear schedule by as much as `0.0055368684341502161`.

## 4. Boundary

This is an exact constructive escape only for an integrable constant,
collinear force history. It does not select the force history in a spatially
varying self-consistent field, prove a neutral-pair energy transaction, or
license a production toggle. It changes the interpretation of FTD-0545/0546:
those results close the frozen linear-time step, not every possible
within-tick common-action solve.
