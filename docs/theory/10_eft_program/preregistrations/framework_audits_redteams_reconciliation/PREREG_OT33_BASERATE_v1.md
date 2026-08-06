# PREREG · OT-3.3 base-rate control (v1)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`
**Date locked:** 2026-08-04
**Parents:** OT-3.3 (`TRACKER_ONTIC_TRUTH.md`), FTD-0791, FTD-0319, OT-5.1
**Production impact:** none — this is a verification instrument.

## 1. Why

`TRACKER_ONTIC_TRUTH.md` OT-3.3 reports **0 dual-matchers across 2,871,576
polynomials** and is cited as numerical-uniqueness support for `x_+ = 1/alpha`
(OT-5.1). Its sibling — the FTD-0319 ~2.65M adversarial 18-constant-basket scan
— was audited by **FTD-0791** and found to sit exactly at its own chance base
rate (null expects 1.42–1.67 matchers, 1 found).

OT-3.3 is a **different runner** (`proof_polynomial_look_elsewhere_extended.py`,
tag `preregister-polynomial-scan-extended-v1`) and **the equivalent base-rate
control has never been run against it.** The tracker states the problem in its
own words:

> "a null expectation near zero and a null expectation near one look identical
> in the raw count."

This pre-registration locks that control before it runs.

## 2. Locked artifacts (SHA-256)

| Artifact | SHA-256 |
|---|---|
| `scripts/experiments/verify_ot33_baserate.py` (this control) | `ead73c6f9558a9f33c768b0a6d81a121f00f3627dbae52f1262d1debdfb9d41b` |
| `scripts/proofs/proof_polynomial_look_elsewhere_extended.py` (target) | `06ae1a1d290ba3f06a3cc4459eff1aa84f3a9cd3d84e6165514fa1e597ac05aa` |
| `scripts/experiments/verify_look_elsewhere_baserate.py` (FTD-0791 precedent) | `5fab6d96905fa8beb9006db0124b43cf375fdf41da0204529f966885632ac8a2` |

Monte Carlo seed **20260804**, 20,000 draws, both locked in the control source.

## 3. The signed observable

> **`N_null`** := the mean number of dual-matchers found in the EXT-A family at
> the **registered gate** (`x_+` within 1.26 ppm, `x_-` within 0.80%) when the
> target pair is displaced to locations carrying no FTD significance
> (`x_+ ~ U(110, 170)`, `x_- ~ U(2.0, 4.5)`), over 20,000 draws.

Reported alongside, as declared secondary measurements: the family's aliasing
factor (nominal vs distinct polynomials), the analytic null (local `x_+` root
density × gate width), and the eliminative power of the `x_-` leg.

## 4. Pre-blessed outcomes

Fixed before execution. No post-hoc tolerance adjustment is permitted.

| Outcome | Condition | Consequence |
|---|---|---|
| **A — OT-3.3 SURVIVES** | `N_null >= 1.0` | The family generically produces ≥1 matcher, so a zero count is genuinely surprising. OT-3.3 stands as numerical-uniqueness support; no retag. |
| **B — OT-3.3 UNINFORMATIVE** | `N_null < 0.1` | Finding zero others is what chance predicts. The count carries no evidential weight; OT-3.3 must be retagged as FTD-0319 was (`[MEASURED]` → `[SELECTION]`), and OT-5.1's remaining support line reduces accordingly. |
| **C — INTERMEDIATE** | `0.1 <= N_null < 1.0` | Report the number. No retag either way without a separate decision. |
| **D — EXECUTION INVALID** | The EXT-A family cannot be reconstructed to its registered size, or the control errors | No verdict admissible; diagnose and re-register. |

## 5. Pre-run observations (recorded before execution, from source reading only)

These are **not** results. They are properties of the target runner visible by
reading it, registered here so the ordering is on the record.

1. **The registered target is the claim, not the measurement.**
   `X_PLUS_TARGET = 137.0361714582` is the master quadratic's own tree root.
   CODATA is `137.035999177`. They differ by 1.26 ppm — *exactly the gate
   width*. The runner's own docstring says "within 1.26 ppm of CODATA 1/α",
   which the code does not implement. The window is centred on the claim.
2. **The dual-match predicate gates on a retired identification.**
   `X_MINUS_TARGET = 3.0239639163` is the `x_- ↔ N_c` identification the
   framework **retired** (FTD-0014, removed in `ca7eb61`; v1.4 §5). This is the
   same defect FTD-0791 found in FTD-0319, where all residual surprise belonged
   to that leg.
3. **The rational extension may be substantially aliased.** EXT-A multiplies
   coefficients by `n/d_n` with `d_n ∈ [1,4]`, and `16/1 = 32/2 = 48/3 = 64/4`.
   If so, the headline "2,359,296 polynomials" overstates the distinct search
   space. Measured as a declared secondary observable above.

## 6. Execution

```
python scripts/experiments/verify_ot33_baserate.py
```

Deterministic under the locked seed. Result to be registered as a new
`FTD-NNNN` row at the next free id, with the outcome letter and `N_null` value
quoted verbatim.
