# Audit — FTD-0729 late-reentry covariance convergence v1

**Status:** `[AUDIT PASS — TARGETED ROOT CONDITIONING CONFIRMED]`  
**Date:** 2026-07-29

## Findings

1. The registered `2e-13` plus-minus scalar defect reproduces FTD-0728
   exactly at `5.6798055148021831e-10`.
2. At `2e-14`, scalar and complete maxima become `6.2501e-12` and
   `2.9612e-12`, with ratios `0.0110` and `0.0131`.
3. Electric, magnetic, and matter components all converge; matter is the
   limiting complete-state component.
4. Graph-transition and final-sign classes are unchanged across conditions.
5. The certificate is targeted. It does not waive the failed FTD-0728 global
   fivefold gate or establish volume stability.

## Correct statement

The recorded worst late-reentry translation defect converges componentwise
under one additional root-tolerance decade. Volume scaling is now admissible
at the selected `2e-14` realization.

## Verification

- protocol `96751A97…D384`;
- runner `98322821…6A16`;
- JSON `C9EF34EB…D6E9`;
- CSV `204724D2…2F3D`;
- certificate `E2DDCB97…721C`, `70/70 PASS`;
- production defaults, tick, toggles, and scenarios unchanged.

