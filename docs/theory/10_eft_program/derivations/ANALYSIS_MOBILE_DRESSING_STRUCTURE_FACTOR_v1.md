# FTD-0655 — Mobile dressing structure factor v1

**Status:** `[EXECUTION INVALID — PROTOCOL/IMPLEMENTATION TICK-COUNT MISMATCH]`  
**Verdict:** `MOBILE_DRESSING_STRUCTURE_FACTOR_EXECUTION_INVALID`  
**Production status:** unchanged

## Controlling result

The locked protocol requires `64w` forward and `64w` reverse ticks per arm.
The implementation ran `32w` in each direction, following the separate
`T_phys=64`, `a=2/w` statement. Those two protocol clauses are inconsistent,
and the executable followed only one of them. The v1 execution is therefore
invalid and supplies no promotable physics verdict.

## Preserved raw observations — non-promotable

All 18 width, direction, mirror, and cubic raw histories complete the exact
forward/reverse common action. Every matter and field-energy structure factor
passes the locked phase, amplitude, velocity, and relative-phase gates.

The worst refinement metrics are:

| width | field–matter velocity mismatch | relative-phase RMS | field amplitude CV |
|---:|---:|---:|---:|
| 2 | 0.0214643 | 0.0246891 | 0.0322034 |
| 3 | 0.0101913 | 0.0100017 | 0.0116770 |
| 4 | 0.00521419 | 0.00669860 | 0.00553221 |

All three raw sequences decrease strictly. Mirror and cubic residuals are
`4.73e-12` and `3.25e-12`, respectively. The worst common-action residual is
`2.00e-11`, inverse recovery `1.06e-9`, and relative edge strain `0.00215`.

## Interpretation boundary

These observations are retained to preserve provenance but do not establish
co-motion because the literal registered arm length was not executed. A fresh
version must resolve the arithmetic before implementation and rerun the full
matrix.

Even a corrected constructive result would not decide whether detached flux is a wake, radiation, or a
pilot-wave-like object. It does not measure a retarded response function,
spectral residue, linewidth, formation mechanism, effective charge, spin,
statistics, or common Lorentz cone. The next licensed question is a
source-response pole on the mobile dressed background.
