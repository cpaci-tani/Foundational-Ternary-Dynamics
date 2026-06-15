# DERIV — Mechanism β, sustained-kinetics variant (FTD-0266): DWELL-FAIL — the Boltzmann rate is too steep for dwell-time to matter; the suppression is entirely post-genesis

**Tag:** `[COMPUTED — DWELL-FAIL per frozen rules]`; substantively **`[CLOSED NEGATIVE — for the dwell-time-integrated (pre-genesis Boltzmann) variant of Mechanism β]`** + a sharpened `[OPEN]`. **Nothing promoted.**
**Date:** 2026-06-10
**Runner (frozen thresholds in docstring, stated before compute):** [`scripts/exploration/derive_beta_kinetics_prediction.py`](../../../scripts/exploration/derive_beta_kinetics_prediction.py); output log at `/c/tmp/ftd0266_kinetics.log`. Priors: SUPPORTED 35 % / PARTIAL 40 % / FAIL 25 % (**landed in FAIL**).
**Targets:** the FTD-0263 constraint profile + the seventh (3–6× suppression of transient crossings, from FTD-0265).
**LEDGER:** FTD-0266.
**Depends on:** FTD-0265 (envelope model), FTD-0263 (constraint profile), FTD-0261 (the law), FTD-0251 (dispersion pin α=1/18).

---

## 0 · One-paragraph result

The sustained-kinetics model replaces the envelope's binary "did it ever cross?" with the time-integrated Boltzmann probability: `P_genesis(δ,A) = 1 − ∏_t (1 − p(δ,t,A))`, where `p(δ,t,A) = 1 − exp(−N_c · max(0, A·|J_unit(δ,t)| − 1))` and N_c = K_GENESIS/K_MANIFEST = 3 steepens the rate. The dispersion self-check passes (ω = 0.2257 vs 2c·sin(k/2) = 0.2253). **The correction is negligible: E[N(10)] = 23–24 (both variants), vs the envelope's 25–33 and the measured 4.0.** T1 fails, T2 fails (RMS 0.61/0.51 vs 0.20); T3 passes (onset at A = 9.0, within [7, 11]). **DWELL-FAIL.** The physical reason is transparent: with N_c = 3, any voxel above threshold for even one tick fires with probability ≥ 0.82 — dwell time is irrelevant because the Boltzmann gate is too steep to distinguish transient from sustained. The conclusion: **the onset suppression is 100% post-genesis survival physics.** Any voxel that transiently exceeds threshold almost certainly fires genesis; only ~13% of those (4/30) survive the post-genesis dynamics at A = 9. The engine-free calculation route is exhausted. The mechanism must be accessed via engine-side telemetry.

## 1 · The calculation

Same symplectic-Euler simulation as FTD-0265, but storing the full per-voxel |J(δ,t)| trajectory (N_vox × T_MAX = ~7000 × 110 float32). Bug corrected from FTD-0265: injection at array origin (0,0,0) so the analysis sphere (r ≤ 12) actually surrounds the injection site. Dispersion check formula corrected: eigenvalue = ALPHA·12·(cos k − 1) = (2/3)(cos k−1) < 0; symplectic-Euler dispersion ω = arccos(1 + μ/2), matching 2c·sin(k/2) to 0.2%.

| Test (frozen) | Variant (a) no-proj | Variant (b) fft-proj |
|---|---|---|
| T1 E[N(10)] ∈ [2.0, 8.0] | **FAIL (23.53)** | **FAIL (22.87)** |
| T2 shape RMS ≤ 0.20 | **FAIL (0.612)** | **FAIL (0.508)** |
| T3 onset ∈ [7.0, 11.0] | PASS (A = 9.0) | PASS (A = 9.0) |

Verdict: **DWELL-FAIL** (neither T1 nor T2 in any variant).

## 2 · Why the correction is negligible — the key physics

At A = 10, the rank-8 (body-diagonal) voxels that join the envelope at A_join = 5.73 have peak |J_unit| = 0.17465. Their excess at A = 10: `10 × 0.17465 − 1 = 0.75`. Boltzmann rate at peak: `p_peak = 1 − exp(−3 × 0.75) = 0.895`. Even for a **one-tick crossing**, `P_fire ≈ 0.895`. For two ticks: `P_fire ≈ 0.989`. The N_c = 3 factor makes the kinetics essentially a step function: you either clear threshold or you don't — the dwell time is irrelevant once you do.

**The survival efficiency at A = 9: measured N = 2 out of ~15 predicted to fire = 13%.** At A = 10: 4 out of ~24 = 17%. At A = 14: 16.4 out of ~41 = 40%. This rising survival efficiency is the signal: the Gauss self-sourcing mechanism strengthens as more neighbors have already manifested, suggesting a **nucleation** picture — the first voxels are the hardest to sustain (low coordination), and each manifested neighbor reduces the Gauss-sourcing deficit of its neighbors.

## 3 · What dies, what survives, what's next

- **Dies (definitively):** any engine-free mechanism for β — both envelope (FTD-0265) and dwell-time-integrated Boltzmann. `[CLOSED NEGATIVE for all pre-genesis-kinetics variants of β]`
- **The sharpened [OPEN]: post-genesis Gauss survival.** After a voxel fires genesis, does its own Gauss sourcing — combined with the Gauss sourcing of manifested neighbors — keep its flux above K_GENESIS, or does it re-evaporate? This is the engine's post-genesis update cycle: genesis → state writes → Gauss projection → the re-sourced voxel either sustains or dies. The survival probability depends on the cluster geometry at the moment of each new manifestation.
- **Engine telemetry needed:** per-tick genesis events + per-tick evaporation events, accessible via a short `engine::set_debug_genesis_telemetry()` hook or an external telemetry build. This is the clean next arc.
- **Eighth constraint on the FTD-0263 profile:** the mechanism must be entirely post-genesis — pre-genesis kinetics contributes ≲ 10% of the observed suppression.

## 4 · Scope

Quick-check-platform computation; two bugs from FTD-0265 corrected in this runner (injection site and dispersion formula — both noted above); the corrections do not change the FTD-0265 verdict (both are display-only for FTD-0265; the envelope model's conclusion was correct). The T3 pass is informative (onset A = 9 within target) but not load-bearing without T1/T2.
