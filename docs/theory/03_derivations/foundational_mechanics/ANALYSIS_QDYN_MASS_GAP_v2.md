# ANALYSIS — Native mass-gap swing, v2: CLOSED-NEGATIVE (no gap)

**Tag:** `[MEASURED — CLOSED-NEGATIVE]`. **LEDGER id:** FTD-0362.
**Pre-registration (hash-locked before the run):** `docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_QDYN_MASS_GAP_v2.md`, git tag `preregister-qdyn-mass-gap-v2`, lock commit `ec48707f`.
**Instrument:** `engine/tests/campaign_mass_gap_v2.cpp`. **Data of record:** `engine/results/mass_gap_v2/mass_gap_v2_v2.csv`.
**Supersedes:** FTD-0333 (v1, verdict INVALID — the run this v2 makes valid).

---

## §0 · Verdict

Does the full nonlinear genesis↔Gauss loop (`de_broglie_clock` OFF) generate a **native rest-mass gap** `ω₀ > 0` at `k=0` that the linear massless analysis cannot see? **No — CLOSED-NEGATIVE.** On all six admissible runs the native `k=0` mode sits **at the FFT resolution floor**, statistically indistinguishable from the massless control. The nonlinear loop does not gap the flux; it **freezes** it.

FTD-0270 `[MEASURED — BOUNDARY]` (the dispersion ceiling / no-native-quantum-dynamics boundary) is **hardened** — now measured on a valid integrator. **Zero promotions:** `x₊=1/α` `[SMC]`; MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; FTD-0270 unchanged in tag (a negative confirming it), FTD-0271 `[CONDITIONAL]` unchanged; no α derived.

## §1 · Why this run is valid where v1 was INVALID

v1 (FTD-0333) failed its stability gate **on all 10 runs** — the "instability" was bare-wave leapfrog amplitude growth (FTD-0337), a discretization artifact, not physics. v2's **E1 stable integrator** (the default-OFF `verlet_wave_integrator` toggle; golden hash `0xb604d81a3d79366e` verified byte-identical) fixes exactly this: the gated bare-wave energy drift `drift_bare` is **~5×10⁻⁵ on every run**, comfortably under the frozen G2a gate `< 1×10⁻⁴`. All four admissibility gates (G0 non-flooding, G2a stability, G1a control-null, G1b control-positive) pass on all six runs.

## §2 · Results (all six runs admissible)

`FLOOR = fft_floor = 3.067961×10⁻³` (lowest nonzero `ω_phys` bin).

| L | A | drift_bare (G2a<1e-4) | ω₀ᵏ⁰ native (quiescent) | ω₀ᵏ⁰ control | probe ctrl (G1b>5·FLOOR) | N_final | admissible |
|---|---|---|---|---|---|---|---|
| 48 | 9.0 | 5.39e-05 | **3.068e-03 = FLOOR** | 3.068e-03 | 0.279 | 7 | ✓ |
| 48 | 9.5 | 5.39e-05 | **3.068e-03 = FLOOR** | 3.068e-03 | 0.279 | 10 | ✓ |
| 48 | 13.0 | 5.39e-05 | **3.068e-03 = FLOOR** | 3.068e-03 | 0.279 | 17 | ✓ |
| 64 | 9.0 | 5.11e-05 | **3.068e-03 = FLOOR** | 3.068e-03 | 0.285 | 6 | ✓ |
| 64 | 9.5 | 5.11e-05 | **3.068e-03 = FLOOR** | 3.068e-03 | 0.285 | 12 | ✓ |
| 64 | 13.0 | 5.11e-05 | **3.068e-03 = FLOOR** | 3.068e-03 | 0.285 | 24 | ✓ |

**The native `k=0` frequency is pinned at the resolution floor on every run**, exactly equal to the massless control's `k=0` channel, while the control's *probe* channel rings normally at ~0.28 (the positive-channel check confirms the detector is sensitive). The native probe channel is likewise floor-pinned in the quiescent window: the manifested cluster's flux stops oscillating.

## §3 · Adjudication against the frozen §4 map

The frozen **CLOSED-NEGATIVE** condition — `ω₀_native` consistent with `ω₀_control` AND both `< 2·FLOOR`, on a majority of admissible runs — is met on **6/6** (unanimous), not merely a majority. The FORCED condition (`ω₀_native > 5·FLOOR` and `> ω₀_control + 3σ`) is met on **0/6**. No post-lock review (§6) is triggered — that path is FORCED-only.

## §4 · Reading (honest scope)

This is a boundary-hardening negative, presented as such: the substrate quantizes with the wrong (cavity, `ω ∝ |k|`) dispersion and the nonlinear back-reaction does **not** manufacture the quadratic-dispersion rest-mass term the Schrödinger sector needs — it damps the resting cluster's flux to a standstill. Consistent with the v1 hint and with FTD-0271 (native flux is massless; a rest-mass clock is an import). **What this does not claim:** it does not prove no gap is reachable under *any* toggle set or larger L — it measures, on the frozen canonical grid `L∈{48,64}×A∈{9,9.5,13}` with the valid E1 integrator, that this loop produces none. A future positive would need a construction outside this frozen design and would re-enter at §6.

## §5 · Provenance note

The instrument and pre-reg were frozen and git-tagged (`preregister-qdyn-mass-gap-v2`, `ec48707f`) **before** the canonical `--sweep`. No gate or band was altered after seeing data (the gates are numeric floor-multiples fixed in the pre-reg; the data lands unambiguously on one outcome). `OMP_NUM_THREADS=1`, `force_cpu()` reference path, seed `0xD0270002`, window 4096.
