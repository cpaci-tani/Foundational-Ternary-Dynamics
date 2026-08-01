# FTD-0733 — Capture-energy shell geometry v1

**Status:** `[THEOREM FOR THE SELECTED POTENTIAL] + [CERTIFIED DATA]`  
**Verdict:** `SELECTED_CAPTURE_ENERGY_SHELL_DERIVED`  
**Production status:** unchanged

## Result

Let `d=r^2`, `D=0.01`, and

```text
V(d) = -16 D (d-3/2)^2 (d-3/4),  0 <= d < 3/2,
       0,                           d >= 3/2.
```

For fixed instantaneous constituent kinetic energy `0<K<D`, define
`E_K(d)=K+V(d)`. The exact derivative is

```text
dE_K/dd = -48 D (d-3/2)(d-1).
```

Therefore `E_K` decreases strictly from `K` to `K-D` on `(3/4,1)`
and increases strictly from `K-D` to `K` on `(1,3/2)`. It has unique roots

```text
d_-(K) in (3/4,1),   d_+(K) in (1,3/2),
```

and its negative-energy radial set is exactly

```text
C_K = (sqrt(d_-(K)), sqrt(d_+(K))).
```

Implicit differentiation gives `d d_-/dK>0` and `d d_+/dK<0`; the interval
contracts monotonically with kinetic energy and collapses to `d=1` at `K=D`.
This theorem is exact for the selected polynomial. It is not a derivation of
that polynomial from the five postulates.

## FTD-0732 data certificate

For each volume and cubic direction, the largest persisted kinetic level is
the preregistered `radial_impulse_plus` variant. Its interval is consequently
the common radial domain for all registered kinetic levels in that group.
Polarity mirrors agree identically.

| `L` | direction | `K_max` | parent `r` | common `r_inner` | common `r_outer` | allowed scale | parent `u` |
|---:|---|---:|---:|---:|---:|---:|---:|
| 33 | face | `0.007986389` | `0.950834958` | `0.937947031` | `1.069637259` | `(0.986445674, 1.124945239)` | `0.0920741` |
| 33 | edge | `0.008898204` | `0.964014274` | `0.953669518` | `1.050324161` | `(0.989269085, 1.089531752)` | `0.1024184` |
| 33 | body | `0.008279452` | `0.943198796` | `0.942491282` | `1.063907765` | `(0.999249878, 1.127978290)` | `0.0054766` |
| 65 | face | `0.007993155` | `0.950964367` | `0.938047864` | `1.069508709` | `(0.986417469, 1.124656975)` | `0.0924518` |
| 65 | edge | `0.008916516` | `0.964514771` | `0.954045353` | `1.049879038` | `(0.989145404, 1.088504883)` | `0.1045920` |
| 65 | body | `0.008337859` | `0.944169469` | `0.943444522` | `1.062721965` | `(0.999232185, 1.125562730)` | `0.0057187` |

Here

```text
u = (r_parent^2-d_-)/(d_+-d_-).
```

All parents are strictly inside the common interval. All old `0.95 r_parent`
probes are outside it, while every `1.05 r_parent` probe is inside. Root signs
were isolated on the exact monotone branches to interval width below `1e-30`.

## What FTD-0732 actually exposed

The selected captured domain is open but non-rectangular in raw position and
momentum coordinates. Increasing kinetic energy shrinks both radial margins.
The body-diagonal parent is especially close to the inner common boundary:
only a multiplicative inward change of about `7.5e-4` remains when combined
with the largest registered radial impulse. A Cartesian `5%` cross therefore
mixes admissible states with points outside the selected negative-energy
sector.

This does not prove dynamical stability. It only supplies the correct domain
on which a dynamical neighborhood test can be posed.

## Ontological reading

The candidate matter state is not a single `+1` voxel and is not specified by
separation alone. At this stage it is a constrained relational state comprising
at least constituent positions, constituent momenta, oriented event current,
and the coupled face/edge field. “Objecthood” corresponds to a region of this
joint state space whose total relational energy is below the continuum
threshold. The inner boundary is energetic rather than a rigid material wall.

No extra primitive is indicated by the FTD-0732 failure. The failure arose
because the proposed probe left the already-defined state domain before the
transaction began.

## Next gate

Use an energy-adapted local coordinate. For each momentum assignment, compute
its own `d_-(K),d_+(K)` and define the nearest squared-radius margin

```text
m_K = min(d_parent-d_-(K), d_+(K)-d_parent).
```

The next mixed-corner campaign should preregister `d_parent`,
`d_parent-m_K/2`, and `d_parent+m_K/2`, crossed with the existing momentum and
dynamic-field perturbations. The factor `1/2` is the midpoint of the certified
local margin, not a fitted replacement for `5%`. Surviving that finite box is
still not an open-basin theorem, but failure would then be dynamical rather
than an initialization artifact.

## Verification

- protocol `E4C639DC…E26DB`;
- frozen FTD-0732 CSV `15926F9E…E2AD`;
- independent certificate `0574272D…812C`, `654/654 PASS`.

