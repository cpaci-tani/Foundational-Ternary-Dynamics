# Audit — Native Moving-Source Pole Correction

**Record:** FTD-0558
**Date:** 2026-07-26
**Verdict:** `NATIVE_MOVING_SOURCE_POLE_CORRECTED`
**Production changes:** none

## Defect corrected

FTD-0115/0120 combined the wrong temporal discretization, wrong spatial
stencil, and an unwrapped Fourier label.  That combination created the false
claim that production lattice phase speed becomes arbitrarily small and every
nonzero source speed radiates.  Q6 then promoted a near-pole counting proxy to
an energy-loss formula it did not compute.

## Locked run

The preregistered MSVC 14.44 CPU observer passed:

| Diagnostic | Result |
|---|---:|
| driven response arms | 12 |
| wrapped threshold arms | 9 |
| Floquet schedule arms | 12 |
| seven-point ratio floor | 0.636619772368 |
| universal production speed floor | 0.212206590789 |
| minimum enumerated principal-direction speed | 0.226221165291 |
| alias symbol residual | 2.22044604925e-16 |
| alias phase residual | 1.66533453694e-16 |
| wrapped/old alias ratio | 15.0 |
| maximum non-fundamental Floquet amplitude | 0.218387780547 |
| maximum identity residual | 7.10542735760e-14 |
| test failures | 0 |

## Status corrections

- FTD-0115's former uniform-velocity formula and any-speed Cherenkov claim are
  retracted.  The replacement is a derived selected-drive resolvent plus a
  theorem-grade positive speed floor.
- FTD-0120 Q6 is retracted; radiation power remains open.
- FTD-0120 Q5 is not a production derivation; production Larmor power remains
  open.
- Q7 survives only as conditional linear form-factor algebra.
- Q8 exact continuity is restricted to routed movement histories, not all
  reaction events.
- FTD-0113's continuous-time seven-point identity is auxiliary.  Its native
  replacement is the zero-frequency production resolvent `1/(C_WAVE^2 M)` in
  the Abel/static sense.

## What remains open

A physical moving-source calculation still needs a stable manifested mobile
configuration, exact conserved deposited current, the source numerator,
retarded field-energy flux, and reciprocal recoil from the same interaction.
The integer-hop Floquet comb is a diagnostic of a prescribed history, not yet
a particle-radiation prediction.
