# FTD-0612 — Uniform single-core stationary refinement v1

**Status:** `[MEASURED — STABLE COMPACT REST STATE CONSTRUCTIVE]`
**Protocol:**
[`PREREG_UNIFORM_SINGLE_CORE_STATIONARY_REFINEMENT_v1.md`](../preregistrations/PREREG_UNIFORM_SINGLE_CORE_STATIONARY_REFINEMENT_v1.md),
prefix SHA-256 `B0C93907D5EEB6BE96ED9BA485E2BC452E6180FE619533052A2D870C73B52002`
**Production status:** unchanged

One full Newton step in the unchanged FTD-0611 basin lowers the energy by
`1.58e-17` and reduces the fourth-order gradient from `1.44e-8` to
`3.94e-14`. The refined Hessian retains nine positive modes with
`lambda_min=0.00100079`, and all 18 signed perturbations raise energy.

The direct field and cubic translation controls pass below `1.78e-15`. A
64-tick zero-momentum history followed by 64 state-only inverse ticks reports
zero centre displacement, zero centre-momentum change, zero energy drift,
worst common-action residual `1.98e-14`, and recovery `3.20e-17`.

Therefore the selected three-constituent action admits a stable, exactly
stationary compact rest state under external uniform periodic neutralization.
This is an existence result for the selected research dynamics. It does not
derive the constituents, binding, charge, neutralizer, a physical particle,
or production ontology. FTD-0613 supplies its separately locked boost test.
