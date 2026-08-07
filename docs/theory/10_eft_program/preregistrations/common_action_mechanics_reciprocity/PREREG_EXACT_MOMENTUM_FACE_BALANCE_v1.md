# PRE-REGISTRATION — Exact momentum face balance

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0514`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only lift of exact scalar worldline continuity to
constituent momentum density and tensor face flux. No production state,
default, toggle, scenario, force, collision law, field normalization, or
ontology change.

## 1. Question

FTD-0513 identifies the kinetic-stress moment that observes axial
counterflow, but does not give a local lattice balance law. This campaign asks
whether the already exact face-current segment supplies that law without a
new stencil or route choice.

For a straight carrier segment with constant momentum `p`, let the unit
trilinear density and exact oriented face current obey

```text
Delta rho + div J = 0.
```

Define componentwise momentum density and integrated momentum face flux

```text
g_i = p_i rho,
Pi_ij = p_i J_j.
```

The registered identity is

```text
Delta g_i + div_j Pi_ij = 0.
```

For an instantaneous momentum change `Delta p` at a fixed vertex, define the
impulse source

```text
I_i = Delta p_i rho_vertex,
Delta g_i - I_i = 0.
```

Piecewise transport plus impulse must therefore satisfy

```text
Delta g_i + div_j Pi_ij - I_i = 0.
```

## 2. Stress bridge

The exact face-current first-moment identity under test is

```text
sum_faces J_j = Delta x_j.
```

Consequently,

```text
sum_faces Pi_ij = p_i Delta x_j.
```

For free dispersion motion `Delta x=v dt`, the summed integrated face flux is

```text
sum_faces Pi_ij = dt p_i v_j.
```

Summing carriers gives exactly `dt Sigma_ij`, with `Sigma` the FTD-0513
kinetic-stress moment. This is a bridge between the local face tensor flux and
the global stress observer; it is not a new field equation.

## 3. Registered fixtures

Use `L=17`, rest energy `0.511`, `c=1/sqrt(3)`, both polarities, three
translations, every nonzero Moore direction, and speeds `1/8` and `1/4`.

Free arms:

```text
26 x 2 polarities x 3 translations x 2 speeds = 312.
```

Selected FTD-0512 collision arms use the same 312 fixtures, with incoming and
outgoing segment length `1/4` around the common boundary vertex.

All residual gates are fixed at `1e-12`.

## 4. Free-transport gates

For every free arm require:

1. local componentwise momentum-balance residual below `1e-12`;
2. exact global momentum conservation;
3. face-flux first moment equal to `p tensor Delta x`;
4. for dispersion-generated displacement, equality to `dt Sigma`;
5. translation, polarity, and signed-cubic covariance;
6. causal displacement below `c dt`.

## 5. Collision gates

For each registered selected reflection:

1. incoming and outgoing segment balances close separately;
2. each constituent impulse source exactly equals its vertex momentum jump;
3. the two internal impulse sources cancel site by site;
4. the complete pair balance closes with zero aggregate external source;
5. the individual impulse-source L1 norm is strictly positive;
6. total momentum and relativistic matter energy are unchanged;
7. under time reversal, endpoint momentum densities negate and swap, while
   tensor momentum flux and the integrated impulse source remain even;
8. the integrated tensor-flux moment equals the sum of the incoming and
   outgoing `p tensor Delta x` segment moments.

These gates show compatibility of the selected contact with exact local
momentum bookkeeping. They do not derive the contact premise or create a
conjugate stress field.

## 6. Locked verdicts

- If free and collision gates pass:
  `EXACT_MOMENTUM_FACE_BALANCE_CLOSES_SELECTED_CONTACT_COMPATIBILITY_ONLY`.
- If free balance closes but collision source does not cancel locally:
  `FREE_STRESS_BRIDGE_ONLY_COLLISION_LOCALITY_FAILS`.
- If the componentwise free lift fails:
  `MOMENTUM_FACE_BALANCE_CLOSED_NEGATIVE`.

No verdict authorizes a production toggle. A common action still requires a
native interaction functional whose variation selects the impulse, rather
than an observer that balances it after selection.

## 7. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked body hash for §§1–6 was
`05C02C5075CD2DA1359094C13CBF40A2D101E18FE5651B7F17A19A71BF5A9419`.

All `5/5` registered checks passed over 312 free and 312 selected-collision
arms. The largest balance/stress residual was `2.22e-16`; aggregate internal
impulse-source L1 was exactly zero and the minimum summed individual source L1
was `0.78512214774851818`. The locked pass verdict applies:

```text
EXACT_MOMENTUM_FACE_BALANCE_CLOSES_SELECTED_CONTACT_COMPATIBILITY_ONLY
```

Canonical result:
[`AUDIT_EXACT_MOMENTUM_FACE_BALANCE.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_EXACT_MOMENTUM_FACE_BALANCE.md).
