# FTD-0703 — Connected-bipole deposited-current form factor v1

**Status:** `[MEASURED — PARTIAL THRESHOLD-EDGE SCREENING]`  
**Verdict:** `DEPOSITED_CURRENT_EDGE_SCREENING_PARTIAL`  
**Production status:** unchanged

## Result

The FTD-0638 orientation-0 refined connected bipole was translated rigidly by
the existing exact quadratic-coat face-current deposition and observed with
FTD-0702 on the locked `v=1/2` resonant curve.

All 96 rows pass continuity, current moment, phase matching, sign, scale,
cyclic covariance, and projection gates. The current result is:

| quantity | value |
|---|---:|
| collinear transverse fraction | `6.99e-32` |
| exact edge total power | `2.711904571e-9` |
| `k_x=0.9pi` transverse power | `2.712740096e-4` |
| maximum registered interior transverse power | `4.926497203e-3` |
| interior/edge contrast | `1.8166189e6` |

The ideal FTD-0701 double edge zero survives in the deposited current as very
strong threshold-edge screening. It is not global screening. The largest
registered channel occurs at `k_x=0.75pi`, and broad off-edge phase-matched
transverse current remains.

## Correct scope

The selected geometry creates a high-wavevector soft onset, not protection
from all sub-cone radiation channels. This campaign deposits current only. It
does not update the field, compute recoil, or distinguish dressing from
radiation.

## Record

- protocol SHA256 `D68433E8...96D`;
- JSON SHA256 `4A405F9F...784B`;
- CSV SHA256 `B9F8C7E1...E227`;
- runner SHA256 `B195D5E2...FE82`;
- independent certificate SHA256 `5E88027B...39E`.

