# Theorem — Native common-mode work pair and production ownership boundary v1

**Identifier:** `FTD-0987`  
**Date:** 2026-08-12  
**Status:** `[THEOREM, CONDITIONAL — EXISTING COVARIANT COMMON-MODE PAIR]` +
`[THEOREM — POSITIVE ACTION-ANGLE / HALF-SCALED SEAM WORK LAW]` +
`[THEOREM — NO NONZERO COMPACT CLOSED C18 MODE]` +
`[CLOSED NEGATIVE — UNCHANGED PRODUCTION AUTONOMOUS WORK PORT]` +
`[SELECTION — PROJECTOR OWNERSHIP CLUTCH / SWITCHING WORK]` +
`[OPEN — NATIVE PROTECTION / CURRENT / FORMATION / PRODUCTION]`

## Result

The local work-port programme does **not** require a seventh continuous
canonical pair.

On a regular neutral-body frame, the existing dual fields contain the
longitudinal common pair

\[
 \boxed{
 Q={e_3\cdot(J_L+J_R)\over\sqrt2},\qquad
 P={e_3\cdot(P_L+P_R)\over\sqrt2}.}                     \tag{1}
\]

Equation (1) is a complete canonical pair, a regional signed-cubic scalar,
and invariant under the production `L/R` swap. On `I>0` it has the exact
positive action-angle chart

\[
 Q=\sqrt{2I}\cos\theta,\qquad
 P=-\sqrt{2I}\sin\theta,qquad
 dQ\wedge dP=d\theta\wedge dI.                          \tag{2}
\]

The observable amplitude-norm contribution of this mode is `2I`. Therefore
the FTD-0982 seam can use the existing pair with the half-scaled action law

\[
 \boxed{I'=I+{H(z)-H(z')\over2}},
 \qquad H(z')+2I'=H(z)+2I.                              \tag{3}
\]

The phase-dependent lift remains exactly symplectic.

What production lacks is **ownership**, not storage. Every existing dual
component is propagated, sourced, optionally clock-driven, damped, and
included in the observable sum. No nonzero compactly supported mode is closed
under the unchanged C18 stiffness. The current energy audit also explicitly
uses an amplitude norm rather than the gradient-plus-cross Hamiltonian of the
wave tick. Thus production does not yet isolate, protect, current-audit, or
invert a local reserve transaction.

The minimum remaining ontic price is consequently a selected local
ownership/protection law and its reciprocal switching ledger—not another
continuous field type.

## Certificate of record

- Parent protocol:
  [`PREREG_NATIVE_COMMON_MODE_WORK_PORT_OWNERSHIP_DISCRIMINATOR_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_COMMON_MODE_WORK_PORT_OWNERSHIP_DISCRIMINATOR_v1.md),
  SHA-256 `7E5E00C9262D3E6AF5D2BBD41D7F2845D4744D902157C32BADA7F6787D86AECF`.
- Immutable parent proof:
  [`proof_native_common_mode_work_port_ownership.py`](../../../../../scripts/proofs/proof_native_common_mode_work_port_ownership.py),
  SHA-256 `88B3296231CAFA4F98E7778B82BE00538D9E66BD3EC54BE1E628F0E1EFBD5DD3`.
- First execution: every reached mathematical gate passed; one over-specific
  source marker failed and SymPy rejected a symbolic exponent in `coeff`.
- Repair protocol:
  [`PREREG_NATIVE_COMMON_MODE_WORK_PORT_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_COMMON_MODE_WORK_PORT_CERTIFICATE_REPAIR_v2.md),
  SHA-256 `FBEA4B287636CFEBF0D29C4B8A14B1FFB0E880F11A6EF03D8803426D9EAB7D7A`.
- Repair wrapper:
  [`proof_native_common_mode_work_port_ownership_v2.py`](../../../../../scripts/proofs/proof_native_common_mode_work_port_ownership_v2.py),
  SHA-256 `5C1F446D34BE5AAC05A0E20A9E4825145E1FF26E8E9AF03CEC07D1611A538CBD`.
- Final execution: inherited `96/96`, repair integrity `13/13`,
  **Outcome B**.
- Frozen source census: four theorem sources and eight CPU/CUDA production
  sources.
- Production mutation: none.

## 1. Six existing pairs and the invariant repacking

For every body-frame axis `a`, define

\[
 q_{\pm,a}={e_a\cdot(J_L\pm J_R)\over\sqrt2},\qquad
 p_{\pm,a}={e_a\cdot(P_L\pm P_R)\over\sqrt2}.           \tag{4}
\]

For one axis the transformation from
`(q_L,p_L,q_R,p_R)` to `(q_+,p_+,q_-,p_-)` is

\[
 T={1\over\sqrt2}
 \begin{pmatrix}
 1&0&1&0\\0&1&0&1\\1&0&-1&0\\0&1&0&-1
 \end{pmatrix}.                                         \tag{5}
\]

It obeys

\[
 T^TT=I,qquad T^T\Omega_4T=\Omega_4,qquad\det T=1.     \tag{6}
\]

The direct sum over three axes is therefore a full six-pair symplectic
repacking. Five whole pairs can hold the clock and four exchange modes while
equation (1) is the sixth. No half pair is split and no pair is counted twice.

For every signed-cubic matrix `R`, FTD-0969 gives `e_a'=Re_a` and the polar
fields obey `J'=RJ`, `P'=RP`. Hence

\[
 (Re_a)\cdot(RJ)=e_a\cdot J,                             \tag{7}
\]

so equation (4) is a regional scalar chart, not a preferred global Cartesian
axis.

The production exchange sends

\[
 (q_+,p_+,q_-,p_-)\mapsto(q_+,p_+,-q_-,-p_-).           \tag{8}
\]

The common port is fixed. Choosing an `R`-only component would not have this
property and would collide with another owned pair after a weak swap.

## 2. Action, time reversal, and reserve domain

Equation (2) has unit Jacobian determinant and pulls the Cartesian symplectic
form back exactly to `dtheta wedge dI`. Canonical time reversal sends

\[
 (Q,P)\mapsto(Q,-P),\qquad
 (\theta,I)\mapsto(-\theta,I).                           \tag{9}
\]

Thus the work action is time-even while its phase orientation is reversed.
No complex or Hilbert-space coordinate is imported.

The origin `I=0` is not an action-angle chart. A transaction is admissible
only on

\[
 I>0,qquad I+{H-H'\over2}\ge0.                         \tag{10}
\]

It must fail closed or change chart at the boundary. This is the same finite
ready-domain principle as FTD-0982, with the energy normalization made
explicit.

## 3. Why the work debit is divided by two

The observable longitudinal components are

\[
 e_3\cdot(J_L+J_R)=\sqrt2Q,qquad
 e_3\cdot(P_L+P_R)=\sqrt2P.                              \tag{11}
\]

Consequently their exact production amplitude norm is

\[
 {1\over2}(\sqrt2Q)^2+{1\over2}(\sqrt2P)^2
 =Q^2+P^2=2I.                                           \tag{12}
\]

For the FTD-0982 seam family, scale the two local shears' phase derivative by
one half while leaving the registered crossing map `R_sigma` unchanged. If

\[
 B_0=G-R_\sigma^TGR_\sigma,                              \tag{13}
\]

then the new family obeys

\[
 R^T\Omega\partial_\theta R\big|_*= {B_0\over2}.        \tag{14}
\]

The canonical action reaction is

\[
 \Delta I={1\over2}z^T{B_0\over2}z
          ={H(z)-H(R_\sigma z)\over2},                  \tag{15}
\]

which proves equation (3). The full Jacobian preserves
`Omega+dtheta wedge dI`, and the inverse recovers the target state before
subtracting the same action debit.

Equation (12) is an exact identity in the current **amplitude-norm audit**.
The source itself warns that this audit is not the production wave
Hamiltonian. Therefore equations (3) and (12) prove representability and
normalization, not an already-running production energy transaction.

## 4. No compact autonomous mode in unchanged C18 propagation

Let a finitely supported regional profile `u` satisfy the closed-mode
condition

\[
 Ku=\lambda u.                                           \tag{16}
\]

Its Laurent transform is a Laurent polynomial `U(z)`, while the C18
stiffness has a nonconstant Laurent symbol `k(z)`. Equation (16) becomes

\[
 [k(z)-\lambda]U(z)=0.                                   \tag{17}
\]

The Laurent ring is an integral domain. Since `k-lambda` and `U` are both
nonzero, equation (17) is impossible. Equivalently, on a one-axis restriction
the coefficient at the upper extremal exponent is the nonzero product of the
outer stiffness coefficient and the upper coefficient of `U`; it cannot
cancel.

Hence

\[
 \boxed{\text{no nonzero compactly supported C18 mode is an autonomous
 production oscillator.}}                               \tag{18}
\]

A local common mode may be an **open** port exchanging current with adjacent
field modes. But then boundary current, replenishment, admission, and inverse
must be explicit. They are not supplied by the word “unused.”

## 5. Source-locked production boundary

The unchanged CPU and CUDA sources establish all of the following:

- both `L` and `R` coordinates have complete wave-velocity momenta;
- the C18 Laplacian advances every component independently;
- matter coupling and the imposed de Broglie-clock term drive both fields;
- phase write advances and may damp every coordinate/momentum component;
- observable fields are the `L+R` sums;
- weak transmutation swaps both coordinates and momenta, under which the
  common pair is invariant;
- `E_L`, `E_R`, `wv_L`, and `wv_R` are split diagnostics; and
- accounted energy uses the observable fields and explicitly disclaims being
  the gradient-plus-cross wave Hamiltonian.

No frozen production source defines a work-port owner, regional projector,
isolation clutch, switching-work ledger, positive port reserve, boundary
current, or inverse transaction. Unchanged production as an autonomous local
work port is therefore closed negative.

## 6. Minimum selected ownership/protection candidate

Let `u` be a normalized regional common-mode profile and set

\[
 \mathsf P=uu^T,qquad \mathsf P_\perp=I-\mathsf P.       \tag{19}
\]

The block-restricted stiffness

\[
 K_{\rm iso}=\mathsf PK\mathsf P+
              \mathsf P_\perp K\mathsf P_\perp          \tag{20}
\]

is symmetric, positive semidefinite whenever `K` is, and satisfies

\[
 \mathsf P_\perp K_{\rm iso}\mathsf P=0.                \tag{21}
\]

It exactly decouples the proposed port and its complement. Using the existing
ternary latch `ell in {-1,0,+1}`, the selected reference clutch is

\[
 K_\ell=K-\ell^2
 (\mathsf PK\mathsf P_\perp+
  \mathsf P_\perp K\mathsf P).                           \tag{22}
\]

Here `ell=0` leaves production stiffness unchanged, while both nonzero signs
give equation (20). The square controls eligibility/isolation; the unsquared
sign remains available to choose the event orientation. This is precisely the
information separation required by the earlier `C4` carrier results.

Switching is not free. For `ell->ell'`, the exact controller work is

\[
 \boxed{W_{\rm switch}={1\over2}q^T
 (K_{\ell'}-K_\ell)q.}                                  \tag{23}
\]

A complete implementation must transfer (23) reciprocally, retain the latch
and inverse history, and compile a region wider than one Moore cone into
causal substeps. Equations (19)--(23) are a selected reference mechanism, not
a production derivation.

## 7. Epistemic disposition

Established:

- **[THEOREM, CONDITIONAL]** an existing body-frame common mode supplies one
  complete cubic-covariant canonical pair;
- **[THEOREM]** the pair is invariant under `L/R` exchange and admits the
  positive action-angle chart (2);
- **[THEOREM]** its current observable amplitude norm is `2I`, and the
  half-scaled seam law (3) is symplectic and energy exact at reference scope;
- **[THEOREM]** unchanged C18 propagation has no nonzero compactly supported
  closed eigenmode; and
- **[CLOSED NEGATIVE]** unchanged production does not own or protect this
  mode as an autonomous work reserve.

Selected but not derived physically:

- designation of the longitudinal common mode as the work owner;
- the projector clutch (22); and
- the switching-work transaction (23) as the controller interface.

Still open:

- derive the ownership/protection clutch from substrate dynamics or retain it
  as a priced selection;
- form the neutral body and regional profile autonomously;
- define causal boundary current, charging, replenishment, routing, and
  recycling;
- couple the clutch to the retained orientation/history latch without
  overwrite;
- prove repeated finite-tick stability, perturbation recovery, CPU/CUDA
  parity, and operational hiding; and
- establish any physical `G*`, Born, Bell, mass, or selector-energy role.

No production integration or whole-framework completeness claim follows.
