# Analysis — FTD-0110 Nonlinear Bridge: N(A) Law is Engine-Emergent (BOUNDARY)

**Tag:** `[MEASURED — BOUNDARY]`
**Date:** 2026-06-11
**LEDGER row:** FTD-0269 (adjudicates FTD-0110 nonlinear bridge)
**Pre-registration:** `PREREG_FTD0110_NA_LAW_v1.md`, tag `preregister-ftd0110-na-law-v1`, lock commit `192203b5`
**Artifacts:** `scripts/exploration/genesis_na_law_forward.py`, `analyze_na_law.py`, `engine/tests/campaign_genesis_geometry.cpp`; run of record `scripts/exploration/results/na_law_2026-06-11/`, `engine/results/genesis_geometry_2026-06-11/`

---

## 0 · Verdict

The genesis-burst forward model that includes the two flux-injection channels the prior
model omitted — the coupling source `G_C·∇s` and the **FFT-exact Gauss-projection boost**
`flux[void] −= ∇φ` (the lattice Poisson Green's function) — reproduces the **geometric
shape** of the FTD-0261 N(A) law from framework-derived inputs, but the law's **absolute
calibration** is fixed by **non-framework engine-tuning constants**. The frozen
three-outcome adjudicator returns **BOUNDARY**: the N(A) broken-power law is
**engine-emergent, not substrate-determined**. The FTD-0110 nonlinear bridge stays
`[OPEN]` as a clean-derivation target; the boundary is now mapped.

This supersedes `DERIV_FTD0110_GENESIS_THROTTLE.md` (which omitted Gauss, matched 3
counts, and mislocated the knee at 23.5).

## 1 · What the framework-derived dynamics DO reproduce

The baseline model (framework inputs: `K_GENESIS=N_c·K_MANIFEST`, the 18-pt O_h Laplacian,
`c²=1/3`, `charge_coupling=1`; with √α coupling on, flagged) gives, vs FTD-0261:

| A | model N̄ | FTD-0261 | A | model N̄ | FTD-0261 |
|---|---|---|---|---|---|
| 10 | 3.9 | 4.0 | 30 | 81.0 | 45.0 |
| 12 | 9.0 | 8.4 | 40 | 156.8 | 91.8 |
| 14 | 17.9 | 16.4 | 50 | 244.4 | 130.2 |
| 16 | 21.1 | 21.6 | 70 | 436.8 | 260.2 |
| 20 | 27.3 | 27.4 | 90 | 668.1 | 383.3 |
| 25 | 41.5 | 32.6 | | | |

- **Broken-power shape, in band:** fit knee = **14** (band [14,18]); super-knee exponent
  **p_hi = 2.07** (band [1.6,2.1]). (Sub-knee exponent p_lo = 4.55 is steeper than
  FTD-0261's 3.69 — out of the [3.3,4.1] band.)
- **Sub-knee normalization near-exact:** A ≤ 20 ratios are 0.97–1.09.
- **Sub-knee firing GEOMETRY matches the engine:** shell-occupancy L1 distance at A=14
  is **0.18** (≤ 0.30). Model `[center .06, SC .34, FCC .22, BCC .31]` vs engine
  `[.06, .36, .13, .37]` — both center + SC + BCC dominant, FCC partial. (Note: FCC
  *does* fire on the current stack ~2/seed, contradicting the throttle doc's
  "FCC does not fire" claim.)
- **Gauss is the decisive sub-knee ingredient (F-1 diagnostic):** with `--gauss off`,
  N(14) collapses 17.9 → 7.0 and the knee pushes up — quantifying the 23.5 → 16 story
  the throttle doc hand-waved. The FFT-exact lattice Green's function, included
  explicitly, is what drives the sub-knee cascade.

## 2 · Why it is a BOUNDARY (the load-bearing engine constants)

| Engine-tuning constant | Test | Effect | Verdict |
|---|---|---|---|
| `K_GENESIS_KINETIC_DRAIN = 0.5` | drain ∈ {0.25, 0.5, 0.75} | fit knee = {25, 14, 30}; \|shift\| = **16**; N(10) swings 23 → 3.9 → 2.4 | decisively load-bearing |
| coupling `G_C = √α` | `--coupling off` vs on | curve log10-RMS = **0.118** (> 0.10), driven by super-knee | mildly load-bearing (super-knee) |
| Langevin friction `γ = 0.02` | omitted in framework-only model | super-knee over-prediction ~1.8× (curve-RMS 0.170); A=30 shell over-spread (L1 0.66, model 59% "outer" vs engine 26%) | load-bearing (super-knee normalization) |

The kinetic drain alone moves the knee by 16 grid-units — far past the |Δ|>2 boundary
threshold — and the √α arm independently trips the 0.10 RMS threshold. Either suffices
for BOUNDARY; both fire.

## 3 · Honest reading

The discrete substrate + O_h Laplacian + Gauss Green's function determine the law's
**geometric structure** — that N(A) is a broken power law, the knee sits near A≈14–16, the
super-knee exponent is ≈2, and the sub-knee firing pattern is center+SC+BCC. This is a
genuine advance: the central two-channel hypothesis (coupling + Gauss boost) is confirmed,
and the knee is no longer mislocated. But the law's **absolute calibration** — its
normalization and the precise knee — is set by engine-tuning constants the framework does
not derive (the 0.5 kinetic drain, the γ friction, and mildly √α). Under the project's
input taxonomy, that is a mapped boundary, not a derivation.

Consistent with FTD-0261's own measurement that γ-friction is "active" (monotone in N) and
the kinetic-drain mechanism (FTD-0263 β v2). The clean-derivation route remains `[OPEN]`:
it would require either (i) deriving the kinetic drain 0.5 and the friction γ from the
action rather than imposing them, or (ii) a framework argument that the calibration is
gauge/convention and only the dimensionless shape is physical.

## 4 · Follow-up (queued, not run)

A pre-registered **v2** with a friction knob (γ) in the forward model would convert the
qualitative super-knee boundary into a quantitative map (knee/exponent vs γ), and a
drain-derivation attempt (is 0.5 forced by `1 − 1/N_base` or similar?) would test exit (i).
Neither is attempted here; the BOUNDARY verdict stands on the frozen v1 criteria.

## 4a · Golden-gate disclosure

During the FTD-0269 campaign the 8-color-SOR Gauss optimization introduced
**non-deterministic energy-audit reductions** in the existing golden gate (4 distinct
`total_energy` hashes across nominally identical runs). The genesis *field* is
bit-reproducible — verified by the `campaign_genesis_geometry` instrument, which produced
identical firing-geometry results across all runs at the same seed. Consequently the
BOUNDARY verdict rests on the geometry + N-count outcomes, which are sound; the
golden-hash coverage does not extend to the energy audit on the current stack. A
deterministic-reduction fix and re-pin are queued as a background task. This golden-hash
issue is separate from the three race conditions fixed in the FTD-0273 determinism-gate
campaign.

## 5 · No promotions

FTD-0013, MC-T4.3, and the SM cluster-mass identification are untouched. The linear k=¼
theorem (O_h representation theory) is mathematics and is unaffected. Nothing is promoted.
