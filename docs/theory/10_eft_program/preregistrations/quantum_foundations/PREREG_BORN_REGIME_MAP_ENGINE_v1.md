# PREREG · Engine-side Born regime map (v1)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`
**Date locked:** 2026-08-07 *(git-tag lock rides the owner's next commit; the
source SHA-256 below is the binding lock in the interim)*
**Parents:** FTD-0187/FTD-0200 (T1c; the pre-registered engine test this
campaign is), FTD-0356, FTD-0798 §2, `PREREG_BORN_DENSITY_UPCROSSING_v1.md`
(v1.1 → E), `PREREG_BORN_DENSITY_SATURATION_v2.md` (v2 → B), temporal-
interior front T3.
**Production impact:** none — the campaign binary is read-only over engine
state (genesis OFF; the field is never written).

## 1. Question

The mechanism-level result of record (v2, OUTCOME B) is that
Born/occupation weighting is the fast-mode/slow-noise asymptote of
threshold-crossing statistics in the FTD-0187/0356 model class. **Does that
regime law transfer to the NATIVE thermal field** — the engine's own wave
dynamics (M18 stencil) thermalized by its own `[IMPOSED]` Langevin
thermostat, with propagating (not iid) noise and the native threshold
`K_GENESIS` on `|J|`?

## 2. Locked artifacts and instrument

| Artifact | SHA-256 |
|---|---|
| `engine/tests/campaign_born_regime_map.cpp` (rev-4) | `90de28e7459458171154a41cd277a6e3f37586658a5a3063f4f3447438cd2ea7` |

Instrument (analytic-overlay, validated in four shakedown revisions, all
pre-lock): engine profile = `wave_propagation + gauss_projection + langevin`
only; the two-mode equal-occupation coherent field
(`J_coh_z = A₁cos(k₁x)cos(Ω₁t+θ₁) + A₂cos(k₂y)cos(Ω₂t+θ₂)`,
`A_i ∝ 1/√Ω_i`, `A₁ = 0.5·σ_z`) is a deterministic overlay added at
counting time — one run yields signal and exact same-noise control. All
observer statistics use the **mean-subtracted** field (the thermostat's
zero mode is divergence-free and otherwise frozen — shakedown lesson).
Per-cell auto-calibration drives `T` to the cell's `σ_target`
(≤5 iterations, guard `T ∈ [10⁻⁶, 100]`, else cell INVALID). Grid: 10
cells over `γ ∈ {0.01, 0.1, 0.5}`, `σ_target ∈ {0.34…0.55}`, mode pairs
`(λ₁,λ₂) ∈ {(16,8),(8,4)}`; `L = 32`, 20,000 measurement ticks, seeds
`20260807 + 977·cell`.

**Declared axes.** Per cell, three regime measures are recorded:
- **`τ_mag` (PRIMARY)** — integrated autocorrelation of the
  mean-subtracted `|J|` at 6 probe sites: the direct analog of the toy's
  noise correlation time; `γ` is its physical knob.
- `τ_cross = 1/ν_ctl` — the control crossing clock (secondary).
- `K/σ_z` — threshold height in noise units (secondary; the toy held it
  fixed at 2.97 and never explored it).

Registered statistic per cell: Born-fraction
`BF = (R̂ − R_amp)/(1 − R_amp)` from the `{1, cos2k₁x, cos2k₂y}`
regression of per-site excess counts.

## 3. Pre-blessed outcomes

Evaluated over valid cells (validity: calibration in bounds, `ctl > 0`,
`c₁ > 0`; ≥ 7 of 10 cells must be valid, else INVALID):

| Outcome | Condition | Consequence |
|---|---|---|
| **T — TRANSFER SIGNATURE** | Spearman ρ(BF, Ω̄·τ_mag) ≥ 0.7 across valid cells (Ω̄ = √(Ω₁Ω₂)) | the mechanism's regime law survives native propagating noise; the strongest T1c development available at this scope; FTD-0200's demanded engine test returns positive at `[MEASURED — engine, imposed thermostat ensemble]` |
| **A — ALTERNATE AXIS** | (T fails) AND Spearman |ρ| ≥ 0.7 for `K/σ` (partial, at fixed γ and pair) or for `τ_cross` | the native weighting is governed by an axis the toy did not isolate; report the surface; mechanism theory must be revised before any Born reading |
| **N — NO STRUCTURE** | no declared axis reaches \|ρ\| ≥ 0.7 | the toy law does not transfer; T1c's mechanism class is disfavoured on the native field |
| **I — INVALID** | < 7 valid cells | diagnose and re-register |

**Pre-run observations (recorded before execution):** shakedown produced
three unlocked data points — BF ≈ 0.011 at `τ_mag ≈ 1.9` (γ=0.01),
BF ≈ 0.047 (γ=0.10, gauss OFF, pre-rev-4 — not comparable), BF ≈ 0.232
(γ=0.10, σ=0.55, gauss ON, pre-mean-subtraction — not comparable). Only
rev-4 cells count; the shakedown numbers carry no evidential weight and
are recorded to prevent hindsight bias. The γ-ladder prediction under
transfer: BF rises from the γ=0.01 cells (fast local noise) to the γ=0.5
cells (slow relaxation).

## 4. Platform declaration (binding)

Locked run on the **WSL2 build** (`engine/build_wsl`) per the standing
GPU-campaign rule; backend recorded from the runner banner. The campaign
is deterministic per backend under the locked seeds; cross-backend
bit-identity is NOT claimed (cuRAND vs CPU RNG differ; the declared run
is the WSL2 one). This campaign can close FTD-0200's *procedural*
requirement (a pre-registered engine test of a Born candidate mechanism);
any substrate-level tag motion remains gated on the `[IMPOSED]`-ensemble
qualifier and is the owner's booking decision.

## 5. Execution

```
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && engine/build_wsl/campaign_born_regime_map"
```

Outcome letter reported verbatim; LEDGER row minted only by the owner.

---

## v1 EXECUTION RECORD (2026-08-07, WSL2 CUDA backend)

**10/10 cells VALID** (all calibrations converged in ≤1 iteration on the
mean-subtracted field; counts 1.1×10⁵–3.2×10⁷ per cell; all coefficients
positive). Note: the binary's footer still printed the shakedown banner —
cosmetic only; the executed source matches the locked SHA-256 above.

| γ | (λ₁,λ₂) | σ_z | τ_mag | τ_cross | K/σ | BF |
|---|---|---|---|---|---|---|
| 0.01 | (16,8) | 0.4720 | 1.9 | 68.0 | 3.21 | 0.0114 |
| 0.10 | (16,8) | 0.5494 | 1.5 | 20.5 | 2.76 | 0.0190 |
| 0.10 | (16,8) | 0.4709 | 1.4 | 67.2 | 3.22 | 0.0232 |
| 0.10 | (16,8) | 0.4005 | 1.4 | 408.6 | 3.79 | 0.0365 |
| 0.10 | (16,8) | 0.3391 | 1.4 | 5782.0 | 4.47 | 0.0096 |
| 0.50 | (16,8) | 0.4679 | 0.6 | 68.7 | 3.24 | 0.0096 |
| 0.50 | (16,8) | 0.3653 | 0.6 | 1572.5 | 4.15 | 0.0519 |
| 0.10 | (8,4) | 0.4690 | 1.4 | 69.5 | 3.23 | 0.0329 |
| 0.10 | (8,4) | 0.3683 | 1.4 | 1403.5 | 4.12 | 0.0483 |
| 0.50 | (8,4) | 0.4002 | 0.6 | 406.8 | 3.79 | 0.0407 |

Registered tests: Spearman ρ(BF, Ω̄·τ_mag) = **+0.01** (primary — fails);
ρ(BF, Ω̄·τ_cross) = **+0.37** (fails); K/σ partial at fixed (γ=0.10,
16–8), n = 4: **−0.20** (fails; the n = 2 partials are non-evidential and
are not scored).

> **OUTCOME N — NO STRUCTURE: no declared axis reaches |ρ| ≥ 0.7. The
> toy regime law does not transfer to the native thermal field at this
> scope; T1c's mechanism class is disfavoured on the native field.**

**Post-hoc diagnosis (explicitly NON-registered, flagged per discipline):**
the primary axis never left the amplitude regime — mean-subtracted
`τ_mag` was pinned at 0.6–1.9 ticks in every cell regardless of γ, so
`Ω̄·τ_mag` spanned only 0.19–0.86, far below the toy's crossover ≈ 17.
Within that reachable span the measured BF values (0.01–0.05) are
*quantitatively consistent* with the toy law's predictions there
(~0.02–0.05). The structural reading: **native thermal flux noise can
never be slow relative to the flux band, because signal and noise are the
same field** — the Born regime `Ω·τ ≫ 1` is unreachable by construction
on this route. If the mechanism class is to survive, FTD's effective
manifestation noise must have a non-flux origin with correlation time
exceeding the band period (candidates: the s-sector's discrete dynamics;
genesis-history dependence). That reframed question is the registered
next step, not a conclusion. No tag moves; FTD-0200's procedural demand
(a pre-registered engine test of a Born candidate mechanism) is now
**satisfied by execution**, with this negative-at-scope outcome of
record.
