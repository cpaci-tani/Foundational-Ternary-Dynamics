# C18 transverse charge-even constraint bundle and axial two-owner boundary v1

**Date:** 2026-08-24

**Status:** **[THEOREM — EXACT FINITE EM-NEUTRAL TRANSVERSE CONSTRAINT BUNDLE]** +
**[THEOREM — SIGNED-CUBIC AND C4-COMPATIBLE RETAINED-RECORD MAP]** +
**[THEOREM — EXACT BLOCKED STF-DIVERGENCE NORMALIZATION]** +
**[THEOREM — AXIAL D4 NO-SELECTOR AND TWO-OWNER MINIMUM]** +
**[OUTCOME B — TRANSVERSE REALIZATION, AXIAL OWNERSHIP OPEN]** +
**[OPEN — FULL CONSTRAINT ACTION, STATIC POLE, SPIN-2, AND LENSING]**

**Production status:** unchanged

**Ledger status:** no FTD claim row minted

**Locked preregistration:**
[PREREG_C18_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_CONTEXT_PRICE_v1.md](../../preregistrations/common_action_mechanics_reciprocity/PREREG_C18_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_CONTEXT_PRICE_v1.md),
pre-execution SHA-256
42A7275C86D27FCBA5264744F6B19EA1E176C5ABF05E37D04DE404C972CCA38B.

**Exact certificate:**
[proof_c18_charge_even_constraint_bundle_axial_context_price.py](../../../../../scripts/proofs/proof_c18_charge_even_constraint_bundle_axial_context_price.py),
SHA-256
FACF5949B3A04A8DCBDD57F047C8ADFB13007F2FD0EA7B25C56F24FAB80EC8F3,
performs 20,126 exact finite, integer, rational, inverse, signed-cubic,
phase, and stabilizer checks. No floating point, target gravitational
coefficient, deflection angle, master root, empirical coupling, or numerical
near-miss search enters.

---

## 1. The finite plane bundle

Let \(r,n\) be signed SC axes with

\[
 r\cdot n=0.                                             \tag{1}
\]

Start with three complete records sharing one C4 phase and one retained
source route:

1. one active SC record oriented on \(-r\);
2. one reserve FCC record labelled \(f_+\); and
3. one reserve FCC record labelled \(f_-\).

Define the ordered plane bundle \({\cal B}(r,n)\) by

\[
 \begin{array}{rcl}
  -r&\longmapsto&+r,\\
  f_+&\longmapsto&-(r+n),\\
  f_-&\longmapsto&-(r-n).
 \end{array}                                             \tag{2}
\]

The FCC labels in equation (2) are part of the retained record. They are not
assigned by coordinate sorting. This matters: coordinate sorting swaps the
two histories under some cubic rotations and fails labelled equivariance.
Binding \(f_+\) to \(+n\) and \(f_-\) to \(-n\) gives the exact family law

\[
 \boxed{g{\cal B}(r,n)={\cal B}(gr,gn)\qquad(g\in O_h).} \tag{3}
\]

The inverse deactivates the same two FCC records and restores the SC record
to \(-r\). Record identifiers, internal charge, C4 phase, layer, and source
route are unchanged. The map rejects before mutation if a reserve is missing,
an identifier is duplicated, a target is occupied, the SC orientation is
wrong, or the three records do not carry a common C4 phase. Disjoint record
sets commute; a second bundle cannot spend an SC owner already consumed by
the first.

Equation (3) was checked for all 48 signed-cubic transformations and all 24
ordered orthogonal pairs \((r,n)\). A global C4 shift commutes with the map.

---

## 2. Exact electromagnetic neutrality and spare-vector loading

Use the two C18 vector-copy coordinates established by the predecessor:

\[
 J_{\rm EM}=J_{\rm SC}+J_{\rm FCC},\qquad
 J_{\rm C}=J_{\rm SC}-J_{\rm FCC}.                      \tag{4}
\]

Equation (2) gives

\[
 \Delta J_{\rm SC}=2r,\qquad
 \Delta J_{\rm FCC}=-(r+n)-(r-n)=-2r.                  \tag{5}
\]

Therefore

\[
 \boxed{\Delta J_{\rm EM}=0,\qquad
        \Delta J_{\rm C}=4r.}                          \tag{6}
\]

The spatial operation is independent of internal charge orientation. Under
the registered internal conjugation that reverses the retained charge label
without changing the geometric record, equation (6) is unchanged. In this
blocked record model the bundle is therefore charge even while the independent
electromagnetic vector remains unaltered.

This is stronger than the predecessor's representation-capacity statement:
an explicit finite retained-record map now loads the spare vector. It is not
yet a production collision or a derivation of the internal charge action.

---

## 3. Exact transverse realization of the STF divergence load

For one normalized manifestation direction \(r\), define

\[
 T_r={1\over18}\left(rr^{\mathsf T}-{\mathbf1\over3}\right). \tag{7}
\]

For a nearest-neighbor derivative ray \(q\), put

\[
 v(r,q)=3(r\cdot q)r-q.                                 \tag{8}
\]

Then exact rational arithmetic gives

\[
 \boxed{216T_rq=4v(r,q).}                               \tag{9}
\]

When \(q\perp r\), equation (8) becomes \(v=-q\). The single bundle

\[
 {\cal B}(-q,r)                                         \tag{10}
\]

has

\[
 \Delta J_{\rm C}=-4q=216T_rq,
 \qquad \Delta J_{\rm EM}=0.                           \tag{11}
\]

Thus every transverse nearest-neighbor chart value of the local STF
divergence has an exact finite C18 realization. The factor 216 is fixed by
the previously registered \(1/18\) event tensor and the minimum finite
\(4r\) spare-vector step. It is a blocked chart normalization, not a gravity
coupling and not a measured physical scale.

---

## 4. Axial incidence costs two plane owners

For \(q=r\), equation (9) requires

\[
 216T_rr=8r,                                             \tag{12}
\]

and for \(q=-r\) it requires \(-8r\). A single plane bundle supplies only
\(\pm4r\). The exact axial load is the sum of the two plane bundles associated
with the two transverse unoriented axes:

\[
 \boxed{
 {\cal B}(\pm r,n_1)+{\cal B}(\pm r,n_2)
 \quad\Longrightarrow\quad
 \Delta J_{\rm C}=\pm8r.}                              \tag{13}
\]

The FCC first-moment lattice is

\[
 D_3=\{z\in\mathbb Z^3:\ z_1+z_2+z_3\equiv0\pmod2\}. \tag{14}
\]

An axial FCC moment in \(D_3\) has even axial coefficient. With
electromagnetic neutrality, the spare-vector change is twice that moment.
Consequently \(4r\) is the minimum nonzero axial spare-vector step, and two
such steps are the minimum required by equation (12).

For fixed ordered axial data \((r,q=\pm r)\), its signed-cubic stabilizer has
order eight and acts transitively on the two transverse unoriented plane
choices. Hence no equivariant context-free function can select just one of
them:

\[
 \boxed{
 \text{ordered axial data do not determine a single transverse plane}.}   \tag{15}
\]

A scalar C4 phase is unchanged by the spatial stabilizer and leaves the same
two-element orbit. It cannot repair equation (15). The unordered pair of
both planes is invariant, but it spends two independent SC owners on the same
line—or an explicitly distributed or time-shared equivalent whose two
histories remain distinct.

The current one-record-per-directed-channel C18 slice cannot execute both
members of equation (13) atomically at one site. Reusing the first owner is
double spending, not a second polarization.

---

## 5. Relation to the common source/recoil generator

The predecessor required the vector-constraint shift

\[
 \Delta\kappa=g_TS q.                                   \tag{16}
\]

Equations (9)--(11) give a finite realization of the **direction and relative
integer load** of equation (16) on transverse nearest-neighbor charts. They
do not derive \(g_T\), the canonical normalization of \(\kappa\), or the
selected type-2 generator that currently books the source, recoil, and clock
debit.

The current chain is therefore

\[
 \begin{aligned}
 \text{manifestation tensor }T_r
 &\longrightarrow 216T_rq\\
 &\xrightarrow{\ q\perp r\ }
 {\cal B}(-q,r)\\
 &\longrightarrow
 \Delta J_{\rm EM}=0,\quad\Delta J_{\rm C}=-4q,
 \end{aligned}                                          \tag{17}
\]

while the axial branch is

\[
 216T_r(\pm r)
 \longrightarrow
 \text{two-owner or distributed paired-plane repair}.  \tag{18}
\]

This retires “no finite charge-even realization is known” for the transverse
sector. It does not retire the full finite constraint action.

---

## 6. Gravity and lensing disposition

### Established exactly

- a reversible, payload-retaining finite C18 plane bundle;
- signed-cubic covariance for the labelled record family;
- global C4 compatibility and fail-closed phase consistency;
- charge-even spatial action under the registered internal conjugation;
- exact preservation of \(J_{\rm EM}\);
- the minimum spare-vector increment \(\Delta J_{\rm C}=4r\);
- exact realization of every transverse value \(216T_rq\);
- the axial stabilizer no-selector theorem; and
- the minimum axial price of two independently owned plane bundles.

### Still selected or open

1. a native source of the second axial owner or a covariant distributed
   schedule;
2. one finite transaction that also loads the scalar and STF canonical pairs,
   material recoil, and clock debit;
3. derivation of the constraint algebra rather than blocked preservation;
4. a positive static scalar/vector Green response;
5. a positive two-mode tensor pole generated by the same finite action;
6. action-derived equality of scalar, tensor, clock, and Maxwell response;
7. universal composite-matter response;
8. blind lensing and Shapiro delay; and
9. nonlinear gravity.

The theorem therefore does not establish a native graviton or an equivalent
complete spin-2 sector. It proves one local finite component needed by such a
sector and identifies the exact axial ownership obstruction.

---

## 7. Contextual measurement and electromagnetic coupling firewall

The retained identifiers, phases, source routes, inverse map, and atomic
double-spend rejection are compatible with a contextual history ledger. They
do not construct a physical history ensemble, its preparation measure, or a
Born pushforward. No probability law follows from the 20,126 finite checks.

Likewise, \(J_{\rm EM}\) neutrality protects the already identified
electromagnetic vector channel but does not measure its work curvature. No
fine-structure value, master-quadratic root, or relation to
\(\alpha\) follows from equations (4)--(18).

The preregistered disposition is **Outcome B**: the transverse finite bundle
passes exactly; axial realization needs a second owner, distributed support,
or explicit paired-plane context.

---

## 8. Subsequent Hodge-framed full-source action

The locked
[Hodge-framed all-axis successor](THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md)
uses the existing electromagnetic Hodge flag

\[
 (r,n,h)\longmapsto(r,\;u=hn,\;v=r\times n)
\]

to supply both axial plane axes covariantly. The transverse one-owner branch
and axial two-owner branch then form one all-axis rule satisfying

\[
 \Delta J_{\rm EM}=0,\qquad
 \Delta J_{\rm C}=216T_rq
\]

for every signed-SC source/derivative chart.

On the prepared bright-history orbit, one signed type-2 generator composes
that finite map with manifestation, charge current, trace/STF loading,
constraint loading, material recoil, clock action, event energy, and the
required port-conjugate reaction. Its complete map is symplectic, energy
conserving, fail closed, and involutive; 748,824 exact checks pass.

The successor resolves the axial **spatial context** and blocked common-source
action. It does not natively form the Hodge flag or owner reserves, generate
the constraint and field poles, fix scalar/tensor normalization, or establish
lensing.
