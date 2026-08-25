# C4 Born/radiation kernel separation and contextual-mixer boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT REAL REGULAR-REPRESENTATION SPLIT]** +
**[THEOREM — BORN/FIELD KERNEL ORTHOGONALITY]** +
**[SCOPED NO-GO — ONE PHASE GRAM CANNOT GIVE POSITIVE PORT EMISSION AND THE RAW BORN FORM]** +
**[SELECTION CANDIDATE — CONTEXT-COVARIANT OFF-DIAGONAL MIXER]** +
**[OPEN — NATIVE MIXER, WORK LEDGER, AND PHYSICAL BORN PUSHFORWARD]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no Born, coupling, or alpha claim promoted

**Exact certificate:**
[proof_c4_born_radiation_kernel_separation.py](../../../../../scripts/proofs/proof_c4_born_radiation_kernel_separation.py)
performs **3,828 exact checks**. It constructs the three central real C4
projectors, exhausts the symmetric invariant commutant, verifies the raw Born
form for all phase multiplicities from zero through four, checks the current
cotangent field readout on every native flag and all three layers, proves the
positive-emission incompatibility on the handoff family, and verifies a
four-context covariant mixer orbit. No measured probability, coupling,
master root, numerical eigensolver, or fitted coefficient enters.

---

## 1. Correction to the “one compatibility Gram” intuition

The
[directional-port coherence theorem](../charge_gauss_native_em/THEOREM_DIRECTIONAL_PORT_COHERENCE_METRIC_HANDOFF_AND_PHASE_COMPATIBILITY_BOUNDARY_v1.md)
correctly proved that the port records and declared symmetries leave a
three-parameter quadratic-form family. It then suggested that one literal
phase-compatibility Gram should control both field energy and Born counting.
The exact C4 comparison rules out that literal identification.

The corrected one-action requirement is:

> One native action must derive the **distinct C4 sector projectors**, the
> physical context that couples those sectors, and the corresponding work and
> event pushforwards. It must not install one shared scalar Gram where the two
> observables require orthogonal irreducible sectors.

This does not weaken the one-action criterion. It makes the required action
more precise.

---

## 2. Exact real C4 decomposition

Let (e_p), (p=0,1,2,3), be the four phase-address basis vectors and let
(S e_p=e_{p+1pmod4}). Define

\[
 u_0=(1,1,1,1)^{\mathsf T},\qquad
 u_2=(1,-1,1,-1)^{\mathsf T},                              \tag{1}
\]

and the real quadrature pair

\[
 c=(1,0,-1,0)^{\mathsf T},\qquad
 s=(0,1,0,-1)^{\mathsf T}.                                \tag{2}
\]

The three orthogonal central projectors are

\[
 \boxed{
 P_0={u_0u_0^{\mathsf T}\over4},\qquad
 P_2={u_2u_2^{\mathsf T}\over4},\qquad
 P_Q={cc^{\mathsf T}+ss^{\mathsf T}\over2}.}              \tag{3}
\]

They obey

\[
 P_i^{\mathsf T}=P_i,\quad P_i^2=P_i,\quad
 P_iP_j=0\ (i\ne j),\quad
 P_0+P_2+P_Q=I_4,                                         \tag{4}
\]

with ranks (1,1,2). Thus the real regular representation splits as

\[
 \mathbb R[C_4]\cong \mathbf1\oplus\mathbf1_{\rm alt}
 \oplus\mathbf2_Q.                                       \tag{5}
\]

This split is internal to the already used four-address carrier; it does not
add a new microscopic phase alphabet.

---

## 3. The Born and current field kernels are orthogonal

For raw equal-weight phase counts
(n=(n_0,n_1,n_2,n_3)^{\mathsf T}), the complex history amplitude is

\[
 Z=(n_0-n_2)+i(n_1-n_3).                                  \tag{6}
\]

Its exact quadratic form is

\[
 |Z|^2=n^{\mathsf T}K_Bn,
 \qquad
 \boxed{K_B=cc^{\mathsf T}+ss^{\mathsf T}=2P_Q.}          \tag{7}
\]

In particular,

\[
 (K_B)_{p,p+2}=-1.                                        \tag{8}
\]

By contrast, the current cotangent `layer_value` readout ignores the C4 phase
address. For the same flag in any two phase bands, its normalized phase Gram
is

\[
 \boxed{K_F=u_0u_0^{\mathsf T}=4P_0,}                     \tag{9}
\]

so

\[
 (K_F)_{p,p+2}=+1.                                       \tag{10}
\]

Equations (7) and (9) give the exact separation

\[
 \boxed{K_FK_B=0.}                                       \tag{11}
\]

The present field phase label is therefore a transported clock/address tag,
not yet the signed complex-amplitude phase used in equation (6).

---

## 4. Direct one-kernel identification forbids positive emission

The four-channel directional-port Gram uses (b) for the same-handedness,
opposite-phase-band cross weight. Exact port-to-free conservation gave

\[
 c=-a,\qquad
 \Delta H_{\rm emit}={b-a\over2}.                         \tag{12}
\]

Literal identification with the raw Born kernel forces \(b=-1\). On the
handoff-conserving section the four Gram eigenvalues then become

\[
 (0,0,2(1+a),2(1-a)).                                    \tag{13}
\]

Positive semidefiniteness requires (-1\le a\le1), while

\[
 \boxed{
 \Delta H_{\rm emit}=-{1+a\over2}
 =-{2(1+a)\over4}\le0,\qquad H_{\rm free}=0.}             \tag{14}
\]

Hence no positive-semidefinite, handoff-conserving member of the registered
port family can simultaneously use the raw Born opposite-phase coefficient
and produce positive emitted field work. This is a scoped no-go for a single
untyped phase Gram, not a no-go for one native action.

---

## 5. Why residual-pair compatibility does not evade the result

The prepared Born branch first performs opposite-phase cancellation:

\[
 (n_0,n_1,n_2,n_3)
 \longmapsto
 (|n_0-n_2|,|n_1-n_3|).                                  \tag{15}
\]

It then counts same-rail ordered pairs. Equation (15) is nonlinear. For
example, one (p=0) record and one (p=2) record separately each leave one
real residual, whereas their union leaves none. Therefore the positive
same-rail click predicate after cancellation is not the raw kernel (K_B)
with its signs erased. It is a quotient/pushforward performed after the
quadrature-sector cancellation.

The
[physical C4 actualization tape](../quantum_foundations/THEOREM_C4_PHYSICAL_BORN_ACTUALIZATION_TAPE_v1.md)
physically registers the resulting bright pairs on its prepared finite tape,
but the common action still has to generate equation (15), prepare the bank,
and connect its context to a detector without importing the target weights.

---

## 6. Exhaustive invariant-commutant result

The exact certificate enumerates the ten-dimensional real symmetric
(4\times4) matrix space and imposes (AS=SA). The solution space has
dimension three and is exactly

\[
 \boxed{A=\lambda_0P_0+\lambda_2P_2+\lambda_QP_Q.}        \tag{16}
\]

Every fixed C4-invariant quadratic action is therefore block diagonal across
the field-trivial and Born-quadrature sectors. It can weight both, but it
cannot convert one into the other.

This proves that actualization needs either a nonlinear interaction or a
context-carrying off-diagonal vertex. Calling the conversion “measurement”
does not supply that vertex.

---

## 7. Linear context-covariant mixer target

Choose one detector quadrature (c). The minimum symmetric mixer

\[
 D_0=u_0c^{\mathsf T}+cu_0^{\mathsf T}                    \tag{17}
\]

has

\[
 P_0D_0P_Q\ne0,\qquad [D_0,S]\ne0.                       \tag{18}
\]

A fixed detector context therefore breaks the free phase-address symmetry, as
it must to select a quadrature. But the complete context orbit

\[
 D_k=S^kD_0S^{-k},\qquad k=0,1,2,3,                       \tag{19}
\]

is exactly covariant:

\[
 SD_kS^{-1}=D_{k+1},\qquad \sum_{k=0}^3D_k=0.             \tag{20}
\]

Thus contextual sector mixing does not require a globally preferred C4
phase. The context must be a physical part of the transaction and transform
with it.

Equations (16)--(20) motivate, but do not adopt, the minimum C4 block

\[
 \mathcal A_{C4}[\psi,C]
 ={1\over2}\psi^{\mathsf T}
 (\lambda_0P_0+\lambda_2P_2+\lambda_QP_Q)\psi
 +g_C\,\psi^{\mathsf T}D(C)\psi.                         \tag{21}
\]

Here (D(C)) must be generated by the detector/matter geometry, and the same
transaction must book its work and reciprocal response. The coefficient
\(g_C\), the context dynamics, and the nonlinear event map are all open.

The exact
[paired-history phase-neutral actualization successor](THEOREM_C4_PAIRED_HISTORY_PHASE_NEUTRAL_ACTUALIZATION_SOURCE_VERTEX_v1.md)
realizes the alternative nonlinear route already allowed above. Two
quadrature histories have a unique normalized symmetric C4 contraction; after
reversible dark cancellation, its positive values control the manifested
token and common charge/stress source. Thus the linear mixer (D(C)) is not
required if the common action generates that paired-history interaction.
Context can then enter through physical outcome routing rather than a preferred
absolute phase.

---

## 8. Consequences for the one-action programme

The exact result replaces one vague missing “compatibility coefficient” with
three concrete requirements:

1. **Field sector:** derive the physical \(P_0\)-sector energy and its coupling
   to the spatial cotangent carrier, including the port-to-free handoff.
2. **History sector:** derive the \(P_Q\)-sector cancellation and residual-bank
   preparation that precede finite Born counting.
3. **Context vertex:** derive either \(D(C)\) or the now-certified nonlinear
   paired-history contraction, together with its material clock, reciprocal
   work, autonomous routing, and pushforward into exclusive manifested events.

The native electromagnetic work fraction still depends on the action-selected
field normalization, recoil inertia, and handoff. The Born frequency law still
depends on autonomous history preparation and detector renewal. The fine-
structure root is not compared to any coefficient here.

The conceptual gain is exact: electromagnetic coupling and contextual
actualization need not share one Gram, but they must be coupled sectors of one
transaction action. That is now the sharper common-action target.
