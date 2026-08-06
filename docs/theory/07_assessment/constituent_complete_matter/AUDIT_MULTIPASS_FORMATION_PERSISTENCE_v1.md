# Audit — FTD-0731 multi-pass formation persistence v1

**Status:** `[AUDIT PASS — FINITE-HORIZON TWO-VOLUME RADIATIVE-CAPTURE WITNESS QUALIFIED]`  
**Date:** 2026-07-29

## Findings

1. All 48 histories pass action, energy, recoil, inverse, parent-persistence,
   and pre-bound-control gates.
2. All 12 `p=0.0120` histories begin positive and outside, enter at tick `7`,
   exit at `26`, and re-enter at the locked cubic time `63`, `79`, or `96`.
3. No `p=0.0120` history has a fourth transition through tick 192.
4. Every `p=0.0120` history is graph-inside and has pair energy `<-1e-6` at
   every tick `129--192`.
5. Every capture has positive net field-energy gain and registered nonzero,
   magnetically active, extended dynamic-field morphology.
6. Complete transition sequences and final classes match exactly across
   `L=33/65`; both polarity orders agree.
7. The independent certificate recomputes transitions, energy tails, field
   gains, morphology gates, control tails, volume pairing, and the verdict
   from persisted histories rather than trusting runner booleans.
8. The result is finite-horizon and finite-volume. It does not prove a stable
   basin, asymptotic binding, or a physical particle.
9. The compact pair well remains selected. The result does not derive binding
   from the five postulates or authorize production defaults.
10. Existing constituent plus face/edge state determines and inverts the
    witness; this campaign supplies no evidence that a new primitive is
    necessary.

## Correct statement

Under the selected common-action and compact-well dynamics, an initially
unbound neutral two-constituent pair exhibits reversible, energy-balanced,
multi-pass radiative capture through tick 192 on both `L=33` and `L=65`, in
all three registered cubic direction classes and both polarity orders.

## Nonclaims

- no infinite/open-volume limit;
- no open basin or perturbative stability;
- no moving-composite or collision qualification;
- no charge, mass, spin, statistics, or quantum-particle derivation;
- no claim that rendered field lines are literal ontic strands;
- no production engine or scenario promotion.

## Verification

- protocol `F319B4CA…C01EE`;
- runner `CE40EFAC…5266`;
- JSON `0D4F8519…F03D`;
- CSV `BC060706…F163`;
- certificate `2894C516…17E1`, `583/583 PASS`;
- production tick, defaults, toggles, and scenarios unchanged.

