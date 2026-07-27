# Audit — Common Moore Worldline Action (FTD-0578)

**Date:** 2026-07-26  
**Verdict:**
`COMMON_MOORE_WORLDLINE_ACTION_DERIVED_ENERGY_CENTERING_MISMATCH_PEIERLS_PINNED`

## Findings

1. **FTD-0577 has an exact spacetime completion.** Temporal hats split the
   coated density and current into endpoint records satisfying
   `D_c Q_0=rho_0-T` and `D_c Q_1=T-rho_1` without a new primitive variable.

2. **The aggregate current is not a new fitted construction.** All 104
   registered path arms reconstruct the FTD-0577 current within `1.39e-17`.

3. **One selected action generates both source and probe coupling.** Direct
   deposition and an independently evaluated orbit-side adjoint gather agree
   within `4.32e-18`. Reciprocal gather is therefore derived for this coated
   coupling rather than appended as a second force law.

4. **This does not yet produce a production particle equation.** The action
   is written in the FTD-0576 work coordinate `R=J-W/2`; no implicit matter
   solve, hop rule, toggle, or scenario evaluates it.

5. **The time-exact action and exact endpoint-energy ledger disagree on
   generic diagonal motion.** The coated squared mismatch is exactly
   `1/1536` for edge diagonals and `5/3072` for body diagonals, while it
   vanishes for axial hops.

6. **The compact carrier has a nonzero self-force.** The same common action
   produces `V_self=V_0+C_i r(1-r)` with positive `C_i`; it pins the carrier
   toward integer sites and is invariant under polarity reversal.

7. **The barrier is not a finite-volume artifact over the registered pair of
   volumes.** The minimum `C_i` is `2.6961904613504844e-4` and the minimum
   half-cell barrier is `6.740476153376211e-5` on `L=17,33`.

8. **Exact energy conservation would preserve, not cancel, this barrier.** A
   conservative Peierls potential can exchange kinetic and field energy
   exactly while still preventing low-energy continuous translation.

9. **The unmodified compact point action is closed as free mobile matter.**
   Both the diagonal centering defect and Peierls pinning independently block
   promotion to FTD-0481.

10. **The viable successor is narrower than “add a force.”** Remaining routes
    are a registered energy-centered multistage variational transaction,
    integer hopping, or a genuinely extended native excitation whose relative
    Peierls barrier scales away. Post-hoc self-field subtraction is not
    licensed.

11. **Production remains unchanged.** No force branch, movement ordering,
    toggle, default, scenario, or renderer was modified.

12. **FTD-0579 closes finite rigid extension as the proposed exact cure.**
    Every nonzero finite rigid carrier retains both diagonal centering and a
    positive Peierls barrier; smooth finite envelopes suppress only.

13. **FTD-0580 closes the centering defect constructively outside the rigid
    trilinear class.** A positive endpoint chord and symmetric face routing
    are exactly centered, but the Peierls barrier survives.

## Reproducibility

- theorem:
  `docs/theory/10_eft_program/derivations/THEOREM_COMMON_MOORE_WORLDLINE_ACTION.md`
- preregistration:
  `docs/theory/10_eft_program/preregistrations/PREREG_COMMON_MOORE_WORLDLINE_ACTION_v1.md`
- preregistration SHA-256:
  `DE4F20274E679F0C0E39967B985025F85D5D6F56A1D142B86CE6DE603A62019B`
- native observer: `test_common_moore_worldline_action`
- independent exact proof:
  `scripts/proofs/proof_common_moore_worldline_action.py`
- run record: `engine/results/ftd_0578/windows_msvc_cpu.json`
- production changed: no
