# AUDIT — Nested source-history translation

**Identifier:** `FTD-0464`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — FINITE-SUPPORT ENERGY DECOMPOSITION]` +
`[CONSTRUCTIVE EXISTENCE — ENERGY/KINEMATICS ONLY]` +
`[MEASURED — DRESSING INDEPENDENCE]` +
`[CLOSED NEGATIVE — PHYSICAL EVENT MAP, FTD-0465]`  
**Run of record:** `engine/results/ftd_0464/windows_msvc_cpu.csv`

## Result

A fixed local part of the polarity-generated history is sufficient to make
every registered one-step event kinematically admissible. The result does not
depend on the selected initial longitudinal dressing. The locked verdict is

`LOCAL_TRANSLATION_SUFFICIENT_DRESSING_INDEPENDENT`.

Every one of the eight `dressing={off,on}` by
`radius={R1,R2,R3,global}` arms passes `42/42`. The event-difference norm
outside the registered support is exactly zero in every arm, and the exact
energy decomposition closes below `3.80e-18`.

## Smallest registered support

`R=1` translates the 27 source-history sites in the source-centered Moore cube
one face step. The complete field-event support is the 36-site union of the
old and new cubes. Its results are:

| Initial dressing | Valid | Required-work RMS | Maximum absolute work | Mean moved history norm |
|---|---:|---:|---:|---:|
| off | 42/42 | `3.01028e-4` | `6.28384e-4` | `41.10%` |
| on | 42/42 | `3.28663e-4` | `6.68476e-4` | `41.10%` |

The moved fraction varies from `29.72%` to `66.75%` over the attempt times.
Thus no fixed fraction of the total history is being relabeled as matter; a
fixed geometric neighborhood is sufficient despite the source history's
continued outward propagation.

## Energy structure

For every arm, the finite event obeys

`Delta H_event = Delta H_self + Delta X - W_external`.

At `R=1`, the isolated partial-translation self change has RMS
`3.01290e-4`. With the initial dressing off, packet/source cross-energy change
has RMS only `7.25825e-6`; with dressing on, the combined cross term is
`1.05351e-4`. The partial translation's own boundary/self-energy change is
therefore the leading term in the smallest-support event.

The global dressing-on control reproduces FTD-0462's required-work RMS to
`2.98e-19`. The global dressing-off arm also passes `42/42`, with required-work
RMS `1.27759e-5`. This corrects a possible overreading of FTD-0463: the seeded
dressing dominates the cross-energy magnitude when present, but is not needed
for kinematic recovery. Moving the source-generated history is the operative
change.

## Ontological consequence

FTD-0462 showed that translating all source history was nonlocal. FTD-0464
shows that this nonlocality is not required for a single admissible event. A
finite endpoint-local field coat can be translated while leaving the distant
history untouched.

That is a constructive opening for a composite object

`manifested polarity + local source-generated field coat`.

It is not yet an emergent particle. The `R=1` cube is imposed geometrically;
the production tick does not select it, no equation separates bound field from
radiation, and the event has not been repeated after its own field update. The
translated block can also create a sharp boundary between moved and unmoved
history. A single admissible virtual event does not prove stable transport.

FTD-0465 closes promotion of this specific map negative. The partial additive
translation has an exact 54-dimensional-or-larger kernel at `R=1` and fails
field/particle momentum closure in all 84 dressing-off/on measurements. The
local-support result is retained only as an energy/kinematics existence bound.

## Next gate

Do not run this noninjective map sequentially. Test an injective local
permutation control against simultaneous energy, particle-kinematic, and
field-momentum closure before any repeated transport campaign.
