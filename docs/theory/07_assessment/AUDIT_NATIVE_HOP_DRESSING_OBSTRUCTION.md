# FTD-0560 Audit — Native Periodic-Hop Dressing Obstruction

**Status:** [THEOREM — POINT-HOP LINEAR CARRIER CLOSED NEGATIVE; GENERAL NONLINEAR CARRIER OPEN]
**Date:** 2026-07-26
**Verdict:** `POINT_HOP_DRESSING_OBSTRUCTED`
**Production changes:** none

## Result

Every finite one-site hop period has a regular intersection between one of
its exact Floquet harmonics and the production wave band.  At that
intersection the native state-gradient source is nonzero.  The curl source
cannot cancel it because the two pieces are longitudinal/transverse
orthogonal.

The resulting Fourier resolvent has a nonintegrable pole.  A periodically
hopping point polarity therefore cannot carry an exactly co-moving,
square-summable linear native dressing on the infinite lattice.

## Locked campaign

| Diagnostic | Result |
|---|---:|
| period/axis/polarity arms | 96/96 |
| maximum pole residual | 4.44089209850e-16 |
| minimum regularity derivative | 9.39272238614e-2 |
| maximum source-orthogonality residual | 8.67361737988e-19 |
| maximum `sqrt(3)/T` coefficient residual | 2.22044604925e-16 |
| minimum effective forcing divided by `G_C` | 6.28777131333e-2 |
| maximum polarity-mirror residual | 0 |
| maximum cubic-covariance residual | 7.77156117238e-16 |
| resonant coefficient bound failures | 0 |

The independent Python proof reproduces the same root, coefficient,
orthogonality, covariance, and resonant-response gates.

## Physical reading

The result separates two statements that were previously easy to conflate:

1. the mean hop speed tends to zero as `1/T`;
2. the discontinuous schedule retains a resonant harmonic at every finite
   `T`.

Its effective amplitude falls as `6pi G_C/T^2`, so the obstruction weakens
rapidly but does not disappear exactly.  FTD-0561 subsequently proves that a
finite rigid profile with the same nonzero net polarity retains the universal
leading `T^-2` term, and FTD-0562 proves that no fixed nonzero finite neutral
form factor cancels the complete slow-hop surface.  A stable carrier must
therefore alter or deform its source dynamically, alter the spectrum
nonlinearly, grow with the hop scale, or realize a self-consistent
defect/topological transaction rather than merely tuning a rigid shape.

## Scope consequence

| Claim | Status after FTD-0560 |
|---|---|
| Single periodic point polarity has a finite-energy exactly co-moving linear dressing | [CLOSED NEGATIVE] |
| Every finite point-hop period carries a nonzero resonant native harmonic | [THEOREM] |
| Slow-hop resonant forcing scales as `T^-2` | [ASYMPTOTIC THEOREM] |
| Every finite periodic engine volume grows secularly | [NOT CLAIMED] |
| Every fixed nonzero finite rigid linear carrier is impossible | [CLOSED NEGATIVE by FTD-0562 for the sufficiently slow periodic-hop family] |
| Every deforming, nonlinear, period-growing, defect/topological manifested carrier is impossible | [OPEN — NOT PROVED] |
| Physical particle radiation power is derived | [OPEN — NOT PROVED] |

No toggle, scenario, force, damping mechanism, or ontology change is licensed.
