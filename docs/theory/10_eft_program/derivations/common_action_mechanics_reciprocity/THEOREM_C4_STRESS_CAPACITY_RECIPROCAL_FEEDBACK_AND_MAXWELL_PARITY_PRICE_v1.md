# THEOREM — C4 stress-capacity reciprocal feedback and Maxwell parity price v1

**Date:** 2026-08-24  
**Status:** [THEOREM — FINITE LOCAL PERMUTATION] +
[THEOREM — ADDITIVE-WORK OBSTRUCTION] +
[BOUNDARY — NOT YET A VARIATIONAL ACTION, PHYSICAL WORK LAW, OR GRAVITY]  
**Ontology status:** uses two independently owned copies of the already priced
candidate A9 carrier; adopts no further alphabet  
**Purpose:** close the smallest autonomous source-to-response-to-next-admission
loop downstream of the phase-neutral shared source vertex, classify its
additive conserved quantities, and determine whether the same C4 response
carrier can also carry charge-odd Maxwell backreaction.

**Exact certificate:**
[proof_c4_stress_capacity_reciprocal_feedback.py](../../../../../scripts/proofs/proof_c4_stress_capacity_reciprocal_feedback.py)
exhausts all 256 physical two-carrier states, verifies the map and its inverse,
checks every C18 source and response tensor, enumerates every orbit, classifies
all additive two-carrier invariants, and classifies all involutions of a
single C4 phase orbit.

---

## 1. Inputs already proved

Let \(\mathcal A_9\) be the exact ternary-square carrier

\[
 \mathcal A_9=\{0\}\sqcup(\mathbb Z_4\times\mathbb Z_2),
\]

with one complete phase/polarity token owned either by a local bond or by its
reserve. Write

\[
 n(X)\in\{0,1\},\qquad c(X)=1-n(X)                 \tag{1}
\]

for link occupation and residual capacity. The established maps are:

- \(T\): the period-eight material tick, consisting of a phase-crossing
  ownership transaction followed by one C4 phase advance;
- \(A\): the ownership involution exchanging a valid link token with its
  reserve; and
- \(R\): one C4 phase advance, which preserves ownership and capacity.

The no-spare-scalar theorem already prices two independently owned A9 copies
for the common-capacity branch. Call them

\[
 M_n\quad\text{(material carrier)},\qquad
 G_n\quad\text{(response/work carrier)}.            \tag{2}
\]

No external permission word or residual event bit will be used.

On a normalized C18 line with direction \(d\), dyad
\(D=dd^{\mathsf T}\), and material polarity \(\epsilon\), the established
phase-neutral source readouts of a manifested material token are

\[
 j={\epsilon d\over9},\qquad
 t={D\over18}.                                       \tag{3}
\]

They vanish when the token is reserve-owned. Thus \(j\) is charge odd and
\(t\) is charge even.

---

## 2. The drift–kick–clock map

Read the old response capacity

\[
 a_n=c(G_n).                                         \tag{4}
\]

First perform the capacity-admitted material drift

\[
 M_{n+1}=T^{a_n}M_n.                                 \tag{5}
\]

This is the local proper-clock step: the global transaction continues whether
or not \(a_n=1\), but the material carrier advances only when the response
slot admits it.

Now read the **post-drift persistent source**

\[
 \mu_n=n(M_{n+1})\in\{0,1\}.                         \tag{6}
\]

The source is the manifested state, not an unsigned count of transitions.
Actualization turns it on; de-actualization turns it off. Apply its response
kick and then the global response clock:

\[
 \widetilde G_n=A^{\mu_n}G_n,\qquad
 G_{n+1}=R\widetilde G_n.                            \tag{7}
\]

Equations (4)–(7) define one local map

\[
 \boxed{\mathcal F(M_n,G_n)=(M_{n+1},G_{n+1}).}       \tag{8}
\]

Its causal order is therefore

\[
 \boxed{
 \text{old capacity}\longrightarrow
 \text{material tick}\longrightarrow
 \text{persistent stress source}\longrightarrow
 \text{response capacity}\longrightarrow
 \text{next admission}.}                            \tag{9}
\]

This is the first exact autonomous feedback loop in the bond-action programme.
It replaces the externally supplied permission word used in the earlier
material-clock boundary theorem.

---

## 3. Exact inverse

Given the output, first undo the global response phase:

\[
 \widetilde G_n=R^{-1}G_{n+1}.                       \tag{10}
\]

The output material state is retained, so its source bit is directly readable:

\[
 \mu_n=n(M_{n+1}).                                   \tag{11}
\]

Undo the response kick, recover the old permission, and undo the material
drift:

\[
 G_n=A^{\mu_n}\widetilde G_n,\qquad
 a_n=c(G_n),\qquad
 M_n=T^{-a_n}M_{n+1}.                                \tag{12}
\]

Because \(A^2=1\), \(R\) is invertible, and \(T^{-1}=T^7\) on every physical
A9 state, equation (12) is unique. Exhaustive evaluation proves

\[
 \mathcal F^{-1}\mathcal F=\mathcal F\mathcal F^{-1}=1
\]

on all \(16\times16=256\) physical states. No event history, random seed,
hidden compatibility word, or unbounded memory is required.

---

## 4. Source/response ledger on every C18 line

For the post-drift state, equations (3) become

\[
 j_n={\mu_n\epsilon_n d\over9},\qquad
 t_n={\mu_nD\over18}.                                \tag{13}
\]

The response kick changes ownership if and only if \(\mu_n=1\). Since reserve
and link capacities differ by exactly one token moment,

\[
 \Delta K_{G,n}\in
 \left\{0,+{D\over18},-{D\over18}\right\},            \tag{14}
\]

with

\[
 \Delta K_{G,n}=0\iff\mu_n=0.                        \tag{15}
\]

Whenever the source is present,

\[
 \boxed{\|\Delta K_{G,n}\|_F^2=\|t_n\|_F^2={1\over324}.} \tag{16}
\]

The sign in equation (14) is not discarded: it is retained by the old
response ownership and is exactly what makes the kick reversible. Equation
(16) is a one-token response normalization, not a measured gravitational
coupling or an energy-work law.

Material charge conjugation gives

\[
 j_n\mapsto-j_n,\qquad t_n\mapsto t_n,\qquad
 \Delta K_{G,n}\mapsto\Delta K_{G,n}.                 \tag{17}
\]

The complete map commutes with that transformation. The response therefore
has precisely the even parity required of a stress/capacity channel.

---

## 5. Complete deterministic orbit census

The 256-state permutation decomposes exactly as

\[
 \boxed{16\times C_{12}\;\sqcup\;16\times C_4.}       \tag{18}
\]

Every sourced period-twelve orbit has

\[
 (N_{\rm global},N_{\rm admitted},N_{\rm source})=(12,8,8), \tag{19}
\]

one reserve-to-link material event, one inverse link-to-reserve event, and ten
ticks with no material ownership change. The material carrier completes its
period-eight proper clock in twelve global ticks:

\[
 {N_{\rm admitted}\over N_{\rm global}}={2\over3}.    \tag{20}
\]

The value \(2/3\) is only a property of this finite reference orbit. It is not
a gravitational prediction, a fitted redshift, or a proposed universal clock
rate.

Every period-four orbit is a closed, unsourced, zero-admission sector in which
only the response phase advances. Its existence is part of the exact
permutation census and must not be silently removed by declaring an
“operational” subset.

---

## 6. Additive-work obstruction

Consider the most general energy that is additive over the two finite
carriers:

\[
 E(M,G)=e_M(M)+e_G(G),                               \tag{21}
\]

where \(e_M\) and \(e_G\) are arbitrary real functions on the sixteen
physical A9 states. This is a 32-coefficient ansatz, not a quadratic or
low-degree truncation.

Imposing

\[
 E(\mathcal F(M,G))=E(M,G)                           \tag{22}
\]

on all 256 inputs gives an exact rank-28 linear system. Its four-dimensional
coefficient kernel is exhausted by functions of the two separately conserved
token-polarity labels:

\[
 e_M(M)=a_{\epsilon_M},\qquad
 e_G(G)=b_{\epsilon_G}.                              \tag{23}
\]

No additive invariant depends on C4 phase, link/reserve ownership, admitted
drift, or the response kick. Therefore

\[
 \boxed{\text{the two-A9 feedback map has no additive cross-sector work
energy beyond separately conserved polarity labels}.} \tag{24}
\]

This turns the earlier caution into an exact obstruction. Reversibility and
equal one-token moment norms do not yet constitute physical work. A genuine
native action must add a cross-sector interaction energy, permit an owned
energy transfer, or enlarge the carrier so that a nontrivial canonical
momentum can change while total energy remains fixed.

---

## 7. Why the same C4 carrier cannot also be Maxwell response

Let \(R\) denote the unit forward C4 phase advance. A charge-conjugation
involution \(C\) that is an internal symmetry of the same forward clock must
satisfy

\[
 CRC^{-1}=R.                                         \tag{25}
\]

A charge-odd unit source kick on that same phase orbit would instead require

\[
 CRC^{-1}=R^{-1}.                                    \tag{26}
\]

Equations (25) and (26) imply \(R=R^{-1}\), hence \(R^2=1\), contradicting the
exact order-four carrier.

The exhaustive permutation classification makes the boundary concrete:

- a four-cycle has ten involutions;
- exactly two commute with \(R\) (identity and the half-turn);
- exactly four conjugate \(R\) to \(R^{-1}\); and
- the two sets are disjoint.

Therefore

\[
 \boxed{\text{one C4 phase orbit cannot be both the forward clock and the
 charge-odd Maxwell response carrier}.}              \tag{27}
\]

This is not an obstruction to the one-action programme. It proves that the
unity must be interactional rather than type-identical: the even
stress/capacity response may use the A9 clock carrier, while the charge-odd
current must kick the distinct signed Maxwell/cotangent sector already
required by the phase-complete rank-fifty closure.

---

## 8. What is now closed

The following finite statements are theorem-grade:

1. a response carrier can generate the material admission permission rather
   than receiving an externally written permission word;
2. the admitted material clock generates the same persistent charge-current
   and stress-tensor readouts as the shared actualization vertex;
3. the charge-even stress source changes the response capacity by exactly one
   token moment;
4. that changed response capacity controls a later material tick;
5. the combined update is a total finite permutation with an explicit local
   inverse and no event log;
6. the complete sourced recurrence jointly contains manifestation, material
   persistence, global ticks, admitted proper ticks, stress sourcing, and
   capacity backpressure;
7. no additive two-A9 conserved energy can represent cross-sector work; and
8. the same single C4 phase orbit is excluded as a charge-odd Maxwell response
   carrier.

This closes the **finite reciprocal-capacity gate**, not the native-action
programme.

---

## 9. Exact boundary and next gate

This theorem does **not** derive:

1. equation (8) from a discrete variational principle or uniquely select its
   drift–kick–clock ordering;
2. physical work or a conserved cross-sector energy;
3. the charge-odd Maxwell field kick or Lorentz force;
4. spatial response propagation, a static \(1/r\) pole, a massless spin-2 or
   equivalent constrained tensor sector;
5. clock/fall equivalence, Shapiro delay, or lensing;
6. stable translating composite formation;
7. a physical Born pushforward; or
8. a native electromagnetic coupling measurement.

The next locked construction is now sharper:

1. lift the even kick \(t_n\) into the phase-complete tensor/capacity carrier;
2. lift the odd kick \(j_n\) into the distinct Maxwell carrier with field
   charge conjugation \(F\mapsto-F\);
3. place both kicks and the admitted material drift inside one conservative
   local generator;
4. derive a cross-sector energy/work invariant rather than merely conserving
   each token count; and
5. compose that generator with a layer-covariant streaming/collision symbol
   before testing poles or lensing.

The epistemic advance is narrow but real: FTD no longer needs an externally
supplied backpressure word to demonstrate reciprocal finite dynamics, while
the remaining Maxwell and physical-work costs are exposed rather than hidden.

**Successor status.** The
[common material/stress/Gauss transaction](THEOREM_C4_COMMON_MATERIAL_STRESS_GAUSS_TRANSACTION_AND_FIELD_BOUNDARY_v1.md)
adds the required distinct charge-odd cotangent packet to the same local
permutation and proves \(E/8=9j\) with exact Gauss incidence. It does not evade
the boundary above: that packet is bound source dressing until a conservative
release, propagation, and field-to-matter response law is derived.
