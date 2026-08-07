# AUDIT — Accelerated quadratic-coat spacetime current

**Date:** 2026-07-26  
**Identifier:** `FTD-0548`  
**Status:** `[DERIVED — EXACT DEPOSIT IDENTITIES] + [NUMERICAL FACT — LOCKED
QUADRATURE]`  
**Verdict:** `ACCELERATED_COAT_CURRENT_EXACT_LINEAR_SPLIT_REJECTED`  
**Pre-registration:**
[`PREREG_ACCELERATED_COAT_SPACETIME_CURRENT_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_ACCELERATED_COAT_SPACETIME_CURRENT_v1.md)  
**Derivation:**
[`DERIV_ACCELERATED_COAT_SPACETIME_CURRENT.md`](../../10_eft_program/derivations/common_action_mechanics_reciprocity/DERIV_ACCELERATED_COAT_SPACETIME_CURRENT.md)  
**Run of record:** `engine/results/ftd_0548/windows_msvc_cpu.json`

## Result

```text
registered arms                       144
worst total-current residual           2.1885271372923398e-14
worst split-recombination residual     2.0816681711721685e-17
worst temporal-partition residual      4.4408920985006262e-16
worst split-continuity residual        1.3988810110276972e-14
worst gauge-endpoint residual          1.9151347174783950e-15
worst reversal residual                4.1633363423443370e-15
largest linear-split difference        0.0016542884780729739
failures                               0
```

The total oriented face current remains the endpoint-only FTD-0541 current,
as required by path reparameterization invariance. The temporal occupation
and endpoint-weighted current split change measurably. Exact continuity and
gauge covariance require the accelerated deposits.

## Scope

This closes reuse of the linear-time FTD-0542 split for accelerated matter.
It constructively supplies the current in the uniform-force integrable
monotone subsector, but it does not yet solve the self-consistent neutral
transaction. Within-tick momentum reversal fails closed because it is outside
the registered single-segment knot decomposition.
No production state, force, phase, toggle, scenario, or default changed.

## Reproducibility

- test: `test_accelerated_coat_spacetime_current`, 144 arms, failures `0`;
- preregistration SHA256:
  `B77EF420ED42CD82C1269F34D9D3C6299106C0C63FA1B805AFFEFA7A0A0DBBBC`;
- header SHA256:
  `0D1DFA5863322245C003A06622650E96B63EC4D4EBEF585C8E894654D99E9F4B`;
- source SHA256:
  `2F457E5E13B2F682D7D3B266536A073143AD4598AAB48DA76889C57C2DBDFD8F`;
- test SHA256:
  `579D40B37080B49FFBE5694F7BED6A97179DAB8F1B1F4229624A08D31AF89824`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
