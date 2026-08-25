# C4 phase-parity half-admitted two-polarization Maxwell carrier v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EIGHT-RAY READOUT HAS EXACTLY TWO OUTGOING POLARIZATIONS]** +
**[THEOREM, CONDITIONAL — REVERSIBLE C4-PARITY HALF-ADMITTED STREAM]** +
**[THEOREM, CONDITIONAL — EXACT SELECTED-METRIC LEDGER AND FIRST-ORDER SPEED (1/6)]** +
**[SELECTION — PHASE-PARITY MOVEMENT ADMISSION NOT YET ACTION-DERIVED]** +
**[OPEN — NONLINEAR PROTECTION, CANONICAL MOMENTUM, CHARGED POLE, AND LORENTZ FORCE]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no photon, coupling, or alpha claim promoted

**Exact certificate:**
[proof_c4_phase_parity_half_admitted_two_polarization_carrier.py](../../../../../scripts/proofs/proof_c4_phase_parity_half_admitted_two_polarization_carrier.py)
performs **52,713 exact checks** after the signed-cubic covariance extension.
It exhausts all 24 ordered SC planes, both propagation branches, all ray
polarization/displacement moments, and the exact Laurent symbol. A
representative spatial covariance orbit then exhausts all four phase origins,
both charge orientations, all twelve internal stages, both parity schedules,
and twelve-tick reversible traces. No target coefficient, fitted speed,
master root, or measured value enters.

---

## 1. Eight spatial rays are not eight field polarizations

Let (r) be the material-selected outgoing direction. Every microscopic ray
readout in the outgoing bank obeys

\[
 E_a\cdot r=0,\qquad B_a\cdot r=0,
 \qquad \boxed{B_a=r\times E_a.}                          \tag{1}
\]

Therefore every ray lies in the one-direction outgoing Maxwell subspace

\[
 \mathcal M_r^+
 =\{(E,B):E\cdot r=0,\ B=r\times E\}.                    \tag{2}
\]

Equation (2) has dimension two. The exact (6\times8) matrix whose columns
are the eight ray readouts has

\[
 \boxed{\operatorname{rank}=2.}                           \tag{3}
\]

In the ordered transverse frame ((d,v)), its coordinate Gram is

\[
 CC^{\mathsf T}=4I_2.                                     \tag{4}
\]

Thus the carrier already has exactly two internal outgoing polarizations.
The eight rays are a finite spatial-velocity quadrature supporting those two
polarizations, not eight independent photon modes.

The remaining mismatch in the ungated stream is its cadence: every ray has
longitudinal displacement one per three ticks, giving speed (1/3), whereas
the certified cotangent Maxwell slow cone has speed (1/6).

---

## 2. The existing C4 clock supplies a half-admission cadence

Each ray record contains a C4 phase address (p\in\{0,1,2,3\}). Define the
movement permission

\[
 \pi_0(p)=
 \begin{cases}
 1,&p\equiv0\pmod2,\\
 0,&p\equiv1\pmod2.
 \end{cases}                                               \tag{5}
\]

Let (t(f)) be the current tangent of the cotangent flag and (U) its
internal tick. The gated local update is

\[
 \boxed{
 (x,f,p)\longmapsto
 \bigl(x+\pi_0(p)t(f),\;Uf,\;p+1\pmod4\bigr).}            \tag{6}
\]

Every tick still advances the internal field clock. Spatially, a record either
holds or makes one SC hop, so local causality is preserved.

The two retained phase bands differ by two. Hence

\[
 \pi_0(p+2)=\pi_0(p),                                     \tag{7}
\]

and the paired records remain co-located. The C4-trivial field readout and its
selected energy metric therefore remain well defined at every tick.

The opposite schedule (pi_1=1-pi_0) is not a new physical parameter:

\[
 \pi_0(p+1)=\pi_1(p).                                     \tag{8}
\]

It is the same cadence shifted by one C4 tick. The finite rule chooses a phase
origin, but the two choices form one time-translation orbit.

---

## 3. Exact reversibility and six-tick displacement

Given the output record, the previous flag and phase are uniquely
(U^{-1}f) and (p-1). Subtracting the previous permitted tangent hop gives
the exact local inverse of equation (6).

Because the tangent flag has period three while movement parity has period
two, a six-tick interval admits each of the three tangent legs exactly once.
For every ray,

\[
 \boxed{x(t+6)-x(t)=\Delta_a,}                            \tag{9}
\]

where (Delta_a) is its prior three-tick BCC displacement. The flag returns
and the phase changes by two. After twelve ticks,

\[
 x(t+12)-x(t)=2\Delta_a,\qquad(f,p)(t+12)=(f,p)(t).        \tag{10}
\]

For all rays,

\[
 \Delta_a\cdot r=1,\qquad
 \sum_{a=1}^{8}\Delta_a=8r.                              \tag{11}
\]

Hence the centroid now advances by

\[
 \boxed{X_{\rm centroid}(t+6)-X_{\rm centroid}(t)=r,}     \tag{12}
\]

giving longitudinal speed (1/6).

---

## 4. Exact selected-metric energy and Poynting

Use the uniquely selected
[C4-trivial/resolved-handedness metric](THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md).
At every tick of every registered trace, the two phase bands remain together
within each internal-handedness channel. The exact ledger is

\[
 \boxed{H_F=1,\qquad P_F={r\over2}.}                      \tag{13}
\]

Record number is sixteen, equation (6) has a strict inverse, and equation
(13) is unchanged on hold and movement ticks. Thus half-admission does not
reopen the port/free energy defect.

The quantity \(P_F\) is still a Poynting/readout moment, not a proven canonical
translation momentum. Its conversion requires the common action's discrete
Noether/Legendre structure.

The exact
[energy-current successor](../common_action_mechanics_reciprocity/THEOREM_C4_HALF_ADMITTED_ENERGY_CURRENT_AND_MECHANICAL_MOMENTUM_BOUNDARY_v1.md)
computes that structure's native precursor from the transported selected
energy. The energy centroid gives \(J_E=r/6\), so
\(P_F=r/2=3J_E\). The raw bilinear is therefore not itself the energy current
or canonical momentum.

---

## 5. Exact two-polarization six-tick symbol

Resolve wavevector coordinates along the ordered frame:

\[
 k_d=k\cdot d,\qquad k_v=k\cdot v,\qquad k_r=k\cdot r.    \tag{14}
\]

Let \(z_j\) be the corresponding Laurent translation characters. Projecting
the exact six-tick ray stream through the rank-two polarization readout gives

\[
 \boxed{
 T_6(z)=z_r\,{(z_d+z_d^{-1})(z_v+z_v^{-1})\over4}\,I_2.} \tag{15}
\]

On the unit Fourier torus, up to the conventional sign of the translation
character,

\[
 T_6(k)=e^{-ik_r}\cos k_d\cos k_v\,I_2.                  \tag{16}
\]

Its first-order expansion per tick is

\[
 T_6(k)^{1/6}
 =I_2-{i\over6}(k\cdot r)I_2+O(|k|^2).                   \tag{17}
\]

Therefore

\[
 \boxed{
 \text{two degenerate outgoing transverse polarizations with }
 c_{\rm eff}={1\over6}.}                                  \tag{18}
\]

The transverse factors begin at second order. They encode finite wavepacket
spreading into higher kinetic moments, not an extra first-order longitudinal
or scalar pole. The exact first and second displacement moments are

\[
 A_i=r_iI_2,\qquad B_{ij}=\delta_{ij}I_2                  \tag{19}
\]

per six-tick macro step.

---

## 6. Relation to the certified vacuum Maxwell sector

The global C3 cotangent collision independently has two transverse Maxwell
pairs with speed (1/6). Equations (2)--(3) supply the two outgoing
polarizations; equations (15)--(18) supply the same first-order speed using
the carrier's own C4 clock. The factor of two that separated the ungated ray
centroid from the slow cone is therefore accounted for by half-admission.

This closes the **kinematic mode-count and speed target** for the finite
outgoing carrier. It does not yet prove:

1. that the same finite collision/action selects equation (5);
2. that the rank-two branch is an exact nonlinear invariant manifold under
   arbitrary interactions;
3. the full Maxwell constraint algebra and stress tensor beyond first order;
4. a charged static pole or Lorentz-force backreaction; or
5. canonical energy-momentum normalization.

---

## 7. Common-action consequence

The result identifies a particularly small clocked streaming term for the
one-action programme:

\[
 \mathcal A_{\rm move}
 \quad\Longrightarrow\quad
 \Delta x=\pi(p)t(f),\qquad p\mapsto p+1.                 \tag{20}
\]

No new movement bit is required. The same C4 address already used by
quadrature histories and local clock recurrence supplies the radiation
cadence, while its phase-trivial field projection preserves energy.

Equation (20) remains **[SELECTION]** until obtained as the stationary update
of the common finite action with its work, collision, and inverse ledger. The
next field-side gate is therefore no longer “eight rays versus two modes.” It
is action selection and nonlinear protection of the exact two-polarization
half-admitted carrier, composed with material recoil and absorption.
