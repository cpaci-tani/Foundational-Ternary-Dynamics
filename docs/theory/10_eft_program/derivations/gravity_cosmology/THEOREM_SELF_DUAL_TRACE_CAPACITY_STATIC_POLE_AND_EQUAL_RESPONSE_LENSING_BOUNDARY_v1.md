# Self-dual trace-capacity static pole and equal-response lensing boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — UNIQUE MANIFESTATION TRACE/STF SOURCE SPLIT]** +
**[THEOREM, CONDITIONAL — SELF-DUAL CAPACITY ACTION HAS ONE MASSLESS STATIC MODE]** +
**[THEOREM, CONDITIONAL — EQUAL TRACE SOURCE GIVES \(U_t=U_s\)]** +
**[SELECTION — SELF-DUAL SOURCE COUPLING AND NORMALIZED MINIMAL READOUTS]** +
**[THEOREM, CONDITIONAL — BLIND CLOCK/FALL/LENSING RESPONSE CLASS TWO]** +
**[OPEN — FINITE TRANSACTION SELECTION, VECTOR CONSTRAINT, NONLINEAR GRAVITY]**  
**Production status:** unchanged; no latency, wave, force, or Hodge operator
changed  
**Ledger status:** no row minted; the class-two result is conditional and is
not booked as native lensing

**Exact certificate:**
[proof_self_dual_trace_capacity_static_pole_lensing_boundary.py](../../../../../scripts/proofs/proof_self_dual_trace_capacity_static_pole_lensing_boundary.py)
performs **910 exact checks**. It exhausts all SC-oriented manifestation
moments and the complete signed cubic group, diagonalizes the self-dual
capacity kernel, verifies its lattice massless pole and long-wave symbol, and
derives the blind response tuple symbolically. It uses no observed deflection,
gravity normalization, master root, fitted coefficient, or continuum target.

---

## 1. One manifestation event already contains scalar and tensor sources

The paired-history actualization/source vertex produces the common moment

\[
 t_d={1\over18}dd^{\mathsf T},\qquad |d|=1.                \tag{1}
\]

Its unique \(O_h\)-irreducible trace/STF decomposition is

\[
 \boxed{
 \rho=\operatorname{tr}t_d={1\over18},\qquad
 t_d={\rho\over3}I+T_d,}                                  \tag{2}
\]

\[
 \boxed{
 {\rho\over3}I={I\over54},\qquad
 T_d={1\over18}\left(dd^{\mathsf T}-{I\over3}\right).}     \tag{3}
\]

The scalar \(\rho\) is independent of orientation, while

\[
 T_d\longmapsto RT_dR^{\mathsf T}                         \tag{4}
\]

under every signed cubic transformation. No new source record is introduced:
the static scalar and radiative/anisotropic tensor sources are two block
readings of the same actualization moment.

Moreover,

\[
 \det T_d={1\over78732}\ne0.                               \tag{5}
\]

The STF part is not TT and must be routed through the gravity constraint
sector identified by the second-order tensor-action theorem.

---

## 2. Minimum self-dual trace-capacity action

Let \(U_t\) be a primal/temporal capacity depth and \(U_s\) an independently
owned dual/spatial-Hodge capacity depth. Consider the static quadratic
reference action

\[
 \boxed{
 {\cal E}_{\rm cap}
 =\sum_x\left[
 {\kappa\over2}\left(|\nabla U_t|^2+|\nabla U_s|^2\right)
 +{\eta\over2}(U_t-U_s)^2
 -\rho(U_t+U_s)
 \right],}                                                \tag{6}
\]

with

\[
 \kappa>0,\qquad \eta\ge0.                                \tag{7}
\]

Equation (6) is a **selected self-dual reference action**. In particular, the
equal coupling of \(\rho\) to both capacities is not yet derived from the
finite collision. It is declared openly so its consequences and falsifiers
can be computed.

For the positive cubic lattice Laplacian symbol

\[
 \Lambda(k)
 =4\sum_{i=1}^{3}\sin^2{k_i\over2},                       \tag{8}
\]

the static kernel is

\[
 K(k)=
 \begin{pmatrix}
 \kappa\Lambda+\eta&-\eta\\
 -\eta&\kappa\Lambda+\eta
 \end{pmatrix}.                                           \tag{9}
\]

Its exact eigenvectors and eigenvalues are

\[
 \boxed{
 (1,1):\ \kappa\Lambda,\qquad
 (1,-1):\ \kappa\Lambda+2\eta.}                           \tag{10}
\]

The self-dual common mode is massless. The relative primal/dual mode is
massive for \(\eta>0\).

---

## 3. Exact equal static solution and massless pole

The selected source vector is

\[
 J_\rho=\rho(1,1).                                        \tag{11}
\]

It is orthogonal to the relative mode, so equation (9) gives

\[
 \boxed{
 U_t(k)=U_s(k)=U(k)
 ={\rho(k)\over\kappa\Lambda(k)}.}                        \tag{12}
\]

The relative response vanishes exactly:

\[
 U_t-U_s=0.                                               \tag{13}
\]

Equation (12) is a genuine lattice massless static pole. Since

\[
 \Lambda(k)=|k|^2+O(|k|^4),                               \tag{14}
\]

its uncontained three-dimensional long-distance Green response has the
inverse-distance class. The absolute strength remains proportional to
\(1/\kappa\), which this theorem does not determine.

On a finite periodic lattice, a nonzero net trace source requires a
neutralizing background or boundary flux because the uniform massless mode
has \(\Lambda(0)=0\). This is the ordinary discrete Gauss solvability
condition, not a license to delete the source.

---

## 4. Conditional normalized response readouts

Now apply the minimum normalized readouts already priced separately by the
clock/Maxwell and primal/dual Hodge theorems:

\[
 \nu_t(U_t)=1-U_t+O(U_t^2),                               \tag{15}
\]

\[
 \nu_s(U_s)=1-U_s+O(U_s^2).                               \tag{16}
\]

The declared physical roles are:

1. slow matter has potential energy \(mU_t\);
2. the material clock advances with temporal factor \(\nu_t\);
3. the complete Maxwell temporal advance uses \(\nu_t\); and
4. the spatial primal/dual Hodge incidence uses \(\nu_s\).

Each unit coefficient in this list is a **normalized minimal-coupling
selection**. The theorem derives what follows from that selection; it does not
claim the finite actualization action has already selected it.

Because equation (12) gives \(U_t=U_s=U\), slow matter obeys

\[
 \ddot x=-\nabla U+O(U^2),                                \tag{17}
\]

the material clock rate is

\[
 {d\tau\over dn}=1-U+O(U^2),                              \tag{18}
\]

and the ray speed is

\[
 {c_{\rm ray}\over c_*}
 =\nu_t\nu_s
 =1-2U+O(U^2).                                            \tag{19}
\]

The refractive response is therefore

\[
 n(U)=1+2U+O(U^2).                                        \tag{20}
\]

In the blind response notation,

\[
 \boxed{
 (a_m,a_t,a_0,a_s)=(1,1,1,1).}                           \tag{21}
\]

---

## 5. Conditional lensing and Shapiro class

The already certified source-normalization-free discriminators are

\[
 {\mathscr R}_{tm}={a_t\over a_m},\qquad
 {\mathscr D}={\mathscr S}={a_0+a_s\over a_m}.             \tag{22}
\]

Substituting equation (21) gives

\[
 \boxed{
 {\mathscr R}_{tm}=1,\qquad
 {\mathscr D}={\mathscr S}=2.}                            \tag{23}
\]

The equality of deflection and delay follows from one common principal
response; the unknown source strength \(1/\kappa\) cancels.

Equation (23) is an exact consequence of equations (6) and (15)--(16), but it
is **not yet a native FTD prediction**. It is the first complete reference
action in this chain that simultaneously has:

- a trace source supplied by actualization;
- a lattice massless static pole;
- clock/fall coherence;
- temporal Maxwell response;
- spatial Hodge response; and
- a nonzero blind lensing class.

Its self-dual source coupling and normalized readouts remain selected.

---

## 6. Why two capacity owners are still required

Using one binary permission twice would give

\[
 g^2=g,                                                    \tag{24}
\]

so it cannot produce the product response in equation (19). The present
construction instead uses two independently owned capacity factors whose
**solutions** are equal by the self-dual action:

\[
 \nu_t\nu_s=(1-U_t)(1-U_s).                               \tag{25}
\]

This is compatible with the earlier primal/dual idempotence price and the
dual-layer mixing theorem. It does not append two independent response
parameters: the exchange symmetry and symmetric source make their static
solutions equal.

The finite transaction must still realize those two owners and the mixing
schedule without an external controller.

---

## 7. Connection to the tensor action

The same event moment in equation (1) supplies both:

\[
 \rho=\operatorname{tr}t_d
\quad\text{for the scalar capacity action},               \tag{26}
\]

and

\[
 T_d=\operatorname{STF}(t_d)
\quad\text{for the tensor/constraint action}.             \tag{27}
\]

The
[even-STF second-order action theorem](THEOREM_EVEN_STF_SECOND_ORDER_ACTION_SPIN2_ESCAPE_AND_CONSTRAINT_PRICE_v1.md)
provides two conditional radiative tensor modes at speed \(1/6\). Equation
(5) proves the local source cannot be inserted directly into their TT leaf.
The missing scalar/vector constraint extension must therefore mediate
equations (26)--(27) while preserving the static solution (12).

This is closer to a single gravity action than two unrelated gravity
postulates: scalar attraction/lensing and tensor radiation are sourced by the
trace and STF components of the same actualization moment. But the present
scalar and tensor actions are still composed reference sectors, not one
derived finite transaction.

---

## 8. What remains before lensing is native

1. Derive equation (6) from the finite primal/dual capacity collision.
2. Derive rather than select the equal source coupling in equation (11).
3. Derive the normalized matter, clock, Maxwell-time, and Hodge-space
   readouts in equations (15)--(19) from one action.
4. Add the vector constraint sector required by moving sources and momentum.
5. Couple the non-TT STF source locally to the radiative tensor action.
6. Preserve the exact Maxwell Gauss constraint and two-polarization cone in
   an inhomogeneous \(U(x)\).
7. Prove positive total energy and exact reversal including source work.
8. Construct the uncontained or boundary-neutralized static source.
9. Run the already frozen blind deflection and Shapiro fixtures only after
   the coefficients are action-derived.
10. Derive nonlinear self-coupling or an Einstein-equivalent completion.

Until items 1--9 pass, FTD's production lensing status remains class zero and
equation (23) remains a conditional reference result.

---

## 9. Common-action significance

The action target can now be written in one source-decomposed form:

\[
 \boxed{
 {\cal S}_{\rm common}
 =
 {\cal S}_{\rm actualization}
 +{\cal S}_{\rm trace}[U_t,U_s;\rho]
 +{\cal S}_{\rm tensor}[h,\xi;T_d]
 +{\cal S}_{\rm matter}[y,\theta;U_t]
 +{\cal S}_{\rm Maxwell}[A;U_t,U_s].}                    \tag{28}
\]

Equation (28) is a scoped construction contract, not a completed Lagrangian.
Its nontrivial improvement is source identity: \(\rho\) and \(T_d\) are no
longer unrelated gravity inputs. They are forced irreducible readings of the
same manifestation transaction.

The next theorem must replace the plus signs in equation (28) by one finite
collision/action rule whose variations generate every sector and reciprocal
work ledger.

The subsequent selected
[reciprocal packet/clock/recoil absorption generator](../common_action_mechanics_reciprocity/THEOREM_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_AND_GRAVITY_SOURCE_BOUNDARY_v1.md)
preserves complete local energy through field absorption and therefore keeps
the scalar $T_{00}$ owner continuous across that seam. It does not transfer
the event's STF stress into the tensor sector, derive the vector/scalar
constraints, or generate the self-dual readouts in equations (15)--(19).
Accordingly it closes one scalar source-accounting interface but does not
promote finite lensing, native spin-2 propagation, or nonlinear gravity.

The later
[C4 symmetric-stress packet discriminator](../common_action_mechanics_reciprocity/THEOREM_C4_SYMMETRIC_STRESS_PACKET_MOMENTUM_AND_SOURCE_HANDOFF_BOUNDARY_v1.md)
shows that, conditional on $J_E=c^2p_F$, one packet stress is
$\Sigma_F=Err^{\mathsf T}=18E t_{\rm evt}$. Its trace is the scalar energy
owner and its STF part is exactly the tensor source. This removes an
independent source-normalization freedom on that conditional branch, but it
still does not install tensor ownership, the constraint algebra, or the
readouts needed by the blind lensing class.

The later
[existing-type constraint-seam theorem](../common_action_mechanics_reciprocity/THEOREM_C18_EXISTING_TYPE_SCALAR_STF_VECTOR_CONSTRAINT_ABSORPTION_SEAM_AND_EQUAL_COUPLING_BOUNDARY_v1.md)
installs those source records in one selected canonical map and leaves two
homogeneous STF modes. It also proves that all seam identities remain valid
for independent scalar and tensor coefficients \((g_0,g_T)\). Therefore the
equal response used in this lensing reference action is not forced by
symplecticity, energy, constraint preservation, or polarization count; it
must come from a common kinetic normalization in the finite action.

The later
[transverse charge-even constraint-bundle theorem](../common_action_mechanics_reciprocity/THEOREM_C18_TRANSVERSE_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_TWO_OWNER_BOUNDARY_v1.md)
supplies an exact finite, EM-neutral spare-vector load for every transverse
STF-divergence chart. Axial incidence instead requires a pair of plane
bundles and two independent owners; the \(D_4\) stabilizer prevents a
context-free scalar-phase selector. Therefore even the finite constraint
source is not yet complete, and nothing in that result promotes the selected
equal source/readout coefficients used here.

The later
[Hodge-framed all-axis signed-event theorem](../common_action_mechanics_reciprocity/THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
resolves that finite axial source context using the existing electromagnetic
Hodge frame and composes all local source records with recoil and clock action
in one exact signed generator. The displayed electromagnetic, scalar, and
tensor normalizations remain independently rescalable, so the equal response
and class-two lensing assumptions in this reference action are still not
derived.

---

## 10. Epistemic firewall

\[
\boxed{
\begin{array}{ll}
\text{trace/STF source split} & \text{exact theorem},\\
\text{self-dual kernel spectrum} & \text{exact conditional},\\
\text{massless symmetric pole} & \text{exact conditional},\\
\text{equal solution }U_t=U_s & \text{exact conditional},\\
\text{self-dual source coupling} & \text{selection},\\
\text{normalized minimal readouts} & \text{selection},\\
\text{response tuple }(1,1,1,1) & \text{conditional theorem},\\
\text{blind response class two} & \text{conditional theorem},\\
\text{finite native lensing} & \text{open},\\
\text{nonlinear gravity} & \text{open}.
\end{array}}
\]
