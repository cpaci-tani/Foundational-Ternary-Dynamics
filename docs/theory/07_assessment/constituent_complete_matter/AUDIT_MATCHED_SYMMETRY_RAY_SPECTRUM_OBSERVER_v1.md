# Audit — FTD-0696 matched symmetry-ray spectrum observer v1

**Verdict:** `[AUDIT PASS — OBSERVER ONLY]`

## Locked artifacts

- protocol SHA256:
  `3A750500246EDDED017E3CBC2D9DB3F5408616E062E15478A79FDAE93CCCB05B`
- header SHA256:
  `1F909A08E576A083F1CAD2FFCE43AC9DCD7B4362CD5D35DCA73ABF63B7BADBE9`
- source SHA256:
  `CC3A71015D22BE756D2D4AEBD18CBE02AE00BB3F8BDDC3121E2E532E6D4E8E72`
- focused test SHA256:
  `34E3C04FF4232F538F0B43BFD9EF81FB28D7F2508A7A8A568E8505CE686DA895`

The focused CTest `matched_symmetry_ray_spectrum` passes.

The hashes above include the FTD-0697 batch API added without changing the
direct observer definition. The FTD-0696 focused test hash remains unchanged.

## Audit checks

- Direct sums use `exp(-i k dot r)` at the positive-face and oriented-edge
  carrier coordinates, not storage-cell centers.
- Longitudinal projection uses the matched-complex wavevector
  `khat_a=2 sin(k_a/2)`.
- The matched curl is transverse under that same projector.
- Translation phase and proper cubic copies are tested independently of power
  invariance.
- Nonfinite input and the zero wavevector fail closed.
- The implementation labels its power as morphology and does not identify it
  with the exact staggered modified energy.

## Claim boundary

The observer is qualified only for requested finite-volume wavevectors. It
does not integrate the unobserved Brillouin zone, extract a pole, establish a
resonance, or count field quanta. A fresh registered matter history remains
required.
