# PREREG: Alpha Estimator Validation v1

**FTD ID:** FTD-0286
**Date:** 2026-06-13
**Status:** [PRE-REGISTRATION -- LOCKED]
**Parent:** FTD-0285 invalidated protocol
**Engine artifact:** `engine/tests/campaign_alpha_estimator_validation.cpp`
**Artifact SHA256:** `dce6018d4ccc7565c1bab6870c9a90647f1bc4290c0fed600cac0fd3883ee570`
**Lock tag:** `preregister-alpha-estimator-validation-v1`

---

## 1. Question

FTD-0285 failed because the live finite-cell engine estimator did not match the
analytic `2 r G_L(r)` absolute Phase-G gate. The next question is not "does FTD
derive alpha?" The next question is:

```text
Is the FTD-0285 failure caused by the production live-tick Gauss estimator,
or by the field-energy observable itself?
```

This is an instrument-validation campaign. It cannot promote `x_+ = 1/alpha`.

---

## 2. Frozen Artifact

Run-of-record artifact:

```text
engine/tests/campaign_alpha_estimator_validation.cpp
```

Frozen source hash:

```text
dce6018d4ccc7565c1bab6870c9a90647f1bc4290c0fed600cac0fd3883ee570
```

Any source edit after this lock requires a v2 or a new FTD ID.

---

## 3. Fixed Protocol

Shared window:

| Quantity | Value |
|---|---:|
| Lattice size | `L = 32` |
| Separations | `r = {5, 7, 9}` |
| Analytic gate | `alpha_r(r,L) = 2 r G_L(r)` |
| Relative tolerance | `10%` |

Mode A: production live-tick estimator.

| Quantity | Value |
|---|---:|
| Ticks | `300` |
| SOR iterations | `100` |
| `coupling` | `false` |
| `coulomb_charge_coupling` | `1.0` |
| Other physics toggles | disabled by leak guard |

Mode B: matched-stencil static projector.

| Quantity | Value |
|---|---:|
| Projector | `ftd::eft::matched_gauss_project` |
| Tolerance | `1e-10` |
| Max iterations | `400` |
| Initial particle flux | `0.0` |
| Coupling | native unit only |

The matched projector already exists as EFT tooling. This campaign does not
introduce a new physics rule.

---

## 4. Outcomes

### Outcome M: `MATCHED_ESTIMATOR_CONFIRMED_PRODUCTION_GATE_INVALID`

Criteria:

- Production live-tick mode fails the absolute Phase-G gate.
- Matched-stencil static mode passes the absolute Phase-G gate.

Interpretation:

FTD-0285 failed because it used the production live-tick Gauss path as an
absolute estimator. Future alpha readout tests must use a matched estimator or
must prove that the production path has converged to the same observable.
No alpha claim is promoted.

### Outcome E: `ENERGY_FUNCTIONAL_MISMATCH`

Criteria:

- Production live-tick mode fails the absolute Phase-G gate.
- Matched-stencil static mode also fails the absolute Phase-G gate.
- Matched projection itself converges.

Interpretation:

The issue is deeper than the production Gauss stencil. The field-energy
interaction estimator is not paired with the analytic Green-function
normalization in this finite-cell protocol. Future work must switch observable
or derive the correct finite-cell energy normalization. No alpha claim is
promoted.

### Outcome B: `PRODUCTION_AND_MATCHED_GATES_CONFIRMED`

Criteria:

- Both modes pass the absolute Phase-G gate.

Interpretation:

This would contradict FTD-0285's observed failure unless the current engine or
artifact behavior changed. Treat as a reproducibility warning and audit the
FTD-0285 lock/run before drawing physics conclusions.

### Outcome C: `MATCHED_PROJECTOR_FAILED_TO_CONVERGE`

Criteria:

- Matched projection reports non-convergence.

Interpretation:

The v2 artifact is invalid for the estimator question. No claim is promoted.

---

## 5. Banned Moves

- No coupling scan.
- No changing `L`, `r`, tolerance, ticks, SOR, or matched-projector tolerance
  after seeing output.
- No adding a Postulate-W arm in this v1. The point is estimator validation,
  not alpha matching.
- No continuum extrapolation claim.
- No promotion or demotion of FTD-0013 from this run.

---

## 6. Commands

Build:

```sh
cmake --build engine/build --config Release --target campaign_alpha_estimator_validation --parallel 24
```

Run of record, after this file is committed and tagged:

```sh
ctest --test-dir engine/build -C Release -R "^alpha_estimator_validation$" --output-on-failure
```

The run is valid only if:

```sh
git rev-list -n1 preregister-alpha-estimator-validation-v1
```

resolves before execution.

---

## 7. Prior

Prior-favored outcome: Outcome M.

Reason: existing EFT tooling documents a stencil mismatch in the production
Gauss projection, while `matched_poisson.h` was built specifically to provide
the Ward/alpha measurement projector. If Outcome E lands instead, the next
obstruction is the field-energy observable, not the projection.
