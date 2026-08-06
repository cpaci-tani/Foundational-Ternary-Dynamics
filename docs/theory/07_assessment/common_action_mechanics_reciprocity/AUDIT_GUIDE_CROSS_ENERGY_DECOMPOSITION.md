# AUDIT — Guide cross-energy decomposition

**Identifier:** `FTD-0463`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — QUADRATIC CROSS-ENERGY ADDITIVITY]` +
`[MEASURED — DRESSING-SOURCE DOMINANCE]` +
`[CLOSED NEGATIVE — REGISTERED PACKET AS DOMINANT GUIDE]`  
**Run of record:** `engine/results/ftd_0463/windows_msvc_cpu.csv`

## Result

The transverse packet is not the dominant interference channel behind
FTD-0462's rigid dressed-event recovery. The locked verdict is

`DRESSING_SOURCE_CROSS_DOMINATES`.

Across the 42 registered event times, the cross-energy-change RMS values are

- packet-source: `1.2775930643866692e-5`;
- initial-dressing-source: `2.3856515736440676e-4`;
- combined-external-source: `2.3718551007826405e-4`.

The dressing-source term is `18.6730x` the packet-source term. The exact
bilinear decomposition closes to `3.59e-16`, endpoint-work additivity closes
to `2.71e-20`, and the reconstructed rigid required-work series reproduces
FTD-0462 to `1.13e-16`.

## Exact statement

For the registered quadratic wave observer,

`X(A,S)=H_wave(A+S)-H_wave(A)-H_wave(S)`

is bilinear in the two histories. Therefore, with
`A=packet+dressing`,

`Delta X(A,S)=Delta X(packet,S)+Delta X(dressing,S)`.

The measured residual is numerical roundoff, not a fitted relation. This
identity permits an exact attribution of FTD-0462's external-source
interference for the frozen component split.

## Physical consequence

FTD-0457 established that the finite transverse packet can supply local recoil
capacity in a counterfactual constrained transaction. FTD-0460 then showed
that it supplies essentially no direct longitudinal endpoint work. FTD-0463
now shows that it is also subdominant in the indirect cross-energy channel
of the registered global dressing-on event.

The dominant component is the one-time longitudinal dressing of amplitude
`1e-4`. That dressing is a selected initial condition. It was not created by
the manifested state, isolated as a stable bound mode, or shown to regenerate
after motion. Consequently the registered construction does not establish the
transverse packet as a pilot wave, and its recovered motion is not native
emergence.

This does not prove that transverse radiation can never guide matter. It
closes the narrower claim for the frozen packet, phase, amplitude, dressing,
and coupling used by FTD-0459 through FTD-0463. No parameter scan is licensed
by this result.

FTD-0464 subsequently established that both local and global translated-
history events remain kinematically admissible with this initial dressing
removed. Dressing dominance in the FTD-0463 cross term is therefore not a
claim of dressing necessity.

## Ontological consequence

The present data separate three roles that had been visually conflated:

1. the ternary polarity is the localized manifestation anchor;
2. the accumulated longitudinal source history creates the dominant static
   self-barrier;
3. the selected longitudinal initial dressing dominates the compensating
   interference when that history is translated.

The transverse packet is presently a recoil-capacity reservoir, not a
demonstrated trajectory selector. A physical particle candidate therefore
requires a native, localized, dynamically maintained longitudinal dressing.
Calling an arbitrary externally supplied flux packet a pilot wave is not
supported by the engine record.

## Next gate

Translate fixed nested pieces of the source history at `R=1,2,3` and the
global support, with the initial dressing reported separately on and off. The
radii are fixed before execution and are not optimized. This will distinguish
a local bound near field from the nonlocal translation of radiative history
and determine whether the selected initial dressing is essential to every
apparent recovery.
