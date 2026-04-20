# TRACKER — Outstanding Re-Derivation Items After the Undefined-Boundary Reframe

**Status:** open work queue.
**Trigger:** [AUDIT_INFINITY_REFRAME.md](AUDIT_INFINITY_REFRAME.md) ruled "L → ∞ as a load-bearing step" inadmissible. The April 19 mechanical-restatement sweep identified 5 inline `[FLAG: re-derivation needed]` markers planted in source documents where the original argument was not merely stylistically completed-infinity, but **substantively load-bearing on the limit**. This file enumerates them so they are not lost in subsequent edits.

Each row gives: location, what the original claim was, why the limit-language was load-bearing, and the shape of a finitary replacement that would discharge the flag.

---

## 1 · Inventory

| # | File | Line(s) | Claim | Why limit was load-bearing | Finitary replacement shape | Status |
|---|---|---|---|---|---|---|
| 1 | [`DERIV_BETA_FUNCTION_MEASURED.md`](../10_eft_program/DERIV_BETA_FUNCTION_MEASURED.md) | ~312 | β-function measurements at L ∈ {16, 32, 64} extrapolated to "β → β_QED as L → ∞" | The whole point was to demonstrate convergence to the QED β-function; "approaching with `O(1/L^p)`" without a justified `p` is just curve-fitting | Pre-register a specific scaling exponent `p` from the lattice action's dimensional structure, then test `β(L) − β_QED ∝ L^{−p}` as a falsifiable claim across `L ∈ {64, 128, 256}` with stated tolerance. Either the exponent is matched (the finitary statement holds) or it isn't (the convergence claim is retracted). | **RESOLVED 2026-04-19 — Restatement A applied.** Sign agreement asserted, magnitude discrepancy reported, scaling exponent prediction queued. |
| 2 | [`DERIV_DYNAMICAL_SM_EMERGENCE.md`](../10_eft_program/DERIV_DYNAMICAL_SM_EMERGENCE.md) | ~23 | "α_eff(L → ∞) extrapolated … consistent with an a → 0 limit" | Two limit appeals stacked: large-L *and* fine-spacing. Both inadmissible as load-bearing | Restate as: "for the canonical reference regime `L ∈ {64, 128, 256, 384}`, the measured `α_eff(L)` exhibits scaling exponent `p` ≈ X (fitted, not derived). Whether this scaling extrapolates to `α_ref` cannot be decided within the framework's ontology and is a **question about how the engine matches QED at fixed `a_phys`**, not a convergence theorem." Then cite [`OPEN_A_PHYS_DERIVATION.md`](../10_eft_program/OPEN_A_PHYS_DERIVATION.md). | **RESOLVED 2026-04-19 — Restatement B applied.** Reframed as calibration-conditional under `a_phys`. |
| 3 | [`DERIV_DAY2_CAMPAIGN.md`](../10_eft_program/DERIV_DAY2_CAMPAIGN.md) | ~312 | "The Phase-2 and Day-2 claim that FTD converges toward [QED] as the lattice grows" | Phase-2 narrative made convergence the headline; the lattice ontology forbids it as load-bearing | Replace with: "the measured `α(L)` is monotonic / non-monotonic / fluctuating in `L`; the rate at which `α(L)` changes between `L=64` and `L=384` is X% per doubling. This is a finite-L scaling diagnostic, not a convergence proof." Headline becomes the rate, not the limit. | **RESOLVED 2026-04-19 — Restatement B applied.** Headline reframed as α_largeL ≈ 3.6 × α_ref under declared `a_phys ≡ ℓ_P`. |
| 4 | [`DERIV_DAY2_CAMPAIGN.md`](../10_eft_program/DERIV_DAY2_CAMPAIGN.md) | ~399 | "[the post-Day-2 measurement] does not demonstrate that the measurement approaches α_ref" | Same issue: framing the question as "does it approach α_ref?" presumes a limit | Re-state the question as: "at the canonical reference regime, what is `α_engine` and what is its scaling rate vs `L`?" The answer is reported as a value with an error bar at each `L`, plus a fitted exponent — not as a limit. | **RESOLVED 2026-04-19 — Restatement B applied.** Measurement framed as falsifiable under declared calibration. |
| 5 | [`CONJ_ALPHA_FROM_CM.md`](../09_mathematical/CONJ_ALPHA_FROM_CM.md) | ~128 | "Path A in §Attack Vectors plus the Self-Consistency conjecture" relied on completed-infinity | The conjecture's Path A used "L → ∞ self-consistency" as the proposed mechanism for `α` | Either drop Path A (its premise is now inadmissible) or restate as a finite-L self-consistency that the framework can actually verify at specified `L`. The other paths in the document are unaffected. | **RESOLVED 2026-04-19 — Path A retracted.** Paths B/C/D unaffected; preferred vector is algebraic + structural. |

---

## 2 · Common pattern across all five

Every flagged item shares one structure:

> **Claim:** "Engine measurement X(L) → physical-target Y as L → ∞."

Under undefined-boundary ontology, this is not a well-posed claim. The two honest restatements are:

**Restatement A — finitary scaling claim.** "X(L) − Y ∝ L^{−p} with exponent `p` ≈ N at the canonical regime, fitted across `L ∈ {…}` to within tolerance ε." This is falsifiable (predict `p`, test it), and does not invoke any limit.

**Restatement B — calibration-conditional claim.** "X(L_canonical) = Y at fixed `a_phys = …` and matched at observable Z." This converts the would-be derivation into a calibration: at one specified `a_phys`, the prediction matches; at another, it doesn't. (See [`OPEN_A_PHYS_DERIVATION.md`](../10_eft_program/OPEN_A_PHYS_DERIVATION.md) and [`DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md`](../10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md).)

Either restatement is acceptable epistemically. The unacceptable position is leaving the limit-language in place as if the reframe had not happened.

---

## 3 · Recommended workflow for the EFT-campaign owner

For each row above:

1. Decide whether the original limit claim was meant to be a *scaling claim* (use Restatement A) or a *calibration claim* (use Restatement B).
2. Replace the inline `[FLAG: re-derivation needed …]` marker with the chosen restatement, in place, surgically.
3. If the restated claim cannot be supported by the existing data, retract the claim and update the surrounding narrative.
4. After all five rows are addressed, delete this tracker (or move it to `archive/` and replace with a short closure note).

Until then, this tracker is the canonical list — every reader of the EFT-program docs should be aware that these claims are queued for re-derivation, not silently accepted.

---

## 4 · Reproducibility

```
docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md            # ontology trigger
docs/theory/10_eft_program/OPEN_A_PHYS_DERIVATION.md           # the calibration question
docs/theory/10_eft_program/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md  # closed candidate
docs/theory/07_assessment/TRACKER_REFRAME_FLAGS.md             # this tracker
```

Each flagged file contains a literal `[FLAG: re-derivation needed]` string at the cited line; `grep -rn "FLAG.*re-derivation" docs/theory/` is the canonical scan.
