# AUDIT — Exact matched regional energy transport

**Date:** 2026-07-28  
**Identifier:** `FTD-0671`  
**Status:** `[THEOREM — EXACT AUXILIARY REGIONAL ENERGY LEDGER]`  
**Verdict:** `MATCHED_REGIONAL_ENERGY_TRANSPORT_EXACT`  
**Theorem:**
[`THEOREM_MATCHED_REGIONAL_ENERGY_TRANSPORT.md`](../../10_eft_program/derivations/constituent_complete_matter/THEOREM_MATCHED_REGIONAL_ENERGY_TRANSPORT.md)

## Result

The selected matched face/edge leapfrog update now has an observer-only
regional ledger that separates:

```text
regional field-energy change
  = source-free boundary transport into the region
  + deposited-current exchange into the field.
```

The regional energy symmetrically assigns the modified-energy cross term
between its face and edge endpoints. Consequently the selected region plus its
complement exactly reconstructs the global invariant, and their source-free
transport terms cancel.

The C++ qualification covers four nested radii, a full-periodic-volume limit,
an independently deposited quadratic-coat current, source-free control,
integer translation, a proper cubic cyclic rotation, inconsistent-update
rejection, and nonfinite-input rejection:

```text
worst identity residual          1.7763568394002505e-14
integer-translation residual     0
proper-cubic-rotation residual   3.3306690738754696e-16
failures                         0
```

An independent exact-rational certificate exhausts all `128` diagonal
face/edge projector pairs in each of two finite-dimensional witnesses. The
all-region transport is exactly zero and the inside/outside source-free
transport cancels in rational arithmetic.

## Epistemic boundary

This closes the missing measurement layer between global field norm and a
physical morphology claim. It does not determine whether the transported field
is bound dressing, returning field memory, a trailing wake, or detached
radiation. Those are fresh-data classifications requiring nested-radius time
histories. The local energy allocation is symmetric and covariant but not
mathematically unique.

No production state, tick phase, force, toggle, scenario, tolerance, or field
normalization changed.

## Reproducibility

- C++ observer:
  `engine/include/ftd/eft/matched_regional_energy_transport.h` and
  `engine/src/eft/matched_regional_energy_transport.cpp`;
- C++ qualification: `test_matched_regional_energy_transport`;
- exact-rational certificate:
  `scripts/proofs/proof_matched_regional_energy_transport.py`;
- header SHA256:
  `E54EC9A3242CEB07EB0EAF6FCB0B0C1B911601E33351286ACD059FECC4E6CECE`;
- source SHA256:
  `626FF439FB1184CE1A4B2F1CAFDB19C40236E56D3B98CDF8977A18BD6CA438C5`;
- test SHA256:
  `6F9C7F22A64BF96959B5FAEFEDB9C21046A22F25440C378CA12DE0027524255C`;
- exact-rational certificate SHA256:
  `EC7262D0674EE98338A62B8ED787330413CFDF7A4F2A8BFAA03F953AE0A5555F`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
