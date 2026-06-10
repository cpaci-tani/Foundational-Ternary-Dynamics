# ANALYSIS — The Current-Stack N(A) Law (FTD-0261): broken power law with knee at A≈16; thermostat effect = pure friction

**Tag:** `[MEASURED — current-stack baseline]` (Q1 law) + `[MEASURED — Outcome A]` (Q2 thermostat) + `[CLOSED NEGATIVE — the FTD-0259 thermal-crossover knee reading]`. **Nothing promoted: FTD-0110's cluster↔mass identification stays `[SMC]` with its evidential basis historical; the linear theorem (k = ¼ from O_h) is mathematics, untouched; FTD-0013/MC-T4.3 untouched.**
**Date:** 2026-06-10
**Pre-registration:** [`PREREG_NA_LAW_CURRENT_STACK_v1.md`](PREREG_NA_LAW_CURRENT_STACK_v1.md) (lock commit `be63223e`, tag `preregister-na-law-current-stack-v1`; runner SHA `2795b5b5…`, analysis SHA `270dea76…`)
**Run of record:** `engine/results/na_law_current_stack_2026-06-10/` (34 CSVs + frozen `verdict.txt`; WSL2 canonical build; 0 failures). **V-1 rig gate: PASS 5/5** — the first valid run of the discriminator program.
**LEDGER:** FTD-0261.

---

## 0 · One-paragraph result

On the canonical current stack (coupling-ON protocol, L = 32, thermostat at the historical γ = 0.02, T = 0.005), the ic1 cluster-size law is a **broken power law with a knee at A ≈ 16**: `N ∝ A^3.69` below the knee and `N ∝ A^1.86` above it (log₁₀-RMS 0.037; beats single-power and pure-A² fits by **AIC margin 13.1** — the pre-registered CLEAN-LAW criterion, met decisively). The asymptotic regime is near-quadratic with effective coefficient k_eff ≈ 0.05 — **not** the historical stack's ¼. The thermostat **materially shapes the law** (pre-registered **Outcome A**: median N_X/N_N = 1.61 over 10 common valid points, rising from 1.25 at A = 10 to 3.2 at A = 70, flooding at 90 without friction) — and the dose arms attribute the entire effect to **friction, not noise**: γ-dose at A = 30 is cleanly monotone (N̄ = 57.7 → 45 → 32 for γ = 0.01 → 0.02 → 0.04) while T-dose is **flat** (N̄ = 45.3 / 45.0 / 45.0 / 45.7 for T = 0 → 0.01). The T-flatness **falsifies the FTD-0259 thermal-crossover reading of the knee** (`A* = √(L³T_L) = 12.8` matching the drift onset was coincidence — the measured knee sits at A ≈ 16 and is T-independent); what survives of Mechanism γ is **friction-dissipation**, now `[MEASURED]` as the active thermostat channel.

## 1 · The law (arm N, 11 valid points, A ∈ [10, 90])

| A | 10 | 12 | 14 | 16 | 20 | 25 | 30 | 40 | 50 | 70 | 90 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| N̄ | 4.0 | 8.4 | 16.4 | 21.6 | 27.4 | 32.6 | 45.0 | 91.8 | 130.2 | 260.2 | 383.3 |
| k = N̄/A² | .040 | .058 | .084 | .084 | .069 | .052 | .050 | .057 | .052 | .053 | .047 |

Fits (frozen candidates): L1 `N = 0.0574·A²` (RMS 0.096); L2 `N = 0.0795·A^1.901` (RMS 0.091); **L3 knee@16: p_lo = 3.690, p_hi = 1.861 (RMS 0.037) — winner**. A ≤ 8 is the trivial/sub-threshold anchor regime (N̄ ≤ 1.4, excluded by frozen rule F-2). Interpretation discipline: the steep p ≈ 3.7 onset and the near-2 asymptote are **measured shape parameters of this stack/config**, reported without numerological identification (anti-target rule).

## 2 · The thermostat verdict (arm X + dose arms)

- **Outcome A (frozen rule: median R ≥ 1.5):** R(A) = N_X/N_N = 1.25, 1.75, 1.36, 1.14, 1.44, 1.57, 1.69, 1.61, 2.27, 3.20 at A = 10…70; median **1.61**. Without friction the box retains injected energy and clusters grow systematically larger, diverging with A (A = 90 floods, N̄ = 1461 > the frozen F-1 bound).
- **Attribution (descriptive arms, A = 30):** γ-dose **monotone** (57.7 / 45 / 32 across γ = 0.01 / 0.02 / 0.04); T-dose **flat** (45.3 / 45.0 / 45.0 / 45.7 across T = 0 / 0.0025 / 0.005 / 0.01). The noise channel (σ = √(2γT)) does nothing measurable at canonical parameters; the friction channel carries the whole effect. `[MEASURED]`
- **Consequence for FTD-0259's observation:** the elevated sub-hypothesis "the drift onset = the thermal crossover A* = √(L³·T_L) = 12.8" is **`[CLOSED NEGATIVE]`** — the knee is at A ≈ 16, is T-independent across a 4× T range (including T = 0), and therefore cannot be a thermal floor. The numerical near-match of 12.8 to the historical onset window was a coincidence; FTD-0259's *general* elevation of the thermostat (via the thermostat-active config fact) is **confirmed in its friction half** and falsified in its thermal half.

## 3 · Consequences

1. **The current-stack baseline now exists** `[MEASURED]`: future engine claims about ic1 cluster scaling cite this law (and this run of record), not the stack-pinned historical table.
2. **FTD-0110 re-assessment input:** the asymptotic is near-A² but with k_eff ≈ 0.05, not ¼, and no ¼-plateau exists in either arm. The linear theorem (k = ¼, O_h mathematics) stands as mathematics; its *engine realization* on the current stack at this config is absent — whether a different protocol/config recovers it, and whether an SM cluster↔mass mapping survives under the new law, is the queued follow-up (the identification stays `[SMC]` with historical evidence; no current-stack support added or removed by fiat here).
3. **Mechanism bookkeeping:** α `[CLOSED NEGATIVE]` (FTD-0259); γ-as-thermal `[CLOSED NEGATIVE]` (this run); **γ-as-friction `[MEASURED — active]`**; β (genesis-kink) and front-energetics remain unprobed candidates for the *shape* of the sub-knee onset.
4. **Flooding boundary:** thermostat-free ic1 at A ≥ 90 (L = 32, periodic, no absorber) floods — friction is the operative energy exit; any future thermostat-off campaign above A ≈ 70 needs the absorbing sponge.

## 4 · Scope

Q1/Q2 are engine-level measurements of the canonical stack at the stated config; no physical-units claim is made (calibration register applies); nothing here bears on α, FTD-0013, or the spine.
