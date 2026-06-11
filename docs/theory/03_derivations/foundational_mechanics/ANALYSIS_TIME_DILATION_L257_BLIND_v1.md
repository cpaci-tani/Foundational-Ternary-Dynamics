# ANALYSIS — Blind L=257 extension of the FTD-0252 time-dilation residual law: PREDICTION_CONFIRMED (7/9, at threshold)

**Tag:** `[MEASURED — blind extension]` (per the locked outcome scheme). **Nothing promoted: PL-4's measured-domain note extends to L=257; FTD-0013, FC-2, and every other tag are untouched.**
**Date:** 2026-06-11 (post-dates the lock; B-5 satisfied).
**Pre-registration:** [`PREREG_TIME_DILATION_L257_BLIND_v1.md`](PREREG_TIME_DILATION_L257_BLIND_v1.md) — lock commit `ee8976b6`, tag `preregister-time-dilation-L257-blind-v1`, locked **before** any L=257 data existed.
**Run of record:** `engine/results/time_dilation_L257_blind_2026-06-11/` (CSV + `score_report.md`; run log `engine/results/time_dilation_L257_blind_2026-06-11.log`). First completed run; nothing discarded (F-c).
**LEDGER:** FTD-0268.

---

## 0 · One-paragraph result

Nine per-group residuals `R_g(257) = |D_meas − √(1−v²)|` were predicted from the five measured lattice sizes (L = 33…193) by per-group log-log extrapolation, with 95% prediction intervals hash-locked before the run. The run landed **7 of 9 groups inside their locked intervals** (the frozen CONFIRM threshold is exactly ≥ 7) **and** the median residual fell from 0.002517 (L=193) to **0.001415** (L=257) — verdict **PREDICTION_CONFIRMED** by the frozen scorer, mechanically: `7 ≥ 7` and `0.001415 < 0.002517`. The two misses are the two *lowest-velocity* groups, where the residual lives at the 10⁻⁵ scale, and they missed in **opposite directions** (n_z=1 high and non-monotone, n_z=2 low) — an informative `[OBSERVATION]`, consistent with the signed residual crossing zero near these (L, v) rather than with any systematic stall (see §2). The pre-registered secondary check — the frozen FTD-0252 v2 analysis re-run over the combined six-point sweep — returns **IR_CONFIRMED with strengthened margins** (median ratio 0.109 vs 0.153; median R(L_max) 0.00259 vs 0.00458).

## 1 · Locked vs observed (the scorer's table, verbatim numbers)

| n_z | R_pred(257) | locked 95% PI | R_obs(257) | inside? | falls vs R(193)? |
|---|---|---|---|---|---|
| 1 | 0.000013 | [0.000006, 0.000032] | 0.000061 | **no (high)** | **no** |
| 2 | 0.000098 | [0.000038, 0.000247] | 0.000005 | **no (low)** | yes |
| 3 | 0.000400 | [0.000284, 0.000564] | 0.000309 | yes | yes |
| 4 | 0.000921 | [0.000634, 0.001338] | 0.000777 | yes | yes |
| 5 | 0.001805 | [0.000957, 0.003402] | 0.001415 | yes | yes |
| 6 | 0.003559 | [0.001018, 0.012445] | 0.002263 | yes | yes |
| 7 | 0.003934 | [0.002276, 0.006800] | 0.003355 | yes | yes |
| 8 | 0.006124 | [0.002410, 0.015560] | 0.004704 | yes | yes |
| 9 | 0.010083 | [0.001712, 0.059395] | 0.006294 | yes | yes |

Threshold arithmetic (frozen §5 rules): inside-PI count **7/9 ≥ 7** ✓; median R_obs(257) = **0.001415 < 0.002517** = median R(193) ✓ ⇒ **PREDICTION_CONFIRMED**. The verdict is the scorer's printed output (F-e); `score_report.md` in the run directory is the artifact.

## 2 · The two misses, honestly

Both misses sit at |R| ~ 10⁻⁵ — two orders below every other group — and deviate in opposite directions:

- **n_z=1** rose (0.000026 → 0.000061) — the only non-monotone group.
- **n_z=2** collapsed (0.000149 → 0.000005), 19× below its point prediction.

The observable is an *absolute value* of a signed residual. A signed residual passing through zero between L=193 and L=257 produces exactly this signature pair (one |R| bottoms out and rebounds, a neighbor lands near its zero). Reading: at the lowest velocities the lattice correction is no longer a clean one-sign power tail at these L — the per-group power-law *model* (not the IR convergence) breaks down at the 10⁻⁵ floor. `[OBSERVATION]`; a sign-resolved follow-up (score the signed residual, not |R|) is the natural v2 if anyone needs these two groups. Neither miss is in the stall direction at any velocity where the residual is resolved above the floor.

## 3 · Secondary check (pre-registered §8 S4, descriptive)

Frozen `analyze_time_dilation_v2.py` (SHA `9a755904…`, unmodified) over {v2 run of record + L=257}: all-direction median trend 0.02459 / 0.01847 / 0.01379 / 0.00984 / 0.00458 / **0.00259** at L = 33/65/97/129/193/**257**; verdict line **IR_CONFIRMED** with median ratio **0.109** (< 0.5) and median R(L_max) **0.00259** (< 0.005). Both margins strengthen on the six-point sweep. Combined-data dir: `engine/results/time_dilation_combined_L33-257_2026-06-11/`.

## 4 · Falsifier self-audit (§6/§7 of the prereg)

- **F-a** ✓ `voxel.tau` never read (runner unchanged; no analysis touches it).
- **F-b** ✓ runner SHA `28c99f87…b1140` and scorer SHA `d6d8799f…0816` verified on disk before the run (mechanical string comparison, recorded in session).
- **F-c** ✓ first completed run is the run of record; exit code 0; 12/12 sanity PASS.
- **F-d** ✓ all 9 groups present at L=257.
- **F-e** ✓ verdict = scorer output verbatim.
- **F-f** ✓ build provenance: binary `engine/build_wsl/campaign_time_dilation` (WSL2 canonical environment; RenderBridge reports the CUDA backend active — same environment class as the v2 run of record; CPU/GPU parity is the bit-exact 70/0 gate). Source-vs-binary freshness: source SHA matches the frozen v2 lock, binary rebuilt from it.
- **B-1..B-5** ✓ no re-fit, no sub-selection, no re-run, no promotions, analysis post-dates lock.

Per prereg §9, PREDICTION_CONFIRMED requires only this self-audit (the v2 methodology review covers the physics; the extrapolation + scoring are deterministic).

## 5 · Consequences and scope

1. **PL-4's measured domain extends to L = 257** (⟨100⟩, n⊥=3): the deviation law's decline continued at a never-measured size, predicted in advance with locked bands. The registry row EP-1 in [`SPEC_PREDICTIONS_FORWARD_2026.md`](../../01_reference/SPEC_PREDICTIONS_FORWARD_2026.md) records the verdict.
2. **Registered refinement of PL-4's shorthand:** the per-group fitted exponents drift from p ≈ 2.6 (low v) to ≈ 0.7 (v → 0.94) — "departure ∝ L⁻²" is a low-velocity statement; the velocity dependence of the convergence exponent is now a measured shape, available to any future PL-4 edit.
3. **New `[OBSERVATION]`:** the |R| floor behaviour at the lowest velocities (§2) — sign-resolved follow-up queued as an optional v2.
4. **Nothing promoted.** FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, FC-2 `[AXIOM]`-class, FTD-0252 `[MEASURED]` at its scope — all unchanged. No physical-units claim; calibration register applies.
