# Audit — FTD-0730 persistence/re-entry volume discriminator v1

**Status:** `[AUDIT PASS — TWO-VOLUME LOCAL RECURRENCE QUALIFIED]`  
**Date:** 2026-07-29

## Findings

1. All 88 histories pass action, energy, recoil, inverse, and bound-control
   gates.
2. All 12 lower-energy representatives persist on both volumes with identical
   tick-96 radius classes.
3. All 26 `p=0.0120` arms re-enter on both volumes with zero matched transition
   time difference.
4. Re-entry time is a cubic ray-class observable: face `63`, edge `79`, body
   diagonal `96`, after common entry/exit ticks `7/26`.
5. Six face arms finish negative on each volume; edge and body arms do not pass
   the final-eight negative classifier at tick 96.
6. Two-volume invariance rules out an `L=33`-specific recurrence explanation,
   not all finite-volume dependence or the need for an infinite-volume limit.
7. Existing state variables determine and invert the recurrence; no new
   history or connection primitive is licensed.

## Correct statement

Under the selected action, `p=0.0120` re-entry and representative lower-energy
persistence are identical on `L=33` and `L=65`. The re-entry is a
direction-dependent local recurrence candidate and opens a separate
multi-pass formation test; it is not yet durable capture or stable matter.

## Verification

- protocol `50582DF6…EB83`;
- runner `6EBAC4DA…D210`;
- JSON `ADA8931C…6DB4`;
- CSV `2C40C6B3…FCCA`;
- certificate `FF8E9EF3…D71C`, `387/387 PASS`;
- production defaults, tick, toggles, and scenarios unchanged.

