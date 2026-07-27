# PRE-REGISTRATION — Subcell representation quotient v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0498`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0478`, `FTD-0484`, `FTD-0497`

## Question

Is the FTD-0497 inverse defect only a redundant-coordinate artifact, or do
frozen production observables distinguish different `(site,remainder)` charts
of the same effective position?

The observer will characterize the exact quotient before considering any
representation repair. It may not modify production dynamics or declare the
quotient ontological by fiat.

## Locked chart space

Use stable interior remainders `r in (-1,1)^3` and projection

```text
pi(n,r)=n+r=x.
```

Define chart equivalence by

```text
(n,r) ~ (n+h,r-h),
h in Z^3,
r and r-h in (-1,1)^3.
```

If exactly `k` coordinates of `x` are integers, the locked multiplicity is

```text
|pi^-1(x)|=2^(3-k).
```

Thus a generic subcell point has eight stable chart representatives, a point
on one lattice plane has four, a point on two planes has two, and a knot has
one. Closed threshold endpoints `r=+/-1` are excluded because production
immediately reanchors them.

## Locked quotient dynamics theorem

For the existing componentwise threshold map `M_d`, prove and test

```text
pi(M_d(n,r))=pi(n,r)+d.
```

Therefore it descends to

```text
bar M_d(x)=x+d,
bar M_-d bar M_d=id,
```

even though FTD-0497 proves `M_d` is not injective on raw charts.

## Locked factorization tests

At generic effective endpoints `x_0` and `x_1`, enumerate all eight charts
of each endpoint and require below `1e-12`:

1. every FTD-0478 trilinear polarity distribution is identical;
2. all 64 start/end chart pairs produce identical charge endpoints and exact
   straight-segment face current;
3. current continuity, effective first moment, and physical quotient reversal
   are chart-independent;
4. both polarities and translated copies agree.

These establish factorization only for the coupling sidecar and face current.

## Locked production non-factorization probes

Encode the same generic effective position in two adjacent raw charts using
actual `RenderBridge` states. With all unrelated toggles disabled:

1. measure the raw ternary-field L1 difference;
2. execute the native `-G_C grad(s)` source phase and measure the complete
   `wave_vel` response difference;
3. place an identical moving probe whose next production hop targets one of
   the two candidate anchors, execute `phase_movement_main_loop`, and compare
   collision/movement outcomes;
4. confirm that the FTD-0478 distributed polarity shapes of the two primary
   states remain identical throughout the initial comparison.

Any nonzero raw state, source, or collision difference proves the frozen
production engine does not factor through the quotient.

## Frozen verdicts

- `SUBCELL_QUOTIENT_IS_ENGINE_GAUGE` only if every registered coupling and
  production observable factors through `pi`.
- `FACE_PHYSICS_FACTORS_PRODUCTION_DOES_NOT` if shapes/currents factor exactly
  but at least one frozen production state/source/collision probe differs.
- `SUBCELL_QUOTIENT_CLOSED_NEGATIVE` if the FTD-0478 shape or exact current
  itself depends on chart choice.

## Scope ceiling

A split verdict is a theorem about the current ontology, not permission to
rewrite it. Quotienting manifestation, canonicalizing remainders, or adding a
chart-memory variable each requires a new selected cycle. No production toggle,
scenario, multi-axis dynamics, or infrared claim follows from this audit.

Run-of-record SHA256 values:

- test: `B70C2C810C6535F1807F59F3CFC021EC1CE4CD12D3E02073DD47D27669B56C25`;
- header: `C86499B196461E040BC714A1B9EDD36CC142B41E2DF5BFA0F8CA83DA553972EB`;
- implementation:
  `8CD5AEFAC60E984D1324F1C594A9154D5FA937CF918AD0D2083CC7EB2EA8E36C`.
