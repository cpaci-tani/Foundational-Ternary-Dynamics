# Pre-registration — Native common-mode work-port ownership discriminator v1

**Identifier:** `FTD-0986`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

FTD-0985 proves that separated same-tick work needs a complete local
phase/action owner or an equivalent canonical work field. FTD-0965 proves
that the dual production fields have one unused scalar canonical pair per
site after a regional frame is chosen. Does that unused capacity already
contain a native, positive, locally closed work port, or does it supply only
the coordinates while a new ownership/protection law remains to be priced?

The discriminator separates five claims:

1. **storage:** whether an existing dual-field pair can be selected without
   adding continuous state;
2. **covariance:** whether a body-derived regional frame makes the pair a
   signed-cubic scalar rather than a global-axis insertion;
3. **canonical work chart:** whether the selected pair has a positive
   action-angle chart and can realize the FTD-0982 seam debit;
4. **production ownership:** whether unchanged CPU/CUDA dynamics preserve a
   local port sector and book its energy/current without double counting; and
5. **minimum price:** if ownership fails, whether a selected local projector
   clutch suffices without adopting a seventh continuous pair.

No engine or production mutation is authorized.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md` | `FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C` |
| `THEOREM_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md` | `100A5539A1116FD6BEC5ABF2B7CE7BA2C32DDA557564EC7C964CDF5877512739` |
| `THEOREM_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md` | `C5C28405CA439BF2341D545F99E9BDFC985BF65155B1CD49075541CD5C258462` |
| `THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md` | `1CF020D3AA4EB78746C8CF7B932B3AB27E265E173E7F81524CF2A4547A38FA91` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/render_bridge_diagnostics.h` | `5A9525591D3D818377E4688FBE4A57229B5CB7C36E62FF07D76941D814D57F69` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/diagnostics_compute.cpp` | `C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |
| `engine/cuda/kernels_stencil_dual.cu` | `25365B176BB333009333E2B5A596F792E2245719D107E754CE3C6BF5BAE9F1C0` |
| `engine/cuda/kernels_aux.cu` | `E385FCFC93A2188E094798FC3A2C0A0839A6139313D738EE2E69254C6921739C` |

## 3. Native common/relative regional chart

On a regular neutral-body frame `F=(e1,e2,e3)`, define for each component
`a=1,2,3`

\[
 q_{\pm,a}={e_a\cdot(J_L\pm J_R)\over\sqrt2},\qquad
 p_{\pm,a}={e_a\cdot(P_L\pm P_R)\over\sqrt2}.           \tag{1}
\]

The certificate must prove that (1) is a six-pair symplectic transform. Under
every signed-cubic transformation `Q`, the body theorem gives `e_a'=Qe_a`,
so all dot products in (1) are invariant regional scalars. Under the
production `L/R` swap,

\[
 (q_{+,a},p_{+,a})\mapsto(q_{+,a},p_{+,a}),\qquad
 (q_{-,a},p_{-,a})\mapsto-(q_{-,a},p_{-,a}).            \tag{2}
\]

Choose the unused pair as the longitudinal common mode

\[
 c_5=(Q,P):=(q_{+,3},p_{+,3}).                           \tag{3}
\]

The other five whole pairs remain available for the clock and four exchange
modes. This is a repacking of existing capacity, not a seventh pair.

## 4. Positive action-angle chart and seam scaling

On the punctured plane `I>0`, use

\[
 Q=\sqrt{2I}\cos\theta,\qquad
 P=-\sqrt{2I}\sin\theta.                                \tag{4}
\]

The certificate must prove

\[
 dQ\wedge dP=d\theta\wedge dI,\qquad
 I={Q^2+P^2\over2},                                      \tag{5}
\]

and time reversal `(Q,P)->(Q,-P)` sends `theta->-theta` and leaves `I`
fixed, modulo the usual angular chart transition.

The observable longitudinal field and wave components are `sqrt(2)Q` and
`sqrt(2)P`. Their production amplitude-norm contribution is therefore

\[
 E_{\rm obs,3}={1\over2}(\sqrt2Q)^2
               +{1\over2}(\sqrt2P)^2=2I.               \tag{6}
\]

For target work `Delta H=H-H'`, the port action debit is consequently

\[
 I'=I+{\Delta H\over2}.                                  \tag{7}
\]

The seam-family phase derivative must be scaled by one half relative to the
unit-energy FTD-0982 port. The exact certificate must show that the extended
map remains symplectic and preserves `H+2I` at the registered crossing.

Equation (7) is defined only when `I>0` and `I+Delta H/2>=0`. Crossing the
origin needs another angular chart or must fail closed.

## 5. Production-ownership discriminator

The current production common mode is not presumed to be a closed reserve.
The frozen source audit must retain that:

- CPU and CUDA apply the C18 Laplacian componentwise to both `L` and `R`;
- matter coupling and the imposed clock source drive both substrates;
- phase write advances every component and may damp it;
- observable `flux` and `wave_vel` are the `L+R` sums;
- weak transmutation swaps all `L/R` coordinate and momentum components;
- the split `E_L/E_R` channels are diagnostics, while accounted energy uses
  the observable sums; and
- the energy audit explicitly says its amplitude norm is not the
  gradient-plus-cross Hamiltonian of the production wave tick.

Thus swap invariance of (3) is necessary but not sufficient. Outcome A
requires an unchanged-production source that isolates the mode, assigns its
boundary current, books switching work and reserve, and preserves a complete
inverse. Absence of those sources forces Outcome B or C.

## 6. Compact closed-mode no-go

Let `K` be the nonconstant translation-invariant C18 stiffness and let `u`
be a finitely supported proposed regional mode. If unchanged free production
closed the mode, then for some scalar `lambda`

\[
 Ku=\lambda u.                                           \tag{8}
\]

In Laurent representation, equation (8) is

\[
 [k(z)-\lambda]U(z)=0.                                   \tag{9}
\]

The Laurent polynomial ring is an integral domain, `k-lambda` is nonzero,
and a finitely supported nonzero `u` has nonzero Laurent polynomial `U`.
Therefore (9) is impossible. The certificate must establish:

\[
 \boxed{\text{unchanged C18 propagation has no nonzero compactly
 supported closed eigenmode.}}                           \tag{10}
\]

This does not prohibit an open port exchanging current with its boundary. It
prohibits calling a finite local mode an autonomous protected oscillator
without a current ledger or protection law.

## 7. Minimum selected protection candidate

For a normalized selected regional mode `u`, set

\[
 \mathsf P=uu^T,\qquad \mathsf P_\perp=I-\mathsf P,       \tag{11}
\]

and define

\[
 K_{\rm iso}=\mathsf P K\mathsf P+
              \mathsf P_\perp K\mathsf P_\perp.         \tag{12}
\]

If `K` is positive semidefinite, (12) is positive semidefinite and obeys
`P_perp K_iso P=0`: the chosen mode and complement are exactly decoupled.
For the existing ternary orientation latch `ell in {-1,0,+1}`, the reference
clutch family

\[
 K_\ell=K-\ell^2(\mathsf P K\mathsf P_\perp+
                  \mathsf P_\perp K\mathsf P)            \tag{13}
\]

uses `ell^2` for isolation while the sign of `ell` remains available for
orientation. Switching `ell->ell'` changes stiffness energy by

\[
 W_{\rm switch}={1\over2}q^T(K_{\ell'}-K_\ell)q.         \tag{14}
\]

Equation (14) must be booked in a reciprocal controller/work ledger. Equations
(11)--(14) are a **selected reference candidate**, not a claimed production
derivation. If the support exceeds one Moore cone, the switch must be compiled
into causal substeps; finite support alone does not prove one-tick locality.

## 8. Frozen checks

- **W1:** protocol and all twelve source hashes;
- **W2:** exact six-pair common/relative symplectic transform, regional scalar
  covariance, swap parity, and five-plus-one repacking;
- **W3:** exact polar canonical chart, positive action, and time reversal;
- **W4:** observable energy coefficient `2`, half-scaled seam reaction,
  extended symplecticity, energy closure, positivity, and inverse domain;
- **W5:** exact compact-eigenmode Laurent obstruction;
- **W6:** exact projector isolation, positivity-by-block restriction,
  ternary-square clutch, switching work, and retained orientation sign;
- **W7:** CPU/CUDA source-locked propagation, source, damping, swap, diagnostic,
  and energy-accounting census;
- **W8:** no unchanged-production port ownership/protection/current/inverse;
- **W9:** no new continuous field, production, `G*`, Born/Bell, Hilbert, mass,
  selector-energy, or completeness promotion and no numerical search.

No fit, numerical near-miss search, parameter scan, or engine mutation is
permitted.

## 9. Frozen classifier

- **Outcome A — native autonomous port:** the common regional pair is
  canonical, positive, swap-stable, compactly closed, and already owned and
  energy/current-audited by unchanged production.
- **Outcome B — native chart / priced ownership law:** existing dual fields
  supply the covariant canonical pair and positive seam representation, so no
  seventh continuous pair is forced; unchanged production does not protect or
  own it as a local reserve. A selected projector/current clutch with switching
  work, history, inverse, and causal compilation remains to be adopted or
  derived.
- **Outcome C — adopt a new work field:** even the regional common-mode chart
  or positive canonical seam representation fails, so an additional complete
  pair is required.
- **Outcome D — invalid:** any frozen hash, identity, source, scope, or
  integrity gate fails.

The expected result is Outcome B. It does not license production integration
or claim that the selected protection candidate forms naturally.
