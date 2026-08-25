# C18 tensor-doublet TT reduction and dynamical boundary v1

**Date:** 2026-08-23
**Status:** **[THEOREM — NONZERO-MODE FOUR-CONSTRAINT REDUCTION TO TWO TENSOR POLARIZATIONS]** +
**[THEOREM — TT PROJECTOR COMPATIBLE WITH NATIVE C4 COMPLEX STRUCTURE]** +
**[SELECTION — CONSTRAINT FUNCTIONS ARE TARGET STRUCTURE, NOT ACTION-DERIVED]** +
**[OPEN — CONSTRAINT GENERATION/ALGEBRA, POSITIVE KINETIC POLE, UNIVERSAL SOURCE, LENSING]**
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_tensor_doublet_tt_reduction.py](../../../../../scripts/proofs/proof_c18_tensor_doublet_tt_reduction.py)
constructs all constraint, gauge, and TT maps with rational arithmetic for all
98 nonzero primitive integer wavevectors in the cube $[-2,2]^3$. It verifies
ranks, annihilation relations, idempotence, Frobenius self-adjointness, the
joint rank-four phase-space projector, C4 compatibility, and the axis
plus/cross basis in 1,572 exact checks. The proof below establishes the rank
statements for every nonzero wavevector.

---

## 1. Native tensor phase-space carrier

The
[common-phase tensor-doublet theorem](THEOREM_C18_COMMON_PHASE_TENSOR_DOUBLET_AND_CONSTRAINT_PRICE_v1.md)
constructs two independent symmetric block tensors

\[
 (Q_{ij},P_{ij})                                    \tag{1}
\]

from the two common C4 quadratures. Their joint covariance has rank twelve,
and global phase advance acts as

\[
 (Q,P)\longmapsto(-P,Q).                            \tag{2}
\]

The missing kinematic question is whether four first-class constraints or an
equivalent reduction can leave exactly two tensor configuration modes. This
document gives the minimum nonzero-wavevector target explicitly.

---

## 2. Four constraint/gauge pairs

Let $k\ne0$ be a real spatial wavevector and $k^2=k_ik_i$. On $Q$, define the
scalar constraint

\[
 \mathcal H_Q(k)
 =\bigl(k_i k_j-k^2\delta_{ij}\bigr)Q_{ij}=0,       \tag{3}
\]

with three spatial gauge directions

\[
 \delta_\xi Q_{ij}=k_i\xi_j+k_j\xi_i.              \tag{4}
\]

Equation (3) annihilates equation (4). Indeed,

\[
 (k_i k_j-k^2\delta_{ij})
 (k_i\xi_j+k_j\xi_i)=0.                            \tag{5}
\]

The map $\xi\mapsto\delta_\xi Q$ is injective for $k\ne0$: resolving $\xi$
into components parallel and transverse to $k$, equation (4) can vanish only
when both components vanish. Thus it has rank three. Equation (3) is a nonzero
rank-one functional, so

\[
 \dim\ker\mathcal H_Q-\dim\operatorname{im}\delta_\xi
 =5-3=2.                                            \tag{6}
\]

On $P$, impose the three momentum/divergence constraints

\[
 \mathcal H_{P,i}(k)=P_{ij}k_j=0,                  \tag{7}
\]

and quotient the scalar gauge direction

\[
 \delta_\eta P_{ij}
 =\eta\bigl(k_i k_j-k^2\delta_{ij}\bigr).          \tag{8}
\]

Equation (8) lies in the kernel of equation (7). The map
$P\mapsto Pk$ is surjective for $k\ne0$: for any vector $y$, the symmetric
tensor

\[
 P={yk^T+ky^T\over k^2}
 -{(y\cdot k)kk^T\over k^4}                        \tag{9}
\]

satisfies $Pk=y$. Hence equation (7) has rank three. Equation (8) is one
nonzero direction, giving

\[
 \dim\ker\mathcal H_P-1=3-1=2.                    \tag{10}
\]

Together, equations (6) and (10) leave two tensor coordinates and two tensor
partners: four physical phase-space dimensions.

---

## 3. Exact transverse-traceless projector

Define the transverse projector

\[
 \Pi_{ij}=\delta_{ij}-{k_i k_j\over k^2}.           \tag{11}
\]

For any symmetric tensor $X$, define

\[
 \boxed{
 (\mathcal P_{\rm TT}X)_{ij}
 =\Pi_{ia}X_{ab}\Pi_{bj}
 -{1\over2}\Pi_{ij}\Pi_{ab}X_{ab}.}                \tag{12}
\]

Direct algebra gives

\[
 \mathcal P_{\rm TT}^2=\mathcal P_{\rm TT},
 \qquad
 \operatorname{rank}\mathcal P_{\rm TT}=2,        \tag{13}
\]

\[
 k_j(\mathcal P_{\rm TT}X)_{ij}=0,
 \qquad
 \delta_{ij}(\mathcal P_{\rm TT}X)_{ij}=0.         \tag{14}
\]

It kills all three $Q$ gauge directions in equation (4) and the scalar $P$
gauge direction in equation (8), and its image satisfies both constraint
families. It is self-adjoint under the symmetric-tensor Frobenius pairing.

For $k\parallel e_z$, a basis of the image is

\[
 X_+=\operatorname{diag}(1,-1,0),
 \qquad
 X_\times=
 \begin{pmatrix}0&1&0\\1&0&0\\0&0&0\end{pmatrix}. \tag{15}
\]

These are the two usual plus/cross tensor types, now as block modes of the C18
common-phase doublet rather than a separately posited field.

---

## 4. Compatibility with the native C4 doublet

Apply equation (12) independently to $Q$ and $P$:

\[
 \mathcal P_{\rm phys}
 =\operatorname{diag}(\mathcal P_{\rm TT},
                       \mathcal P_{\rm TT}).        \tag{16}
\]

Then

\[
 \operatorname{rank}\mathcal P_{\rm phys}=4,       \tag{17}
\]

and, for the native complex structure

\[
 \mathbb J=
 \begin{pmatrix}0&-I_6\\I_6&0\end{pmatrix},       \tag{18}
\]

one has

\[
 \boxed{
 \mathcal P_{\rm phys}\mathbb J
 =\mathbb J\mathcal P_{\rm phys}.}                 \tag{19}
\]

Thus the two physical tensor coordinates retain their two C4-rotated partners.
The native quarter-turn does not take the reduced sector out of itself.

---

## 5. Conditional positive gapless action

If the finite transaction action generates equations (3), (4), (7), and (8)
as a closed first-class structure or exact discrete equivalent, the minimum
positive quadratic physical Hamiltonian has the form

\[
 H_{\rm TT}
 ={1\over2}\sum_{k\ne0}
 \left[
 \|P_{\rm TT}(k)\|_F^2
 +c_T^2\widehat{k}^{,2}\|Q_{\rm TT}(k)\|_F^2
 \right],                                          \tag{20}
\]

giving two positive gapless modes

\[
 \omega^2(k)=c_T^2\widehat{k}^{,2}.                \tag{21}
\]

Equations (20)--(21) are a **conditional target**, not a derived FTD action.
The coefficient $c_T$, lattice derivative $\widehat k$, constraint algebra,
and source law must all follow from the finite update. Choosing them to match
GR or the vector light cone would be an insertion.

---

## 6. Why this is not yet lensing or gravity

The TT reduction solves only the propagating type/count problem. Static
gravity and lensing also depend on the constrained scalar/vector sectors and
their universal coupling to matter energy and momentum. The
[shared source vertex](../common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md)
inserts rank-one tensor moments, but the action has not solved the constraints
they source or shown that stable composite clocks follow the resulting
effective geometry.

A gravity pass still requires:

1. derivation of equations (3)--(8) from finite conservation and redundancy;
2. closure of their discrete constraint algebra under the actual tick;
3. derivation of equation (20) with positive energy and the common signal cone;
4. reciprocal universal sourcing by the energy/work of every material clock;
5. a static long-distance potential; and
6. deflection and delay of the relative-vector signal by the same source.

Only item 6 is a lensing test. Two TT modes alone do not imply it.

---

## 7. Exact epistemic conclusion

The C18 alphabet no longer lacks a place for spin-2/equivalent kinematics:

\[
 \text{finite common C4 records}
 \longrightarrow(Q,P)
 \longrightarrow(Q_{\rm TT},P_{\rm TT})
 \quad\text{with two polarizations}.               \tag{22}
\]

What remains open is the physics: the native action must generate the four
constraints and their pole. Until then, equation (22) is an exact carrier and
reduction theorem, not an emergent graviton or recovery of GR.

The later
[cotangent STF parity/curl theorem](THEOREM_COTANGENT_STF_PARITY_PRICE_AND_SPIN2_CURL_TARGET_v1.md)
now supplies the missing first-derivative target. It proves that an
inversion-even/odd STF pair is required, constructs the unique isotropic
symmetric curl, and verifies that the TT projector is invariant with
$\mathcal C_k^2=-|k|^2I_{\rm TT}$. Conditional reuse of the cotangent
incidence rate gives the same $1/6$ cone as Maxwell. The finite staggered
permutation and action-derived constraints remain open.
