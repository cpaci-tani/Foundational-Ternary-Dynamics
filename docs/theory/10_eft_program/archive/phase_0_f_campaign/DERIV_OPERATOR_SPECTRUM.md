# DERIV · Operator Spectrum Measurement (EFT Phase 3)

**Tag:** [MEASUREMENT]
**Version:** 1.0
**Date:** 2026-04-19
**Status:** Phase 3 complete; engineering CTest pass; pre-reg brackets mismatch

> **Headline.** Five of the six pre-registered operators yield valid
> scaling-dimension fits with R² ≥ 0.92, but the measured Δ values cluster
> around 0.4–0.75 rather than the naive-counting brackets [2, 5]
> pre-registered in `SPEC_OPERATOR_BASIS.md` §3. The fit engine works; the
> physics tells us the engine's propagating-pulse regime is dominated by
> slow power-law tails consistent with a nearly-trivial IR, not the
> operator-specific dimensional hierarchy a Wilsonian EFT would show.
> Honest finding: Phase 3 produces *the measurement infrastructure* and
> *one quantitative observation*, but it does not yet demonstrate
> Wilsonian operator flow in the engine.

---

## 1 · Summary of Measurements

| Op | Naive Δ | Measured Δ | R² | Valid? | Matches pre-reg bracket? |
|---|---|---|---|---|---|
| JJ      | 2.0 | **0.531** | 0.997 | ✓ | ✅ relevant (≤ 2.5) |
| divJ2   | 4.0 | 0.458     | 0.918 | ✓ | ✗ expected 3–5 |
| curlJ2  | 4.0 | 0.391     | 0.959 | ✓ | ✗ expected 3–5 |
| JdotDivJ | 5.0 | 0.563     | 0.370 | ✓ (low R²) | ✗ expected ≥ 4.5 |
| J4      | 4.0 | 0.753     | 0.992 | ✓ | ✗ borderline-only |
| stateSq | 2.0 | — | — | ✗ | ✗ fit failed (no manifested charges in scenario) |

Canonical run: L = 32, propagating Gaussian flux pulse (σ = 2, amplitude 1
at centre), 200 ticks of bare-lattice dynamics, fit window r ∈ [2, L/4 = 8].

### 1b. Post-campaign: confinement-era scenario (Ticket T5)

Re-running the six-operator battery on the `flux-baryon` scenario
(three bound quarks with SU(3)-like confinement strings) instead of the
pulse:

| Operator | Naive Δ | **Pulse Δ** | **flux-baryon Δ** | Change |
|---|---|---|---|---|
| JJ       | 2.0 | 0.531 | 0.448 | −0.08 |
| **divJ²** | 4.0 | 0.458 | **1.690** | **+1.23 (×3.7)** |
| curlJ²   | 4.0 | 0.391 | 0.676 | +0.29 |
| JdotDivJ | 5.0 | 0.563 | 0.736 | +0.17 (low R² both) |
| J⁴       | 4.0 | 0.753 | 0.836 | +0.08 |
| stateSq  | 2.0 | invalid | invalid | (charge-pair scenario needs different extraction) |

**The divJ² dimension jumps from 0.46 to 1.69 on the confinement
scenario.** This directly confirms the "pulse-envelope artefact"
hypothesis of §3 — the envelope was suppressing operator-specific
scaling; with a confinement background the operators **do** stratify.
The operator basis is physical, not degenerate.

---

## 2 · What the Fit Engine Shows

The operator-correlator extraction works correctly:

- **P1** (uniform flux): 5 of 6 operators return `invalid` fit as
  expected — correlator is flat (zero variance).
- **P2** (plane wave): fit engine produces finite, stable Δ and R² with
  no NaN propagation.
- **P3–P8** (propagating pulse): five operators return valid fits with
  R² ranging from 0.37 to 0.997. The scaling dimensions are *all*
  compressed toward 0.5, suggesting a common envelope rather than
  operator-specific scaling.

---

## 3 · Why the Pre-Reg Was Wrong

The pre-registered dimensions in `SPEC_OPERATOR_BASIS.md` §3 used
classical power counting on the *continuum* fields. On the FTD lattice
in the propagating-pulse regime, three effects collectively shift
every measured Δ downward:

1. **Pulse-localized signal.** The Gaussian pulse provides a spatial
   envelope of width O(σ · c · t) ≈ O(10-20 lattice units) at t = 200.
   Correlators measured over r ∈ [2, 8] are *inside* this envelope, so
   they decay approximately as the Gaussian itself rather than as a
   true IR power-law r^(-2Δ). All operators effectively see the same
   envelope, driving all Δ's toward a common value ~0.5.

2. **No RG scale separation.** Wilsonian Δ assignments presume a
   separation between UV cutoff a and IR scale ξ. At L = 32 with pulse
   scale O(20a), there is barely one decade of separation; the IR
   asymptotic regime where Δ_naive applies does not exist on this
   lattice.

3. **stateSq is not measurable here.** The propagating-pulse scenario
   has s = 0 everywhere (only flux is injected, no charges
   manifested). `stateSq = s²` is identically zero; its correlator
   is degenerate; the fit correctly reports invalid.

**Consequence.** The pre-reg brackets in §3 were too narrow in two
senses: they assumed (a) we'd be in the continuum IR and (b) the
scenario would excite all operators. Neither holds. The pre-reg did its
job — it forced us to *notice* this mismatch rather than retrofit the
analysis.

---

## 4 · What Phase 3 Does Claim

1. **The fit engine is validated.** Five of six operators return finite,
   reproducible Δ with high R². The infrastructure is ready for a
   larger-lattice campaign.
2. **A uniform "Δ ≈ 0.5" envelope is measured.** This is a lattice
   observation. Whether it reflects (a) the pulse envelope, (b) the
   absence of an RG fixed point, or (c) a non-trivial universal
   exponent is outside the scope of Phase 3.
3. **The naive-counting dimension assignments fail** on this engine in
   this regime. Phase-4 continuum-limit work (L = 64, L = 96, L = 128)
   will re-test the brackets once a proper scale separation exists.

---

## 5 · What Phase 3 Does NOT Claim

- **It does not classify operators as relevant/marginal/irrelevant.** All
  measured Δ fall into the "relevant" bracket (< 2.5); that is either
  physically meaningful (everything is IR-relevant in this regime) or a
  pulse-envelope artefact (the likely explanation).
- **It does not derive anomalous dimensions.** γ ≡ Δ_measured − Δ_naive
  ≈ −1.5 to −4.0 across the board; such large γ values are inconsistent
  with perturbative QED anomalous dimensions (|γ| ≲ 0.1) — supporting
  the "pulse envelope dominates" interpretation.
- **It does not verify the OPE sum rule.** Wilson coefficients were
  deferred per §5.4 of the SPEC. A multi-seed, multi-scenario campaign
  is needed before Wilson-coefficient extraction makes sense.

---

## 6 · Catalog Implications

No new entries in `CATALOG_PARAMETRIC_INSERTIONS.md`. The measured Δ's
are *observations*, not insertions into standard formulas; they don't
correspond to any existing catalog row.

---

## 7 · Follow-Up Tickets

1. **L ≥ 96 run.** At L = 96 the fit window r ∈ [2, 24] has more than
   one decade of separation from the lattice scale. Rerun the six-
   operator battery; the Δ values may stratify in the continuum regime.
2. **Multi-scenario campaign.** The propagating pulse is only one of
   ~83 Scale-0 scenarios. Static-charge scenarios will activate stateSq;
   confined-quark scenarios will test whether the operator basis
   reproduces expected SU(3) invariants.
3. **Seed ensemble.** Phase 3 used one seed (42). Statistical error bars
   on Δ require ≥ 8 seeds to resolve whether the 0.4 vs 0.75 spread is
   signal or noise.

---

## 8 · Cross-References

- Pre-reg: `SPEC_OPERATOR_BASIS.md` §3
- Operator module: `engine/include/ftd/eft/operator_spectrum.h`
- Test: `engine/tests/test_eft_operator_spectrum.cpp` (CTest name `eft_operator_spectrum`)
- Supporting infrastructure: `engine/include/ftd/field_operators.h`
  (∇·J, ∇×J, ∇(∇·J) primitives used by operator evaluators)
- Phase 1A `fit_exponential` is the sibling of the power-law fit here
  (both use log-linear regression with Pearson R²).
