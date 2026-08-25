# Cotangent native-alpha action-scale obstruction v1

**Date:** 2026-08-24
**Status:** **[THEOREM — COTANGENT SPEED AND CANONICAL SOURCE-EDGE NORMALIZATION FIXED]** +
**[THEOREM — POSITIVE ONE-PARAMETER FIELD-ACTION NORMALIZATION ORBIT]** +
**[SCOPED NO-GO — CURRENT KINEMATIC CERTIFICATES CANNOT DETERMINE NATIVE ALPHA]** +
**[CONDITIONAL TARGET — ALPHA IS THE BLOCKED ACTION-CURVATURE RATIO DIVIDED BY $4\pi c_{\rm eff}$]** +
**[OPEN — CHARGED STATIC POLE AND MICROSCOPIC DERIVATION OF THE ACTION CURVATURE]**
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_native_alpha_action_scale_obstruction.py](../../../../../scripts/proofs/proof_cotangent_native_alpha_action_scale_obstruction.py)
performs 765 exact symbolic and rational checks. It verifies the packet Gram
normalization on all SC directions, Gauss-constrained minimizers on four
connected graph families, their cycle-space orthogonality, the complete
positive action-scale orbit, vacuum-equation scale cancellation, and the
resulting conditional coupling family. It performs no parameter fit, target
comparison, or numerical search.

---

## 1. What the strict-discrete chain now fixes

The
[cotangent vacuum-Maxwell theorem](THEOREM_GLOBAL_C3_COTANGENT_LAYER_COLLISION_AND_VACUUM_MAXWELL_PASS_v1.md)
derives two transverse electric--magnetic pairs with

\[
 \boxed{c_{\rm eff}={1\over6}}                      \tag{1}
\]

in lattice units per global tick on its selected invariant vacuum subspace.

The
[stabilizer-packet source theorem](THEOREM_COTANGENT_STABILIZER_PACKET_REVERSIBLE_GAUSS_SOURCE_v1.md)
derives a symmetry-complete eight-record packet with total electric readout
\(8d\). Because the electric Gram block is \(64I_3\),

\[
 \boxed{(8d)^T(64I_3)^{-1}(8d)=1.}                 \tag{2}
\]

Thus one packet is exactly one canonical electric edge quantum. Its reversible
activation gives the incidence identity

\[
 \boxed{D\mathcal E=\rho}                          \tag{3}
\]

for the manifested endpoint pair, conditional on the declared ternary-charge
identification.

Equations (1)--(3) remove arbitrary rescaling of the **reported carrier
coordinates**. They do not yet price the action or energy of one such field
quantum.

---

## 2. The surviving action-scale orbit

Let \(D\) be a finite connected incidence operator and let the canonical
electric edge field obey equation (3). The most general isotropic quadratic
static energy at this normalization contains a positive coefficient
\(\Gamma\):

\[
 \boxed{H_\Gamma[\mathcal E]
 ={\Gamma\over2}\langle\mathcal E,\mathcal E\rangle,
 \qquad \Gamma>0.}                                  \tag{4}
\]

For a neutral charge configuration \(\rho\), minimizing equation (4) subject
to equation (3) gives

\[
 \mathcal E_{\min}=D^T(DD^T)^+\rho.                \tag{5}
\]

The coefficient \(\Gamma\) cancels from the stationarity equation. The field
configuration and Gauss residual are therefore independent of \(\Gamma\),
while

\[
 \boxed{
 H_{\Gamma,\min}
 ={\Gamma\over2}
 \langle\rho,(DD^T)^+\rho\rangle.}                 \tag{6}
\]

Every divergence-free cycle perturbation is orthogonal to equation (5), so
this is the unique minimum modulo the usual gauge zero mode.

The same obstruction holds dynamically. For a quadratic Maxwell action

\[
 S_\Gamma[A]
 ={\Gamma\over2}\sum_n
 \left(\|\Delta_tA_n\|^2-\langle A_n,KA_n\rangle\right), \tag{7}
\]

the Euler--Lagrange equation is

\[
 \Gamma(\Delta_t^2A+KA)=0.                         \tag{8}
\]

Every \(\Gamma>0\) gives the same vacuum modes and the same speed in equation
(1). Multiplying the complete constrained field action by \(\Gamma\) also
leaves equation (3) unchanged, but multiplies every on-shell action and static
source energy.

Therefore the currently proved kinematics lie on a positive
one-parameter normalization orbit:

\[
 \boxed{
 \Gamma>0:quad
 (c_{\rm eff},\;\|e_{\rm packet}\|,\;D\mathcal E-\rho)
 \text{ fixed},qquad
 V_{\rm static}\propto\Gamma.}                     \tag{9}
\]

---

## 3. Consequence for native alpha

Let \(I_*\) be the physical action unit that converts the finite phase history
into \(\exp(iS/I_*)\). Under the direct-response convention already frozen in
the alpha-readout contract, equation (6) would identify, conditionally,

\[
 g_{\rm eff}^2=\Gamma                              \tag{10}
\]

for the unit canonical charge and the correspondingly normalized lattice
Green kernel. With equation (1),

\[
 \boxed{
 \alpha_{\rm native}
 ={\Gamma\over4\pi I_*c_{\rm eff}}
 ={3\over2\pi}{\Gamma\over I_*}.}                 \tag{11}
\]

The existing finite certificates do not determine \(\Gamma/I_*\). Hence they
do not determine \(\alpha_{\rm native}\).

Equation (11) is not a coupling prediction. It exposes the unique scalar ratio
that a microscopic blocking/action calculation must supply before a native
coupling exists.

---

## 4. Why the existing exact numbers do not close the ratio

Three tempting quantities are already exact:

1. **Packet norm \(1\).** Equation (2) fixes a field coordinate relative to
   the product-reference Gram matrix. It is not an on-shell action or static
   interaction energy.
2. **Token ledger \(8\).** The reserve and active packet each retain eight
   positive unit-energy records. Their ledger difference is
   \(8-8=0\), so the ledger does not give the active-versus-reserve field work
   or \(\Gamma\).
3. **Pair tangent weight \(2^{-191}\).** This is the probability weight of an
   exactly two-occupied event in the registered independent half-filled
   product reference. It is reference-distribution data, not a source--source
   residue or action curvature.

All three remain fixed while \(\Gamma/I_*\) varies in equation (11). Relabeling
any of them as alpha would therefore be a normalization substitution.

---

## 5. A second, independent boundary: the charged pole

Even a microscopic value of \(\Gamma/I_*\) would not by itself complete the
measurement. The current finite collision passes the transverse vacuum cone,
but its scalar block preserves a Gauss graph only for

\[
 \kappa^2={|k|^2\over3},                            \tag{12}
\]

not for a local wavevector-independent charge coefficient. The source packet
creates an exact local Gauss edge, but no common action has yet relaxed a
separated neutral pair into the long-distance minimum-energy Coulomb coat.

The earlier production-Hodge reciprocal action cannot supply that coat: its
derivative source and probe cancel the massless static pole and reverse the
electric force sign. That route remains closed negative.

Thus the native-alpha program now has two sharply separated debts:

1. **pole debt:** derive a charged local relaxation whose static Green kernel
   has the required long-distance massless pole; and
2. **action-scale debt:** derive \(\Gamma/I_*\) from the same microscopic
   transaction action.

Neither debt can be paid by the master quadratic.

---

## 6. The corrected blind observable

Let a future finite blocking calculation produce the dimensionless effective
action \(S_{\rm eff}/I_*\) in the canonical packet coordinates of equation
(2). Define its transverse electric curvature at the vacuum by

\[
 \boxed{
 \chi_{\rm EM}
 =D^2\!\left({S_{\rm eff}\over I_*}\right)_{0}
 [e_{\rm packet},e_{\rm packet}].}                 \tag{13}
\]

The normalization \(\|e_{\rm packet}\|=1\) makes equation (13) basis-fixed.
For the quadratic family above, \(\chi_{\rm EM}=\Gamma/I_*\). A reciprocal
static calculation must independently recover the same coefficient from

\[
 {V_{+-}(r)\over I_*}
 =-\chi_{\rm EM}G_{\rm vac}(r)+o(G_{\rm vac}(r)).   \tag{14}
\]

Only after equations (13)--(14) agree and the charged pole exists may one
report

\[
 \boxed{
 \alpha_{\rm native}
 ={\chi_{\rm EM}\over4\pi c_{\rm eff}}
 ={3\chi_{\rm EM}\over2\pi}.}                     \tag{15}
\]

The action, source preparation, blocking scale, fit window, and finite-support
error must be frozen before evaluating \(\chi_{\rm EM}\). The master root,
CODATA alpha, and any desired relaxation eigenvalue remain forbidden inputs.

---

## 7. What is established and what remains open

Established exactly:

- a selected transverse vacuum speed \(c_{\rm eff}=1/6\);
- one canonical electric source edge from the eight-record packet;
- exact local Gauss incidence for that edge;
- invariance of all those data under the positive action-scale orbit; and
- linear scaling of the static interaction energy with \(\Gamma\).

Still open:

- a common charged collision/relaxation with a static massless pole;
- derivation of the blocked dimensionless curvature \(\chi_{\rm EM}\);
- active-versus-reserve Hamiltonian work;
- stable separated charged matter sources;
- reciprocity between the curvature and static residue; and
- any comparison between equation (15) and the master root \(x_+\).

The fine-structure root therefore remains a mathematical correspondence. The
new result narrows the physical coupling debt to a named observable and proves
that current kinematics alone cannot determine it.

The subsequent
[charged-pole reciprocal-alpha protocol](THEOREM_COTANGENT_CHARGED_POLE_RECIPROCAL_ALPHA_MEASUREMENT_PROTOCOL_v1.md)
closes the pole and measurement-definition debts for one explicitly selected
canonical Maxwell--Gauss reference action. Its cubic static kernel is
$1/\Lambda(k)$, and its static source residue and canonical free-field Hessian
both return the same blind coefficient $\chi_{\rm EM}$. This does not remove
the obstruction: the finite transaction has not derived the reference action,
realized its constraint locally and reversibly, or fixed the value of
$\chi_{\rm EM}$.

---

## 8. Next locked gate

Derive the finite-history quadratic response of the **same** cotangent
collision/source transaction in the canonical packet basis. The pass requires:

1. a local reversible realization of the selected charged massless-pole
   reference action;
2. a block-size-stable value of equation (13), obtained without target data;
3. exact active/reserve work and inverse accounting;
4. agreement of equations (13) and (14); and
5. unchanged transverse speed \(1/6\).

Failure to derive a unique \(\chi_{\rm EM}\) is a normalization obstruction,
not permission to identify packet norm, token count, collision weight, or the
master root with the coupling.

The later
[directional-port recoil partition theorem](THEOREM_DIRECTIONAL_PORT_RECOIL_ENERGY_PARTITION_AND_COUPLING_MEASURE_BOUNDARY_v1.md)
supplies the first common-transaction observable that can constrain such a
field scale without reading a Coulomb target. The exact
[C4-trivial field-sector successor](THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md)
then selects emitted field work \(\Gamma/2\), closes the port/free energy
handoff conditionally. The still later
[clocked-remainder recoil successor](../common_action_mechanics_reciprocity/THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
replaces the immediate unit-step recoil by a speed-\(1/L\) orbit. If
\(\mu=m/L\) denotes its canonical impulse magnitude, the current conditional
ledger is

\[
 I_*={\Gamma\over2}+{\mu\over2L},\qquad
 \chi_{\rm work}={\Gamma L\over\Gamma L+\mu}.             \tag{16}
\]

For the blocked/static curvature definition in equation (13), the same
normalization would imply

\[
 \chi_{\rm EM}={\Gamma\over I_*}
 ={2\Gamma L\over\Gamma L+\mu}.                           \tag{17}
\]

This does not retire the obstruction: the common action has not derived
\(L,m\), supplied field canonical translation momentum and a charged static
pole, or proved that the emission coefficient and blocked/static residue are
the same action parameter. No alpha or master-root identification is
licensed.
