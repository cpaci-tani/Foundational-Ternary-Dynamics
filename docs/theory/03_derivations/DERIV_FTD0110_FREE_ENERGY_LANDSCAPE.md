# Derivation — Cluster phenomenology as free-energy landscape (FTD-0110 Phase 6)

**Tag:** [DERIVED · structural framework] / [PARTIAL · empirical landscape parameters from Phase 4 data]
**Date:** 2026-04-29
**LEDGER row:** FTD-0110 (theoretical extension of Phase 1-5 empirical results)
**Companion:** [`DERIV_FTD0110_VARIANCE_ENTROPY.md`](DERIV_FTD0110_VARIANCE_ENTROPY.md) — empirical variance + regime structure
**Reframe origin:** chat conversation 2026-04-28/29 (user: "frequency IS time; amplitude is event complexity; entropy is difficult bookkeeping").

---

## 0 · Summary

The Phase 1-5 empirical data (committed `cadd2ef` and `bei4sn71g` follow-up) reveals a **non-monotone temperature dependence** of cluster-size variance that does not fit a naïve Arrhenius (single-barrier activation) reading. From T=0.005 to T=0.040 at A=50:

```
T=0.005:  events 0.0075/tick   σ_within = 1.17    (5/10 metastable)
T=0.010:  events 0.0006/tick   σ_within = 0.11    (homogenization minimum)
T=0.020:  events 0.0323/tick   σ_within = 6.39    (active regime)
T=0.040:  events 0.0632/tick   σ_within = 8.37    (linear-in-T)
```

This pattern — small activity at low T, MINIMUM at intermediate T, then growing activity at high T — is characteristic of a **multi-basin free-energy landscape**. The cluster manifestation is not a single-barrier activated process but a configuration-space landscape with at least two qualitatively different basin types:

- **Shallow metastable basins** (cluster sizes 540-560 voxels at A=50): accessible from a wide initial-condition distribution; thermal noise above some small T_meta drives seeds out into the global minimum.
- **Deep global minimum** (~553 voxels at A=50, T=0.010): the stable bound-state configuration; thermal noise below T_homog stays trapped, above T_homog → T_active escapes via boundary events.

The two-stage T-dependence (homogenization minimum + activation threshold) is a quantitative empirical fingerprint of this multi-basin structure.

---

## 1 · The free-energy landscape framing

### 1.1 Cluster configuration space

The cluster's microscopic state is the manifestation pattern: which voxels in some neighborhood of the injection center have `state ≠ 0`. For a cluster of mean size `⟨N⟩`, this is a binary vector in `{0, 1}^V` where `V` is the cluster's effective volume. The cluster size `N = Σ_v X_v` is one collective coordinate.

For variance analysis at fixed amplitude `A`, the cluster size `N` is a 1-d order parameter. The **free-energy landscape** `F(N)` describes the (effective) potential surface in this 1-d cut:

```
F(N) = -k_B T ln ⟨exp(-βE(config)) | total cluster size = N⟩
```

where the conditional ensemble is over all microstates with the given cluster size. In equilibrium, the probability `p(N) ∝ exp(-βF(N))`, so `F(N)` directly determines which N values are populated.

### 1.2 Multi-basin structure inferred from data

The observed variance structure tells us about `F(N)`:

**Phase 2A (A=10, T=0.005):** all 10 seeds produce N ∈ {25, 26}. Implies `F(N)` has a **sharp single minimum** near N=25 with very narrow (~2 voxel) basin, no accessible neighboring minima.

**Phase 1C (A=10, 30 seeds):** distribution is {22, 24, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 28, 29, 29, 30, 30, 31}. Modal at 26, secondary peaks at 25/27, tail to 22 and 31. Implies `F(N)` has **a primary minimum at N≈26 plus shallow secondary minima** at N ≈ {22, 24, 25, 27, 29, 30, 31} — a "ladder" of metastable configurations with increasing F.

**Phase 2D (A=50, T=0.005):** all 10 seeds produce N ∈ {540, 543, 543, 550, 553, 559, 566, 566} (with some temporal drift in 5/10 seeds). Implies `F(N)` has **multiple shallow minima** on the scale of ~5-10 voxels apart, with thermal noise able to push seeds between them but not over a deeper barrier to runaway.

**Phase 3 (A=50, T-sweep):** the T=0.010 minimum at ⟨N⟩=553.6 with σ=7.07, σ_within=0.11 implies thermal noise has homogenized seeds INTO ONE deepest basin (the global minimum of `F(N)` at A=50). At T=0.020 (σ_within=6.39, events 0.032/tick) the noise is large enough to enable barrier-crossing within and beyond this minimum.

The picture: `F(N | A=50)` has a deepest basin around N=553-554 plus several shallower neighboring minima (at e.g. 540, 543, 559, 566). At T=0.005, seeds can land in any of these. At T=0.010, seeds equilibrate to the deepest. At T=0.020+, thermal energy exceeds the smallest barrier and ongoing transitions occur.

### 1.3 Quantitative landscape parameters from the data

For a multi-minimum `F(N)`, the typical barrier height between adjacent minima `ΔF*` controls the activation threshold:

```
T_meta < T < T_homog : metastability accessible by IC; thermal noise too small to escape
T_homog < T < T_active : thermal noise drives all seeds to global minimum
T_active < T          : thermal noise crosses inter-basin barriers, ongoing events
```

From the Phase 3 data at A=50:
- T_meta < 0.005 (some metastable seeds at T=0.005)
- 0.005 < T_homog ≲ 0.010 (homogenization observed at T=0.010)
- 0.010 < T_active ≲ 0.020 (activation observed at T=0.020)

So `T_homog ≈ 0.008 ± 0.003` and `T_active ≈ 0.015 ± 0.005` at A=50.

The barrier height `ΔF* ≈ T_active · ln(timescale)` for typical Langevin dynamics. With observation timescale ~500 ticks and characteristic correlation time ~50 ticks, `ln(10) ≈ 2.3`, so `ΔF* ≈ 0.015 · 2.3 ≈ 0.035` (in engine energy units). This is the typical inter-basin barrier height in `F(N)` near the global minimum.

### 1.4 The homogenization minimum as a free-energy signature

The non-monotonicity at T=0.010 (event rate 0.0006, far less than T=0.005's 0.0075) is structurally meaningful. It says:

> At T=0.010, the thermal noise IS large enough to escape ALL shallow metastable minima (so the 5/10 metastable seeds at T=0.005 get driven into the deepest minimum), but is NOT yet large enough to cross the larger barriers OUT OF the deepest minimum to ongoing churn.

The minimum exists because the homogenization (escape from shallow basins) and activation (escape from deep basin) have different barrier heights. The observation `T_homog ≈ 0.008` < `T_active ≈ 0.015` says the deepest basin is roughly 2× as deep as the shallow ones in `F(N)`.

This is a quantitative empirical handle on the cluster's free-energy landscape that wasn't accessible from snapshot-only analysis.

---

## 2 · Connection to the Anova decomposition

The empirical Anova decomposition `Var_total = Var_within + Var_between` maps cleanly onto the free-energy framework:

- **Var_between** = variance of per-seed mean cluster sizes ⟨N⟩_seed
  - At low T (frozen regime): different seeds reach different basins of F(N); Var_between = variance of basin minima populated by the seed ensemble
  - At high T (active regime): all seeds reach equilibrium; Var_between = small (just noise in the equilibrium average)

- **Var_within** = average per-seed temporal variance σ_t²
  - At low T (frozen regime): each seed locked in one basin, σ_t = 0
  - At high T (active regime): seeds undergo barrier-crossing transitions, σ_t > 0

This explains the empirical pattern:

```
T=0.005:  Var_within ≈ 1.4     Var_between ≈ 98     %temporal = 1.4%
                              ↓ shallow basins populated by IC, then frozen
T=0.010:  Var_within ≈ 0.01    Var_between ≈ 50     %temporal = 0.0%
                              ↓ all seeds in deepest basin (homogenized)
T=0.020:  Var_within ≈ 41      Var_between ≈ 143    %temporal = 22%
                              ↓ active transitions; deepest-basin width grows
T=0.040:  Var_within ≈ 70      Var_between ≈ 244    %temporal = 22%
                              ↓ more transitions, but %temporal capped
```

The 22% asymptote at T=0.020-0.040 is interpretable: in the active regime, both Var_within (transitions WITHIN the wide thermal-equilibrium basin) and Var_between (basin-to-basin variation across seeds) grow proportionally, keeping the ratio approximately constant. This is the signature of a **single dominant basin with thermal noise** rather than multiple deep basins.

---

## 3 · Phase boundary `T*(A)` empirically mapped (Phase 4 + 5)

The activation threshold `T_active*(A)` was measured at six amplitudes A ∈ {10, 20, 30, 50, 80, 118} (Phases 1-5, 2026-04-29 RTX 5090). Result: **non-monotone U-shape with a unique sweet spot at A=50.**

### 3.1 Per-amplitude T-sweep — full data table

| A | N (mean) | T=0.005 | T=0.010 | T=0.020 | T=0.040 |
|---:|---:|---:|---:|---:|---:|
| 10 | 26 | 0% temporal | — | — | — |
| 20 | 93-99 | 0% | 0% | 2.3% | 2.6% |
| 30 | 236-254 | 0% | 0% | 0.1% | 11.8% |
| **50** | **553-615** | **1.4%** | **0%** (homog) | **22.2%** | **22.3%** |
| 80 | 1354-1475 | 0% | 0% | 0% | **0%** |
| 118 (tau) | 2858-2985 | 0% | — | 0% | — |

(Bold cells: regime-4 active. % is fraction of total variance attributable to within-seed temporal fluctuations.)

### 3.2 Striking finding: A=50 is the unique activation sweet spot

The data reveals a **non-monotone U-shape** in regime-4 activation versus amplitude:

- **At small A (≤ 30):** activation requires T ≥ 0.04, and even there the temporal contribution is small (2-12%). Cluster is geometrically pinned by the discrete lattice; binding is dominated by local cohesion within a 27-block.
- **At A=50:** sharp activation at T = 0.020 with 22% temporal contribution and linear-in-T scaling at higher T. The cluster size ~553 voxels spans ~20 27-blocks — boundary is geometrically free but not yet bulk-stabilized.
- **At large A (≥ 80):** **NO activation observed up to T=0.040.** Cluster sizes 1354 (A=80) and 2858 (A=118) are completely temporally frozen at all tested temperatures. Each seed locks into a specific configuration and stays there.

This **breaks** the simple `T_active ~ 1/A` scaling that initial A=20/30/50 data suggested. The actual behavior is U-shaped (or monotonically rising for small A then catastrophically rising past A=50), with the most-thermally-susceptible cluster size near A ≈ 50 (N ≈ 550 voxels).

### 3.3 Free-energy landscape interpretation of the U-shape

The U-shape is structurally interpretable in the multi-basin landscape:

**Small A (single deep basin):** the cluster fits within a small geometric volume where local flux gradients pin the configuration tightly. F(N) has a single sharp minimum; barrier to neighboring minima is large compared to thermal energy.

**Intermediate A ≈ 50 (multiple shallow basins):** the cluster has extended beyond local geometric constraints into a free-boundary regime. F(N) develops multiple shallow minima at slightly different sizes (550-566 voxels), with barriers between them comparable to T = 0.02. Easy thermal hopping → maximum regime-4 activity.

**Large A (extended bulk-stabilized basin):** the cluster's bulk volume is large enough that boundary fluctuations are damped by the cluster's interior cohesion. F(N) has a broad single minimum (the basin width is wide, but barriers to escape are very high because the bulk acts as a "buffer" against boundary events). Thermal noise spreads cluster size in initial conditions but doesn't drive ongoing transitions.

Mathematically, this is consistent with `F(N | A) ≈ -⟨ε⟩ · N + γ_surf(A) · N^{2/3}` where surface tension `γ_surf` becomes large (more rigid boundary) at large N — possibly through bulk-cluster's rigid flux network.

### 3.4 Phase-5 finding: large clusters are temporally frozen, not boundary-thickened

The Phase 5 test (A=117.93 at L=80) was designed to probe the regime-3 "boundary thickening" hypothesis at very large N. **Result: cluster fully frozen at both T=0.005 and T=0.020 with σ_within = 0.**

This **revises** our earlier interpretation of the snapshot-only T7 result (LEDGER FTD-0110 EXTENDED, 2026-04-27 evening: tau cluster sizes {2834, 2878, 2877, 2891, 2826}, std=26.1). The std=26.1 we attributed to "boundary thickening δ ~ N^{0.15}" is in fact pure cross-seed initial-condition spread (Var_between), not temporal boundary fluctuation. Per-tick observation shows the cluster is locked.

**Therefore the originally proposed "regime 3 boundary thickening" sub-hypothesis is REVISED**: at large N, the cross-seed std is initial-condition divergence (regime 1-3 spatial), not regime-4 temporal thickening. The σ ~ N^{0.5} scaling we saw at T7 is a Poisson-like spread of initial nucleation states, not an ongoing dynamics signature.

### 3.5 What this empirically establishes

```
Cluster phenomenology — full empirical map (Phases 1-5, 2026-04-29):

 Amplitude A    Cluster N      Regime-4 activation        Notes
 ==========    ==========     ====================      =====================
 10            ~26            none observed              fits one 27-block
 20-30         93-235         T ≥ 0.04 (low %)           geometric semi-pinned
 50            ~553           T = 0.020 (sharp, 22%)     free-boundary sweet spot
 80            ~1400          NONE up to T=0.040         bulk-stabilized
 118 (tau)     ~2900          NONE up to T=0.020         bulk-stabilized
```

The free-energy landscape F(N | A) has three qualitatively different forms:

1. **Single sharp minimum** for small A (regime 1 — geometric)
2. **Multi-basin shallow ladder** at A ≈ 50 (regime 2 — boundary-fluctuation accessible)
3. **Single broad bulk-buffered minimum** for large A (regime 3 — surface tension dominates over thermal noise)

The activation threshold T_active is NOT monotone in A. The "sweet spot" A ≈ 50 maximizes thermal susceptibility; both smaller and larger clusters are more thermally rigid.

This is a substantive new finding about the engine's cluster phenomenology that the earlier snapshot-only analysis missed entirely.

---

## 4 · LEDGER tag movement

**FTD-0110 (post-2026-04-29 Phase 6 free-energy framework):**

- **Spatial regimes 1-3:** [DERIVED + EMPIRICALLY CONFIRMED] (per cadd2ef commit)
- **Regime 4 temporal/frequency:** [DERIVED + EMPIRICALLY MEASURED across (A, T)] (per cadd2ef + this followup)
- **Free-energy landscape framework:** [DERIVED · structural] (this document) — the four-regime structure is the empirical signature of a multi-basin `F(N)` with hierarchy of barrier heights `T_meta < T_homog < T_active`.
- **Quantitative `T*(A)` scaling:** [PARTIAL · pending Phase 4 analysis]
- **Cluster-mass identification at SM particles:** [STRONGLY MOTIVATED CONJECTURE] (unchanged)

The free-energy landscape framing **structurally explains** the four-regime variance structure that was previously presented as an empirical observation. Each regime corresponds to a specific configuration-space sampling pattern:

- Regime 1 (lattice-pinned, low A): single sharp minimum
- Regime 2 (free-boundary, intermediate A): multiple shallow metastable minima (rare-event tails visible at large ensemble)
- Regime 3 (boundary-thickening, large N): broader minimum with thermal-fluctuation-induced surface roughening (untested at L=80; pending Phase 5)
- Regime 4 (temporal/frequency, high T): activated ongoing transitions across landscape barriers

---

## 5 · Cross-references

- Phase 1-3 empirical results: [`DERIV_FTD0110_VARIANCE_ENTROPY.md`](DERIV_FTD0110_VARIANCE_ENTROPY.md) §3.5
- Bridge-I derivation: [`DERIV_FTD0110_NONLINEAR_BRIDGE.md`](DERIV_FTD0110_NONLINEAR_BRIDGE.md)
- Linear-level k=1/4: [`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md)
- Engine binary: `engine/tests/campaign_amplitude_time_series.cpp`
- Phase 4 launcher: `scripts/exploration/run_phase45_followups.sh`
- Analysis script: `scripts/exploration/analyze_regime4_full_2026-04-29.py`

---

## 6 · Single-line summary

**The four-regime variance structure of FTD's cluster phenomenology is the empirical signature of a multi-basin free-energy landscape `F(N | A)` with hierarchical barrier heights: at low Langevin temperature seeds populate shallow metastable basins (regime-4 partial); at intermediate T (homogenization regime, T_homog ≈ 0.008 at A=50) thermal noise drives all seeds into the deepest basin; at high T (T > T_active ≈ 0.015 at A=50) thermal noise crosses inter-basin barriers and ongoing genesis/evaporation events at the boundary contribute regime-4 variance scaling linearly in T. The non-monotone T-dependence (homogenization minimum + activation threshold) is the quantitative fingerprint of the multi-basin landscape, providing empirical handles on `T_meta < T_homog < T_active` and (from barrier-height estimates) the typical inter-basin barrier `ΔF* ≈ 0.035` engine-energy-units at A=50. Phase 4 T-sweeps at A ∈ {20, 30, 80} (in flight) will test whether `T_active*(A)` scales as `1/A`, `1/A²`, or constant.**
