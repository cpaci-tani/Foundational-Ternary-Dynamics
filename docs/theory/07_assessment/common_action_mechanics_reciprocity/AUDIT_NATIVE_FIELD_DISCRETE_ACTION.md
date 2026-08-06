# Audit — Native field discrete action and source-operator boundary (FTD-0574)

**Date:** 2026-07-26  
**Verdict:**
`NATIVE_FIELD_DISCRETE_ACTION_DERIVED_MAGNETIC_SOURCE_ACTION_MISMATCH`

## Findings

1. **The source-free production field tick has an exact local discrete
   action.** Its discrete Euler--Lagrange equation is exactly the frozen
   kick-drift update. This is stronger than the prior observation that its
   transfer matrix is symplectic.

2. **Production `wave_vel` is the exact discrete Legendre momentum.** The
   standard `(J,W)` form assumed by FTD-0570--0573 is therefore native in the
   free field sector. Genesis remains noncanonical relative to that native
   form; no bath law is supplied here.

3. **The exact modified tick energy is independently recovered and uniquely
   normalized.** With onsite `W-W=I`, the one-mode invariant space is spanned
   by `[[a,-a/2],[-a/2,1]]`. The position-space form is the existing
   gradient-plus-cross invariant, not the amplitude norm used by older energy
   diagnostics.

4. **An exact autonomous continuous generator has a nonlocality price.** Its
   shadow-Hamiltonian multiplier is `mu(a)=theta/sin(theta)`, a nonpolynomial
   function with a finite complex branch point. No fixed finite-range
   translation-invariant continuous generator reproduces all exact ticks,
   although the discrete action itself is finite-range.

5. **The coded moving source is variational only under a different interaction
   from the one documented.** The exact prescribed-source functional is

   ```text
   G_C <s,div J> + G_C <curl J,s v>.
   ```

   Its variation is the coded `-G_C grad(s)+G_C curl(sv)`.

6. **The documented onsite velocity interaction fails as a source action.**
   `-G_C <sv,J>` varies to `-G_C sv`. For uniform `s=1` and constant nonzero
   `v`, production codes zero source while the documented term gives magnitude
   `G_C=0.08542454310285437`. All eight locked counterexamples reproduce the
   exact mismatch.

7. **The reciprocal variation is classified by FTD-0575.** It produces a
   Lorentz-form Hodge force with induction and `curl curl J`, but its derivative
   vertices cancel the static massless pole and give same-polarity attraction.
   It is not the optional production `alpha q v cross curl J` rule and is not
   Coulomb electromagnetism. Exact finite-step common energy and mobile matter
   remain open.

## Corrections required

- `engine/include/ftd/lagrangian.h` must describe its velocity term as a
  selected matter-side diagnostic, not the origin of the coded curl source.
- `compute_el_residual` must be described as a production field-equation replay
  in the moving-source sector, not an Euler--Lagrange residual of the written
  six-term action.
- `engine/SPEC_ENGINE.md` must cite the correct prescribed-source functional
  and state that it is not yet a common dynamic action.
- the optional Lorentz-force comment must not imply that its normalization and
  the coded moving source come from one action.

These are epistemic/comment corrections only. The production update, source,
force, toggles, event order, RNG, and defaults remain unchanged.

## Reproducibility

- preregistration:
  `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_NATIVE_FIELD_DISCRETE_ACTION_v1.md`
- preregistration SHA-256:
  `09970E8A18974B56F399DC68023BD7527FDCED50A937054413C3FC53B7F1AFEB`
- native observer: `test_native_field_discrete_action`
- independent proof: `scripts/proofs/proof_native_field_discrete_action.py`
- run record: `engine/results/ftd_0574/windows_msvc_cpu.json`
- production changed: no
