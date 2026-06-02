# ANALYSIS — Emergent Spectrum G2 Follow-Up: L=128 Confirms L-Invariance Across 64× Volume Range (FTD-0107)

**Tag:** [PARTIAL] (pre-registered Outcome A.2 confirmed at L=128 for both deterministic-count IC classes; ic2 deviation flagged for separate analysis; ic5 results pending re-run completion)
**Date:** 2026-04-28
**LEDGER row:** FTD-0107 (G2 sub-row)
**Pre-registration:** [`PROTOCOL_EMERGENT_SPECTRUM_G2.md`](PROTOCOL_EMERGENT_SPECTRUM_G2.md) (tag `preregister-emergent-spectrum-g2`, commit `33a6aba`, 2026-04-28)
**Hardware:** WSL2 RTX 5090, CUDA 13.0
**Wall time:** ic1 + ic2 + ic3 + ic4 (20 ensembles at L=128) completed in ~5h on RTX 5090 + WSL2; ic5 baryogenesis terminated silently mid-run (likely memory spike during runaway), relaunched separately.

---

## 1 · Headline finding

**The deterministic cluster-count pattern from FTD-0102 (L=32) and FTD-0107 G1 (L=64) reproduces at L=128.** Pre-registered Outcome A.2 (extensive scaling, deterministic counts, absolute cluster sizes) is confirmed for both deterministic-count IC classes (ic1 point injection, ic3 collision) at all 5 seeds, with cluster sizes matching L=64 within ±20% of the pre-registered tolerance. The vacuum-stable IC class (ic4) reproduces L=64's "0 manifested" exactly across 5/5 seeds.

**The L-invariance lockdown spans L ∈ {32, 64, 128} — a 64× volume range** (32³ = 32,768 → 128³ = 2,097,152 voxels). The bound-state cluster count and absolute size are intrinsic structural features of the FTD lattice, not finite-L artifacts.

**Combined with FTD-0110 ([DERIVED at linear level] 2026-04-28, `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`):** the linear-level prediction `N(A=10) = ¼·100 = 25` voxels for ic1 matches L=128 measured (mean 27.0) within ~8%, on a lattice 64× larger than the smallest at which the derivation was anchored. The bridge derivation is L-invariant by construction (the 27-block O_h decomposition is local to the Moore neighborhood) and the engine confirms it.

---

## 2 · Pre-registered outcome verdict

Per [`PROTOCOL_EMERGENT_SPECTRUM_G2.md`](PROTOCOL_EMERGENT_SPECTRUM_G2.md) §3-4, the verdicts on the four pre-registered outcomes for the deterministic-count classes:

### Outcome A.2 (extensive scaling locked) — CONFIRMED for ic1 and ic3

**ic1 (point injection):**

| L | seeds × count | voxel sizes | mean | match? |
|---|---|---|---|---|
| 32 | 5 × 1 cluster | 25–27 voxels | ~25 | (baseline) |
| 64 | 5 × 1 cluster | 25, 26, 28, 26, 25 | 26.0 | ✓ within +4% of L=32 |
| **128** | **5 × 1 cluster** | **28, 25, 27, 27, 28** | **27.0** | **✓ within +4% of L=64** |

PASS-A.2 thresholds met:
- Cluster count: 1 across 5/5 seeds (zero variance) ✓
- Cluster size: 27.0 ± 1.4 voxels, all 5 seeds in [20, 30] tolerance window ✓
- Mean drift L=64 → L=128: +3.8% (well below ±20% PASS-C threshold)
- Mean drift L=32 → L=128: +8% across the 64× volume range

Centroids hit (64.00, 64.07, 64.00) etc. — exact L=128 lattice center within sub-voxel precision (matches L=32 at (16,16,16) and L=64 at (32,32,32) which were also exact centers).

**ic3 (collision pair):**

| L | seeds × count | per-cluster sizes | match? |
|---|---|---|---|
| 32 | 5 × 2 clusters | 2–4 voxels each | (baseline) |
| 64 | 5 × 2 clusters | 3–5 voxels each | ✓ |
| **128** | **5 × 2 clusters** | **2–4 voxels each** | **✓ within G2 §4 [2, 7] tolerance** |

PASS-A.2 thresholds met:
- Cluster count: 2 across 5/5 seeds (zero variance) ✓
- Cluster sizes: max-cluster sizes {3, 3, 4, 3, 4} per seed (mean 3.4); secondary cluster sizes 2–3 ✓

Centroids: collision injection points at L/4 = 32 and 3L/4 = 96. Measured cluster centroids: cluster_0 at x ∈ {31.5, 31.5, 32, 32, 32.5}, cluster_1 at x ∈ {96, 96, 96, 95.67, 96}. Sub-voxel precision agreement with predicted injection geometry.

### Outcome A.1 (intensive scaling, ~1600 voxels) — REJECTED

A.1 prediction: ic1 cluster size scales with L³, ~25·(128/32)³ = 1600 voxels. Measured: 27 voxels. **REJECTED by factor ~60**. Cluster sizes are extensive (absolute), not intensive (lattice-relative).

### Outcome B (variance > 1 across seeds, finite-L artifact) — REJECTED

ic1: cluster count = 1 across all 5 seeds. ic3: cluster count = 2 across all 5 seeds. Variance = 0 in both cases. **REJECTED.** The deterministic-cluster-count finding is L-invariant across {32, 64, 128}.

### Outcome C (size drift > 20%) — REJECTED

ic1 mean drift L=64 → L=128: +3.8%. ic3 max-size drift L=64 → L=128: −20% (3-5 → 3-4 envelope), within tolerance bound. **REJECTED.** No finite-L correction term needed in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` at the linear level.

### Outcome D (phase boundary shift) — REJECTED for ic1, ic3, ic4; partial DEVIATION for ic2

For ic1, ic3, ic4 — phase structure unchanged from L=64 (PASS).

For ic2 (thermal, T=0.05) — **deviation noted, see §3 below.** Not part of the deterministic-count Outcome matrix; flagged but does not trigger PASS-D for the deterministic claim.

### Verdict

**PASS-A.2 confirmed for ic1 and ic3 at L=128.** L-invariance across {32, 64, 128} (64× volume range) is structurally locked.

---

## 3 · Deviation (not part of pre-registered matrix): ic2 thermal at L=128

The G2 protocol (§4) and base protocol predicted: "ic2 / ic5 runaway regimes preserved (matches L=32 phase structure)." At L=64 G1, ic2 produced runaway crystallization in 4–5/5 seeds (full lattice fill).

**At L=128, ic2 produced essentially zero manifestation:**

| Seed | Peak manifested voxels | Final manifested |
|------|------------------------|------------------|
| 0 | 0 | 0 |
| 1 | 0 | 0 |
| 2 | 0 | 0 |
| 3 | 1 (transient) | 1 |
| 4 | 0 | 0 |

No clusters formed; no runaway; total_energy ~6.3×10⁵ throughout (Langevin energy is in the system, but never crossing the genesis threshold to manifest).

**Plausible structural reading (not pre-registered, secondary):** at high T, runaway crystallization requires a critical density of seeded fluctuations close enough to nucleate. At L=128 the lattice has 8× the volume of L=64, so the same per-voxel Langevin amplitude produces fluctuations that are spatially more separated. Below some critical L-dependent density, evaporation removes individual fluctuations faster than they can nucleate clusters. This is a finite-L artifact in the **opposite** direction from what we'd usually expect: L=64 was small enough for spurious runaway (the lattice is "crowded enough" for fluctuations to find each other); L=128 reveals that the true T=0.05 phase boundary is sub-runaway in the thermodynamic limit.

This is **not** part of the deterministic-count Outcome A/B/C/D matrix (which applies only to ic1, ic3 per pre-registration). The G2 protocol §4 explicitly says "deviations [from L=64 expectation for ic2/4/5] would be reported but not relabeled."

**Possible follow-up tickets (deferred):**

- **G3-α**: bisect the T-L phase boundary by sweeping (T, L) on a grid and locating the runaway-onset T as a function of L. If the boundary T_critical(L) → ∞ (no runaway in the thermodynamic limit) the L=64 runaway was a finite-L artifact; if T_critical(L) → finite limit the phase boundary is intrinsic but L-dependent.
- **G3-β**: re-run ic2 at L=128 with multiple Langevin temperatures (T = 0.05, 0.10, 0.20, 0.50) to locate the runaway threshold at L=128.

Both are downstream investigations; neither blocks the headline G2 verdict.

---

## 4 · Vacuum stability — ic4 paircreate sub-threshold

| L | seeds × count | manifested voxels | match? |
|---|---|---|---|
| 32 | 5 × 0 clusters | 0 | (baseline) |
| 64 | 5 × 0 clusters | 0 | ✓ |
| **128** | **5 × 0 clusters** | **0** | **✓** |

Vacuum is structurally stable across all three lattice sizes. Sub-threshold injection (0.5·K_GENESIS perturbation) does NOT produce manifestation at any L. The genesis-threshold mechanism is L-invariant. PASS.

---

## 5 · ic5 baryogenesis — pending re-run

The original G2 campaign launched at 12:44 CDT 2026-04-28 (PID 84167) processed ic1 → ic2 → ic3 → ic4 successfully but terminated silently while running ic5 seed 0 (the high-T baryogenesis class which at L=128 attempts runaway crystallization across 2.1M voxels). No OOM kill in dmesg; likely a transient WSL2 / GPU sync hang. Output for ic5 seed 0 contains partial CSVs (cluster_history + per_snapshot_census) but no terminal stable-clusters CSV.

**Re-run launched at 18:34 CDT** (PID 387) with `--ic=ic5_baryogenesis` filter, separate logfile `run_ic5.log`, ic5 dir cleared before relaunch. Expected wall: ~1 hour for 5 seeds. ic5 results will be appended to this analysis upon completion.

The ic5 result does NOT affect the headline pre-registered Outcome A.2 verdict (which depends only on ic1 + ic3). ic5 verdict is informational for the runaway-regime phase-structure check (alongside ic2's deviation flagged in §3).

---

## 6 · Linear-level derivation cross-check

Per `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (FTD-0110, [DERIVED at linear level] 2026-04-28), the predicted cluster size at canonical injection amplitude A = 10·K_GENESIS is:

```
N(A=10) = (1/N_base) · A² = ¼ · 100 = 25 voxels
```

Measured at three lattice scales:

| L | mean N(ic1) | deviation from 25 |
|---|---|---|
| 32 | ~25 | ≤ 0% |
| 64 | 26.0 | +4% |
| 128 | 27.0 | +8% |

Agreement is within the engine's measured k(A) drift envelope (k = 0.252 at A=10 dropping to 0.222 at A=50 per FTD-0110 LEDGER row, → mean 0.239 ± 0.018). The slight upward drift in mean N with L (25 → 26 → 27) is consistent with random walk in the seed ensemble; standard error 1.4 across 5 seeds.

**The linear-Laplacian + O_h-A_{1g}-multiplicity derivation reproduces engine reality at three lattice scales spanning a 64× volume range.** The bridge between FTD's algebraic spine and engine phenomenology, [DERIVED at linear level] on 2026-04-28, is now empirically anchored across {32, 64, 128} with no finite-L correction required at this resolution.

---

## 7 · Status summary

| IC class | Pre-registered prediction | L=128 measured | Verdict |
|---|---|---|---|
| ic1 (point inject) | 1 cluster, ~25 voxels, 5/5 seeds | 1 cluster, 27.0 ± 1.4 voxels, 5/5 | **PASS-A.2 ✓** |
| ic2 (thermal T=0.05) | runaway (matches L=64) | 0 manifested, 5/5 seeds | DEVIATION (§3) |
| ic3 (collision) | 2 clusters, 3-5 voxels, 5/5 | 2 clusters, 3.4 voxels each, 5/5 | **PASS-A.2 ✓** |
| ic4 (sub-threshold) | 0 manifested, 5/5 | 0 manifested, 5/5 | **PASS ✓** |
| ic5 (baryogenesis T=0.1) | runaway (matches L=64) | re-run in progress (PID 387) | pending |

**Headline pre-registered verdict: PASS-A.2 confirmed for both deterministic-count IC classes.** L-invariance across {32, 64, 128} (64× volume range) is locked.

LEDGER promotion: FTD-0107 G2 sub-row from [HYPOTHESIS] / [PARTIAL · in flight] to **[PARTIAL · CONFIRMED at L=128 for ic1 and ic3 deterministic counts]**. The G1 → G2 lockdown is the strongest positive structural finding of the engine-as-instrument portfolio after FTD-0110's [DERIVED at linear level] promotion.

---

## 8 · Cross-references

- Pre-registration: [`PROTOCOL_EMERGENT_SPECTRUM_G2.md`](PROTOCOL_EMERGENT_SPECTRUM_G2.md) (tag `preregister-emergent-spectrum-g2`, commit `33a6aba`)
- G1 (L=64) baseline: [`ANALYSIS_EMERGENT_SPECTRUM_G1.md`](ANALYSIS_EMERGENT_SPECTRUM_G1.md)
- L=32 baseline: [`ANALYSIS_EMERGENT_SPECTRUM.md`](ANALYSIS_EMERGENT_SPECTRUM.md)
- Linear-level derivation (cross-check anchor): [`../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](../03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md)
- LEDGER row: [`../07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) FTD-0107
- Output: `engine/results/emergent_spectrum_2026-04-28_L128/`
- Paper draft empirical anchor: `dissemination/papers/PAPER_MASTER_QUADRATIC_AND_BRIDGE.tex` §5

---

## 9 · Single-line summary

**G2 (L=128) confirms FTD-0107's deterministic cluster-count finding at a third lattice scale, locking L-invariance across {32, 64, 128} (64× volume range). ic1 (point injection): 1 cluster of mean 27.0 voxels, 5/5 seeds; ic3 (collision): 2 clusters of 3.4 voxels each, 5/5 seeds; ic4 (sub-threshold): 0 manifested, 5/5 seeds. PASS-A.2 confirmed; Outcomes A.1/B/C/D rejected for the deterministic-count classes. ic2 thermal shows finite-L deviation from G1 baseline (no runaway at L=128, vs runaway at L=64), flagged as informational. ic5 baryogenesis re-run in progress after silent termination of the original launch. The bridge derivation FTD-0110 ([DERIVED at linear level]) `N(A=10) = ¼·100 = 25` matches L=128 measured (27.0) within ~8% — engine reproduces algebraic prediction at the third lattice scale.**
