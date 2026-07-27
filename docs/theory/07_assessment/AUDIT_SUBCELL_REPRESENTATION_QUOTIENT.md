# AUDIT — Subcell representation quotient

**Date:** 2026-07-25  
**Identifier:** `FTD-0498`  
**Status:** `[THEOREM — QUOTIENT MULTIPLICITY/FACTORIZATION]` +
`[MEASURED — PRODUCTION NON-FACTORIZATION]` +
`[CLOSED NEGATIVE — QUOTIENT AS FROZEN ENGINE GAUGE]`  
**Verdict:** `FACE_PHYSICS_FACTORS_PRODUCTION_DOES_NOT`  
**Pre-registration:**
[`PREREG_SUBCELL_REPRESENTATION_QUOTIENT_v1.md`](../10_eft_program/preregistrations/PREREG_SUBCELL_REPRESENTATION_QUOTIENT_v1.md)  
**Run of record:** `engine/results/ftd_0498/windows_msvc_cpu.json`

## 1. `site + remainder` is an overlapping coordinate atlas

For stable interior remainders define

```text
pi(n,r)=n+r=x,
r in (-1,1)^3.
```

In one noninteger coordinate, `x=m+f` with `0<f<1` has two charts:

```text
(m,f),
(m+1,f-1).
```

An integer coordinate has only `(m,0)` after excluding transient threshold
endpoints. Taking Cartesian products gives

```text
|pi^-1(x)|=2^(3-k),
```

where `k` is the number of integer coordinates. The registered counts are
therefore `8,4,2,1` for a generic point, lattice plane, lattice line, and knot.

This eight-chart generic multiplicity is a coordinate fact. It is not the
FTD-0491 eight-branch dynamical claim, although both arise from the eight
incident cubic sectors.

## 2. Threshold motion is reversible on the quotient

Every component of the existing threshold update changes anchor and remainder
by opposite integers. Hence

```text
pi(M_d(n,r))=pi(n,r)+d.
```

The induced quotient map is simply

```text
bar M_d(x)=x+d,
bar M_-d bar M_d(x)=x.
```

The observer finds zero quotient-position and quotient-inverse residual. This
explains the FTD-0497 split: its failed raw inverse still recovered the exact
physical position.

## 3. Trilinear polarity is exactly chart-independent

For either chart in one dimension, the endpoint weights reduce to

```text
w_m=1-f,
w_(m+1)=f.
```

Their three-dimensional tensor product is therefore the unique physical shape

```text
rho_i(x)=q product_a max(0,1-|x_a-i_a|),
```

independent of which of the eight incident sites is named the anchor. Both
polarities and all eight charts agree exactly in the run of record.

## 4. Exact face current also factors through the quotient

FTD-0484 identifies the FTD-0478 current as a cubical Whitney one-form line
integral,

```text
K_f=q integral_path W_f^(1)(x) dot dx.
```

It depends on the physical straight path and not its endpoint chart labels.
The observer evaluates both polarities over all `8 x 8` start/end chart pairs:
`128` complete transactions. Endpoint charge and every oriented face-current
component are identical, with

```text
worst chart/current residual  0
worst continuity residual     2.78e-17.
```

Thus subcell polarity, current, relative Gauss transport, and face work are
honest quotient observables.

## 5. Primitive ternary manifestation is not a quotient observable

The same generic position was encoded in two adjacent actual `RenderBridge`
states:

```text
(n,r),
(n+e_x,r-e_x).
```

Their distributed FTD-0478 polarity shapes are identical. Their primitive
ternary arrays differ at two sites, giving exact L1 distance `2`. Therefore
the actual state field does not factor through `pi`.

This is not a renderer-only distinction. Production rules consume the raw
site field.

## 6. Native source dynamics distinguishes the charts

With all unrelated toggles disabled, each bridge executes the production

```text
Delta wave_vel=-G_C grad(s)
```

rule with formula residual zero. The two complete source responses differ by

```text
max |Delta wave_vel_A-Delta wave_vel_B|
=0.042712271551427185.
```

The face coupling sidecar treats the charts as identical while the native
state-flux source treats them as sources one lattice site apart.

## 7. Production collision outcomes distinguish the charts

An identical positive probe was placed one site below the lower candidate
anchor with remainder `0.9` and velocity `+0.2`. Under the real movement phase:

- when the equivalent primary is anchored at the lower site, the probe meets
  same-sign matter, remains at its source site, and reverses to `-0.2`;
- when the primary is anchored at the adjacent site, the lower target is void,
  the probe moves into it, and retains `+0.2`.

The registered velocity-outcome difference is exactly `0.4`. Thus collision
occupancy, not merely a diagnostic field, distinguishes the charts.

## 8. Ontological consequence

The frozen engine currently combines two incompatible readings:

1. the face-current construction treats effective position and distributed
   polarity as physical, making the anchor a redundant chart label;
2. production manifestation, sources, collisions, reactions, and update order
   treat the anchor itself as physical.

Both can coexist as a selected approximation, but they cannot support a claim
that `(site,remainder)` chart changes are gauge redundancy of the present
engine. The quotient is exact for face physics and false for production matter.

The three honest continuations remain distinct:

- **site-ontic:** retain production matter unchanged and demote `site+remainder`
  from literal physical position to a kinematic accumulator;
- **shape-ontic:** promote the quotient and rewrite every site-local matter
  rule to consume the distributed polarity shape;
- **history-ontic:** retain anchor physics and add sufficient reversible chart
  history to distinguish merged preimages.

No option is selected here.

## 9. Plan consequence and reproducibility

The verdict `FACE_PHYSICS_FACTORS_PRODUCTION_DOES_NOT` strengthens the
FTD-0497 closure. A production mobile-face branch cannot be added by changing
only the field representation; matter ontology must change first.

FTD-0528 later sharpens the source boundary for the identical-contact pair:
even when primitive occupancy is the same, native `curl(sv)` distinguishes
the raw velocity assignment in every Moore direction. The complete matched
history current still factors through the quotient.

- checks: `13/13 PASS`;
- test SHA256:
  `B70C2C810C6535F1807F59F3CFC021EC1CE4CD12D3E02073DD47D27669B56C25`;
- header SHA256:
  `C86499B196461E040BC714A1B9EDD36CC142B41E2DF5BFA0F8CA83DA553972EB`;
- implementation SHA256:
  `8CD5AEFAC60E984D1324F1C594A9154D5FA937CF918AD0D2083CC7EB2EA8E36C`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
