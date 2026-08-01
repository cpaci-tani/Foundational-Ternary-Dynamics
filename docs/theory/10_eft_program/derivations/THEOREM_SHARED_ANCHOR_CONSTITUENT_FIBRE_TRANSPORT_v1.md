# FTD-0609 — Shared-anchor constituent-fibre transport v1

**Status:** `[SELECTED ONTIC EXTENSION — DEFAULT OFF]` +
`[MEASURED — REVERSIBLE SHARED-FIBRE TRANSPORT]` +
`[CONSTRUCTIVE — v=1/32 ARM]` +
`[CLOSED NEGATIVE — LOCKED TWO-VELOCITY CONJUNCTION]`
**Protocol:**
[`PREREG_SHARED_ANCHOR_CONSTITUENT_FIBRE_TRANSPORT_v1.md`](../preregistrations/PREREG_SHARED_ANCHOR_CONSTITUENT_FIBRE_TRANSPORT_v1.md),
prefix SHA-256 `8CA3984F9E3FF2B8BE53BBBEA20028618EACFFC54C1B361994D10AD8B95D4D95`
**Production status:** unchanged; option defaults false

## 1. The priced extension

The integer anchor is treated as a coordinate-chart label for a constituent's
continuous effective position. At most two distinct constituent records may
share that label. Primitive ternary `s` is unchanged and is not asked to store
the two records; their compact polarity coats and face currents superpose in
the derived coupling representation.

No action, force, current, field, binding, energy, solver tolerance, initial
state, or inverse equation changes. The strict default-false branch still
fails at ticks `4` and `2`, reproducing FTD-0608.

## 2. Algebraic and reversible result

With the fibre option enabled, both histories cover every registered step:

| velocity | forward/reverse | hops | fibre states | max multiplicity | energy drift | recovery |
|---|---:|---:|---:|---:|---:|---:|
| `1/64` | `128/128` | 22 | 22 | 2 | `1.11e-15` | `5.11e-13` |
| `1/32` | `64/64` | 15 | 36 | 2 | `4.44e-16` | `8.88e-15` |

Every common-action gate stays below `1.21e-14`; integer translation
covariance is exact. The minimum distance between any two constituent records
is greater than `1.4139`, so shared anchor does not mean coincident effective
position. Every shared state is internal to one trimer, never between the two
finite-volume neutralizing partners; the first shared pair is records `(3,5)`.

This establishes that a two-record local chart fibre is sufficient to remove
the FTD-0608 representation failure while retaining exact current, energy,
and state-only inversion for the complete histories.

## 3. Physical transport result

The faster arm passes every registered physical gate: longitudinal
displacement `1.8784` for nominal `2`, transverse drift `0.0270`, and maximum
pair-separation change `0.1505`.

The slower arm does not. Its longitudinal displacement is only `0.2833`, and
the separation of the two distant trimers changes by `1.0626`, beyond the
locked `0.25` gate. Its common action, internal trimer geometry, energy, fibre
regularity, and state-only inverse all pass. Phase 15 also has an outward
one-step intertrimer impulse in the earlier FTD-0606 record, so this arm is
confounded by the dynamics of the finite-volume neutralizing partner; the
record does not prove that interaction is the sole cause.

The locked conjunction therefore yields

```text
SHARED_ANCHOR_FIBRE_TRANSPORT_CLOSED_NEGATIVE
```

## 4. Ontological consequence

The local fibre is a constructive representation repair, not a complete
matter theory. It supports the interpretation that a material core contains
multiple continuous internal degrees of freedom whose site anchors may alias;
ternary `s` is an aggregate manifestation channel, not a container slot for
each constituent.

The next discriminator must separate the mobility of one charged trimer from
the force exerted by the distant object used to neutralize the periodic box.
Adding an intertrimer binding primitive now would be premature because the two
trimers were introduced as neutralizing partners, not derived as one particle.

