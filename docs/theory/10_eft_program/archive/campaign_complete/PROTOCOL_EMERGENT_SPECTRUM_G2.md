# PROTOCOL — Emergent Spectrum G2 Follow-Up: L=128 Confirmation

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-28
**LEDGER row:** FTD-0107 (G2 sub-row)
**Companion:** [`PROTOCOL_EMERGENT_SPECTRUM_G1.md`](PROTOCOL_EMERGENT_SPECTRUM_G1.md) (the L=64 G1 protocol; this G2 is a tight delta from it), [`ANALYSIS_EMERGENT_SPECTRUM_G1.md`](ANALYSIS_EMERGENT_SPECTRUM_G1.md) (G1 results)

This protocol is **pre-registered before measurement** per CLAUDE.md epistemic discipline. The git tag `preregister-emergent-spectrum-g2` will be applied at this commit, BEFORE any L=128 production run.

---

## 1 · Why this protocol exists

`ANALYSIS_EMERGENT_SPECTRUM_G1.md` (FTD-0107, 2026-04-27) confirmed Outcome A.2 of the G1 pre-registration: at L=64, ic1 produces exactly 1 cluster of ~25 voxels across 5/5 seeds (matching L=32 absolute size), ic3 produces exactly 2 clusters of 3-5 voxels each across 5/5 seeds. Cluster sizes are **absolute** (extensive scaling), not lattice-relative. The deterministic-cluster-count finding holds at L ∈ {32, 64}.

Subsequent work (2026-04-28, FTD-0110 [DERIVED at linear level], `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`) showed the cluster-efficiency coefficient `k = 1/N_base = 1/4` derives from O_h representation theory: `mult(A_{1g}) = 4` in the 27-dim permutation rep on the 3³ Moore block; δ_center is A_{1g}-pure; the 18-pt Laplacian projects onto 4 A_{1g} eigenmodes with mean energy fraction 1/4. The derivation is L-independent at the linear level (the 27-block is local, the A_{1g} subspace is intrinsic to O_h).

**G2's role is to test that L-invariance one step further** — from {32, 64} to {32, 64, 128}. The L=128 lattice is 64× larger by volume than L=32 and 8× larger than L=64. If the deterministic cluster counts hold at L=128 with the same absolute sizes (~25 voxels for ic1, 3-5 for ic3), the L-invariance claim is structurally locked at three lattice scales spanning a 64× volume range. If sizes drift, the linear-level derivation needs re-examination at finite-L corrections. If counts vary across seeds, the deterministic-count claim closes negative as an artifact of L ∈ {32, 64}.

**The investigation question (NOT a claim):** does the L-invariant deterministic cluster pattern (ic1: 1 cluster ~25 voxels; ic3: 2 clusters 3-5 voxels) confirmed at L ∈ {32, 64} reproduce at L=128?

---

## 2 · Pre-registered scope

**Reuse G1 PROTOCOL** (`PROTOCOL_EMERGENT_SPECTRUM_G1.md`) with one change:

1. **L = 128** instead of L = 64

All other parameters carry over from G1 unchanged: N_BURN = 200, N_SAMPLES = 50, SAMPLE_STRIDE = 50, stable_threshold = 100 ticks, 5 seeds × 5 IC classes = 25 ensembles, GPU on RTX 5090 via WSL2 (`engine/build_wsl/`).

**Estimated wall time:** L=64 took 54 min for 25 ensembles (G1 actual). L=128 has 8× volume; per-tick GPU time scales sub-linearly with volume on RTX 5090 (per G1 §2 reasoning). Realistic estimate: **~4–8 GPU hours**. Allow up to 12 hours for safety margin given the 8× volume jump and finite hostdevice snapshot overhead.

---

## 3 · Pre-registered prediction matrix

The L=64 G1 results land Outcome A.2 (extensive scaling, absolute sizes ~25 / 3-5 voxels). G2 pre-registers **four binned outcomes for L=128** that mirror G1 §3 with explicit numerical thresholds tightened to reflect the now-confirmed L=64 baseline:

### Outcome A — L-invariance holds at L=128 (structural confirmation)

- **A.1 (intensive scaling, REJECTED at L=64; included for completeness):** ic1 cluster size scales with L³ → at L=128 expect ~25·(128/32)³ = 1600 voxels.
- **A.2 (extensive scaling, CONFIRMED at L=64):** ic1 at L=128 produces 1 cluster of 25±5 voxels across 5/5 seeds; ic3 produces 2 clusters of 3-5 voxels each across 5/5 seeds.

A.2 is the strongly-favored sub-outcome given G1 results. Reproducing A.2 at L=128 **structurally locks** L-invariance across {32, 64, 128} (64× volume range).

### Outcome B — Cluster counts vary across seeds at L=128 (finite-L artifact closure)

- ic1 at L=128: cluster count varies (some seeds give 1, some give 2 or more).
- ic3 at L=128: cluster count varies.

This would close the deterministic-cluster finding as an artifact of L ∈ {32, 64}. **Honest negative result.**

### Outcome C — Cluster sizes drift (finite-L correction surfaces)

- Cluster *count* stays deterministic (1 for ic1, 2 for ic3, 5/5 seeds).
- Cluster *size* drifts away from ~25 voxels (e.g., ~30 or ~20 at L=128).

This would mean the absolute-size claim has finite-L corrections that were below threshold at L=64 but visible at L=128. The linear-level derivation `N(A) = ¼·A²` from `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` would need a finite-L correction term.

### Outcome D — Phase boundary shifts (vacuum or all-runaway)

- ic1, ic3 at L=128 either produce no manifested voxels or runaway crystallization.

Falsifier for the existence of the bound-state regime at L=128 with the L=64 IC parameters.

**Pre-registered acceptance**: each outcome has explicit numerical match criteria. ±1 cluster count tolerance per seed; consistency across all 5 seeds required for "deterministic" verdict; ±20% size tolerance for A.2 confirmation; >20% size drift triggers Outcome C tag.

---

## 4 · Pre-registered falsifier (mandatory)

For each IC class (ic1, ic3 — the two that produced deterministic counts at L ∈ {32, 64}):

- **PASS-A.2** (extensive scaling locked): mean cluster count matches G1 prediction within ±1 across 5/5 seeds, AND mean cluster size matches L=64 within ±20% (i.e., ic1 in [20, 30] voxels; ic3 in [2, 7] voxels). **L-invariance structurally confirmed at three scales.**
- **PASS-A.1** (intensive scaling, surprise): cluster size scales with L³, ~1600 voxels for ic1. **Would re-open the linear-level derivation; almost certainly will NOT trigger given G1 result.**
- **PASS-B**: cluster count variance > 1 across 5 seeds → **closes deterministic-cluster finding as finite-L artifact of L ∈ {32, 64}**.
- **PASS-C**: count deterministic but size drifts >20% → **finite-L correction needed in linear-level derivation; opens new theoretical work.**
- **PASS-D**: vacuum or runaway → phase boundary is L-dependent past L=64.

For ic2, ic4, ic5 (the runaway-crystallization, vacuum, and high-T regimes), the prediction is straightforward: **same behavior as L ∈ {32, 64} expected**. Deviations would be reported but not relabeled.

---

## 5 · Pre-registered ensemble parameters (locked)

| Parameter | Value | Source |
|---|---|---|
| L | 128 | this protocol |
| N_BURN | 200 ticks | G1 §5, base protocol §3 |
| N_SAMPLES | 50 | G1 §5, base protocol §3 |
| SAMPLE_STRIDE | 50 ticks | G1 §5, base protocol §3 |
| stable_threshold | 100 ticks | G1 §5, base protocol §3 |
| N_SEEDS | 5 | G1 §5, base protocol §3 |
| IC classes | ic1_inject, ic2_thermal, ic3_collision, ic4_paircreate, ic5_baryogenesis | G1 §5, base protocol §2 |
| Toggles | wave_propagation, gauss_projection, genesis, langevin, dual_substrate=false | G1 §5, base protocol §3 |
| Langevin T | per-IC (0.005 default; 0.05 ic2; 0.1 ic5) | G1 §5, base protocol §2 |
| Output dir | `engine/results/emergent_spectrum_2026-04-28_L128/` | this protocol |

Engine binary: `engine/build_wsl/campaign_emergent_spectrum_2026-04-27` (verified post-refactor parity per `b6e9b58`, 2026-04-28). No source changes required — `--L=128 --output-dir=...` CLI flags already supported.

---

## 6 · Implementation checklist

1. [x] G1 PROTOCOL committed and tagged (`preregister-emergent-spectrum-g1`, `37ea371`, 2026-04-27)
2. [x] G1 ANALYSIS committed (FTD-0107 [PARTIAL · Outcome A.2 confirmed], 2026-04-27)
3. [x] FTD-0110 [DERIVED at linear level] committed (`306837c`, 2026-04-28) — provides theoretical anchor for the L-invariance claim being tested at L=128
4. [x] WSL2 build verified post-refactor (b6e9b58, 2026-04-28)
5. [x] PROTOCOL_EMERGENT_SPECTRUM_G2.md committed (this commit)
6. [x] REF_PREREGISTER_MANIFEST.md updated with G2 row (this commit)
7. [ ] `git tag preregister-emergent-spectrum-g2` applied at this commit — **MANDATORY GATE**
8. [ ] Tag pushed
9. [ ] Production run: `engine/build_wsl/campaign_emergent_spectrum_2026-04-27 --L=128 --seeds=5 --samples=50 --burn=200 --stride=50 --output-dir=engine/results/emergent_spectrum_2026-04-28_L128/` via WSL2 + RTX 5090
10. [ ] ANALYSIS_EMERGENT_SPECTRUM_G2.md written comparing L ∈ {32, 64} vs L=128 under the pre-registered outcome bins

---

## 7 · Anti-targets (locked)

This protocol **WILL NOT**:

- Adjust the prediction matrix (§3) after seeing measurement results
- Promote any outcome to [DERIVED]/[THEOREM]-grade without seed-replicated consistency (5/5 seeds for the deterministic claim) AND size-tolerance match
- Treat L=128 cluster sizes as "matching L=64" without explicit ±20% check
- Ignore Outcome B (negative result) if the data shows it — variance > 1 across seeds is a clean closure
- Re-interpret a PASS-C (size drift) outcome as a "weakened A.2 confirmation" — size drift triggers the Outcome C tag and opens new theoretical work, full stop
- Conflate this G2 with the FTD-0110 nonlinear-bridge proof (G2 is empirical L-invariance lockdown; the nonlinear bridge proof is a separate [OPEN] item per `WHERE_WE_LEFT_OFF.md` §5 Option 1)

This protocol **WILL**:

- Lock parameters and falsifiers BEFORE measurement
- Apply tag `preregister-emergent-spectrum-g2` at this commit BEFORE engine run
- Report measured cluster counts and sizes with bootstrap stderr
- Write ANALYSIS comparing L ∈ {32, 64, 128} under the pre-registered outcome bins
- Tag results honestly per CLAUDE.md epistemic ladder

---

## 8 · Single-line summary

**Pre-registers L=128 follow-up to FTD-0107 G1 (which confirmed deterministic cluster counts and absolute sizes at L=64). Tests whether the L-invariance pattern (ic1: 1 cluster ~25 voxels 5/5 seeds; ic3: 2 clusters 3-5 voxels 5/5 seeds) extends to L=128 (Outcome A.2: structural lock across 64× volume range), drifts in size (Outcome C: finite-L correction needed in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`), or varies across seeds (Outcome B: closes the L ∈ {32, 64} result as artifact). Reuses G1's parameters; only L doubles from 64 to 128. Estimated wall: ~4–8 GPU hours on RTX 5090 + WSL2. LEDGER row FTD-0107 G2 sub-row; promotes G1's [PARTIAL] toward [STRUCTURALLY MOTIVATED] (PASS-A.2) or closes negative (PASS-B/D) post-measurement. Pre-reg gate `git tag preregister-emergent-spectrum-g2` at this commit; mandatory before campaign launch.**
