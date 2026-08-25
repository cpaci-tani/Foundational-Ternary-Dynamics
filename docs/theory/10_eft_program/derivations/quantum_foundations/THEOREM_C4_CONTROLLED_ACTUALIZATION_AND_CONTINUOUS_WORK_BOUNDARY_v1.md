# C4 controlled actualization and continuous-work boundary v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT REVERSIBLE C4-CONTROLLED MANIFESTATION MACRO]** +
**[THEOREM — FINITE RESERVE CANNOT PAY A GENERIC CONTINUOUS SWITCHING WORK]** +
**[SELECTION — DISCRETE-FIRST ROUTE MAKES THE REAL FIELD ACTION EFFECTIVE]** +
**[OPEN — MICROSCOPIC TOKEN ACTION, BLOCKING TO THE BOND HAMILTONIAN, STABLE MATTER]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_controlled_actualization_transaction.py](../../../../../scripts/proofs/proof_c4_controlled_actualization_transaction.py)
checks the controlled macro, inverse, charge neutrality, token payload,
capacity complement, C4 covariance, event-bit reset, and fail-closed collision
states. The certificate proves a finite token ledger only; it does not claim
the continuous bond Hamiltonian work is closed.

---

## 1. Why this boundary matters

The one-action program currently combines:

- finite ternary manifestation states;
- finite C4 bond records; and
- real canonical fields \(q_\pm,p_\pm\).

A local manifestation changes the finite occupancy/capacity state and
therefore changes the real bond Hamiltonian by a generally continuous amount.
The inverse transaction must preserve both the complete record and the exact
energy. This document separates what a finite record can do exactly from what
requires either a continuous work carrier or a fully discrete microscopic
field ontology.

---

## 2. One oriented phase token

Let one complete reserve token be

\[
 \tau=(k,\epsilon),\qquad
 k\in\mathbb Z_4,\quad \epsilon\in\{-1,+1\}.         \tag{1}
\]

Its phase is \(i^k\), and \(\epsilon\) is the local oriented-current sign that
determines which endpoint receives \(+1\). Define the two valid ownership
states

\[
 V_\tau=(s_x,s_y;\lambda;R)=(0,0;0;\tau),            \tag{2}
\]

\[
 M_\tau=(s_x,s_y;\lambda;R)
 =(\epsilon,-\epsilon;\tau;0).                       \tag{3}
\]

The token is not created or destroyed. Equation (2) is dispositional reserve
ownership; equation (3) is actual bond/matter ownership.

---

## 3. Palindromic controlled transaction

Let \(c\in\mathbb Z_2\) be the local reversible event bit produced by the
[C4 cancellation/click circuit](THEOREM_REVERSIBLE_C4_CANCELLATION_AND_CLICK_CIRCUIT_v1.md),
and let \(\chi\in\{0,1\}\) be its physical compatibility predicate. Define:

\[
 \mathfrak C_\chi:c\mapsto c\mathbin{\mathrm{xor}}\chi, \tag{4}
\]

\[
 \mathfrak M_c:
 \begin{cases}
 V_\tau\longleftrightarrow M_\tau,&c=1,\\
 \text{fixed},&c=0.
 \end{cases}                                         \tag{5}
\]

The complete local macro is

\[
 \boxed{
 \mathfrak A_\chi
 =\mathfrak C_\chi\,
  \mathfrak M_c\,
  \mathfrak C_\chi.}                                 \tag{6}
\]

The two compatibility operations are the arm and disarm subphases of the same
global tick while the address heads are held fixed.

### Theorem 1 — exact finite controlled actualization

\(\mathfrak A_\chi\) is an involution. Starting with \(c=0\):

\[
 \chi=0:\quad V_\tau\mapsto V_\tau,\qquad
 \chi=1:\quad V_\tau\longleftrightarrow M_\tau,       \tag{7}
\]

and the event bit returns to zero. The macro preserves:

1. net ternary charge \(s_x+s_y=0\);
2. exactly one complete C4 token;
3. the phase and orientation payload \((k,\epsilon)\);
4. the complement of link capacity and link occupation;
5. global C4 covariance; and
6. the positive microscopic token energy

\[
 E_{\rm token}=E_*
 \bigl(n_\lambda+n_R\bigr)=E_*,\qquad E_*>0.          \tag{8}
\]

States with a missing reserve, a preoccupied link, inconsistent endpoint
charges, or simultaneous link/reserve ownership remain fixed as
collision/backpressure states.

### Proof

Both \(\mathfrak C_\chi\) and \(\mathfrak M_c\) are involutions. The
palindrome in equation (6) therefore satisfies

\[
 \mathfrak A_\chi^2
 =\mathfrak C_\chi\mathfrak M_c
  \mathfrak C_\chi\mathfrak C_\chi
  \mathfrak M_c\mathfrak C_\chi
 =1.                                                 \tag{9}
\]

Equations (2)--(3) retain the same token and opposite endpoint charges. A
common shift \(k\mapsto k+q\) commutes with ownership transfer. Equation (8)
counts the same one token on both sides. \(\square\)

This is the minimum exact finite manifestation transaction presently visible.
It uses no random acceptance probability and no outcome weight.

---

## 4. Finite-reserve continuous-work no-go

Let a finite reserve alphabet be \(\mathcal R_f\), with energy
\(e:\mathcal R_f\to\mathbb R\). Its possible energy transfers form the finite
set

\[
 D_f=\{e(r')-e(r):r,r'\in\mathcal R_f\}.             \tag{10}
\]

Let \(X\) be a connected set of real field configurations, and let the
manifestation switch change the field Hamiltonian by a continuous,
nonconstant function

\[
 \Delta H:X\to\mathbb R.                             \tag{11}
\]

### Theorem 2 — finite work alphabet obstruction

No exact energy-conserving transition using only \(\mathcal R_f\) can be
available for every \(x\in X\).

### Proof

Exact conservation would require

\[
 \Delta H(x)\in-D_f\qquad\text{for every }x\in X.    \tag{12}
\]

The continuous image \(\Delta H(X)\) of a connected set is connected.
Because \(\Delta H\) is nonconstant, that image contains more than one point
and hence an interval. A finite subset of \(\mathbb R\) contains no nontrivial
interval, contradicting equation (12). \(\square\)

Restricting transitions to finitely many level sets can evade the theorem,
but then generic configurations cannot manifest. Enlarging the reserve while
keeping it finite only enlarges \(D_f\); it does not remove the obstruction.

---

## 5. Application to the current bond-action skeleton

For one available bond, the real-field portion of the candidate Hamiltonian
contains terms of the form

\[
 H_b(q_+,q_-)
 ={a_b\over2}
 \left[(Bq_+)_b^2+(Bq_-)_b^2\right].                \tag{13}
\]

Moving a C4 record from reserve ownership to link ownership changes the
capacity coefficient and therefore changes equation (13) by a continuous,
nonconstant function of the real field amplitudes. The finite token energy in
equation (8) cannot by itself pay that generic change.

Consequently the current
[phase-complete bond-action scope](../../scopes_and_specs/SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md)
has an unavoidable interpretation fork:

### Route A — continuous-work ontology

Treat \(q_\pm,p_\pm\) as ontically real canonical fields and designate an
existing local canonical normal mode as the positive work reservoir. A
symplectic energy-transfer map and its finite ownership still have to be
derived.

### Route B — discrete-first ontology

Treat the C4/ternary transactions as microscopic and equations (13) and the
real \(q_\pm,p_\pm\) fields as blocked effective observables. Exact native work
is then conserved by finite record/token permutations such as equations
(6)--(8), while the continuous quadratic Hamiltonian must be derived as a
large-block response functional.

**[SELECTION]** The user's declared discrete-first ontology selects Route B
for this research branch. This is not a production change and does not erase
the current framework's explicit real \(J\) field; it states the derivation
required to reinterpret that field as emergent.

---

## 6. Revised one-action architecture

Under Route B, the requested hierarchy becomes:

\[
 \boxed{
 \text{finite reversible transaction action}
 \xrightarrow{\text{blocking}}
 \text{real common/relative field action}
 \xrightarrow{\text{response}}
 \text{continuum physics}.}                          \tag{14}
\]

The microscopic state must contain only finite local records, capacities,
ternary actual states, and a finite phase-complete environment sufficient for
inverse transactions. Its one gate family must generate:

- streaming of oriented records;
- opposite-phase transfer to dark memory;
- compatible-event actualization by equation (6);
- reciprocal unmanifestation;
- collision/backpressure; and
- complete record export rather than erasure.

The C18 block consequences of the same ownership transfer are now exact in the
[actualization shared-moment source theorem](../common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md).
One token manifestation simultaneously creates the neutral ternary endpoints,
a relative-vector increment, a common tensor-doublet increment, and a capacity
debit, with a common inverse and target-free norm ledger. This is a kinematic
source vertex; autonomous compatibility, blocked propagation, work, stable
matter, lensing, Born, and native coupling remain open.

The blocked first and second moments are then the candidates

\[
 J_i^{\rm rel}
 \sim\left\langle\sum_b \sigma_b n_b d_{b,i}\right\rangle, \tag{15}
\]

\[
 \mathcal K_{ij}
 \sim\left\langle\sum_b c_b d_{b,i}d_{b,j}\right\rangle.   \tag{16}
\]

Equation (15) is the electromagnetic/current channel. Equation (16) is the
common capacity trace-plus-shear channel. The matter clock is the recurrence
phase of the same token network. This is the intended single-action
unification; none of those infrared identifications is proved by the finite
transaction alone.

---

## 7. What is now closed and what remains open

### Closed exactly at the finite transaction level

- target-blind compatibility arm/disarm;
- reversible reserve-to-link manifestation;
- net-charge neutrality;
- complete phase/orientation payload retention;
- capacity debit and inverse;
- one-token positive energy conservation;
- C4 covariance; and
- fail-closed collision states.

### Still open

1. a non-tautological microscopic action selecting the gate family;
2. a Moore-local router and finite dark-memory ownership;
3. stable self-maintaining matter rather than detector-bound pairs;
4. derivation of the body clock from a localized recurrence;
5. blocking from token permutations to equation (13);
6. electromagnetic response and a blind native coupling;
7. capacity-shear dynamics, lensing, and two physical tensor modes;
8. the general-amplitude Born limit and multipartite no-signalling; and
9. detector amplification and record-preserving reset.

---

## 8. Next gate

The next action calculation must not add another finite work token to the
hybrid real-field Hamiltonian; Theorem 2 shows why that cannot close generic
work.

Instead, freeze one of two explicit pre-registrations:

1. **Route-A symplectic reservoir:** name the existing canonical normal mode,
   derive its exact energy-transfer map, and keep real \(J\) ontic; or
2. **Route-B blocking:** freeze a finite reversible streaming/collision action
   and derive its quadratic common/relative response without inserting the
   continuum Hamiltonian.

For the discrete-first program, Route B is the aligned next step. A failure to
derive the real response from the finite action falsifies the proposed
ontology bridge rather than licensing the quadratic field action as
microscopic by assertion.
