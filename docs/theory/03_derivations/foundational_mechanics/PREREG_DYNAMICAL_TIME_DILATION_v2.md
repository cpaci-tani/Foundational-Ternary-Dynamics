# PRE-REGISTRATION — Dynamical Time Dilation, v2 (the IR limit)

**Tag:** `[PRE-REGISTRATION]` (locked before the verdict). **Supersedes the T2 sub-question of v1.**
**Date:** 2026-06-07
**LEDGER:** FTD-0252 (continued).
**Builds on:** [`PREREG_DYNAMICAL_TIME_DILATION_v1.md`](PREREG_DYNAMICAL_TIME_DILATION_v1.md) + its result
[`ANALYSIS_DYNAMICAL_TIME_DILATION.md`](ANALYSIS_DYNAMICAL_TIME_DILATION.md) + the v1 RedTeamAuditor review.
**Runner (frozen, v2):** `engine/tests/campaign_time_dilation.cpp`
SHA256 `28c99f87f82b82bb25eea14be7e72ae4c422307e955840ac92c7dbd75b3b1140`
**Analysis (frozen, v2):** `scripts/exploration/analyze_time_dilation_v2.py`
SHA256 `9a7559046f8bac01f5644a4f908f080ff220d6d7492a85cd90e90a72c5d9046c`
**Git tag:** `preregister-dynamical-time-dilation-v2` — owner-deferred (SHA256 content-hashes are the lock).

---

## §1 · Why v2

The v1 RedTeamAuditor (PASS, 8.5/10) confirmed the central defect: v1's mass `n⊥ ∝ L` **pinned**
`k⊥ = 2π·n⊥/L` near-constant across L, so increasing L never softened the mode — the IR limit was
**not probed**, and v1 honestly left "γ emerges in the IR" `[OPEN]`. v2 fixes this and the two v1
analysis bugs (the `direction` int/str filter; the Windows-Unicode crash).

## §2 · The Question (LOCKED)

Holding `n⊥` **fixed** so `k⊥ → 0` as `L` grows: does the lattice's departure from exact γ,
`R(L) = |D_meas − √(1−v²)|`, **decrease toward 0** as the mode softens — i.e. does the moving clock
dilate as √(1−v²) in the IR limit? (Same observable, definitions D1–D8, and non-circularity
discipline as v1; `voxel.tau` is never read.)

## §3 · Method (LOCKED)

- Runner mode **`--nperp-fixed=3`** (n⊥ held at 3 for every L); `--Llist=33,65,97,129,193`; the three
  motion directions ⟨100⟩/⟨110⟩/⟨111⟩; `n_z` swept to `3·n⊥`, K_MAX-capped pre-turnover.
- The frozen v2 analysis groups by **matched `(direction, n⊥, n_z)`** (continuum velocity ~fixed,
  `k` shrinking with L) and reports `R(L)` and the ratio `R(L_max)/R(L_min)`.

## §4 · Benchmark + decision (LOCKED)

`R(L) = |D_meas − √(1−v²)|`. Frozen thresholds in the analysis: `RATIO_TOL = 0.5`, `ABS_TOL = 0.005`.

## §5 · Three pre-blessed outcomes (LOCKED)

- **IR_CONFIRMED** — median `R(L_max)/R(L_min) < RATIO_TOL` **and** median `R(L_max) < ABS_TOL`.
  → `[MEASURED — γ emerges in the IR]`; the clock-hypothesis `[AXIOM]` may be annotated
  `[AXIOM with measured IR-emergent dynamical support]` — nothing stronger. **Does NOT promote
  FTD-0013 or derive α.**
- **IR_OPEN** — convergence not demonstrated at these (L, k). → `[OBSERVATION]`; report the trend.
- (No third "diverges" branch is expected, but a *rising* `R(L)` would be reported as IR_OPEN with a
  flagged anomaly.)

## §6 · Falsifiers + banned moves (LOCKED, inherit v1 §7–§8)

- **F-a** no `voxel.tau` read. **F-b** parameter-free predictions only. **F-c** every point carries
  its measured `v`. **F-g (new)** an IR claim requires `R(L)` measurably decreasing across **≥3** L
  values, not two-point noise. **F-h (new)** the matched groups must hold `n⊥` fixed (verify the run
  used `--nperp-fixed`).
- Banned: no threshold tuning post-hoc; no `(L, v, dir)` sub-selection; no `tau`; no claim of
  *deriving* γ (FTD-0208 closed that); no promotion of FTD-0013/α.

## §7 · Adversarial review

The v1 methodology review (8.5/10) covers the runner physics and analysis logic. v2 is a focused
bug-fix + IR extension; a **lighter self-audit against §6 falsifiers** suffices **unless the outcome
is IR_CONFIRMED** (a load-bearing positive), in which case a fresh independent `RedTeamAuditor` pass
is required before the result doc is updated.
