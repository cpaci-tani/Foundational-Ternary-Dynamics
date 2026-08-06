# FTD-0703 — Connected-bipole deposited-current form factor v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Qualified observer:** FTD-0702  
**State source:** FTD-0638 orientation-0 refined final coordinates, CSV
SHA-256 `8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F`

## 1. Question

Does the selected connected bipole's *actual quadratic-coat deposited current*
retain the ideal FTD-0701 threshold-edge screening while leaving off-edge
transverse phase-matched channels available?

## 2. Frozen current histories

Read exactly the 16 orientation-0 final constituent positions and polarities
from the registered FTD-0638 state CSV. For each constituent, deposit the
existing exact `QuadraticCoatFaceCurrent` on a rigid straight segment.

- displacement axes: `+x,+y,+z` by cyclic whole-state rotation;
- displacement signs: `+1,-1`;
- displacement magnitudes: `delta={5e-7,1e-6}`;
- current normalization: `16 delta`, the total transported unsigned polarity;
- lattice carrier coordinates and transverse projection: qualified FTD-0702
  observer, with no `L^-3` factor.

No matter equation, field update, recoil, or redressing is executed here.

## 3. Frozen resonant curve

Use source speed `v=1/2`. Register

```text
k_parallel/pi = {2/3, 0.70, 0.75, 0.80, 0.85, 0.90, 0.95, 1.00}.
```

For each point, choose the nonnegative transverse component from

\[
\sin^2(k_\perp/2)
=3\sin^2(k_\parallel/4)-\sin^2(k_\parallel/2).
\]

The third component is zero. Rotate wavevector and object together for the
`y` and `z` arms.

## 4. Locked gates

Execution is valid only if:

- exactly 16 constituents are read with net polarity zero;
- every deposited segment passes validity, continuity `<=1e-12`, current
  moment `<=1e-12`, locality, and causality;
- every wavevector satisfies `|Omega(k)-(1/2)k_parallel|<=2e-15`;
- every FTD-0702 observation is valid and has projection residual `<=1e-14`;
- normalized opposite-direction coefficients mirror within `5e-6`;
- normalized `delta`/`delta/2` coefficients agree within `5e-6`;
- cyclic current coefficients and powers agree within `2e-12`;
- the collinear `k_parallel=2pi/3` transverse fraction is `<=1e-24`;
- at the exact edge witness the transverse fraction differs from `1/3` by
  `<=1e-12` and normalized total current power is `<=1e-7`;
- at `k_parallel=0.9pi`, normalized transverse current power is `>=1e-5`;
- the largest registered interior transverse power exceeds the edge
  transverse power by at least `100`.

## 5. Verdicts

- `DEPOSITED_CURRENT_EDGE_SCREENING_PARTIAL`: every gate passes;
- `DEPOSITED_CURRENT_COMPLETE_SCREENING_CANDIDATE`: execution passes but every
  registered interior transverse power is below `1e-5`;
- `DEPOSITED_CURRENT_EDGE_SCREENING_CLOSED`: execution passes but the edge or
  covariance gates fail;
- `DEPOSITED_CURRENT_FORM_FACTOR_EXECUTION_INVALID`: any algebraic, source,
  hash, continuity, or observer gate fails.

The first verdict supports a soft threshold onset but explicitly does not
measure radiation. No post-run wavevector insertion or tolerance change is
allowed.
