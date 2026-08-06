# FTD-0697 — Batched matched symmetry-ray spectrum v1

**Status:** `[PRE-REGISTRATION — EXACT OBSERVER EQUIVALENCE]`  
**Production status:** unchanged

## Purpose

The qualified FTD-0696 direct observer scans the complete field separately for
each wavevector. This protocol locks a batched implementation that must return
the same complex coefficients and projections after one modular-bin pass per
registered symmetry ray.

## Frozen algorithm

For primitive integer ray direction `d`, bin every component difference by

\[
u=d\cdot(x,y,z)\pmod L.
\]

For each requested nonzero harmonic `n`, apply the length-`L` direct DFT to the
bins and multiply each component by its frozen FTD-0696 face/edge carrier phase
`exp(-2 pi i n d dot offset/L)`. Normalize by `L^-3`, then apply the unchanged
FTD-0696 lattice projector at mode `n d`.

The implementation may share validation and finalization code with the direct
observer. It may not change Fourier sign, carrier offsets, normalization,
projection, or reported power.

## Locked qualification

Use `L=31`, rays `<100>`, `<110>`, and `<111>`, harmonics `3,5,7`, both
transverse polarizations, longitudinal modes, curl-generated modes, a
multi-ray/multi-harmonic superposition, translated copies, zero input, and a
nonfinite control.

For every requested wavevector:

- batch/direct complex electric and magnetic coefficients agree within
  absolute `1e-14`;
- every projected coefficient agrees within absolute `1e-14`;
- total, transverse, and longitudinal powers agree within relative `1e-12`
  or are both exactly zero;
- projection residuals remain at most `1e-14`;
- output ordering is ray-major and requested-harmonic-major;
- duplicate/zero directions, zero harmonics, and nonfinite fields fail closed.

Any failure closes version 1. No matter history is measured here.

## Allowed conclusion

Passing licenses the batch implementation as an exact numerical refinement of
FTD-0696 for registered symmetry rays. It adds no spectral matter evidence and
does not convert morphology power into exact energy.
