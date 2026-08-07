# AUDIT — Symmetry-aware localized-basin observer

**Date:** 2026-07-28  
**Identifier:** `FTD-0677`  
**Status:** `[THEOREM — EXACT SELECTED OBSERVER]`  
**Verdict:** `LOCALIZED_BASIN_OBSERVER_EXACT`  
**Theorem:**
[`THEOREM_LOCALIZED_BASIN_OBSERVER.md`](../../10_eft_program/derivations/constituent_complete_matter/THEOREM_LOCALIZED_BASIN_OBSERVER.md)

**Correction:** the original implementation/qualification hashes in this
audit were superseded by FTD-0680 after an x/z flat-index reversal was found.
The corrected qualification uses an asymmetric origin and passes `27/27`.

## Result

The selected connected-composite branch now has an observer that does not
confuse whole-object motion with internal deformation.  It removes only a
common constituent translation and common momentum boost, records those
collective offsets separately, and measures the remaining labelled internal
phase distance.  It independently partitions the positive control-relative
difference-field norm into preregistered near, intermediate, and far shells.

The Release C++ qualification returned:

```text
checks=27 failures=0
position=0.0625 momentum=0.16000000000000003 phase=0.41000000000000003
field=(6,13.5,6) residual=0
```

The exact-rational certificate returned:

```text
phase=41/100
field_total=51/2
signed_cubic_maps=48
arithmetic=rational
```

The test also covers identical states, common translation, common boost,
periodic integer translation, a cyclic proper cubic rotation of matter and all
field components, edge-length change, and rejection of a charge-label
mismatch.

## Epistemic boundary

This audit qualifies the observer, not a localized matter solution.  A falling
internal metric in a future history will mean motion toward the registered
reference family over that window.  A far-shell difference field will mean a
remote field disturbance.  Neither observation alone establishes attraction,
binding, radiation, or permanent object/environment separation.

No production behavior or default changed.

## Reproducibility

- API: `engine/include/ftd/eft/localized_basin_observer.h`;
- implementation: `engine/src/eft/localized_basin_observer.cpp`;
- qualification: `engine/tests/test_localized_basin_observer.cpp`;
- exact-rational certificate:
  `scripts/proofs/proof_localized_basin_observer.py`;
- registered CTest: `localized_basin_observer`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.

Current corrected hashes are recorded in
[`AUDIT_LOCALIZED_BASIN_OBSERVER_STORAGE_INDEX_CORRECTION.md`](AUDIT_LOCALIZED_BASIN_OBSERVER_STORAGE_INDEX_CORRECTION.md).
