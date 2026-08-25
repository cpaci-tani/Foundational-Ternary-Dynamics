# THEOREM — C4 common material/stress/Gauss transaction and field boundary v1

**Date:** 2026-08-24  
**Status:** [THEOREM — EXACT FINITE MATCHED-CARRIER PERMUTATION] +
[THEOREM — COMMON CHARGE/STRESS SOURCE REALIZATION] +
[BOUNDARY — GAUSS PACKET IS SOURCE DRESSING, NOT PROPAGATING RESPONSE]  
**Ontology status:** composes already priced candidate resources; no new
alphabet is introduced  
**Purpose:** put manifestation, the autonomous material clock, charge-odd
electromagnetic Gauss sourcing, and charge-even stress/capacity feedback into
one reversible local transaction.

**Exact certificate:**
[proof_c4_common_material_stress_gauss_transaction.py](../../../../../scripts/proofs/proof_c4_common_material_stress_gauss_transaction.py)
exhausts 3,072 matched states on each of the three unoriented SC lines, checks
the total map and inverse, verifies all source/readout/Gauss identities and
charge conjugation, and enumerates every composite orbit.

---

## 1. The three finite resources

On one unoriented SC line \(b=[x,y]\) with selected positive chart direction
\(d\), use:

1. one A9 material carrier \(M\);
2. one independently owned A9 stress/capacity response carrier \(G\); and
3. one eight-record stabilizer-complete cotangent packet \(P\).

The exact local token price is therefore

\[
 N_{\rm token}=1+1+8=10.                             \tag{1}
\]

The cotangent packet is not one preferred perpendicular flag. It is the full
eight-element orbit of the \(D_4\) stabilizer of \(d\), so its active field
readout is context-free and cubic-covariant:

\[
 E_P=8d_P,\qquad B_P=0.                              \tag{2}
\]

Its internal free clock has twelve stages. Let \(U\) denote one exact packet
advance and \(U^{12}=1\).

---

## 2. Matched source subspace

Let the material token polarity be

\[
 \epsilon=\epsilon(M)\in\{-1,+1\}.                  \tag{3}
\]

The packet direction and ownership are matched to the material source:

\[
 d_P=\epsilon d,\qquad
 n_P=n(M),\qquad c_P=1-n_P.                          \tag{4}
\]

This relation contains both charge signs. Charge conjugation reverses
\(\epsilon\) and \(d_P\) while preserving packet occupation.

The packet's twelve-stage internal record is independent of whether it is
active or reserve-owned. Thus an ownership event moves the complete packet
without destroying its phase/flag payload.

---

## 3. One common transaction

Read the old stress capacity

\[
 a_n=c(G_n).                                         \tag{5}
\]

Perform the admitted material tick:

\[
 M_{n+1}=T^{a_n}M_n.                                 \tag{6}
\]

The material ownership-event bit is

\[
 \eta_n=n(M_{n+1})\oplus n(M_n).                     \tag{7}
\]

It moves the complete Gauss packet between reserve and active ownership:

\[
 \widetilde P_n=A_P^{\eta_n}P_n.                    \tag{8}
\]

Read the post-drift persistent stress source

\[
 \mu_n=n(M_{n+1})                                    \tag{9}
\]

and kick the stress response:

\[
 \widetilde G_n=A_G^{\mu_n}G_n.                     \tag{10}
\]

Finally advance the global response and cotangent clocks:

\[
 G_{n+1}=R\widetilde G_n,\qquad
 P_{n+1}=U\widetilde P_n.                            \tag{11}
\]

Equations (5)–(11) define the single local map

\[
 \boxed{
 \mathcal F_{\rm CSG}(M_n,G_n,P_n)
 =(M_{n+1},G_{n+1},P_{n+1}).}                       \tag{12}
\]

Its ordered semantics are

\[
 \boxed{
 \text{stress capacity}
 \to\text{material clock/actualization}
 \to\text{Gauss packet ownership}
 \to\text{persistent stress kick}
 \to\text{global field clocks}.}                    \tag{13}
\]

---

## 4. Exact inverse

Given the output:

1. undo the cotangent and response clocks,
   \(\widetilde P_n=U^{-1}P_{n+1}\) and
   \(\widetilde G_n=R^{-1}G_{n+1}\);
2. read \(\mu_n=n(M_{n+1})\) and undo the stress kick;
3. read the recovered old permission \(a_n=c(G_n)\);
4. undo the material tick, \(M_n=T^{-a_n}M_{n+1}\);
5. reconstruct \(\eta_n=n(M_{n+1})\oplus n(M_n)\); and
6. undo the packet ownership event.

Every operation is local and unique. Exhaustive evaluation proves

\[
 \mathcal F_{\rm CSG}^{-1}\mathcal F_{\rm CSG}
 =\mathcal F_{\rm CSG}\mathcal F_{\rm CSG}^{-1}=1    \tag{14}
\]

on all

\[
 16_{\rm matter}\times16_{\rm stress}\times12_{\rm packet}
 =3072                                               \tag{15}
\]

matched states per SC line. The image is the complete matched state set.

---

## 5. Electromagnetic source identity

When the material token is manifested, the phase-neutral current is

\[
 j={\epsilon d\over9}.                               \tag{16}
\]

The simultaneously active cotangent packet obeys

\[
 {E_P\over8}=\epsilon d=9j,\qquad B_P=0.             \tag{17}
\]

When the material token is reserve-owned, both \(j\) and the active packet
field vanish. Therefore equation (17) holds on every matched state, not only
at the ownership transition.

Using the material endpoints as the charge cochain,

\[
 \rho(x)=-s_x,\qquad \rho(y)=-s_y,                   \tag{18}
\]

the oriented packet edge satisfies exactly

\[
 \boxed{\partial E_P=\rho,\qquad \sum_z\rho(z)=0.}   \tag{19}
\]

For \(\epsilon=+1\), the edge runs from \(x\) to \(y\); for
\(\epsilon=-1\), the oriented packet reverses. This is the exact finite
charge-odd realization demanded by the preceding one-C4 parity obstruction.

Equation (17) is a target-free chart normalization between the one-token
material current and the eight-record canonical packet. It is not the
fine-structure constant and does not determine a Coulomb residue.

---

## 6. Stress/capacity source identity

On the same manifested state, the phase-neutral even tensor source is

\[
 t={dd^{\mathsf T}\over18}.                          \tag{20}
\]

The stress response changes ownership if and only if this source is present:

\[
 \Delta K_G\in
 \left\{+{dd^{\mathsf T}\over18},
        -{dd^{\mathsf T}\over18}\right\},            \tag{21}
\]

and

\[
 \boxed{\|\Delta K_G\|_F^2=\|t\|_F^2={1\over324}.}   \tag{22}
\]

Thus the same material state has two distinct but simultaneous outputs:

\[
 \boxed{
 \begin{aligned}
 \epsilon d/9 &\longrightarrow \text{charge-odd Gauss packet},\\
 dd^{\mathsf T}/18
 &\longrightarrow \text{charge-even capacity response}.
 \end{aligned}}                                     \tag{23}
\]

The two sectors are joined by the material transaction, not by a forbidden
vacuum linear intertwiner.

---

## 7. Charge conjugation

The complete composite conjugation is

\[
 M\mapsto\overline M,\qquad
 G\mapsto G,\qquad
 d_P\mapsto-d_P.                                    \tag{24}
\]

It commutes with the full map:

\[
 \mathcal C\mathcal F_{\rm CSG}
 =\mathcal F_{\rm CSG}\mathcal C.                   \tag{25}
\]

Consequently

\[
 j\mapsto-j,\qquad E_P\mapsto-E_P,\qquad
 t\mapsto t,\qquad G\mapsto G.                      \tag{26}
\]

This is the required charge-parity split in one exact finite transaction.

---

## 8. Complete orbit census

For each SC line, all 3,072 states lie on period-twelve orbits:

\[
 \boxed{192\text{ sourced }C_{12}\text{ orbits}
 \;\sqcup\;64\text{ closed }C_{12}\text{ orbits}.}   \tag{27}
\]

Every sourced orbit has

\[
 (N,N_{\rm admitted},N_{\rm source},N_{\rm packet\ active},
 N_{\rm ownership\ events})
 =(12,8,8,8,2).                                     \tag{28}
\]

The two ownership events are one manifestation and its inverse. The packet
remains active throughout the eight manifested states, while the material
proper clock accumulates eight admitted ticks per twelve global packet ticks.

Every closed orbit has zero admissions, zero material source, zero active
packet states, and zero ownership events; only the twelve-stage reserve packet
clock advances.

The reference rates in equation (28) are census facts, not physical
predictions.

---

## 9. What is now closed

The following statements are theorem-grade:

1. the stress feedback map and cotangent Gauss source can occupy one matched
   finite state space;
2. one ordered local map advances both and has a unique inverse;
3. material actualization activates and de-actualization returns the complete
   eight-record packet without payload loss;
4. the persistent material current equals the normalized active electric
   packet exactly;
5. the packet boundary equals the material endpoint charge;
6. the same material state simultaneously drives the even stress/capacity
   response;
7. charge conjugation reverses the current/electric packet and preserves the
   tensor/stress response; and
8. global packet ticks, admitted material ticks, manifestation, and
   backpressure coexist on every sourced composite orbit.

This is the first finite local map in the programme containing material,
clock, electromagnetic Gauss sourcing, and stress backpressure together.

---

## 10. Exact boundary and next gate

The active cotangent packet in this theorem is **source dressing**. It remains
owned by the same local composite until the inverse material event returns it
to reserve. The theorem does not derive:

1. a distinct number-neutral transverse response released without consuming
   the bound Gauss packet;
2. a conservative collision/streaming map with the two transverse Maxwell
   poles;
3. electromagnetic field-to-matter momentum transfer or a Lorentz force;
4. a cross-sector work/energy invariant;
5. the phase-complete rank-fifty Maxwell/tensor physical quotient;
6. a static Coulomb or tensor \(1/r\) pole;
7. a massless spin-2/equivalent sector, Shapiro delay, or lensing;
8. native preparation and physical Born basin weights; or
9. a native measurement of \(\alpha\).

The
[framed-plaquette radiation successor](../charge_gauss_native_em/THEOREM_COTANGENT_FRAMED_PLAQUETTE_NUMBER_NEUTRAL_RADIATION_RELEASE_v1.md)
proves that letting this packet leave directly is the wrong state type. It
constructs instead a distinct 64-record particle--hole circulation with
\(\Delta N=0\) and \(\partial\Delta E=0\), whose first Bloch moment lies in the
certified transverse Maxwell sector. Its four-way plane context is the quotient
$v=hn$ and can be supplied by an ordered perpendicular material turn. The next
locked action step is therefore to make the material recurrence retain that
turn, compose the finite-amplitude collision/streaming schedule, and make its
signed field momentum act reciprocally on the material route. That lift must
share a conservative energy ledger with the even tensor/capacity response
before either pole is interpreted physically.

The later
[square-material turn theorem](THEOREM_C4_SQUARE_MATERIAL_TURN_CLOCK_AND_ENDOGENOUS_RADIATION_FRAME_v1.md)
does retain the required ordered turn on a prepared period-four neutral loop.
Its
[reciprocal-work successor](THEOREM_C4_SQUARE_MATTER_STRESS_RADIATION_RECIPROCAL_WORK_EXCHANGE_v1.md)
then product-composes that loop with an A9 stress-capacity owner and the
number-neutral seed, closing a local complementary energy ledger. The bound
eight-record Gauss dressing has not yet been included in that exhaustive
product, and the transverse seed has not yet acquired propagation, momentum,
or reciprocal force, so the full conservative common-action gate remains open.
