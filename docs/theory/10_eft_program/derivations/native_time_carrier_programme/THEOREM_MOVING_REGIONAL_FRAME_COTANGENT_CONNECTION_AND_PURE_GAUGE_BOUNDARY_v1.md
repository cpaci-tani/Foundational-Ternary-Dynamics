# Moving regional-frame cotangent connection and pure-gauge boundary

**Identifier:** `FTD-0970`  
**Status:** `[THEOREM — UNIQUE PASSIVE COTANGENT CONNECTION/RECIPROCAL MOMENTUM SHIFT]` +
`[THEOREM — REGULAR-CHART MAURER--CARTAN FLATNESS/TRIVIAL CLOSED HOLONOMY]` +
`[OPEN — ACTIVE DISCRETE GEARBOX/DEGENERACY PORT/PRODUCTION]`  
**Date:** 2026-08-11

## 1. Result

The FTD-0969 regional frame has an exact and unique canonical moving-frame
lift. Its connection couples body momentum to the angular momentum of every
field pair and automatically exposes the reciprocal momentum correction.

That connection is nevertheless a pure-gauge Maurer--Cartan form. It has zero
curvature on every smooth regular chart, identity holonomy around any closed
loop for which the single-valued body frame returns to itself, and no token or
work port. A finite passive frame jump is symplectic but leaves the physical
lab fields unchanged.

Therefore the body frame derives the kinematic connection hardware, not the
active FTD-0963 quarter-turn gearbox. Active exchange across a smooth path or
a `kappa=0` transition remains a separately declared dynamical law with
reaction, work, history, reserve, and inverse.

## 2. Unique cotangent lift

Let `s=(s^A)` coordinatize one smooth regular body stratum and let the
FTD-0969 frame be

\[
 F(s)=[e_1(s),e_2(s),e_3(s)]\in O(3),qquad
 A_A=F^T\partial_AF.                                      \tag{1}
\]

For each polar field pair, write

\[
 q_j=Fy_j,qquad p_j=Fz_j.                                \tag{2}
\]

Differentiating (2) in the canonical one-form gives

\[
 \sum_jp_j^Tdq_j+P_A ds^A
 =\sum_jz_j^Tdy_j+
 \left(P_A+\sum_jz_j^TA_Ay_j\right)ds^A.                 \tag{3}
\]

Hence the unique new canonical body momentum is

\[
 \Pi_A=P_A+G_A,qquad
 G_A=\sum_jz_j^TA_Ay_j.                                  \tag{4}
\]

Uniqueness follows coefficient by coefficient in `ds^A`: once (2) and the
body coordinates are fixed, any other correction would violate equality of
the canonical one-forms.

If the old body kinetic term is positive,

\[
 T={1\over2}P^TM^{-1}P,
\]

then the moving chart gives the exact complete square

\[
 T={1\over2}(\Pi-G)^TM^{-1}(\Pi-G).                       \tag{5}
\]

This resembles the selected FTD-0963 connection for a principled reason:
canonical moving frames require covariant momentum. It does not identify the
two profiles or derive the selected FTD-0963 normalization.

## 3. Angular-momentum form of the reaction

Orthogonality implies

\[
 A_A^T=-A_A.                                               \tag{6}
\]

Write `A_Av=omega_A cross v`. Then

\[
 G_A=\omega_A\mathbin{\cdot}
       \sum_j(y_j\mathbin{\times}z_j).                    \tag{7}
\]

The conjugate body momentum therefore contains exactly the field angular
momentum seen by the moving body frame. This is the reciprocal bookkeeping
term that a state-dependent projection would miss if it merely rotated field
components after every update.

For the planar witness `F(theta)=R_z(theta)`,

\[
 A=J_z,qquad G=L_z=y_1z_2-y_2z_1,qquad
 H={ (\Pi-L_z)^2\over2M}+h(y,z).                           \tag{8}
\]

The full nonlinear eight-dimensional map

\[
 (\theta,\Pi,y,z)\mapsto
 (\theta,P=\Pi-L_z,q=Fy,p=Fz)                             \tag{9}
\]

has unit Jacobian determinant and preserves the canonical symplectic form
exactly. For rotationally invariant `h`, `Pi`, `L_z`, and `P=Pi-L_z` are all
constant. The passive connection therefore leaves bare mechanical motion
unchanged and transfers no token.

## 4. Flatness and holonomy theorem

Because `A=F^TdF` is the pullback Maurer--Cartan form,

\[
 dA+A\wedge A=0,
\]

or in components

\[
 \partial_AA_B-\partial_BA_A+[A_A,A_B]=0.                 \tag{10}
\]

The certificate verifies (10) exactly on the noncommuting two-parameter
frame `F=R_z(alpha)R_y(beta)`, so this is not an artifact of an Abelian planar
example.

Keeping the lab vector fixed while the body frame moves gives

\[
 \dot y=-A_ty,qquad A_t=F^T\dot F,
\]

with exact transport

\[
 y(t_1)=F(t_1)^TF(t_0)y(t_0).                             \tag{11}
\]

Consequently:

- endpoint transport composes exactly;
- reverse traversal is the exact inverse; and
- if a closed regular path returns to the same single-valued frame, its
  holonomy is identity.

Thus the snapshot frame cannot generate a nonzero closed `pi/2` exchange
merely by being carried around. A nontrivial active holonomy requires
additional connection curvature, a non-single-valued chart transition, or a
physical coupling/port that is not contained in `F^TdF`.

## 5. Finite discrete frame changes are passive

Between two regular snapshots define

\[
 R=F_+^TF_-,qquad y_+=Ry_-,qquad z_+=Rz_-.                \tag{12}
\]

Since `R` is orthogonal, `diag(R,R)` is symplectic for every complete field
pair, has exact inverse `diag(R^T,R^T)`, and obeys

\[
 F_+y_+=F_-y_-,qquad F_+z_+=F_-z_-.                      \tag{13}
\]

The physical lab coordinate and momentum have not changed. Equation (12) is
a coordinate update, not an energy exchange, record actualization, or token
loading event.

An active lab-frame rotation must instead specify a Hamiltonian generator or
an exact discrete symplectic transaction, along with the body/port reaction,
work, finite reserve, backpressure, and inverse.

## 6. Degeneracy and lattice boundary

The differentiable derivation applies only on a fixed regular stratum. At
`kappa=0`, the FTD-0969 frame and `F^TdF` are undefined. Actual ternary
supports also change by discrete site events rather than by an already-given
continuous canonical body coordinate.

Therefore neither equation (4) nor equation (12) defines how production must
cross a frame degeneracy or change support. A reversible transition record or
explicit work/history port is required; otherwise distinct support paths can
be silently identified and the reaction ledger is incomplete.

## 7. Certificate

- protocol SHA-256:
  `5222BE4E93A244871EB656DFA7AF9D502210DFE0F5C8A915A4C2BCA689E92BAC`;
- proof SHA-256:
  `7FF0AC6E0D51B4F135E0897100D604B6AF71D7ED03F042B3986D670E7260E54B`;
- first immutable execution: `45/45`, Outcome B;
- exact non-Abelian curvature, full nonlinear Jacobian, transport, inverse,
  and discrete-jump gates all pass;
- no repair and no production mutation.

## 8. Scope firewall

This theorem does not derive:

- the selected FTD-0963 connection profile or its `pi/2` normalization;
- a nontrivial closed-loop active holonomy;
- autonomous formation, persistence, or a crossing rule at `kappa=0`;
- a token source, work port, reserve, backpressure, routing, or recycling;
- complete positive phase-error export or attraction;
- `G*` synchronization or the CM-prime/substrate gearbox;
- Born/Bell recovery or preferred-tick hiding; or
- production integration or whole-framework completeness.

The next admissible branch must choose between two explicit mechanisms: a
curved active connection with a source/reaction ledger, or a reversible
singular/chart-transition event whose retained record carries the missing
holonomy. The passive body frame cannot be counted twice as either one.
