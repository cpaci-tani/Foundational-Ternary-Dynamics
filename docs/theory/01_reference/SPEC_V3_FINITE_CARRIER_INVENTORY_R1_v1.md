# SPEC — V3 finite carrier inventory R1 v1

**Date:** 2026-08-24  
**R2-driven revision:** each C18 relation owns an independent primary/reserve
A9 pair; this removes the hidden diagonal-selection controller exposed by the
first rule construction.  
**Status:** **[SUPERSEDED DRAFT CARRIER — PROVENANCE RETAINED]**  
**Successor:**
[`SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md`](SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md)
replaces the six single-occupancy ports by the finite exclusion-channel bank
required by the exact cotangent collision and R5 wave recovery. The v1 counts
remain correct for this narrower carrier, but it is no longer the selected v3
inventory.  
**Constitution:**
[`SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md`](SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)  
**Machine register:**
[`strict_discrete_common_action_register_v3.json`](strict_discrete_common_action_register_v3.json)  
**Exact certificate:**
[`proof_v3_finite_carrier_inventory.py`](../../../scripts/proofs/proof_v3_finite_carrier_inventory.py)

---

## 1. Disposition of blocker R1

This specification closes **V3-R1 for the present draft carrier**. It gives a
finite alphabet, a unique incidence owner, and explicit readouts for every
microscopic slot. It does not select the update law. R1 must reopen if the
R2 construction requires a state not representable by this inventory.

The inventory adopts the leading C18 construction branch:

\[
 \mathcal D_{18}=\mathcal D_{\rm SC}\sqcup\mathcal D_{\rm FCC},
 \qquad |\mathcal D_{18}/\{d\sim-d\}|=3+6=9.
\]

The Moore shell remains the causal ceiling. BCC corner relations are not
microscopic storage slots in this carrier. They can still occur as coarse
three-tick displacements of SC Hodge packets. This is a selected active
carrier architecture, not a theorem that nature must choose C18.

---

## 2. Primitive finite sets

### 2.1 Ternary actuality

\[
 \mathbb T=\{-1,0,+1\}.
\]

The site variable `s` is the public manifestation value. It is not the full
site record.

### 2.2 Phase/polarity ownership token

\[
 \mathcal A_9=\mathbb T^2
 =\{(0,0)\}\sqcup(\mathbb Z_4\times\mathbb Z_2).
\]

For `z=(u,v)`, set

\[
 r=u^2+v^2,\qquad d=u^2v^2,
\]

\[
 n(z)=r-d,\qquad c(z)=1-n(z),\qquad
 \epsilon(z)=r-3d.
\]

Thus `n` is occupancy, `c` is available capacity, and `epsilon` is carried
polarity. The exact phase readout `(U,V)` is the C4 readout in the
[ternary-square carrier theorem](../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_TERNARY_SQUARE_PHASE_POLARITY_CARRIER_AND_AUTONOMOUS_CROSSING_CLOCK_v1.md).
One blank plus four phases and two polarities requires exactly nine states;
`A9` is therefore the exact minimum for this declared payload.

### 2.3 Hodge-routed field packet

Let

\[
 \mathcal D_{\rm SC}=\{\pm e_1,\pm e_2,\pm e_3\}
\]

and, for a directed SC tangent `d`, let

\[
 \mathcal N_d=\{n\in\mathcal D_{\rm SC}:n\cdot d=0\},
 \qquad |\mathcal N_d|=4.
\]

The port alphabet at tangent `d` is

\[
 \mathcal F_d=\{\varnothing\}\sqcup
 \bigl((\mathcal A_9\setminus\{0\})
 \times\mathcal N_d\times\{-1,+1\}\bigr).
\]

A nonblank record `(z,n,h)` carries C4 phase and polarity in `z`, an axial
face normal `n`, and the pseudoscalar handed flag `h`. Therefore

\[
 |\mathcal F_d|=1+8\cdot4\cdot2=65.
\]

The conditional count 65 is exact for the declared independent payload. The
handed bit is not decorative: the shared-edge centralizer theorem proves
that a cubic-covariant quarter-turn choice is impossible without it. Polarity
and handedness are kept distinct because their physical identification has
not been proved.

---

## 3. Cell alphabets and ownership

### 3.1 Sites

Every site owns one actuality slot and one exclusion port for each directed
SC tangent:

\[
 \boxed{
 \mathcal A_0=\mathbb T\times
 \prod_{d\in\mathcal D_{\rm SC}}\mathcal F_d,}
\]

\[
 |\mathcal A_0|=3\cdot65^6=226{,}256{,}671{,}875.
\]

The large cardinality is a product description, not a table inserted into
the rule. Each directed port holds at most one complete packet. Collisions
must fail closed, exchange, or route under R2; no unbounded queue or packet
identifier exists.

The manifestation quotient is

\[
 m(s,(f_d)_d)=s.
\]

It is surjective and many-to-one because all port configurations are hidden
by the ternary readout.

### 3.2 SC bonds

Every unoriented nearest-neighbor edge owns one primary A9 transaction slot
and one independently owned reserve/response companion:

\[
 \boxed{\mathcal A_1=\mathcal A_9^{P}\times\mathcal A_9^{D},
 \qquad|\mathcal A_1|=9^2=81.}
\]

The edge orientation is structural incidence data, not a second stored copy.
Reversing the presentation of an edge reverses its directed current readout;
it does not duplicate the token.

### 3.3 Plaquettes

Every oriented square plaquette owns exactly four independent A9 slots:

1. one primary slot for each of its two face diagonals; and
2. one reserve/axial-response companion for each diagonal.

Hence

\[
 \boxed{
 \mathcal A_2=
 (\mathcal A_9^{P,+}\times\mathcal A_9^{D,+})
 \times
 (\mathcal A_9^{P,-}\times\mathcal A_9^{D,-}),
 \qquad |\mathcal A_2|=9^4=6{,}561.}
\]

The two diagonal labels are exchanged by the appropriate cubic
transformations. Each companion is the second independently owned A9
placement required by the no-spare-scalar theorem; it is not the complement
of its primary link-ownership bit. Giving each diagonal its own companion is
load-bearing: one companion shared by both diagonals leaves the reverse
transaction unable to recover which diagonal owned the token unless another
selector is hidden in the rule.

For R2 the companion is a relation-owned reserve/response slot carried by the
same minimal enclosing cell. The Hodge packet supplies axial face orientation.
This does not yet prove that the companion is a literal dual-complex face
field; that stronger geometric identification remains a P7 recovery question.

### 3.4 Cubes

No independent body-diagonal or volume token is presently required:

\[
 \boxed{\mathcal A_3=\{\varnothing\},\qquad|\mathcal A_3|=1.}
\]

Making cube storage trivial is an Occam choice, not a claim that blocked
volume observables are trivial. A future R2 rule that requires independent
BCC/cube memory pays a new type and reopens R1.

---

## 4. Exact ownership doctrine

At one tick every nonblank microscopic payload is owned by exactly one named
slot:

| Payload | Unique owner |
|---|---|
| Ternary actuality | one site actuality slot |
| Mobile Hodge packet | one directed site port `(x,d)` |
| SC primary/reserve pair | two separately owned slots on one unoriented SC edge |
| FCC primary/reserve pair | two separately owned slots on one labeled diagonal of one plaquette |

“Ownership” means slot occupancy. It does not mean that an unbounded serial
number follows an otherwise identical token. A transaction may move a
complete payload between compatible slots, but it may not copy it. Any
blocked identity or ancestry is reconstructed only from surviving finite
records and their present positions.

Coordinate addresses and the global tick are structural indices supplied by
P1 and P2. They are not per-cell replay tapes. A local record contains no
arbitrarily large coordinate, birth-time integer, history list, floating
point value, or hidden random seed.

---

## 5. Derived readouts, not extra fields

The following quantities are functions of the inventory and introduce no
new microscopic type:

- occupation and residual capacity from `n(z)` and `c(z)`;
- phase and polarity from the A9 readouts;
- directed packet current from port occupancy times its tangent `d`;
- axial circulation from `(d,n,h)` and the oriented plaquette incidence;
- C18 vector, scalar, trace, and STF moments from sums of occupied relation
  directions;
- reserve density from finite packet/slot counts; and
- ternary source and endpoint current from changes of `s` and occupied link
  slots.

Real vector fields, tensor fields, actions, energies, and probabilities are
not members of `A0` through `A3`. They remain P7 block readouts requiring
finite-region recovery proofs.

---

## 6. Why this inventory is not yet dynamics

This carrier can represent the already proved finite ingredients:

- A9 phase/polarity ownership and reversible transfer;
- independent primal and axial capacity;
- cubic-covariant Hodge routing;
- C18 moment readouts; and
- finite collision/backpressure through exclusion slots.

It does not determine which allowed transaction occurs. In particular, it
does not yet prove:

1. a homogeneous collision/stream/manifest/expiry rule;
2. conflict-free composition across shared cells;
3. formation of a mobile reserve or localized recurrent body;
4. a genuine expiry event;
5. wave/action recovery; or
6. any physical coupling.

Those are R2--R6. Passing this inventory certificate must never be reported
as passing those gates.

---

## 7. R1 closure contract

V3-R1 is closed for this draft exactly when the machine register and
certificate agree on:

1. `|A0| = 3*65^6`;
2. `|A1| = 9^2`;
3. `|A2| = 9^4`;
4. `|A3| = 1`;
5. all nine unoriented C18 directions having one primary owner;
6. one independently owned A9 companion per C18 relation;
7. the many-to-one site manifestation quotient;
8. no unbounded queue, identity, replay address, or continuum register; and
9. an explicit reopen condition if R2 needs another type.

This is a carrier **selection** constrained by exact lower-bound theorems. It
is not a theorem of the rewritten P1--P5 that this selection is unique.
