# FTD-0614 — Refined-core Peierls landscape and covariance

**Status:** `[MEASURED — POSITIVE SELECTED-PATH BARRIERS]` +
`[MEASURED — PROPER-CUBIC WHOLE-STATE COVARIANCE]` +
`[NUMERICAL FACT — INTERNAL BRANCH HYSTERESIS]` +
`[NUMERICALLY UNRESOLVED — UNIQUE PASSIVE LANDSCAPE]`
**Verdict:** `REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED`
**Production status:** unchanged

## 1. Result

The FTD-0612 fixed point reproduces exactly.  All ten registered integer-cell
paths are periodic, and their rigid sampled barriers are positive.  After
allowing the six orientation/strain coordinates to relax locally, the
registered forward barriers lie in

```text
1.1302707069732617e-4 <= Delta_path <= 1.5793178714636528e-4.
```

The corresponding three-constituent production-dispersion speeds are

```text
0.012141242130712626 <= v_path <= 0.014350863870611992.
```

These are selected-path energy thresholds in lattice units.  They are not a
physical limiting speed and not a proof that every configuration-space path
has a barrier.

## 2. The important failure: the passive landscape is multibranch

The `z` paths converge to one local branch from both scan directions:

```text
maximum z-path hysteresis < 5.3e-18.
```

The `x/y` paths do not.  Forward and backward continuation settle into two
different admissible internal shapes, with maximum energy separation

```text
1.3431639117269861e-4.
```

That separation is comparable to the barrier itself.  The locked requirement
of a unique locally relaxed landscape therefore fails, and the preregistered
verdict is numerically unresolved.

This is nevertheless a concrete matter-dynamics result.  A centre coordinate
alone does not determine the compact object's quasistatic response.  Its
internal constituent configuration carries a branch/phase-like state that
changes which relaxation route is available.  The existing constituent phase
space already contains that information; this result does not force a new
primitive.  It does rule out reducing the object to a featureless charged
point with one scalar potential.

## 3. Correct cubic covariance

FTD-0613 compared three laboratory axes while holding one anisotropic body
fixed.  FTD-0614 instead applies

```text
R(x,y,z)=(y,z,x)
```

to the complete body, field, and launch direction.  The rotated energy curves
agree within `1.98e-17`.  Twelve forward/inverse common-action arms at speeds
`1/64` and `1/32` give a maximum rotated later-state residual
`1.78e-15`; every action gate and inverse gate passes.

Thus the large fixed-body axis difference in FTD-0613 is body orientation
anisotropy, not a failure of cyclic cubic covariance.

## 4. Relation to the launch data

The FTD-0613 `1/128` common launch has three-constituent kinetic budget
`1.4036961892471833e-4`, and `1/64` has
`5.617099425139216e-4`.  No qualified-mobile arm lies below its registered
forward-branch barrier, so the energy audit finds no contradiction.

But static path energy is not sufficient for motion.  Several nonqualified
FTD-0613 arms carry more energy than the selected barrier; the largest ratio
is `4.97`.  The missing ingredient is therefore dynamical: energy must enter
the correct internal/field channel and traverse the branch structure rather
than merely exist as common translational kinetic energy.

## 5. Ontological consequence

The strongest current compact-matter picture is now:

1. three ternary-polarity constituents plus a matched field form a stable
   selected rest pattern;
2. the pattern has exact reversible common-action dynamics;
3. its motion is controlled by a periodic lattice energy landscape;
4. its response depends on an internal configuration branch, not only its
   centre and total polarity;
5. high launches cross, while low launches can remain trapped even when a
   static energy budget appears sufficient.

The next discriminating experiment is an internal-mode launch from the same
rest state: inject zero-centre-momentum excitation into registered normal-mode
families and test whether a recurring internal phase transfers energy across
successive lattice saddles.  Separately, an extended carrier must show that
its energy-weighted ultraviolet content and depinning scale decrease with
width.  Neither result is currently derived.

