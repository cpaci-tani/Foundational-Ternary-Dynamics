# PROTOCOL — Emergent Spectrum G1 Follow-Up: L=64 Multilatitude Rerun

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-27
**LEDGER row:** FTD-0107
**Companion:** [`PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md`](PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md) (the L=32 base protocol), [`ANALYSIS_EMERGENT_SPECTRUM.md`](ANALYSIS_EMERGENT_SPECTRUM.md) (the L=32 results)

This protocol is **pre-registered before measurement** per CLAUDE.md epistemic discipline. The git tag `preregister-emergent-spectrum-g1` will be applied at this commit, BEFORE any L=64 production run.

---

## 1 · Why this protocol exists

`ANALYSIS_EMERGENT_SPECTRUM.md` (FTD-0102, 2026-04-27) recovered three structural findings at L=32 from the engine-as-instrument emergent-particle-spectrum campaign:

1. **Three-regime phase structure**: stable vacuum (ic4), deterministic bound states (ic1, ic3), runaway crystallization (ic2, ic5).
2. **Deterministic cluster counts** in the bound-state regime: ic1 (point injection) → exactly 1 cluster of 25-27 voxels, 5/5 seeds; ic3 (collision) → exactly 2 clusters of 3-4 voxels each, 5/5 seeds.
3. **Q-conservation breaks at the phase boundary** (ic2/ic5) but is preserved in steady-state regimes.

The deterministic-cluster-count finding (#2) is the **most novel positive structural finding of the entire engine-as-instrument portfolio**. Per `WHERE_WE_LEFT_OFF.md` priority queue, it's the highest-leverage thread queued. The G1 follow-up tests whether this pattern persists at L=64.

**The investigation question (NOT a claim):** does the L=32 deterministic cluster pattern (1 from point, 2 from collision) reproduce at L=64? If yes, **structural finding** — the lattice has a discrete number of bound-state slots determined by IC topology, not parameter tuning. If no, the pattern is an L-dependent finite-size artifact and the L=32 result was a coincidence of ratio (cluster size ~ 25 voxels at L=32^3 = 32768 voxels gives ~0.08% lattice fill).

---

## 2 · Pre-registered scope

**Reuse base PROTOCOL** (`PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md`) with three changes:

1. **L = 64** instead of L = 32
2. **Maintain ticks**: N_BURN = 200, N_SAMPLES = 50, SAMPLE_STRIDE = 50 (same as base)
3. **Maintain seeds**: 5 seeds × 5 IC classes = 25 ensembles (same)

All other parameters (IC class definitions, threshold latencies, stable-cluster threshold ≥ 100 ticks, GPU on RTX 5090) carry over from the base protocol unchanged.

Estimated wall time: L=32 took ~30 min for 25 ensembles. L=64 has 8× volume; per-tick GPU time scales sub-linearly with volume due to better parallelism, but more host↔device sync per snapshot. Realistic estimate: **~2-4 GPU hours**.

---

## 3 · Pre-registered prediction matrix

The L=32 results give us four binned outcome candidates for L=64. **Pre-register all four BEFORE measurement**:

### Outcome A — Deterministic cluster counts persist (structural finding)

- ic1 at L=64: **1 cluster, 5/5 seeds**, voxel count proportional to L³ (i.e., 25 × (64/32)³ = 200 voxels) OR fixed (25 voxels regardless of L)
- ic3 at L=64: **2 clusters, 5/5 seeds**, similar voxel-count scaling

Either of two sub-cases lands Outcome A:
- **A.1 (stronger)**: cluster sizes scale with L³ → "intensive" bound state, the lattice has fixed number of slots
- **A.2 (weaker)**: cluster sizes stay near 25 / 3-4 → "extensive" bound state, sizes are absolute

Either form of A is a **structural finding**: deterministic cluster counts are L-invariant, so they reflect IC topology (point vs collision) not lattice scale.

### Outcome B — Cluster counts vary across seeds at L=64 (finite-L artifact)

- ic1 at L=64: cluster count varies (some seeds give 1, some give 2 or more)
- ic3 at L=64: cluster count varies (some give 2, some give 3 or 4)

This would close the deterministic-cluster finding as an L=32 finite-size artifact. **Honest negative result.**

### Outcome C — Cluster counts shift uniformly (L-dependent topology)

- ic1 at L=64: deterministic but DIFFERENT count (e.g., always 2 instead of always 1)
- ic3 at L=64: deterministic but DIFFERENT count (e.g., always 3 instead of always 2)

This would mean the count is L-dependent in a structured way. Interesting but harder to interpret.

### Outcome D — Phase boundary shifts (no clusters or all-runaway)

- ic1, ic3 at L=64 either produce no manifested voxels (vacuum) or runaway crystallization (full lattice fill)

This would mean the phase boundary T_critical is L-dependent. Falsifier for the existence of the bound-state regime at L=64 with the L=32 IC parameters.

**Pre-registered acceptance**: each outcome has explicit numerical match criteria. ±1 cluster count tolerance per seed; consistency across all 5 seeds required for "deterministic" verdict.

---

## 4 · Pre-registered falsifier (mandatory)

For each IC class (ic1, ic3 — the two that produced deterministic counts at L=32):

- **PASS-A.1** (intensive scaling): mean cluster count matches L=32 prediction within ±1 across 5/5 seeds, AND mean cluster size scales with L³ within ±20%.
- **PASS-A.2** (extensive scaling): mean cluster count matches L=32 prediction within ±1 across 5/5 seeds, AND mean cluster size matches L=32 within ±50%.
- **PASS-B**: cluster count variance > 1 across 5 seeds → **closes deterministic-cluster finding as finite-L artifact**.
- **PASS-C**: deterministic but shifted count → report new L-dependent count, secondary investigation.
- **PASS-D**: vacuum or runaway → phase boundary is L-dependent.

For ic2, ic4, ic5 (the runaway-crystallization, vacuum, and high-T regimes), the prediction is straightforward: **same behavior as L=32 expected** (vacuum at sub-threshold; runaway at high T). Deviations from this would be reported but not relabeled.

---

## 5 · Pre-registered ensemble parameters (locked)

| Parameter | Value | Source |
|---|---|---|
| L | 64 | this protocol |
| N_BURN | 200 ticks | base protocol §3 |
| N_SAMPLES | 50 | base protocol §3 |
| SAMPLE_STRIDE | 50 ticks | base protocol §3 |
| stable_threshold | 100 ticks | base protocol §3 |
| N_SEEDS | 5 | base protocol §3 |
| IC classes | ic1_inject, ic2_thermal, ic3_collision, ic4_paircreate, ic5_baryogenesis | base protocol §2 |
| Toggles | wave_propagation, gauss_projection, genesis, langevin, dual_substrate=false | base protocol §3 |
| Langevin T | per-IC (0.005 default; 0.05 ic2; 0.1 ic5) | base protocol §2 |
| Output dir | `engine/results/emergent_spectrum_2026-04-27_L64/` | this protocol |

Engine binary: existing `engine/build_wsl/campaign_emergent_spectrum`, no source changes required (CLI flag `--L=64` covers it).

---

## 6 · Implementation checklist

1. [x] Base PROTOCOL committed (2026-04-27, FTD-0102)
2. [x] Base ANALYSIS committed (FTD-0102)
3. [x] PROTOCOL_EMERGENT_SPECTRUM_G1.md committed (this commit)
4. [x] LEDGER FTD-0107 [HYPOTHESIS] row added (this commit)
5. [ ] `git tag preregister-emergent-spectrum-g1` applied at this commit — **MANDATORY GATE**
6. [ ] Tag pushed
7. [ ] Production run: `./engine/build_wsl/campaign_emergent_spectrum --L=64 --seeds=5 --samples=50 --burn=200 --stride=50` on RTX 5090
8. [ ] ANALYSIS_EMERGENT_SPECTRUM_G1.md written with L=32 vs L=64 comparison

---

## 7 · Anti-targets (locked)

This protocol **WILL NOT**:

- Adjust the prediction matrix (§3) after seeing measurement results
- Promote any outcome to [SELECTION] without seed-replicated consistency (5/5 seeds for the deterministic claim)
- Treat L=64 cluster sizes as "matching L=32" without explicit ±20% (intensive) or ±50% (extensive) check
- Ignore Outcome B (negative result) if the data shows it — variance > 1 across seeds is a clean closure
- Bundle this with the FTD-0106 G\*/π scan (FTD-0107 is independent; G\*/π scan stays at [HYPOTHESIS] until its own follow-ups land)

This protocol **WILL**:

- Lock parameters and falsifiers BEFORE measurement
- Apply tag `preregister-emergent-spectrum-g1` at this commit BEFORE engine run
- Report measured cluster counts and sizes with bootstrap stderr
- Write ANALYSIS comparing L=32 (existing) vs L=64 (new) under the pre-registered outcome bins
- Tag results honestly per CLAUDE.md epistemic ladder

---

## 8 · Single-line summary

**Pre-registers L=64 multilatitude rerun of FTD-0102's emergent-spectrum campaign. Tests whether the deterministic cluster counts (ic1: 1 cluster 5/5 seeds; ic3: 2 clusters 5/5 seeds) at L=32 persist at L=64 (Outcome A: structural finding) or vary across seeds (Outcome B: closes the L=32 result as finite-L artifact). Reuses the base protocol's IC classes, toggles, and tick budget; only L doubles. Estimated wall: ~2-4 GPU hours on RTX 5090. LEDGER row FTD-0107 [HYPOTHESIS]; promotes to [PARTIAL] / [CLOSED NEGATIVE] post-measurement. Pre-reg gate `git tag preregister-emergent-spectrum-g1` at this commit; mandatory before campaign launch.**
