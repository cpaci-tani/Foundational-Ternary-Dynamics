# Audit — Native Hodge reciprocity and static-pole cancellation (FTD-0575)

**Date:** 2026-07-26  
**Verdict:**
`NATIVE_HODGE_FORCE_DERIVED_STATIC_POLE_CANCELED_SAME_SIGN_ATTRACTIVE`

## Findings

1. **The exact FTD-0574 source action has a reciprocal matter variation.**
   Writing `Phi_J=-G_C div J` and `A_J=G_C curl J` converts the interaction to
   `rho(-Phi_J)+j dot A_J`. A point path therefore receives
   `q(E_J+v cross B_J)` from the same functional that generates the field
   source.

2. **The Lorentz form is algebraic, not yet electromagnetic.** The derived
   fields are `E_J=G_C grad div J-G_C partial_t curl J` and
   `B_J=G_C curl curl J`. They are not the engine's legacy visual/force
   identifications. Magnetic scalar work is exactly zero and the homogeneous
   identities hold, but those facts alone do not supply Coulomb behavior.

3. **Both ends of static exchange are derivative coupled.** The static
   charge and transverse-current response is controlled by
   `R(k)=3 sum_i sin(k_i)^2/M(k)`, not by `1/M(k)`.

4. **The static massless pole cancels exactly.** With
   `u_i=1-cos(k_i)`, the identity
   `M-sigma^2=Q/3+sum_(i<j)(u_i-u_j)^2/3>=0` proves `0<=R<=3`.
   The registered principal-direction limits all approach three. There is no
   `1/k^2` static Coulomb singularity.

5. **The static polarity sign is reversed relative to electromagnetism.**
   Eliminating the field gives
   `V_eff=-(G_C^2/2)<rho,R rho>`. Equal polarities attract and opposite
   polarities repel in this channel. The locked numerical arms reproduce the
   sign with cross energies `-0.021058840860053937` and
   `+0.021058840860053937`.

6. **Soft radiative coupling also decouples.** The on-shell source residue
   carries `G_C^2 sigma^2=O(k^2)`. Finite-frequency waves remain admissible,
   but the soft limit does not support a long-range photon claim.

7. **The selected Gauss/Poisson force is not derived by this action.** Any
   long-range Coulomb response currently observed with projection or Poisson
   toggles comes from those separately selected mechanisms. It cannot be
   cited as reciprocal closure of the FTD-0574 action.

8. **FTD-0576 resolves the energy algebra and closes local cardinal mobility.**
   Exact common energy follows conditionally from central continuity, but a
   cardinal hop requires a nonlocal central current and no finite-range
   face-to-native projection exists. A stable carrier therefore still needs a
   staggered, face-native, nonlocal, nonlinear, or otherwise enlarged route.

## Reproducibility

- theorem:
  `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_NATIVE_HODGE_RECIPROCITY_STATIC_POLE.md`
- preregistration:
  `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_NATIVE_HODGE_RECIPROCITY_STATIC_POLE_v1.md`
- preregistration SHA-256:
  `BE33049A5C93E887574BDE5509E93F666150A5CAF02E2B93989D96980D1788F6`
- native observer: `test_native_hodge_reciprocity`
- independent proof:
  `scripts/proofs/proof_native_hodge_reciprocity_static_pole.py`
- run record: `engine/results/ftd_0575/windows_msvc_cpu.json`
- production changed: no
