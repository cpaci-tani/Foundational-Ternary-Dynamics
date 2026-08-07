# AUDIT — Endpoint schedule underdetermination

**Date:** 2026-07-26  
**Identifier:** `FTD-0549`  
**Status:** `[THEOREM — ENDPOINT/MIDPOINT INSUFFICIENCY]`  
**Verdict:** `ENDPOINT_DATA_DO_NOT_DETERMINE_SPACETIME_CURRENT`  
**Pre-registration:**
[`PREREG_ENDPOINT_SCHEDULE_UNDERDETERMINATION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_ENDPOINT_SCHEDULE_UNDERDETERMINATION_v1.md)  
**Theorem:**
[`THEOREM_ENDPOINT_SCHEDULE_UNDERDETERMINATION.md`](../../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_ENDPOINT_SCHEDULE_UNDERDETERMINATION.md)  
**Run of record:** `engine/results/ftd_0549/windows_msvc_cpu.json`

## Result

```text
registered arms                    96
worst endpoint residual            0
worst endpoint/mid derivative      0
worst total-current residual       2.2204460492503138e-17
worst split-recombination residual 3.3480163086352378e-17
worst analytic-moment residual     1.7130394325270970e-17
worst reversal residual            1.7130394325270970e-17
smallest monotonicity margin        0.90377495513506234
largest split difference           0.003333333333333334
failures                           0
```

The two polynomial schedules have the same endpoints and the same first
derivative at the start, midpoint, and end. The total oriented face current is
also identical. Exact coat moment identities nevertheless separate the
temporal occupation and endpoint current split by `q d epsilon/30`.

## Verdict and scope

Post-processing an endpoint solve cannot supply the FTD-0548 spacetime source.
The next constructive object must be a genuinely atomic path/force/current/
field solve with internal stages. Those stages can be derived numerical
unknowns; this result does not promote them to primitive ontology and does not
close such a solve negative.

No production state, force, phase, toggle, scenario, or default changed.

## Reproducibility

- test: `test_endpoint_schedule_underdetermination`, 96 arms, failures `0`;
- preregistration SHA256:
  `4D766E9DD345B02221F8D3C00D3649F4BA42E55C6C5D7F6AF9A1E6515784AE3A`;
- header SHA256:
  `E79E3C58D43F159868E0AA9EB7BC16ECABBA75C9BA2E03DE67D80D922D8D91F4`;
- source SHA256:
  `97276011AB2DC1264CB360697016CBEA58C5831DAF45AD8F6C207212DFC0BF3B`;
- test SHA256:
  `14E4FC27DEA960E965C9F63A5A3ED6E9AB6FAA52D03AB6166E19F453D3D81490`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
