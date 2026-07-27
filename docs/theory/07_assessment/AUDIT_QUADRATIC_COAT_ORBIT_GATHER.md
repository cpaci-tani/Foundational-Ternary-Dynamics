# AUDIT — Quadratic-coat orbit gather

**Date:** 2026-07-26  
**Identifier:** `FTD-0550`  
**Status:** `[SELECTION — QUADRATIC COAT] + [THEOREM — ELECTRIC ADJOINT AND SPLINE-CURL COMMUTATION] + [NUMERICAL FACT — LOCKED CAMPAIGN]`  
**Verdict:** `QUADRATIC_COAT_ORBIT_GATHER_CONSTRUCTIVE`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_ORBIT_GATHER_v1.md`](../10_eft_program/preregistrations/PREREG_QUADRATIC_COAT_ORBIT_GATHER_v1.md)  
**Derivation:**
[`DERIV_QUADRATIC_COAT_ORBIT_GATHER.md`](../10_eft_program/derivations/DERIV_QUADRATIC_COAT_ORBIT_GATHER.md)  
**Run of record:** `engine/results/ftd_0550/windows_msvc_cpu.json`

## Result

```text
registered arms                       72
worst electric-adjoint residual       3.469446951953614e-18
curl-commutation residual             1.691355389077387e-17
worst magnetic scalar-work residual   2.710505431213761e-20
worst kinematic residual              8.049116928532385e-16
maximum axial transverse force        0.0470346
worst polarity residual               0
worst reversal residual               1.387778780781446e-17
translation residual                  2.168404344971009e-17
rotation residual                     1.734723475976807e-18
failures                              0
```

The exact FTD-0541 current and the quadratic face reconstruction are adjoints.
The matched discrete curl commutes with the quadratic face/edge
reconstruction by the exact `B2'` difference identity. Axial paths retain a
finite nonzero transverse electric gather, eliminating the earlier
zero-displacement component ambiguity.

## Verdict and scope

The representation/gather layer is constructive. The old trilinear midpoint
electric and magnetic interpolants are no longer needed for the research
branch. This does not rescue the already closed FTD-0539 common-action branch
and does not itself establish a self-consistent mobile law. The next gate is
an implicit relativistic discrete-gradient transaction using these exact
gathers and solving its path stages with the fields.

No production state, force, phase, toggle, scenario, or default changed.

## Reproducibility

- test: `test_quadratic_coat_orbit_gather`, 72 arms, failures `0`;
- preregistration SHA256:
  `0E085F240318D93D54F2D1AC7CA820298B1C3551386E170C1713D5EFE136714A`;
- header SHA256:
  `0B2F9CDADC5DA390A86FBE5FE37BC23B3178B0EBCD7E67C3302CDD3CAA87BBB3`;
- source SHA256:
  `45CE7C88B1BFBCE58F95DD9590275A9985E6A6DA887D5E6BDFEDF9728C0A277C`;
- test SHA256:
  `F2F1E2EE9E1FB00A054E297F14FF392DCBD4F10367ECB5579BECFEEA74B5E292`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
