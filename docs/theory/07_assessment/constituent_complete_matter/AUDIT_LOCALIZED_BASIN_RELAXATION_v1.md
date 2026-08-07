# AUDIT — Localized-basin relaxation v1

**Date:** 2026-07-28  
**Identifier:** `FTD-0678`  
**Status:** `[EXECUTION INVALID — NO DYNAMICS SAMPLED]`  
**Verdict:** `LOCALIZED_BASIN_RELAXATION_EXECUTION_INVALID`  
**Preregistration:**
[`PREREG_LOCALIZED_BASIN_RELAXATION_v1.md`](../../10_eft_program/preregistrations/constituent_complete_matter/PREREG_LOCALIZED_BASIN_RELAXATION_v1.md)

## Result

The locked runner stopped during initialization before tick zero was recorded.
Both signs produced the same values:

```text
p_max                         2.5e-7
canonical target energy       6.9965761811730761e-13
quotiented core phase metric  1.3993146260354155e-12
relative identity residual    4.3606986038625706e-7
locked maximum                1e-8
```

The parent, observer preflight, and bitwise-equal initial-field gates passed.
The CSV contains only its header.  No forward step, fit, field-shell history,
or inverse history exists, so v1 has no physical verdict about matter.

## Defect in the preregistered identity

The locked equation `D_phase(0)=2 E_target(0)` equated different quotients.
`D_phase` removes the common constituent momentum by construction; the target
doublet coordinate does not apply that demeaning operation.  A mode with any
finite collective component therefore need not satisfy the equality, even
when both observers are implemented correctly.  Loosening the `1e-8`
tolerance would conceal the category error.  The identity is withdrawn as an
execution gate.

A corrected protocol may require positivity and sign symmetry of the initial
quotiented metric, while retaining the target energy as a separate diagnostic.
It may not reinterpret the v1 mismatch as dynamics.

## Reproducibility

- protocol SHA256:
  `3876E9CBF7017E68426C26E6829D4751513F183D3CA3EB6536C73E0718FFD156`;
- runner SHA256:
  `2B99AA2B1F72628FEC8503EA65ADE57C81126857A138E43311F97846A6FF3D81`;
- executed Release binary SHA256:
  `93407DCF4732FC392FB4CC0E35D86ED68F1AEA9AA31F37023B213028D696B578`;
- result JSON SHA256:
  `63348AEA5319376F1D2800ED1BB4B1CD7BEFA55C269C0FCB8E7A990706A733BE`;
- header-only CSV SHA256:
  `298D0F86CA7A9C5150726013FC09BFBF77A1C28BE9E5CB681EDD2CD23CA6AF18`.

No production state or behavior changed.
