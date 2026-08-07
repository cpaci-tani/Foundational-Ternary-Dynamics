# AUDIT — Finite-memory reversible lift

**Date:** 2026-07-25  
**Identifier:** `FTD-0499`  
**Status:** `[THEOREM — FINITE-FIBER REVERSIBLE-LIFT OBSTRUCTION]` +
`[CONSTRUCTIVE — UNBOUNDED HISTORY CONTROL]` +
`[CLOSED NEGATIVE — FINITE LOCAL CHART MEMORY]`  
**Verdict:** `UNBOUNDED_HISTORY_REQUIRED_FOR_FROZEN_PROJECTION`  
**Pre-registration:**
[`PREREG_FINITE_MEMORY_REVERSIBLE_LIFT_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_FINITE_MEMORY_REVERSIBLE_LIFT_v1.md)  
**Run of record:** `engine/results/ftd_0499/windows_msvc_cpu.json`

## 1. The finite-memory obstruction is exact

Let the frozen raw threshold map `f:S->S` have `m>=2` preimages of one output
`t`, and let `H` be any nonempty finite hidden-state set. A hidden-state lift
that leaves the projected raw update unchanged must satisfy

```text
pr_S F(s,h)=f(s).
```

The `m|H|` states in the colliding fibers map into only the `|H|` states over
`t`. Hence no such lift is injective. Its exact local cardinality deficit is

```text
(m-1)|H|.
```

This proof is independent of the numerical size of `H`. The executable checks
the exact count for binary and eight-way collisions at every power-of-two
capacity from `1` through `2^20`; those checks exercise the implementation but
do not delimit the theorem.

## 2. A fixed finite chart label cannot repair repeated hops

One extra bit can distinguish one binary merge only if its prior contents are
not also required. At the next merge, both the new branch and the old record
must survive. Repeating the pigeonhole argument therefore rules out every
fixed finite local memory, including a boolean chart bit, finite enum, finite
integer, or IEEE-754 payload, while the raw projection remains many-to-one.

The information lower bound after `N` independent `m`-way merges is

```text
N log2(m) bits.
```

It grows without bound with trajectory length.

## 3. An unbounded stack is a constructive existence control

The registered control pushes a branch digit by

```text
h' = m h + b,
b in {0,...,m-1},
```

and reverses it by quotient and remainder. The executable exactly reverses 63
binary merges and 21 eight-way merges in the available 63 payload bits. This
does not evade the theorem: it demonstrates that reversibility is restored
only for as many events as the allocated history can hold. An indefinitely
running system requires an indefinitely growing record or an environment into
which the record is exported without erasure.

## 4. Current face-side variables do not carry the missing branch

The explicit FTD-0497 preimages

```text
(site_x=8, remainder_x=+0.875),
(site_x=9, remainder_x=-0.125)
```

have the same effective position. Under one common axial transaction the
lower chart hops, the upper chart does not, and both reach the same projected
raw output. The run of record finds:

```text
shape branch difference       0
current branch difference     5.55e-17
field branch difference       0
momentum branch difference    0
work branch difference        0
dressing branch difference    0.
```

Thus the trilinear polarity, exact face current, updated face field, matter
momentum, scalar work, and the FTD-0495 dressing coordinate are quotient
variables. None retains which manifested anchor entered the merge.

## 5. Infinite-real encodings are not an existing escape

As a set-theoretic construction, one exact real number could encode an
unbounded binary history in successively finer digits. The engine contains
finite-precision numbers, not exact reals, and the registered dressing update
stores accumulated work rather than chart digits. A hidden digit encoding
would also be a new dynamically protected history primitive, not a consequence
of the current face action.

Therefore the existence of mathematical real encodings does not change the
frozen-engine result.

## 6. Minimum repair trilemma

Exact reciprocity can be recovered only by changing at least one frozen
condition:

1. **quotient the ontology:** make effective position/distributed shape the
   state and remove anchor-dependent production behavior;
2. **change the raw transition:** adopt a one-to-one canonical position chart
   rather than the current overlapping ±1 threshold map;
3. **retain/export history:** attach a record whose capacity grows with every
   many-to-one event.

A fixed local chart tag while preserving the present raw update is not a
fourth option.

## 7. Plan consequence and reproducibility

FTD-0499 closes the `history-ontic` repair for every fixed finite added state.
It does not choose between a quotient rewrite, a canonical-chart transition,
or an open-system environmental record. Consequently FTD-0481 remains closed
for the frozen ontology and tick.

- checks: `11/11 PASS`;
- test SHA256:
  `57B2D6321F51D94D94040477CBB0465A2EFC7BEA1E85BC0148463A072B06ED32`;
- header SHA256:
  `D593C991597A69DEF1BE389CB69DEE3168F44B1B774FBBBE7D6B30C59D92B092`;
- implementation SHA256:
  `13E2C4E8F4777C38C9AA01260E44A0D823DC89E89E92DA58C3BC5704ED9E5265`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
