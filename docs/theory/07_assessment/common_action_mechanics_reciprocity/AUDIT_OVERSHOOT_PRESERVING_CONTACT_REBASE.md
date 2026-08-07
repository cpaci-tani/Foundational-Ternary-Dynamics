# AUDIT — Overshoot-preserving contact rebase

**Date:** 2026-07-25  
**Identifier:** `FTD-0527`  
**Status:** `[THEOREM — OVERSHOOT-PRESERVING IDENTICAL-CONTACT REBASE]` +
`[THEOREM — TWO-TO-ONE RAW PROJECTION]` +
`[CONSTRUCTIVE — ONE-BIT SINGLE-EVENT LIFT]` +
`[OPEN — QUOTIENT PRODUCTION/DISTINGUISHABLE COLLISION/FIELD ORIGIN]`  
**Verdict:**
`OVERSHOOT_REPAIR_CLOSES_PHYSICS_RAW_INVERSE_NEEDS_BRANCH_RECORD`  
**Pre-registration:**
[`PREREG_OVERSHOOT_PRESERVING_CONTACT_REBASE_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_OVERSHOOT_PRESERVING_CONTACT_REBASE_v1.md)  
**Run of record:** `engine/results/ftd_0527/windows_msvc_cpu.json`

## 1. Exact transaction

Let two identical carriers occupy adjacent charts with anchor difference
`d`, midpoint contact `c`, unit normal `n=d/|d|`, and opposing speed `v`.
Define

```text
N=ceil(|d|/(2v)),
delta=Nv-|d|/2 >= 0.
```

One tick before the occupied-target horizon, the crossing representative is

```text
(c+(N-1)v n, +v n), (c-(N-1)v n, -v n),
```

while the already-bounced representative is the same two phase points in the
opposite chart association. They are distinct raw states and the same
unlabelled physical state.

For the crossing representative, exchange the complete carrier records across
the two occupied charts and retain the normalized residual displacement. In
the order of the original anchors, the output is

```text
a1: remainder=-delta n, velocity=-v n,
a2: remainder=+delta n, velocity=+v n.
```

Ordinary free movement of the bounced representative gives exactly this same
raw output after bookkeeping identity is removed. Unlike frozen production,
the transaction never resets the remainder to zero.

## 2. Why density, current, and invariants close

The two input representatives contain the same unordered phase points, and
the two final worldline sets are identical. Compact trilinear density depends
only on those phase points. Exact face current depends only on the oriented
worldline 1-chains. Therefore both representations produce the same density
and current, including the edge/corner overshoot.

The paired exchange also preserves the velocity multiset
`{+v n,-v n}`. Total polarity, relativistic momentum, and matter energy are
unchanged. Every carrier displacement over the final tick has magnitude `v`,
so causality is inherited from `v<C_SPEED`. Reversing the output velocities
and applying one free inverse step recovers the physical input quotient.

The registered observer confirmed these identities over both polarities,
three translations, all 26 nonzero Moore directions, and speeds `1/8` and
`1/4`:

```text
registered arms                              312
commensurate arms                             72
positive-overshoot arms                      240
minimum raw-preimage residual                0.5
worst quotient phase residual                0
worst density residual                       0
worst face-current residual                  0
worst continuity residual                    2.9143354396410359e-15
worst common-output residual                 6.9388939039072284e-17
minimum positive overshoot                   0.0089745962155614034
maximum positive overshoot                   0.1339745962155614
worst invariant residual                     0
worst causal residual                        0
worst physical-reversal residual             0
worst translation-covariance residual        0
worst polarity-mirror residual               0
worst cubic-covariance residual              0
```

## 3. Exact raw-inverse boundary

The crossing and bounced inputs are two raw preimages of the same physical
output. Their minimum raw residual is `0.5`, while their quotient residual is
zero. The output difference is only the exchanged bookkeeping identities;
that difference has residual `1` and is not an intrinsic physical attribute.

Consequently the physical repair is necessarily two-to-one on the registered
raw chart histories:

```text
preimage multiplicity = 2,
minimum event record = ceil(log2(2)) = 1 bit.
```

One explicit branch bit reconstructs either preimage for this single event.
Repeated mergers require one additional retained bit per merger; a fixed
one-bit state does not solve indefinite reversal. This is the concrete contact
instance of the FTD-0499 finite-fiber theorem, not an exception to it.

## 4. Correct research statement

> Existing ternary sites plus continuous remainders are sufficient to perform
> an exact, local, cubic-covariant, face-current-compatible identical-contact
> rebase that preserves diagonal overshoot. They are not sufficient to retain
> both raw chart histories under an exactly invertible raw transition.

This closes the immediate FTD-0526 physical overshoot repair constructively.
It does not establish a production rule, a distinguishable-particle collision
law, a field-derived force, reciprocal mobile matter, or infrared particle
physics. FTD-0528 subsequently proves that unchanged native pre-movement
coupling distinguishes the two input representatives through `curl(sv)`;
the construction composes only with complete matched-history ordering unless
the native source or phase order is rewritten. FTD-0529 further proves that
the unchanged elastic output is only a kinematic repair: on edge/corner
contact it cannot pay arbitrary matched-field work without a simultaneous
field-dependent impulse or an explicit dressing ledger. The live architectural
alternatives are:

1. make downstream production factor through the physical chart quotient;
2. retain an indefinitely growing event history for exact raw reversal; or
3. change the raw transition/state representation.

No production code, default, toggle, scenario, force, collision rule, field,
normalization, ontology, or tolerance changed.

- checks: `7/7 PASS`;
- test SHA256:
  `645A49860B974642F59E4A17C09C801823E3468F00008F1576FF4782EE44F794`;
- header SHA256:
  `E0FF019D960DA483571980D82935CF7630B4302CCF5E8B7949C8BBE00F67975E`;
- implementation SHA256:
  `F19F725A5956B64BCE20A6200F2AC0620653BBAC0097C5A61BEF6DBE9D7F79C5`;
- locked preregistration SHA256:
  `F04A9832F68E57D3389A85C69F2133E769E281EF6C0DF51FFFAD6AD89183FD4D`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
