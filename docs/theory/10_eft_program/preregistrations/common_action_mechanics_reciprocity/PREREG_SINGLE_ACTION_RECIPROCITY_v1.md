# PRE-REGISTRATION — Single-action reciprocity v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0467`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0466`  
**Engine artifact:** `engine/tests/campaign_single_action_reciprocity.cpp`

**Locked campaign SHA-256:**
`2EB51724924C07631FBDBE2C218E1D3B02144DB0398B1E5BFA5FB251AE94A6AF`

## 1. Question

Does any current production electric-force branch act as the matter-side
variation of the same native interaction whose field-side variation supplies
the coupling source `-G_C grad(s)`?

The registered interaction is

`L_int = +G_C sum_x s(x) div(J)(x)`.

With the periodic central-difference operators used by the engine, its exact
field variation is `-G_C grad(s)`. Its point-probe spatial variation at fixed
field is `+G_C s grad(div J)`. This campaign tests those operator, sign, and
normalization statements independently of fitted physical targets.

## 2. Frozen fixtures

- CPU `RenderBridge`, `L=17`, periodic boundary, center probe, zero initial
  velocity, all unrelated toggles disabled;
- signs `s=+1,-1` and axes `x,y,z`;
- fixed amplitude `a=1e-3` and affine background `J0=0.1`;
- no parameter, stencil, sign, or normalization scans.

Three fixtures are registered:

1. **Adjoint/source fixture.** One manifested polarity at the center. Compare
   the production coupling-only one-tick `wave_vel` increment with
   `-G_C grad(s)`, and verify periodic summation by parts
   `sum J dot[-G_C grad(s)] = G_C sum s div(J)` on a fixed deterministic
   vector field.
2. **Quadratic fixture.** Along each axis `i`, set
   `J_i=a r_i^2`, other components zero. At the center,
   `grad(div J)=2a e_i` while the tier-2 `grad|J|` is zero.
3. **Affine fixture.** Along each axis `i`, set
   `J_i=J0+a r_i`, other components zero. At the center,
   `grad(div J)=0` while the tier-2 `grad|J|=a e_i`.

The quadratic and affine fields need only be polynomial on the radius-two
neighborhood read by the force operators. Their periodic boundary
discontinuities are outside every registered stencil.

## 3. Production branches and controls

- **Action prediction:** `F_action=+G_C s grad(div J)`.
- **Published helper:** record `coupling_force(s,grad(div J))` exactly as
  implemented in `lagrangian.h`.
- **Legacy J branch:** invoke the actual `phase_forces_main_loop` with
  `emergent_forces=false`, `poisson_coulomb=false` and read `f_coulomb`.
- **Emergent-density branch:** invoke the actual loop with
  `emergent_forces=true` and read `f_coulomb`.
- **Poisson branch structural control:** hold the polarity configuration
  fixed, change the prescribed `J` fixture, solve the production Poisson
  potential, and compare the resulting forces. Exact independence from `J`
  classifies this as a separate auxiliary-potential force rather than the
  fixed-`J` variation of `L_int`.

Each measured production branch must first reproduce its coded analytical
formula to `1e-12`; otherwise the protocol is invalid.

## 4. Gates

- source increment and periodic adjoint residuals `<=1e-12`;
- coded-formula controls `<=1e-12`;
- polarity oddness and axis covariance residuals `<=1e-12`;
- a branch is a single-action partner only if it matches `F_action` in both
  quadratic and affine fixtures for every sign and axis to `1e-12`;
- the published `coupling_force` helper is assessed separately because it is
  not the branch currently executed by `phase_forces_main_loop`.

## 5. Locked classifications

- `NATIVE_SINGLE_ACTION_RECIPROCITY_FOUND`: at least one production branch
  passes every action-variation fixture and normalization gate;
- `NO_PRODUCTION_FORCE_BRANCH_IS_NATIVE_SINGLE_ACTION_PARTNER`: all branch
  controls are valid, but none passes the common-action gates;
- `PROTOCOL_INVALID`: any source, adjoint, formula-replay, covariance, or
  finiteness control fails.

## 6. Interpretation boundary

Failure does not prove that no reciprocal matter-field action can be written.
It proves that the current production branches do not instantiate the
registered native interaction as one common action. The Poisson control is not
called wrong merely for using an auxiliary potential; it is classified as a
separate selected mechanism. No production tick or ontology is modified.

## 7. Execution record

All source, summation-by-parts, formula-replay, polarity, and direct-branch
axis-covariance controls pass. The source residual is exactly zero and the
periodic adjoint residual is `2.17e-19`. The legacy, emergent-density, and
Poisson branches all fail the common-action gate. Locked verdict:

`NO_PRODUCTION_FORCE_BRANCH_IS_NATIVE_SINGLE_ACTION_PARTNER`.
