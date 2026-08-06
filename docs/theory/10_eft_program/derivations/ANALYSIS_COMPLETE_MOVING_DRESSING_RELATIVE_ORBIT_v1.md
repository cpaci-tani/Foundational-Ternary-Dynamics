# FTD-0706 — Complete moving-dressing relative-orbit test v1

**Status:** `[EXECUTION INVALID — REST PREPARATION NOT A FIXED POINT]`  
**Verdict:** `MOVING_DRESSING_RELATIVE_ORBIT_EXECUTION_INVALID`  
**Production status:** unchanged

## Result

The locked candidate asked whether the `L=33` static dressing used by
FTD-0704, after assigning every constituent velocity `v=1/2`, obeys the exact
complete-state relation

\[
F^2(X)=T_{(1,0,0)}X.
\]

The common-action evolution, state-only inverse, and integer-translation
covariance controls pass. Two forward ticks produce 12 site hops, with maximum
energy drift `8.57e-12`, maximum common residual `1.96e-11`, inverse residual
`1.14e-11`, and translated-covariance residual `5.91e-14`.

The registered rest control fails: the supposedly static `L=33` preparation
changes by `1.88059e-5` after two ticks, above the locked `1e-9` fixed-point
gate. Therefore the execution verdict is invalid and no relative-orbit
classification is promoted.

The component residuals remain useful diagnostics but are not a verdict:

| component of `F^2(X)-T_1X` | maximum residual |
|---|---:|
| effective constituent position | `8.65126e-4` |
| constituent momentum | `3.55671e-3` |
| electric face field | `2.98069e-1` |
| magnetic edge field | `1.44323e-1` |
| complete state | `2.98069e-1` |

The constituent scaffold is close to the requested translation while the
instantaneously boosted static field is not. Because the rest preparation is
also imperfect, these numbers localize the likely defect but do not prove that
a self-consistent moving dressing is absent.

## Ontological consequence

Uniform matter motion must be defined as a relative periodic orbit of the
complete relational state, not as momentum assigned to a static field cloud.
FTD-0706 shows that the current `L=33` preparation is inadequate for that
test. It does not force a new primitive: the next admissible repair is a fresh,
locked construction that first produces a true `L=33` rest fixed point and
then solves the state-only moving-orbit residual. The failed preparation and
its result remain preserved.

## Record

- protocol SHA256 `D07F8CE1...E8FA7`;
- JSON SHA256 `3F7E0A2A...094F6`;
- metrics SHA256 `EB29D546...D5039`;
- runner SHA256 `E719B13C...F709`;
- proof SHA256 `0AAF1464...AE14`.

