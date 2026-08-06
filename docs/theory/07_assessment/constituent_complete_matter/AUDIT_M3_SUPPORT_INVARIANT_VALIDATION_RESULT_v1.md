# FTD-0755 — Support-invariant matter-family validation result v1

**Status:** `[CLOSED — VALIDATION INFRASTRUCTURE UNRESOLVED; NO PHYSICS VERDICT]`  
**Date:** 2026-07-30  
**Protocol:** `PREREG_M3_SUPPORT_INVARIANT_VALIDATION_v1.md`  
**Certificate:** `scripts/proofs/proof_m3_support_invariant_validation.py`

## Verdict

The locked certificate returns

```text
FTD-0755 artifact: 311/311 checks
verdict=M3_VALIDATION_INFRASTRUCTURE_UNRESOLVED
```

All nine candidate arms and all three causal-fibre arms were executed exactly
once. They produced the required twelve CSV/JSON pairs, but every arm stopped
before a tick-160 parent checkpoint was initialized. Every candidate records
`small_initialized = large_initialized = 0` and every fibre records
`baseline_initialized = 0` at both volumes. No hostile candidate, nested-volume
comparison, or remote causal fibre was therefore subjected to dynamics.

This is not `M3_FINITE_TIME_FAMILY_CLOSED_NEGATIVE`. It supplies no evidence
that the state-only matter predicate either persists or fails to persist.

## Scope of the failure

The initial finite-support preparation itself remains independently valid in
the established FTD-0753 record: at tick 160 all three rays are graph-inside
and have negative pair energy. The follow-on frozen FTD-0756 replay localizes
the new failure to the validation wrapper rather than silently treating an
empty candidate matrix as physics evidence.

FTD-0755 is consumed and cannot be rerun, repaired, or reinterpreted. Any
corrected validation requires a new identifier and must first reproduce the
FTD-0753 parent through tick 160 with the corrected observer transaction.

## Artifacts and invariance

- protocol SHA-256:
  `1E713DB4B997DAED0D55F098A6E7D63FC0F2D773391CE44FFE03AADD92A504BC`;
- 24 registered files under `engine/results/ftd_0755/`;
- independent certificate: `311/311`;
- all result metadata records `held_out_validation = true` and
  `dynamics_changed = false`.

Production defaults, established CUDA dynamics, scenarios, and ontology were
unchanged.
