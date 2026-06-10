# PREREG — Sub-Knee Onset Mechanism: the 27-Block Geometric Hypothesis v1

**Status:** `[PRE-REGISTRATION — design locked before any run]`
**Date:** 2026-06-10
**LEDGER row:** FTD-0263 (reserved)
**Provenance:** the narrowed FTD-0261/0262 successor item. The current-stack law has one unexplained feature: the knee at A ≈ 16 with the steep `p ≈ 3.7` onset below it. FTD-0261 already excluded the thermal floor (T-dose flat) and the existing data excludes friction as the onset's *author* (the knee survives in the thermostat-OFF arm at the same N-band).
**Runner (frozen):** `engine/tests/campaign_thermostat_off_sweep.cpp` (v3: adds `--dir=axial|diag`) — SHA256 `2a58c8b67814ba445b6a8067b0d0dff8430170cf2ed6d1c622ba6505c41902a7`
**Analysis (frozen):** `scripts/exploration/analyze_subknee_block_hypothesis.py` — SHA256 `0259a0827aa927cfb332c904f5f703bdbfc19ae1f95d57d58d221c6721fdeeb4`
**Git tag:** `preregister-subknee-block-hypothesis-v1` (applied at the lock commit).

## 1 · Hypothesis (stated before the run)

**The knee is the 27-block boundary.** Below it, the cluster fills the injection voxel's own Moore block — orbit shells cumulating 1 → 7 → 19 → 27 — producing the steep effective slope; above it, the cluster grows into the bulk lattice under the wave-envelope threshold (the smooth `p ≈ 1.86` branch). *Motivating (post-hoc) observations:* knee-N ≈ 22 in the thermostat-ON arm and ≈ 22–25 in the thermostat-OFF arm — the same N-band under different dynamics — and the knee radius `R* = (3N/4π)^{1/3} ≈ 1.7` ≈ the block edge.

**Aesthetic-capture guard (explicit):** the 27-block is FTD's foundational object, making this hypothesis maximally attractive; the verdict therefore hangs **only** on the three frozen invariance kill-tests below, never on the resemblance. The fine-grid staircase table (orbit milestones 1/7/19/27) is **descriptive only** and may not enter the verdict (anti-apophenia).

## 2 · Design (frozen; canonical protocol: coupling ON, thermostat ON γ = 0.02 / T = 0.005, burn 200, 500-tick window, seeds 0xE0102000+s)

| Arm | Config | Grid | Seeds | Role |
|---|---|---|---|---|
| **F** | axial, L = 32 | A ∈ {8.5 … 16 step 0.5, 17, 18} (18 pts) | 5 | knee-N localization + descriptive staircase |
| **D** | **body-diagonal** (components A·K_GENESIS/√3, same magnitude), L = 32 | A ∈ {10, 12, 14, 16, 18, 20, 24} | 3 | direction-invariance kill-test |
| **L24 / L48** | axial | A ∈ {10, 12, 14, 16, 20, 30} | 3 | sub-knee L-invariance kill-test (+ first bulk-branch L-dependence data, descriptive) |

## 3 · Criteria (mechanical; thresholds frozen)

- **C1 — knee-N in the block band:** broken-power fit over arm F + the frozen FTD-0261 bulk reference points; **knee_N ∈ [19, 33]** (through-edges → full block + margin).
- **C2 — direction invariance:** `N_diag/N_axial ∈ [0.6, 1.67]` at A ∈ {14, 16} (both must hold). Block geometry is direction-blind at the block scale; an injection-envelope mechanism is not.
- **C3 — sub-knee L-invariance:** `N_L24/N_L32` and `N_L48/N_L32` ∈ [0.65, 1.54] at A ∈ {10, 12, 14}; ≥ 5 of 6 comparisons in band. Local-block physics cannot know L.
- **Outcomes:** **GEOM-CONFIRMED** (all three; prior 45 %) ⇒ the sub-knee onset = Moore-block filling `[MEASURED]`; **GEOM-PARTIAL** (exactly two; prior 30 %); **GEOM-DISFAVORED** (≤ 1; prior 20 %) ⇒ the block reading fails its own kill-tests and the onset mechanism re-opens (β/front-energetics); **UNDETERMINED** (data gaps; 5 %).
- Hygiene: F-1 no re-runs; F-2 thresholds frozen (changes ⇒ v2); F-3 the staircase table is non-verdict; F-4 no scanning outside the declared grids. **Under every outcome nothing is promoted; FTD-0110/0013/MC-T4.3 untouched.**
