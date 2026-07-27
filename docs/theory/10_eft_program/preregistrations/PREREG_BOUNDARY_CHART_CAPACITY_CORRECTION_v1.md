# PRE-REGISTRATION — Boundary chart-capacity correction

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0507`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only audit of the already selected
`x_eff=site+remainder` representation. No production rule, default, toggle,
scenario, tolerance, or field normalization may change.

## 1. Defect under test

FTD-0505 assigned two coincident same-sign carriers to one ternary site and
therefore reported a unit charge defect and a five-symbol charge alphabet.
FTD-0498 had already proved that a generic effective position has multiple
stable `(site,remainder)` charts. The capacity claim is valid only if collision
write-back first selects one canonical anchor. Production does not presently
perform that canonicalization.

This campaign tests whether distinct existing chart representatives provide
exact raw ternary storage for the coincident boundary state. It may scope or
retract the capacity horn of FTD-0505; it may not promote chart equivalence to
a production gauge symmetry, which FTD-0498 closed negative.

## 2. Frozen definitions

For finite unwrapped position `x` and stable remainder `r in (-1,1)^3`,

```text
pi(n,r)=n+r=x.
```

Let `k(x)` be the number of integer coordinates of `x`. The registered stable
chart count is

```text
M(x)=2^(3-k(x)).
```

For `m` identical same-sign unit carriers, each primitive ternary anchor may
store at most one. The chart-aware minimum missing charge is frozen as

```text
defect_chart=max(0,m-M).
```

If all `m` carriers are forced onto one canonical anchor, the old defect
remains `m-1`. These are different representation assumptions and must be
reported separately.

## 3. Registered fixtures

Use `L=17`, `dt=1`, speed `v=1/4`, both polarities, and three integer
translations. Exercise every nonzero Moore direction
`d in {-1,0,+1}^3`. The collision point has half-integer coordinates on the
support of `d` and integer coordinates elsewhere. Choose the two antipodal
stable charts whose anchors differ by `d`.

The registered arms are:

```text
26 directions x 2 polarities x 3 translations = 156.
```

Also test an integer lattice knot, generic half-cell point, and multiplicities
`m=1..10` against the exact count formula.

## 4. Algebraic gates

The correction is accepted only if all of the following hold at `1e-12`:

1. enumerated stable charts equal `2^(3-k)` and have distinct anchors;
2. chart-aware storage gives zero defect for `m=2` on every face, edge, and
   corner collision point;
3. a lattice knot has `M=1`, retains defect one for `m=2`, and still requires
   the old five-symbol single-site alphabet;
4. the two stored carriers' aggregate trilinear polarity equals twice the
   chart-independent single-carrier shape, with exact total charge and first
   moment;
5. identical bounce and pass-through descriptions yield the same aggregate
   endpoint density and oriented face current, with exact continuity;
6. signed cubic maps and integer translations preserve chart count, defect,
   shape, and current.

## 5. Actual production continuation gate

Initialize the two collision charts as distinct actual `RenderBridge` sites,
assign equal/opposite outward velocities `+/-v d/|d|`, enable only movement,
and run one CPU tick. Require:

- both ternary anchors remain distinct and occupied;
- effective positions separate by `2v` without an occupied-target event;
- raw state, velocity, and remainder agree with analytic drift;
- reversing both velocities and running one more tick restores both collision
  charts within `1e-12`;
- no field or RNG-dependent branch is involved.

This gate demonstrates representability and reversible continuation only. It
does not derive the collision impulse that chooses outward velocities.

## 6. Locked verdicts

- If every gate passes:
  `BOUNDARY_CAPACITY_DEPENDS_ON_CHART_MULTIPLICITY`.
- If the algebra passes but production cannot continue/reverse:
  `CHART_CAPACITY_IS_OBSERVER_ONLY`.
- If distinct charts cannot store the exact state:
  `SINGLE_SITE_CAPACITY_BOUND_CONFIRMED`.

The first verdict retracts the unconditional five-symbol claim in FTD-0505
and replaces it with a conditional theorem: extra capacity is required only
after canonical single-anchor write-back or at positions with insufficient
chart multiplicity. It leaves the need for an explicit collision impulse and
the FTD-0498 production non-factorization result intact.

## 7. Execution record

The locked body above had SHA256
`1FE273915B5C760AF3BEEC23532DF6980F5CB6C1CFEFB937E90A6D2877446007`
before this execution section and status transition were appended. The
registered verdict was reached without changing a gate or tolerance.

- test SHA256:
  `81530A5A596C140DA1F522DB6877F7C4D2E92D2A78EB931BE9A5334E4F1A2638`;
- header SHA256:
  `97E8E322383AF793196CB6A7548FF9137D42A705AAE4FBBFE7352690FF15E658`;
- implementation SHA256:
  `DE9B13E38423BE3251F7F497314588A01FA91AA4E9AB69DA3BCD9506AF6BD378`;
- result: `6/6 PASS`;
- verdict: `BOUNDARY_CAPACITY_DEPENDS_ON_CHART_MULTIPLICITY`.

