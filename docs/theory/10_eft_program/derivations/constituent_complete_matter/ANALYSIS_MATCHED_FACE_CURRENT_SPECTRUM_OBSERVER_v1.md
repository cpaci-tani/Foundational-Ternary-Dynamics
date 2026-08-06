# FTD-0702 — Matched face-current spectrum observer v1

**Status:** `[THEOREM — QUALIFIED SELECTED OBSERVER]`  
**Production status:** unchanged

## Result

The observer evaluates the complex Fourier coefficient of an oriented
face-current at the actual face carriers, projects it with
`khat_i=2 sin(k_i/2)`, and reports transverse, longitudinal, and total
quadratic power. It applies only the normalization supplied by the caller and
does not silently insert an `L^-3` factor.

The focused qualification passes one-face phase, dense/sparse equivalence,
sign, translation, cubic rotation, projection, and fail-closed controls:

| diagnostic | maximum residual |
|---|---:|
| one-face coefficient | `0` |
| dense/sparse | `0` |
| sign mirror | `0` |
| translation phase | `1.60e-16` |
| cubic covariance | `3.33e-16` |
| power partition | `2.65e-16` |

Verdict: `MATCHED_FACE_CURRENT_SPECTRUM_QUALIFIED`.

## Scope

This is an exact observer qualification. It does not show that matter emits a
field, that a transverse coefficient is radiative, or that Fourier power is
energy or photon number.

## Artifacts

- header SHA256 `50ABAD58...3B19`;
- implementation SHA256 `8B0427FC...B900`;
- test SHA256 `E2C7E4F9...BB77`;
- active CTest `matched_face_current_spectrum`.

