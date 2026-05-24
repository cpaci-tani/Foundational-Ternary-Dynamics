# PRE-REGISTRATION — Threshold-Crossing Statistics vs Born Rule (v1)

**Tag:** `[PRE-REGISTRATION]`. Locks the methodology of a measurement **before** the measurement is run.
**Date:** 2026-05-23.
**LEDGER row:** **FTD-0200** (T1c sub-investigation under FTD-0187).

> **Renumbering annotation (outside frozen scope).** This pre-registration was hash-locked on 2026-05-23 with the proposed LEDGER ID **FTD-0198**. The same session contained an earlier ARC-B1 alpha-readout pre-registration that had already taken FTD-0198 (commit `eaf7681`). Per the FTD-0189 renumbering convention, the chronologically earlier commit keeps the ID; this test was renumbered to **FTD-0200**. The hash-locked tag `preregister-threshold-crossing-born-v1` and the runner SHA256 in §6 retain the registration-time literal "FTD-0198" as provenance; FTD-0200 is the canonical LEDGER ID for all live cross-references. No element of §§2–5 (the frozen construction, sweep, falsifiable prediction, outcome map) is touched by this annotation — only the LEDGER ID labelling.
**Runner:** `scripts/exploration/explore_threshold_crossing_born.py`, SHA256 filled at hash-lock time.
**Hash-lock status:** **PENDING** (see §7). This pre-registration is **not in force** until the git tag `preregister-threshold-crossing-born-v1` is created over a commit containing this file and the runner. **The script must NOT be executed before that tag exists** — a pre-run measurement voids the pre-registration.

---

## §0 — Pre-registration discipline

Everything in §§2–5 below — the construction, the sweep grid, the falsifiable prediction, the outcome → tag mapping, the controls — is **fixed now, before measurement**. The git tag locks the runner's SHA256 at registration time. Any edit to §§2–5 invalidates v1 and forces a fresh v2. The result is reported as it returns — **including a result that retracts an existing corpus assertion** — with no reinterpretation.

This test is corpus-hygiene + a sub-investigation of LEDGER FTD-0187 / target T1c. It tests one specific assertion that appears in the corpus, not all of FTD's Born-rule machinery.

## §1 — Purpose and motivation

LEDGER FTD-0187 (2026-05-21) consolidated the Born-rule derivation status across ~12 theory documents and tagged the load-bearing step **probability = normalized energy density** as `[OPEN]` (target T1c). The audit explicitly notes that "every treatment bridges energy-density → probability-density by assertion."

A subsequent corpus sweep (2026-05-23) identified two documents NOT in the original 12-doc consolidation that carry an even stronger assertion:

- [`SPEC_SIX_ALGORITHMS.md:65`](../01_reference/SPEC_SIX_ALGORITHMS.md): "The probability follows from `|ψ|²` because `ρ = |J|` and **the threshold crossing statistics produce the Born rule**."
- [`AUDIT_EPISTEMIC_AUDIT.md:393`](../07_assessment/AUDIT_EPISTEMIC_AUDIT.md): "threshold crossing statistics reproduce `|ψ|²`."

Neither carries an attached derivation; both assert the claim as if obvious. A standard upcrossing-rate analysis (Rice 1944) for a Gaussian process with mean `μ` and variance `σ²` crossing level `K` gives upcrossing rate `(ω/2π) · exp(−(K − μ)²/2σ²)`, which is *not* a `|μ|²` scaling. So either:

- (a) The corpus claim is true under FTD-specific structure that makes the substrate's stochasticity *not* Gaussian, and identifying that structure is itself a foothold toward T1c, or
- (b) The corpus claim is wrong and the cited documents need honest retags.

This pre-registration tests the claim *as stated* in a simplified Python substrate. A clean positive or negative outcome is informative either way:

- **Positive outcome** is partial support for the corpus assertion (still requires engine-canonical confirmation) and partial empirical foothold for T1c.
- **Negative outcome** is a `[CLOSED NEGATIVE]` on the corpus assertion in this regime, with a recommended retag to either remove the assertion or mark it `[CONJECTURE — unverified]`.

This test does NOT close T1c either way. T1c is a much larger question.

## §2 — The construction (FROZEN)

### §2.1 Carrier — FROZEN

A 3D cubic lattice of side `L = 24` with periodic boundaries. State `s ∈ {−1, 0, +1}³` per voxel; flux `J ∈ ℝ³` per voxel. Standard 6-neighbour face Laplacian for the wave equation. `numpy` floating-point, `numpy.random.default_rng(seed=...)` for all randomness.

**Caveat (acknowledged):** the 6-neighbour Laplacian is a simplification of FTD's 26-neighbour Moore stencil. Any positive or negative outcome of this test is `[NUMERICAL FACT]` of this simplified dynamics. An engine-canonical confirmation is a separate, larger experiment that this v1 does not attempt.

### §2.2 Dynamics — FROZEN

The discrete wave equation with face Laplacian:

```
J(t+1) = 2·J(t) − J(t−1) + c² · Δ_6 J(t)
```

with `c² = 1/3`. No source coupling (we initialize `J` directly to avoid the small-source-amplitude failure mode of the prior scoping run). Mild uniform damping `J ← (1 − γ)·J` per tick with `γ = 0.001` to prevent slow drift.

**Manifestation rule (FTD-canonical):**

```
s(v) = sign(J_dominant_component(v))   if  s(v) = 0  AND  |J(v)| > K_B
s(v) = 0                                if  |s(v)| = 1  AND  |J(v)| < K_B_evap
```

Otherwise `s(v)` unchanged. `K_B` and `K_B_evap` are pre-registered below.

### §2.3 Initial condition — FROZEN

`J(t=0)` is a smooth spatial profile plus a per-trial random perturbation:

```
J_x(v) = A · g(v) · sin(2π·n_x·v_x/L) + ε(v)
J_y(v) = A · g(v) · sin(2π·n_y·v_y/L) + ε(v')
J_z(v) = A · g(v) · sin(2π·n_z·v_z/L) + ε(v'')
```

where:

- `g(v)` is a Gaussian envelope centred at the lattice mid-point with width `σ_env = L/6`, normalized so `max g = 1`.
- `(n_x, n_y, n_z) = (2, 3, 1)`: three orthogonal incommensurate wavenumbers so the profile has rich spatial variation.
- `A = 2.0`: peak amplitude (well above `K_B`).
- `ε(v) ~ Normal(0, 0.05·A)`: per-voxel deterministic per-trial perturbation drawn from `numpy.random.default_rng(seed = 42 + trial_idx)`.

`J(t=-1) = J(t=0)` (zero initial velocity).

### §2.4 Parameters — FROZEN

| Parameter | Value | Source |
|---|---|---|
| `L` (lattice side) | 24 | balances statistics vs runtime |
| `c²` | 1/3 | FTD-canonical from D=3 |
| `γ` (damping) | 0.001 | small, prevents slow drift |
| `K_B` (manifest threshold) | 0.5 | tuned so manifestations occur in the bulk of the |J| range, not on extremes |
| `K_B_evap` (evaporation threshold) | 0.25 | half of K_B, conventional |
| `A` (initial amplitude) | 2.0 | so `|J|` spans roughly 0 to A, crossing K_B |
| `σ_env` (envelope width) | L/6 = 4.0 | gives broad spatial variation |
| `(n_x, n_y, n_z)` | (2, 3, 1) | incommensurate wavenumbers |
| `ε_scale` | 0.05·A = 0.10 | small per-trial perturbation |
| `n_trials` | 100 | ensemble size for averaging |
| `ticks_per_trial` | 80 | enough ticks for J to oscillate and cross K_B many times |
| `seed_master` | 42 | deterministic |

## §3 — Measurement (FROZEN)

For each voxel `v` (excluding the central 3×3×3 cube and the periodic-boundary 2-cell rim):

- `count(v)` = number of manifestation **events** (transitions `s = 0 → ±1`) across all 100 trials and all 80 ticks per trial.
- `mu_sq(v)` = time-and-trial-averaged `|J(v)|²` over all (trial, tick) pairs.
- `sigma_sq(v)` = time-and-trial-variance of `|J(v)|²` at site `v` (used in the Rice-formula fit).
- `freq(v) = count(v) / (n_trials · ticks_per_trial)`.

Mask: only include voxels with `mu_sq(v) ∈ [0.05, 4·A²]` (well-defined, non-noise, non-saturating).

## §4 — Falsifiable predictions and outcome → tag mapping (FROZEN)

For the masked voxels, three competing fits are made on bin-averaged `(mu_sq, freq)` pairs (14 equal-count bins by `mu_sq`):

### §4.1 Fit H_power: `freq = A · mu_sq^(n/2)`

Equivalently `log freq = log A + (n/2)·log mu_sq`. Fitted via linear regression on `(log mu_sq, log freq)`. Bootstrap 95% CI on `n` over 1000 resamples.

- Born predicts `n = 2.0` (freq ∝ |J|² = mu_sq¹).
- Classical linear predicts `n = 1.0` (freq ∝ |J| = mu_sq^0.5, i.e., n=1 in the freq~|J|^n convention).

### §4.2 Fit H_Rice: `freq = B · exp(− k · (K_B − mu)² / sigma²)`

where `mu = sqrt(mu_sq)`. Fitted by linear regression on `log freq` vs `(K_B − mu)²/sigma²`. Reports goodness-of-fit (R²) and the slope `k`.

### §4.3 Decision rule (PRE-COMMITTED)

| Condition | Outcome | Tag |
|---|---|---|
| H_power n ∈ [1.8, 2.2] AND H_power R² > 0.95 AND H_Rice R² < H_power R² | **A. Born scaling holds** | `[NUMERICAL FACT — Born scaling in 6-neighbour substrate]` + `[OBSERVATION supporting corpus assertion]`; corpus claim partially supported; engine-canonical confirmation still required. |
| H_power n ∈ [0.8, 1.2] AND H_power R² > 0.95 | **B. Linear scaling** | `[NUMERICAL FACT — linear scaling]` + `[CLOSED NEGATIVE for Born]`; corpus assertion does not hold in this regime; SPEC_SIX_ALGORITHMS.md:65 + AUDIT_EPISTEMIC_AUDIT.md:393 need retag. |
| H_Rice R² > H_power R² + 0.05 AND H_Rice R² > 0.90 | **C. Rice / upcrossing scaling** | `[NUMERICAL FACT — Gaussian-process upcrossing rate]` + `[CLOSED NEGATIVE for Born]`; corpus assertion does not hold in this regime; retag as B; additional structural insight: substrate behaves as a Gaussian-like process for threshold crossings. |
| None of A/B/C, or H_power R² ≤ 0.90 and H_Rice R² ≤ 0.90 | **D. No clean scaling** | `[NUMERICAL FACT — no clean scaling]`; inconclusive in this regime; corpus assertion remains untested at the present construction. Need engine experiment. |

Outcomes A, B, C all retag the corpus documents if they fire. Outcome D does not.

### §4.4 Items explicitly out of scope

- The `|ψ|²` *form* question (EF-C3: why quadratic vs `|ψ|` or `|ψ|⁴`) — this is an algebraic-uniqueness question, not addressable by this simulation.
- The Born rule in the canonical engine (26-neighbour Moore stencil, `K_B = 0.511`, full toggle stack). A v2 engine experiment is the natural follow-up if outcome A or any informative outcome obtains.
- The `x_+ = 1/α` identification (FTD-0013) — unaffected by any outcome of this test.
- The collapse-mechanism question (DERIV_COLLAPSE_MECHANISM.md) — the Lindblad / Softplus / Type III₁→I structure is a separate framework not engaged here.
- Bell, interference, entanglement, decoherence dynamics.

## §5 — Methodological guards

**F1 (pattern-matching overreach).** Three competing fits are pre-committed (Born, linear, Rice). The decision rule requires both a good fit AND outperforming the alternatives. No "what other functional form could match" shopping after the fact.

**F3 (aesthetic capture).** Born scaling is the most "elegant" outcome; the pre-registration demands `n ∈ [1.8, 2.2]` AND R² > 0.95 — a tight tolerance that the regression must meet, not a generous post-hoc "near Born" reading.

**F9 (collusion bias).** The runner is deterministic — same seeds, same parameters, same construction. Reproducible by `git checkout preregister-threshold-crossing-born-v1`.

**F10 (tag-as-resolution).** A positive Outcome A does NOT close T1c. It supports one specific corpus assertion in one simplified regime. T1c still requires (i) an engine-canonical version, (ii) a structural reason *why* threshold crossings produce Born statistics specifically (vs the Rice-formula expectation), (iii) the `|ψ|²` form question (EF-C3).

**Scope guard:** the result is conditional on the 6-neighbour face Laplacian, the chosen `K_B`, and the chosen initial-condition family. A negative result here does NOT prove the corpus assertion is wrong universally — only that it fails in this regime. The retag of `SPEC_SIX_ALGORITHMS.md:65` and `AUDIT_EPISTEMIC_AUDIT.md:393` should reflect this: "unverified in the simplest substrate setting tested" rather than "false".

## §6 — Runner specification

**File:** `scripts/exploration/explore_threshold_crossing_born.py`
**SHA256 (hash-lock):** `2781b3cec9d62db9f3635c8672e8da6e31575a37e5c4903c4a9d14d060cab465`
**Dependencies:** `numpy` (≥1.24). Pure stdlib otherwise.
**Output:**
- `scripts/exploration/results/threshold_crossing_born_2026-05-23.csv` — one row per voxel: `(x, y, z, count, mu_sq, sigma_sq, freq)`.
- `scripts/exploration/results/threshold_crossing_born_2026-05-23.md` — bin table + fits + outcome interpretation per §4.3.

**Reproducibility:** running the script at the pre-registered git tag must produce identical CSV byte-for-byte.

## §7 — Hash-lock and execution authorization

This pre-registration becomes hash-locked when:

1. This file is committed to `main`.
2. The runner is committed to the same commit (with real SHA256 inserted in §6).
3. The git tag `preregister-threshold-crossing-born-v1` is created over that commit.

Until all three are done, the runner must NOT be executed.

## §8 — Cross-references

- [`LEDGER.md`](../07_assessment/LEDGER.md) — FTD-0187 (Born-rule consolidation row).
- [`DERIV_COLLAPSE_MECHANISM.md`](DERIV_COLLAPSE_MECHANISM.md) — canonical collapse proposal (out-of-scope for this run).
- [`FOUND_THE_EXISTENCE_FILTER.md`](FOUND_THE_EXISTENCE_FILTER.md) — EF-T5 theorem, EF-C3 conjecture.
- [`SPEC_SIX_ALGORITHMS.md`](../01_reference/SPEC_SIX_ALGORITHMS.md) §line 65 — the target corpus assertion.
- [`AUDIT_EPISTEMIC_AUDIT.md`](../07_assessment/AUDIT_EPISTEMIC_AUDIT.md) §line 393 — the second target assertion.
- [`PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md`](../09_mathematical/PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md) — the prior pre-registration whose pattern this one follows; closed negative as FTD-0197.
