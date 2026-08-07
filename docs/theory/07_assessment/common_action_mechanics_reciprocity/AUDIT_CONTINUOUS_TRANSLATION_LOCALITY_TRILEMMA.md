# AUDIT — Continuous-translation locality trilemma

**Date:** 2026-07-26  
**Identifier:** `FTD-0554`  
**Status:** `[THEOREM — HOMOGENEOUS UNITARY FINITE-RANGE TRANSLATION NO-GO] +
[CONSTRUCTIVE — NONLOCAL BAND-LIMITED ESCAPE]`  
**Verdict:** `EXACT_TRANSLATION_REQUIRES_NONLOCAL_COUPLING`  
**Pre-registration:**
[`PREREG_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA_v1.md)  
**Theorem:**
[`THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md`](../../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md)  
**Run of record:** `engine/results/ftd_0554/windows_msvc_cpu.json`

## Result

```text
registered volumes                     2  (L=17,33)
registered shift samples               10
registered continuity samples          12
minimum noninteger kernel support       L in each volume
minimum density-change support          L in each volume
minimum current support                 L in each volume
most negative kernel weight            -0.21619535286678662
worst cardinal residual                1.4432899320127035e-15
worst group residual                   1.0130785099704553e-15
worst neutral-energy residual          6.9388939039072284e-17
worst identity residual                6.5527365553298786e-15
smallest local quadratic coefficient   0.0034321130744224032
smallest local quadratic barrier       2.1450706715140020e-04
failures                               0
```

The exact fractional translation group removes the neutral composite's
subcell energy variation. It succeeds only by spreading both the coupling
kernel and the exact continuity current over the complete periodic lattice.
The compact quadratic control retains a nonzero barrier.

## The defect is categorical

A finite Laurent polynomial of unit modulus on the circle is necessarily a
single monomial times a phase. Consequently a continuous family of
homogeneous finite-range unitaries cannot change from the identity's shift
degree zero to the one-site shift's degree one. Raising the local interpolation
order does not evade this integer obstruction.

The band-limited witness is the exact opposite corner: phase-only Fourier
translation gives composition, cardinality, constant norm, and constant
translation-invariant field energy, but loses compact support and positivity.

## Program consequence

FTD-0552/0553 are not repaired by a better compact interpolation polynomial.
The original arbitrary-subcell static gate is incompatible with exact
microscopic locality if it is interpreted as demanding a genuine continuous
unitary translation group.

This does not justify ignoring the observed pinning. It changes the correct
target: an extended native excitation or hopping quasiparticle must have a
Peierls barrier whose ratio to its inertial scale tends toward zero in the
infrared. No such excitation has yet been derived. The default-off mobile
toggle, dressing scenario, and infrared particle claims remain unlicensed.

No production state, force, phase, toggle, default, scenario, normalization,
or tolerance changed.

## Reproducibility

- focused test: `continuous_translation_locality`, `1/1` passed;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- preregistration SHA256:
  `15BAA804B8018187C72B03BE14886F4FE653FC48A18A6D5E643F548A9D56C114`;
- header SHA256:
  `41BB544B0113B701C5EBD42E1035720BAEEDF8E8542E7467E951C81F0D796D73`;
- source SHA256:
  `F1AD507701793B728E551F65705B44A59564F7B0C143645E8B370AE52783B7F3`;
- test SHA256:
  `36BF16448D6EA1CD639E3320C37F57DD117A089A4943F4E173A0EFB1E0B90653`.
