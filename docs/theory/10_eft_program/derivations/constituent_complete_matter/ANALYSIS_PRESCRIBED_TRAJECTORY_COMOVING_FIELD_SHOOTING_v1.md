# FTD-0710 — Prescribed-trajectory co-moving field shooting v1

**Status:** `[SELECTED FIELD DYNAMICS — LOCKED ITERATIVE SOLVE NOT RESOLVED]`  
**Verdict:** `PRESCRIBED_TRAJECTORY_FIELD_SHOOTING_NOT_RESOLVED`  
**Production status:** unchanged

The exact 32 segment currents for rigid translation by one site in two ticks
pass continuity at `2.50e-16`, causality exactly, Gauss below `8.89e-16`, and
translation covariance at `3.61e-16`.

The locked 215,622-variable restarted-GMRES solve reduces the co-moving field
residual substantially but does not reach its gate:

| quantity | value |
|---|---:|
| initial residual `L2` | `1.3501317996690008` |
| final residual `L2` after 480 iterations | `0.015709558535695275` |
| electric infinity residual | `2.18554e-4` |
| magnetic infinity residual | `2.37404e-4` |
| correction maximum | `0.320297` |

The reciprocal replay was correctly not attempted because the field gate did
not close. The result neither proves nor disproves exact field solvability;
it localizes the problem to a poorly resolved co-moving wave operator. FTD-0711
therefore replaces iteration by exact per-mode Fourier blocks.

Record: protocol `82E52438...93A4B`; summary `194AA2AA...122A`; proof
`5EE56CA7...2F53`.

