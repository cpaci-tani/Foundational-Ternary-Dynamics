# PRE-REGISTRATION — Blind L=257 extension of the FTD-0252 time-dilation residual law

**Tag:** `[PRE-REGISTRATION]` (locked before the L=257 run).
**Date:** 2026-06-11
**LEDGER:** FTD-0268 (new row on verdict).
**Builds on:** [`PREREG_DYNAMICAL_TIME_DILATION_v2.md`](PREREG_DYNAMICAL_TIME_DILATION_v2.md) (FTD-0252, IR_CONFIRMED) and its run of record `engine/results/time_dilation_v2_2026-06-07/`.
**Runner (frozen, unchanged from v2):** `engine/tests/campaign_time_dilation.cpp`
SHA256 `28c99f87f82b82bb25eea14be7e72ae4c422307e955840ac92c7dbd75b3b1140`
**Prediction + scoring script (frozen, this prereg):** `scripts/exploration/predict_time_dilation_L257.py`
SHA256 `d6d8799ff0981c2f5b49bcf29e63806e6d6e0d2209547d832f2adf7ec36b0816`
**Git tag:** `preregister-time-dilation-L257-blind-v1` (lock commit precedes the run of record).

---

## §1 · Context

FTD-0252 v2 measured PL-4's central deviation law: holding `n⊥ = 3` fixed so `k⊥ → 0` as `L` grows, the departure from exact γ, `R(L) = |D_meas − √(1−v²)|`, falls toward 0 across `L ∈ {33, 65, 97, 129, 193}` (IR_CONFIRMED, `[MEASURED — γ emerges in the IR]`). This prereg is a **blind quantitative extension**: predict `R(257)` per matched ⟨100⟩ group from the five measured points BEFORE any L=257 data exists, then run once and score. A confirmation extends PL-4's measured domain by 33% in L; a stall fires PL-4's pre-stated kill-condition relevance ("the L⁻² law breaking on ⟨100⟩ at larger L").

**Prior disclosure (F9 hygiene):** the engine is deterministic and the per-group residual trend is smooth, so the prior strongly favors PREDICTION_CONFIRMED. The information content is in the *locked numeric bands* — a miss in either direction is the surprising outcome. Nothing here promotes FTD-0013, FC-2, or any α claim.

## §2 · The Question (LOCKED)

At the never-measured lattice size **L = 257** (⟨100⟩ motion, `n⊥ = 3` fixed, same observable and discipline as v2 — `voxel.tau` is never read), do the nine matched-group residuals `R_g(257)` land inside the per-group 95% prediction intervals extrapolated from the five measured L, and does the median residual continue to fall?

## §3 · Definitions (LOCKED)

- **D1** Observable, runner, and matched-group construction exactly as FTD-0252 v2 (`R = |dilation_meas − √(1−v_norm²)|`, computed from the runner CSV exactly as the frozen `analyze_time_dilation_v2.py` does).
- **D2** Scope: the nine groups `(direction=100, n⊥=3, n_z ∈ {1..9})`.
- **D3** Fit: per-group least squares of `log₁₀R` on `log₁₀L` over all measured L for that group (5 points for `n_z ≤ 6`; 4 points for `n_z ∈ {7,8,9}`, which the K_MAX cap excludes at L=33).
- **D4** 95% PI half-width: `max(t₀.₉₇₅(n−2) · RMSE · leverage, 0.05 dex)`; the 0.05 dex floor absorbs cross-build floating-point drift.
- **D5** `R_g(193)` = the group's residual at the previous largest L (run of record).

## §4 · Locked predictions (computed 2026-06-11 from the v2 run of record, before any L=257 run)

| n_z | p_fit | rmse(dex) | R(193) | R_pred(257) | 95% PI lo | 95% PI hi |
|---|---|---|---|---|---|---|
| 1 | 2.608 | 0.0870 | 0.000026 | 0.000013 | 0.000006 | 0.000032 |
| 2 | 2.328 | 0.0938 | 0.000149 | 0.000098 | 0.000038 | 0.000247 |
| 3 | 1.980 | 0.0346 | 0.000651 | 0.000400 | 0.000284 | 0.000564 |
| 4 | 1.810 | 0.0376 | 0.001431 | 0.000921 | 0.000634 | 0.001338 |
| 5 | 1.603 | 0.0639 | 0.002517 | 0.001805 | 0.000957 | 0.003402 |
| 6 | 1.225 | 0.1262 | 0.003962 | 0.003559 | 0.001018 | 0.012445 |
| 7 | 1.544 | 0.0361 | 0.005790 | 0.003934 | 0.002276 | 0.006800 |
| 8 | 1.249 | 0.0615 | 0.007971 | 0.006124 | 0.002410 | 0.015560 |
| 9 | 0.723 | 0.1170 | 0.010407 | 0.010083 | 0.001712 | 0.059395 |

median R_pred(257) = 0.001805; median R(193) = 0.002517. (The fitted exponents themselves are a registered observation: the "L⁻²" shorthand is accurate only at low n_z — p drifts from ≈2.6 down to ≈0.7 as v grows, an honest refinement of PL-4's stated form.)

## §5 · Three pre-blessed outcomes (LOCKED — applied mechanically by the frozen scorer)

- **PREDICTION_CONFIRMED** — ≥ 7 of 9 groups inside their 95% PI **and** median `R_g(257)` < median `R_g(193)`. → `[MEASURED — blind extension]`; PL-4's measured domain note may be extended to L=257. **No tag promotions anywhere.**
- **PREDICTION_BENT** — median falls but < 7/9 inside PI. → `[OBSERVATION]`; the power-law *shape* is wrong; report per-group misses; queue a follow-up; PL-4 row annotated, not retagged.
- **CONVERGENCE_STALLED** — median `R_g(257)` ≥ median `R_g(193)`. → `[OBSERVATION — escalation]`; this is the direction of PL-4's registered kill condition ("emergence stalling"); a confirming re-run + independent review are mandatory before any FC-2-level consequence is even drafted.

## §6 · Falsifiers (LOCKED)

- **F-a** `voxel.tau` is never read (inherited; the runner does not read it).
- **F-b** Runner and scorer must match the SHA256 hashes above at run time; any edit voids the lock.
- **F-c** The run of record is the **first completed** L=257 run; discarding a completed run for any reason other than a crash/incomplete CSV → INVALID.
- **F-d** All 9 groups must be present in the L=257 CSV; missing groups → INVALID (not a verdict).
- **F-e** Verdict is whatever `predict_time_dilation_L257.py --score` prints; no manual re-derivation.
- **F-f** Build provenance (binary path + source SHA check) recorded in the analysis doc.

## §7 · Banned moves (LOCKED)

- **B-1** No re-fitting, band adjustment, or threshold change after the run.
- **B-2** No group sub-selection or velocity-window selection in the verdict.
- **B-3** No re-running with altered parameters to obtain a preferred verdict.
- **B-4** No promotion of FTD-0013, FC-2, PL-4, or any α claim on a CONFIRMED outcome.
- **B-5** The analysis doc must post-date this prereg's lock commit.

## §8 · Method (LOCKED)

1. **S1** Verify runner + scorer SHA256 against §0.
2. **S2** Run (WSL2 canonical environment; the runner is CPU-deterministic):
   `engine/build_wsl/campaign_time_dilation --L=257 --nperp-fixed=3 --output-dir=engine/results/time_dilation_L257_blind_2026-06-11/`
3. **S3** Score: `python scripts/exploration/predict_time_dilation_L257.py --score --new-csv engine/results/time_dilation_L257_blind_2026-06-11/wave_clock_dilation.csv --out <analysis output>`
4. **S4** Secondary (descriptive, not the verdict): re-run the frozen v2 analysis over the combined {v2 run of record + L=257} CSVs and report its IR_CONFIRMED/IR_OPEN line for the six-point sweep.
5. **S5** Write `ANALYSIS_TIME_DILATION_L257_BLIND_v1.md`; add LEDGER row FTD-0268; update the prereg manifest.

## §9 · Adversarial review

The v2 methodology review (8.5/10) covers runner physics and the residual construction; this prereg adds only a deterministic extrapolation + mechanical scoring. A self-audit against §6/§7 suffices for PREDICTION_CONFIRMED or PREDICTION_BENT; **CONVERGENCE_STALLED requires a fresh independent RedTeamAuditor pass** before any doc beyond the analysis file is touched.
