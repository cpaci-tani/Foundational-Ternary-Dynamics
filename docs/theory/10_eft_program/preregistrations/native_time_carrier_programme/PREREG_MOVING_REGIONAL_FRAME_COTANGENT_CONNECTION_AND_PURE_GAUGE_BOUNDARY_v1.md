# Pre-registration — Moving regional-frame cotangent connection and pure-gauge boundary v1

**Identifier:** `FTD-0970`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Does the FTD-0969 regional body frame itself determine the missing canonical
moving-frame reaction, and is that induced connection sufficient to realize
the active FTD-0963 quarter-turn gearbox?

The test must distinguish:

1. the passive cotangent lift forced by a state-dependent orthogonal chart;
2. reciprocal momentum reaction required by the canonical one-form;
3. an active physical exchange that changes lab-frame fields or records; and
4. a nontrivial closed-loop holonomy capable of loading a token.

No selected connection profile, `G*` value, Born weight, target outcome,
fitted tolerance, numerical search, or production mutation is permitted.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md` | `56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1` |
| `THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md` | `FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C` |
| `THEOREM_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md` | `100A5539A1116FD6BEC5ABF2B7CE7BA2C32DDA557564EC7C964CDF5877512739` |

No engine source, public state type, ontology rule, selector, clock law, or
production phase may change under this protocol.

## 3. Frozen smooth-stratum construction

Let `s=(s^A)` be continuous coordinates on one regular body stratum where the
FTD-0969 frame is differentiable, single-valued, and has fixed
`chi=det F in {+1,-1}`. Let

\[
 F(s)=[e_1(s),e_2(s),e_3(s)]\in O(3),qquad
 A_A=F^T\partial_A F.                                      \tag{1}
\]

For each existing polar canonical field pair `(q_j,p_j)`, introduce body
components

\[
 q_j=F y_j,qquad p_j=F z_j.                               \tag{2}
\]

The original body variables are `(s^A,P_A)`. Define

\[
 G_A=\sum_j z_j^T A_Ay_j,qquad
 \Pi_A=P_A+G_A.                                            \tag{3}
\]

The certificate must prove the exact canonical one-form identity

\[
 \sum_jp_j^Tdq_j+P_A ds^A
 =\sum_jz_j^Tdy_j+\Pi_A ds^A.                              \tag{4}
\]

It must also prove that (3) is the unique momentum correction compatible
with (2), fixed `s`, and equality (4).

For a positive body inverse-mass matrix `M^{-1}`, the body kinetic energy must
become

\[
 {1\over2}P^TM^{-1}P
 ={1\over2}(\Pi-G)^TM^{-1}(\Pi-G).                         \tag{5}

This is the natural complete square induced by the moving chart. Its sign is
a convention fixed by (2), not a fit to FTD-0963.

## 4. Connection and curvature gates

From `F^TF=I`, prove

\[
 A_A^T=-A_A.                                                \tag{6}
\]

For a local axial connection vector `omega_A` defined by
`A_Av=omega_A cross v`, prove

\[
 G_A=\omega_A\mathbin{\cdot}
      \sum_j(y_j\mathbin{\times}z_j).                       \tag{7}

The connection is the pullback Maurer--Cartan form and must satisfy

\[
 \partial_AA_B-\partial_BA_A+[A_A,A_B]=0.                  \tag{8}

The certificate must verify (8) exactly for a noncommuting two-parameter
three-dimensional frame, not only for one planar angle.

Parallel transport that leaves lab coordinates fixed obeys

\[
 \dot y=-A_t y,qquad A_t=F^T\dot F,                        \tag{9}
\]

and therefore

\[
 y(t_1)=F(t_1)^TF(t_0)y(t_0).                              \tag{10}

For every closed loop inside a single-valued regular chart with
`F(t_1)=F(t_0)`, its holonomy is identity. Reverse traversal gives the exact
inverse endpoint rotation.

## 5. Exact planar witness and reaction

Freeze the embedded planar frame

\[
 F(\theta)=\begin{pmatrix}
 \cos\theta&-\sin\theta&0\\
 \sin\theta& \cos\theta&0\\
 0&0&1
 \end{pmatrix},qquad A=F^TF'=J_z.                          \tag{11}
\]

For one field pair,

\[
 G=z^TJ_zy=y_1z_2-y_2z_1=L_z,qquad
 H={ (\Pi-L_z)^2\over2M}+h(y,z).                            \tag{12}

The certificate must verify the full nonlinear eight-dimensional Jacobian
condition for the transformation

\[
 (\theta,\Pi,y,z)\mapsto
 (\theta,P=\Pi-L_z,q=Fy,p=Fz).                              \tag{13}

For rotationally invariant `h`, prove `Pi`, `L_z`, and hence the mechanical
momentum `P=Pi-L_z` are constant. This is the bare-motion control: the passive
connection alone does not transfer a token or dissipate phase error.

## 6. Discrete-jump and degeneracy boundary

For two regular snapshots `F_-` and `F_+`, the passive coordinate update

\[
 y_+=F_+^TF_-y_-,\qquad z_+=F_+^TF_-z_-                    \tag{14}

must be orthogonal, symplectic for each complete pair, invertible, and leave
the lab variables `(q,p)` unchanged. Therefore it is a representation change,
not an active physical gearbox.

An active rotation of lab fields requires an independently specified
generator plus reciprocal body/port reaction. Equation (14) cannot be counted
as token loading, energy export, outcome selection, or production dynamics.

At the FTD-0969 `kappa=0` stratum the frame and its derivative are undefined.
The smooth connection does not specify a discrete ternary support jump across
that surface. Such a jump needs a reversible transition record or explicit
work/backpressure port and a fresh locked law.

## 7. Frozen checks

- **G1:** hashes and all scope/source markers;
- **G2:** exact one-form identity and uniqueness of the momentum shift;
- **G3:** nonlinear full-Jacobian symplecticity for the planar witness;
- **G4:** skew connection, angular-momentum generator, and complete square;
- **G5:** exact non-Abelian Maurer--Cartan flatness on a two-parameter frame;
- **G6:** endpoint parallel transport, identity closed-loop holonomy, and
  reverse inverse;
- **G7:** rotationally invariant bare-motion/no-transfer control;
- **G8:** discrete passive jump orthogonality, symplecticity, inverse, and
  unchanged lab fields;
- **G9:** active/passive, degeneracy, temporal, `G*`, Born, and production
  firewalls.

All algebra is exact. Floating comparisons and numerical scans are forbidden.

## 8. Frozen classifier

- **Outcome A — native active moving-frame gearbox:** G1--G9 pass and the
  derived connection itself produces a nontrivial closed-loop physical
  exchange with complete reaction and production provenance.
- **Outcome B — exact passive cotangent connection / active gearbox open:**
  the cotangent lift and reaction shift are exact, but the connection is flat,
  closed-loop holonomy is trivial on the regular chart, and active discrete
  dynamics remains a separately priced law.
- **Outcome C — no canonical moving-frame lift:** the regional frame cannot
  be lifted symplectically even on a regular stratum.
- **Outcome D — invalid:** any lock, exact identity, or scope gate fails.

The expected result is Outcome B. It derives the kinematic connection and
reciprocal momentum correction but does not promote the selected FTD-0963
profile, a token-loading law, or production dynamics.
