# AUDIT — Exact connected-state reservoir decomposition

**Date:** 2026-07-28  
**Identifier:** `FTD-0673`  
**Status:** `[THEOREM — EXACT SELECTED PERTURBATION LEDGER]`  
**Verdict:** `CONNECTED_RESERVOIR_DECOMPOSITION_EXACT`  
**Theorem:**
[`THEOREM_CONNECTED_RESERVOIR_DECOMPOSITION.md`](../10_eft_program/derivations/THEOREM_CONNECTED_RESERVOIR_DECOMPOSITION.md)

## Result

The selected connected matter/field system now has an observer-only exact
excited-minus-control ledger:

```text
Delta total energy
  = target tangent modes
  + all other tangent modes
  + exact nonlinear matter remainder
  + dynamic-field self energy
  + control/dynamic field interference.
```

This is the first registered split that can ask which internal reservoir funds
the FTD-0670/0672 doublet recovery without adding tangent-mode energy to the
exact binding energy and thereby double-counting the matter sector.

The executable qualification returned:

```text
orthonormality residual          0
field decomposition residual    1.3552527156068805e-20
matter decomposition residual   0
complete decomposition residual 5.082197683525802e-20
failures                         0
```

It also rejects a nonorthogonal basis, an incomplete basis, duplicate target
indices, a graph mismatch, and nonfinite field input. The exact-rational
certificate verifies two independent witnesses and all `22` nonempty target
subsets with exact arithmetic.

The cancellation-safe qualification includes a perturbation down to `1e-7` in
matter and `1e-8` in the field. It uses difference-of-squares forms rather than
subtracting the large constituent rest-energy background. The registered
CTest passed `1/1` under the pinned Release toolchain.

## Epistemic boundary

The theorem supplies accounting, not ontology. In particular, its nonlinear
matter remainder is whatever part of the exact relativistic-plus-binding
difference is not represented by the complete tangent quadratic at the chosen
control. It may contain anharmonic binding, relativistic kinetic corrections,
control displacement effects, and mode-coupling terms. A fresh preregistered
history is required before any one of those is named as the donor.

No production state, tick phase, force, toggle, scenario, tolerance, or field
normalization changed.

## Reproducibility

- C++ observer:
  `engine/include/ftd/eft/connected_reservoir_decomposition.h` and
  `engine/src/eft/connected_reservoir_decomposition.cpp`;
- C++ qualification: `test_connected_reservoir_decomposition.cpp`;
- exact-rational certificate:
  `scripts/proofs/proof_connected_reservoir_decomposition.py`;
- header SHA256:
  `BFA1E52798E8763D87655CBC79C092058492C702CAC732FA3A4B3EC3356A2E82`;
- source SHA256:
  `9F241EB72AECFC2A24566DBB05637979758CAEC25BEA7A183A6DB33069EAA036`;
- test SHA256:
  `793894579D808B67AA64B14D7DC2B8544077E7C3D346B54322E3C41DDDB3877B`;
- exact-rational certificate SHA256:
  `F8B16A2848F5D85BCA05BA3704A1A1B358FBF15AAE845A5D8AAF24535DA28DB5`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
