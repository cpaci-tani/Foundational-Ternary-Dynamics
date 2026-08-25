# Preregistration — C18 scalar/STF/vector-constraint ownership absorption seam v1

**Date frozen:** 2026-08-24  
**Campaign status:** pre-execution lock  
**Ledger status:** no FTD identifier or claim row reserved  
**Production status:** no engine mutation authorized

## 1. Question

Can the existing phase-complete C18 bond module supply, without a new spatial
representation type, the minimum scalar, STF, and charge-even vector
ownership needed to accept one packet's symmetric stress at the reciprocal
absorption seam?

The candidate seam must:

1. retain the charge-odd electromagnetic vector channel;
2. use an independent existing \(T_{1u}\) copy for the longitudinal gravity
   constraint record;
3. load the capacity trace and common-phase STF tensor from the same packet
   stress;
4. preserve one local divergence constraint exactly;
5. remain canonical, energy conserving, and history invertible when composed
   with packet/clock/recoil absorption; and
6. leave exactly two homogeneous STF tensor coordinates and two conjugate
   partners at every nonzero derivative symbol.

The campaign must expose rather than hide any remaining source-coupling,
constraint-action, static-pole, or lensing freedom.

## 2. Frozen source chain

The exact certificate may import or compare only:

1. proof_moore_bond_capacity_type_census.py, SHA-256
   D8D83F4600822A7C8CB426120B61D8CA57465B379340FF51D87EE81D3A95A7F6;
2. proof_even_tensor_second_order_action_spin2_escape.py, SHA-256
   820193844E22420205DC04CC4E1D957E2AE76A86C94017512CA199E55178CFE8;
3. proof_c18_tensor_doublet_tt_reduction.py, SHA-256
   437392221691D2579D55A078C0CF4F2D3B5AE08D1EB54DCC7469FF3458D67436;
4. proof_c4_symmetric_stress_packet_momentum_and_source_handoff.py,
   SHA-256
   312FA1071D09FEBE61225A8BAFBA2C6D7994E80A584DE4B9220EE5274ACCB938;
5. proof_reciprocal_packet_clock_recoil_absorption_generator.py, SHA-256
   4B824C3B37A8BADEC9F50ED1785602734B75D6CCF03234D65826E0541CDC2576;
   and
6. this preregistration's pre-execution SHA-256.

No numerical search, floating-point tolerance, target gravity coefficient,
deflection angle, master root, or empirical coupling may enter.

## 3. Frozen existing-type census

For the nine unoriented C18 lines (three SC plus six FCC), the
inversion-odd line module must decompose as

\[
 \boxed{V_{\rm C18,odd}=2T_{1u}\oplus T_{2u}.}            \tag{P1}
\]

The SC and FCC shell first moments give two independent covariant vectors
\(J_{\rm SC}\) and \(J_{\rm FCC}\). Freeze the invertible shell-copy basis

\[
 J_{\rm EM}=J_{\rm SC}+J_{\rm FCC},
 \qquad
 J_{\rm C}=J_{\rm SC}-J_{\rm FCC}.                       \tag{P2}
\]

\(J_{\rm EM}\) is the already used electromagnetic-vector slot. \(J_{\rm C}\)
is only a candidate ownership slot for the charge-even constraint record.
Equation (P1) proves spatial representation capacity, not the finite
internal-charge action on that copy.

The even C18 module contains

\[
 2A_{1g}\oplus2E_g\oplus T_{2g}.                          \tag{P3}
\]

The two common C4 quadratures each carry this same even module. Their blocked
second moments therefore contain one scalar canonical pair and one STF
canonical pair without a new spatial irrep.

## 4. Frozen local source and constraint

Let a packet batch have energy \(E>0\), direction \(r\), symmetric stress

\[
 \Sigma_F=E\,rr^{\mathsf T},
\]

and STF part

\[
 S=E\left(rr^{\mathsf T}-{\mathbf1\over3}\right).        \tag{P4}
\]

Let \(q\ne0\) be an arbitrary exact nonzero local derivative symbol. The STF
divergence map is

\[
 D_q(X)=Xq.                                             \tag{P5}
\]

Freeze a tensor momentum owner \(\Pi\in\operatorname{STF}(3)\) and a
three-component constraint momentum owner \(\kappa\). The extended local
constraint is

\[
 \boxed{{\cal C}_q(\Pi,\kappa)=\Pi q-\kappa=0.}           \tag{P6}
\]

The source load uses one tensor coefficient \(g_T\):

\[
 \Pi'=\Pi+g_TS,
 \qquad
 \boxed{\kappa'=\kappa+g_TS q.}                         \tag{P7}
\]

The second relation must be proved necessary and sufficient for equation
(P6) to be preserved for every STF source and nonzero \(q\). No inverse
Laplacian or TT projection is allowed.

The scalar trace owner \(\sigma\) is loaded independently as

\[
 \sigma'=\sigma+g_0E.                                  \tag{P8}
\]

The equality \(g_0=g_T\) is a candidate universal-coupling condition, not an
input to the exact seam tests.

## 5. Frozen common generator

Collect material, tensor, scalar, and constraint momenta into \(p\), with
constant source shift

\[
 a=(6Er,\;g_TS,\;g_0E,\;g_TS q).                       \tag{P9}
\]

Let \(H(p)\) be a positive differentiable quadratic owner Hamiltonian and let
the packet carry energy \(E\). Freeze the type-2 generator

\[
 \boxed{
 F_2(\theta,x;I',p')
 =\theta I'+x\!\cdot\!(p'-a)
 -{\theta\over\omega}
 \left[E+H(p'-a)-H(p')\right].}                       \tag{P10}
\]

Its derivative map must be

\[
 p'=p+a,\qquad \theta'=\theta,                         \tag{P11}
\]

\[
 I'=I+{E+H(p)-H(p+a)\over\omega},                     \tag{P12}
\]

\[
 x'=x-{\theta\over\omega}
 \left[\nabla H(p)-\nabla H(p+a)\right].              \tag{P13}
\]

At the seam \(\theta=0\), all canonical coordinates remain fixed while every
owned momentum/source record changes atomically.

## 6. Exact acceptance gates

### G1 — integrity and blindness

- Every frozen hash matches.
- All calculations are symbolic, integer, or rational.
- No target coupling, lensing ratio, gravity coefficient, or observed
  constant is present.

### G2 — existing-type price

- Prove equations (P1)--(P3) by exact \(O_h\) characters.
- Prove the shell-copy map (P2) is invertible and cubic covariant.
- Classify \(J_{\rm C}\) only as an available spatial-vector copy; do not claim
  that its charge-even ownership rule is already generated by the finite
  collision.

### G3 — source/constraint seam

- Prove \(\operatorname{rank}D_q=3\) on STF tensors for every symbolic
  \(q\ne0\), hence \(\dim\ker D_q=2\).
- Prove the oriented source (P4) is never in \(\ker D_q\).
- Prove (P7) preserves (P6) exactly.
- Prove that a general vector shift \(b\) preserves (P6) for all \(\Pi,\kappa\)
  if and only if \(b=g_TS q\).
- Verify cubic covariance of \(S\), \(D_qS\), and the constraint.

### G4 — one reciprocal generator

- Derive equations (P11)--(P13) from (P10).
- Verify the complete Jacobian is symplectic.
- Verify

  \[
   \omega I+H(p)+E=\omega I'+H(p')
  \]

  exactly.
- Construct the exact inverse and rational fail-closed admission fixtures.
- Verify equation (P6) before and after both absorption and inverse emission.

### G5 — two-mode homogeneous boundary

For every nonzero exact derivative symbol:

- the source-free constraint owner \(\kappa=0\) gives
  \(\Pi\in\ker D_q\), of dimension two;
- imposing the corresponding coordinate constraint on the STF coordinate
  gives two configurations plus two conjugate momenta;
- the scalar and vector owners remain explicit nonradiative constraint-sector
  records rather than being counted as tensor polarizations.

This is a kinematic/source-seam reduction, not a derivation of their dynamics.

### G6 — equal-coupling obstruction

The exact seam identities must pass for arbitrary independent
\((g_0,g_T)\). Therefore symplecticity, energy, constraint preservation, and
two-mode counting do not force \(g_0=g_T\). The certificate must also verify
that canonical rescaling of the scalar or tensor owner changes the displayed
source coefficient while preserving the seam algebra.

Consequently universal scalar/tensor coupling, the static pole, lensing, and
the gravitational strength remain action-normalization questions.

## 7. Outcome classes

- **Outcome A:** the seam passes and the frozen finite C18 collision itself
  derives the charge-even spare-vector action, \(g_0=g_T\), constraint
  dynamics, and the two-mode pole.
- **Outcome B:** the exact existing-type seam and two-mode boundary pass, but
  charge parity, constraint dynamics, equal coupling, or the finite lift
  remains selected/open.
- **Outcome C:** a canonical seam exists only after adopting a new spatial
  representation type.
- **Outcome D:** an integrity or exact algebra gate fails.

The expected honest ceiling is Outcome B.

## 8. Interpretation firewall

A pass may establish:

- no new **spatial irrep** is needed for one scalar, STF pair, EM vector, and
  separate vector-constraint owner;
- one local canonical generator can book packet recoil and all three gravity
  source records without violating energy or the inverse;
- the vector source shift is uniquely forced by local constraint
  preservation; and
- the homogeneous STF sector has exactly two configuration modes.

A pass may not claim:

- the finite C18 collision already assigns the spare vector its required
  charge-even ownership;
- the constraint multiplier/action is native;
- the scalar/vector sector has the required elliptic static pole;
- the tensor modes are an emergent graviton;
- light bends or experiences Shapiro delay;
- \(g_0=g_T\) or any gravitational strength is derived;
- the physical Born pushforward is closed; or
- a native \(\alpha\) has been measured.
