# AUDIT — Ternary collision vertex

**Date:** 2026-07-25  
**Identifier:** `FTD-0504`  
**Status:** `[THEOREM — SAME-SIGN TERNARY CAPACITY BOUND]` +
`[THEOREM — IDENTICAL INTERIOR-CROSSING QUOTIENT]` +
`[CLOSED NEGATIVE — CONSERVATION-ONLY 3D COLLISION LAW]` +
`[OPEN — DISTINGUISHABLE/BOUNDARY COLLISION VERTEX]`  
**Verdict:** `IDENTICAL_INTERIOR_CROSSING_IS_PERMUTATION_GAUGE`  
**Pre-registration:**
[`PREREG_TERNARY_COLLISION_VERTEX_v1.md`](../10_eft_program/preregistrations/PREREG_TERNARY_COLLISION_VERTEX_v1.md)  
**Run of record:** `engine/results/ftd_0504/windows_msvc_cpu.json`

**Successor correction (FTD-0507):** the capacity formula below is exact for
`m` carriers forced into one site. Its former inference about every stored
tick-boundary collision is retracted. Non-knot effective positions have
multiple stable `(site,remainder)` charts, and production retains their
distinct anchors as real ternary storage slots.

## 1. A transient vertex is not a manifested snapshot

The ternary capacity obstruction applies when multiplicity must be written at
a tick, not whenever worldlines meet between ticks. For `m>=2` coincident
same-sign unit carriers, one site would have to store charge `q=+m` or `q=-m`.
Since the primitive state is `s in {-1,0,+1}`, the exact best-case defect is

```text
min_{s in {-1,0,+1}} |sign*m-s| = m-1.
```

This was checked for both signs and `m=2,...,8` (14 arms). A transient event
vertex, by contrast, is part of the oriented worldline 1-chain and need not be
materialized as a stored intermediate site.

## 2. Identical pass-through and bounce are the same physical event

Two same-sign equal-mass carriers start at

```text
x_L=c-a*n,  p_L=+p*n,
x_R=c+a*n,  p_R=-p*n,
a=0.25, v=0.40, dt=1,
n=(1,2,3)/sqrt(14).
```

They meet at `tau=a/v=0.625` and have `0.375` tick remaining. For
pass-through, the carrier from the left retains `+p*n`; for an elastic bounce,
the two equal carriers exchange momenta at the common vertex. The resulting
unlabeled endpoint phase-space multisets are exactly the same:

```text
{(c+b*n,+p*n), (c-b*n,-p*n)},  b=v(dt-tau).
```

The two descriptions differ only in which bookkeeping label is attached to
each outgoing segment. No `particle_id` is a physical datum in this quotient.

## 3. The exact face current also quotients

The comparison was not made from endpoints alone. Each pass-through segment
and each two-piece bounce polyline was deposited with the analytic FTD-0478
face current. The incoming/outgoing overlap cancels as an oriented 1-chain, so
both histories have the same charge endpoints and the same link current.

Across all 48 signed cubic maps and three integer translations:

```text
transformed arms                    144
worst phase-space residual           0
worst face-current residual           2.42861286636753e-16
worst continuity residual             4.85722573273506e-17
worst energy/momentum/charge/causal   0
worst full reversal residual          1.77635683940025e-15.
```

Therefore an interior crossing of physically identical carriers does not
require a pass-versus-bounce primitive. Those words name two labeled
representatives of one unlabeled phase-space/current event.

## 4. Exact scope of the quotient

The quotient is lost when any transported intrinsic attribute differs. The
observer separately changes polarity, spin, color, flavor, and an additional
physical tag. Each change makes the two attribute worldlines distinguishable,
even though the untagged charge current can remain equal. A bookkeeping
identifier alone is deliberately excluded from this discriminator.

This result does not establish spin, color, or flavor dynamics in FTD. Those
fields appear only as generic transported labels defining the theorem's
distinguishability ceiling.

## 5. Tick-boundary coincidence remains impossible in one canonical site

At `v=0.25`, the collision time is exactly `tau=1`. Both same-sign carriers
then occupy the common endpoint at the stored snapshot, with no remaining
subtick interval on which label quotienting can separate the endpoints. The
observer returns

```text
TERNARY_ENDPOINT_OVERLOAD
minimum charge defect = 1
```

and does not infer bounce, annihilation, delayed motion, or multi-occupancy.
Thus the single-anchor snapshot-capacity obstruction is exact. FTD-0507 later
showed that it does not cover a boundary point represented by two distinct raw
charts; the unconditional boundary inference is superseded.

## 6. Conservation cannot supply a three-dimensional collision law

In the center-of-momentum frame, every pair

```text
(p',-p'),  |p'|=|p|
```

has total momentum zero and the same production-dispersion energy. Five
distinct axes were registered: x, y, z, a face diagonal, and a body diagonal.
All have zero momentum and energy residual, with minimum direction separation
`0.6058108930553725`.

Consequently charge, energy, momentum, causality, and reversibility do not
select a 3D scattering angle. A central interaction, impact parameter,
internal-attribute rule, or another local collision mechanism is additional
dynamics. Conservation-only collision recovery is closed negative.

## 7. Consequence

The event-native mainline can treat physically identical crossings as
permutation gauge. A canonical site still cannot write same-sign multiplicity,
but retained non-knot charts can (FTD-0507). Distinguishable crossings and
nontrivial scattering still require an explicit collision vertex. This
observer supplies neither and changes no production state, toggle, default, or
scenario.

- checks: `10/10 PASS`;
- test SHA256:
  `649ECBC66A1FAAD9ED01F71F953965BE9D1668040D41722712DC4A066F55DE1F`;
- header SHA256:
  `E46C42E89D61A9C897E7246E83AA95CD511BC831411C31BA736E1F53E6CE4D97`;
- implementation SHA256:
  `6CA470A788FA209839B0C2A4DB8AFE7350A57FCC2DE54B9FAAB55B87C345DB4A`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
