# ANALYSIS — SM ClusterMass Identification on the Current Stack (FTD-0262): IDENT-NULL

**Tag:** `[MEASURED — IDENT-NULL]`: the electron anchor holds `[MEASURED]`, law self-consistency holds `[MEASURED — circular, as pre-flagged]`, and the specialness probe returns **SMOOTH** — the current stack assigns **no structural distinction to the SM mass ratios**. **FTD-0110's clustermass identification stays `[SMC]`** with its quantitative SM evidence **historical/stack-pinned only**; the current-stack support is exactly: the anchor. **Nothing promoted; nothing demoted by fiat — this documents the evidence basis.**
**Date:** 2026-06-10
**Pre-registration:** [`PREREG_SM_MASS_IDENT_CURRENT_STACK_v1.md`](PREREG_SM_MASS_IDENT_CURRENT_STACK_v1.md) (lock `2adf80b1`, tag `preregister-sm-mass-ident-current-stack-v1`; priors: IDENT-NULL 65 % — **landed**)
**Run of record:** `engine/results/sm_mass_ident_2026-06-10/` (15 CSVs + frozen `verdict.txt`; 65 seed-runs, 0 failures; canonical protocol, WSL2 build).
**LEDGER:** FTD-0262.

---

## 0 · One-paragraph result

On the canonical current stack, the three frozen layers returned: **E — PASS** (the electron anchor: 20/20 seed-runs across A ∈ {1.5, 2, 3, 5} yield a largest cluster of **exactly 1 voxel, time-stable** — the R = 1  minimal-manifestation identification, the discrete-object face of the `M_REST = m_e` calibration, is robust); **S — CONSISTENT** (the FTD-0261 law, inverted at R_μ and R_π, predicted the off-grid points to **3.6 % and 3.0 %**: N̄(62.59) = 199.4 vs 206.8, N̄(72.46) = 281.3 vs 273.1 — pre-flagged as circular for the *identification*, but a genuine extrapolation success for the *law*); **P — SMOOTH** (the verdict-bearing probe: local slope **p_local = 2.052** across the 7-point μ-window, window-mean 204.2 sitting right at R_μ yet with no plateau; the π-window slope 1.983 concurs). **Outcome: IDENT-NULL** — N(A) passes through the muon mass ratio exactly as it passes through every other value; the engine assigns the SM ratios no attractor structure. The clustermass identification therefore gains **no current-stack support beyond the electron anchor**, and its quantitative SM evidence (the historical 5 % five-particle match) remains a property of the stack-pinned pre-correction engine.

## 1 · Layer verdicts (numbers from the frozen analysis)

| Layer | Result | Detail |
|---|---|---|
| **E anchor** | **PASS** | `[n_min, n_max] = [1, 1]` for every seed at every anchor amplitude — exact, stable, band-robust |
| **S self-consistency** (circular, pre-flagged) | **CONSISTENT** | μ: N/R = 0.964; π: N/R = 1.030 (band [0.843, 1.186]) — the law extrapolates off-grid at 3–4 % |
| **P specialness** (verdict-bearing) | **SMOOTH** | p_local = 2.052 (SMOOTH threshold ≥ 0.95; law slope ≈ 1.9); N rises 166.7 → 248.3 monotonically across the window with no flat segment; π-window 1.983 |

## 2 · What this settles, and what it does not

1. **Settled `[MEASURED]`:** the current stack carries a clean two-parameter scaling law through the SM region with no quantization, plateaus, or attractor structure at R_μ (and descriptively none at R_π). An identification with structural force would have required the SM value to be special; it is not. The FTD-0261 law also earns an off-grid extrapolation validation (3–4 %) as a by-product.
2. **Settled (documentation):** FTD-0110's `[SMC]` identification now has its evidence basis stated precisely — **historical stack-pinned quantitative matches + the current-stack electron anchor + nothing else.** The tag does not move (it already said conjecture); what moves is the honesty of the support inventory.
3. **Not settled:** K/p/τ (require A ∈ [141, 278] — beyond the measured law and L = 32 validity; an L-scan follow-up *if* warranted, which IDENT-NULL makes less urgent); the sub-knee onset mechanism (A ≈ 16, p ≈ 3.7 — β/front-energetics candidates); whether any protocol/config realizes the linear theorem's ¼ on the current stack.
4. **Scope:** engine-level; calibration register applies; nothing here bears on the algebraic mass formulas (m_μ/m_e = 207 from framework integers etc. — those are LEDGER-tagged independently and never depended on the engine), on α, FTD-0013, or the spine.
