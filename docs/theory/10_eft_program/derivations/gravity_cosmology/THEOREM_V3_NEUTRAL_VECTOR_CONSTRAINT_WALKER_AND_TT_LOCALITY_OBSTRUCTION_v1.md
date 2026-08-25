# V3 neutral vector-constraint walker and TT-locality obstruction v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT CARRIER-COMPLETE RANK-THREE NEUTRAL VECTOR
WALKER]** +
**[THEOREM, CONDITIONAL — COMPONENTWISE DIRICHLET GREEN LIMIT AND
$1/\Lambda$ VECTOR POLE]** +
**[THEOREM — EXACT FINITE-RANGE INSTANTANEOUS TT-PROJECTOR OBSTRUCTION]** +
**[BOUNDARY — LOCAL AUXILIARY CONSTRAINT DYNAMICS REQUIRED]** +
**[OPEN — COMMON CONSTRAINT ACTION, PHI, PROTECTION, UNIVERSAL COUPLING,
NORMALIZATION, AND LENSING]**  
**Carrier price:** one additional opposite-polarity payload pair beyond the
rotor/marker header; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Scalar parent:**
[`THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md`](../charge_gauss_native_em/THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md)  
**Tensor sibling:**
[`THEOREM_V3_NEUTRAL_STF_ROTOR_WALKER_GREEN_SEAM_v1.md`](THEOREM_V3_NEUTRAL_STF_ROTOR_WALKER_GREEN_SEAM_v1.md)  
**Joint-bundle successor:**
[`THEOREM_V3_NEUTRAL_SCALAR_VECTOR_STF_BUNDLE_AND_COMMON_GREEN_SEAM_v1.md`](THEOREM_V3_NEUTRAL_SCALAR_VECTOR_STF_BUNDLE_AND_COMMON_GREEN_SEAM_v1.md)  
**Exact certificate:**
[`proof_v3_neutral_vector_constraint_walker_tt_locality_obstruction.py`](../../../../../scripts/proofs/proof_v3_neutral_vector_constraint_walker_tt_locality_obstruction.py)

---

## 1. Why a vector constraint carrier is needed

The neutral STF walker supplies five tensor components and a conditional
blocked-history `1/Lambda` pole. A physical spin-2 sector additionally needs
local scalar/vector constraints. Declaring an instantaneous TT projection is
not enough: on a lattice that projection is nonlocal.

This theorem separates the two questions:

1. can the existing finite carrier transport a full neutral vector payload
   through the deterministic Green construction? and
2. can an exact TT constraint instead be imposed by one finite-radius
   translation-invariant collision?

The first answer is yes at carrier/history level. The second is no.

---

## 2. Six-record neutral vector marker

For one field controller `q`, let

\[
 U(q)=\{(q,+),(q,-)\}.                                  \tag{1}
\]

Let `R` be the native period-twelve internal tick. A vector-marked site is

\[
 V(q;a)=U(q)\cup U(R^4q)\cup U(a).                     \tag{2}
\]

The unique `R^4`-related controller pair identifies the rotor and marker; the
third controller is the vector payload. Because all three occur in opposite-
polarity pairs,

\[
 E_{\rm additive}=B_{\rm additive}=0                   \tag{3}
\]

on every C3 layer.

Read the polar half of the native layer value as

\[
 v(a,\ell)=\bigl[\operatorname{layer\_value}(a,\ell)\bigr]_E.
                                                               \tag{4}
\]

Three fixed nonrouter clock-orbit representatives give

\[
 v_1=(-1,0,0),\qquad v_2=(0,-1,0),\qquad v_3=(0,0,-1), \tag{5}
\]

an exact rank-three polar-vector basis. The opposite-polarity physical field
cancels while the finite controller relation retains the vector payload.

---

## 3. Local transport and vector Green seam

For a marked departure and unmarked neighboring destination, select

\[
 \bigl(V(q;a),U(p)\bigr)
 \longmapsto
 \bigl(U(Rq),V(p;Ra)\bigr).                             \tag{6}
\]

Equation (6):

1. conserves eight occupied field records across the two sites;
2. moves the vector payload one SC hop;
3. preserves exact zero additive `E/B`;
4. keeps equation (4) constant under the combined C4/C3 clock; and
5. is covariant under all 48 signed-cubic transformations.

The certificate checks 432 local transactions and 144 covariance rows.

Sequential injection into an absorbing finite box reproduces the scalar rotor
visit history exactly. If `n_N(x)` is the visit count and `v` one basis vector,
define

\[
 U_N(x)=v\,{n_N(x)\over6N}.                             \tag{7}
\]

Then componentwise

\[
 \left\|L_DU_N-v\delta_s\right\|_\infty
 \le {8\over N}\max_i|v_i|.                            \tag{8}
\]

Thus the controlled large-domain history limit conditionally supplies

\[
 \boxed{U_i(k)={q_i(k)\over\Lambda(k)}}                 \tag{9}
\]

for a full rank-three vector source. This is a carrier-complete auxiliary
constraint Green seam, not yet an autonomous constraint equation in `Phi`.

---

## 4. Exact finite-range TT obstruction

A translation-invariant finite-range lattice operator has a Laurent-polynomial
symbol in `x,y,z`. The cubic Laplacian symbol is

\[
 \Lambda(x,y,z)
 =6-(x+x^{-1}+y+y^{-1}+z+z^{-1}).                      \tag{10}
\]

An exact transverse or TT projector contains longitudinal subtraction terms
of the representative form

\[
 {D_iD_j\over\Lambda},                                 \tag{11}
\]

with forward/backward difference symbols. For the cross term choose
`D_x=x-1`, `D_y=y-1`. At the exact algebraic point

\[
 x=y=-1,
 \qquad z=5+2\sqrt6,                                   \tag{12}
\]

one has

\[
 \Lambda=0,
 \qquad (x-1)(y-1)=4\ne0.                              \tag{13}
\]

Therefore `Lambda` does not divide the cross numerator in the Laurent ring,
and equation (11) is not Laurent polynomial. Hence

\[
 \boxed{
 \text{no exact translation-invariant finite-range instantaneous TT
 projector exists}.}                                  \tag{14}
\]

Changing between compatible forward, backward, or centered differences does
not remove the denominator; at equation (12) the corresponding cross factors
remain nonzero.

---

## 5. Gravity consequence

Equation (14) forbids a common shortcut: the five-component STF carrier cannot
be made physical merely by applying an exact local TT filter after each tick.
The admissible strict-discrete alternatives are:

- propagate local scalar/vector constraint carriers whose histories generate
  the required inverse Laplacian;
- introduce an explicitly nonlocal rule and price the violation of P4; or
- weaken exact TT to a controlled blocked approximation and state its error.

The first branch now has a finite rank-three vector carrier and deterministic
Green seam. It still lacks:

1. native constraint sources, sinks, and canonical `Phi` integration;
2. one common scalar/vector/tensor action;
3. exact composition of the auxiliary solves into lattice TT dynamics;
4. an autonomous protected tensor collision and common cone;
5. universal conserved-stress coupling and physical normalization; and
6. lensing and nonlinear self-coupling.

The Deser bootstrap cannot replace these microscopic structures.

The joint-bundle successor now composes this vector payload with the scalar
visit unit and full STF payload in one state-only recognizable, reversible,
signed-cubic-covariant packet of existing records. Thus separate carrier
transport is no longer the open item. Native propagated constraint dynamics,
autonomous pole protection, and their common `Phi` provenance remain open.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_neutral_vector_constraint_walker_tt_locality_obstruction.py
```

Expected result: `12/12` exact checks pass, with vector basis
`{(-1,0,0),(0,-1,0),(0,0,-1)}`, 432 local rows, and 144 signed-cubic rows.
