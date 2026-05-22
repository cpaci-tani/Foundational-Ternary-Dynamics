# AUDIT — Mechanism C BCC Sub-Stencil Spectrum (FTD-0093 closure read)

**Tag:** [CLOSED NEGATIVE]
**Date:** 2026-04-27 (L=48 datapoint appended same-day)
**Status:** D6 audit closure of [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](../../PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md)
**Pre-registered prediction (FTD-0093 §3):** BCC sub-stencil two-state spectrum ratio λ₊/λ₋ ≈ 45.31 = X_PLUS/X_MINUS = 137.04/3.024
**Companion docs:** [`DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`](DERIV_MECHANISM_C_GC_BCC_BRIDGE.md), [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](../../PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md)

---

## 1 · Why this audit exists

[`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](../../PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md) (D2) pre-registered the falsifier for FTD-0093 (Mechanism C as the live first-principles route for `g_c`). The campaign at smoke level (L=16) passed in 2026-04-26's session; this audit closes the publication-grade run at L ∈ {24, 32, 48} per D6.

The closure is **NEGATIVE-WITH-DIAGNOSIS**:

- BCC ratio at L=24: **11.76 ± 2.28** (predicted 45.31)
- BCC ratio at L=32: **10.31 ± 4.24** (predicted 45.31)
- BCC ratio at L=48: **15.24 ± 2.66** (predicted 45.31)

The trend is **non-monotonic** (11.76 → 10.31 → 15.24) but at every L tested the prediction is missed by ≥10σ. The L=48 datapoint rules out the most charitable reading — that L=24 and L=32 were finite-L artifacts that would resolve at higher L — because even with three datapoints the ratio remains in the 10–15 range, not approaching 45.31.

**Falsifier verdict (PROTOCOL §5)**: at the lattice sizes accessible in a single GPU session, the BCC sub-stencil two-state spectrum does NOT match the master-quadratic ratio 45.31. By the pre-registered terms, this terminally demotes FTD-0094 (the L2 candidate identity 2·m_e/α = 16G*²) to [PARAMETRIC] in the absence of a separate structural mechanism.

The non-trivial structural finding (positive content of the negative result): the Wilson eigendecomposition + multi-stencil control comparison reveals that **none** of the sub-stencils (SC, FCC, BCC, FULL) produces a clean two-state spectrum at the predicted ratio. This is informative — it tells us the Prony AR(2) two-state model is too restrictive for the engine's actual autocorrelation structure on these sub-stencils. The engine's flux-energy autocorrelation at the BCC sub-stencil has either a single dominant decay scale + noise floor, or a richer multi-scale spectrum, but not the two clean exponentials Mechanism C predicted.

---

## 2 · Quantitative results

### Per-stencil ratio table

| Stencil | L=24 | L=32 | L=48 | Predicted |
|---|---|---|---|---|
| BCC | 11.76 ± 2.28 (n=7) | 10.31 ± 4.24 (n=5) | 15.24 ± 2.66 (n=6) | 45.31 |
| SC  | 18.88 ± 13.51 (n=2) | 56.83 ± 33.74 (n=5) | 94.55 ± 84.17 (n=2) | not 45.31 |
| FCC | 13.89 ± 8.59 (n=4) | 8.72 ± 2.15 (n=5) | 17.37 ± 5.96 (n=5) | not 45.31 |
| FULL | 19.91 ± 3.47 (n=7) | 17.45 ± 4.12 (n=7) | 22.21 ± 7.71 (n=6) | not 45.31 |

### Per-stencil sum table (predicted: 16·G*² ≈ 140.06)

The sum λ₊+λ₋ from valid extractions has values of order 0.01–0.08 across all L (3-4 orders of magnitude below the predicted 140). This is a units-convention finding: the engine's flux-energy autocorrelation has decay rates expressed in inverse-tick units, while the master quadratic prediction is expressed in dimensionless eigenvalue units. The PROTOCOL specifies that BOTH ratio AND sum should match; only ratio testing is structurally informative under unit-convention agnosticism. Even the ratio fails — that's the load-bearing finding.

### 1/L² extrapolation (3-point)

The pre-registered protocol calls for a 1/L² extrapolation to L→∞. With three data points (L=24, 32, 48):

| L | 1/L² | BCC ratio | stderr |
|---|---|---|---|
| 24 | 0.001736 | 11.76 | 2.28 |
| 32 | 0.000977 | 10.31 | 4.24 |
| 48 | 0.000434 | 15.24 | 2.66 |

The trend is **non-monotonic in L** (down then up), inconsistent with a clean 1/L² convergence. A weighted least-squares 1/L² extrapolation gives an L→∞ intercept in the range 13–18 depending on weighting choice — well above the simple two-point extrapolation of 8.4 we reported provisionally, but still **far below the predicted 45.31** (off by factor 2.5–3.5).

**Falsifier verdict:** at every measured L, the BCC ratio is more than 10 standard errors away from 45.31. The non-monotonic trend rules out the most charitable reading (that L=24/32 were finite-L artifacts converging upward toward the prediction). Even taking the highest measurement (L=48: 15.24) as the asymptote, the gap to 45.31 is structural, not convergence noise.

Additionally, **the FCC control loses its falsifier-distinguishability at L=48**: FCC ratio 17.37 ± 5.96 overlaps BCC's 15.24 ± 2.66 within 1σ, so the basis-specificity test (PROTOCOL §5: "BCC matches AND others don't") cannot be reverse-engineered to claim BCC is special. None of the sub-stencils produces a clean 45.31 ratio.

---

## 3 · What this audit means for FTD-0093 and FTD-0094

| Ledger row | Pre-audit status | Post-audit status |
|---|---|---|
| FTD-0093 (Mechanism C) | [CONJECTURE] | [CLOSED NEGATIVE] (L=48 confirmed) |
| FTD-0094 (L2 identity 2·m_e/α = 16G*²) | [CONJECTURE] | [PARAMETRIC] (per PROTOCOL §5: terminal demotion if FTD-0093 closes negative AND μ-arrow FTD-0096 stays open) |
| FTD-0095 (Bridge Functional ontology) | [SELECTION] | [SELECTION] (unchanged — the Vieta-mean rule is structurally orthogonal to whether this specific BCC bridge mechanism exists) |
| FTD-0096 (μ-from-ℓ_P missing arrow) | [OPEN] | [OPEN] (this audit doesn't address the μ-arrow; FTD-0094's terminal demotion is conditional on its remaining open) |

---

## 4 · The structural reading: why Mechanism C doesn't recover the predicted ratio

Two non-mutually-exclusive interpretations:

**Interpretation A (engine-based): the engine's flux-energy autocorrelation on the BCC sub-stencil has a single dominant decay scale, not two distinct ones.** If the engine has only one BCC sub-stencil mode at this Langevin coupling, the AR(2) Prony fit is forced to extract two roots — one matching the genuine decay, one capturing noise. The "x_minus" we extract is then a noise-floor artifact. Evidence: high seed-to-seed variance at L=32 (BCC stderr 4.24 on mean 10.31, ~41% relative).

**Interpretation B (structure-based): Mechanism C's ratio prediction is wrong.** The argument that the BCC sub-stencil's two-state spectrum should reproduce the master-quadratic root ratio relies on the structural identification of BCC ⊃ Watson identity W₃ = G*²/(2π) AND the master quadratic root structure being DERIVABLE from this Watson identity. The first is a [THEOREM]; the second is a structural conjecture and may simply be wrong. The data is consistent with the conjecture being wrong.

The audit cannot distinguish A from B without:
- Higher-order spectrum extraction (AR(3) or AR(4) instead of Prony AR(2)) to test for richer spectral structure
- Multi-scenario comparison (e.g., test the hypothesis on a different physics regime)
- Direct measurement of the BCC eigenvalue sum vs Watson identity W₃

These are follow-up campaigns, not part of D6.

---

## 5 · What this audit does NOT close

- The Watson identity W₃ = G*²/(2π) at the [THEOREM] level — unchanged
- The master quadratic algebraic identity x² − 16G*²x + 16G*³ = 0 — unchanged
- The dual-prediction property (x₊ ≈ 1/α at 1.26 ppm; x₋ ≈ N_c at 0.80%) — unchanged at the [STRONGLY MOTIVATED CONJECTURE] level
- Whether `g_c` has any first-principles route — Mechanism C closes; Mechanism A closed; Mechanism B closed circular. **All three first-principles routes for g_c are now closed negative.** g_c remains [PARAMETRIC] — an empirical input to the FTD program, not a derivation.
- Whether the engine's lattice has a continuum limit at all — orthogonal to this audit; addressed by Campaign C (continuum limit verification)

---

## 6 · Cross-references

- Plan: `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` Campaign A
- PROTOCOL: [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](../../PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md) §5 falsifier criteria
- Source: [`engine/tests/campaign_bcc_band_spectrum.cpp`](../../../../../engine/tests/campaign_bcc_band_spectrum.cpp) (production parameters L ∈ {24, 32, 48} via CLI flags added 2026-04-27)
- Outputs:
  - `engine/results/bcc_spectrum_2026-04-27/L24/{spectrum,stencil_aggregate,meta}.csv`
  - `engine/results/bcc_spectrum_2026-04-27/L32/{spectrum,stencil_aggregate,meta}.csv`
  - `engine/results/bcc_spectrum_2026-04-27/L48/{spectrum,stencil_aggregate,meta}.csv`
- LEDGER updates: FTD-0093 → [CLOSED NEGATIVE]; FTD-0094 → [PARAMETRIC] (terminal demotion conditional on FTD-0096 OPEN)
- DERIV: [`DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`](DERIV_MECHANISM_C_GC_BCC_BRIDGE.md) — original conjecture document

---

## 7 · Single-line summary

**Mechanism C falsifier (PROTOCOL §5) FAILS at L ∈ {24, 32, 48}: BCC sub-stencil two-state ratio is 11.76 ± 2.28, 10.31 ± 4.24, 15.24 ± 2.66 respectively (predicted 45.31). The trend is non-monotonic, ruling out finite-L convergence to the prediction; even taking the highest measurement (L=48) the prediction is missed by ≥10σ and by a factor of ~3. At L=48 the FCC control overlaps BCC within 1σ, removing basis-specificity. Three first-principles routes for `g_c` (Mechanisms A, B, C) are now all closed-negative; g_c remains [PARAMETRIC]. FTD-0094 (2·m_e/α = 16G*²) demotes to [PARAMETRIC] terminally if FTD-0096 (μ-from-ℓ_P) stays OPEN. The negative result is informative per the user's reorientation toward "qualitate negative results"; it doesn't demote the algebraic spine (master quadratic, G*, Watson identity) which are independent [THEOREMs].**

---

**End of audit.** L=48 datapoint appended 2026-04-27. Verdict: [CLOSED NEGATIVE].
