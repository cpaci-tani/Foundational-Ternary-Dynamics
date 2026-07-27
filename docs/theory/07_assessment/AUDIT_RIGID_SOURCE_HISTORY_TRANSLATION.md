# AUDIT — Rigid source-history translation

**Identifier:** `FTD-0462`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — PERIODIC TRANSLATION IDENTITY]` +
`[MEASURED — FULL KINEMATIC RECOVERY]` +
`[CLOSED NEGATIVE — LOCAL EVENT REALIZATION]`  
**Run of record:** `engine/results/ftd_0462/windows_msvc_cpu.csv`

## Result

Translating the complete polarity-generated `J/W` history with the polarity
removes the FTD-0459 kinematic barrier at every registered attempt, but the
instantaneous event is nonlocal. The locked verdict is:

`RIGID_SOURCE_HISTORY_TRANSLATION_FULL_RECOVERY_NONLOCAL_EVENT_SUPPORT`

Across the 42 attempts:

- production partial carry admits `12/42` particle updates;
- rigid source-history translation admits `42/42`;
- required-work RMS falls from `0.008462839477058743` to
  `0.00023156579861414742`, a factor `36.55` (`97.26%` reduction);
- squared event-difference norm outside R1 ranges from `0.12233` to `0.70617`;
- all full endpoint works reproduce FTD-0459 exactly.

The recovery is therefore diagnostic, not an admissible local rule.

## Exact energy split

Periodic translation preserves the isolated source-history observer energy.
The worst measured residual is `1.179611963664229e-16`. With external history
held fixed, the full rigid-event change obeys

`Delta H_rigid = Delta X_wave-W_external`,

where `X_wave` is the quadratic wave cross energy between external and
source-generated histories. The worst identity residual is
`1.163891032163189e-16`.

The external scalar endpoint work has RMS `8.97637731327757e-5`. The cross
energy change has RMS `0.000237185510078264`, with every sampled value positive
between `7.2420e-5` and `3.8345e-4`. Thus the dressed event is controlled more
strongly by field interference than by the external divergence endpoint term.

FTD-0460 established that the transverse packet itself has essentially zero
direct scalar endpoint work. FTD-0462 shows that a divergence-free external
history can nevertheless affect a dressed manifestation through quadratic
cross energy. It does not yet establish whether the packet or the small
longitudinal dressing dominates that cross term; they must be separated next.

## Locality failure

Rigid translation changes every occupied part of the source-generated history
at the same event. Already at tick 6, `12.23%` of the event difference norm lies
outside the 36-site R1 support. The fraction rises above `70%` at later ticks.
This is not a local hop followed by causal propagation. It is a synchronized
translation of an extended history triggered by one site event.

The source history also contains the response to abrupt source creation. It
may mix a bound near field with outgoing or periodic radiation. Moving all of
it is therefore physically overinclusive even apart from event locality.

## Ontological consequence

The self-barrier is not intrinsic to polarity. It is produced by changing the
polarity location while leaving most of its generated field referenced to the
old site. Carrying the full generated history restores a small, admissible
energy change, but does so by assuming the answer: that the entire history is
part of the object.

The viable middle ground must be a dynamically identified localized dressing:
large enough to remove the self-barrier, small enough to update locally, and
separated from radiation by an equation-derived criterion rather than a fitted
radius.

## Next gate

First decompose `Delta X_wave` exactly into packet-source and dressing-source
cross terms. Then test fixed nested translation supports `R=1,2,3` without
optimizing the radius. This establishes whether the transverse packet is the
dominant guide and whether any causal-sized part of the source history is
sufficient for recovery.
