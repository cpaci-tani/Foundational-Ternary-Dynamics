# Cotangent charged-pole reciprocal-alpha measurement protocol v1

**Date:** 2026-08-24

**Status:** **[THEOREM, CONDITIONAL — CANONICAL MAXWELL--GAUSS REFERENCE ACTION HAS A CHARGED MASSLESS POLE]** +
**[THEOREM — STATIC RESIDUE/FREE-FIELD CURVATURE RECIPROCITY]** +
**[THEOREM — TARGET-BLIND NATIVE-ALPHA ESTIMATOR]** +
**[SELECTION — COMMON QUADRATIC ACTION]** +
**[OPEN — MICROSCOPIC CURVATURE VALUE AND LOCAL REVERSIBLE REALIZATION]**

**Native-alpha status:** a basis-fixed, target-blind measurement protocol now
exists, but the dimensionless curvature it measures remains undetermined.
The master root is still a mathematical correspondence, not a derived
electromagnetic coupling.

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_charged_pole_reciprocal_alpha_measurement_protocol.py](../../../../../scripts/proofs/proof_cotangent_charged_pole_reciprocal_alpha_measurement_protocol.py)
performs 284 exact symbolic and rational checks. It verifies canonical packet
normalization on all SC directions, constrained Gauss solutions and cycle
orthogonality for all neutral pairs on four connected graph families, the
cubic infrared pole, equality of the static and free-field estimators, and the
current conditional source-work relation. It performs no numerical search,
fit, or comparison with a fine-structure target.

---

## 1. The distinction this theorem makes

The
[action-scale obstruction](THEOREM_COTANGENT_NATIVE_ALPHA_ACTION_SCALE_OBSTRUCTION_v1.md)
proved that the existing speed, packet norm, and Gauss-incidence certificates
are invariant under a positive field-action rescaling. It left two debts:

1. exhibit a charged massless static pole; and
2. determine the dimensionless action curvature multiplying that pole.

These are not the same problem. This theorem closes the first debt for one
explicit selected reference action and proves how the second quantity would
be measured twice. It does not derive that action or its coefficient from the
finite transaction rule.

---

## 2. Canonical source coordinate

The eight-record cotangent source packet has electric total $8d$ and inverse
electric Gram $I_3/64$. Therefore every SC direction satisfies

\[
 (8d)^{\mathsf T}{I_3\over64}(8d)=1.                     \tag{1}
\]

Together with the selected charge convention, reversible activation of one
packet gives a neutral endpoint source obeying

\[
 D E=\rho,\qquad \sum_x\rho(x)=0.                       \tag{2}
\]

Equation (1) freezes the coordinate in which the response curvature is read.
It does not assign an energy to that coordinate.

---

## 3. Selected common reference action

Let

\[
 \chi_{\rm EM}>0                                           \tag{3}
\]

be the dimensionless coefficient of the field action $S_{\rm EM}/I_*$. On a
finite connected lattice graph, select the constrained static functional

\[
 {H_{\rm stat}\over I_*}
 ={\chi_{\rm EM}\over2}\langle E,E\rangle,
 \qquad DE=\rho.                                          \tag{4}
\]

Writing $L_G=DD^{\mathsf T}$, variation gives

\[
 E_{\min}=D^{\mathsf T}L_G^+\rho,qquad
 {H_{\min}\over I_*}
 ={\chi_{\rm EM}\over2}
 \langle\rho,L_G^+\rho\rangle.                           \tag{5}
\]

Every divergence-free cycle $z\in\ker D$ is orthogonal to $E_{\min}$, so
equation (5) is the unique minimum modulo the constant potential mode.

For the cubic nearest-neighbor lattice the Fourier symbol is

\[
 \Lambda(k)=2\sum_{a=1}^3(1-\cos k_a)
 =|k|^2+O(|k|^4).                                         \tag{6}
\]

It is positive away from the constant mode. Hence the selected action has

\[
 \boxed{G(k)={1\over\Lambda(k)}}                          \tag{7}
\]

as its charged massless static Green pole. For a separated unit positive and
negative source, the cross term has the ordinary opposite-charge sign,
$V_{+-}(r)/I_*=-\chi_{\rm EM}G(r)$ after the position-independent self terms
are removed.

This is a reference-action pole theorem. The current finite collision has not
yet produced equation (4) as a local constrained evolution or formed stable
separated charges dynamically.

---

## 4. Two blind measurements of one curvature

For any nonzero neutral Fourier source mode,

\[
 {H_k\over I_*}
 ={\chi_{\rm EM}\over2}{|\rho_k|^2\over\Lambda(k)}.       \tag{8}
\]

The static estimator is therefore

\[
 \boxed{
 \widehat\chi_{\rm stat}(k)
 ={2(H_k/I_*)\Lambda(k)\over|\rho_k|^2}
 =\chi_{\rm EM}.}                                        \tag{9}
\]

Independently, take the Hessian of the free transverse field action in the
canonical unit-packet coordinate $e_{m packet}$ of equation (1):

\[
 \boxed{
 \widehat\chi_{\rm free}
 =D^2(S_{\rm EM}/I_*)_0[e_{m packet},e_{m packet}]
 =\chi_{\rm EM}.}                                        \tag{10}
\]

Thus a common Maxwell--Gauss action must pass the reciprocity identity

\[
 \boxed{\widehat\chi_{\rm stat}=\widehat\chi_{\rm free}.} \tag{11}
\]

Equation (11) prevents an independent electrostatic fit after the radiative
action has been chosen. The source, packet coordinate, lattice Green kernel,
and estimator are fixed before evaluation. No algebraic root or experimental
alpha enters the protocol.

---

## 5. Conditional native alpha

The selected cotangent vacuum cone has

\[
 c_{\rm eff}={1\over6}.                                   \tag{12}
\]

If one microscopic action passes equation (11), then its native
dimensionless electromagnetic coupling is operationally

\[
 \boxed{
 \alpha_{\rm native}
 ={\chi_{\rm EM}\over4\pi c_{\rm eff}}
 ={3\chi_{\rm EM}\over2\pi}.}                           \tag{13}
\]

Equation (13) defines a measurement; it does not predict a number.

The present conditional port/recoil work ledger reads

\[
 I_*={\Gamma\over2}+{\mu\over2L},                         \tag{14}
\]

where $\Gamma/2$ is emitted field work and $\mu/(2L)$ is clocked material
recoil work. If, and only if, the same $\Gamma$ is the static and free-field
coefficient, then

\[
 \boxed{
 \chi_{\rm EM}={\Gamma\over I_*}
 ={2\Gamma L\over\Gamma L+\mu}.}                         \tag{15}
\]

The current framework has not derived $L$, $m$, $\mu=m/L$, or the equality of
the emission and blocked-action coefficients. Equation (15) is therefore a
conditional reduction of the unknown, not its value.

---

## 6. What is established and what remains open

### Established exactly for the selected reference action

- canonical unit charge-edge normalization;
- a cubic charged massless pole $1/\Lambda(k)$;
- the finite-volume neutral-source response;
- a target-blind static residue estimator;
- a target-blind free-field Hessian estimator;
- exact reciprocity of the two estimators; and
- the conditional conversion to $\alpha_{\rm native}$.

### Still open

1. derivation of the common Maxwell--Gauss action from the phase-complete
   transaction;
2. a finite local reversible realization of its longitudinal constraint;
3. stable dynamically formed separated charges and their dressing;
4. a block-size-stable microscopic value of $\chi_{\rm EM}$;
5. derivation of the cadence and material inertia entering equation (15);
6. field translation charge and reciprocal Lorentz backreaction; and
7. comparison with the master root only after items 1--6 pass blindly.

The pole and measurement-definition debts are now sharply separated. FTD can
state exactly what alpha would mean internally, but it cannot yet claim to
have derived the observed fine-structure constant.

---

## 7. Next locked gate

Realize equation (4) in the same finite reversible transaction that emits the
rank-two $c_{\rm eff}=1/6$ carrier and supplies clocked recoil. Then evaluate
equations (9) and (10) without target data. A pass requires one stable
$\chi_{\rm EM}$ from both protocols, exact inverse and work accounting, and
unchanged Gauss incidence and transverse speed. Any residual free scale is an
honest normalization freedom, not permission to substitute the master root.

The subsequent selected
[packet/clock/recoil absorption generator](../common_action_mechanics_reciprocity/THEOREM_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_AND_GRAVITY_SOURCE_BOUNDARY_v1.md)
now supplies exact reciprocal work accounting for an admitted batch. For one
rest-clock action quantum it gives

\[
 \chi_{\rm EM}
 = {\omega\over d-|p|^2/(2m\Gamma)},
\]

reducing to $\omega/d$ when $p=0$. This is a compliance relation, not a
coupling prediction: the microscopic action still fixes none of
$p,m,\omega,d,\Gamma/I_*$ and has not yet made the charged pole and propagating
packet two limits of one finite transaction.

On the subsequent conditional symmetric-stress branch, batch energy
$E=N\Gamma$ fixes $p=6Er$ and sharpens the same relation to

\[
 \chi_{\rm EM}
 ={\omega\over N-18N^2\Gamma/m},
 \qquad m\ge18N\Gamma.
\]

This removes the freely declared $p$ only after selecting stress symmetry. It
still predicts no coupling because the finite action has not derived that
symmetry or the remaining scale data.
