# AUDIT — Localized-basin observer storage-index correction

**Date:** 2026-07-28  
**Identifier:** `FTD-0680`  
**Status:** `[CORRECTION — QUALIFIED]`  
**Verdict:** `LOCALIZED_BASIN_STORAGE_INDEX_CORRECTED`

## Defect

The first FTD-0677 implementation decoded a flat matched-field index as

```text
x = index mod L,  z = index / L^2,
```

while `MatchedFaceFlux` and `MatchedEdgeField` store

```text
index = x L^2 + y L + z.
```

The original qualification used the symmetric origin `(3,3,3)` and
self-consistent but reversed test indexing, so it could not detect the x/z
exchange.  The theorem was correct but the general implementation claim was
not qualified.

## Correction

The observer now decodes

```text
z = index mod L,
y = (index/L) mod L,
x = index/L^2.
```

The field translation and cyclic-rotation fixtures now use the engine's actual
x-major flat index.  The shell origin is deliberately asymmetric `(3,2,1)`.
The corrected Release CTest passes `27/27`; the exact-rational 48-map
certificate also passes.

## Scope of FTD-0679

FTD-0679 intended an origin at the box center, but the observer uses the
instantaneous floating-point constituent center.  That center is not bitwise
equal in all three coordinates.  The corrected decoder therefore reallocates
small boundary-cell contributions between shells: the complete FTD-0681
replay leaves the core and total-field histories bitwise unchanged, while the
history-scale changes are `1.15%` near, `0.65%` intermediate, and `3.40%` far.
FTD-0679 is non-promotable for both this qualification mismatch and its
independent target-column schema defect.

## Reproducibility

- corrected header SHA256:
  `C2FC41FA50E187F516C4EA758248BDDBC3FFF471C082A4CDD52ADE8E800B7955`;
- corrected source SHA256:
  `E7BA078079A24A41DF30B39ABEFBBD60897831D60DB4E087DA7CC547274727C7`;
- corrected asymmetric-origin qualification SHA256:
  `1559C331BA44BA9C70AEF7741CEDC741F6089E7533A77D29D689ED00433FBBEA`;
- exact certificate SHA256:
  `D21C946A8C8B00880AB792B0721FBDA5DF1A056D673F489E705AB963E9A4BDAB`;
- CTest: `localized_basin_observer`, `1/1` passed;
- toolchain: pinned MSVC `14.44.35207`, Release.

No production state or behavior changed.
