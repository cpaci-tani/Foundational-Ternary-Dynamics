# ANALYSIS -- Collective-coordinate genesis counting model v1 (FTD-0277)

**Status:** `[CLOSED NEGATIVE -- v1 collective-coordinate counting route]`
**Date:** 2026-06-14
**Pre-registration:** [`PREREG_GENESIS_COUNTING_v1.md`](../../10_eft_program/preregistrations/PREREG_GENESIS_COUNTING_v1.md)
**Scope:** [`SCOPE_GENESIS_COUNTING_MODEL.md`](SCOPE_GENESIS_COUNTING_MODEL.md)
**Run of record:** `scripts/exploration/results/genesis_counting_v1/analysis.txt`

---

## SS1 - Verdict

The locked v1 collective-coordinate / genesis-counting model is
**CLOSED NEGATIVE**.

It correctly reproduces two qualitative directions:

- `k_eff ~ drain^-1` (measured exponent `-1.000`, gate `[-1.20,-0.70]`);
- increasing `gamma` moves the fitted knee upward and suppresses the high-A ratio.

Those two passes are not enough. The primary gates fail: the model predicts an
exact `N ~ A^2` law with no current-stack broken-power structure, over-predicts
the count by about `20x-40x`, and gates essentially the same 389-site region at
every amplitude.

## SS2 - Frozen-gate results

| Gate | Frozen pass condition | Result | Verdict |
|---|---:|---:|---|
| F1a knee | `[14,18]` | `24.25` | FAIL |
| F1b sub-knee exponent | `[3.3,4.1]` | `2.000` | FAIL |
| F1c super-knee exponent | `[1.6,2.1]` | `2.000` | PASS |
| F2 curve RMS | `log10 RMS <= 0.15` | `1.449` | FAIL |
| F3 A=10 count | `[3,7]` | `160.040` | FAIL |
| F4 A=14 shell L1 | `<= 0.30` | `1.830` | FAIL |
| F5 drain exponent | `[-1.20,-0.70]` | `-1.000` | PASS |
| F6 gamma direction | knee non-decrease + high-A ratio decrease | PASS | PASS |

Overall analyzer token:

```
COUNTING_MODEL_V1_CLOSED_NEGATIVE
```

## SS3 - Main sweep

| A | N_model | gated sites | FTD-0261 target | ratio |
|---:|---:|---:|---:|---:|
| 10 | 160.040 | 389 | 4.0 | 40.010 |
| 12 | 230.458 | 389 | 8.4 | 27.435 |
| 14 | 313.679 | 389 | 16.4 | 19.127 |
| 16 | 409.703 | 389 | 21.6 | 18.968 |
| 20 | 640.161 | 389 | 27.4 | 23.364 |
| 25 | 1000.252 | 389 | 32.6 | 30.683 |
| 30 | 1440.363 | 389 | 45.0 | 32.008 |
| 40 | 2560.646 | 389 | 91.8 | 27.894 |
| 50 | 4001.009 | 389 | 130.2 | 30.730 |
| 70 | 7841.977 | 389 | 260.2 | 30.138 |
| 90 | 12963.268 | 389 | 383.3 | 33.820 |

The fitted broken-power diagnostic returns `p_lo = p_hi = 2.000` with essentially
zero internal residual. That is not a hidden success: it means the ansatz reduced
to a pure quadratic law and missed the current-stack knee/shape.

## SS4 - Geometry failure

The FTD-0269 A=14 engine geometry is concentrated in the first Moore shells:

```
center 0.059701, SC 0.358209, FCC 0.134328, BCC 0.373134, SC2 0.074627
```

The v1 model gates mostly outside those shells:

```
center 0.002571, SC 0.015424, FCC 0.030848, BCC 0.020566,
SC2 0.015424, outer 0.915167
```

The shell L1 distance is `1.830`, far above the frozen `0.30` pass gate. This is
the static-threshold snowball identified before lock: the model lacks flux
consumption and therefore does not self-limit the firing region.

## SS5 - What survives

The drain exponent pass is real and useful. It says the slosh-pass dissipation
picture captures one sign-level feature of the engine:

| drain | N_model(A=12) | k_eff |
|---:|---:|---:|
| 0.125 | 921.832 | 6.4016 |
| 0.250 | 460.916 | 3.2008 |
| 0.375 | 307.277 | 2.1339 |
| 0.500 | 230.458 | 1.6004 |
| 0.625 | 184.366 | 1.2803 |
| 0.750 | 153.639 | 1.0669 |

But this is only one leg. It does not derive the observed `N(A)` law because the
same model fails the magnitude, geometry, and shape gates.

## SS6 - Consequences

- FTD-0277 v1 is closed negative.
- FTD-0110 nonlinear bridge remains `[OPEN -- boundary mapped]`.
- FTD-0250 transport-inertia / collective-coordinate reduction remains `[OPEN]`.
- Any successor v2 must be a fresh pre-registration. It cannot inherit success
  from v1; it must add at least flux consumption/self-limiting gates and a
  dispersal-race capture functional, then face the same kind of frozen gates.

No claims are promoted. FTD-0013 stays `[SMC]`; MC-T4.3 stays
`[FOUNDATIONAL OBSTRUCTION]`; the FTD-0110 linear `k = 1/4` theorem is untouched.
