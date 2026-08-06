# Audit — FTD-0614 refined-core Peierls landscape

**Status:** `[AUDIT — POSITIVE PATH BARRIERS; INTERNAL BRANCH HYSTERESIS; PROPER COVARIANCE]`
**Verdict:** `REFINED_CORE_PEIERLS_LANDSCAPE_NUMERICALLY_UNRESOLVED`

- protocol prefix SHA-256: `D4095014...D82`;
- runner: `engine/tests/test_refined_core_peierls_landscape.cpp`;
- certificate: `scripts/proofs/proof_refined_core_peierls_landscape.py`;
- independent checks: 19/19 pass;
- run of record: `engine/results/ftd_0614/`.

All path endpoints, threshold identities, common-action gates, cyclic
whole-state comparisons, and inverses close.  Positive locally relaxed
selected-path barriers span `1.13027e-4...1.57932e-4`.  The locked unique-path
gate fails because four transverse paths have forward/backward local-minimum
hysteresis `1.34316e-4`; both axial paths agree at machine precision.

The correct rotated-state comparator passes at `1.78e-15`, closing the
FTD-0613 symmetry-bookkeeping defect.  The compact model is cubic-covariant
under the tested proper cyclic rotations while a fixed oriented body remains
anisotropic.

The result does not establish a global minimum-energy path, a universal
depinning speed, a physical particle, or a new primitive.  It establishes
that centre-only point-particle mechanics is insufficient for this selected
compact core: the already explicit internal constituent state affects the
available translation branch.

