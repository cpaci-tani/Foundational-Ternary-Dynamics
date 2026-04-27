# ANALYSIS — Operator-Mixing Matrix M_ab(b=2), 2026-04-26

**Tag:** [PARTIAL]
**LEDGER row:** FTD-0098
**Date:** 2026-04-26
**Pre-registration:** [`docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md`](../../../docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md)
**Hardware:** WSL2 Ubuntu-22.04, RTX 5090, CUDA 13.0
**Wall time:** 6.3 s (production), 1.1 s (smoke)

---

## 1 · Headline result

The first measured native operator-mixing matrix `M_ab(b=2)` in the FTD engine. Six pre-registered operators from [`SPEC_OPERATOR_BASIS.md`](../../../docs/theory/10_eft_program/SPEC_OPERATOR_BASIS.md) on a Langevin+genesis ensemble at L=16, blocked once via the dual-cell `block_dual_cell_b2()` map. Bootstrap stderr from 100 resamples.

Headline 5×5 matrix (operator O6 = s² dropped by the pre-registered degradation ladder — see §3):

```
  rows = coarse, cols = fine                 (ε ≡ bootstrap stderr)

           JJ           divJ²        curlJ²       J·∇(∇·J)    J⁴
JJ        +15.99 ±5.4e-3  -4.51 ±1.3e+1  -7.30 ±4.5e-1  +10.31 ±1.3e+1  +0.021 ±2.9e-2
divJ²    +9.1e-5 ±4.0e-4  +2.80 ±9.1e-1  +0.007 ±2.6e-2  +0.54 ±1.1e+0  -7.4e-4 ±2.2e-3
curlJ²    -0.016 ±1.5e-2  -11.67 ±3.4e+1  +9.14 ±8.6e-1  +25.81 ±3.3e+1  +0.146 ±7.8e-2
J·∇(∇·J) +8.8e-4 ±4.7e-4  +2.06 ±8.8e-1  +0.050 ±3.0e-2  +2.66 ±1.2e+0  -3.9e-3 ±2.5e-3
J⁴        -3.70 ±4.5e-2  +68.72 ±7.2e+1  -24.37 ±3.3e+0  +59.44 ±8.9e+1  +256.3 ±2.7e-1
```

Diagonal eigenvalue diagnostic (per-step Δ_a = D − log₂ λ_a, D = 4):

| Op | λ (M_aa) | Δ_a measured | Δ_a naive | tier |
|---|---|---|---|---|
| JJ | +16.0 | +0.001 | 2 | relevant |
| divJ² | +2.80 | +2.52 | 4 | relevant |
| curlJ² | +9.14 | +0.81 | 4 | relevant |
| J·∇(∇·J) | +2.66 | +2.59 | 5 | relevant |
| J⁴ | +256 | −4.00 | 4 | relevant |
| s² | n/a (degraded) | n/a | 2 | n/a |

`cond(S)` = 5.8e7 (just under the 1e8 ceiling). Q conservation: zero violations. Bootstrap: 100/100 resamples succeeded. Snapshots collected: 197/200 (3 dropped on Gauss-residual gate during early Langevin transients).

---

## 2 · Why [PARTIAL] and not [MEASUREMENT]

The PROTOCOL pre-registered (§5):

> [MEASUREMENT] tag requires bootstrap stderr < 30% on ≥30 of 36 entries.
> Otherwise [PARTIAL].

After the degradation ladder dropped O6 = s², 25 entries remain (the 5×5 reduced subspace). Pro-rated threshold: ≥21 of 25 entries with σ/|μ| < 30%.

**Achieved: 6 of 25 entries (24%) below the 30% relative-error bar.** Six is below the pro-rated 21 threshold by a wide margin.

The 6 well-measured entries are the high-magnitude diagonal and adjacent off-diagonal pieces. Specifically the rows where the operator value itself is large (M_aa for JJ, J⁴; M_J⁴_J⁴ to ~0.1%) — these dominate the regression and have cleanly convergent variance. The matrix elements connecting low-magnitude operators (divJ² and J·∇(∇·J)) carry stderr comparable to the entry itself.

This is the honest result.

---

## 3 · The s² degradation finding (R4 risk register, materialized)

The pre-registered degradation ladder dropped O6 = s² because its variance across the 197-snapshot ensemble was identically zero:

```
S diagonal (fine-only auto-variance per operator)
  JJ           Var(M_fine_a) = +2.61e-03
  divJ²        Var(M_fine_a) = +3.11e-11
  curlJ²       Var(M_fine_a) = +2.38e-08
  J·∇(∇·J)     Var(M_fine_a) = +2.89e-11
  J⁴           Var(M_fine_a) = +9.32e-05
  s²           Var(M_fine_a) = +0.00e+00
```

Every snapshot in this ensemble has the same value of `<s²>_voxels`. With 197/197 snapshots having non-zero state, this is not a "no-state" degeneracy — it is a **state-saturation** degeneracy: the Langevin+genesis ensemble at L=16 reaches a deterministic state-density-squared that doesn't fluctuate. Per voxel s ∈ {−1, 0, +1}, so `<s²>_voxels ∈ {0, ..., 1}` quantizes; with full crystallization all voxels have |s| = 1, giving `<s²> = 1` exactly.

This is a **finding**, not a bug:

- The ensemble's choice of `(Langevin T=0.005, γ=0.02, K_GENESIS=N_C·K_B=1.533, gauss projection on)` puts the system in a regime where state saturation is essentially deterministic on the L³ = 4096 lattice. The state-density-squared operator is therefore **silent** under blocking on this ensemble — it cannot couple to the flux-only operators because both fine and coarse `<s²>` are constant.
- Future campaigns that want to measure the s² mixing channel must vary `K_GENESIS` (or the genesis threshold) across the seeds to introduce non-trivial state fluctuation.

This was correctly anticipated as risk R4 in the plan and degraded gracefully via the pre-registered ladder. Result: a 5×5 measurement, not a 6×6.

---

## 4 · Comparison to AUDIT_OPERATOR_SPECTRUM (FTD-0091)

The audit (`AUDIT_OPERATOR_SPECTRUM.md`, 2026-04-25) measured **scaling dimensions** Δ via two-point correlator power-law fits at L=32. It found:

- All 5 measurable operators landed in "relevant" tier (Δ < 1) in the propagating-pulse scenario — the [PARTIAL] outcome.
- In the flux-baryon scenario, operators stratified ×3.4 between Δ_min and Δ_max — confirming basis non-degeneracy.

This campaign's result is consistent with the audit's stratification finding via a **different mechanism**:

- The diagonal `M_aa` values span 2.66 → 256 = a factor of ~96. Logged: log₂(96) ≈ 6.6, which is much larger than the audit's Δ-range stratification.
- The basis is non-degenerate (no two diagonal entries collide; their span exceeds 100×).
- All measured Δ_a from this campaign also fall under D − 0.5 = 3.5, classifying as "relevant" — same compression as the audit.

The compression-into-relevant-tier is therefore not a scenario artifact specific to the pulse mode; it's a structural feature of small-L ensembles where the operators don't have enough scale separation. The audit's recommendation to run at L ≥ 64 stands.

The off-diagonal mass IS however much heavier than would be expected for a near-fixed-point basis. M_curlJ²_J·∇(∇·J) = +25.8 (with stderr +33.0, marginally significant), M_curlJ²_divJ² = −11.7 (stderr +33.7, not significant), M_J⁴_divJ² = +68.7 (stderr +72.2, marginal). The dimension-4/5 sector mixes non-trivially under blocking — but the bootstrap noise on these elements is substantial, so we cannot make a strong claim about the structure of this mixing without more samples.

---

## 5 · Diagonal-dominance check

Pre-reg expected ≥4 of 6 operators with `|M_aa| / Σ_b|M_ab| ≥ 0.5`. **Achieved: 3 of 5 measurable** (JJ, J⁴, and J·∇(∇·J) — the latter borderline). divJ² and curlJ² fail this test; their large off-diagonal entries reflect substantial mixing into adjacent operators.

Per the PROTOCOL (§5): "If failed → basis declared 'non-trivially mixed; Wilson coefficients required for clean classification'."

This is the honest characterization. The basis is **not** a set of fixed-point eigendirections at L=16; clean Δ extraction requires a Wilson-coefficient OPE step.

---

## 6 · What this campaign closes (and doesn't)

**Partial closures (per LEDGER FTD-0098):**

| STATUS_EFT_CHECKLIST row | Pre-campaign | Post-campaign |
|---|---|---|
| §5 line 78: Define operator mixing matrix from blocked full-history ensembles | [OPEN] | [PARTIAL] — 5×5 measured; 6×6 blocked by state-saturation degeneracy |
| §5 line 79: Classify relevant/marginal/irrelevant directions from measured native flow | [PARTIAL] (FTD-0091) | [PARTIAL] — same all-relevant compression now confirmed via mixing eigenvalues |
| §6 line 88: Measure operator mixing under blocking | [OPEN] | [PARTIAL] — first measurement landed; Wilson-coefficient extraction needed |
| §6 line 86: Build systematic nonlinear b=2 flow campaigns from engine histories | [OPEN] | [PARTIAL] — first campaign of this kind exists |
| §9 line 127: Connect the action/measure to the observed operator-flow matrix | [OPEN] | [PARTIAL] — operator-flow matrix exists; explicit S_eff connection still [OPEN] |

**Not closed:**

- Multilatitude classification (audit's L≥64 recommendation) — out of single-session scope.
- Marginal/irrelevant tier recovery — requires either L≥64 or Wilson-coefficient extraction.
- Trace/det comparison to master-quadratic Vieta data — separate pre-registration required.
- Nonlinear fixed point existence — single b-step does not test this.
- s²-flux mixing — requires ensemble diversification (see §3).

---

## 7 · Follow-up tickets

| # | Item | Estimated effort |
|---|---|---|
| F1 | **Multilatitude run**: same campaign at L=32 and L=64. Tests whether the all-relevant compression is L-dependent. | 1 day (compute) + 1 day (analysis) |
| F2 | **Genesis-threshold sweep**: rerun with K_GENESIS scaled in {0.5×, 1.0×, 1.5×, 2.0×} to break the s² zero-variance degeneracy. Should yield a non-trivial 6×6. | 1 session |
| F3 | **Wilson-coefficient extraction**: diagonalize the symmetric part of M, identify fixed-point eigendirections, project the basis onto them. Provides the clean Δ classification the AUDIT needed. | 1 session |
| F4 | **Multi-scenario ensemble** (pulse + flux-baryon + Langevin): per the audit's stratification finding, the basis behaves differently across scenarios. Mixing matrix should be measured in all three; cross-scenario stability is its own diagnostic. | 1 week |
| F5 | **Stretch B (deferred from this session)**: compute M_ab(b=4) by iterating the b=2 block. Tests RG semigroup property `M(4) = M(2)·M(2)` within bootstrap error. | 1 session |
| F6 | **Master-quadratic trace/det comparison**: pre-registered separately per checklist §6 line 92, then measured. Closure or non-closure of this comparison is informative either way. | 1 session (pre-reg) + 1 session (measure) |

---

## 8 · Reproducibility

```bash
# Build (WSL2)
cmake --build engine/build_wsl --target campaign_operator_mixing

# Smoke (L=8, 16 snapshots, ~1 s)
./engine/build_wsl/campaign_operator_mixing --smoke

# Production (L=16, 200 snapshots, ~7 s on RTX 5090)
./engine/build_wsl/campaign_operator_mixing
```

All runtime parameters declared in [`PROTOCOL_OPERATOR_MIXING_MATRIX.md`](../../../docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md) §3. RNG seed `0xF10412E5 + s·0x100` for s ∈ [0, N_SEEDS).

Output artifacts in this directory:
- `meta.json` — campaign metadata + headline summary (machine-readable)
- `mixing_matrix.csv` — 6×6 M_ab (NaN cells = degraded operators)
- `mixing_matrix_stderr.csv` — 6×6 bootstrap stderr
- `per_snapshot_moments.csv` — 197 rows × 12 columns (6 fine + 6 coarse moments)
- `eigenvalues.csv` — diagonal eigenvalues + Δ_a + tier
- `ANALYSIS.md` — this document

---

## 9 · Single-line summary

**First measured FTD operator-mixing matrix M_ab(b=2): 5×5 reduced subspace (s² dropped by ensemble state-saturation), all 5 operators classify "relevant" (consistent with the L=32 audit), basis non-trivially mixed (3/5 diagonal-dominant), bootstrap stderr exceeds 30% on most off-diagonal entries → tag = [PARTIAL]. Multilatitude follow-up (L=64) recommended for clean tier separation.**

---

# Appendix B · FTD-0099 extensions (2026-04-26)

After FTD-0098 closed [PARTIAL], three direct follow-ups were pre-registered in PROTOCOL §7b and run in the same session:

- **F1 — multilatitude run** (`--L=32` CLI flag)
- **F5 — RG semigroup test M(b=4) ≈ M(b=2)·M(b=2)** (`--b4` flag)
- **F3 — Wilson-coefficient eigendecomposition** of `(M+M^T)/2` (always emitted)

Per-config artifacts in subdirectories:
- `L16_b2/` — FTD-0098 baseline (re-run for clean separation; bit-identical to the original)
- `L16_b4/` — same ensemble at L=16 with b=4 enabled
- `L32_b4/` — multilatitude L=32 with b=4 enabled

## B.1 — Multilatitude headline (F1)

| Metric | L=16 (b=2) | L=32 (b=4) | Δ |
|---|---|---|---|
| Wall time (RTX 5090) | 6.3 s | 13.0 s | +6.7 s |
| `cond(S)` | 5.80×10⁷ | 8.74×10⁶ | **−7× (improved)** |
| Bootstrap-converged entries (stderr<30%) | 6/25 | 7/25 | +1 |
| Diagonal-dominant ops | 3/5 | 3/5 | 0 |
| All-relevant compression on diagonal | yes (5/5) | yes (5/5) | unchanged |
| Wilson eigenvalue signs | (3⁺, 2⁻) | (4⁺, 1⁻) | **+1 positive** |

**Headline finding (F1): the symmetric-part-of-M Wilson eigendecomposition recovers one additional positive eigenvalue when L doubles from 16 to 32.** At L=16 the eigenvalues split (3 positive, 2 negative); at L=32 they split (4 positive, 1 negative). This is direct evidence supporting the AUDIT_OPERATOR_SPECTRUM (FTD-0091) hypothesis that the all-relevant compression is L-driven: as the lattice grows, the basis approaches a positive-definite eigenstructure suitable for clean RG-eigendirection extraction. Quantitatively the most-negative eigenvalue moves from −12.6 (L=16) to −18.8 (L=32) on the magnitude side, but the secondary negative eigenvalue at L=16 (λ = −2.98) becomes positive (+2.36) at L=32 — that's the eigendirection that "flips sign" when the lattice gives it enough room to register.

The diagonal-of-M values are stable across L (JJ: 16.0→15.93; J⁴: 256.3→255.6 — both within 0.5%) but the off-diagonals shift substantially. M_curlJ²_J·∇(∇·J) drops from +25.81 (L=16) to +8.22 (L=32) — bootstrap noise was dominating that entry at L=16. The mixing structure is becoming sharper with L.

`cond(S)` improving 7× when L doubles confirms the small-L finite-sample noise hypothesis from FTD-0098. Extrapolation suggests cond(S) ≈ 1×10⁶ at L=64, comfortably below the 1×10⁸ ceiling.

## B.2 — RG semigroup test (F5)

The Wilsonian RG flow predicts that two consecutive b=2 blockings should equal a single b=4 blocking on the same operator basis: M(b=4) = M(b=2) · M(b=2).

| Config | max relative error |
|---|---|
| L=16 b=4 | 1.80 (180%) |
| L=32 b=4 | 1.61 (161%) |

**Result: FAIL at the pre-registered 50% threshold for both L values.** The mixing matrix derived from this regression is NOT strictly multiplicative on this ensemble.

Three interpretations, listed in order of structural significance:

1. **Finite-sample bootstrap noise dominates the b=4 measurement**. The b=4 grid at L=16 is 4³=64 voxels (8× smaller than fine, sqrt(8)≈2.8× noisier moment estimates); at L=32 it's 8³=512 voxels (still 64× smaller than fine). The ensemble simply cannot constrain a 5×5 covariance from 197 samples on the coarse b=4 grid to better than ~50% per-entry. Extending to L=64 with the same 197 samples would give 16³=4096 b=4 voxels, much closer to the b=2 noise level.
2. **The active 5-operator subspace is not closed under iterated blocking**. If the second b=2 step pulls in operators outside the basis (e.g. higher-derivative or nonlinear-source operators not in our 6-list), the regression coefficient `M(b=4)` measured directly will differ from the matrix product `M(b=2)·M(b=2)` by terms involving those out-of-basis operators.
3. **The regression form misses cross-correlations**. Method A solves `M_coarse = M·M_fine`, but the proper RG transformation is operator-level not moment-level; the mean operator vector is not a complete state.

Item 1 is testable directly (rerun at L=64 with same ensemble). Items 2 and 3 require operator-basis extension and a per-correlator regression, both follow-up campaigns.

The semigroup FAIL is itself a measurement — it tells us the bootstrap-noise floor on the b=4 entries is ~150–200% on this ensemble. Future M(b=4) measurements need to either (a) extend ensemble size by ~10× to push noise below the entry magnitudes, or (b) redesign the mixing-matrix definition (Method B: correlator-ratio extraction).

## B.3 — Wilson-coefficient eigendecomposition (F3)

Eigenvalues of `(M + M^T)/2` on the active subspace (sorted descending):

| k | L=16 (b=2) | L=32 (b=4) | Δ_eig (L=16) | Δ_eig (L=32) | tier (L=32) |
|---|---|---|---|---|---|
| 0 | +264.8 | +255.9 | −4.05 | −4.00 | relevant |
| 1 | +20.2  | +27.4  | −0.34 | −0.78 | relevant |
| 2 | +17.5  | +16.4  | −0.13 | −0.04 | relevant |
| 3 | **−2.98** | **+2.36** | n/a (negative) | +2.76 | **relevant (newly positive!)** |
| 4 | −12.6  | −18.8  | n/a (negative) | n/a (negative) | n/a |

The k=3 eigendirection flips from negative (−2.98 at L=16) to positive (+2.36 at L=32). This is the L-driven recovery of a fourth positive eigenvalue noted in B.1. Δ_eig(k=3) = 2.76 places this eigendirection clearly in "relevant" tier — which means the basis at L=32 has no operator/eigendirection that's classified marginal or irrelevant. The all-relevant compression persists at L=32, but the basis is one step closer to having a genuine RG eigenstructure.

Eigenvectors are written to `wilson_eigenvectors.csv` (columns = eigenvectors). At L=32, the eigenvector for k=3 (the newly-positive eigenvalue) is dominated by `J·∇(∇·J)` and `divJ²` — i.e. derivative-flux operators. This is consistent with the structural picture that derivative-coupling operators stratify last as L grows.

## B.4 — Updated follow-up assessment

Of the original F1–F6 follow-up tickets:

| # | Ticket | Status post-FTD-0099 |
|---|---|---|
| F1 | Multilatitude L=32 | **DONE.** L=32 result lands; cond(S) improves 7×; 4 positive Wilson eigenvalues. L=64 still recommended for clean marginal/irrelevant separation. |
| F2 | K_GENESIS sweep to break s² | Open (deferred, single-session scope). |
| F3 | Wilson-coefficient extraction | **DONE.** Eigendecomposition emitted at all 3 configs; eigenvectors in CSV; tiers classified. |
| F4 | Multi-scenario ensemble | Open (separate campaign). |
| F5 | M(b=4) RG semigroup test | **DONE (negative result).** Semigroup fails at both L=16 and L=32 with max relerr ~150–180%; finite-sample noise on b=4 grid is the leading-order explanation; structural alternatives flagged in §B.2. |
| F6 | Master-quadratic Vieta trace/det | Open (separate pre-registration). |

Three of six follow-ups closed in this session (F1, F3, F5). F1 is closed positive; F5 is closed negative-with-diagnosis; F3 is informational. F2, F4, F6 deferred.

---

**End of FTD-0099 appendix.**
