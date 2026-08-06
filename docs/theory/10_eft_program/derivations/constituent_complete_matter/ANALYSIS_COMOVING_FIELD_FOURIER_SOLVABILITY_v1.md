# FTD-0711 — Co-moving field Fourier solvability v1

**Status:** `[NUMERICAL FACT — EXACT FINITE-VOLUME BLOCK ALGEBRA]` +
`[CLOSED NEGATIVE — RIGID TWO-TICK SOURCE]`  
**Verdict:** `FINITE_VOLUME_COMOVING_SOURCE_NULLSPACE_INCOMPATIBLE`  
**Production status:** unchanged

The exact Fourier symbol reproduces the FTD-0710 GMRES residual within
`8.08e-16` in Euclidean norm and `2.48e-17` in infinity norm. Reality and
Parseval residuals are `9.98e-17` and `1.78e-15`.

The minimum-norm spectral solution leaves

```text
nullspace source projection L2 = 4.6345148020027714e-4
spectral residual max          = 9.460163725237027e-5
real-space residual max        = 3.457389920921627e-6
```

The residual is exactly its discarded-left-nullspace projection. It occupies
only eight modes:

\[
(k_x,k_y,k_z)=(\pm2\pi/3,\pm2\pi/3,\pm2\pi/3).
\]

Thus the rigid `v=1/2` current is incompatible with the exact `L=33`
co-moving field equation. The remaining sourced spectrum is not strongly
ill-conditioned: minimum retained source-active singular value is
`4.49e-4`, solution amplification is `4.90`, and maximum correction is
`0.2976`.

This closes only the prescribed rigid two-tick source on this finite torus.
It does not close deforming composites, different periods, constituent
permutations, or causal radiation-bearing histories.

Record: protocol `BD9B0543...21E5F`; summary `DB3669BE...B1E`; proof/runner
`E35BB836...F43A9`.

