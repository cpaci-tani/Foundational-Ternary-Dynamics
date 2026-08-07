# AUDIT — Matched midpoint Poynting identity

**Date:** 2026-07-26  
**Identifier:** `FTD-0544`  
**Status:** `[THEOREM — EXACT AUXILIARY FIELD ENERGY AND GAUSS TRANSPORT]`  
**Verdict:** `MATCHED_MIDPOINT_POYNTING_EXACT`  
**Pre-registration:**
[`PREREG_MATCHED_MIDPOINT_POYNTING_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_MATCHED_MIDPOINT_POYNTING_v1.md)  
**Theorem:**
[`THEOREM_MATCHED_MIDPOINT_POYNTING.md`](../../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_MATCHED_MIDPOINT_POYNTING.md)  
**Run of record:** `engine/results/ftd_0544/windows_msvc_cpu.json`

## Result

The matched midpoint Maxwell step transfers exactly

```text
Delta U_field=-<Ebar,K>
```

for every registered quadratic-coat current. Fifteen stationary, polarity,
axial/diagonal, crossing, periodic, and reversal arms passed:

```text
midpoint reconstruction             4.3368086899420177e-18
Ampere/Faraday update                2.7755575615628914e-17
curl-adjoint identity                4.4892746204477918e-18
Poynting/work identity               1.042048392019268e-14
Gauss/continuity transport           8.3266726846886741e-17
polarity work mirror                 1.865174681370263e-14
reversal work mirror                 1.865174681370263e-14
```

The field side of exact energy is therefore constructive. The remaining
energy gate is wholly explicit: the action-derived matter update must satisfy
`Delta H_matter=<Ebar,K>` using the production dispersion. That has not yet
been proved or solved.

No production state, default, phase, energy normalization, force, toggle,
scenario, or tolerance changed.

## Reproducibility

- test: `test_matched_midpoint_poynting`, `15` arms, failures `0`;
- preregistration SHA256:
  `D431E624B0D9AB476F4F56BBDB2F64134DD394C497F0ADC3EDCC6114CE343D88`;
- test SHA256:
  `62973DEBB509B2B92F95527D3F373E0365056215BCEC89367DF6FEA3CA35E4DD`;
- header SHA256:
  `1F9C85C03155349B2F6A67E6BB121B9E395FA82CC09FF3B246C31C3E490C6212`;
- source SHA256:
  `4489C987C44318959821533208170AD8FDF066DC5F1F0CA608989672AEA8807C`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
