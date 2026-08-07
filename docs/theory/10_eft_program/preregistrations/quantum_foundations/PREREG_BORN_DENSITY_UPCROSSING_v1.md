# PREREG · Born-density upcrossing discrimination (v1 → v1.1)

**Status:** `[PRE-REGISTRATION — v1 EXECUTED → OUTCOME D (EXECUTION INVALID);
v1.1 RE-LOCKED BEFORE RE-EXECUTION]`
**Date locked:** 2026-08-07 (v1); v1.1 re-locked same day *(git-tag lock rides
the owner's next commit; runner SHA-256s below are the binding locks)*

> **v1 EXECUTION RECORD (before any v1.1 run).** v1 ran once. Stage A's A1
> gate failed at `3.566e-2` against `1e-10` — the runner's Verlet-ellipse
> invariant was mathematically wrong (instrument defect, not physics) — which
> **voids the run** under §5's kill conditions: Outcome **D, EXECUTION
> INVALID**. The blind Stage B result is retained for the record and for
> design (not for verdict): `R̂ = 0.5124`, 99% CI `[0.5073, 0.5182]` — the
> amplitude prediction `0.5004` excluded by ~3.5 half-widths, a small
> significant shift *toward* Born, consistent with §6.2's velocity channel at
> `Ω·τ ≪ 1`. **v1.1 changes:** (i) A1 uses the machine-exact two-step
> invariant `z_k = Φ_k(n+1) − e^{−iΩ_k}Φ_k(n)`, `n_k = Ω_k|z_k|²/(2sin²Ω_k)`;
> (ii) Stage B runs TWO arms — slow (λ = 64, 32; `Ω·τ ≈ 0.45–0.91`) and fast
> (λ = 16, 8; `Ω·τ ≈ 1.80–3.56`) — with a new first-class outcome **E
> (REGIME-DEPENDENT)**: the Born-fraction `(R̂ − R_amp)/(R_born − R_amp)`
> rises from slow to fast arm with non-overlapping 99% CIs. All other
> parameters, gates, and outcomes unchanged. v1.1 runner SHA-256:
> `269e3a2da015a5515fa25c966bec573acf9f26408f8d7e0504bfb4ce8c60f690`.
**Parents:** FTD-0187 (T1c `[OPEN]`), FTD-0200 (pre-registered engine test
required), FTD-0356 (mechanism demotion — weak-field, sign-conditional,
offset-subtraction), FTD-0798 §2 (the positive-frequency Born density),
Temporal-interior programme front T3.
**Production impact:** none — verification instrument.

## 1. Question

The threshold-manifestation mechanism (`DERIV_BORN_PROPORTIONALITY.md`, as
corrected by FTD-0356) predicts an excess upcrossing rate `∝ ⟨J_coh²⟩` at
leading order — **amplitude² weighting**. FTD-0798 §2 established that the
free second-order flux carries a conserved positive **occupation** density
`n = |φ₊|²` that is Born-valued at equal mode occupation. These two
weightings **differ** for modes of different frequency (by exactly the
`1/√(2Ω)` mode normalisation), and energy weighting differs from both. No
registered result says which weighting threshold statistics actually follow.
This pre-registration locks the three-way discrimination.

## 2. Locked artifacts

| Artifact | SHA-256 |
|---|---|
| `scripts/experiments/verify_born_density_upcrossing.py` | `a3395d9becd642bf7c80665d9574e0a08edd7378c63cac9a8f37645cf133247c` |

Master seed **20260807**; 48 noise seeds; 2,000 bootstrap resamples; every
instrument parameter frozen in the runner (L = 4096 ring, `C_WAVE = 1/√3`,
`K = W_SC = 0.5054620197`, `σ_n = 0.17 < K`, OU correlation 8 ticks,
λ = 64 and 32, `A₁ = 0.10`, `A₂ = A₁√(Ω₁/Ω₂)` — equal occupation,
20,000 ticks).

## 3. Platform declaration (binding)

Mechanism-level **quick-check platform** (standalone 1D instrument
implementing the FTD-0187/0356 model as written: deterministic coherent
wave + per-site OU noise + threshold), per the project's measurement-
platform rule. **This run cannot close FTD-0200** — closure requires the
subsequent engine-side campaign. What this run can do is select among the
three candidate weightings at mechanism level, or invalidate.

## 4. Stages and locked gates

**Stage A (exact, deterministic).**
- A1 — total occupation `Σ n_k`, normalised with the **discrete-exact**
  leapfrog frequency `Ω_k = 2 arcsin(ω_k/2)`, conserved to `< 1e-10`
  relative over 20,000 ticks. (Sharpens FTD-0798's reported `1.05e-2`
  drift, attributed to continuum-frequency normalisation.)
- A2 — two modes at equal occupation split `0.5 : 0.5` within `2e-3`.
- A3 — scope documentation, no gate: additive source and threshold
  projection each break conservation (magnitudes reported). Free-sector
  only, per FTD-0798 §4.

**Stage B (statistical discrimination).** Site-resolved excess upcrossing
rate (signal minus same-seed control), regressed on
`{1, cos 2k₁x, cos 2k₂x}`; registered statistic `R = c₂/c₁` (mode-weight
ratio) with a 99% bootstrap CI over seeds. Rival predictions, fixed by the
frozen `Ω₂/Ω₁`:

| Weighting | Prediction |
|---|---|
| **amplitude²** (registered null — the DERIV leading order) | `R = Ω₁/Ω₂ ≈ 0.52` |
| **occupation / Born** (FTD-0798 density) | `R = 1` |
| **energy** (the FTD-0356 worry) | `R = Ω₂/Ω₁ ≈ 1.94` |

## 5. Pre-blessed outcomes

| Outcome | Condition | Consequence |
|---|---|---|
| **A — AMPLITUDE** | only `R_amp` inside the 99% CI | the mechanism's literal prediction confirmed; the Born-density reading FAILS at mechanism level; T1c's candidate weakens; the temporal-interior programme loses its probability pillar pending a new mechanism |
| **B — BORN** | only `R_born = 1` inside CI | occupation weighting selected — a NEW lead: threshold statistics natively perform the `1/√(2Ω)` normalisation; strongest T1c development since FTD-0187; engine campaign next; tag ceiling `[MEASURED — mechanism-level, imposed ensemble, quick-check platform]` |
| **C — ENERGY** | only `R_en` inside CI | `ω²`-weighting confirmed — the FTD-0356 concern realised; Born reading fails worse than the null |
| **D — INDETERMINATE / INVALID** | ≥ 2 predictors in CI, or zero; or kill conditions | no verdict; report power; redesign required |

**Kill conditions (any → D):** net excess counts `< 5,000`; either
regression coefficient `≤ 0` (anti-Born regime, `σ_n ≥ K` violated or
model outside validity); Stage A gate failure voids Stage B's
interpretation (instrument invalid).

## 6. Pre-run observations (recorded before execution, theory only)

1. The DERIV mechanism's leading-order excess is `∝ ⟨J_coh²⟩` — the
   registered null is therefore **amplitude**, not Born.
2. A velocity-channel contribution exists (the coherent `φ̇` raises the
   crossing-rate spectral moment `m₂`) and scales `∝ A²Ω²`; mixed with the
   offset channel `∝ A²`, intermediate effective weightings are possible —
   that is what outcome D's "≥ 2 inside CI" case will catch honestly.
3. Whatever wins, the result moves **no** substrate-level tag: the noise
   ensemble is `[IMPOSED]` (P5 supplies no ensemble; FC-1 declines M), and
   the finding is a property of the declared model class.

## 7. Execution

```
python scripts/experiments/verify_born_density_upcrossing.py
```

Deterministic under the locked seeds. Result to be reported with the
outcome letter verbatim; LEDGER row minted only by the owner.

---

## v1.1 EXECUTION RECORD (2026-08-07)

Stage A: **PASS** — A1 drift `7.143e-14` (gate `1e-10`); A2 split
`0.500000 : 0.500000`; A3 breakage documented (source `+68,020%`;
projection `−1.0%`). Stage B: both arms valid (net excess `2,182,968` and
`2,104,010` crossings; all coefficients positive).

> **OUTCOME E — REGIME-DEPENDENT: Born-fraction rises from 0.024
> [0.014, 0.036] (slow, `Ω·τ ≈ 0.45–0.91`) to 0.099 [0.090, 0.109]
> (fast, `Ω·τ ≈ 1.81–3.56`) — the weighting moves toward occupation as
> mode frequency exceeds the noise bandwidth.**

Neither pure amplitude nor Born nor energy weighting survived in either
arm; the registered E pattern (rising Born-fraction, non-overlapping CIs)
matched. Registered v2 question: saturation vs approach to 1 as
`Ω·τ → ∞` (larger `τ`, modes near the band top). This record moves no
tag; a LEDGER row is the owner's to mint.
