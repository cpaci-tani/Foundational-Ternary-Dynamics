# C4-trivial field sector and unique directional-port handoff metric v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT PHASE-BLIND C4 METRIC FACTORIZATION]** +
**[THEOREM, CONDITIONAL — UNIQUE POSITIVE HANDOFF-CONSERVING METRIC IN THE REGISTERED CLASS]** +
**[THEOREM, CONDITIONAL — CLOSED CANONICAL PORT/FREE ENERGY AND POYNTING LEDGER]** +
**[SUCCESSOR — TWO-POLARIZATION SPEED TARGET CLOSED CONDITIONALLY BY C4 HALF-ADMISSION]** +
**[SUCCESSOR — MATERIAL WORLDLINE/QUADRATIC LEGENDRE MAP CLOSED CONDITIONALLY]** +
**[BOUNDARY — FIELD NOETHER CHARGE, ACTION SELECTION, NONLINEAR PROTECTION, AND SCALES OPEN]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no coupling or alpha claim promoted

**Exact certificate:**
[proof_c4_trivial_field_sector_unique_handoff_metric.py](../../../../../scripts/proofs/proof_c4_trivial_field_sector_unique_handoff_metric.py)
performs **254 exact symbolic and signed-cubic checks**. It factors the channel
Gram, diagonalizes its invariant sectors, applies the exact handoff condition,
and evaluates the energy and bilinear Poynting ledger on every edge of all 24
ordered SC planes and both propagation branches. No fitted coefficient,
measured constant, master root, or target probability enters.

---

## 1. The kernel-separation theorem fixes the field phase sector

The
[C4 Born/radiation kernel-separation theorem](../common_action_mechanics_reciprocity/THEOREM_C4_BORN_RADIATION_KERNEL_SEPARATION_AND_CONTEXTUAL_MIXER_BOUNDARY_v1.md)
proves that the transported C4 phase address has two different physical
readouts:

- the raw Born amplitude occupies the quadrature sector \(P_Q\); and
- the current cotangent (E/B) readout is phase blind and occupies the trivial
  sector \(P_0\).

The previous
[coherence-metric theorem](THEOREM_DIRECTIONAL_PORT_COHERENCE_METRIC_HANDOFF_AND_PHASE_COMPATIBILITY_BOUNDARY_v1.md)
allowed three abstract channel weights \((a,b,c)\). Once the actual field type
is imposed, those weights are no longer independent.

In channel order

\[
 (h_+,p),(h_-,p),(h_+,p+2),(h_-,p+2),                    \tag{1}
\]

phase blindness gives the factorized Gram

\[
 G_F(a)=
 \underbrace{\begin{pmatrix}1&1\\1&1\end{pmatrix}}_{C4\text{-trivial phase}}
 \otimes
 \underbrace{\begin{pmatrix}1&a\\a&1\end{pmatrix}}_{h\text{ channels}}. \tag{2}
\]

Explicitly,

\[
 G_F(a)=
 \begin{pmatrix}
 1&a&1&a\\
 a&1&a&1\\
 1&a&1&a\\
 a&1&a&1
 \end{pmatrix}.                                          \tag{3}
\]

Therefore the predecessor notation is restricted to

\[
 \boxed{b=1,\qquad c=a.}                                  \tag{4}
\]

The four eigenvalues are

\[
 \boxed{2(1+a),\quad2(1-a),\quad0,\quad0.}                \tag{5}
\]

Positivity alone permits (-1\le a\le1).

---

## 2. Handoff conservation uniquely resolves internal handedness

The exact port/free geometry independently gave the conservation condition

\[
 c=-a.                                                     \tag{6}
\]

Combining equations (4) and (6) yields

\[
 a=-a,
\]

and therefore

\[
 \boxed{(a,b,c)=(0,1,0).}                                 \tag{7}
\]

The selected Gram is

\[
 \boxed{
 G_F=
 \begin{pmatrix}
 1&0&1&0\\
 0&1&0&1\\
 1&0&1&0\\
 0&1&0&1
 \end{pmatrix}.}                                         \tag{8}
\]

Equation (8) has rank two and spectrum ((2,2,0,0)). It says:

- the two C4 phase addresses are the same phase-blind field channel; and
- the two microscopic internal-handedness channels are orthogonal in physical
  field energy.

This is not an arbitrary choice between the controls in the predecessor. It
is the unique member compatible with the actual C4 field type and exact
collisionless handoff conservation. The remaining conditionality is physical:
the common action has not yet been constructed to generate equation (8) as its
Hamiltonian metric.

---

## 3. Exact standing, outgoing, and free ledger

With equation (8), the registered directional-port states have

\[
 \boxed{
 H_{\cal S}={1\over2},\qquad
 H_{\cal O}=1,\qquad
 H_{\rm free}=1.}                                        \tag{9}
\]

The corresponding bilinear Poynting moments are

\[
 \boxed{
 P_{\cal S}=0,\qquad
 P_{\cal O}={r\over2},\qquad
 P_{\rm free}={r\over2}.}                                \tag{10}
\]

Thus

\[
 \boxed{
 \Delta H_{\rm emit}={1\over2},\qquad
 \Delta H_{\rm handoff}=0,\qquad
 \Delta P_{\rm handoff}=0.}                             \tag{11}
\]

The old ((H,P):(2,r)\to(1,r/2)) defect came from squaring the fully coarse
moment after also merging the two internal-handedness channels. Equation (8)
retains the action-required channel resolution. No energy is lost when the
rays spatially separate.

Equation (10) is an exact Poynting/readout ledger. It is not yet a canonical
translation momentum theorem: a discrete Noether generator, carrier-speed
conversion, and material Legendre map remain absent.

---

## 4. Provisional immediate-recoil work partition

Let \(\Gamma>0\) multiply the selected field quadratic form, and let
\(\mu>0\) be the unique signed-cubic unit-recoil kinetic coefficient. One
emission event carries

\[
 W_F={\Gamma\over2},\qquad
 W_R={\mu\over2}.                                         \tag{12}
\]

Exact source-work closure therefore requires

\[
 \boxed{I_*={\Gamma+\mu\over2}.}                          \tag{13}
\]

The operational field-work fraction becomes

\[
 \boxed{
 \chi_{\rm work}={W_F\over I_*}
 ={\Gamma\over\Gamma+\mu}
 =1-{\mu\over2I_*}.}                                     \tag{14}
\]

The bare field coefficient relative to source work is

\[
 {\Gamma\over I_*}={2\Gamma\over\Gamma+\mu}.             \tag{15}
\]

The coherence ambiguity has disappeared. The remaining native-coupling debt
is now the dynamical ratio \(\mu/\Gamma\), together with the proof that the
same common action produces both kinetic and field metrics.

No equality between equation (14) or (15) and the fine-structure constant is
asserted. Such a comparison is prohibited until the operational observable,
normalization convention, charged pole, and inertia ratio are uniquely fixed.

### 4.1 Clocked-worldline correction

The field result \(W_F=\Gamma/2\) is unchanged. The later
[clocked-remainder recoil theorem](../common_action_mechanics_reciprocity/THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
shows that the material term in equation (12) cannot remain \(\mu/2\) for a
stable slow worldline.

With cadence \(L\), inertia \(m\), and impulse magnitude
\(\mu=m/L\), the material action gives

\[
 W_R={m\over2L^2}={\mu\over2L}.                           \tag{16}
\]

The current conditional work ledger is therefore

\[
 \boxed{
 I_*={\Gamma\over2}+{\mu\over2L},\qquad
 \chi_{\rm work}={\Gamma L\over\Gamma L+\mu}.}            \tag{17}
\]

Equations (13)--(15) remain the exact diagnostic of the old immediate unit
step. Equations (16)--(17) are the selected slow-worldline successor, still
conditional on the material action and still lacking a field Noether charge.

---

## 5. What is closed and what remains

### Closed in the registered quadratic field class

1. the field C4 phase sector is trivial, not the Born quadrature sector;
2. exact handoff conservation uniquely resolves internal handedness;
3. the port-to-free energy and Poynting defect vanishes; and
4. the field emission debit is uniquely \(\Gamma/2\).

### Still open

1. a finite common action whose Hamiltonian/transfer matrix actually selects
   equation (8);
2. a field translation Noether map relating the native energy current to
   physical field momentum;
3. formation of stable recoiling matter and derivation of \(L\), \(m\), and
   \(\mu=m/L\);
4. action selection and nonlinear protection of the successor's rank-two
   half-admitted Maxwell carrier, including the continuum stress tensor;
5. Lorentz force/backreaction on general matter configurations; and
6. a native fine-structure coupling measurement.

The exact
[C4 phase-parity half-admission successor](THEOREM_C4_PHASE_PARITY_HALF_ADMITTED_TWO_POLARIZATION_MAXWELL_CARRIER_v1.md)
then proves that the eight rays already have rank-two outgoing field readout
and that the native phase clock can reduce their centroid cadence from (1/3)
to the certified Maxwell speed (1/6) while preserving equation (9). Thus the
kinematic mode-count/speed target is closed conditionally. The next field-side
gate is common-action selection, nonlinear protection, and physical
translation momentum.

The clocked-remainder successor closes the material-side reference map:
\(Y=Lx+a\) advances by one persistent impulse token, the stable speed is
\(1/L\), and the quadratic action gives \(p_M=(m/L)d\). It does not close the
field momentum normalization or select the complete interaction action.
