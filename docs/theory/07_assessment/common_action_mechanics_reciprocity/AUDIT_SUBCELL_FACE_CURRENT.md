# FTD-0478 — Subcell polarity and exact straight-segment face current

**Date:** 2026-07-25  
**Status:** `[DERIVED — UNDER DECLARED PIECEWISE-TRILINEAR SHAPE]` +
`[THEOREM — EXACT MATCHED CONTINUITY]` + `[SELECTION — FACE REPRESENTATION]`  
**Verdict:** `SUBCELL_FACE_CURRENT_EXACT_SELECTED_REPRESENTATION`

## Result

Primitive manifestation remains one ternary site with a continuous remainder.
The pair defines, without a new degree of freedom, a compact signed coupling
shape whose effective position is `anchor+remainder`. The tensor-product hat
has at most eight nonzero coefficients, preserves total polarity, and
reproduces the first moment.

For a straight effective-position segment, integrating the two transverse hat
functions on each integer-plane interval produces oriented face current `K`
with

```text
rho_after - rho_before + div(K) = 0.
```

This is an algebraic property of the declared shape and matched difference
complex. It is not inferred from a fitted trajectory or repaired by Gauss
projection.

## Normalization boundary

The native linear sector has infrared susceptibility
`G_C/C_WAVE^2`. Mapping a matched field with `div(E)=rho` to native units
therefore fixes `J_face=zE` with `z=G_C/C_WAVE^2`. With longitudinal energy
scale `C_WAVE^2`, the current-work coefficient is

```text
C_WAVE^2 z^2 = G_C z,
```

the coefficient supplied by the existing written interaction. The measured
value is `z=0.25627362930856312`; both compatibility residuals are zero.

This establishes one internally compatible normalization. Choosing the
oriented-face complex as the microscopic carrier remains `[SELECTION]`; the
five postulates do not uniquely force this representation.

## Gates and scope

The MSVC run closes partition, first moment, locality, continuity, polarity,
translation, rotation, inversion, and threshold-crossing gates. Worst
continuity residual is `6.66134e-16` against `1e-12`.

No production state or tick is changed. Site-centered `J` is not yet replaced,
and no mechanical impulse, motion, dressing, wake, photon, gauge field, or
Lorentz claim follows. FTD-0479 must derive and test the reciprocal coupled
matter-field step before any engine toggle is admissible.
