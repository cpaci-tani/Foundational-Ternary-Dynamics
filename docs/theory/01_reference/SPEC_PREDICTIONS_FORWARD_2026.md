# SPEC — Forward Predictions on Behalf of the Framework

**Tag:** `[SYNTHESIS]` — a registry. **No row is promoted by inclusion here**; every claim carries its source tag, and the LEDGER wins on any disagreement.
**LEDGER:** FTD-0268 (the EP-1 blind measurement) + this registry row.
**Companions:** [`SPEC_NOVEL_PREDICTIONS.md`](SPEC_NOVEL_PREDICTIONS.md) (value catalog, pre/post-diction flagged), [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](SPEC_PREDICTION_LEDGER_DEVIATIONS.md) (FTD-0258, structural deviations PL-1..PL-6), [`SPEC_FTD_FRAMEWORK_V1.md`](SPEC_FTD_FRAMEWORK_V1.md) (constitution; FC kill conditions §6.2), [`SPEC_ENERGY_SCALES_AND_DETECTABILITY.md`](SPEC_ENERGY_SCALES_AND_DETECTABILITY.md) (energy-scale lens; FTD-0306).
**Computed numbers:** every number below is computed by `scripts/exploration/forward_predictions_2026.py` (SHA256 `bb3b61d3ab98b78c4ca936625ec4e143a7868ecc19d836ff2c6c51d2e29ecd6d`, lock commit `ee8976b6`) — none is recalled or hand-copied from prose.

---

## §0 · Scope and standard caveat

This registry collects **genuinely forward-looking, falsifiable predictions** that follow from already-tagged claims. It adds no derivations and runs no searches. The FTD-0258 standard caveat applies to every row: engine results are substrate-level statements at the stated lattice size/stencil/protocol; physical readings are conditional on the calibration register (`a_phys ≡ ℓ_P`, `M_REST = m_e`, `t_phys = ℓ_P/(√3·c)`); dimensionless deviation structure is calibration-invariant, absolute scales are not. A tag is a label, not a resolution (F10).

## §1 · Digest — the honest shape of the framework

- **Theorem-grade spine** (`SPEC_ALGEBRAIC_SPINE.md` §0): the G* identity, the master quadratic, CM uniqueness (h=1 scope), coefficient 16 = |Aut(E)|², the Watson identity, Phase-G geometric Coulomb, plus the honestly-tiered rows. This is mathematics and stands independent of physics.
- **Central conjectures, unpromoted:** `x₊ = 1/α` is `[STRONGLY MOTIVATED CONJECTURE]` (FTD-0013); the cluster-mass identification is `[SMC]` with historical evidential basis (FTD-0110/0261/0262); MC-T4.3 remains a `[FOUNDATIONAL OBSTRUCTION]` — **FTD does not derive α**, and 11+ routes are closed negative.
- **The constitution** (FTD-0254): FC-0 (ℤ[i] reading), FC-1 (declines the measurement-map import M), FC-2 (native arrow; Lorentzian metric emergent-IR, sector-scoped) are `[AXIOM]`-class declarations with registered kill conditions.
- **The Higgs claim, stated at its actual grade.** `DERIV_HIGGS_FROM_MANIFESTATION.md` v3.0 gives `m_H = v·√(2λ)` with `v = M_P·√(2π)·α⁸` `[SELECTION]` and `λ = 3/23` (from `sin²θ_W = 3/13`, **`[PARAMETRIC]` FTD-0018**), loop-corrected by `(1−α)` (applied as a principle, **not derived from the substrate**). Computed against the **canonical** external reference (PDG 2024 combined, `scripts/constants.py` `Experimental.m_Higgs = 125.20 ± 0.11`):
  - tree level 125.69 GeV → **+4.44σ** (excluded at tree level);
  - loop-corrected 125.23 GeV → **+0.27σ** (within 1σ).
  The popular "1σ" statement is therefore true **only through the applied loop factor**, and the chain is a `[SELECTION]`-grade construction with a `[PARAMETRIC]` input — not a derivation. Owner flags: the source doc (i) uses a non-canonical PDG value (125.25 ± 0.17) versus the project's REF_EXTERNAL_CONSTANTS standard, and (ii) tags HIGGS-5 inconsistently (`[SELECTION]` in §9 vs `[STRUCTURALLY MOTIVATED PARAMETRIC]` in §8.3). Both flagged here without editing the in-flight doc.

## §2 · Lab-facing forward predictions (FP rows)

Each row: what FTD predicts, what the SM/QM expectation is, the experiment that decides it, and the kill condition. "Forward" means the deciding measurement does not yet exist.

### FP-1 — A first-order electroweak phase transition (the sharpest discriminator)

- **Source:** substrate genesis/evaporation threshold asymmetry (`K_GENESIS = 3K_B` vs `K_B`) produces a hysteresis loop in the manifestation transition (`DERIV_HIGGS_FROM_MANIFESTATION.md` §10.3 HIGGS-OPEN-3; engine campaign `campaign_ew_phase_transition.cpp`, in-flight). Substrate claim `[EMERGENT/MEASURED]`-grade once the campaign lands; **identification with the cosmological EW transition `[CONJECTURE]`**.
- **Prediction:** the EW transition is **first-order** ⇒ a relic stochastic gravitational-wave background exists, with standard transition-GW phenomenology placing a first-order EWPT peak broadly in the **10⁻⁴–10⁻² Hz band** (the spectrum parameters (α_GW, β/H) are *not* supplied by FTD — this is an existence + band prediction, `[PARAMETRIC]` on the spectrum side). A first-order transition also re-opens electroweak baryogenesis `[CONJECTURE]`.
- **SM expectation:** lattice-SM result is a **crossover** at m_H ≈ 125 GeV ⇒ **no** EWPT relic background.
- **Decided by:** LISA-class mHz GW observatories.
- **Kill condition:** LISA-class sensitivity reached with no EWPT-compatible background ⇒ the cosmological identification of the substrate hysteresis is falsified (the substrate measurement itself survives at its scope).

### FP-2 — Higgs self-coupling locked at λ = 3/23

- **Source:** the v3.0 Higgs chain (tags as in §1). λ is **fixed** by the framework — no freedom left.
- **Prediction (computed):** `λ_FTD = 3/23 = 0.130435`; against `λ_SM = m_H²/(2v²) = 0.129281` (PDG 2024 inputs): **κ_λ = 1.0089**, trilinear `3m_H²/v ≈ 191.1 GeV`. I.e. FTD predicts the Higgs self-coupling **0.9% above** the SM inference — indistinguishable at HL-LHC (~±50%), at the edge of FCC reach (~±5%).
- **Kill condition:** any measured `κ_λ` deviating from ≈1.01 by more than the joint uncertainty (in practice: |κ_λ − 1| > 0.1 established at FCC precision) kills the λ = 3/23 reading outright.

### FP-3 — Lorentz violation: nulls at dimensions 5 and 6, pattern-locked anisotropy beyond

- **Source:** PL-4/PL-5 (FTD-0251/0252 `[MEASURED]`): the wave-sector dispersion's k² correction is **isotropic**; rotation-breaking first enters with the **k⁴ phase-speed anisotropy** (dimension-7-class, strongly irrelevant), prefactor 6.95×10⁻⁴, exponent p = 4.0008 ± 0.0006.
- **Prediction:** **continued null results** in all dimension-5 and dimension-6 photon-sector Lorentz-violation searches (GRB time-of-flight dispersion, vacuum birefringence) — under `a_phys ≡ ℓ_P` the first FTD-native anisotropy is (k·ℓ_P)⁴-suppressed, ~10⁻⁴ × (E/E_P)⁴, unobservably far below any foreseeable bound.
- **Kill condition (two-sided):** (a) a **confirmed detection** of linear- or quadratic-Planck-suppressed LV contradicts the FTD wave sector (which forbids anything stronger than quartic); (b) engine-side, the k⁴ decay reversing or plateauing at larger L kills PL-5 directly (see EP-3).

### FP-4 — Structural nulls, sharpened (cross-ref PL-6: three `[THEOREM]` + the proton `[SELECTION]/[BOUNDARY]` — not duplicated here)

The clean `[THEOREM]` nulls: **no magnetic monopole** is ever detected (∇·B = 0 is an identity); **no SUSY partner** at any future collider (no fermionic grading exists to mirror); **no fourth fermion generation** (Moore-layer decomposition yields exactly 3); **no extra spatial dimensions** (the arithmetic |Aut(E)|² = 2^D·(D−1)! has D = 3 as its unique solution [THEOREM]; D=3 as the physical dimension is [SELECTION — declared], FTD-0355 — not forced). Each is killed immediately by a single contrary observation.

**Proton stability is NOT a clean theorem null** (LEDGER FTD-0301, [`ANALYSIS_PROTON_STABILITY_v1.md`](../10_eft_program/ANALYSIS_PROTON_STABILITY_v1.md)): the substrate's exact charge is the U(1) Σs only — there is **no baryon/B−L current**, so Σs conservation does *not* entail `τ_proton = ∞` (it does not forbid the charge-balanced `p → e⁺ + π⁰`), and FTD's own weak channel transmutes the mixed-sign uud proton. A large proton lifetime is a `[SELECTION]` (emergent metastability), not a forced prediction; an observed proton decay would be consistent with FTD's UNFORCED-METASTABLE boundary, **not** a falsifier. Hyper-Kamiokande's continued decay-null is consistent with FTD but — unlike the three structural nulls — not specifically evidence *for* it.

## §3 · Engine-native blind predictions (EP rows — the lockable spine)

### EP-1 — Blind L=257 extension of the FTD-0252 time-dilation residual law

Pre-registered in [`PREREG_TIME_DILATION_L257_BLIND_v1.md`](../03_derivations/foundational_mechanics/PREREG_TIME_DILATION_L257_BLIND_v1.md) (lock commit `ee8976b6`, tag `preregister-time-dilation-L257-blind-v1`): per-group 95% prediction intervals for the nine ⟨100⟩/n⊥=3 residuals at the never-measured L=257, locked before the run. Registered observation: the fitted per-group exponents drift from p ≈ 2.6 (low v) to ≈ 0.7 (highest n_z) — the "L⁻²" shorthand of PL-4 is accurate only at low velocity.

**Verdict (frozen scorer): PREDICTION_CONFIRMED — 7/9 groups inside their locked 95% intervals (the frozen threshold exactly), median residual 0.002517 → 0.001415.** The two misses are the two lowest-velocity groups (|R| ~ 10⁻⁵), in opposite directions — an `[OBSERVATION]` consistent with a signed-residual zero crossing, not a stall. The frozen v2 secondary analysis over the combined six-point sweep strengthens IR_CONFIRMED (median ratio 0.153 → 0.109). Full result: [`ANALYSIS_TIME_DILATION_L257_BLIND_v1.md`](../03_derivations/foundational_mechanics/ANALYSIS_TIME_DILATION_L257_BLIND_v1.md) (FTD-0268). **No tag promoted.**

### EP-2 — N(A) blind interpolation at A ∈ {35, 60} (registered now, run later)

- **Source:** the FTD-0261 current-stack law `[MEASURED]` (broken power law, knee A ≈ 16; `ANALYSIS_NA_LAW_CURRENT_STACK_v1.md`). A = 35 and A = 60 were never measured.
- **Locked predictions (registry table-level fit, knee frozen at 16; bands ±2·RMS = ±0.14 dex):**
  - **N(35) = 72.6**, band [52.5, 100.4]
  - **N(60) = 180.9**, band [130.9, 250.1]
- **Protocol when run:** the FTD-0261 rig unchanged (`campaign_thermostat_off_sweep` arm-N invocation: L=32, γ=0.02, T=0.005, thermostat=on, coupling=on, 8 seeds, 50 samples/seed); verdict = inside/outside band per point; no re-fitting.
- **Verdict (run after the bands were committed in `3f909301`): BOTH INSIDE — 2/2 CONFIRMED.** N̄(35) = **66.1** (8 seeds, range 59–70; −8.9% from the point prediction) and N̄(60) = **192.3** (range 187–199; +6.3%). k_eff = 0.054/0.053 at the two points, consistent with FTD-0261's asymptotic ≈ 0.05. Run of record `engine/results/ep2_na_blind_2026-06-11/`. The FTD-0261 broken-power law now has two blind interpolation confirmations in addition to its fit.

### EP-3 — k⁴ anisotropy extrapolation to L ∈ {512, 768}

- **Source:** PL-5 closed-form law (`AUDIT_LORENTZ_ANISOTROPY.md`). The power-law fit over L ∈ [32, 256] (p = 4.0003, prefactor 6.95×10⁻⁴) was extrapolated to the never-tabulated L = 512 and 768, then checked against the exact 18-point Moore symbol at 50-digit precision (double precision underflows beyond L ≈ 384 — a previously unrecorded boundary, now noted).
- **Locked rule:** extrapolation holds iff pred/exact ∈ [0.95, 1.05] at both L. **Result: pred/exact = 0.9997 (L=512) and 0.9995 (L=768) — HOLDS.** PL-5's tabulated domain extends a factor 2 deeper into the IR, δ down to 3.11×10⁻¹².

## §4 · Anti-overclaim guard — what FTD must NOT be claimed to predict

- **Not α.** MC-T4.3 is a `[FOUNDATIONAL OBSTRUCTION]`; 11+ derivation routes are `[CLOSED NEGATIVE]`; the master-quadratic operator assembly is route-invariantly unforced (FTD-0242). `x₊ = 1/α` is an identification, not a prediction.
- **Not the Born rule.** PL-1's registered prediction is **Rice statistics, not Born** — FTD *deviates* from QM here by design; claiming Born would invert the framework's own ledger.
- **Not lab Bell S > 2.** The substrate bound is S ≤ 2 `[THEOREM]`; the observer-layer account of measured S > 2 is `[SELECTION]+[OPEN]` and is the framework's highest-risk entry (PL-2 honesty block).
- **Not absolute dimensional scales** without the calibration register; only the dimensionless deviation structure is calibration-invariant.
- **Not particle masses as derivations.** The mass formulas (m_e, m_p/m_e, m_H, …) are `[PARAMETRIC]`/`[SELECTION]`-grade constructions per the CATALOG; their forward content lives only in rows like FP-2 where a fixed internal ratio meets a not-yet-performed measurement.

## §5 · Maintenance

Rows are added or updated only with: a source doc at its canonical tag + a computed-numbers artifact + (for EP rows) a hash-locked pre-registration. When a kill condition fires, the LEDGER row is retagged first; this registry is annotated second. Conflict precedence: LEDGER > constitution > this registry.
