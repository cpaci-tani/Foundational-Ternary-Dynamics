# C18 existing-type scalar/STF/vector-constraint absorption seam and equal-coupling boundary v1

**Date:** 2026-08-24

**Status:** **[THEOREM — TWO C18 VECTOR COPIES AND EXISTING SPATIAL TYPE CAPACITY]** +
**[THEOREM — UNIQUE LOCAL VECTOR-CONSTRAINT SOURCE LOAD]** +
**[THEOREM — ONE SYMPLECTIC ENERGY-CONSERVING SOURCE/RECOIL GENERATOR]** +
**[THEOREM, CONDITIONAL — TWO HOMOGENEOUS STF CANONICAL MODES]** +
**[THEOREM — SEAM IDENTITIES DO NOT FORCE EQUAL SCALAR/TENSOR COUPLING]** +
**[OUTCOME B — EXACT REFERENCE SEAM, FINITE CONSTRAINT ACTION OPEN]** +
**[OPEN — AXIAL/DISTRIBUTED OWNERSHIP, STATIC POLE, LENSING, AND SCALE]**

**Production status:** unchanged

**Ledger status:** no FTD claim row minted

**Locked preregistration:**
[PREREG_C18_SCALAR_STF_VECTOR_CONSTRAINT_OWNERSHIP_ABSORPTION_SEAM_v1.md](../../preregistrations/common_action_mechanics_reciprocity/PREREG_C18_SCALAR_STF_VECTOR_CONSTRAINT_OWNERSHIP_ABSORPTION_SEAM_v1.md),
pre-execution SHA-256
85474A9D7FA5AD38B25208944204F469E73004F643D304ACB23F11F43AC69D47.

**Exact certificate:**
[proof_c18_scalar_stf_vector_constraint_absorption_seam.py](../../../../../scripts/proofs/proof_c18_scalar_stf_vector_constraint_absorption_seam.py),
SHA-256
29C6BC475F6DDFCC3FC73DA5683D0F14ECAC96233924084920F682389D1B1F6E,
performs 111,768 exact character, symbolic, signed-cubic, and rational
checks. It proves the C18 representation census, the two-copy vector basis,
the universal rank of the STF divergence map, the unique local constraint
load, full cubic covariance, a complete symplectic/energy/inverse generator,
two homogeneous tensor modes, fail-closed action admission, and the
equal-coupling normalization obstruction. No floating point, target gravity
coefficient, deflection angle, master root, or empirical coupling enters.

---

## 1. The missing constraint vector does not require a new spatial irrep

The nine unoriented C18 lines split into the three SC and six FCC lines. Their
inversion-odd line module has the exact signed-cubic decomposition

\[
 \boxed{V_{\rm C18,odd}=2T_{1u}\oplus T_{2u}.}             \tag{1}
\]

The two \(T_{1u}\) copies can be exhibited without a character-table
ambiguity. Let

\[
 J_{\rm SC},\qquad J_{\rm FCC}                            \tag{2}
\]

be the separate shell first moments. Both transform as spatial vectors.
The copy-basis change

\[
 \boxed{
 J_{\rm EM}=J_{\rm SC}+J_{\rm FCC},
 \qquad
 J_{\rm C}=J_{\rm SC}-J_{\rm FCC}}                       \tag{3}
\]

has determinant \(-8\) and commutes with every signed-cubic transformation.
Thus \(J_{\rm EM}\) and \(J_{\rm C}\) are independent vector slots.

The first slot is already used by the electromagnetic current construction.
The second has the correct **spatial** type to own the vector constraint
record required by a local STF source. This does not yet prove that the
finite C18 collision assigns \(J_{\rm C}\) the needed charge-even internal
parity or update law. Spatial representation capacity and finite ownership
are distinct claims.

The even C18 line module is

\[
 \boxed{V_{\rm C18,even}=2A_{1g}\oplus2E_g\oplus T_{2g}.} \tag{4}
\]

Its two common C4 quadratures each carry this module. Their second moments
therefore contain a scalar canonical pair and a five-component STF canonical
pair. At blocked type level the existing phase-complete alphabet contains:

\[
 \begin{array}{rcl}
 A_{1g}\text{ pair} &\to& \text{scalar trace owner},\\
 (E_g\oplus T_{2g})\text{ pair} &\to& \text{STF owner},\\
 T_{1u}^{(1)} &\to& \text{electromagnetic vector},\\
 T_{1u}^{(2)} &\to& \text{constraint vector}.
 \end{array}                                             \tag{5}
\]

No new spatial irrep is required for this minimum seam.

---

## 2. The local symmetric-stress source

For a packet batch of energy \(E\) moving on the SC ray \(r\), the conditional
symmetric-stress theorem gives

\[
 \Sigma_F=E\,rr^{\mathsf T}.                             \tag{6}
\]

Split it into trace and STF parts:

\[
 \rho_F=E,
 \qquad
 S=E\left(rr^{\mathsf T}-{\mathbf1\over3}\right).        \tag{7}
\]

For every SC ray,

\[
 \det S={2E^3\over27}\ne0.                               \tag{8}
\]

Thus \(S\) is never transverse to every nonzero derivative direction. A
local source cannot be loaded directly into a TT-only field.

Let \(q\ne0\) be an exact local derivative symbol and define the STF
divergence map

\[
 D_q(X)=Xq.                                             \tag{9}
\]

In a rational five-element STF basis its matrix is

\[
 D_q=
 \begin{pmatrix}
 q_1&0&q_2&q_3&0\\
 0&q_2&q_1&0&q_3\\
 -q_3&-q_3&0&q_1&q_2
 \end{pmatrix}.                                         \tag{10}
\]

Three explicit minors cover every nonzero real \(q\):

\[
 q_1(q_1^2+q_3^2),\qquad
 -q_2(q_2^2+q_3^2),\qquad
 -q_3(q_1^2+q_3^2).                                    \tag{11}
\]

Consequently

\[
 \boxed{\operatorname{rank}D_q=3,\qquad
 \dim\ker D_q=2\quad(q\ne0).}                           \tag{12}
\]

Equation (12) is the exact local two-configuration-mode count for an STF
field after its three divergence constraints.

---

## 3. Unique local vector-constraint load

Let \(\Pi\in\operatorname{STF}(3)\) be the tensor momentum owner and
\(\kappa\in\mathbb R^3\) the second-vector constraint owner. Define

\[
 \boxed{{\cal C}_q(\Pi,\kappa)=\Pi q-\kappa.}             \tag{13}
\]

Load the STF source with coefficient \(g_T\):

\[
 \Pi'=\Pi+g_TS.                                         \tag{14}
\]

Suppose the vector owner changes by an otherwise arbitrary \(b\). Then

\[
 {\cal C}_q(\Pi',\kappa+b)-{\cal C}_q(\Pi,\kappa)
 =g_TS q-b.                                            \tag{15}
\]

Therefore constraint preservation for every initial state is equivalent to

\[
 \boxed{b=g_TS q,}
\]

or

\[
 \boxed{\kappa'=\kappa+g_TS q.}                         \tag{16}
\]

This shift is not an adjustable second vector coupling. It is necessary and
sufficient once the tensor load \(g_TS\) is chosen.

Equation (16) is local: \(q\) is the symbol of a nearest-neighbor divergence.
No \(1/q^2\), inverse Laplacian, or TT projector occurs. The exact certificate
verifies

\[
 (gSg^{\mathsf T})(gq)=g(Sq)                            \tag{17}
\]

for every signed-cubic transformation \(g\), every registered nonzero
derivative fixture, and every SC source ray.

---

## 4. One reciprocal source/recoil generator

Collect the material, STF, scalar, and vector-constraint momenta into one
vector \(p\). Their source shift is

\[
 a=\left(
 6Er,\;
 g_TS,\;
 g_0E,\;
 g_TS q
 \right).                                               \tag{18}
\]

The four entries are respectively:

1. material recoil;
2. STF source ownership;
3. scalar trace ownership; and
4. the uniquely required longitudinal constraint record.

For any positive differentiable quadratic owner Hamiltonian \(H(p)\), define

\[
 \boxed{
 F_2(\theta,x;I',p')
 =\theta I'+x\cdot(p'-a)
 -{\theta\over\omega}
 \left[E+H(p'-a)-H(p')\right].}                        \tag{19}
\]

Exact differentiation gives

\[
 \boxed{p'=p+a,\qquad\theta'=\theta,}                   \tag{20}
\]

\[
 \boxed{
 I'=I+{E+H(p)-H(p+a)\over\omega},}                     \tag{21}
\]

\[
 \boxed{
 x'=x-{\theta\over\omega}
 [\nabla H(p)-\nabla H(p+a)].}                         \tag{22}
\]

At the physical seam \(\theta=0\), every canonical coordinate remains fixed
while recoil and all source owners update atomically.

The complete Jacobian obeys

\[
 M^{\mathsf T}\Omega M=\Omega,                          \tag{23}
\]

and equation (21) gives exact energy:

\[
 \boxed{
 \omega I+H(p)+E=\omega I'+H(p').}                     \tag{24}
\]

Inverse emission subtracts the same \(a\), restores every momentum and
source owner, reverses the off-seam coordinate reaction, and restores the
clock action. Nonnegative-action admission remains fail closed.

This is one generator, not four independently appended balance equations.
Its variables and coefficients are still selected blocked structures rather
than a derived finite C18 permutation.

---

## 5. Two homogeneous tensor modes and explicit constraint sectors

In a source-free region, put

\[
 \kappa=0.
\]

Equation (13) then requires

\[
 \Pi q=0.
\]

By equation (12), \(\Pi\) has exactly two independent components. Imposing
the corresponding coordinate constraint on the STF coordinate leaves

\[
 \boxed{
 2\text{ tensor configurations}
 +2\text{ conjugate momenta}.}                          \tag{25}
\]

The scalar and vector owners in equation (18) are not counted as additional
tensor polarizations. For a persistent source they retain the nonradiative
trace and longitudinal records that a TT-only field cannot own.

Equation (25) closes the source-seam/type/count problem conditionally. It does
not generate the second-order tensor transfer, the vector/scalar constraint
dynamics, or their static Green functions. The existing even-STF action
remains a conditional radiative target.

---

## 6. Equal scalar/tensor coupling is not forced at the seam

The scalar load in equation (18) has coefficient \(g_0\); the STF and its
forced vector record share \(g_T\). Every symplectic, energy, inverse,
constraint, and mode-count identity above is valid for arbitrary independent

\[
 (g_0,g_T).                                             \tag{26}
\]

Hence

\[
 \boxed{\text{seam closure does not imply }g_0=g_T.}     \tag{27}
\]

This is not merely an omitted algebraic step. A canonical owner rescaling

\[
 q\mapsto\lambda q,\qquad
 p\mapsto{p\over\lambda}                                \tag{28}
\]

preserves the canonical two-form while changing the displayed source
coefficient. Independent scalar and tensor rescalings therefore change
\(g_0/g_T\) without changing equations (13), (20), (23), or (24).

The common finite action must fix the kinetic normalizations and couple the
full stress with one derived coefficient before equal scalar/tensor response
can be claimed. Stress geometry alone does not do so.

---

## 7. Gravity and lensing boundary

The reference chain is now

\[
 \begin{aligned}
 \text{packet stress}
 &\longrightarrow
 (\rho_F,S,Sq)\\
 &\xrightarrow{F_2}
 \text{scalar owner + STF owner + vector constraint owner}\\
 &\longrightarrow
 \text{two homogeneous STF canonical modes}.
 \end{aligned}                                          \tag{29}
\]

This is the first exact absorption seam that accepts the **full local**
symmetric stress without a nonlocal TT projection and without adding a new
spatial representation type.

Native gravity still requires:

1. a finite axial two-owner or distributed rule for the spare \(T_{1u}\)
   copy (the transverse rule is closed by the successor);
2. derivation of equation (19) from the local C18 transaction;
3. a closed scalar/vector constraint action;
4. a nonzero static scalar Green response;
5. the positive two-mode tensor pole from the same update;
6. one action-derived normalization forcing the scalar, tensor, clock, and
   Maxwell readouts;
7. universal response of stable composite matter; and
8. blind deflection and Shapiro-delay evaluation.

Until these pass, production lensing remains class zero and the two-mode
sector is not a native graviton.

---

## 8. Contextual measurement and electromagnetic coupling

Because the source is the same packet generated by the routed manifestation
event, equation (29) can append scalar/STF/constraint records to the existing
clock/recoil apparatus record without irreversible erasure. It does not form
the Born history bank, prove generic basin measure, or close multipartite
no-signalling.

Likewise, equation (19) does not fix the electromagnetic work curvature.
The scalar/tensor normalization freedom is structurally analogous to the
remaining field/clock normalization in the native-alpha protocol. No
fine-structure value or comparison with the master root follows.

---

## 9. Epistemic disposition

### Established exactly

- \(V_{\rm C18,odd}=2T_{1u}\oplus T_{2u}\);
- the two shell vectors give independent cubic-covariant vector copies;
- the even module supplies scalar and STF spatial types to both C4
  quadratures;
- the STF divergence has rank three at every nonzero real symbol;
- the local event stress is never TT;
- equation (16) is the unique constraint-preserving vector load;
- one type-2 generator produces all source/recoil shifts;
- the complete map is symplectic, energy conserving, and invertible;
- the homogeneous constrained STF phase space has four dimensions; and
- the seam identities do not force \(g_0=g_T\).

### Still selected or open

1. finite axial two-owner or distributed use of the spare vector copy
   (transverse use is closed by the successor);
2. microscopic derivation of the scalar/STF/constraint ownership map;
3. native constraint algebra and multipliers;
4. static scalar/vector response and universal coupling;
5. positive native tensor dynamics at the common cone;
6. equal normalized scalar/tensor/clock/Maxwell response;
7. lensing, Shapiro delay, and nonlinear gravity;
8. autonomous physical Born preparation and multipartite measurement; and
9. native electromagnetic coupling and the value of \(\alpha\).

The preregistered disposition is **Outcome B**: an exact existing-type
reference seam and two-mode boundary, while its finite constraint action and
physical normalizations remain open.

---

## 10. Subsequent finite-bundle discriminator

The locked
[C18 transverse charge-even constraint-bundle successor](THEOREM_C18_TRANSVERSE_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_TWO_OWNER_BOUNDARY_v1.md)
now realizes the spare vector by an explicit retained-record map. One SC
orientation reversal plus two reserve FCC activations gives

\[
 \Delta J_{\rm EM}=0,\qquad\Delta J_{\rm C}=4r,
\]

is charge even in the registered internal-conjugation model, commutes with
global C4 advance, and is covariant under all 48 signed-cubic transformations.
For every transverse nearest-neighbor derivative it realizes the exact
blocked load \(216T_rq\).

The axial branch is an exact boundary rather than a completion. Its ordered
data have a \(D_4\) stabilizer that exchanges the two transverse plane choices,
so scalar C4 phase cannot choose one plane equivariantly. The full axial load
requires both plane bundles and therefore two independently owned SC records,
or a distributed/time-shared equivalent with retained history. The current
one-owner local slice cannot execute that pair atomically.

The later
[Hodge-framed all-axis signed-event theorem](THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
constructs that two-owner repair using the existing electromagnetic Hodge
flag's \(hn\) and \(r\times n\) axes. It covers every transverse and axial
chart and composes the finite records with manifestation, current,
trace/STF/constraint loading, recoil, clock action, event energy, and the full
port reaction in one exact signed generator.

This closes the blocked prepared-source action, but not the native constraint
dynamics, static and tensor poles, relative normalization, or blind lensing
tuple.
