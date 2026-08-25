# C18 common-phase tensor doublet and constraint price v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT FINITE-ALPHABET RANK-TWELVE TENSOR DOUBLET]** +
**[THEOREM — NATIVE C4 COMPLEX STRUCTURE AND SYMPLECTIC-FORM PRESERVATION]** +
**[SELECTION — CANDIDATE GRAVITY PHASE-SPACE INTERPRETATION]** +
**[OPEN — NATIVE BRACKET, CONSTRAINTS, KINETIC POLE, SOURCING, LENSING]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_common_phase_tensor_doublet.py](../../../../../scripts/proofs/proof_c18_common_phase_tensor_doublet.py)
derives the antipodal common-quadrature covariance, both symmetric-tensor
covariances, their joint rank, the C4 complex structure, symplectic-form
preservation, and covariance invariance using rational arithmetic only.

---

## 1. Motivation from the collision failure

The
[two-record kernel theorem](THEOREM_C18_TWO_RECORD_LINEARIZED_KERNEL_AND_TENSOR_BOUNDARY_v1.md)
closes the selected binary collision by itself as a gapless tensor carrier.
The capacity tensor is a single six-component response variable, and its
shears relax rather than form protected modes.

That does not exhaust the phase-complete C18 alphabet. Each antipodal line
already contains two independent common C4 quadratures. Blocking both against
the same Moore dyads produces two symmetric tensors without introducing a new
continuous microscopic field.

---

## 2. Finite definition

For each of the nine antipodal C18 lines $\ell$, let the directed records have
finite coordinates

\[
 \lambda_{\ell,\pm}=u_{\ell,\pm}+iv_{\ell,\pm}
 \in\{0,1,i,-1,-i\}.                                \tag{1}
\]

Define the even common quadratures

\[
 \bar u_\ell={u_{\ell,+}+u_{\ell,-}\over2},
 \qquad
 \bar v_\ell={v_{\ell,+}+v_{\ell,-}\over2}.        \tag{2}
\]

Let $M_\ell=d_\ell d_\ell^T$ be the normalized symmetric line dyad used by
the bare C18 blocking theorem. Define

\[
 Q={1\over9}\sum_{\ell=1}^{9}\bar u_\ell M_\ell,
 \qquad
 P={1\over9}\sum_{\ell=1}^{9}\bar v_\ell M_\ell.  \tag{3}
\]

Both $Q$ and $P$ are even symmetric rank-two block variables with six cubic
components

\[
 A_{1g}\oplus E_g\oplus T_{2g}.                    \tag{4}
\]

Equation (3) is a finite-frequency block map, not an ontically real tensor at
one voxel.

---

## 3. Exact rank-twelve covariance

Under the uniform five-state alphabet, one antipodal line has

\[
 \operatorname{Cov}(\bar u,\bar v,\bar c)
 =\operatorname{diag}\left({1\over5},{1\over5},{2\over25}\right), \tag{5}
\]

where $\bar c$ is the averaged blank-capacity coordinate. The phase
quadratures and capacity are uncorrelated at this bare quadratic order.

In symmetric coordinates $(xx,yy,zz,xy,xz,yz)$,

\[
 \boxed{
 \operatorname{Cov}(Q)=\operatorname{Cov}(P)
 ={1\over810}
 \begin{pmatrix}
 4&1&1&0&0&0\\
 1&4&1&0&0&0\\
 1&1&4&0&0&0\\
 0&0&0&1&0&0\\
 0&0&0&0&1&0\\
 0&0&0&0&0&1
 \end{pmatrix}.}                                   \tag{6}
\]

Both matrices have exact rank six, their cross-covariance vanishes, and the
joint $(Q,P)$ covariance has exact rank twelve. Thus the existing finite
alphabet has enough blocked kinematic capacity for a six-coordinate tensor
and a six-coordinate partner.

---

## 4. Native quarter-turn on the tensor doublet

Global C4 phase advance multiplies equation (1) by $i$:

\[
 (u,v)\longmapsto(-v,u).                            \tag{7}
\]

Therefore equation (3) transforms as

\[
 \boxed{(Q,P)\longmapsto(-P,Q).}                    \tag{8}
\]

Let

\[
 \mathbb J=
 \begin{pmatrix}0&-I_6\\I_6&0\end{pmatrix},
 \qquad
 \Omega=
 \begin{pmatrix}0&I_6\\-I_6&0\end{pmatrix}.       \tag{9}
\]

The exact finite action obeys

\[
 \mathbb J^2=-I_{12},
 \qquad
 \mathbb J^T\Omega\mathbb J=\Omega,                \tag{10}
\]

and preserves the joint covariance. This supplies a native complex structure
with the correct algebraic form for a tensor coordinate/momentum doublet.

It does **not** yet prove that $Q$ and $P$ possess a microscopic Poisson
bracket. Symplectic-form preservation is a kinematic compatibility result;
the bracket and action must be derived from the finite transaction rule.

---

## 5. The exact structural gain

The previous capacity-only route had six positive tensor-response coordinates
but no independent conjugate response. Equations (3)--(10) show that no new
continuum tensor primitive is required merely to pay the twelve-dimensional
phase-space type price. The phase-complete C18 records already contain a
natural candidate doublet:

\[
 \text{common phase quadratures}
 \longrightarrow
 (Q_{ij},P_{ij}).                                   \tag{11}
\]

At the same time, the antipodal **relative** first moment supplies the vector
channel from the bare theorem. This sharpens the one-action architecture:

\[
 \begin{array}{rcl}
 \text{relative phase, first moment} &\to& \text{vector candidate},\\
 \text{common phase, second moment} &\to& \text{tensor doublet candidate},\\
 \text{blank complement} &\to& \text{capacity/work availability},\\
 \text{token ownership} &\to& \text{ternary manifestation}.
 \end{array}                                        \tag{12}
\]

All four rows use the same finite phase-complete record alphabet. Their common
dynamics is still open.

---

## 6. Constraint price for two tensor polarizations

A symmetric tensor coordinate and momentum contain twelve local phase-space
components. Two propagating configuration polarizations require four
phase-space dimensions. In a first-class constrained Hamiltonian realization,
each independent first-class constraint removes two phase-space dimensions.
The minimum count is therefore

\[
 {12-2N_{\rm FC}\over2}=2
 \quad\Longrightarrow\quad
 \boxed{N_{\rm FC}=4}.                              \tag{13}
\]

The familiar linearized-gravity pattern is one scalar plus one
three-component vector constraint. FTD may realize an equivalent structure,
but equation (13) is only the required count. Neither the constraint functions
nor their closed algebra follow from the C4 covariance.

The selected binary collision fails this test: it protects no $E_g$ or
$T_{2g}$ tensor mode. A viable larger action must generate four local
first-class constraints or an explicitly proven discrete equivalent while
retaining positive energy and a common causal cone.

---

## 7. Lensing and source requirements

Even a two-mode tensor pole is not yet gravity. The same action must make the
tensor doublet respond universally to the energy/work ledger of every stable
matter clock. A pass requires:

1. a native bracket or exact reversible finite substitute for $(Q,P)$;
2. four closed constraints and their gauge or redundancy interpretation;
3. a positive derivative transport law with exactly two gapless tensor modes;
4. universal reciprocal sourcing by matter energy and momentum;
5. modification of both material-clock propagation and the relative vector
   signal cone; and
6. a lensing observable derived before comparison with GR.

Without items 4--6, the tensor doublet would be an unsourced wave field rather
than spacetime/gravity.

---

## 8. Next locked gate

Construct one finite local transaction on the existing C18 alphabet whose
blocked linearization simultaneously:

- couples the relative vector moment to ternary charge continuity;
- couples the common tensor doublet to the shared capacity/work ledger;
- generates, rather than imposes, the four constraint generators; and
- includes the controlled manifestation inverse on the same owned token.

No coefficient may be chosen from $\alpha$, $G_N$, a desired light-bending
angle, or a target tensor dispersion. Until that gate passes, equation (11)
is a strong kinematic carrier result and not a gravity derivation.

The first shared source map is now certified in the
[actualization moment-source vertex](../common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md).
Moving one owned C4 token from reserve to a C18 bond produces simultaneous
increments in the relative vector, both tensors $(Q,P)$, the capacity tensor,
and neutral ternary endpoints. This supplies a common kinematic source vertex,
but not the missing tensor constraints, pole, universal composite response, or
lensing.

The four-constraint kinematic target is now explicit in the
[tensor-doublet TT reduction theorem](THEOREM_C18_TENSOR_DOUBLET_TT_REDUCTION_AND_DYNAMICAL_BOUNDARY_v1.md).
One scalar $Q$ constraint plus three spatial gauge directions and three $P$
momentum constraints plus one scalar gauge direction leave exactly two tensor
coordinates and two C4-rotated partners. The constraints and kinetic action
remain selected targets rather than consequences of the finite collision.

The later
[existing-type scalar/STF/vector-constraint seam](../common_action_mechanics_reciprocity/THEOREM_C18_EXISTING_TYPE_SCALAR_STF_VECTOR_CONSTRAINT_ABSORPTION_SEAM_AND_EQUAL_COUPLING_BOUNDARY_v1.md)
shows that the C18 odd line module contains a second \(T_{1u}\) vector copy
besides the electromagnetic channel. At blocked reference level it can own
the longitudinal record required by a local STF source, and exact constraint
preservation uniquely fixes its shift. The finite C18 collision still has not
assigned that copy a charge-even action or generated the constraint algebra.

The subsequent
[transverse finite constraint-bundle theorem](../common_action_mechanics_reciprocity/THEOREM_C18_TRANSVERSE_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_TWO_OWNER_BOUNDARY_v1.md)
assigns that copy an explicit retained-record, charge-even, EM-neutral update
for every transverse nearest-neighbor STF-divergence chart. The axial chart
requires two plane bundles; its \(D_4\) stabilizer forbids selecting one plane
from scalar C4 phase, and the current one-owner slice cannot execute both.
Thus transverse ownership is closed at blocked finite level while the full
constraint algebra and axial owner remain open.
