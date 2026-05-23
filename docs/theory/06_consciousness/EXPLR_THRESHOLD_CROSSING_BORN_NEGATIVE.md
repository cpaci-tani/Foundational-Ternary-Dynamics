# EXPLR — Threshold-Crossing Statistics ≠ Born Rule (in the 6-Neighbour Substrate)

**Document type:** Exploratory test result (closed-negative)
**Status:** `[CLOSED NEGATIVE]` — under the pre-registered construction, manifestation event frequency does NOT scale as `|J|²` (Born); it scales as Rice's upcrossing formula for a Gaussian-like process.
**Created:** 2026-05-23
**Pre-registration:** [`PREREG_THRESHOLD_CROSSING_BORN_v1.md`](PREREG_THRESHOLD_CROSSING_BORN_v1.md)
**Git tag (hash-lock):** `preregister-threshold-crossing-born-v1` (commit `e4a5813`, runner SHA256 `2781b3cec9d62db9f3635c8672e8da6e31575a37e5c4903c4a9d14d060cab465`)
**Runner:** `scripts/exploration/explore_threshold_crossing_born.py`
**Results:** `scripts/exploration/results/threshold_crossing_born_2026-05-23.{csv,md}`
**Sub-investigation of:** LEDGER FTD-0187 / target T1c (the `[OPEN]` step *probability = normalized energy density*).
**LEDGER row:** FTD-0198.

---

## 0 · One-paragraph result

The pre-registered test fired **Outcome C** per the manifest's outcome → tag map. In a 3D cubic lattice (L=24) with 6-neighbour face Laplacian wave dynamics, Gaussian-envelope sinusoidal initial flux + per-trial perturbations, ReLU manifestation at `K_B = 0.5`, and 100×80 = 8000 ticks of statistics across 7,973 in-mask voxels (5.3 M total events), the Rice upcrossing form `log freq = log B − k · (K_B − μ)²/σ²` fits with **R² = 0.9923**, while the power-law form `freq ∝ |J|^n` (where Born predicts n = 2) fits at only **R² = 0.7137**. Rice beats power-law by 0.28 in R² — far above the 0.05 pre-registered threshold. The corpus assertions in `SPEC_SIX_ALGORITHMS.md:65` and `AUDIT_EPISTEMIC_AUDIT.md:393` that "threshold crossing statistics produce the Born rule" do not hold in this regime. `[CLOSED NEGATIVE for Born in 6-neighbour substrate]` + `[NUMERICAL FACT — Gaussian-process upcrossing rate]`. No FTD tag is promoted; two corpus assertions are retagged (see §5).

---

## 1 · What was tested

LEDGER FTD-0187 (2026-05-21) tagged the load-bearing step *probability = normalized energy density* as `[OPEN]` (target T1c). A subsequent corpus sweep identified two documents NOT in the FTD-0187 consolidation that carry an even stronger claim:

- [`SPEC_SIX_ALGORITHMS.md:65`](../01_reference/SPEC_SIX_ALGORITHMS.md): "the threshold crossing statistics produce the Born rule"
- [`AUDIT_EPISTEMIC_AUDIT.md:393`](../07_assessment/AUDIT_EPISTEMIC_AUDIT.md): "threshold crossing statistics reproduce `|ψ|²`"

Both are asserted without a derivation. Standard upcrossing-rate analysis (Rice 1944) for a Gaussian process with mean `μ` and variance `σ²` crossing level `K` gives a rate `∝ exp(−(K − μ)² / 2σ²)`, which is *not* `|μ|²` scaling. The test was set up to discriminate which of `{Born, classical linear, Rice, none}` fires.

Construction (frozen in manifest §2):

- 3D cubic lattice `L = 24`, periodic BCs, 6-neighbour face Laplacian, mild damping `γ = 0.001`.
- Initial flux: smooth Gaussian envelope `× sin(2π n_i v_i / L)` with incommensurate wavenumbers `(2, 3, 1)` and amplitude `A = 2.0`, plus per-trial perturbation `~ Normal(0, 0.10)`.
- Manifestation rule: `s = sign(J_x + J_y + J_z) · 𝟙[|J| > K_B]` with `K_B = 0.5`; evaporation at `K_B_evap = 0.25`.
- Ensemble: 100 trials × 80 ticks each = 8,000 samples per voxel. Per-trial seed `42 + trial_idx`.

For each in-mask voxel, count `count(v)` = number of `s = 0 → ±1` transitions, time-and-trial-average `mu_sq(v) = ⟨|J(v)|²⟩`, time-and-trial-variance `sigma_sq(v)`, and frequency `freq(v) = count(v) / total_samples`. Bin into 14 equal-count percentile bins by `mu_sq`. Fit two competing models on bin-averaged data: `H_power` and `H_Rice`.

---

## 2 · What the run produced

Full per-voxel data in [`threshold_crossing_born_2026-05-23.csv`](../../../scripts/exploration/results/threshold_crossing_born_2026-05-23.csv) (13,824 rows). Bin-averaged summary:

| bin range (μ²) | mean μ² | mean σ²(\|J\|²) | mean freq | n sites |
|---|---|---|---|---|
| 0.070–0.080 | 0.0770 | 0.0040 | 0.0203 | 570 |
| 0.080–0.085 | 0.0824 | 0.0046 | 0.0268 | 569 |
| 0.085–0.089 | 0.0869 | 0.0051 | 0.0317 | 570 |
| 0.089–0.093 | 0.0910 | 0.0057 | 0.0369 | 569 |
| 0.093–0.096 | 0.0945 | 0.0062 | 0.0410 | 570 |
| 0.096–0.100 | 0.0980 | 0.0068 | 0.0446 | 569 |
| 0.100–0.105 | 0.1026 | 0.0077 | 0.0494 | 569 |
| 0.105–0.111 | 0.1082 | 0.0092 | 0.0524 | 570 |
| 0.111–0.118 | 0.1140 | 0.0106 | 0.0562 | 569 |
| 0.118–0.126 | 0.1214 | 0.0129 | 0.0598 | 570 |
| 0.126–0.136 | 0.1297 | 0.0157 | 0.0610 | 569 |
| 0.136–0.153 | 0.1432 | 0.0203 | 0.0663 | 570 |
| 0.153–0.177 | 0.1639 | 0.0313 | 0.0691 | 569 |
| 0.177–0.320 | 0.2177 | 0.0879 | 0.0657 | 570 |

Notice: the mean frequency *saturates* in the top bins (~0.066–0.069) instead of continuing the power-law growth. That saturation is the Rice envelope tail; a power-law fit averages over the saturating tail and the rising body, producing the misleading n ≈ 2.19 with poor R².

Fit results:

| Model | Form | Fit parameter | R² |
|---|---|---|---|
| `H_power` | `freq = A · \|J\|^n` | n = 2.1858 (95% CI [1.26, 3.99]) | **0.7137** |
| `H_Rice`  | `log freq = log B − k · (K_B − μ)² / σ²` | k = 0.0971, log B = −2.50 | **0.9923** |

`H_Rice` outperforms `H_power` by 0.2786 R². Per manifest §4.3 decision rule:

> `H_Rice R² > H_power R² + 0.05 AND H_Rice R² > 0.90` → **Outcome C: Rice / upcrossing scaling**; `[NUMERICAL FACT — Gaussian-process upcrossing rate]` + `[CLOSED NEGATIVE for Born]`; SPEC_SIX_ALGORITHMS and AUDIT_EPISTEMIC_AUDIT need retag.

---

## 3 · Why the substrate produces Rice statistics, not Born

`[STRUCTURAL OBSERVATION].` The 6-neighbour Laplacian wave equation is *linear* in `J`, so a Gaussian initial condition stays Gaussian under evolution. Each voxel's `|J(t)|` is a sum of independent oscillating Gaussian components, and over a sufficiently long trajectory it behaves as a stationary Gaussian process with site-dependent mean `μ(v) = √⟨|J(v)|²⟩` and variance `σ²(v)`. The site-dependent variance is itself a result of the linear superposition of modes from the smooth initial condition.

`[STRUCTURAL OBSERVATION].` Rice's 1944 formula gives the upcrossing rate of level `K` by a stationary Gaussian process with mean `μ` and variance `σ²`:

```
upcrossings per unit time ∝ exp(−(K − μ)² / 2σ²)
```

This is exactly what the fit returns (with `k ≈ 0.0971` setting the proportionality between `(K_B − μ)²/σ²` and the log frequency). The R² of 0.9923 is essentially the Rice formula obtaining identically.

`[STRUCTURAL OBSERVATION].` Born scaling `freq ∝ |J|²` would require either (i) the substrate process to be non-Gaussian in a specific way that converts `|J|²` directly into a probability without a threshold-crossing intermediary, or (ii) the manifestation rule itself to be probabilistic in a `|J|²`-weighted way (e.g. a Softplus regularization in the limit where the probability per tick is `∝ |J|²`). Neither is true in the construction tested. So the substrate's deterministic threshold rule, applied to a linear wave-equation field, generically produces Rice statistics, not Born.

`[CONJECTURE — testable in v2].` It is possible — but presently unverified — that the FTD-canonical 26-neighbour Moore stencil, with all dynamical toggles enabled (Gauss projection, source coupling, non-linear back-reaction from manifestation events), drives the substrate sufficiently far from Gaussian that Born scaling reappears. A v2 engine experiment is the natural follow-up. The present result does not falsify Born in the canonical engine; it falsifies *the assertion that "threshold crossing" alone, in the simplest substrate, suffices* to produce Born.

---

## 4 · What is **not** falsified by this run

- **The Born rule itself.** Standard QM `P = |ψ|²` is unaffected.
- **The `|ψ|²` *form* question** (EF-C3) — this is an algebraic-uniqueness question, not what this test addressed.
- **The Parseval-energy-density theorem** — `|J|²` IS the conserved energy density of the discrete wave equation. That is `[THEOREM]` and is not what this test asked.
- **The Existence Filter Pythagorean identity** (EF-T5) — `P = E(x)² + E(ix)² = |x|²` is an algebraic identity, unaffected.
- **`x₊ = 1/α`** (FTD-0013) — unchanged.
- **The collapse mechanism in `DERIV_COLLAPSE_MECHANISM.md`** — the Softplus/Lindblad/Type III₁→I framework is a different proposal; its Born derivation goes through the Lindblad formalism (which already carries Born structure), not through threshold-crossing of a deterministic field. The present result does not engage that framework.

---

## 5 · Retag recommendations (applied in the same commit as this document)

Two corpus assertions are surgically retagged per manifest §4.3:

### 5.1 `docs/theory/01_reference/SPEC_SIX_ALGORITHMS.md:65`

**Before:** "**This IS wave function collapse.** The flux field (= wave function) is spread out. When it concentrates past K_B somewhere, that point manifests. The probability follows from `|ψ|²` because `ρ = |J|` and the threshold crossing statistics produce the Born rule."

**After:** the assertion is retagged `[CONJECTURE — falsified in 6-neighbour substrate per FTD-0198, status under canonical 26-neighbour engine OPEN]` and the inline "produce the Born rule" claim is qualified with a footnote pointing at this document.

### 5.2 `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md:393`

**Before:** "**Relation to manifestation:** The manifestation threshold (|J| > K_B) acts as a ReLU on the ontic field: max(Re(ψ²), 0). This is the bridge between the ontic quadratic and the epistemic Born rule — threshold crossing statistics reproduce `|ψ|²`."

**After:** the "threshold crossing statistics reproduce `|ψ|²`" clause is retagged `[CONJECTURE — falsified in 6-neighbour substrate per FTD-0198]` with a footnote.

In both cases the surrounding prose is preserved; only the load-bearing sentence is qualified.

---

## 6 · What this rules out

`[CLOSED NEGATIVE].` The claim *"threshold crossing statistics alone, in the FTD substrate as deterministically formulated, produce the Born rule"* is false in the simplest substrate setting we can test. In a 6-neighbour Laplacian wave-equation substrate with a ReLU threshold rule, the manifestation frequency scales as the Rice upcrossing rate `∝ exp(−(K_B − μ)²/2σ²)`, not as `|J|²`.

`[METHODOLOGICAL].` Future variants — engine-canonical 26-neighbour Moore stencil, Softplus-regularized manifestation rule, fully driven non-equilibrium dynamics with back-reaction — need fresh pre-registration before any of their results can count.

`[STRUCTURAL].` The Rice null implies that *if* Born is going to emerge in the canonical engine, it must come from non-Gaussian structure of the substrate process — i.e. from features that the 6-neighbour linear wave equation lacks. Candidates include: the multiplicative BCC eigenvalue structure (which Phase G already shows produces non-trivial geometric coupling), the toggle-mediated non-linear feedback from manifestation events, or the discrete Z[i] arithmetic at lattice sites. None of these is present in the v1 construction.

---

## 7 · Items still out of scope (per PREREG §4.4)

- The Born rule form (EF-C3): why quadratic vs `|ψ|` vs `|ψ|⁴`. Algebraic-uniqueness question, not addressable here.
- Engine-canonical T1c test (26-neighbour Moore, full toggle stack).
- The Lindblad/Softplus collapse framework in `DERIV_COLLAPSE_MECHANISM.md`.
- Bell, interference, entanglement, decoherence dynamics.
- The `x₊ = 1/α` central conjecture.

---

## 8 · Cross-references

- [`PREREG_THRESHOLD_CROSSING_BORN_v1.md`](PREREG_THRESHOLD_CROSSING_BORN_v1.md) — the manifest that locked this test.
- [`LEDGER.md`](../07_assessment/LEDGER.md) — FTD-0187 (Born consolidation), FTD-0198 (this test).
- [`SPEC_SIX_ALGORITHMS.md`](../01_reference/SPEC_SIX_ALGORITHMS.md) §1A — site of retag.
- [`AUDIT_EPISTEMIC_AUDIT.md`](../07_assessment/AUDIT_EPISTEMIC_AUDIT.md) — site of retag.
- [`FOUND_THE_EXISTENCE_FILTER.md`](FOUND_THE_EXISTENCE_FILTER.md) — EF-T5 [THEOREM] + EF-C3 [CONJECTURE] (the `|ψ|²` form question).
- [`DERIV_COLLAPSE_MECHANISM.md`](DERIV_COLLAPSE_MECHANISM.md) — canonical collapse proposal (out-of-scope here).
- [`EXPLR_TERNARY_MATRIX_BCC_SNAP_NEGATIVE.md`](../09_mathematical/EXPLR_TERNARY_MATRIX_BCC_SNAP_NEGATIVE.md) — sibling closed-negative document; same epistemic pattern (pre-registered test, Outcome != A, honest retag in canonical sources).

---

## 9 · Bookkeeping

- **Pre-registration discipline:** fully respected. Construction, prediction, outcome map, seeds frozen before runner executed.
- **No FTD tag promoted.** Two corpus assertions retagged from un-tagged-but-asserted to `[CONJECTURE — falsified in 6-neighbour substrate per FTD-0198]`.
- **Engine touched:** none. Pure Python lattice.
- **Manuscript touched:** none.
- **Paper touched:** none.
- **Result lives in:** this file + LEDGER FTD-0198 + the CSV/MD result artifacts + the two retag sites. Nowhere else.
