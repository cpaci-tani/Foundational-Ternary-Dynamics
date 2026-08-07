# AUDIT — Boundary chart capacity

**Date:** 2026-07-25  
**Identifier:** `FTD-0507`  
**Status:** `[THEOREM — STABLE-CHART CAPACITY]` +
`[CORRECTION/RETRACTION — FTD-0505 UNCONDITIONAL CAPACITY HORN]` +
`[MEASURED — REVERSIBLE PRODUCTION CONTINUATION AFTER SELECTED IMPULSE]` +
`[THEOREM — AXIAL RELATIVE-MODE CURRENT KERNEL]`  
**Verdict:** `BOUNDARY_CAPACITY_DEPENDS_ON_CHART_MULTIPLICITY`  
**Pre-registration:**
[`PREREG_BOUNDARY_CHART_CAPACITY_CORRECTION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_BOUNDARY_CHART_CAPACITY_CORRECTION_v1.md)  
**Run of record:** `engine/results/ftd_0507/windows_msvc_cpu.json`

## 1. FTD-0505 used the wrong storage premise

FTD-0505 treated two coincident same-sign carriers as two units written to one
ternary anchor. That is a valid conditional calculation, but it is not the
frozen `(site,remainder)` representation. For stable charts,

```text
pi(n,r)=n+r=x,   r in (-1,1)^3.
```

If `k` coordinates of `x` are integers, exact enumeration gives

```text
M(x)=|pi^-1(x)|=2^(3-k).
```

The `M` charts have distinct anchors. Because primitive ternary occupancy is
attached to the anchor and FTD-0498 proves production distinguishes those
anchors, they are real storage slots in the frozen raw state, not gauge copies
of one production site.

For `m` identical same-sign carriers, the exact raw storage defect is therefore

```text
defect_chart=max(0,m-M),
```

not `m-1` unless canonical single-anchor write-back is imposed. The complete
`m=1..10` enumeration at points with `M={8,4,2,1}`, for both polarities,
passed with zero position, shape, charge, and first-moment residual.

## 2. Face, edge, and corner boundary collisions fit the ternary state

For a Moore direction `d`, place the collision point at a half integer on each
nonzero component of `d` and at an integer on every zero component. If `h` is
the number of nonzero components, the point has

```text
M=2^h.
```

Thus a face, edge, or corner event supplies respectively `2`, `4`, or `8`
distinct stable anchors. Two same-sign carriers fit without changing the
three-symbol alphabet. Across all 26 directions, both polarities, and three
translations (`156` arms), the selected antipodal charts:

- had anchor difference exactly `d`;
- represented the identical effective collision point;
- stored one primitive polarity on each of two distinct sites;
- reproduced twice the chart-independent trilinear shape exactly;
- preserved exact charge and first moment;
- gave identical aggregate currents for pass-through and equal-mass bounce,
  with worst continuity residual `4.16e-17`.

The FTD-0504 identical-particle permutation quotient therefore extends across
these stored boundary slices. The labels may exchange while the raw anchors
continue to carry two manifested records.

## 3. The old five-symbol theorem survives at a knot or after canonicalization

At an integer lattice knot, stable-chart enumeration gives `M=1`. Two
same-sign carriers then have defect one and require local charge `+/-2`; a
charge-faithful single-site alphabet still needs

```text
{-2,-1,0,+1,+2}.
```

The same conclusion holds if a future shape-ontic branch first canonicalizes
every coincident carrier to one anchor. The corrected theorem is therefore:

> A coincident multiplicity exceeds ternary capacity exactly when its carrier
> count exceeds the number of distinct retained raw charts at that point.

FTD-0505's unconditional five-symbol claim and its resulting
capacity/range/phase trilemma are retracted. Its zero-posttime separation
theorem and its demonstration that a hard-core radius is an added selection
remain valid.

## 4. Existing production can carry the outgoing phase

The observer manually assigned the equal-mass outgoing velocities at each
stored boundary event and then ran the actual CPU movement phase with every
other toggle disabled. This does not derive the impulse; it asks only whether
the frozen raw state can carry the result.

Across all `156` arms:

```text
analytic drift residual              0
separation residual                  2.72e-15
reverse-tick phase-space residual    0
field residual                       0
different-seed output residual       0
journaled site-hop events             0.
```

Both anchors remain occupied, the effective positions separate by `2v`, and
velocity reversal returns both collision charts exactly. Consequently an
instantaneous collision impulse at the stored slice needs no new capacity,
finite radius, or extra temporal phase for face/edge/corner events. What is
missing is the law that selects that impulse.

## 5. The trilinear coupling loses the axial relative mode

The correction exposes a different obstruction. In one dimension, inside one
linear cell,

```text
rho(x)=(1-f)e_m+f e_(m+1).
```

At the face midpoint and for `0<=delta<=1/2`,

```text
rho(x-delta)+rho(x+delta)=2 rho(x).
```

The two same-sign carriers can move apart while their aggregate deposited
polarity remains exactly unchanged. The two exact straight-segment face
currents cancel on every face. The measured outgoing current norms were

```text
face directions    0 exactly
edge directions    0.125 within 4.5e-16
corner directions  0.125 within 1.4e-15.
```

The edge/corner tensor products retain even cross terms, while a pure axial
hat is affine and loses the counterflow completely. This is the dynamical
version of FTD-0501's multibody noninjectivity: aggregate face density/current
does not contain every constituent relative mode.

Therefore the face field alone cannot generate or account for an axial
collision impulse at exact overlap. A reciprocal collision law must consume
the already-associated constituent phase space before aggregation, or the
coupling representation must gain a higher moment/internal channel. Enlarging
only the site charge alphabet would not repair this loss.

## 6. Corrected boundary

The surviving collision problem is narrower and sharper:

1. storage capacity is already present at non-knot boundary points through raw
   chart multiplicity;
2. the production bounce remains invalid because it resets phase and omits
   target/field recoil (FTD-0506);
3. conservation plus identical-particle quotient permits a reciprocal
   outward continuation, but does not select its impulse in general 3D;
4. aggregate trilinear face coupling erases the axial relative mode needed to
   derive that impulse from the field.

No production code, default, toggle, scenario, force, collision rule, or
tolerance changed.

- checks: `6/6 PASS`;
- test SHA256:
  `81530A5A596C140DA1F522DB6877F7C4D2E92D2A78EB931BE9A5334E4F1A2638`;
- header SHA256:
  `97E8E322383AF793196CB6A7548FF9137D42A705AAE4FBBFE7352690FF15E658`;
- implementation SHA256:
  `DE9B13E38423BE3251F7F497314588A01FA91AA4E9AB69DA3BCD9506AF6BD378`;
- locked preregistration-body SHA256:
  `1FE273915B5C760AF3BEEC23532DF6980F5CB6C1CFEFB937E90A6D2877446007`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.

