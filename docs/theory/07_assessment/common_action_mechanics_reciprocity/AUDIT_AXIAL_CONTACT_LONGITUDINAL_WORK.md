# AUDIT — Axial contact longitudinal work

**Date:** 2026-07-25  
**Identifier:** `FTD-0530`  
**Status:** `[PRE-REGISTERED NONZERO-WORK HYPOTHESIS REJECTED]` +
`[THEOREM — POINTWISE NET CURRENT NULL ON THE REGISTERED AXIAL QUOTIENT]` +
`[THEOREM + MEASURED — FIXED-PATH AXIAL RECIPROCITY]` +
`[OPEN — ASYMMETRIC/DISTINGUISHABLE CONTACT]`  
**Verdict:** `AXIAL_ELASTIC_CONTACT_IS_RECIPROCAL_ON_FIXED_PATH`  
**Pre-registration:**
[`PREREG_AXIAL_CONTACT_LONGITUDINAL_WORK_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_AXIAL_CONTACT_LONGITUDINAL_WORK_v1.md)  
**Run of record:** `engine/results/ftd_0530/windows_msvc_cpu.json`

## 1. The preregistered nonzero-work prediction is false

FTD-0529 left 72 curl-free axial histories outside its transverse obstruction.
FTD-0530 preregistered the prediction that their Gauss-fixed longitudinal work
would nevertheless be nonzero. It is zero.

The stronger exact result is

```text
K_pair=K_left+K_right=0 face by face,
rho_after-rho_before=0 site by site.
```

For the registered equal-polarity, equal-speed, zero-COM axial pair, the two
unlabelled histories traverse the same axial support in opposite directions.
Their oriented face currents cancel pointwise. The endpoint fractional density
is also unchanged. This is not merely `C^T K=0`; the complete deposited source
is null.

## 2. Consequences for work and Gauss

Because `K_pair=0`, every compatible matched field obeys

```text
E_after=E_before,
Delta H_field=0,
W=<K_pair,E_mid>=0.
```

The unchanged elastic rebase already has `Delta H_matter=0`. Exact total energy
therefore closes without a longitudinal impulse. Adding either a transverse
curl field or a constant harmonic field cannot change this result.

The deterministic routed baseline has large cancelling contributions, so its
floating-point field-energy difference is `1.89e-15..7.66e-15`; the algebraic
current and density arrays themselves are bitwise zero in every arm. The
largest inferred momentum correction is `3.06e-14`, also roundoff.

## 3. Registered results

All 72 arms cover both polarities, all six face directions, speeds `1/8` and
`1/4`, and three translations:

```text
worst history residual                    0
worst continuity residual                 0
worst ||C^T K||^2                         0
worst harmonic current                    0
worst ||K||^2                             0
worst endpoint-density change             0
worst transverse work change              0
worst harmonic work change                0
worst absolute Gauss                      6.1487056765563430e-14
worst midpoint-energy identity            6.1617377866696188e-14
maximum apparent elastic energy defect    7.6605388699135801e-15
```

The preregistered gates demanding a defect above `1e-10` and a nonzero required
impulse failed. The preregistered alternative verdict applies.

## 4. Scope

FTD-0530 does not undo FTD-0529. The geometry now separates exactly:

- face-normal identical symmetric contact has zero aggregate current and is
  invisible to the matched field on this fixed path;
- edge/corner contact has a nonzero loop-current component and cannot retain
  the unchanged elastic output in arbitrary admissible fields.

No general collision law follows. Unequal speeds, nonzero COM motion,
distinguishable attributes, opposite polarities, external fields that alter
the trajectories, and simultaneous field-dependent endpoints remain open.
The next constructive target is the self-consistent edge/corner solve, not an
axial force added where the exact aggregate source is zero.

No production code, default, toggle, scenario, force, collision rule, phase
order, field ontology, normalization, or tolerance changed.

## 5. Reproducibility

- classification checks: `7/7 PASS` after explicitly rejecting the locked
  nonzero-work hypothesis;
- test SHA256:
  `8252913BAD3803427AEBDD05BECCA62E209FC8EB90FAFCD3651BBADDE5297EAC`;
- header SHA256:
  `34A4E5CAE8828F22DFA55FAAE67E7AE611FDB0DE953AEB47EDEEEF3C4C3E6E06`;
- implementation SHA256:
  `77E3C4B659EDF859EB0019EF99580DBCFA97BD4E91172F69FCE84DCF1F6A4554`;
- locked preregistration SHA256:
  `2B61DD31DF9E020488DEA6087C3036DB122646FB3E1D61147FD514FD0266AE77`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.

