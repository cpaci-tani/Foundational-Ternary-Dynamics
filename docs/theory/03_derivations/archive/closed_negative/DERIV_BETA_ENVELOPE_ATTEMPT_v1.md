# DERIV — Mechanism β, envelope variant (FTD-0265): BETA-PARTIAL — the initial-crossing approximation fails shape and scale; the sustain/survival physics is load-bearing

**Tag:** `[COMPUTED — BETA-PARTIAL per frozen rules]`; substantively **`[CLOSED NEGATIVE — for the envelope-only (initial-crossing, sharp-kinetics) variant of Mechanism β]`** + a sharpened `[OPEN]`. **Nothing promoted.**
**Date:** 2026-06-10
**Runner (frozen thresholds in docstring, stated before compute):** [`scripts/exploration/derive_beta_envelope_prediction.py`](../../../scripts/exploration/derive_beta_envelope_prediction.py); output log preserved in this doc's tables. Priors: PARTIAL 40 % (**landed**) / SUPPORTED 30 / FAIL 30.
**Targets:** the FTD-0263 constraint profile (elbow N ≈ 14.6; the F-arm staircase; onset facts).
**LEDGER:** FTD-0265 (renumbered from a drafted 0264 on collision with the concurrent blocked-effective-action row).
**Companion:** [`ANALYSIS_BETA_ENVELOPE_MODEL_v1.md`](ANALYSIS_BETA_ENVELOPE_MODEL_v1.md) — a parallel write-up of the same runner from the concurrent session (same verdict; carries the corrected symplectic-Euler dispersion estimator note). This DERIV doc is the verdict of record per the LEDGER row.

---

## 0 · One-paragraph result

The parameter-free envelope model — pre-genesis dynamics is exactly linear, so compute `e(δ) = max_t |J(δ,t)|` per unit injection on the 18-pt lattice (α = 1/18, dispersion self-check 0.2257 vs 2c·sin(k/2) = 0.2253 ✓) and predict `N(A) = #{δ : e(δ) > 1/A}` — **fails the shape test decisively in both projection variants** (T2 log₁₀-RMS 0.75 / 0.58 vs the frozen 0.20): it **over-predicts cluster size 3–6×** through the onset (e.g. 25–33 predicted vs 4.0 measured at A = 10; 41–45 vs 16.4 at A = 14) and puts the second voxel's onset at A ≈ 4.6–5.8 vs the measured ≈ 8.75 (T3, ~1.7× low). The formal verdict is **BETA-PARTIAL** because variant (a) clears the frozen T1 elbow band — but that pass is **evidentially hollow and is reported as such**: the predicted elbow sits at knee_A = 5.8 (measured 13.5) with a near-degenerate break (p_lo = 2.44 vs p_hi = 2.32), and the frozen T1 criterion checked only knee-N, not knee-A — a weakness of this pre-registration, recorded rather than repaired post-hoc. The informative content: **far more voxels transiently cross threshold than ever join the steady cluster** — the linear envelope is not the bottleneck. By elimination *within* β, the load-bearing physics is **sustained-excess kinetics + survival**: the Boltzmann rate `p = 1 − exp(−excess/K_MANIFEST)` integrated over the *dwell time* near peak (a transient graze ≠ a manifestation), and the post-pulse question of which manifested voxels are *sustained* by their own Gauss self-field rather than evaporating. The envelope-only β dies; the **sustained-kinetics β** is the sharpened `[OPEN]`.

## 1 · The computation (no parameters, no engine)

Symplectic-Euler wave update on the 18-pt (2:1) stencil with α = 1/18 (pinned by the FTD-0251 dispersion), unit x-flux delta at the origin, L = 64, 110 ticks, analysis radius r ≤ 12; envelope = per-voxel running max |J|; two variants for the one modeling unknown (the engine's discrete Gauss projector): (a) no per-tick projection, (b) FFT divergence removal (central-difference symbol). The ranked spectrum comes out in symmetry multiplets (1, 6, 8, 6, …) — orbit-degenerate join thresholds, qualitatively consistent with the measured *smoothness* only via Boltzmann smearing.

| Test (frozen) | Variant (a) | Variant (b) |
|---|---|---|
| T1 elbow N ∈ [9.7, 21.9] | PASS — but knee_A = 5.8 (meas. 13.5), break near-degenerate: **hollow** | FAIL (knee_N = 6.4) |
| T2 shape RMS ≤ 0.20 | **FAIL (0.749)** | **FAIL (0.583)** |
| T3 onset of 2nd voxel (~8.75) | 4.62 (~1.9× low) | 5.76 (~1.5× low) |

## 2 · What survives, what dies, what's next

- **Dies:** β-as-envelope — "the cluster = everyone who ever crossed threshold." Over-counts 3–6× with onset amplitudes systematically low. `[CLOSED NEGATIVE for this variant]`
- **Survives (and is now mandatory):** any viable β must price **dwell time** (the Boltzmann crossing probability integrated over the transient's residence above threshold — short grazes near peak rarely fire) and **survival** (post-pulse sustenance by the manifested voxel's own Gauss sourcing vs evaporation). Both effects shrink N and raise onset amplitudes — the observed directions of the envelope model's failures. Quantifying them requires either a kinetics-weighted linear calculation (envelope → dwell-time functional, still engine-free) or per-tick crossing telemetry from the engine; either is a well-posed next arc.
- The FTD-0263 six-point constraint profile stands unchanged; this attempt adds a seventh: **the mechanism must suppress transient crossings by ~3–6× relative to envelope counting.**

## 3 · Scope

Quick-check-platform computation (linear algebra; engine remains canonical for any follow-up); the T1 criterion weakness is recorded above per the no-post-hoc-repair rule; nothing here touches FTD-0110's tags, the law (FTD-0261), or any spine claim.
