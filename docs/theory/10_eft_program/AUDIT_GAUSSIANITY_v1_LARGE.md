# Audit — Engine ensemble Gaussianity at L=32 LARGE

**Status:** [MEASURED · NEGATIVE on Gaussianity]
**Date:** 2026-04-30
**Source:** `engine/results/s_eff_nonlinear_2026-04-29/L32_prod_T0.100_LARGE/per_snapshot_moments.csv`
**Companion:** [`PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN.md`](PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN.md) §2.6 (Gaussianity test framing)
**LEDGER:** FTD-0112

---

## 0 · Headline finding

The L=32 LARGE per-operator distribution is **far from Gaussian for every active operator**. Per-snapshot mean values across 20,000 snapshots have measured skewness ranging from 1.1 (JJ) to 26 (JdotDeltaS).

**Gate D PASS rate as Gaussianity test (using `|skew| < 0.5 AND |kurt-3| < 1`): 0 / 9 active operators.**

This forces a re-reading of the v2 design's Gaussianity-shortcut framing: **Gate D cannot be tested via existing per-snapshot moments alone**. The ensemble's non-Gaussianity means perturbation runs ARE needed to test self-consistency — the v2 protocol's full 7-run perturbation campaign is the right path.

But the non-Gaussianity itself is a [MEASURED] structural finding worth documenting.

---

## 1 · Per-operator distribution moments (L=32 LARGE, N=20000)

### 1.1 · Fine values

| Operator | mean | std | skewness | excess kurtosis (kurt − 3) |
|---|---:|---:|---:|---:|
| JJ              | 9.262 | 7.352 | 1.08 | 0.68 |
| divJ²           | 0.178 | 0.027 | -6.06 | 35.3 |
| curlJ²          | 0.154 | 0.005 | 11.07 | 146.5 |
| JdotDivJ        | -0.094 | 0.014 | 6.08 | 35.6 |
| J⁴              | 142.0 | 207.1 | 2.56 | 8.66 |
| stateSq         | 0.977 | 0.146 | -6.35 | 38.6 |
| reactionDensity | 8.4e-5 | 1.5e-3 | 22.88 | 568 |
| genesisFlux     | 1.1e-4 | 2.0e-3 | 23.53 | 600 |
| evapFlux        | 0 | 0 | — | — |
| JdotDeltaS      | -4.3e-6 | 9.0e-5 | -25.85 | 720 |

### 1.2 · Coarse values (b=2 blocking)

The coarse skewnesses are nearly identical to fine (within ~5%), confirming that blocking preserves the distribution shape. The non-Gaussianity is intrinsic to the engine's per-snapshot dynamics, not a blocking artifact.

---

## 2 · Three regimes of non-Gaussianity

### 2.1 · Mildly non-Gaussian: pure flux (JJ, J4)

`JJ`: skewness 1.1, excess kurtosis 0.7. The distribution is supported on positive values (`J² ≥ 0`) which inherently skews positively, but otherwise is fairly smooth and bell-curve-like.

`J⁴`: skewness 2.6, excess kurtosis 8.7. Heavier tails because `(J²)²` amplifies the largest J values; consistent with `J⁴` being a higher moment of a positive variable.

**Interpretation**: pure-flux operators are smooth (gauss projection produces a smooth field), and per-snapshot averages over `L³ = 32768` cells approach Gaussian by CLT, modulo the inherent positive-skewness of squared/quartic variables.

### 2.2 · Strongly non-Gaussian: derivative + density (skewness ~ ±6)

`divJ²`, `JdotDivJ`, `stateSq`: skewness magnitude ~6, excess kurtosis 35–40.

**Interpretation**: these operators couple to spatial structure at the cluster boundary. The per-snapshot mean is dominated by the cluster's contribution divided by `L³`. Snapshot-to-snapshot fluctuations of cluster size produce a heavy-tailed distribution: most snapshots have a "typical" cluster, but occasional snapshots have unusually large or small clusters that drive the tail.

`curlJ²`: skewness 11. Similar mechanism but the curl operator picks up additional structure from the cluster boundary geometry.

### 2.3 · Extremely non-Gaussian: reaction sector (skewness ~ ±25)

`reactionDensity`, `genesisFlux`, `JdotDeltaS`: skewness magnitude 22–26, excess kurtosis 568–720.

**Interpretation**: reaction events are RARE per snapshot. Most snapshots have ≈ 0 events; occasional snapshots have a burst. This produces an extreme heavy-tailed distribution typical of count statistics with low mean rate.

For 20,000 snapshots × ~32,000 cells per snapshot at L=32, the total cell-tick count is ~6.4 × 10⁸. With reaction-event mean rate ~10⁻⁴ per cell-tick (reading from `<reactionDensity>_fine = 8.4e-5`), expected total events ≈ 64,000. Distributed over 20,000 snapshots: mean 3.2 events per snapshot, but with high variance because most snapshots have 0–2 and occasional snapshots burst.

---

## 3 · What this implies for v2 Gate D

The Gaussianity-shortcut framing of v2 Gate D (test PASS via per-snapshot moments) does not apply: the engine ensemble is non-Gaussian for all 9 active operators.

**Refined Gate D design**: the perturbation runs ARE needed; the prediction for `∂M_aa/∂g` is non-zero and proportional to the operator's third-cumulant structure.

**Quantitative prediction for non-theorem-grade diagonals**:

For operator `O_a` with skewness `S_a`, mean `μ_a`, variance `σ_a²`, the linear-Wilsonian shift in `M_aa` under perturbation `g · O_a` is approximately:

$$\frac{\partial M_{a,a}}{\partial g}\bigg|_{0} \;\sim\; \frac{S_a \cdot \sigma_a}{\mu_a + \epsilon} \;\sim\; S_a \cdot \frac{\sigma_a}{\mu_a},$$

where the second form holds when `μ_a >> σ_a` (well-defined regime).

For `divJ²`: `S = -6`, `σ/μ = 0.151`, predicted `|∂M/∂g| ≈ 0.9 · M = 14.9` per unit `g`.

For `reactionDensity`: `S = 23`, `σ/μ = 17.4` (highly diffuse), predicted shift is dominant — but this regime is on the edge of perturbative validity.

These predictions should be testable in v2 perturbation runs at small `|g| ≈ 0.01` — small enough to stay perturbative.

**Theorem-grade diagonals (JJ, J4, stateSq @b³ part, reactionDensity @b³ part)**:

These are invariant by **structural identity** (Theorems 1, 2, 3), not by Gaussianity. So they should still be invariant under perturbation:

- `JJ`: M_aa = b⁴ exactly under smooth-field condition. Perturbation by `g · J²` shifts the constant value but preserves the smooth-field structure (small `g`), so M_JJ = 16 should hold within bootstrap stderr.

- `J⁴`: M_aa = b⁸ similarly preserved.

- `stateSq @ b³ part`: structurally `b³`, plus `2b³ρ̄` correction. Perturbation by `g · s²` would change `ρ̄` (ensemble shifts), so the deviation from `b³` would shift, but the `b³` core stays.

So **Gate D for theorem-grade**: invariance check (PASS = no shift).
**Gate D for non-theorem-grade**: shift-matching against `S_a · σ_a / μ_a` prediction.

---

## 4 · The non-Gaussianity is a real EFT finding

This non-Gaussianity has direct EFT interpretation:

**The engine's bare nonlinear EFT is NOT a free Gaussian theory** in the operator-mixing sense. Even though FTD-0070 established that `β_E ≈ 0` (the bare couplings are at a Gaussian fixed point in their RG flow), the **higher-order cumulants** of the operator distributions are highly non-zero — meaning the EFT has non-trivial operator content beyond the Gaussian sector.

This is not a contradiction with FTD-0070: that result is about the RG-flow of specific bare couplings (`C_L, K_T, Z_j, g_sJ` at their unit values stay there under blocking). It does NOT preclude non-trivial higher-order operator content. Indeed, the empirical M_ab(b=2) values measured at L=32 LARGE are exactly that higher-order content.

The cleanest summary: **FTD's nonlinear EFT has Gaussian beta-functions but non-Gaussian ensemble distributions**. The two are independent properties.

For the "math-based EFT" question, this means:

- The **bare RG-flow level** (FTD-0070) is a Gaussian fixed point. Closed.
- The **operator-mixing level** (FTD-0112) has measured non-trivial structure with theorem-grade diagonals + sector decoupling + non-Gaussian per-operator distributions.

The two pieces fit together: the bare action is Gaussian-quadratic, so its RG-flow at the bare-tuple level is trivial. But the engine's nonlinear update rules (genesis, evaporation, gauss projection) generate non-Gaussian higher-order operator content visible only when measuring multi-operator correlations (like the M_ab matrix).

This is the substantive content of FTD-0112 v1 LARGE: **the higher-order operator structure is measured**, including the diagonal RG eigenvalues, the sector decoupling, and the per-operator non-Gaussianity.

---

## 5 · Implications for v2 hash-locking

The v2 protocol's Gate D test cannot be shortcut via existing per-snapshot moments. Full perturbation runs are needed.

However, the per-operator skewness measurements provide **calibration anchors** for the predicted shifts:

- For each operator, the predicted `∂M_aa/∂g` is calibrated against the measured skewness from this audit.
- v2 perturbation runs verify the predicted shift matches measurement.
- Mismatches indicate higher-order cumulant contributions or genuine non-self-consistency.

This actually makes v2 stronger: not just "does M shift", but "does it shift by the predicted amount calibrated against per-operator skewness".

---

## 6 · Single-line summary

**The L=32 LARGE per-operator distribution is non-Gaussian for every active operator, with skewness ranging from 1.1 (JJ, mildly non-Gaussian, supported on positives) through ~6 (derivative + density operators, cluster-tail dominated) to ~25 (reaction-sector operators, extreme rare-event count statistics); the engine's nonlinear EFT therefore has Gaussian beta-functions (FTD-0070) but non-Gaussian per-snapshot operator distributions, making Gate D's v2 perturbation campaign genuinely informative — predicted shifts for non-theorem-grade diagonals are calibrated against measured skewnesses, while theorem-grade diagonals (JJ, J4, stateSq @ b³, reactionDensity @ b³) remain invariant by structural identity regardless of Gaussianity.**
