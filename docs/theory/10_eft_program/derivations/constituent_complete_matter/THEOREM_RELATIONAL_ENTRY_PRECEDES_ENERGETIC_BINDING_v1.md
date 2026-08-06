# FTD-0738 — Relational entry precedes energetic binding

**Status:** `[THEOREM FOR THE SELECTED COMPACT PAIR DYNAMICS + CERTIFIED NUMERICAL INSTANCE]`  
**Date:** 2026-07-29  
**Depends on:** FTD-0721 selected `DerivedCompactPair` interaction and the
FTD-0551/0722 common matter–current–field energy identity  
**Does not depend on:** FTD-0736/0737 numerical outcomes for the theorem

## 1. Definitions

Write squared constituent separation as `d=r^2`, well depth as `D>0`, and
the relational cutoff as

```text
d_c = 3/2.
```

The selected compact-pair potential is

```text
U(d) = -16 D (d-3/2)^2 (d-3/4),   0 <= d < 3/2,
U(d) = 0,                           d >= 3/2.
```

The relational graph contains the pair exactly when `d<3/2`. Its internal
energy is

```text
E_pair = K + U(d),
K = sum_a [sqrt(E_REST^2+C_SPEED^2 |p_a|^2)-E_REST].
```

`K>=0`, with equality exactly when every constituent momentum vanishes.

## 2. Boundary lemma

At the graph boundary,

```text
U(3/2)=0,
U'(3/2)=0.
```

Indeed, the interior polynomial is

```text
U(d)/D = -16d^3 + 60d^2 - 72d + 27,
```

whose value and first derivative vanish at `d=3/2`. The squared cutoff factor
makes graph activation force-continuous. It also makes binding energy grow
only quadratically immediately inside the graph.

The potential is negative exactly on

```text
3/4 < d < 3/2,
```

and has its unique interior minimum `U(1)=-D` at `d=1`.

## 3. Entry theorem

Consider a continuous subcell trajectory crossing from `d>=3/2` to
`d<3/2`. If at least one constituent has nonzero momentum at the crossing,
then

```text
E_pair(crossing) = K(crossing) > 0.
```

Because both the relativistic lattice kinetic energy and `U(d)` are
continuous along the subcell segment, there is a nonzero trajectory interval
immediately after entry on which `E_pair>0`.

Therefore:

> generic relational-graph entry is not energetic binding. A moving pair
> activates the selected interaction before it can enter the negative-energy
> sector.

This proves that the strict FTD-0736 classifier—negative energy beginning at
the exact graph-entry sample—tested a stronger conjunction than formation
requires. Its negative verdict remains valid for that conjunction, but it is
not a release theorem.

For a finite discrete tick, the first stored inside state can in principle
land beyond the positive interval; the theorem orders the continuous
subcell transaction, not every possible coarse sampling of it.

## 4. Receiver corollary

In the reaction-free selected common action,

```text
Delta E_pair + Delta E_field = 0
```

up to the registered numerical residual. If a history changes from
`E_pair>0` to `E_pair<0`, then

```text
Delta E_field = E_pair(before)-E_pair(after) > 0.
```

Thus field-energy reception is not decorative morphology. It is required by
the complete energy ledger for capture in this closed two-sector model.

## 5. FTD-0736/0737 instance

The certified `L=129` histories instantiate the theorem before periodic
self-contact:

| ray | graph re-entry | `E_pair` at re-entry | continuous-negative onset | delay |
|---|---:|---:|---:|---:|
| `<001>` | 63 | `+4.81810e-5` | 78 | 15 |
| `<01-1>` | 79 | `+6.41234e-5` | 94 | 15 |
| `<111>` | 96 | `+7.11121e-5` | 111 | 15 |

All three tails remain graph-inside and negative through tick 122, while the
field gains `4.116e-4--1.170e-3`. Exact current support bounds possible
periodic self-contact to tick 123 or later. The common 15-tick delay is a
`[NUMERICAL FACT]` predicted by FTD-0737 and independently reproduced; this
theorem explains why a delay must generically exist, not why its value is 15.

## 6. Ontological consequence

The selected interaction graph is an **interaction-support relation**, not
an object predicate. Within the tested ontology, three layers must be kept
distinct:

1. **encounter:** constituents approach while graph-disconnected;
2. **relational entry:** the local interaction becomes active;
3. **energetic formation:** current-mediated field transfer leaves a
   graph-connected negative internal core.

Matter identity therefore cannot be assigned at first contact or to either
site alone. The smallest presently justified candidate is the complete
history comprising constituents, their event current, the active relational
core, and the receiving face/edge field.

## 7. Scope

The theorem is exact for the selected compact potential and selected
two-sector common action. The five FTD postulates do not force that potential,
its depth, cutoff, or the measured 15-tick delay. Nothing here establishes an
invariant basin, asymptotic localization, an uncontained solution, a physical
particle, mass, charge, spin, or quantum statistics.
