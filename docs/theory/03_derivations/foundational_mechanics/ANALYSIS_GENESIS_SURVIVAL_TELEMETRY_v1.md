# ANALYSIS — Genesis-vs-survival engine telemetry (FTD-0267): SURVIVAL-NULL — the β arc's premise is falsified; genesis is a one-shot throttle, not a survival bottleneck

**Tag:** `[MEASURED — SURVIVAL-NULL per frozen rules]`; the β arc (FTD-0265/0266) post-genesis-survival conclusion is **`[CLOSED NEGATIVE]`**; the true mechanism (nonlinear genesis throttling) is the sharpened `[OPEN]`. **Nothing promoted.**
**Date:** 2026-06-10
**Platform:** canonical C++ engine, **CPU backend** (genesis/evap event counters are CPU-only; the GPU kernel path does not populate the host-side counters). **Current stack: post-`c2a8f606`** (the concurrent session's backend-parity refactor — see §4).
**Instrument:** `engine/tests/campaign_genesis_trajectory.cpp` + two observation-only event counters in `engine/src/render_bridge_phases/phase_write.cpp` (`genesis_events_this_tick_` / `evaporation_events_this_tick_`, accessors on `RenderBridge`). **Counters proven observation-only:** golden hash bit-identical with and without them (`0xebaa6f314f66db3f`, stash/unstash test, §4).
**Analyzer:** `scripts/exploration/analyze_genesis_trajectory.py`. **Run of record:** `engine/results/genesis_trajectory_2026-06-10/`.
**LEDGER:** FTD-0267. **Depends on:** FTD-0265/0266 (the β arc this tests), FTD-0263 (the onset profile), FTD-0261 (the law).

---

## 0 · One-paragraph result

The β derivation arc concluded the sub-knee onset suppression is **100% post-genesis survival** — at A=10 roughly 23 voxels fire genesis but only ~4 survive (~17% survival efficiency, inferred to rise to ~40% at A=14 as a nucleation signature). The first-ever **direct** engine measurement of genesis and evaporation *events* falsifies that premise. On the current canonical stack (L=64, 4 seeds), at A=10 the engine fires only **~5 genesis events** (seeds {3,7,5,4}), not ~23, and **`cumulative_genesis == peak_manifested` in every single run** — genesis is a **one-shot early burst** (firing confined to ticks 0–~15, then it stops entirely; there is no sustained genesis⇄evaporation equilibrium). Evaporation is **near-zero** below A=30 (cum_evap=1 at A≤14), so survival is **high (67–93%)**, not the low 17% β required. The cluster size simply ≈ the genesis-firing count. **Verdict: SURVIVAL-NULL** (the peak-and-decay signature S1 is absent; ¬S1). The β arc's engine-free envelope (FTD-0265) and dwell-time (FTD-0266) models both over-predicted genesis by ~4–5× because they modeled a *linear* wave and ignored the engine's nonlinear flux throttling: genesis consumes flux (`flux *= 1 − K_GENESIS/|J|`), the state↔flux coupling drains it, the Gauss projection redistributes it, and damping/Langevin friction bleeds it — so neighbours rarely sustain density above K_GENESIS. **The suppression is at the genesis stage, not post-genesis survival.**

## 1 · The measurement (frozen bands, stated before compute)

| Test (frozen) | Band | Measured (A=10) | Result |
|---|---|---|---|
| P1 peak manifested | [12, 30] | 3 | **FAIL** |
| P2 steady cluster | [2, 8] | 1.0 | **FAIL** |
| P3 peak/steady ≥ 2.0 | — | 3.0 | pass (but not via decay — disconnected voxels) |
| P4 cumulative genesis | [15, 60] | 3 (seeds mean 4.8) | **FAIL** |
| S1 peak-and-**decay** | drop ≥ 50% | peak 3 → steady 2 | **FAIL** |
| S2 survival↑ with A | A14 > A9 | 0.93 > 0.33 | pass (but artifact — see §3) |
| S3 steady gen ≈ evap > 0 | both > 0 | gen 0, evap 0 | **FAIL** |

**VERDICT: SURVIVAL-NULL** (¬S1; the β post-genesis conclusion is refuted).

## 2 · The amplitude trajectory (single-seed canonical, L=64)

| A | cum_gen | peak_manif | burst_end (tick) | steady cluster | cum_evap | survival eff |
|---|---|---|---|---|---|---|
| 9  | 3  | 3  | 5  | 1.0  | 1  | 0.33 |
| 10 | 3  | 3  | 5  | 1.0  | 1  | 0.33 |
| 14 | 15 | 15 | 14 | 14.0 | 1  | 0.93 |
| 30 | 47 | 47 | 33 | 33.5 | 16 | 0.71 |

Multi-seed cum_gen: A=10 → {3,7,5,4} (mean 4.8); A=14 → {15,18,15,17} (mean 16.2). The one-shot-burst structure (cum_gen = peak_manif) holds across all 8 seed-runs. Only at A=30 does post-burst evaporation become non-trivial (47→33), the lone amplitude where any "survival" attrition operates — and even there it is mild.

## 3 · What this means

- **β's premise is the error, not its survival sub-model.** Both β variants assumed the linear envelope's threshold-crossing count (~23 at A=10) was the genesis count. The engine fires ~5. The "missing ~19 voxels" β attributed to post-genesis evaporation never manifest in the first place. The envelope/dwell-time route was unfaithful to the nonlinear engine. `[CLOSED NEGATIVE]` for the post-genesis-survival reading.
- **The N(A) law is a genesis-throttling law.** Cluster size ≈ number of genesis firings in the one-shot burst. The open question for the onset is therefore: *how many voxels does the full nonlinear injection push above K_GENESIS before flux consumption + coupling + Gauss + damping quench the pulse?* This is a nonlinear-dynamics question, not a survival question — and it is exactly the FTD-0110 "nonlinear bridge" `[OPEN]` in sharpened form.
- **Convergence with the concurrent β v2 model (FTD-0263).** Independently and in the same session, the concurrent track built a **β v2 envelope model that ADDS the center back-reaction** (center-voxel kinetic + flux drains + Red-Black SOR Gauss projection suppressing neighbour flux) and found `BETA_v2_CONFIRMED`: the back-reaction shifts the 1% onset amplitude from the naive A≈5.6 up to the observed A≈8.5–9.0. That is the *same mechanism this telemetry measures directly* — genesis throttling by nonlinear flux suppression — reached from the modelling side. The two results are complementary: β v2 (FTD-0263) is the corrected forward model; this telemetry (FTD-0267) is the direct engine confirmation that genesis fires in a throttled one-shot burst. Both retire the β v1 post-genesis-survival reading (FTD-0265/0266).
- **S2's "pass" is an artifact, not nucleation.** survival_eff = steady_cluster / peak_manif. At A=9 the 3 manifested voxels are spatially *disconnected* (cluster=1, manifested 2), so the ratio is low (0.33) — by connectivity, not by evaporation (cum_evap=1). The rising ratio with A reflects the cluster becoming connected as more fire, not the nucleation-survival mechanism β posited.
- **Eighth/ninth constraints retired.** FTD-0265's "7th constraint" (3–6× transient-crossing suppression) and FTD-0266's "8th constraint" (pre-genesis kinetics ≲10%) were framed around the survival picture; both are superseded — the suppression is a genesis-count effect, measured directly here.

## 4 · Scope, caveats, and the golden-hash finding

- **Single backend, current stack.** Genesis/evap event counters populate on the **CPU path only** (the GPU genesis kernel does not touch the host counters). The measurement is on the **post-`c2a8f606`** stack — the concurrent session's backend-parity refactor that split phase_write into a post-write genesis loop and switched Langevin noise to a per-voxel deterministic stream. The cluster magnitudes differ from the FTD-0263 staircase (continued stack drift: A=14 gives 14 here vs 16.4; A=30 gives ~33 vs 45), but the **qualitative finding (one-shot genesis burst, genesis-limited, high survival) is robust across 4 seeds** and is a structural property of the genesis+evaporation toggle physics, not of the RNG-stream change.
- **The counters are observation-only — proven, not asserted.** The golden hash is **bit-identical** with the counters present and stashed (`0xebaa6f314f66db3f` both ways). This is the strongest form of the golden gate.
- **⚠ PRE-EXISTING GOLDEN REGRESSION (owner action).** The frozen `GOLDEN_HASH = 0xc13713f0e11a96da` (`test_render_bridge_golden.cpp`) is **stale**: commit `c2a8f606` (concurrent session, "Establish backend parity by resolving pre-write vs post-write divergence in CPU genesis"; also switched Langevin noise from `rng.thread_normal()` to per-voxel `voxel_normal()`) **intentionally changed CPU genesis physics** and did not re-pin GOLDEN_HASH. The canonical stack now deterministically produces `0xebaa6f314f66db3f` (reproduced ×2, with and without this arc's counters). `test_render_bridge_golden` therefore FAILS on `main` independent of this work. **Recommendation:** re-pin `GOLDEN_HASH = 0xebaa6f314f66db3f` and document the c2a8f606 physics change in the test header — left to the owner / the c2a8f606 author to avoid a shared-file collision.
- Nothing here moves FTD-0013, MC-T4.3, FTD-0110's tags, or any spine claim.
