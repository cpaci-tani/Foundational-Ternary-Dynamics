# C4 field cocycle and minimum canonical suspension

**Identifiers:** `FTD-0973`, `FTD-0974`  
**Status:** `[THEOREM — EXACT Z4 DISCRETE CONNECTION CLASSIFICATION]` +
`[THEOREM — CARRIER SYMMETRY UNDERDETERMINES FIELD REPRESENTATION]` +
`[SELECTION — MINIMUM FAITHFUL POSITIVE CANONICAL SUSPENSION]` +
`[THEOREM, CONDITIONAL — EXACT QUADRANT CO-ROTATION/REACTION/ENERGY/INVERSE]` +
`[OPEN — PHYSICAL IDENTITY/FORMATION/SWITCHING/G*/PRODUCTION]`  
**Date:** 2026-08-12

## 1. Result

The retained FTD-0972 four-cycle does not uniquely determine how a continuous
field must respond. Every edge may carry a power of the real quarter-turn,
producing 256 exact reversible cocycles organized into four gauge-invariant
`Z_4` holonomy classes.

Requiring carrier-phase translation symmetry makes the edge label constant.
Those homogeneous maps are direct products: the carrier can coexist with the
trivial field action, the faithful `+J` action, the half-turn, or the faithful
`-J` action. The carrier structure alone therefore cannot promote one field
identification as derived.

The minimum continuous Hamiltonian realization needs one additional complete
controller phase/action pair. A selected positive complete square then gives
an exact faithful suspension:

\[
 H_{\rm susp}={ (A-I)^2\over2M}+\nu I,qquad
 I={Q^2+P^2\over2}.                                        \tag{1}
\]

It rotates the field by one quadrant in the interaction picture whenever the
controller advances one quadrant, conserves the full energy, has an exact
inverse, and books field action in the controller's canonical momentum. This
closes the reference coupling, not its substrate identity, autonomous
formation, switching implementation, or `G*` cadence.

## 2. Exact discrete connection classes

Let `k in Z_4` label the retained carrier phase and let

\[
 J=\begin{pmatrix}0&-1\\1&0\end{pmatrix},qquad J^2=-I_2. \tag{2}
\]

For any edge cochain

\[
 a=(a_0,a_1,a_2,a_3)\in Z_4^4,                              \tag{3}
\]

define

\[
 U_a(k,z)=(k+1,J^{a_k}z),qquad z=(Q,P)^T.                  \tag{4}
\]

Every `J^a` is orthogonal and symplectic. Hence every one of the `4^4=256`
maps (4) is bijective, preserves the field action

\[
 I={Q^2+P^2\over2},                                        \tag{5}
\]

and has inverse

\[
 U_a^{-1}(k,z)=(k-1,J^{-a_{k-1}}z).                        \tag{6}
\]

One full carrier cycle gives

\[
 U_a^4(k,z)=(k,J^m z),qquad
 m=\sum_{r=0}^3a_r\pmod4.                                  \tag{7}
\]

Thus an active discrete connection can possess a holonomy not present in the
uncoupled carrier.

## 3. Gauge classification

A phase-dependent redefinition of the fiber basis,

\[
 z_k\mapsto J^{b_k}z_k,                                    \tag{8}
\]

changes the edge labels by

\[
 a'_k=a_k+b_{k+1}-b_k\pmod4.                               \tag{9}
\]

The sum in (7) is unchanged because the `b` terms telescope. Conversely,
choosing

\[
 b_0=0,quad b_1=-a_0,quad b_2=-a_0-a_1,quad
 b_3=-a_0-a_1-a_2                                          \tag{10}
\]

puts every cocycle in the canonical form

\[
 (0,0,0,m).                                                 \tag{11}
\]

The exact discrete connection classes are therefore `Z_4`, labeled by their
full-cycle holonomy. Selecting an edge rule is not mere notation; only its
gauge-dependent distribution can be moved, while `m` remains physical within
the candidate model.

## 4. What carrier symmetry does and does not force

Let carrier translation act by `R(k,z)=(k+1,z)`. Then

\[
 RU_a=U_aR
 \quad\Longleftrightarrow\quad
 a_0=a_1=a_2=a_3=a.                                        \tag{12}
\]

For constant `a`,

\[
 U_a=T\times J^a,qquad (J^a)^4=I.                          \tag{13}
\]

This is homogeneous co-rotation rather than a phase-localized gate. The four
choices are all reversible and norm preserving. The faithful real
representations are

\[
 a=1\quad(+J),qquad a=3\quad(-J).                          \tag{14}
\]

Orientation exchanges them, but neither faithfulness nor carrier symmetry
chooses which physical field pair should realize the representation. The
trivial map `a=0` satisfies the same structural gates. This is a theorem of
underdetermination, not an obstruction to adopting (14).

## 5. Minimum fixed time-reversal boundary

Use the minimal field conjugation

\[
 \Theta(k,z)=(-k,Cz),qquad
 C=\operatorname{diag}(1,-1),qquad CJC=J^{-1}.              \tag{15}
\]

Exact covariance

\[
 \Theta U_a\Theta=U_a^{-1}                                 \tag{16}
\]

holds precisely when

\[
 a_0=a_3,qquad a_1=a_2.                                   \tag{17}
\]

There are 16 such cocycles, and their full-cycle holonomy is

\[
 m=2(a_0+a_1)\pmod4\in\{0,2\}.                             \tag{18}
\]

Therefore a net full-cycle `+J` or `-J` holonomy cannot coexist with this
minimal, vertex-independent reversal action. Odd holonomy requires additional
vertex-dependent reversal phase, a time-odd port, or explicit symmetry
breaking. Those are new data and must be priced.

This does not forbid a `J` rotation on each homogeneous stroke: four such
strokes have trivial full holonomy.

## 6. Why a canonical suspension costs one complete pair

A lone continuous phase coordinate has zero antisymmetric form and cannot be
a nondegenerate Hamiltonian controller. The minimum continuous suspension
therefore introduces one conjugate pair `(theta,A)`.

For one normalized field pair `(Q,P)`, adopt equation (1) as a **selected
reference law**, with

\[
 M>0,qquad \nu\ge0,qquad K=A-I.                           \tag{19}
\]

The coefficient of `I` is the minimum faithful `C4` identification: one
controller quadrant is represented by one field quadrant. Its value is not a
fit, but identifying the finite carrier with this continuous controller and
choosing the physical modes and scales remain selections.

The exact equations are

\[
 \dot\theta={K\over M},qquad \dot A=0,qquad
 \dot I=0,qquad \dot K=0,                                 \tag{20}
\]

and

\[
 {d\over dt}\binom QP=left({K\over M}-\nu\right)
 J\binom QP.                                                \tag{21}
\]

The complete-square relation

\[
 A=K+I                                                       \tag{22}
\]

is the reciprocal reaction ledger. At fixed canonical momentum `A`, added
field action changes the controller rate by `-1/M`; at fixed mechanical `K`,
the bare controller rate is unchanged.

## 7. Exact co-rotation

Let

\[
 R(\alpha)=e^{\alpha J}.
\]

Since `K` is constant, the exact field solution is

\[
 z(t)=R\!\left[\left({K\over M}-\nu\right)t\right]z(0).
                                                               \tag{23}
\]

In the interaction picture `w(t)=R(\nu t)z(t)`,

\[
 w(t)=R\!\left({K t\over M}\right)w(0)
     =R(\theta(t)-\theta(0))w(0).                           \tag{24}
\]

Therefore

\[
 \Delta\theta={\pi\over2}\Rightarrow w_1=Jw_0,qquad
 \Delta\theta=2\pi\Rightarrow w_1=w_0.                    \tag{25}
\]

Negative elapsed time gives the exact inverse. The Hamiltonian is autonomous
and nonnegative, and its full value is conserved. This is an active field
phase change, unlike the passive frame relabelling in FTD-0970.

## 8. The remaining physical price

Equation (1) is permanently coupled. It preserves field action, so the ideal
flow transfers phase without exporting energy or consuming a finite action
reserve. This makes the reference map clean, but it does not make switching
free.

Turning the interaction on or off, changing orientation, choosing only one
carrier edge, resetting a phase, or transferring action requires an enlarged
time-dependent/reversible transaction with work and history. None is supplied
by the finite four-state carrier.

The finite carrier becomes a stroboscopic section of this continuous flow
only after `(theta,A)` and the field identification are adopted. FTD-0965
shows existing-type capacity conditionally, not formation or production.

## 9. Certificate

- FTD-0973 protocol SHA-256:
  `6328CD0FCA455BB135F1642D9A85C4BADFB63C3A9DA070B3BC8765434E4F1E87`;
- immutable parent proof SHA-256:
  `B83F616681E1E27D2F9AE6F2F935403032E5FB536E8B6942D7157DB909C2A3B8`;
- first parent execution: `63/64`, Outcome D on one source-marker adjective;
- FTD-0974 repair protocol SHA-256:
  `F32E722B1C684A01C9A523282D8F178C5D9240D2BADCA4A90535FC7ABF5B7EE4`;
- repair wrapper SHA-256:
  `31A2D7040AF685F54D04A2A4B4A8213027ADD8A75B127DA00312D5FF9B0A845A`;
- repaired inherited certificate: `64/64`, Outcome B;
- repair integrity: `12/12`;
- no engine or production file changed.

## 10. Scope firewall

This theorem does not establish:

- that production forms or maintains the four-state carrier;
- the physical identity or normalization of either canonical pair;
- that the unchanged tick contains equation (1);
- free switching, phase reset, action transfer, positive export, or erasure;
- perturbative or repeated-map attraction;
- the duration of a quadrant or its `G*`/CM provenance;
- Born/Bell recovery or preferred-tick hiding; or
- production integration or whole-framework completeness.

The next admissible test is production representability of the **specific
minimum suspension** (1), including whether an existing regional pair can
supply `(theta,A)` and one orthogonal field pair without double-booking the
FTD-0963 modes. Only after that source/capacity audit may a switched or
phase-localized production transaction be preregistered.
