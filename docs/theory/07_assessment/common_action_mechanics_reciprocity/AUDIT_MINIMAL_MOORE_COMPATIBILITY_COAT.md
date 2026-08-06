# Audit — Minimal Moore compatibility coat (FTD-0577)

**Date:** 2026-07-26  
**Verdict:**
`MINIMAL_MOORE_COAT_RESTORES_LOCAL_CENTRAL_CONTINUITY_NONCARDINAL_SELECTED`

## Findings

1. **The FTD-0576 obstruction has one minimal separable radius-one escape.**
   Symmetry, unit normalization, and checkerboard cancellation uniquely fix
   axial weights `(1/4,1/2,1/4)` in the registered class.

2. **The escape is a 27-site Moore coupling coat.** Its exact center, face,
   edge, and corner weights are `1/8`, `1/16`, `1/32`, and `1/64`. It is
   positive, normalized, centered, translation covariant, and cubic
   covariant.

3. **Primitive ternary manifestation is preserved but cardinal coupling is
   abandoned.** The site still stores only `s=+/-1`; fractional values occur
   only in the deterministic coupling sidecar. At integer remainder the
   manifested site carries coupling weight `1/8`, not `1`.

4. **The exact face current now has a finite-range native central-current
   image.** The local map
   `Q_i=((1+T_i^-1)/2) product_(j!=i) B_j K_i` satisfies
   `d_c Q=B_M d_f K` identically.

5. **The resulting central continuity equation is exact in actual 3D
   straight-segment histories.** Across 36 registered path arms the worst
   residual is `9.02e-17`.

6. **The construction is genuinely local and geometric.** Density support is
   27--64 sites and current support is 18--56 sites on both `L=17` and `L=33`.
   Translation and proper-cubic covariance residuals are at binary64 roundoff.

7. **The FTD-0576 conditional total-energy identity now has a local current
   witness.** Four independent fixtures close the full field, interaction,
   and matter ledger to `4.34e-18`.

8. **This does not close reciprocal mobile matter.** No action has yet selected
   the coat as the matter coupling, no matched gather has converted its work
   into the production dispersion, and no self-force or implicit inverse has
   been proved.

9. **The static FTD-0575 defect survives.** The coat is regular with unit
   infrared limit. It multiplies the already finite static kernel and cannot
   restore a Coulomb `1/k^2` pole or reverse the same-sign attraction.

10. **Production remains unchanged.** No toggle, scenario, force, field
    source, movement rule, or default was added.

**Successor disposition (FTD-0578):** the common reciprocal action is now
derived, but its compact carrier is Peierls-pinned and its diagonal
time-centering is incompatible with the FTD-0576 endpoint-energy source. The
unmodified point-action mobility branch is closed negative.

## Reproducibility

- theorem:
  `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_MINIMAL_MOORE_COMPATIBILITY_COAT.md`
- preregistration:
  `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_MINIMAL_MOORE_COMPATIBILITY_COAT_v1.md`
- preregistration SHA-256:
  `94C706936189B077A144ACA7B64D4FEBE93DCDB93AEA36BA604C466480C80F8D`
- native observer: `test_minimal_moore_compatibility_coat`
- independent proof:
  `scripts/proofs/proof_minimal_moore_compatibility_coat.py`
- run record: `engine/results/ftd_0577/windows_msvc_cpu.json`
- production changed: no
