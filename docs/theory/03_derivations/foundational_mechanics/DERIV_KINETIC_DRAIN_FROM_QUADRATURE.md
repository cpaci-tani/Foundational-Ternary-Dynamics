# Analysis — Can the kinetic drain (and the ¼ coefficient) be derived? (FTD-0276)

**Tag:** `[CLOSED NEGATIVE]` (drain² origin) + `[MEASURED]` (k_eff(drain), γ-map)
**Date:** 2026-06-12
**LEDGER row:** FTD-0276 (Legs A + B + C); adjudicates the FTD-0269 §4 queued follow-ups.
**Pre-registration (Leg A):** [`PREREG_DRAIN_SCALING_v1.md`](../../10_eft_program/preregistrations/PREREG_DRAIN_SCALING_v1.md), tag `preregister-drain-scaling-v1`, lock commit `1dc4db23`.
**Artifacts:** `engine/tests/campaign_drain_scan.cpp`, `scripts/exploration/analyze_drain_scan.py` (Leg A, SHA-locked); `scripts/exploration/genesis_na_law_gamma_sweep.py` + the post-lock γ knob in `genesis_na_law_forward.py` (Leg B). Runs of record: `engine/results/drain_scan/drain_scan_v1.csv`, `gamma_sweep_legB.log`.

---

## 0 · Verdict

The FTD-0269 §4 follow-ups asked whether the engine-tuning constants that calibrate
the N(A) law — the kinetic drain 0.5 and the Langevin friction γ — can be **derived**
from the framework rather than imposed. Three legs:

- **Leg A — drain² origin: `[CLOSED NEGATIVE]`.** The cluster efficiency
  `k_eff = N̄/A²` does **not** scale as `drain²` (measured exponent **−0.93**, not
  +2) and `k_eff(0.5) = 0.059 ≠ 0.25`. The `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`
  §12 hypothesis `k = drain² = 1/N_base` is falsified on the current stack.
- **Leg B — friction-knob map: `[MEASURED]`.** A quantitative `knee(γ)` / `exponent(γ)`
  map of the N(A) forward model. The engine's own friction `γ = 0.02` reproduces the
  FTD-0261 engine law (knee 16, p_hi 1.81 vs 1.86) in the forward model; γ is the
  super-knee calibrator. It calibrates, it does not derive.
- **Leg C — derivation attempt: `[CLOSED NEGATIVE]`.** No clean framework source
  (quadrature equipartition, `1 − 1/N_base`) yields the drain value 0.5 or the
  measured `k_eff`. The kinetic drain is an **engine-tuning constant**; `k_eff` is
  **engine-emergent** (`k_eff ∝ 1/drain`), reaffirming the FTD-0269 BOUNDARY.

**The linear k = ¼ theorem (FTD-0110, O_h representation theory) is untouched** — it
is mathematics, independent of the engine drain. What is closed is the *engine-side
drain² origin* of that number, and the §12 claim that `k` is engine-parameter-independent.

## 1 · Leg A — k_eff(drain) is ∝ 1/drain, not drain²

Run of record `campaign_drain_scan --L=32 --drains=0.125..0.75 --As=10..40 --seeds=5
--settle=300 --cpu`, sub-knee window A ≤ 16 (the FTD-0261 knee). Frozen analyzer:

| drain | 0.125 | 0.25 | 0.375 | 0.5 | 0.625 | 0.75 |
|---|---|---|---|---|---|---|
| sub-knee k_eff | 0.147 | 0.118 | 0.090 | **0.059** | 0.036 | 0.029 |

- **R1 (scaling):** log-log fit `k_eff(drain) ≈ 0.027 · drain^{−0.93}` (R² 0.88). The
  exponent **−0.93** is the opposite sign to the drain²-CONFIRMED band [1.8, 2.2] ⇒
  **CLOSED-NEGATIVE**. Physically `k_eff ∝ 1/drain`: removing twice the wave energy per
  genesis event roughly halves the cluster — the drain is a **linear calibration
  prefactor** on N(A), not a squared structural quantity.
- **R2 (value coincidence):** `k_eff(0.5) = 0.059`, vs `drain² = 0.25` — relative
  deviation 0.76 ⇒ **COINCIDENCE-FAILS**. (The measured 0.059 instead matches the
  FTD-0261 current-stack `k_eff ≈ 0.05`, a clean cross-validation: at the default drain
  the instrument reproduces the canonical N(A) law — `N̄(10) = 4.0`, exactly FTD-0261.)
- **Overall Leg A: CLOSED-NEGATIVE.** The drain² hypothesis fails both readings.

## 2 · Leg B — the friction-knob map (knee/exponent vs γ)

FTD-0269 found the Langevin friction γ "load-bearing (super-knee normalization)": the
friction-free forward model over-predicts the super-knee. Leg B makes this quantitative
(`genesis_na_law_gamma_sweep.py`, L=32, 5 seeds, the FTD-0261 amplitude grid):

| γ | knee | p_lo | p_hi | super-knee ratio (model/engine) |
|---|---|---|---|---|
| 0.000 | 14 | 4.57 | 2.09 | 1.531 (over-predicts) |
| 0.010 | 14 | 4.65 | 1.94 | 1.102 |
| **0.020** | **16** | **3.57** | **1.81** | **0.913** ← engine's actual γ |
| 0.050 | 20 | 3.04 | 1.43 | 0.663 |
| 0.100 | 25 | 2.30 | 1.42 | 0.449 |

- At **γ = 0** the forward model reproduces FTD-0269's framework-only numbers (knee 14,
  p_hi 2.09 ≈ their 2.07) — confirming the post-lock γ knob is byte-faithful at its 0.0
  default. The super-knee over-prediction is **1.53×** (FTD-0269 estimated "~1.8×").
- At the engine's actual **γ = 0.02** the model lands on **knee 16, p_lo 3.57, p_hi 1.81**,
  matching FTD-0261 (16, 3.69, 1.86) within a few percent; the super-knee ratio is
  closest to 1. The friction is a clean **monotone calibrator** of the knee and the
  super-knee exponent.
- **Honest scope:** γ is an **engine-tuning constant** (the FTD-0269 BOUNDARY). This map
  *calibrates* it — it does not derive it. The γ knob models only the dissipative part
  of the OU drift (`v ← v(1−γ)`), not the thermal kick.

## 3 · Leg C — no framework source for the drain or k_eff

Two candidate derivations of the drain value 0.5 (and hence of `k_eff`), tested honestly:

1. **Quadrature equipartition (½ from the symplectic (q,p) pair, FTD-0257).** The natural
   reading: the wave field carries energy half in the kinetic quadrature (`|wave_vel|²`)
   and half in the potential quadrature (`|J|²`), so genesis "should" drain the kinetic
   half ⇒ drain = ½. But `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §12 already showed the
   naive equipartition accounting does **not** reproduce the ¼ coefficient, and Leg A now
   shows the decisive obstruction: `k_eff` is **drain-dependent** (∝ 1/drain). A fixed
   ½-from-quadrature would give a *single* k_eff; the measured k_eff varies with drain
   across a decade. Equipartition cannot be the mechanism. **[CLOSED NEGATIVE].**
2. **`drain = 1 − 1/N_base` = 0.75.** One line: `1 − 1/4 = 0.75 ≠ 0.5`. The physical drain
   is 0.5, not 0.75. **[CLOSED NEGATIVE].**

**Correction to `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §12.** That doc (historical, the
pre-FTD-0260 stack where `k ≈ ¼`) asserts (line ~499) that "the ¼ is *not*
engine-parameter-dependent — engine parameters [like] K_GENESIS_KINETIC_DRAIN would NOT
change k, only renormalise the cluster lifetime." Leg A **falsifies this on the current
stack**: `k_eff` changes by ~5× across the drain range (0.147 → 0.029). The engine
cluster efficiency is decisively a function of the drain. (The §12 ¼ was a property of the
historical stack; FTD-0261 already re-measured the current stack as `k_eff ≈ 0.05`, and
this leg shows that value is drain-set, not structural.)

## 4 · Honest reading and what it feeds

The kinetic drain and γ are **engine-tuning constants**: the drain sets the cluster
efficiency linearly (`k_eff ∝ 1/drain`), γ calibrates the super-knee. Neither is
framework-derived; the N(A) law's calibration is engine-emergent — the FTD-0269 BOUNDARY,
now quantified on both knobs. The clean-derivation route for the nonlinear bridge stays
`[OPEN]`: it would need the drain 0.5 and γ 0.02 derived from the action, or a framework
argument that only the dimensionless *shape* (broken power, knee location *in units of*
the drain) is physical and the prefactor is convention. Leg A removes one candidate
(drain²) from that search; the search itself is unchanged.

**Feeds Arc 3 (FTD-0110 genesis-counting model):** the counting model must treat the
drain as an [IMPOSED] calibration input (`k_eff ∝ 1/drain` measured here), not as a
derived ¼ — and must not re-assume the §12 "parameter-independent ¼."

## 5 · No promotions

FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, FTD-0110 linear k=¼ theorem
(mathematics), FTD-0261/0269 — all unchanged. The drain² origin and the §12
parameter-independence claim are closed negative; nothing is promoted.
