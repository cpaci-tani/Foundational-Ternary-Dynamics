# ANALYSIS: Alpha Estimator Validation v2

**FTD ID:** FTD-0286 (v2)
**Date:** 2026-06-13
**Status:** [MEASUREMENT ANALYSIS -- HALF ENERGY GATE CONFIRMED MATCHED]
**Pre-registration:** `preregistrations/PREREG_ALPHA_ESTIMATOR_VALIDATION_v2.md`
**Artifact:** `engine/tests/campaign_alpha_estimator_validation_v2.cpp`
**Gate helper:** `engine/include/ftd/eft/lattice_coulomb_gate.h`

---

## 1. Run Of Record

Command:

```sh
ctest --test-dir engine/build -C Release -R "^alpha_estimator_validation_v2$" --output-on-failure
```

Direct executable (same payload):

```sh
engine/build/Release/campaign_alpha_estimator_validation_v2.exe
```

---

## 2. Frozen Output

```text
FTD-0286 alpha estimator validation v2
protocol,L,32,r_values,5|7|9,production_ticks,300,production_sor,100,matched_tol,1e-10,matched_max_iter,400,rel_tol,0.1
scope,estimator_validation_not_alpha_recovery
energy_convention,half_sum_j2
gate,alpha_r_expected,r_G_L_r
leak_guard,production_coupling,false,production_charge_coupling,1.0
mode,production_live_tick,projection_converged,true,worst_deep_vacuum_after,0
row,production_live_tick,5,0.0392130644178632,0.0458913088499133,0.145523076142613
row,production_live_tick,7,0.0333856940608977,0.0324996945519469,0.0272617795694884
row,production_live_tick,9,0.0155202364541491,0.0204936372394081,0.242680239098576
summary,production_live_tick,mean_alpha,0.02937299831097,mean_phase_g,0.0329615468804228,mean_ratio,0.8796861547761,mean_rel_err,0.138488364936892,max_rel_err,0.242680239098576,absolute_gate,false
mode,matched_static_projector,projection_converged,true,worst_deep_vacuum_after,3.05175804230584e-05
row,matched_static_projector,5,0.045978753632865,0.0458913088499133,0.00190547589822875
row,matched_static_projector,7,0.0325540134504217,0.0324996945519469,0.00167136643047728
row,matched_static_projector,9,0.0205467370781383,0.0204936372394081,0.00259104023896959
summary,matched_static_projector,mean_alpha,0.0330265013871417,mean_phase_g,0.0329615468804228,mean_ratio,1.00205596085589,mean_rel_err,0.00205596085589188,max_rel_err,0.00259104023896959,absolute_gate,true
verdict,HALF_ENERGY_GATE_CONFIRMED_MATCHED
```

---

## 3. Verdict

Frozen outcome: `HALF_ENERGY_GATE_CONFIRMED_MATCHED`.

| Criterion | Result |
|---|---:|
| Matched-stencil absolute gate (v2) | **pass** (max rel err 0.26%) |
| Production live-tick absolute gate (v2) | fail (max rel err 24%) |
| Matched projection converged | pass |

---

## 4. Interpretation

FTD-0286 v1's `ENERGY_FUNCTIONAL_MISMATCH` was caused by pairing
`energy_audit().field_energy = ½ Σ|J|²` with the legacy gate
`α_r = 2 r G_L(r)` (valid only for `Σ|J|²`). Re-pairing to
`α_r = r G_L(r)` closes the matched arm to sub-percent precision
(mean ratio 1.002, max rel err 0.26%).

Production live-tick Gauss still undershoots by ~12% mean ratio (0.88) with
per-r spread up to 24% — the known 18-point Laplacian / 6-point divergence
stencil mismatch documented in `matched_poisson.h`. That is a separate
production-path issue, not an observable-definition failure.

**Theorem/code drift note:** `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` §2 still
derives against `Σ|J|²`. The engine accumulator changed in
`diagnostics_compute.cpp` (2026-04-27). Future Phase-G citations must state
which convention is paired.

This analysis does not promote, demote, or reclassify `x_+ = 1/alpha`.
It does not support a dynamical-alpha claim. It says:

1. Matched-stencil + half-energy gate is the canonical finite-cell estimator.
2. FTD-0285 no-alpha discriminator v2 should use this pairing before rerun.
3. Production-path convergence to the matched readout remains `[OPEN]`.

---

## 5. Claim Status

- FTD-0286 v1: `[MEASUREMENT ANALYSIS -- ENERGY FUNCTIONAL MISMATCH]` (unchanged).
- FTD-0286 v2: `[MEASUREMENT ANALYSIS -- HALF ENERGY GATE CONFIRMED MATCHED]`.
- FTD-0285: remains `[INVALIDATED PROTOCOL]` until v2 discriminator rerun.
- FTD-0013 / MC-T4.3: unchanged. No α derivation obtained.
