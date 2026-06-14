# PREREG: Alpha Estimator Validation v2

**FTD ID:** FTD-0286 (v2 continuation)
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED]
**Parent:** FTD-0286 v1 returned `ENERGY_FUNCTIONAL_MISMATCH`
**Engine artifact:** `engine/tests/campaign_alpha_estimator_validation_v2.cpp`
**Artifact SHA256:** `9b6431c1f37f835969e38bf1de0f79d75625a75c86db0de6921a68323e6bdc74`
**Shared gate helper:** `engine/include/ftd/eft/lattice_coulomb_gate.h`

---

## 1. Question

FTD-0286 v1 paired `energy_audit().field_energy` (which uses `½ Σ|J|²`
since 2026-04-27) with the legacy Phase-G gate `α_r = 2 r G_L(r)` derived
for `Σ|J|²`. The matched arm landed at mean ratio ≈ 0.501 — exactly half.

The v2 question is:

```text
Does the field-energy observable match the lattice Coulomb gate once the
½ prefactor is paired correctly (α_r_expected = r G_L(r))?
```

This is still instrument validation. It cannot promote `x_+ = 1/alpha`.

---

## 2. Fixed Protocol

Identical simulation setup to v1 except for the analytic gate:

| Quantity | Value |
|---|---:|
| Lattice size | `L = 32` |
| Separations | `r = {5, 7, 9}` |
| **Analytic gate (v2)** | **`α_r(r,L) = r G_L(r)`** (pairs with `½ Σ|J|²`) |
| Relative tolerance | `10%` |
| Production ticks / SOR | `300` / `100` |
| Matched projector | `matched_gauss_project`, tol `1e-10`, max iter `400` |

Energy convention tag printed in output: `energy_convention,half_sum_j2`.

---

## 3. Outcomes

### Outcome H: `HALF_ENERGY_GATE_CONFIRMED_MATCHED`

Criteria:

- Matched-stencil static mode passes the v2 absolute gate.
- Production live-tick mode fails the v2 absolute gate.

Interpretation:

The v1 mismatch was an observable/gate pairing error, not a failure of
matched-stencil projection. The matched estimator is the canonical finite-cell
readout for `energy_audit().field_energy`. Production Gauss stencil drift
remains a separate issue (~12% systematic at this window). No alpha claim is
promoted.

### Outcome B: `BOTH_GATES_CONFIRMED`

Criteria:

- Both modes pass the v2 absolute gate.

Interpretation:

Would contradict v1 + FTD-0285 unless engine behavior changed. Audit before
drawing physics conclusions.

### Outcome S: `STILL_MISMATCH`

Criteria:

- Matched mode fails even with the half-energy gate.
- Matched projection converges.

Interpretation:

The pairing fix is insufficient; a different observable (potential response,
source work) is required. No alpha claim is promoted.

### Outcome C: `MATCHED_PROJECTOR_FAILED_TO_CONVERGE`

Criteria:

- Matched projection reports non-convergence.

Interpretation:

Artifact invalid for the estimator question.

---

## 4. Banned Moves

- Do not claim α derivation or promote FTD-0013 / MC-T4.3.
- Do not retroactively rewrite v1 output or verdict.
- Do not change global `field_energy` ½ convention without a separate
  golden/audit regression arc.

---

## 5. Claim Status (pre-run)

- FTD-0286 v1 remains `[MEASUREMENT ANALYSIS -- ENERGY FUNCTIONAL MISMATCH]`.
- v2 addresses the documented pairing gap only.
