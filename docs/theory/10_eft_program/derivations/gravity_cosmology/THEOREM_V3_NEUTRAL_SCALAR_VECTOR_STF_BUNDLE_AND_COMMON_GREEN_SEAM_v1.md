# V3 neutral scalar/vector/STF bundle and common Green seam v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT EXISTING-CARRIER RANK-NINE GRAVITY-SOURCE BUNDLE]** +
**[THEOREM — RADIUS-ONE REVERSIBLE AND SIGNED-CUBIC-COVARIANT TRANSPORT]** +
**[THEOREM, CONDITIONAL — COMMON COMPONENTWISE DIRICHLET GREEN LIMIT AND
$1/\Lambda$ KERNEL]** +
**[BLOCKED-HISTORY SEAM ONLY]** +
**[OPEN — PHI INTEGRATION, PROTECTION, CONSTRAINT DYNAMICS, DYNAMICAL POLES,
UNIVERSAL COUPLING, NORMALIZATION, AND LENSING]**  
**Carrier price:** five existing opposite-polarity controller pairs at the
marked site; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Scalar parent:**
[`THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md`](../charge_gauss_native_em/THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md)  
**Tensor parent:**
[`THEOREM_V3_NEUTRAL_STF_ROTOR_WALKER_GREEN_SEAM_v1.md`](THEOREM_V3_NEUTRAL_STF_ROTOR_WALKER_GREEN_SEAM_v1.md)  
**Constraint parent:**
[`THEOREM_V3_NEUTRAL_VECTOR_CONSTRAINT_WALKER_AND_TT_LOCALITY_OBSTRUCTION_v1.md`](THEOREM_V3_NEUTRAL_VECTOR_CONSTRAINT_WALKER_AND_TT_LOCALITY_OBSTRUCTION_v1.md)  
**Exact certificate:**
[`proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam.py`](../../../../../scripts/proofs/proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam.py)

---

## 1. The separate carriers compose without a new primitive

The scalar parent gives one neutral rotor whose visit history conditionally
approaches the cubic Dirichlet Green function. The tensor and constraint
parents separately attach a five-component STF payload and a three-component
polar-vector payload to that rotor. The unresolved carrier question was
whether those sectors can coexist in one locally recognizable finite packet.

Let `R` be the native period-twelve internal tick and let

\[
 U(q)=\{(q,+),(q,-)\}                                  \tag{1}
\]

be one opposite-polarity neutral controller pair. Define

\[
 \boxed{
 {cal G}(q;a,b)=
 U(q)\cup U(R^4q)\cup U(a)\cup U(R^2a)\cup U(b).}      \tag{2}
\]

Require the clock orbits of `q`, `a`, and `b` to be distinct. The roles in
equation (2) are then intrinsic to the instantaneous finite state:

1. `q,R^4q` is the unique same-orbit pair with directed separation four;
2. `a,R^2a` is the unique same-orbit pair with directed separation two; and
3. `b` is the remaining singleton controller.

The first pair is the rotor/marker header. The latter three controllers carry
the tensor pair `(a,b)` and vector controller `R^2a`. No coordinate label,
external type tag, or new spatial representation is added.

Every controller occurs with both polarities, so for all three C3 layers

\[
 \boxed{E_{\rm additive}=B_{\rm additive}=0.}          \tag{3}
\]

The ten occupied records are retained labels, not an electromagnetic source.

---

## 2. Exact joint source rank

Read one packet as

\[
 \boxed{
 {cal P}(a,b;\ell)
 =\left(1,\operatorname{STF}_5(a,b;\ell),
 v(R^2a;\ell)\right).}                                \tag{4}
\]

The first entry is the scalar visit unit, the middle five entries are the
two-record STF cross stress, and the last three entries are the native polar
vector readout.

The certificate exhausts every ordered pair `(a,b)` in distinct native clock
orbits. There are 34,560 admissible signatures, and their exact integral
matrix has

\[
 \boxed{\operatorname{rank}\operatorname{span}{\cal P}=9=1+5+3.} \tag{5}
\]

Exact row reduction selects nine canonical pivot packets. Their signatures
are

\[
\begin{aligned}
 &(1,2,2,0,0,0,0,0,1),\\
 &(1,6,-6,0,0,0,0,0,1),\\
 &(1,4,-2,0,0,3,0,0,1),\\
 &(1,-6,6,0,0,0,0,0,1),\\
 &(1,0,0,6,0,0,0,0,1),\\
 &(1,0,0,3,3,0,0,0,1),\\
 &(1,6,-6,0,0,0,0,0,-1),\\
 &(1,4,-2,0,0,3,0,-1,0),\\
 &(1,0,0,3,0,3,1,0,0).
\end{aligned}                                         \tag{6}
\]

Equation (5) is a capacity and transport theorem. It does not identify the
vector entries as dynamically generated constraint multipliers or the STF
entries as protected gravitational radiation.

---

## 3. One local reversible transport rule

For a marked departure neighboring an unmarked destination `U(p)`, define

\[
 \boxed{
 \bigl({\cal G}(q;a,b),U(p)\bigr)
 \longmapsto
 \bigl(U(Rq),{cal G}(p;Ra,Rb)\bigr).}                 \tag{7}
\]

The displacement is the SC neighbor served by `Rq`. Equation (7):

1. retains twelve occupied records across the two sites;
2. moves the complete scalar/vector/STF packet exactly one SC hop;
3. retains exact additive `E/B=0` on both sites;
4. keeps all nine payload coordinates covariantly constant under the combined
   C4/C3 clock; and
5. is covariant under all 48 signed-cubic transformations.

The inverse is explicit. From `U(Rq)` and
`G(p;Ra,Rb)`, apply `R^11` to the source rotor and both payload controllers:

\[
 \bigl(U(Rq),{cal G}(p;Ra,Rb)\bigr)
 \longmapsto
 \bigl({\cal G}(q;a,b),U(p)\bigr).                    \tag{8}
\]

The certificate checks 1,296 selected local transactions and their 1,296
exact inverses, plus 432 full signed-cubic covariance rows. Blank,
orbit-colliding, or malformed packets fail closed.

---

## 4. One common blocked-history Green seam

Sequentially inject a selected packet at a source in a finite cubic domain
with absorbing exterior. Equation (7) follows exactly the parent rotor path.
If `n_N(x)` is the visit count after `N` injections, set

\[
 G_N(x)={{n_N(x)}\over{6N}}.                            \tag{9}
\]

For the nine-coordinate packet signature `P`, define

\[
 \mathcal H_N(x)={\cal P}\,G_N(x).                    \tag{10}
\]

The scalar theorem gives

\[
 \left\|L_DG_N-\delta_s\right\|_\infty\le {8\over N}. \tag{11}
\]

Therefore, component by component,

\[
 \boxed{
 \left\|L_D\mathcal H_{N,A}-{\cal P}_A\delta_s\right\|_\infty
 \le {|{\cal P}_A|\,8\over N}.}                      \tag{12}
\]

Because the nine packets in equation (6) span the full joint source space,
the controlled large-domain history limit conditionally supplies one common
diagonal kernel

\[
 \boxed{
 \mathcal H_A(k)={{\cal P_A(k)}\over{\Lambda(k)}},
 \qquad A=1,\ldots,9,}                                \tag{13}
\]

where

\[
 \Lambda(k)=6-2\sum_{i=1}^3\cos k_i.                  \tag{14}
\]

This closes the former composition gap between the three separate walkers.
It is still a deterministic blocked-history readout, not an instantaneous
autonomous field or a protected dynamical pole.

---

## 5. Gravity consequence

The finite-carrier question has become sharply localized:

```text
scalar carrier/history kernel:          present conditionally
rank-three vector carrier/history:      present conditionally
rank-five STF carrier/history:          present conditionally
one joint rank-nine packet/transport:   present conditionally
exact local inverse and O_h covariance: present
homogeneous Phi integration:            absent
protected constraint/tensor dynamics:  absent
dynamical static and wave poles:        absent
universal coupling and lensing:         absent
```

In particular, FTD no longer needs three unrelated carrier mechanisms merely
to move the linear scalar/vector/tensor gravity source data. The same neutral
packet and the same radius-one rule suffice at blocked-history level.

That result does **not** establish native gravity. The next action must still:

1. form, renew, absorb, and arbitrate these packets under the one
   state-complete homogeneous `Phi`;
2. protect the STF composite and generate the scalar/vector constraint
   algebra locally;
3. turn the history kernel into a positive instantaneous static pole and a
   protected two-mode tensor wave pole;
4. derive the interacting dynamical Hessian rather than importing the bare
   counting metric;
5. couple the pole reciprocally to stable conserved material stress with one
   action-fixed normalization and shared cone; and
6. derive fall, clock response, lensing, delay, and only then nonlinear
   self-coupling.

The Deser bootstrap still cannot replace any of these microscopic steps.

The
[`matter-anchored event-seam successor`](../common_action_mechanics_reciprocity/THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)
selects two orbit-disjoint instances of this packet at every charged event and
proves the exact finite handoff `T_++T_-=3S_d` and
`2(v_++v_-)=3S_dq`, with a local inverse and full signed-cubic covariance.
This closes one prepared material-stress/source-coordinate connection. It
does not protect a gravitational mode or turn the blocked-history Green seam
into an autonomous response.

The
[`redundant packet-protection successor`](THEOREM_V3_REDUNDANT_JOINT_GRAVITY_BUNDLE_PROTECTION_AND_GREEN_INHERITANCE_v1.md)
places three complete copies of the local transaction on prepared parallel
rails and uses one existing A2 READ/COMMIT/work owner. All 1,296 parent
transactions, 3,888 one-copy substitutions, and every one of 2,088 finite
Green-history fault ticks pass; the protected visit measure equals this
parent's measure exactly. That closes a finite packet-symbol protection basin,
not rail formation, router-background protection, propagated constraints, TT
isolation, or a dynamical gravitational pole.

---

## 6. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_neutral_scalar_vector_stf_bundle_common_green_seam.py
```

Expected result: `14/14` exact checks pass, with 34,560 admissible source
rows, joint rank nine, 1,296 local transactions, 432 signed-cubic covariance
rows, and 2,088 finite history steps.
