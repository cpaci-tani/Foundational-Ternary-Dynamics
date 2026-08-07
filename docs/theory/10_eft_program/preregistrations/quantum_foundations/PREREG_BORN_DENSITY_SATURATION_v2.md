# PREREG · Born-fraction saturation scan (v2)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`
**Date locked:** 2026-08-07 *(git-tag lock rides the owner's next commit;
the runner SHA-256 below is the binding lock in the interim)*
**Parents:** `PREREG_BORN_DENSITY_UPCROSSING_v1.md` (v1.1 → OUTCOME E),
FTD-0187/FTD-0200/FTD-0356/FTD-0798, temporal-interior front T3.
**Production impact:** none — verification instrument.

## 1. Question

v1.1 established (Outcome E) that the excess-upcrossing weighting moves
from amplitude toward occupation as `Ω·τ` grows: Born-fraction
`0.024 [0.014, 0.036]` at `Ω·τ ≈ 0.45–0.91` rising to
`0.099 [0.090, 0.109]` at `1.81–3.56`. The decisive follow-up: **does the
Born-fraction saturate below 1 (the mechanism class can never yield Born
weighting) or approach 1 (Born statistics as the fast-mode/slow-noise
asymptote)?**

## 2. Locked artifacts

| Artifact | SHA-256 |
|---|---|
| `scripts/experiments/verify_born_saturation_v2.py` | `2887218838b7b6d9a49f8eff04617a8210ce7bb9b4096ed1f43e92e0db283ed4` |

Five arms — `(λ₁,λ₂|τ)` = `(64,32|8)`, `(16,8|8)`, `(16,8|32)`,
`(8,4|32)`, `(8,4|128)` — spanning characteristic `Ω·τ ≈ 0.64 → 78`, with
**fresh seed bases** (`20260807 + 100000·arm`), so arms 1–2 independently
replicate v1.1's two points. All other instrument parameters inherited
frozen (L = 4096, `σ_n = 0.17`, `K = W_SC`, `A₁ = 0.10`, equal occupation,
20,000 ticks, 48 seeds, 2,000 bootstrap resamples). Registered statistic:
per-arm Born-fraction `BF = (R̂ − R_amp)/(1 − R_amp)` with 99% CI.

## 3. Pre-blessed outcomes

| Outcome | Condition | Consequence |
|---|---|---|
| **A — SATURATES** | `BF₅` CI-upper `< 0.5` AND `BF₅ − BF₄ ≤ 0.05` | the mechanism class cannot reach Born weighting; T1c requires a different mechanism class; the temporal-interior Born row records a bounded ceiling |
| **B — APPROACHES** | BF point estimates non-decreasing AND `BF₅` CI-lower `≥ 0.5` | Born weighting is credibly the `Ω·τ → ∞` asymptote — the strongest T1c development on record; registered next step: engine-side fast-mode campaign (FTD-0200 path) |
| **C — RISING-UNRESOLVED** | non-decreasing AND `BF₅` CI-upper `< 0.5` AND `BF₅ − BF₄ > 0.05` | direction confirmed, asymptote unresolved at reachable `Ω·τ`; redesign toward the band top |
| **D — INVALID / INDETERMINATE** | any arm killed (excess `< 5,000`; non-positive coefficient) or no registered pattern | no verdict; diagnose |

A descriptive fit `BF = B_∞·x²/(c²+x²)` is reported with **no decision
weight**.

## 4. Pre-run observations (theory only, recorded before execution)

1. Arm 5's mode `λ = 4` sits at `Ω = 0.841`, two-thirds of the field band
   top `1.231` — near the fastest coherent mode the lattice dispersion
   supports; if BF is still `< 0.5` there, outcome A is the honest
   reading even though larger `τ` could in principle be scanned further.
2. Background crossing rates fall roughly as `1/τ` (Rice); the 5,000-count
   floor guards power at `τ = 128`.
3. Whatever the outcome, no substrate-level tag moves (ensemble
   `[IMPOSED]`; quick-check platform; FTD-0200 engine closure pending).

## 5. Execution

```
python scripts/experiments/verify_born_saturation_v2.py
```

Deterministic under the locked seeds; outcome letter to be reported
verbatim; LEDGER row minted only by the owner.

---

## v2 EXECUTION RECORD (2026-08-07)

All five arms valid (net excess 1.30–2.10M crossings each; all
coefficients positive).

| arm | (λ₁,λ₂ \| τ) | Ω·τ char | BF | 99% CI |
|---|---|---|---|---|
| 1 | (64,32 \| 8) | 0.64 | 0.0494 | [0.0359, 0.0623] |
| 2 | (16,8 \| 8) | 2.54 | 0.1020 | [0.0898, 0.1144] |
| 3 | (16,8 \| 32) | 10.15 | 0.2735 | [0.2549, 0.2903] |
| 4 | (8,4 \| 32) | 19.59 | 0.4694 | [0.4583, 0.4797] |
| 5 | (8,4 \| 128) | 78.36 | **0.8362** | [0.8130, 0.8612] |

> **OUTCOME B — APPROACHES: Born weighting is the fast-mode/slow-noise
> asymptote.** Strictly monotone; `BF₅` CI-lower `0.813 ≥ 0.5`.
> Descriptive fit (no decision weight): `B_∞ = 0.860`, crossover
> `c ≈ 16.6`.

Honest riders, recorded with the result: (i) arm 2 replicates v1.1's
fast point exactly (`0.102 [0.090, 0.114]` vs `0.099 [0.090, 0.109]`);
arm 1's fresh-seed slow point (`0.049 [0.036, 0.062]`) sits marginally
above v1.1's (`0.024 [0.014, 0.036]`) — a phase-draw systematic of order
`0.02` beyond seed noise, immaterial to the outcome (which rides on the
`0.05 → 0.84` trend) but a declared averaging target for any v3.
(ii) Whether the asymptote is exactly 1 or `≈ 0.86` is the registered
residual question. (iii) Scope unchanged: mechanism-level, `[IMPOSED]`
ensemble, quick-check platform; the FTD-0200 engine campaign now has a
quantitative design target — the fast-mode regime `Ω·τ ≳ 30`.
