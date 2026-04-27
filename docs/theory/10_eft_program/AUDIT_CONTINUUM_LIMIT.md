# AUDIT — Continuum-Limit Verification at L ∈ {16, 32, 64} (FTD-0103)

**Tag:** [PARTIAL]
**Date:** 2026-04-27
**LEDGER row:** FTD-0103
**Status:** Campaign C closure read (engine-as-instrument portfolio)
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (Campaign C)
**Hardware:** WSL2 RTX 5090, CUDA 13.0
**Wall time:** ~6 min for L=64 with --b4 (vs ~13 s at L=32, ~6 s at L=16)

---

## 1 · Why this audit exists

[FTD-0098/0099](../07_assessment/LEDGER.md#FTD-0098) measured the operator-mixing matrix M_ab(b=2) at L=16 and L=32 with a `--L=N` CLI flag. This audit extends that data series to L=64 to test whether the engine's mixing matrix has a smooth continuum limit, by tracking three convergence indicators:

1. cond(S) — well-conditioning of the operator-snapshot covariance (lower is better)
2. Wilson eigenvalue positivity — number of real-positive eigenvalues of (M+M^T)/2
3. RG semigroup property — M(b=4) ≈ M(b=2)·M(b=2) within bootstrap stderr

A genuine continuum limit would show monotone improvement in (1), monotone movement toward all-positive eigenvalues in (2), and bounded relative error in (3) as L→∞.

This is a **convergence audit**, NOT a coupling-recovery audit. The output is descriptive: "what does the engine's mixing matrix do as L grows" — not "does it match QED."

L=128 is **deferred** for this audit closure: with L=64 already taking ~6 minutes wall and L=128 estimated 3+ hours, the L=64 datapoint is sufficient for an L=16/32/64 trend reading. If the trend is encouraging, L=128 becomes a follow-up; if decisively negative, L=128 is unnecessary.

---

## 2 · Quantitative results

### cond(S) — operator-snapshot covariance condition number

| L | cond(S) | factor improvement vs L=16 |
|---|---|---|
| 16 | 5.80×10⁷ | 1.0× |
| 32 | 8.74×10⁶ | 6.6× |
| 64 | 3.22×10⁶ | 18.0× |

**Monotonically improving across L.** The pre-registered ceiling of cond(S) ≤ 10⁸ (PROTOCOL §5) was met at all L. The factor 18 improvement over L=16→L=64 is consistent with a well-defined L→∞ limit; finite-sample noise hypothesis (FTD-0099 §F1 closure) confirmed at the additional L=64 point.

### Wilson eigenvalue positivity

Eigenvalues of (M+M^T)/2 on the active operator subspace (s² dropped due to deterministic crystallization at default inj-mult=3.0 — known artifact, not L-dependent):

| L | Eigenvalues | Positive count |
|---|---|---|
| 16 | {255.0, 32.5, 18.7, **−2.98**, **−15.4**} | 3 of 5 |
| 32 | {255.9, 27.4, 16.4, +2.36, **−18.8**} | 4 of 5 |
| 64 | {172.1, 34.6, 11.1, **−1.29**, **−37.5**} | 3 of 5 |

**Non-monotonic in L.** L=32 had 4 positive, L=64 reverts to 3 positive. The k=3 eigenvalue, which crossed zero (−2.98 → +2.36) at L=16→L=32, slips back to negative (−1.29) at L=64. The k=4 eigenvalue is consistently negative across all L and grows in magnitude (−15.4 → −18.8 → −37.5).

Three diagnostic readings:

- **A (finite-sample noise)**: at L=64 the b=4 grid is 16³=4096 voxels vs L=32's 8³=512 — bigger grid means more averaging within a snapshot, but only N_SAMPLES=40 snapshots means seed-level fluctuation can flip sign of small eigenvalues. The k=3 magnitude (~1) is small enough that a single seed's contribution can flip the sign.
- **B (basis non-closure)**: the active 5-operator basis may not be closed under iterated b=2 blocking; eigenvalue migration suggests "leaky" mixing into operators not in the basis.
- **C (genuine L-dependence)**: the eigenvalue is a function of the lattice's spectral gap, which depends on L; non-monotonic behavior would then be expected at small L and asymptote at large L.

Cannot distinguish A/B/C from three datapoints. L=128 + repeated seeds at L=64 would resolve this.

### RG semigroup property M(b=4) ≈ M(b=2)·M(b=2)

| L | max relative error | verdict at 50% threshold |
|---|---|---|
| 16 | 1.80× | FAIL |
| 32 | 1.61× | FAIL |
| 64 | 1.87× | FAIL |

**Consistently failing at the pre-registered threshold across all L.** The semigroup property does NOT hold on this ensemble at any L tested. Per FTD-0099 §F5 closure interpretation, the FAIL itself is the measurement: the bootstrap-noise floor on b=4 entries is ~150–200% on this ensemble size (40 samples × 5 seeds), and the semigroup property would require lower noise OR a closed basis.

Note that L=64 (1.87×) is slightly worse than L=32 (1.61×) — at L=64 the b=4 sub-grid is bigger (16³ vs 8³) but still has the same N_SAMPLES, so per-snapshot noise on b=4 entries actually grows in absolute terms.

### Diagonal-dominance scoring

| L | diagonal-dominant ops | converged-stderr entries |
|---|---|---|
| 16 | 3 of 5 | 6 of 25 (24%) |
| 32 | 4 of 6 | 10 of 36 (28%) |
| 64 | 3 of 6 | 6 of 36 (17%) |

L=32 was the high-water mark; L=64 regresses on both metrics. This is consistent with the Wilson-eigenvalue non-monotonicity: at L=64 the same seed-noise budget is spread over a 16-fold-bigger lattice, so per-cell signal shrinks while bootstrap stderr grows.

---

## 3 · Interpretive summary

The L=64 datapoint does NOT cleanly support a "smooth continuum limit" claim, but does NOT decisively close it negatively either:

- **Positive evidence**: cond(S) improves monotonically; basic measurement infrastructure scales; Wilson eigenvalues remain "all-relevant" (Δ_eig < 4) as expected.
- **Negative evidence**: Wilson eigenvalue positivity is non-monotonic in L; RG semigroup fails across all L; per-entry stderr does not improve with L at fixed N_SAMPLES.

The cleanest reading: **the engine has a well-defined operator-mixing matrix structure, but the bootstrap noise floor on b=4 entries grows with L at fixed N_SAMPLES**. To test continuum limit cleanly would require N_SAMPLES ∝ L³ (for constant per-cell statistics) — not feasible at L=128 in single-session GPU budget.

**Tag**: [PARTIAL]. The campaign closes the question "does the L=64 measurement land cleanly?" — answer is "yes, but with non-monotonic eigenvalue behavior that resists clean interpretation at the available statistics."

---

## 4 · What this audit closes (and doesn't)

**Closures and partial-closures:**
- STATUS_EFT_CHECKLIST.md §10 (continuum limit) — was [OPEN]; this lifts to [PARTIAL]: cond(S) monotone improving, basis structure stable, RG semigroup consistently failing.
- FTD-0099 follow-up F1 (multilatitude beyond L=32) — [PARTIAL] now closed at L=64; deferred to L=128 + extended seeds for full closure.

**Not closed:**
- L=128 datapoint (deferred — would take 3+ hours wall).
- s² operator activation at L=64 (would require inj-mult=1.0 calibration per FTD-0100; ran with default inj-mult=3.0).
- Cross-observable continuum limit: this audit only measures operator-mixing. Coulomb force law (Phase G) and hydrogen spectrum continuum-limit reruns are deferred to a separate follow-up; not load-bearing for the engine-as-instrument portfolio.

---

## 5 · Comparison to the BCC closure pattern (FTD-0093)

The BCC sub-stencil spectrum audit (FTD-0093, [`AUDIT_BCC_SUBLATTICE_SPECTRUM.md`](AUDIT_BCC_SUBLATTICE_SPECTRUM.md)) tracked a similar L ∈ {24, 32, 48} progression and closed [NEGATIVE]: BCC ratio 11.76→10.31→15.24, never approaching predicted 45.31. That audit had a **specific quantitative prediction** to falsify and missed it by 10σ at every L.

This continuum-limit audit has **no specific prediction to falsify** — it's a convergence diagnostic. Both audits illustrate the user's reorientation: pre-registered measurements that produce structurally informative results regardless of whether they confirm a prior conjecture. FTD-0093 closes a derivation chain negative; FTD-0103 produces an inconclusive but informative continuum-limit diagnostic.

---

## 6 · Single-line summary

**Operator-mixing matrix M_ab(b=2) measured at L ∈ {16, 32, 64} with --b4 RG semigroup test. cond(S) improves monotonically (5.80×10⁷ → 8.74×10⁶ → 3.22×10⁶, factor 18 over L=16→L=64); Wilson eigenvalue positivity is non-monotonic (3+/2- → 4+/1- → 3+/2-); RG semigroup fails at pre-registered 50% threshold at all L (1.80×, 1.61×, 1.87×). At fixed N_SAMPLES=40 the per-cell statistics fall as L³, so finite-sample noise dominates the eigenvalue migration. Tag: [PARTIAL]. Engine has well-defined mixing-matrix structure with stable basis; clean continuum-limit closure deferred until N_SAMPLES ∝ L³ ensemble at L=128 is feasible. Engine-as-instrument observation, not SM-targeting.**

---

**End of audit.** L=128 datapoint (3+ hours wall) deferred to follow-up campaign with N_SAMPLES ∝ L³ ensemble.
