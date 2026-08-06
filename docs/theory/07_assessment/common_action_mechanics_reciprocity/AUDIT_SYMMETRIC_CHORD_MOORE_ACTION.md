# Audit — Symmetric Chord Moore Action (FTD-0580)

**Date:** 2026-07-26  
**Verdict:**
`SYMMETRIC_CHORD_CLOSES_MOORE_CENTERING_PEIERLS_PINNING_REMAINS`

## Findings

1. **The FTD-0578 diagonal centering defect is representation-dependent.** It
   disappears when the diagonal coupling density is the energy-centered
   endpoint chord instead of a tensor-product trilinear orbit.

2. **Positivity makes the energy-centered chord shape unique under the
   registered conditions.** A nonnegative time history whose average has
   support only at the two endpoints cannot carry weight anywhere else; the
   first moment then fixes weights `1-t,t`.

3. **Diagonal face routing need not choose an x/y/z order.** Uniformly
   averaging all shortest monotone paths yields exact subset-flow weights and
   zero divergence at every intermediate vertex.

4. **The route is selected, not absolutely unique.** Permutation symmetry
   fixes it inside the monotone-shortest-path class. Divergence-free curls and
   longer paths remain mathematically available.

5. **The Moore coat preserves exact continuity.** All 104 raw and central
   path arms pass; the maximum central residual is `1.39e-17`.

6. **The common temporal action is now exactly energy-centered in every
   direction.** `T_0+T_1=(rho_0+rho_1)/2` and `Q_0=Q_1=q/2`; the split residual
   is below `6.94e-18`.

7. **The repair does not produce a gapless particle.** Every nonzero Moore
   displacement retains a positive Peierls coefficient and a finite
   half-cell depinning barrier.

8. **The barrier is polarity-even and cubic-covariant.** Polarity residual is
   zero at printed precision; 24 proper rotations agree within `5.43e-19`.

9. **The construction changes coupling morphology.** During a diagonal hop,
   polarity weight occupies only the two endpoint sites before coating. It is
   not the FTD-0478 straight trilinear density and must not be silently
   substituted into production.

10. **The next gate is dynamical rather than kinematic.** A live branch must
    show that existing nonlinear `(s,J,W)` dressing supplies an internal mode
    that traverses or cancels the Peierls barrier without a fitted
    counterforce or new hidden route state.

    **FTD-0581 successor:** stable passive dressing cannot do so. Only a
    finite-excitation, phase-resolved native traversal mechanism remains open.

11. **Production remains unchanged.** No shape, force, movement phase,
    toggle, default, scenario, renderer, or primitive state was modified.

## Reproducibility

- theorem:
  `docs/theory/10_eft_program/derivations/THEOREM_SYMMETRIC_CHORD_MOORE_ACTION.md`
- preregistration:
  `docs/theory/10_eft_program/preregistrations/PREREG_SYMMETRIC_CHORD_MOORE_ACTION_v1.md`
- preregistration SHA-256:
  `E3B651CA2E4D05395DA876DA61B873A11E6E5BD17220CDC70EB055F944527DF3`
- native observer: `test_symmetric_chord_moore_action`
- independent exact proof: `scripts/proofs/proof_symmetric_chord_moore_action.py`
- run record: `engine/results/ftd_0580/windows_msvc_cpu.json`
- production changed: no
