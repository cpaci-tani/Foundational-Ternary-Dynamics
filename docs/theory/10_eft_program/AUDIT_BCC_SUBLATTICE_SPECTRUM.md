# AUDIT — Mechanism C BCC Sub-Stencil Spectrum (FTD-0093 closure read)

**Tag:** [CLOSED NEGATIVE] (pending L=48 confirmation)
**Date:** 2026-04-27
**Status:** D6 audit closure of [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md)
**Pre-registered prediction (FTD-0093 §3):** BCC sub-stencil two-state spectrum ratio λ₊/λ₋ ≈ 45.31 = X_PLUS/X_MINUS = 137.04/3.024
**Companion docs:** [`DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`](DERIV_MECHANISM_C_GC_BCC_BRIDGE.md), [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md)

---

## 1 · Why this audit exists

[`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md) (D2) pre-registered the falsifier for FTD-0093 (Mechanism C as the live first-principles route for `g_c`). The campaign at smoke level (L=16) passed in 2026-04-26's session; this audit closes the publication-grade run at L ∈ {24, 32, 48} per D6.

The closure is **NEGATIVE-WITH-DIAGNOSIS**:

- BCC ratio at L=24: **11.76 ± 2.28** (predicted 45.31)
- BCC ratio at L=32: **10.31 ± 4.24** (predicted 45.31)
- BCC ratio at L=48: *(pending)*

The trend is **away from** the predicted value, not toward it. Even allowing for finite-L bias and 1/L² extrapolation, the prediction's order of magnitude is not reached at any L tested.

**Falsifier verdict (PROTOCOL §5)**: at the lattice sizes accessible in a single GPU session, the BCC sub-stencil two-state spectrum does NOT match the master-quadratic ratio 45.31. By the pre-registered terms, this terminally demotes FTD-0094 (the L2 candidate identity 2·m_e/α = 16G*²) to [PARAMETRIC] in the absence of a separate structural mechanism.

The non-trivial structural finding (positive content of the negative result): the Wilson eigendecomposition + multi-stencil control comparison reveals that **none** of the sub-stencils (SC, FCC, BCC, FULL) produces a clean two-state spectrum at the predicted ratio. This is informative — it tells us the Prony AR(2) two-state model is too restrictive for the engine's actual autocorrelation structure on these sub-stencils. The engine's flux-energy autocorrelation at the BCC sub-stencil has either a single dominant decay scale + noise floor, or a richer multi-scale spectrum, but not the two clean exponentials Mechanism C predicted.

---

## 2 · Quantitative results

### Per-stencil ratio table

| Stencil | L=24 | L=32 | L=48 | Predicted |
|---|---|---|---|---|
| BCC | 11.76 ± 2.28 | 10.31 ± 4.24 | *(pending)* | 45.31 |
| SC  | 18.88 ± 13.51 | 56.83 ± 33.74 | *(pending)* | not 45.31 |
| FCC | 13.89 ± 8.59 | 8.72 ± 2.15 | *(pending)* | not 45.31 |
| FULL | 19.91 ± 3.47 | 17.45 ± 4.12 | *(pending)* | not 45.31 |

### Per-stencil sum table (predicted: 16·G*² ≈ 140.06)

The sum λ₊+λ₋ from valid extractions has values of order 0.01–0.08 (3-4 orders of magnitude below the predicted 140). This is a units-convention finding: the engine's flux-energy autocorrelation has decay rates expressed in inverse-tick units, while the master quadratic prediction is expressed in dimensionless eigenvalue units. The PROTOCOL specifies that BOTH ratio AND sum should match; only ratio testing is structurally informative under unit-convention agnosticism. Even the ratio fails — that's the load-bearing finding.

### 1/L² extrapolation

The pre-registered protocol calls for a 1/L² extrapolation to L→∞. From two data points (L=24, L=32):

- BCC ratio slope vs 1/L²: ratio decreases as L grows (NOT increases toward 45.31)
- Linear extrapolation to L→∞ via points (1/24²=0.00174, 11.76), (1/32²=0.000977, 10.31): slope ≈ 1900, intercept ≈ 8.4
- Extrapolated L→∞ ratio: ≈ 8.4

Predicted: 45.31. **Extrapolated: 8.4. Off by factor 5.4.**

L=48 datapoint will lock the trend definitively. If L=48 BCC ratio < 11 (continuing the trend), the extrapolation stays at ~8. If L=48 BCC ratio > 12 (reversal), then the L=24 and L=32 measurements were finite-L artifacts and the 1/L² extrapolation needs more data. Either way, the prediction of 45.31 is not supported.

---

## 3 · What this audit means for FTD-0093 and FTD-0094

| Ledger row | Pre-audit status | Post-audit status |
|---|---|---|
| FTD-0093 (Mechanism C) | [CONJECTURE] | [CLOSED NEGATIVE] — pending L=48 confirmation |
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
- PROTOCOL: [`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`](PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md) §5 falsifier criteria
- Source: [`engine/tests/campaign_bcc_band_spectrum.cpp`](../../engine/tests/campaign_bcc_band_spectrum.cpp) (production parameters L ∈ {24, 32, 48} via CLI flags added 2026-04-27)
- Outputs:
  - `engine/results/bcc_spectrum_2026-04-27/L24/{spectrum,stencil_aggregate,meta}.csv`
  - `engine/results/bcc_spectrum_2026-04-27/L32/{spectrum,stencil_aggregate,meta}.csv`
  - `engine/results/bcc_spectrum_2026-04-27/L48/...` *(pending)*
- LEDGER updates: FTD-0093 → [CLOSED NEGATIVE]; FTD-0094 → [PARAMETRIC] (terminal demotion conditional on FTD-0096 OPEN)
- DERIV: [`DERIV_MECHANISM_C_GC_BCC_BRIDGE.md`](DERIV_MECHANISM_C_GC_BCC_BRIDGE.md) — original conjecture document

---

## 7 · Single-line summary

**Mechanism C falsifier (PROTOCOL §5) FAILS at L ∈ {24, 32}: BCC sub-stencil two-state ratio is 11.76 ± 2.28 and 10.31 ± 4.24 respectively, trending DOWNWARD (away from predicted 45.31). 1/L² extrapolation lands at ~8.4, far below predicted 45.31. Three first-principles routes for `g_c` (Mechanisms A, B, C) are now all closed-negative; g_c remains [PARAMETRIC]. FTD-0094 (2·m_e/α = 16G*²) demotes to [PARAMETRIC] terminally if FTD-0096 (μ-from-ℓ_P) stays OPEN. The negative result is informative per the user's reorientation toward "qualitate negative results"; it doesn't demote the algebraic spine (master quadratic, G*, Watson identity) which are independent [THEOREMs].**

---

**End of audit.** L=48 datapoint will be appended when the production run completes.
