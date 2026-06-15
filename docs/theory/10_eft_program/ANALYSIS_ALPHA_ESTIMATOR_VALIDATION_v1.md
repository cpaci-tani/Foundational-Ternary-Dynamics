# ANALYSIS: Alpha Estimator Validation v1

**FTD ID:** FTD-0286
**Date:** 2026-06-13
**Status:** [MEASUREMENT ANALYSIS -- ENERGY FUNCTIONAL MISMATCH]
**Pre-registration:** `archive/superseded/PREREG_ALPHA_ESTIMATOR_VALIDATION_v1.md` (archived 2026-06-15)
**Lock tag:** `preregister-alpha-estimator-validation-v1`
**Lock commit:** `7cf8bd5caab26a6c66a78be3b1270c571ebbbf97`
**Artifact:** `engine/tests/campaign_alpha_estimator_validation.cpp`
**Artifact SHA256:** `dce6018d4ccc7565c1bab6870c9a90647f1bc4290c0fed600cac0fd3883ee570`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^alpha_estimator_validation$" --output-on-failure
```

CTest result:

```text
Test #225: alpha_estimator_validation ....... Passed   30.96 sec
100% tests passed, 0 tests failed out of 1
```

The full console payload was recovered from `engine/build/Testing/Temporary/LastTest.log`.

---

## 2. Frozen Output

```text
protocol,L,32,r_values,5|7|9,production_ticks,300,production_sor,100,matched_tol,1e-10,matched_max_iter,400,rel_tol,0.1
scope,estimator_validation_not_alpha_recovery
leak_guard,production_coupling,false,production_charge_coupling,1.0
mode,production_live_tick,projection_converged,true,worst_deep_vacuum_after,0
row,production_live_tick,5,0.0392130644178632,0.0917826176998266,0.572761538071306
row,production_live_tick,7,0.0333856940608977,0.0649993891038937,0.486369110215256
row,production_live_tick,9,0.0155202364541491,0.0409872744788163,0.621340119549288
summary,production_live_tick,mean_alpha,0.02937299831097,mean_phase_g,0.0659230937608455,mean_ratio,0.43984307738805,mean_rel_err,0.56015692261195,max_rel_err,0.621340119549288,absolute_gate,false
mode,matched_static_projector,projection_converged,true,worst_deep_vacuum_after,3.05175804230584e-05
row,matched_static_projector,5,0.045978753632865,0.0917826176998266,0.499047262050886
row,matched_static_projector,7,0.0325540134504217,0.0649993891038937,0.499164316784761
row,matched_static_projector,9,0.0205467370781383,0.0409872744788163,0.498704479880515
summary,matched_static_projector,mean_alpha,0.0330265013871417,mean_phase_g,0.0659230937608455,mean_ratio,0.501027980427946,mean_rel_err,0.498972019572054,max_rel_err,0.499164316784761,absolute_gate,false
verdict,ENERGY_FUNCTIONAL_MISMATCH
```

---

## 3. Verdict

Frozen outcome: `ENERGY_FUNCTIONAL_MISMATCH`.

Criteria check:

| Criterion | Result |
|---|---:|
| Production live-tick absolute gate | fail |
| Matched-stencil static absolute gate | fail |
| Matched projection converged | pass |

The matched projector did converge, so this is not a projector-convergence
failure. The production live-tick estimator reproduces the FTD-0285 native
ratio (`0.43984307738805`). The matched projector improves the structure but
lands at mean ratio `0.501027980427946`, with maximum relative error
`0.499164316784761`, far outside the frozen `10%` gate.

---

## 4. Interpretation

FTD-0285 did not fail only because of the production Gauss stencil. The
field-energy difference observable used by the frozen protocol is not paired
with the analytic `2 r G_L(r)` normalization in this finite-cell setup. The
matched result is close to a half-normalized readout, but no post-hoc factor is
introduced here.

This campaign does not promote, demote, or reclassify `x_+ = 1/alpha`. It does
not support a dynamical-alpha claim. It says the next alpha-readout attempt must
first derive or directly measure the correct finite-cell observable: potential
response, source-response work, or a proven energy normalization. **Update
(2026-06-13): FTD-0286 v2 shows the v1 failure was a ½-prefactor gate pairing
error; see `ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v2.md`.** Only after
that may the no-alpha-vs-Postulate-W discriminator be asked again.

---

## 5. Claim Status

- FTD-0284 remains a locked discriminator, not a measurement.
- FTD-0285 remains `[INVALIDATED PROTOCOL]`.
- FTD-0286 v1 closes as `[MEASUREMENT ANALYSIS -- ENERGY FUNCTIONAL MISMATCH]`.
- FTD-0286 v2 (`ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v2.md`) resolves the v1
  pairing error: matched projector passes the half-energy gate; production
  live-tick still fails on stencil drift. **No alpha derivation obtained.**
