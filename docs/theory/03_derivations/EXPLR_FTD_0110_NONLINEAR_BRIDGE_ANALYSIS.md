# EXPLR — FTD-0110 Nonlinear Bridge Analysis: Sharpening the [OPEN] Gap

**Document type:** Exploratory analysis (does NOT close the bridge)
**Status:** [PARTIAL] — bridge analyzed and sharpened; closure requires engine measurements + further perturbation theory
**Created:** 2026-05-01
**Provenance:** Path D from the 2026-05-01 strategic-direction recommendation; explicit response to CLAUDE.md flag "the cleanest remaining derivation gap; closing it via perturbation theory in the irrep mixing would convert FTD-0110 to [THEOREM]-grade"
**Related:** `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (linear theorem); `FOUND_MINIMUM_DIMENSIONS.md §6.5` (empirical k(A) data)

---

## 0 · Status of the bridge before this document

**Linear theorem (closed):** for δ-localised injection of amplitude `A` at the O_h-fixed center voxel, the 18-point Laplacian's A_{1g} subspace of the 27-block carries mean energy `1/N_base = 1/4` per A_{1g} eigenmode. Verified at machine precision: energy fractions `{3/8, 1/8, 3/8, 1/8}` with mean exactly 0.25.

**Empirical engine data (10 amplitudes from FOUND_MINIMUM_DIMENSIONS.md §6.5, GPU RTX 5090, 2026-04-27):**

| A | k_eng(A) = N_eng/A² |
|---|---|
| 2.00  | 0.250 |
| 10.00 | 0.252 |
| 15.00 | 0.224 |
| 20.00 | 0.234 |
| 28.77 | 0.253 |
| 30.00 | 0.262 |
| 33.05 | 0.245 |
| 50.00 | 0.222 |
| 62.42 | 0.224 |
| 85.70 | 0.212 |
| 117.93 | 0.206 |

Empirical fit reported in `FOUND_MINIMUM_DIMENSIONS.md` line 164:

```
k(A) ≈ 1/4 · (1 − 0.07·log₁₀(A/2))  for A ∈ [2, 120]
     ≈ 1/4 · (1 − 0.030·ln(A/2))
```

**The gap:** the linear theorem predicts `k_linear = 1/4` exactly, but the engine measurement shows a **logarithmic drift** with slope `dk/d(ln A) ≈ −0.030/4 ≈ −0.0076` (about 3% drift per e-fold of amplitude).

This document sharpens the gap by:
1. Identifying three candidate mechanisms for the log-A drift
2. Setting up the perturbation framework for each
3. Estimating the magnitude where possible
4. Identifying the engine experiments that would discriminate among mechanisms

It does **NOT** close the bridge. Closure requires either (a) a successful perturbation calculation matching the empirical slope, or (b) engine experiments isolating the dominant mechanism.

---

## 1 · The structural significance of log-A drift

**Log-A corrections are not arbitrary.** Power-law corrections (1/A, 1/A², etc.) typically come from polynomial nonlinearities at fixed scale. Logarithmic corrections come from **scale-integration** — integrating contributions across many length scales gives a log of the scale ratio.

In QFT this is the running-coupling phenomenon (one-loop logs from RG flow). In condensed matter it appears in critical phenomena near continuous transitions. In FTD's lattice context, log-A drift signals that **as the cluster grows with A, it spans a hierarchy of length scales, and integrating contributions from each scale gives a log of the cluster size ratio**.

A cluster of size `N(A) ~ k·A²` has linear extent `R(A) ~ N(A)^(1/3) ~ A^(2/3)`. The number of "lattice scales" the cluster spans is `~ ln(R(A)) ~ (2/3)·ln(A)`. If each scale contributes a fixed correction `−γ` to k, then total drift is `−γ·(2/3)·ln(A)`, predicting empirical slope `−0.030 = γ·(2/3)` so `γ ≈ 0.045`.

This is structurally consistent with **multi-scale irrep mixing** as the dominant mechanism. The cluster spans many 27-blocks; each block contributes some non-A_{1g} leakage; summing log-many shells gives log-A drift.

---

## 2 · Three candidate mechanisms

Let `k(A) = 1/4 · (1 − Δk(A))` where `Δk(A)` is the fractional drift. The empirical fit gives `Δk_emp(A) ≈ 0.030·ln(A/2)`.

### 2.1 · Mechanism α — Multi-block irrep mixing (likely dominant)

**Picture:** the linear A_{1g} analysis lives on the single 27-block centered at the injection point. For a cluster of radius `R(A) ~ A^(2/3)`, the cluster spans a ball of radius `R` that contains `O(R³)` lattice voxels and `O(R³)` overlapping 27-blocks centered on different voxels.

For each off-center 27-block, the original injection at the original center is NOT at that block's O_h fixed point. So the injection is **not A_{1g}-pure** with respect to that block. Energy "leaks" into non-A_{1g} irreps with respect to off-center blocks.

**Perturbation parameter:** `r/R_block = r` (in units of lattice spacing, since R_block = 1). For a voxel at distance `r` from the original injection center, the leakage to non-A_{1g} irreps of the block centered at distance `r` scales as some function of `r`.

**Order-of-magnitude estimate:** if each off-center block contributes a fractional leakage `~α` to non-A_{1g} modes, and the cluster spans `~R³` blocks effectively (but only `~R` independent radial shells contribute due to spherical symmetry of the underlying Green's function), then total leakage is `~α·R = α·A^(2/3)`.

This gives a **power-law correction `~A^(2/3)`, not log-A**. So the naïve multi-block leakage picture predicts the wrong functional form.

**Refined estimate:** if leakage per shell scales as `~1/r` (geometric attenuation in 3D), summing over shells `r = 1, 2, ..., R`:

```
Δk_α(A) ~ Σ_{r=1}^{R(A)} 1/r ~ ln(R(A)) ~ (2/3)·ln(A)
```

This **does give log-A**. The slope coefficient depends on the per-shell leakage prefactor, which requires a detailed lattice Green's function calculation.

**What's tractable:** the per-shell leakage prefactor can be computed by projecting the lattice Green's function `G_L(r)` from the central injection onto the 4 A_{1g} eigenvectors of each off-center block. This is a finite calculation (involves the 27-block character table at multiple centers) but tedious.

**Estimated effort to close this mechanism:** ~1 week of careful calculation. Output: a structural prediction for the log-A slope coefficient. If it matches empirical `−0.030`, this is the dominant mechanism.

### 2.2 · Mechanism β — Genesis-induced nonlinear irrep mixing

**Picture:** the engine's genesis step is a **sign-thresholding nonlinearity** on the flux density: voxels with `|J|² > K_GENESIS²` manifest with state `s = sign(J·n̂)` for some axis n̂. This is a hard nonlinear function of `J`, even within a single 27-block.

**Effect:** even if the linear A_{1g}-projected energy is exactly `A²/4`, the genesis step pumps energy from A_{1g} into other irreps via the threshold-crossing kink. Above-threshold modes with non-A_{1g} symmetry components are no longer protected by O_h-equivariance (the threshold function is not O_h-equivariant in the irrep-projected sense).

**Perturbation parameter:** `1/(A·K_GENESIS)` — at very high `A`, the threshold-crossing happens at every voxel near the cluster, smoothing the kink into an effectively-linear large-A regime. At intermediate `A`, the kink is sharp and pumps significant non-A_{1g} energy.

**Functional form prediction:** kink-induced nonlinearity in 1D classically gives `Δk_β(A) ~ 1/A^something` (power law), not log-A. So this mechanism by itself probably does NOT explain the log-A drift.

**However**: if the genesis kink interacts with multi-scale irrep structure (Mechanism α), the combined effect could have a log-A signature. Cross-coupling is possible.

**What's tractable:** estimate the per-tick energy leakage from A_{1g} to non-A_{1g} via the genesis sign function. For a Gaussian-distributed flux `J` with variance `σ² ~ A²·G_L(r)`, the rate of threshold-crossing is `~erfc(K_GENESIS/σ)`. Summing over voxels gives a tractable estimate.

**Estimated effort to close:** ~3-5 days. Output: a structural prediction for the genesis-induced non-A_{1g} energy fraction. If small (<0.5%), Mechanism β is sub-dominant; if large, it's competing with α.

### 2.3 · Mechanism γ — Langevin non-equipartition / dissipation

**Picture:** the engine runs Langevin friction `γ_L` and noise temperature `T_L` continuously. In equilibrium, each mode has thermal energy `T_L`. Total Langevin equilibrium energy across all modes is `M·T_L` for `M` modes.

**For amplitudes `A` with injected energy `A² >> M·T_L`:** the cluster's energy dominates the thermal background. The slow A_{1g} mode retains most of the injected energy, with a small fraction dissipated to thermal modes via friction.

**Estimate of friction loss per tick:** `−γ_L·E_slow`. Over the cluster's relaxation time `τ ~ 1/|λ_slow| ~ 1/1.586`, the dissipated fraction is `~γ_L·τ ~ γ_L/1.586`.

**Engine value:** Langevin friction `γ_L = 0.02` per tick (per FTD-0051 Langevin infrastructure). Per-tick dissipation fraction `~0.02/1.586 ≈ 0.013`. After equilibration time `~1/γ_L = 50` ticks, dissipated fraction is `~50·0.013 = 0.63`. Far too large.

**Refined estimate:** the cluster reaches steady state when injection rate balances dissipation rate. Steady-state cluster energy = `(injected/4) · γ_L·τ_inj/(γ_L·τ_inj + 1)` for some injection timescale `τ_inj`.

**Functional form:** this gives a steady-state energy fraction that is **A-independent** (since γ_L is A-independent). Cannot explain log-A drift on its own.

**Refined picture: Langevin-amplitude interaction.** The Langevin temperature `T_L` is a fixed engine parameter. At small A (cluster energy ~ T_L), Langevin completely dominates and the cluster is "thermal". At large A (cluster energy >> T_L), Langevin is negligible. The crossover is at `A* ~ √(M·T_L)`. **For `A` near `A*`, Langevin contributes a non-trivial drift.** For `A >> A*`, drift saturates.

**Engine value:** `T_L = 0.005` (per FTD-0051). Total mode count `M ~ L³` for L=32, so `M·T_L ~ 32³·0.005 = 164`. So `A* ~ √164 ≈ 13`.

**The crossover is at A* ≈ 13**, which is exactly in the middle of the empirical k(A) drift range (A from 2 to 120)! This is suggestive — Mechanism γ may contribute significantly at small/intermediate A.

**What's tractable:** standard Langevin equilibrium analysis on the linear-Laplacian-projected modes. Should give an explicit `k(A)` curve from the Langevin-interaction balance.

**Estimated effort to close:** ~3-5 days of paper-and-pencil. Output: predicted k(A) curve from Langevin-only mechanism. Compare to engine data.

---

## 3 · Mechanism comparison and discrimination

**Mechanism predictions for k(A):**

| Mechanism | Functional form | Direction | Tractable? |
|---|---|---|---|
| α: Multi-block leakage | `~ −γ_α · ln(A)` | Decrease | Yes (~1 week) |
| β: Genesis nonlinear mixing | `~ −γ_β · A^(−p)` (p > 0) | Decrease | Yes (~3-5 days) |
| γ: Langevin dissipation | crossover at A* ≈ 13 | Both | Yes (~3-5 days) |

**Empirical fit** `Δk(A) ≈ 0.030·ln(A/2)` is most consistent with **Mechanism α** (multi-scale irrep leakage), with possible Mechanism γ contribution at small `A`.

**Engine experiments to discriminate (proposed in `FOUND_MINIMUM_DIMENSIONS.md` §6.5 D3a/D3b but not yet run):**

- **D3a — Vary `K_GENESIS_KINETIC_DRAIN`:** if `k ∝ DRAIN²`, Mechanism β is dominant.
- **D3b — Vary `K_EVAP_RATE`:** if `k` scales monotonically with evaporation, Mechanism γ-like dynamics (cluster energy balance) is dominant.
- **D3c — Vary `T_L` (Langevin temperature):** if `k(A)` curve shifts with `T_L`, Mechanism γ is significant. If unaffected, γ is sub-dominant.
- **D3d — Vary `L` (lattice size):** Mechanism α predicts saturation at `L < R_cluster` (cluster fills the lattice). Engine measurement at L=64 vs L=128 at fixed A would discriminate.

These four experiments, plus the perturbation calculations above, are the path to closing the bridge.

---

## 4 · What this analysis establishes

**[NEW]:**
- Quantification of the log-A drift slope: `−0.030/ln-unit` empirically, equivalently `−0.0076 absolute per e-fold of A`.
- Identification of three candidate mechanisms with specific functional forms.
- Mechanism α (multi-scale irrep leakage) is structurally consistent with log-A drift; mechanisms β and γ require additional assumptions to match.
- Concrete engine experiments (D3a-D3d) to discriminate among mechanisms, building on the proposals already in `FOUND_MINIMUM_DIMENSIONS.md` §6.5.
- Structural significance: log-A drift is **not arbitrary** — it's a signature of multi-scale physics, suggesting the cluster is RG-like in some lattice analog of running couplings.

**[STILL OPEN] (the bridge itself):**
- Mechanism α perturbation calculation (per-shell A_{1g} leakage prefactor) — ~1 week of work.
- Mechanism β estimate (genesis kink-induced energy redistribution) — ~3-5 days.
- Mechanism γ Langevin equilibrium analysis with A-dependent crossover — ~3-5 days.
- Engine experiments D3a-D3d — bounded engine work, ~2-3 days each.

**[NOT CHANGED]:**
- The linear theorem (k_linear = 1/4 from O_h representation theory) stands as a [DERIVED] result.
- The empirical match (cluster size matches mass formulas within ~5% across SM particles e/μ/π/K/p/τ) stands as [STRONGLY MOTIVATED CONJECTURE].
- The bridge between linear theorem and full-engine empirical match remains [OPEN].

---

## 5 · Why this is genuinely hard

The bridge has three layers of complication that compound:

1. **Multiple mechanisms at play.** The empirical k(A) drift is plausibly the sum of contributions from α, β, γ — possibly with cross-coupling. Disentangling them requires both theory and parameter-varied engine experiments.

2. **Nonequilibrium dynamics.** The cluster is not in thermal equilibrium with the Langevin bath at large A. It's a long-lived metastable structure. Standard equilibrium statistical mechanics doesn't directly apply.

3. **Discrete-continuous mismatch.** The linear A_{1g} theorem is on a continuous-coefficient Laplacian, but the genesis step is a discrete sign function. The mismatch between continuous and discrete dynamics is the source of multiple corrections.

These are not "just unsolved" — they are genuinely hard. A complete closure requires sophisticated machinery: lattice perturbation theory, irrep-mixing tensor calculations, Langevin equilibrium analysis, and engine experiments. **The cleanest path forward is theory + engine experiments together, not theory alone.**

---

## 6 · LEDGER status

This document does NOT close FTD-0110's [OPEN] sub-claim. It updates the description of the gap to be more concrete and identifies the three concrete sub-questions:

- **FTD-0110-α:** multi-block irrep leakage perturbation calculation (PERT)
- **FTD-0110-β:** genesis-kink induced non-A_{1g} energy estimate (PERT)
- **FTD-0110-γ:** Langevin equilibrium with amplitude crossover (THEORY)
- **FTD-0110-D3:** engine experiments D3a/b/c/d (ENGINE)

A LEDGER entry FTD-0119 is filed at the [BRIDGE-ANALYZED] tag to record this analysis without claiming closure.

---

## 7 · Verification

Linear theorem energy distribution `{3/8, 1/8, 3/8, 1/8}` and mean `1/4` re-verified at floating-point precision via 4×4 A_{1g}-projected Laplacian diagonalization (matches `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` C2 result independently). No new closed-form result added.

---

## 8 · What this document does NOT claim

- **NOT a closure of the bridge.** The bridge analysis sharpens the gap; it does not close it.
- **NOT a derivation of the empirical slope `−0.030`.** That requires Mechanism α's perturbation calculation, which is identified but not executed here.
- **NOT a falsification of any mechanism.** All three mechanisms (α, β, γ) remain candidates; the analysis only argues α is most consistent with the *functional form*.
- **NOT a new theorem.** Spine count remains 9; FTD-0110 main claim remains [STRONGLY MOTIVATED CONJECTURE] for the full nonlinear regime.

---

## 8.7 · Phase C (Langevin equipartition extension) — also FALSIFIED

After Phase B's falsification of the 1/√d law as a per-block efficiency,
the next natural candidate was the **Langevin equipartition** extension
of the linear theorem:

> **Hypothesis (Phase C):** at each voxel x, the per-voxel manifestation
> efficiency is `η(x) = (block_energy_at_x × 1/d_G(x)) / total_injected_energy`.
> Cluster size: `N(A) = A² × Σ_x η(x)` over cluster voxels.

This recovers the linear theorem at the central block: block energy ≈ A²,
d_G = 4 → k = 1/4. ✓

### 8.7.1 · Phase C result

Tested in `scripts/proofs/proof_ftd0110_full_aggregation.py`:

| A | k_emp | k_pred | k_pred/k_emp |
|---|---|---|---|
| 10 | 0.252 | 0.914 | 3.6 |
| 50 | 0.222 | 1.514 | 6.8 |
| 117.93 | 0.206 | 1.717 | 8.3 |

`k_pred` ranges 0.9-1.7 — **dramatically off**, and worse than that:
**`k_pred > 1` for clusters at A ≥ 15**, which is unphysical
(`k > 1` means manifesting more energy than was injected).

### 8.7.2 · Diagnosis of the Phase C failure

The over-counting is structural: each lattice voxel `y` belongs to **27
different "home blocks"** (the block centered at y, plus the 26 blocks
centered at y's neighbors where y appears as a non-center voxel). My
per-block summation counts each voxel's field amplitude in all 27
blocks it belongs to.

A naïve fix `η(x) = |G_L(x)|² / d_G(x) / total_E` (per-voxel rather than
per-block) avoids over-counting but gives `η(0) ≈ 0.18 / 4 ≈ 0.045`,
much smaller than the linear theorem's 1/4 = 0.25.

The fundamental issue: **the linear theorem's "1/N_base = 1/4" is a
TOTAL energy fraction (over all 4 A_{1g} modes of the central block),
not a per-voxel quantity**. The cluster size A²/4 emerges from
"slow mode supports A²/4 voxels of cluster" — a TOTAL claim, not
per-voxel.

For multi-block extension, we'd need a coherent way to aggregate
TOTAL slow-mode energies across blocks without double-counting. The
candidate frameworks tested in Phases B and C don't supply this
aggregation cleanly.

### 8.7.3 · Cumulative status

After this commit:
- **Mechanism α as 1/√d law**: FALSIFIED (Phase B)
- **Mechanism α as Langevin-equipartition**: FALSIFIED (Phase C, this section)
- **Mechanism β (genesis-kink)**: untested
- **Mechanism γ (Langevin amplitude-crossover at A* ≈ 13)**: untested
- **f_slow(r) distance-dependence (Phase B finding)**: documented but
  not promoted as a candidate framework — the smooth-vs-localized
  distinction it captures predicts k INCREASING with A, opposite to
  empirical drift.

**The bridge gap is sharper but more discouraging.** The two natural
representation-theoretic frameworks both fail. The remaining
candidates (β, γ) are not representation-theoretic and would require
different machinery (genesis-kink statistical mechanics, or
non-equilibrium Langevin response).

### 8.7.4 · Honest verdict

Phase B + Phase C jointly establish that **per-block local-symmetry
analysis does NOT give a clean closed-form derivation of empirical
k(A) drift**. The 1/√d match at large A reported in commit `e05d9d6`
remains POSSIBLY COINCIDENTAL.

Bridge closure via this route is now closed-negative. Future work
must either:
1. Invoke Mechanism β (genesis-kink induced energy redistribution)
2. Invoke Mechanism γ (Langevin amplitude-crossover dynamics)
3. Take a fundamentally different framework (e.g., RG flow on the
   global lattice, or direct simulation matching)

Each requires substantial new machinery. The bridge remains [OPEN]
with the gap now well-characterized: **two natural candidates
ruled out, two more to investigate, and no obvious shortcut**.

CLAUDE.md anti-target discipline preserved throughout: when each
candidate framework was tested by direct calculation, we reported
the result honestly. No fishing for laws that fit; we ruled out
what didn't fit and named what's still open.

New verification script: `scripts/proofs/proof_ftd0110_full_aggregation.py`.

---

## 8.6 · Mechanism α 1/√d hypothesis FALSIFIED (2026-05-01, Phase B)

**The 1/√d empirical match identified in §8.5 is now FALSIFIED as a
structural per-block efficiency law** by the Phase B analysis
(`scripts/proofs/proof_ftd0110_langevin_steady_state.py`).

### 8.6.1 · Phase B method

For each off-center block at sample positions, compute:
1. The global lattice Poisson Green's function `G_L(r)` at L=32 (standard
   cubic-lattice convention).
2. The 27-component field vector `J_block(δ) = G_L(x_0 + δ)` at each
   block center `x_0`.
3. The trivial-subspace fraction `f_block = ‖P_G·J_block‖² / ‖J_block‖²`.
4. The slow-mode fraction
   `f_slow = |⟨v_slow_local, P_G·J_block⟩|² / ‖P_G·J_block‖²`
   where `v_slow_local` is the eigenvector of the smallest |λ| eigenvalue
   of the trivial-subspace-projected Laplacian.

### 8.6.2 · Phase B findings

**Finding 1: f_block = 1.0 universally.** The global field generated by
an origin source is invariant under each block's local symmetry (since
local symmetry ⊂ O_h and field is O_h-symmetric). It therefore lives
entirely in the trivial-irrep subspace at every block. **Expected
structurally; confirms Phase A.**

**Finding 2: f_slow varies with both symmetry AND distance.** Per-block
slow-mode fractions averaged over sample positions:

| Symmetry | d | ⟨f_slow⟩ | 1/√d | f_slow / (1/√d) |
|---|---|---|---|---|
| O_h | 4 | 0.87 | 0.50 | 1.74 |
| C_4v | 9 | 0.79 | 0.33 | 1.99 |
| C_3v | 10 | 0.68 | 0.32 | 2.13 |
| C_2v | 12 | 0.71 | 0.29 | 2.47 |
| C_s | 18 | 0.79 | 0.24 | 3.36 |
| C_1 | 27 | 0.83 | 0.19 | 4.34 |

`f_slow` is consistently large (0.68-0.87), NOT matching the 1/√d law
(0.19-0.50). The ratio f_slow / (1/√d) varies systematically with d
from 1.74 to 4.34 — **definitely not consistent with 1/√d as the
underlying structural law**.

**Finding 3: distance-dependence dominates.** For fixed symmetry,
f_slow grows toward ~0.85 as the block's distance from origin
increases. Examples:
- C_4v at (1,0,0): f_slow = 0.66
- C_4v at (5,0,0): f_slow = 0.85
- C_3v at (1,1,1): f_slow = 0.41 (block contains origin → highly
  localized field)
- C_3v at (3,3,3): f_slow = 0.84 (smooth field at distance)

**Structural interpretation:** the global field's overlap with the
local slow mode (the "uniform-on-block" eigenvector) approaches 1 as
the field becomes more uniform within the block — i.e., as distance
from the source grows. Blocks containing or near the origin have
strongly localized field profiles that distribute energy across
multiple eigenmodes, lowering f_slow. **This is a smooth-vs-localized
distinction, NOT a symmetry-driven 1/√d law.**

### 8.6.3 · Verdict

**The Mechanism α "1/√d as per-block manifestation efficiency"
hypothesis is FALSIFIED.** The actual per-block f_slow does not match
1/√d either as a function of d or as a function of position.

The empirical `⟨1/√d⟩ ≈ k_emp(A)` match at large A reported in §8.5
is therefore **POSSIBLY COINCIDENTAL** — an artifact of the radial-
averaging procedure used in `proof_ftd0110_multiblock_structure.py`,
not a structural law of the cluster physics. The fact that the
spherical average of 1/√d over a uniform-density ball happens to
produce numbers near 0.20 (close to empirical k(A) at large A) is
geometric, not dynamical.

### 8.6.4 · What this rules out vs leaves open

**Ruled out (this commit):**
- Per-block manifestation efficiency = 1/√d_G [FALSIFIED]
- 1/√d as a structural law derived from local-symmetry trivial-irrep
  dimensions [FALSIFIED]
- Mechanism α as "off-center blocks lose efficiency by local-symmetry
  spreading" [FALSIFIED]

**Still open:**
- Mechanism β (genesis-kink-induced irrep mixing): not tested in this
  Phase. May still contribute to log-A drift.
- Mechanism γ (Langevin amplitude-crossover at A* = √(L³·T_L) ≈ 13):
  not tested in this Phase. The crossover scale matches the
  empirical drift range and remains plausible.
- Multi-scale RG-style picture without per-block-symmetry reduction:
  not tested. The empirical log-A drift might come from genuine
  multi-scale renormalization rather than block-by-block analysis.
- The actual f_slow distance dependence revealed in Phase B is a
  candidate replacement law: cluster manifestation might be governed
  by `<f_slow(r)>` averaged over the cluster's radial profile, where
  the dependence on r is set by the global Green's function smoothness,
  not by symmetry. Worth investigating in a future session.

### 8.6.5 · Consequences for FTD-0110

The bridge **remains [OPEN]**. The 1/√d empirical match is now flagged
as POSSIBLY COINCIDENTAL. The cleanest structural framing of cluster
manifestation continues to be the linear theorem (k = 1/N_base = 1/4
at the central block); the multi-block extension does NOT have a
clean closed form following from Mechanism α.

Closing the bridge requires either:
1. Investigating Mechanism β or γ via Phase B-style direct calculation.
2. Computing `<f_slow(r)>` for varying cluster radii and identifying
   a structural pattern.
3. A different framework entirely (e.g., direct solution of the
   genesis-Langevin steady state).

Each is a substantial research direction. The bridge gap is
**sharper than before this Phase** but not closed.

### 8.6.6 · Honest meta-note

The plan (`~/.claude/plans/i-want-to-try-crispy-charm.md`) explicitly
included a failure path: "if the 1/√d law doesn't derive, document
honestly". Phase B is that failure path. The work product:
- The 1/√d empirical match is now **explicitly flagged** as possibly
  coincidental (no longer presented as structurally suggestive).
- The actual per-block f_slow data is **published** for future
  reference, including the distance-dependence finding.
- The Mechanism α hypothesis is **explicitly closed-negative**.
- Phases C/D/E of the original plan are **superseded**: there's no
  reason to build a cluster aggregation or interpolation function on
  top of a falsified per-block law.

This is the discipline working as designed. CLAUDE.md anti-target
rule held: when the empirical match was tested via direct
calculation, it failed; we report the failure rather than searching
for a different law that happens to fit. The bridge stays [OPEN]
with substantially sharpened understanding.

New verification script: `scripts/proofs/proof_ftd0110_langevin_steady_state.py`.

---

## 8.5 · Mechanism α detailed analysis (2026-05-01 — SUPERSEDED by §8.6)

> **NOTE (2026-05-01 evening):** The 1/√d empirical regularity reported
> in this section was tested via direct calculation in Phase B and
> **FALSIFIED**. See §8.6 for the falsification analysis. The content
> below is preserved for the historical record.

A focused session on Mechanism α (multi-block irrep leakage) yielded a
**non-trivial empirical regularity** but did not close the bridge.

### 8.5.1 · Off-center block trivial-irrep dimensions

For a 27-voxel block centered at distance r from the origin, the local
symmetry that fixes the block center is a subgroup of O_h. The trivial-
irrep dimension of the natural representation on the 27 voxels gives
the dimension of the "fully-symmetric" subspace under that local
symmetry:

| Block position | Local symmetry | Trivial-irrep dim |
|---|---|---|
| Central (origin) | O_h (order 48) | **4** = N_base [linear thm] |
| Axis (n, 0, 0) | C_4v (order 8) | 9 |
| Body-diagonal (n, n, n) | C_3v (order 6) | 10 |
| Face-diagonal (n, n, 0) | C_2v (order 4) | 12 |
| Face-general (n, m, 0), n≠m | C_s (order 2) | 18 |
| Generic (n, m, p), all distinct | C_1 (order 1) | 27 |

Computed via Burnside's lemma: dim(trivial) = (1/|G|) Σ_g #fixed(g).

### 8.5.2 · Naive 1/d model fails

Using per-block manifestation efficiency `η = 1/d` (analogous to the
linear theorem's `1/N_base = 1/4` at the central block):

```
< η > = Σ_categories (fraction of cluster) × 1/d_category
```

For a sphere of radius R(A), this gives `< η > = 0.10 → 0.04` across
A ∈ {10, 120}. Empirical k(A) drops only from 0.25 to 0.20 — the naive
1/d prediction overshoots empirical drift by ~5×.

### 8.5.3 · The 1/√d empirical regularity (NEW, 2026-05-01)

Substituting `η = 1/√d` instead:

| A | k_emp | < 1/√d > | Match |
|---|---|---|---|
| 10 | 0.252 | 0.315 | overshoots (linear regime) |
| 28.77 | 0.253 | 0.243 | within 4% |
| 50 | 0.222 | **0.224** | **EXCELLENT (1%)** |
| 85.70 | 0.212 | **0.212** | **EXCELLENT (0%)** |
| 117.93 | 0.206 | 0.207 | EXCELLENT (1%) |

**At A ≥ 50, the `< 1/√d >` average over the cluster matches
empirical k(A) to ~1-2%** — within engine measurement precision. At
smaller A, the linear theorem `k = 1/N_base = 1/4` captures the data;
the 1/√d model overshoots.

**Asymptotic structural prediction:** as the cluster grows, generic
blocks (C_1, d=27) dominate, giving asymptotic
`< 1/√d > → 1/√27 = 1/(3√3) = 1/D^{3/2}` for D=3. This is a clean
structural number; numerical value 0.1925.

### 8.5.4 · What this is and is NOT

**What this is:**
- A **structural framework** mapping multi-block geometry to per-block
  efficiency via the local-symmetry trivial-irrep dimension.
- A **non-trivial empirical regularity**: the < 1/√d > law captures
  empirical k(A) at large A within engine precision.
- A **clean asymptotic prediction** at large clusters:
  k_asymptotic → 1/D^{3/2} for D=3, a structural number.

**What this is NOT:**
- A *derivation* of the 1/√d law from FTD axioms. The 1/√d agreement
  is an empirical fit, not a theorem. **Possible coincidence; possibly
  structural.**
- A complete bridge from the linear theorem to empirical drift. The
  1/√d model FAILS at small A (where the linear theorem applies) and
  the small-A → large-A *interpolation function* is unknown.
- A proof that the asymptote is exactly `1/D^{3/2}`. The empirical fit
  `k(A) = ¼·(1 − 0.030·ln(A/2))` predicts continued logarithmic drift
  past 1/√27 = 0.192 — the < 1/√d > approach to 1/√27 disagrees with
  the empirical fit's asymptote at very large A. Higher-A data would
  discriminate.

### 8.5.5 · Updated path to closure

The closure path identified in §10 of this document is now refined:

1. **Derive the 1/√d law structurally.** If the per-block manifestation
   efficiency really IS 1/√d, this is a representation-theoretic
   prediction that should follow from a careful Langevin analysis of
   the slow-mode manifestation rate as a function of local symmetry.
   ~3-5 days of careful calculation. Output: either a derivation that
   confirms 1/√d, or a closed form for the actual law.

2. **Compute the small-A to large-A interpolation.** The empirical k(A)
   is between linear-theorem (1/4 at small A) and 1/√d-asymptote
   (0.192 at very large A). The interpolation function is governed by
   the cluster's spatial extent vs the linear-theorem central block. A
   Padé-approximant-style interpolation derived from the leading order
   could match empirical data.

3. **Engine experiments at higher A** (A > 200) to discriminate between
   the empirical-fit log-A drift and the 1/√d structural asymptote.
   ~1-2 days engine work.

### 8.5.6 · Status update

**Mechanism α is now MORE plausible structurally** than initially
characterized, given the < 1/√d > empirical match at large A. But the
bridge **remains [OPEN]** — the 1/√d law is empirical, not derived,
and the small-A regime requires the linear theorem.

The single-session output: a concrete framework + empirical regularity
+ refined closure path. Verification script:
`scripts/proofs/proof_ftd0110_multiblock_structure.py` (PASS at large
A, by construction tabulating the < 1/√d > average).

---

## 9 · Summary

The FTD-0110 nonlinear bridge gap is **structurally sharper after this analysis** but **not closed**. Three concrete mechanism-candidates (multi-block irrep leakage, genesis nonlinear mixing, Langevin amplitude-crossover) are identified, each with tractable perturbation routes (~3-5 days to ~1 week per mechanism). The empirical log-A drift signature is structurally consistent with Mechanism α's multi-scale picture. Engine experiments D3a-D3d would discriminate among mechanisms with bounded effort (~2-3 days each). The full closure path is now mapped: ~3-4 weeks of combined theory + engine work, or a focused ~1-week perturbation calculation if Mechanism α turns out to be unambiguously dominant.

This is the cleanest possible "structural gap analysis" without the closure itself. The closure remains the highest-leverage open derivation gap in the project per CLAUDE.md.

---

*End of analysis.*
