# Audit — Native Hodge energy and central-continuity obstruction (FTD-0576)

**Date:** 2026-07-26  
**Verdict:**
`NATIVE_HODGE_ENERGY_IDENTITY_CENTRAL_LOCAL_MOBILE_CURRENT_OBSTRUCTED`

## Findings

1. **The driven production field tick has an exact work identity.** For any
   prescribed kick `S`, its FTD-0574 invariant changes by
   `<S,(W_0+W_1)/2>`.

2. **The exact work coordinate is uniquely staggered.** Within `R=J-cW`, only
   `c=1/2` gives `delta R=(W_0+W_1)/2` for arbitrary field and source data.
   A constant source therefore preserves `H-<S,R>` exactly.

3. **A conditional common-energy identity exists.** If endpoint density and
   integrated current obey `delta rho+D Q=0` under the native central
   divergence, then field work, `-G_C<rho,D R>` interaction energy, and matter
   work `G_C<Q,GD R_bar-C delta R>` sum to zero exactly.

4. **The frozen cardinal hop cannot supply the required current locally.** A
   one-site hop requires `Q(z)=-2z/(z+1)`. The checkerboard pole makes this
   non-finite-range.

5. **Even periodic volumes have no solution.** The hop has checkerboard
   overlap of magnitude two while the central divergence vanishes at that
   mode.

6. **Odd periodic volumes conceal nonlocality rather than curing it.** The
   registered exact solution occupies all `L` sites and reaches radius
   `(L-1)/2`; alternative constant-null-mode representatives remain
   box-spanning.

7. **The exact face current cannot be locally converted into the native site
   current.** Divergence commutation uniquely requires the projection symbol
   `2/(z+1)`, which has the same checkerboard pole.

8. **The mobile common-action branch remains closed for the frozen minimal
   variables.** Exact energy itself is not the obstruction. The incompatible
   conjunction is exact energy, native central operators, cardinal hopping,
   and finite-range locality. A face-field dynamics, staggered primitive,
   nonlocal current, or genuinely different carrier is an enlarged model.

9. **FTD-0577 supplies the noncardinal local carrier representation but does
   not invalidate this obstruction.** Its Moore coat cancels the checkerboard
   factor by spreading coupling weight. The cardinal endpoint representation
   tested here remains impossible; the successor still lacks force,
   self-force, and mobile-action closure.

## Reproducibility

- theorem:
  `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_NATIVE_HODGE_ENERGY_CONTINUITY.md`
- preregistration:
  `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_NATIVE_HODGE_ENERGY_CONTINUITY_v1.md`
- preregistration SHA-256:
  `98B3F8D13E6FBAAD26931C6DD7EC37C9377BD054899012B109C63A0512C26E78`
- native observer: `test_native_hodge_energy_continuity`
- independent proof:
  `scripts/proofs/proof_native_hodge_energy_continuity.py`
- run record: `engine/results/ftd_0576/windows_msvc_cpu.json`
- production changed: no
