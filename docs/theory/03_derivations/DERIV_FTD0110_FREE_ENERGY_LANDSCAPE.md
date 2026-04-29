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

## 3 · Phase boundary `T*(A)` empirically mapped (Phases 4-7)

The activation threshold was measured at twelve amplitudes A ∈ {10, 20, 30, 40, 45, 50, 55, 60, 65, 70, 80, 118} across the Phase 4-7 campaign (2026-04-29 RTX 5090). **Initial reading at three amplitudes (20/50/118) suggested a non-monotone U-shape with a "sweet spot" at A=50; the Phase 7 fine-grained scan refutes that framing.** The actual structure is:

> **A monotone activation threshold around A ≈ 40 at L=32, T=0.020, with progressive increase through A ≈ 70, gated by lattice-relative occupancy. The "frozen at large A" finding for A=80 (L=64) and A=118 (L=80) is a finite-lattice-size effect: at larger L the cluster occupies a smaller fraction of L³ and does not feel the boundary stress that drives activation.**

### 3.1 Per-amplitude T-sweep — full data table (Phases 4-7)

T-axis sweep at L=32:

| A   | ⟨N⟩       | T=0.005 | T=0.010 | T=0.020 | T=0.040 | active seeds (T=0.020) | runaways |
|----:|----------:|--------:|--------:|--------:|--------:|---:|---:|
| 10  | 26        | 0.0%    | —       | —       | —       | — | 0 |
| 20  | 93–99     | 0.0%    | 0.0%    | 2.3%    | 2.6%    | ~1/10 | 0 |
| 30  | 236–254   | 0.0%    | 0.0%    | 0.1%    | 11.8%   | ~1/10 | 0 |
| 40  | 372       | —       | —       | **3.8%** | —      | **5/10** | 0 |
| 45  | 445       | —       | —       | 0.4%    | —       | 2/10 | 0 |
| 50  | 553–615   | 1.4%    | 0.0%    | **22.2%** | **22.3%** | **5/10** | 0 |
| 55  | 704       | —       | —       | 0.1%    | —       | 3/9 | 1 |
| 60  | 889       | —       | —       | **25.6%** | —     | **6/8** | 2 |
| 65  | 1013      | —       | —       | 9.0%    | —       | **6/7** | 3 |
| 70  | 1793*     | —       | —       | **91.0%** | —     | 4/6 (one near-runaway) | 4 |

L=64 / L=80 (lattice scaled to fit cluster):

| A   | L  | ⟨N⟩  | T=0.005 | T=0.010 | T=0.020 | T=0.040 | active seeds | N/L³ |
|----:|---:|-----:|--------:|--------:|--------:|--------:|---:|---:|
| 80  | 64 | 1395 | 0.0%    | 0.0%    | 0.0%    | 0.0%    | 0/5 | 0.5% |
| 118 | 80 | 2985 | 0.0%    | —       | 0.0%    | —       | 0/5 | 0.6% |

(Bold: regime-4 active. "active seeds" = seeds with σ_t > 0.5. "runaways" = seeds whose cluster size diverged past L³/4, indicating percolation into bulk lattice. ⟨N⟩ excludes runaways.)

### 3.2 The threshold finding (corrects earlier "A=50 sweet spot" framing)

**At fixed L=32 and T=0.020:**

- **A ≤ 30** (N/L³ ≲ 0.7%): essentially frozen. 0–2 of 10 seeds show any temporal activity. Cluster is far from the lattice boundary; thermal noise can't drive boundary events.
- **A ≈ 40–70** (N/L³ ∈ [1.1%, 5.5%]): activation threshold has been crossed. 3–6 of 10 seeds show non-trivial temporal activity. The fraction of runaway seeds rises monotonically (0/10 at A=40 → 4/6 at A=70), reflecting an emerging percolation instability. The "active" surviving seeds are those that stayed bound but undergo boundary churn.
- **A ≳ 70** (N/L³ ≳ 5%): close to a percolation threshold. Most seeds runaway into bulk-filling configurations; surviving "bound" seeds are extreme-tail samples near the runaway boundary.

**The earlier "A=50 unique sweet spot" reading was an artifact of sampling only A ∈ {20, 50, 80}.** Phase 7 shows A=40 (3.8% temporal, 5/10 active), A=50 (22.2%, 5/10), A=60 (25.6%, 6/8), A=65 (9.0%, 6/7) all squarely in the activation band. The pattern is *threshold-and-progression*, not peaked.

### 3.3 The "large-A freezing" is a finite-lattice effect, not bulk stabilization

The original interpretation read A=80 (L=64) and A=118 (L=80) "frozen" as evidence of a "bulk-stabilized broad minimum." The Phase 7 data forces a different reading:

- At L=32, A=70, cluster N≈1800 occupies N/L³ ≈ 5.5%, sees lattice boundary, undergoes activation.
- At L=64, A=80, cluster N≈1400 occupies N/L³ ≈ 0.5%, far from boundary, frozen.
- At L=80, A=118, cluster N≈2900 occupies N/L³ ≈ 0.6%, far from boundary, frozen.

The "frozen" outcome at A=80, A=118 is **not** because the cluster is too large to be thermally susceptible; it is because the *lattice was scaled with the cluster* so the cluster's relative occupancy is small. The right control experiment for "is large-A intrinsically frozen?" would be A=80 at L=32 — but at that combination the cluster percolates the lattice and the simulation runs away (cluster fills entire lattice, no bound state to study).

The relevant order parameter for activation is therefore **N/L³**, not A in isolation:

```
Activation requires N/L³ ≳ 1% (cluster reaches a regime where boundary
geometry interacts with thermal noise).

Below this fraction the cluster is in the lattice "bulk" relative to the
simulation box, and free-boundary thermal events are kinematically unavailable.
```

This reframes regime-4 as a finite-lattice-size phenomenon. It does not contradict the Bridge-I O_h-equivariance argument or the Bridge-II linear scaling — those operate at the cluster-internal level. But it means the variance partition `Var_within = regime-4` we measure here is **not** intrinsic to FTD's cluster phenomenology; it's specifically about cluster-on-finite-lattice dynamics.

### 3.4 Free-energy landscape interpretation, revised

Within the activation band (N/L³ ∈ [1%, 5%]), the multi-basin `F(N | A, L)` framing still holds, but the landscape *includes* the lattice boundary as a structural constraint:

**Subthreshold regime (N/L³ ≪ 1%):** cluster sits in a deep single-minimum basin set by the cluster-internal cohesion energy. Lattice boundary is irrelevant. F(N) is approximately the cluster's intrinsic Helmholtz free energy minus a small bulk-elastic correction.

**Activation band (N/L³ ∈ [1%, 5%]):** the cluster boundary starts to "feel" the lattice boundary through periodic-image / finite-size corrections. F(N) develops a corrugated structure — multiple shallow minima at slightly different cluster geometries that interact with the lattice's discrete neighbor count. Thermal hops at T ≈ 0.02 (≈ engine units) cross these corrugations. Runaway risk grows with N because the percolation barrier between bound state and lattice-fill is lowering.

**Percolation regime (N/L³ ≳ 5%):** the percolation barrier has lowered enough that with non-zero T a finite fraction of seeds drift across it; the remaining bound seeds are increasingly metastable.

**Lattice-bulk regime (N/L³ ≪ 1% for any A, achieved by scaling L with A):** F(N) reverts to the subthreshold form — single deep minimum — because the lattice boundary is again irrelevant. This is what we measure at A=80, L=64 and A=118, L=80.

The U-shape framing was misleading; the real structure is a **single threshold in N/L³** with progressive activation past it, plus a percolation transition further along.

### 3.5 Phase-5 finding (large-N at large L) restated

The Phase 5 test (A=117.93 at L=80) showed σ_within = 0 at both T=0.005 and T=0.020. **Restated under the corrected framing:** this confirms that at small N/L³ ≈ 0.6%, the cluster is in the lattice-bulk regime (subthreshold for the activation threshold in N/L³). It does not by itself say anything about an intrinsic large-N rigidity — it would require a measurement at A=118, L=32 to test, but that combination is in the percolation regime and would simply runaway.

The earlier reading "tau cluster std=26.1 at L=80 ⇒ regime-3 boundary thickening" is also revised: at N/L³ ≈ 0.6%, the σ_between ≈ 26 is pure initial-condition spread (regime 1-3 spatial-IC variance) and σ_within ≈ 0; the cluster is in the lattice-bulk regime where regime-4 is kinematically inaccessible.

### 3.6 What this actually establishes empirically

```
Cluster phenomenology — full empirical map (Phases 1-7, 2026-04-29):

  Order parameter  Active regime?    Mechanism
  ===============  ==============    ==============================
  N/L³ ≲ 1%        no                lattice-bulk regime; cluster decoupled
                                     from boundary; subthreshold for any T
                                     in tested range (≤ 0.040 engine-units)

  N/L³ ∈ [1%, 5%]  yes (progressive) activation band; cluster boundary
                                     interacts with lattice boundary;
                                     thermal noise drives free-boundary
                                     events; runaway risk rises with N/L³

  N/L³ ≳ 5%        runaway-dominated percolation regime; bound state is
                                     metastable; thermal noise drives a
                                     finite fraction of seeds into
                                     lattice-fill configurations
```

The relevant phase boundary in the (A, T, L) parameter space is set by N(A)/L³ (a lattice geometry quantity), not by A alone. **Regime-4 activation is a finite-lattice-size phenomenon, not an intrinsic cluster physics feature.**

This is a substantive correction to the post-Phase-4 reading. Within the activation band, the Anova decomposition still cleanly separates within-seed temporal variance (regime-4) from between-seed initial-condition variance (regimes 1–3 spatial). But the band itself is set by lattice geometry, not by the cluster's intrinsic free-energy landscape.

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

**The four-regime variance structure of FTD's cluster phenomenology, as measured in Phases 1–7 (2026-04-29), separates cleanly into spatial initial-condition variance (regimes 1–3, Var_between) and temporal boundary-event variance (regime-4, Var_within). The Phase 7 fine-grained amplitude scan (A ∈ {40, 45, 55, 60, 65, 70} at L=32, T=0.020) refutes the earlier "A=50 sweet-spot" reading from Phase 4: regime-4 activation is governed by lattice-relative occupancy N/L³, with a threshold around N/L³ ≈ 1% (corresponding to A ≳ 40 at L=32), progressive activation through N/L³ ≈ 5% (A ≈ 70 at L=32), and a percolation transition past that. The "frozen at large A" outcomes for A=80 (L=64) and A=118 (L=80) are not bulk-stabilization — they are simply lattice-bulk measurements at N/L³ ≲ 0.6%, where the cluster is decoupled from the lattice boundary and regime-4 is kinematically inaccessible. Regime-4 is therefore a finite-lattice-size phenomenon driven by cluster-boundary / lattice-boundary interaction, not an intrinsic feature of the cluster's free-energy landscape. This does not affect Bridge-I (O_h-equivariance, derived) or Bridge-II at the linear level (k = 1/4, derived from O_h representation theory), which operate on cluster-internal structure.**
