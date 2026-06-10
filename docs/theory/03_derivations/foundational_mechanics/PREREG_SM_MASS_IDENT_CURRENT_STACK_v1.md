# PREREG — SM Cluster↔Mass Identification Re-Assessment on the Current Stack v1

**Status:** `[PRE-REGISTRATION — design locked before any run]`
**Date:** 2026-06-10
**LEDGER row:** FTD-0262 (reserved)
**Provenance:** the remaining half of the FTD-0260/0261 successor item. FTD-0110's cluster↔mass identification (`mass = N·m_e`; SM matches at A = 2√R) is `[SMC]` with **stack-pinned historical evidence only**; FTD-0261 measured the current-stack law (broken power, knee A≈16, `N = 0.0795·A^1.901` global). This campaign asks what identification content survives on the canonical stack — designed explicitly to separate circular from non-circular evidence (the anti-target rule forbids fishing; every criterion is frozen here).
**Runner (frozen, unchanged from FTD-0261):** `engine/tests/campaign_thermostat_off_sweep.cpp` — SHA256 `2795b5b52af27cfb8a684ba7ac08b17dd9db5c6761d25b039cbadcdbe9667bc4`
**Analysis (frozen):** `scripts/exploration/analyze_sm_mass_ident_current_stack.py` — SHA256 `502bedb8cac3001737b994dffbda916dfda0bcb0aed874fac9d4c073a95e3137`
**Git tag:** `preregister-sm-mass-ident-current-stack-v1` (applied at the lock commit).

## 1 · The three layers (frozen), and what each is worth

| Layer | Arm | Test | Evidential status |
|---|---|---|---|
| **E — anchor** | A ∈ {1.5, 2, 3, 5} × 5 seeds | the electron (R = 1) as the **1-voxel minimal manifestation**: every seed, every A in the band, largest cluster **exactly 1**, time-stable (n_min = n_max = 1) | **non-circular** — ties the calibration `M_REST = m_e` to a discrete engine object |
| **S — self-consistency** | A_μ = 62.59, A_π = 72.46 (inverting the frozen FTD-0261 L2 fit at R_μ = 206.7683, R_π = 273.132) × 5 seeds | `N̄/R ∈ [1/1.186, 1.186]` (the law's 2σ residual band) for both | **CIRCULAR — flagged**: tests law extrapolation, not the identification; pre-stated as such (any value on the law "matches" itself) |
| **P — specialness** | μ-window {56, 58, 60, 62.59, 64, 66, 68} × 3 seeds (π-window {70, 72.46, 74, 76} descriptive) | local log-log slope `p_local` on the window: **SMOOTH** iff p_local ≥ 0.95; **PLATEAU-AT-R** iff p_local < 0.95 AND window-mean N within 10 % of R_μ; else STRUCTURED-ELSEWHERE | **the real content** — an identification with structural force requires the SM value to be *special* (an attractor/plateau), not merely a point the smooth law passes through |

Protocol: the canonical FTD-0261 arm-N config (L = 32, coupling ON, thermostat ON γ = 0.02 / T = 0.005, burn 200, 500-tick window stride 10, seeds 0xE0102000+s). Scope: **μ and π only** — K/p/τ require A ∈ [141, 278], beyond the measured law's grid (≤ 90) and into flooding/finite-L territory at L = 32; they are an L-scan follow-up, not this campaign.

## 2 · Outcome map (mechanical; priors stated)

- **IDENT-NULL (prior 65 %):** E-PASS + S-CONSISTENT + SMOOTH ⇒ the identification keeps the electron anchor and law self-consistency but **gains no structural specialness**: its quantitative SM evidence remains historical/stack-pinned. Consequence: a documentation-grade clarification of FTD-0110's `[SMC]` evidential basis (no tag change — the tag already says conjecture).
- **IDENT-STRUCTURE (prior 5 %):** PLATEAU-AT-R ⇒ an attractor at the muon mass ratio on the current stack — would require an independent pre-registered confirmation before any further claim.
- **IDENT-BROKEN (prior 15 %):** E-FAIL or S-INCONSISTENT ⇒ the identification loses even its anchor/self-consistency on the canonical stack; the evidential note hardens.
- **MIXED (prior 15 %):** anything else; component verdicts reported, no closure.

**Under every outcome: FTD-0110 stays `[SMC]`; no promotion anywhere; the linear theorem and FTD-0013/MC-T4.3 untouched.**

## 3 · Hygiene rules

F-1: flooding guard N̄ > 1000 excludes a point. F-2: no re-runs/seed adjustments; first valid run is the run of record. F-3: the P-thresholds (0.95, 10 %) are frozen; any change requires v2. F-4: no scanning beyond the declared windows (anti-target: a wider search for plateaus anywhere would be a coincidence hunt and is banned).
