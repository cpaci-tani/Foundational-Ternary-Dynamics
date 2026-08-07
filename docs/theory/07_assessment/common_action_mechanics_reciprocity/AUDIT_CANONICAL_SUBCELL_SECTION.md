# AUDIT — Canonical subcell section

**Date:** 2026-07-25  
**Identifier:** `FTD-0500`  
**Status:** `[CONSTRUCTIVE — CENTERED SECTION OFF TIES]` +
`[THEOREM — HALF-CELL CUBIC OBSTRUCTION]` +
`[MEASURED — FINITE-PRECISION TIE NONREVERSAL]` +
`[CLOSED NEGATIVE — FROZEN PRODUCTION COMPATIBILITY]`  
**Verdict:** `CANONICAL_CHART_REQUIRES_RULE_REWRITE`  
**Pre-registration:**
[`PREREG_CANONICAL_SUBCELL_SECTION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_CANONICAL_SUBCELL_SECTION_v1.md)  
**Run of record:** `engine/results/ftd_0500/windows_msvc_cpu.json`

## 1. A centered section is constructive away from ties

The frozen candidate selects, componentwise,

```text
a(x)=floor(x+1/2),
r(x)=x-a(x) in [-1/2,1/2).
```

This is a single nearest-site chart for every real input under the chosen
half-open convention. On the registered 819-point grid it reproduces physical
position exactly. It is integer-translation covariant to `4.44e-16`, and all
48 signed cubic permutations commute exactly with it away from half-integer
tie planes.

Canonical translation also repairs the generic raw inverse: translate the
physical position, reselect its chart, and apply the negative displacement.
Away from ties the raw chart returns exactly in every registered arm.

## 2. Exact cubic covariance fails at the half-cell

Let `a(x)` be the integer anchor of any single-valued section satisfying
integer-translation covariance and inversion covariance. At `x=1/2`,

```text
a(-1/2)=a(1/2)-1                 translation,
a(-1/2)=-a(1/2)                 inversion.
```

Therefore

```text
2a(1/2)=1,
```

which has no integer solution. This is independent of the half-open convention.
The selected convention has exact anchor and remainder mismatches of one under
raw inversion at the tie, while the projected physical position still inverts
exactly.

In three dimensions each half-cell plane inherits the same obstruction, so an
exact `O_h`-covariant unique nearest-site section does not exist on the full
state space.

## 3. The pre-registered finite-precision reversal gate failed

The locked rational grid included reachable half-cell states. Of its forward
and reverse arms:

```text
tie raw-reversal failures       91
off-tie raw-reversal failures    0
worst raw residual               1
worst physical residual          1.11e-16.
```

At a tie, floating addition followed by subtraction can return on the opposite
side of the half-open boundary. Canonicalization then changes anchor by one and
remainder by one even though physical position is recovered. Thus the
pre-registered raw-reversal gate is negative. The executable's `14/14 PASS`
means it correctly detects and classifies this outcome; it does not relabel the
research gate as passed.

Exact rational or integer arithmetic could remove this floating ambiguity, but
that would be a new production representation and would not remove the exact
symmetry theorem in Section 2.

## 4. Face physics is indifferent to the canonical choice

For both polarities and all 128 overlapping start/end chart pairs, the
canonical chart gives the same trilinear polarity and cubical Whitney face
current:

```text
worst shape difference     0
worst current difference   0.
```

Therefore canonicalization neither improves nor damages the exact quotient
physics already proved by FTD-0498. Its defects concern manifested-anchor
physics.

## 5. Canonicalization moves the physical hop boundary

The production threshold retains an anchor until `|remainder|>=1`. A unique
centered chart changes anchor at `|remainder|=1/2`. For the locked path

```text
(8,0.49) + 0.02,
```

the existing update returns `(8,0.51)` while the canonical section returns
`(9,-0.49)`. Both project to `x=8.51`, but the canonical manifested hop occurs
half a lattice unit earlier.

This is not a coordinate-only alteration because production dynamics reads the
anchor.

## 6. Native source and collision behavior changes

The two equivalent endpoint charts have primitive ternary L1 difference `2`.
Both execute the exact native source rule

```text
Delta wave_vel=-G_C grad(s)
```

with zero formula residual, yet their source responses differ by
`0.042712271551427185`.

Under the locked incoming-probe collision, one anchor placement causes a
same-sign bounce and the other permits motion into the empty site. The probe
velocity outcomes differ by exactly `0.4`.

Hence a canonical section changes manifestation, field sourcing, collision
timing, and update ordering even while physical position and face current stay
fixed.

## 7. Consequence

Canonicalization is a viable ingredient of a new matter ontology, not a
drop-in repair of the current one. To adopt it, FTD must explicitly rewrite
the hop boundary and every anchor-consuming source, collision, reaction, and
ordering rule. Exact cubic covariance at tie states must either be relaxed,
represented on a quotient, or supplied with additional boundary structure.

The frozen face-mobile branch therefore remains closed. After FTD-0499 and
FTD-0500, neither fixed finite chart memory nor a unique canonical chart
preserves the current production dynamics. The live clean route is to test a
shape/quotient ontology as a genuinely new dynamics, not to call it a gauge
rewrite of the existing engine.

## 8. Reproducibility

- implementation checks: `14/14 PASS`;
- pre-registered raw reversal gate: `FAIL` at 91 tie arms;
- test SHA256:
  `A045BF7A69E23A8231C9F66B53AEF65C55D0147CE770DA3C7386FBD060A375AC`;
- header SHA256:
  `8DBA6784C6B0D61B5A78430EB6A5949F215AFCD1C635B67BAF05F2B94595B42F`;
- implementation SHA256:
  `248B0F6309EBF9B0324E61E63592422F7509DC406F611D271E3EC114F35E89FD`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
