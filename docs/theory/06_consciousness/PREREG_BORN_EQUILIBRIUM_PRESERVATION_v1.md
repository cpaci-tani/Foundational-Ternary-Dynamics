# PRE-REGISTRATION — Born-Equilibrium Preservation Test (v1)

**Tag:** `[PRE-REGISTRATION]`. Locks methodology before measurement.
**Date:** 2026-05-23.
**LEDGER row:** to be assigned (proposed FTD-0199; T1c sub-investigation under FTD-0187).
**Runner:** `scripts/exploration/explore_born_equilibrium_preservation.py`, SHA256 filled at hash-lock time.
**Hash-lock status:** **PENDING**. The script must NOT be executed before the git tag `preregister-born-equilibrium-preservation-v1` exists over a commit containing this file and the runner.

---

## §0 — Pre-registration discipline

Everything in §§2–4 below is **fixed before measurement**. Result reported as it returns. v2 of any element invalidates v1; no retro-credit.

This is a sub-investigation of LEDGER FTD-0187 / T1c. It does NOT close T1c. It tests the (a)+(c) framing — the Dürr-Goldstein-Zanghì (DGZ) 1992 reading — that Born is the equilibrium distribution of the substrate dynamics, so an ensemble of event tests yields Born by LLN + equilibrium preservation.

## §1 — Purpose and motivation

FTD-0198 (this session) ran a v1 threshold-crossing test that asked "does the substrate generate Born scaling from a non-Born initial ensemble?" — and got Rice 1944 upcrossing statistics, not Born. That test asked the wrong question per the (a)+(c) framing.

The right question, per DGZ 1992: **is `|ψ|²` an equilibrium of the substrate dynamics?** That is — *if* an ensemble of substrate configurations is initialized with spatial variance profile `⟨|J(v)|²⟩ ∝ |ψ(v)|²`, do the manifestation events under deterministic evolution preserve that profile, so that the long-run manifestation rate at site `v` tracks `|ψ(v)|²`?

If yes: Born holds by LLN over Born-distributed initial conditions; T1c gains a real foothold; the substrate IS Bohmian-equilibrium-compatible. If no: Born is not preservation in this substrate; T1c requires a different mechanism.

Either outcome is informative for FTD-0187.

## §2 — Construction (FROZEN)

### §2.1 Substrate

Same as FTD-0198 v1:
- 3D cubic lattice `L = 24`, periodic BCs, 6-neighbour face Laplacian.
- Wave equation `J(t+1) = (1−γ) · (2J(t) − J(t−1) + c²·Δ_6 J(t))`, `c² = 1/3`, `γ = 0.001`.
- Manifestation rule: `s = sign(J_x+J_y+J_z)` if `s=0 ∧ |J| > K_B = 0.5`; evaporation if `|s|=1 ∧ |J| < K_B_evap = 0.25`.

**Caveat (acknowledged):** 6-neighbour Python substrate, not engine-canonical Moore stencil. Any outcome is `[NUMERICAL FACT]` of this regime.

### §2.2 Three target `|ψ(v)|²` profiles (FROZEN)

For each profile, define `psi_sq_target(v) = |ψ(v)|²` (unnormalized), then set the per-voxel variance `σ²(v) = α · psi_sq_target(v)` where `α` is chosen so that the maximum value of `σ²(v)` equals `(A_v1)² · (1/3) = (2.0)²/3 = 4/3 ≈ 1.333` (matches v1's initial-amplitude energy scale).

- **Profile 1 — Single Gaussian:** `psi_sq_target(v) = exp(−|v − v_c|² / (2 · σ_ψ²))` with `v_c = (L/2, L/2, L/2)` and `σ_ψ = 3.0` (= L/8). Concentrated initial distribution.
- **Profile 2 — Uniform-with-envelope:** `psi_sq_target(v) = ½·(tanh((R − |v−v_c|)/w) + 1)` with `v_c = (L/2, L/2, L/2)`, `R = 6.0`, `w = 1.5`. Broad uniform region with smooth boundary.
- **Profile 3 — Two-bump superposition:** `psi_sq_target(v) = exp(−|v−v_a|²/(2σ_ψ²)) + exp(−|v−v_b|²/(2σ_ψ²))` with `v_a = (L/2 − 5, L/2, L/2)`, `v_b = (L/2 + 5, L/2, L/2)`, `σ_ψ = 2.5`. Tests whether two-peak structure preserves or merges.

### §2.3 Initial-condition sampling (FROZEN)

For ensemble member `trial_idx`, draw three independent fields:

```
J_x(v) = ε_x(v) · σ(v)
J_y(v) = ε_y(v) · σ(v)
J_z(v) = ε_z(v) · σ(v)
```

where `σ(v) = √σ²(v)` and `ε_{x,y,z}(v) ~ Normal(0, 1)` are drawn from `numpy.random.default_rng(seed = 42 + trial_idx * 3 + axis_idx)` (deterministic per trial). Then `⟨|J(v)|²⟩_ensemble = 3 · σ²(v) ∝ psi_sq_target(v)`.

Initial-velocity term `J(t=−1) = J(t=0)` (zero velocity).

### §2.4 Parameters (FROZEN)

| Parameter | Value |
|---|---|
| `L` | 24 |
| `c²` | 1/3 |
| `γ` | 0.001 |
| `K_B` | 0.5 |
| `K_B_evap` | 0.25 |
| `α` (variance normalization) | 4/3 (matches v1 energy scale) |
| `n_trials` | 100 per profile |
| `ticks_per_trial` | 80 |
| `seed_master` | 42 |

Total runs: 3 profiles × 100 trials × 80 ticks = 24 000 substrate samples.

## §3 — Measurements (FROZEN)

For each profile and each in-mask voxel:

**Primary measurement — long-run rate preservation.**
- `count_long(v)` = total `s = 0 → ±1` transitions across all trials and all ticks `t ∈ [20, 80]` (skip first 20 ticks burn-in).
- `freq_long(v) = count_long(v) / (n_trials · 60)`.
- Compare `freq_long(v)` vs `psi_sq_target(v)`.

**Secondary measurement — first-event distribution.**
- For each trial: record the *first* voxel that manifests during ticks ∈ [0, 80]. Histogram across the ensemble: `hist_first(v) = (count of trials where v was the first manifestation) / n_trials`.
- Compare `hist_first(v)` vs `psi_sq_target(v)`.

Mask: exclude periodic rim of 2 cells. Include all voxels with `psi_sq_target(v) > 0.01` (well-defined target).

## §4 — Falsifiable predictions and outcome → tag mapping (FROZEN)

For each profile, fit `log freq_long(v) = log A + n · log psi_sq_target(v)` on bin-averaged data (14 equal-count percentile bins). Compute slope `n`, R², bootstrap 95% CI.

### §4.1 Per-profile classification

| Condition | Per-profile outcome |
|---|---|
| `n ∈ [0.85, 1.15]` AND R² > 0.95 | **A. Preservation.** Substrate preserves Born scaling. |
| `\|n\| < 0.15` AND R² < 0.50 | **B. Equipartitioned.** Substrate has drifted to uniform distribution (Born profile washed out). |
| Slope outside both ranges, `\|n\| > 0.30` | **C. Drift to non-trivial non-Born equilibrium.** |
| R² ∈ [0.50, 0.95] OR otherwise ambiguous | **D. Inconclusive in this regime.** |

### §4.2 Aggregate verdict across the three profiles

| Aggregate condition | Outcome | Tag |
|---|---|---|
| All 3 profiles in A | **A_strong** | `[NUMERICAL FACT — Born preservation]` + `[OBSERVATION supporting DGZ-equilibrium reading]` — strong T1c foothold; substrate IS Born-equilibrium-compatible. |
| 2 of 3 in A | **A_partial** | `[NUMERICAL FACT — partial preservation]`; preservation depends on profile shape. |
| 2+ of 3 in B | **B_equip** | `[CLOSED NEGATIVE for DGZ-equilibrium in 6-neighbour substrate]` — substrate equilibrium is uniform/equipartition, not Born. T1c not closable via DGZ route in this regime. |
| 2+ of 3 in C | **C_drift** | `[NUMERICAL FACT — drift to non-Born equilibrium]`; substrate has non-Born stationary distribution. |
| Otherwise | **D_mixed** | `[NUMERICAL FACT — mixed/inconclusive]`. |

### §4.3 Secondary observation (no outcome map)

`hist_first(v)` is reported but does not drive the tag. It is an observation about first-event distributions, useful for future v3 design but not pre-committed to any outcome class.

### §4.4 Items out of scope

- The `|ψ|²` *form* question (EF-C3): why quadratic vs `|ψ|` or `|ψ|⁴`. Algebraic-uniqueness; not addressable here.
- The engine-canonical version (26-neighbour Moore + full toggle stack). v3 territory.
- The `|ψ|²` *form* under the Lindblad/Softplus framework (`DERIV_COLLAPSE_MECHANISM.md`).
- Bell, interference, entanglement.
- The central conjecture `x₊ = 1/α` (FTD-0013).

## §5 — Methodological guards

**F1.** Three independent profiles; aggregate verdict requires majority agreement. No single-profile fishing.

**F3.** Outcome A (preservation) is the most "elegant" outcome; the pre-registration requires `n ∈ [0.85, 1.15]` AND R² > 0.95 AND aggregate majority. Tight tolerance against aesthetic capture.

**F9.** Deterministic seeds (`42 + trial_idx*3 + axis_idx` for each initial field; `42` for bootstrap). Reproducible by `git checkout preregister-born-equilibrium-preservation-v1`.

**F10.** A positive Outcome A does NOT close T1c. It supports one specific structural reading (DGZ equilibrium) in one simplified regime. T1c still requires the engine-canonical version + the EF-C3 form question + the question of why the substrate's equilibrium is `|ψ|²`-shaped rather than something else.

**Structural prior (declared before running).** Standard wave equations equipartition energy across modes over long times. Without a confining mechanism, **Outcome B_equip is the *a priori* most likely outcome** for the single-Gaussian and two-bump profiles. The uniform-envelope profile is closer to equipartition initially, so its preservation is partly trivial (Outcome A would be artifactual unless slope ≈ 1 holds across all three). For substrate to genuinely preserve Born, the manifestation+evaporation feedback (the non-linearity in the substrate) must counteract the linear wave-equation spreading. Whether it does is exactly what the test asks.

## §6 — Runner specification

**File:** `scripts/exploration/explore_born_equilibrium_preservation.py`
**SHA256 (hash-lock):** `94b280f40c6ef69b2d6b1f964ca165cdaadc3fd975a56504bb54a9f519ff0732`
**Dependencies:** `numpy` (≥1.24). Stdlib otherwise.
**Output:**
- `scripts/exploration/results/born_equilibrium_preservation_2026-05-23.csv` — one row per (profile, voxel).
- `scripts/exploration/results/born_equilibrium_preservation_2026-05-23.md` — per-profile bin tables + fits + aggregate outcome.

**Reproducibility:** running at the locked tag produces identical CSV byte-for-byte.

## §7 — Hash-lock and execution authorization

Locked when (1) this file is committed, (2) runner committed with real SHA256 in §6, (3) tag `preregister-born-equilibrium-preservation-v1` is created over that commit. Runner MUST NOT be executed before all three.

## §8 — Cross-references

- [`LEDGER.md`](../07_assessment/LEDGER.md) — FTD-0187 (Born consolidation), FTD-0198 (v1 closed-negative), FTD-0199 (this test).
- [`EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md`](EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md) — v1 result that motivated this v2.
- [`PREREG_THRESHOLD_CROSSING_BORN_v1.md`](PREREG_THRESHOLD_CROSSING_BORN_v1.md) — v1 manifest; same machinery, different question.
- [`FOUND_THE_EXISTENCE_FILTER.md`](FOUND_THE_EXISTENCE_FILTER.md) — EF-T5 / EF-C3 (the `|ψ|²` form question, separate workstream).
- Dürr, Goldstein, Zanghì 1992 — quantum equilibrium and the origin of absolute uncertainty (the analytical framework this test mirrors).
