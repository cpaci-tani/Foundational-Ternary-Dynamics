# C4 autonomous reversible Born renewal detector v1

**Date:** 2026-08-24

**Status:** **[THEOREM — TOTAL FINITE REVERSIBLE RENEWAL PERMUTATION]** +
**[THEOREM, CONDITIONAL — PREPARED STEADY-STREAM EXCLUSIVE GAUSS-EVENT BORN
PUSHFORWARD]** + **[SELECTION — EVENT-DEFINED TRIAL/THREE-STAGE DETECTOR]** +
**[OPEN — NATIVE BANK FORMATION, EXTERNALLY HERALDED TRIALS, FINITE-WINDOW
ROBUSTNESS, MULTIPARTITE NO-SIGNALLING]**

**Physical Born status:** prepared-bank physical pushforward strengthened from a
preallocated detector tape to one autonomous reusable detector/source resource;
the general native Born rule remains open

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_autonomous_reversible_born_renewal_detector.py](../../../../../scripts/proofs/proof_c4_autonomous_reversible_born_renewal_detector.py)
performs 206,313 exact checks. It exhausts every one-outcome C4 count vector in
the box $\{0,1,2,3,4\}^4$, four multi-outcome preparations, the complete
pointer--detector product for each preparation, both manifestation
orientations, and every forward/inverse state.

---

## 1. The remaining tape and exclusivity debt

The
[physical Gauss-event tape](THEOREM_C4_BORN_TO_COTANGENT_PHYSICAL_GAUSS_EVENT_PUSHFORWARD_v1.md)
proved that a prepared residual C4 bank gives

\[
 M_o=|Z_o|^2                                                   \tag{1}
\]

field-bearing manifestations. It assigned one detector token and one
eight-record cotangent source packet to every state of the coprime address
orbit. For bank capacity $L$, that is $9L(L+1)$ independently owned working
tokens. The tape also displays all compatible events after one pass, leaving
the interpretation of one exclusive event at a time external.

This theorem removes those two prepared-detector assumptions. It retains the
prepared signal bank and address rings but reuses one detector/source resource
as an autonomous reversible renewal machine.

---

## 2. Prepared C4 bank and target-blind address clock

After payload-complete opposite-phase cancellation, let the shared bank
contain the residual records and explicit blanks. Its capacity is $L$. The
two address heads advance over consecutive rings, so pointer state
$n\in\mathbb Z_T$ reads

\[
 x_n=n\pmod L,\qquad y_n=n\pmod{L+1},\qquad
 T=L(L+1).                                                   \tag{2}
\]

Let $\chi(n)=1$ exactly when both addressed records are nonblank and carry the
same physical outcome route and surviving C4 phase. When bright, write that
unique route as $o(n)$. The coprime-ring theorem gives

\[
 B_o=|Z_o|^2,\qquad B=\sum_o B_o,                            \tag{3}
\]

where $B_o$ is the number of bright pointer states routed to $o$ in one
address period.

No transition below reads $Z_o$, $|Z_o|^2$, $B_o$, or a requested
probability.

---

## 3. One ternary renewal state

Let

\[
 q\in\{-1,0,+1\}                                             \tag{4}
\]

be the detector ownership stage:

\[
 q=0:\text{ ready/reserve},\qquad
 q=+1:\text{ manifested},\qquad
 q=-1:\text{ recovery}.                                     \tag{5}
\]

This uses the native balanced-ternary alphabet. The physical payload is one
phase/polarity detector token plus one eight-record cotangent source packet.
The stage may be realized as the ownership location of that retained payload;
the theorem does not add a continuous microscopic variable.

On the full finite product $\mathbb Z_T\times\{-1,0,+1\}$ define

\[
 F(n,0)=
 \begin{cases}
  (n,+1),&\chi(n)=1,\\
  (n+1,0),&\chi(n)=0,
 \end{cases}                                                 \tag{6}
\]

\[
 F(n,+1)=(n,-1),                                             \tag{7}
\]

and

\[
 F(n,-1)=
 \begin{cases}
  (n+1,0),&\chi(n)=1,\\
  (n,+1),&\chi(n)=0.
 \end{cases}                                                 \tag{8}
\]

All pointer additions are modulo $T$.

At an ordinary dark state, a ready detector immediately advances. At a bright
state it dwells through

\[
 (n,0)\longrightarrow(n,+1)\longrightarrow(n,-1)
 \longrightarrow(n+1,0).                                   \tag{9}
\]

The second branch of equation (8) is a fail-closed totalization: a
misprepared nonready detector at a dark pointer alternates between $+1$ and
$-1$ at that pointer and cannot enter the operational orbit.

---

## 4. Total permutation theorem

### Theorem 1

Equation (6)--(8) is a bijection of the entire $3T$-state
pointer--detector product.

### Proof

The inverse is explicit. For a ready state $(n,0)$, inspect the unique previous
pointer $m=n-1$:

\[
 F^{-1}(n,0)=
 \begin{cases}
  (m,-1),&\chi(m)=1,\\
  (m,0),&\chi(m)=0.
 \end{cases}                                                 \tag{10}
\]

For recovery,

\[
 F^{-1}(n,-1)=(n,+1).                                       \tag{11}
\]

For manifested label $+1$,

\[
 F^{-1}(n,+1)=
 \begin{cases}
  (n,0),&\chi(n)=1,\\
  (n,-1),&\chi(n)=0.
 \end{cases}                                                 \tag{12}
\]

Equations (10)--(12) are defined uniquely on all $3T$ states and are both a
left and right inverse of $F$. Hence $F$ is a permutation. $\square$

This is stronger than declaring only correctly prepared states admissible.
The unwanted states are present in the same finite alphabet but are dynamically
quarantined.

---

## 5. Operational orbit and autonomous reset

Start from any ready pointer state. The reached component contains:

1. one ready state for every address pointer;
2. one manifested state for every bright pointer; and
3. one recovery state for every bright pointer.

It is one cycle of exact length

\[
 \boxed{T_{\rm renewal}=T+2B.}                              \tag{13}
\]

Every dark pointer is traversed once. Every bright pointer is traversed once
and contributes the two additional dwell states in equation (9). After the
complete cycle, the pointers, detector stage, detector token, and source
packet return to their initial ownership.

Only one detector can occupy $q=+1$ because the construction has only one
detector resource. Thus simultaneous outcome competition is absent in the
prepared steady stream: every registered event has one route $o(n)$, and the
detector must recover before the next pointer is processed.

This is an autonomous reset theorem, not an appeal to irreversible erasure.
The recovery state retains the payload and supplies the unique inverse.

---

## 6. Physical Gauss event

At the transition $q=0\to+1$, move the retained detector token into manifested
bond ownership and its eight cotangent records into active field ownership.
For apparatus route $d_o$ and carried polarity $\epsilon$, the packet is
oriented as $\epsilon d_o$. It has

\[
 E_{\rm raw}=8\epsilon d_o,\qquad B_{\rm raw}=0,             \tag{14}
\]

and therefore one unit canonical electric edge after the already registered
$1/8$ packet normalization. With the existing selection $\rho=-s$, both
orientations obey

\[
 \boxed{\Delta\operatorname{div}E=\Delta\rho},
 \qquad \sum_x\Delta\rho(x)=0.                              \tag{15}
\]

The transition $+1\to-1$ returns the manifested endpoint/field ownership to a
retained recovery channel. The transition $-1\to0$ makes that same nine-token
resource ready for the next event. Signal records, opposite-phase dark records,
detector payload, and source records are never consumed.

---

## 7. Born event-frequency theorem

Count the entries into $q=+1$ during one renewal cycle. Every bright pointer is
entered exactly once, so

\[
 \boxed{M_o=B_o=|Z_o|^2.}                                   \tag{16}
\]

Whenever $B>0$, conditioning on a manifested renewal event gives

\[
 \boxed{
 f_o={M_o\over\sum_rM_r}
 ={|Z_o|^2\over\sum_r|Z_r|^2}.}                            \tag{17}
\]

Each event remains active for the same one-state dwell, so event counts and
manifested-state time counts give the same conditional ratio. The additional
recovery dwell affects the absolute event rate, not the relative outcome
frequency.

Combining equation (17) with the
[Gaussian-integer general-amplitude limit](THEOREM_C4_GAUSSIAN_INTEGER_GENERAL_AMPLITUDE_PHYSICAL_LIMIT_v1.md)
gives the same controlled convergence to every finite complex Born
distribution. The former $L(L+1)$ detector tape is now only an address
traversal time; the reusable detector/source working payload is nine finite
tokens rather than $9L(L+1)$. The prepared bank and physical ring carriers
still have finite size depending on $L$.

---

## 8. What this closes and what it does not

### Closed for the declared prepared steady-stream model

- no prewritten detector or source-packet tape;
- one target-blind finite reversible renewal rule;
- one exclusive outcome route per event;
- autonomous finite recovery/reset;
- exact field-bearing counts $M_o=|Z_o|^2$;
- exact payload and Gauss-incidence retention; and
- a total permutation on the full pointer--detector product.

### Still open

1. native generation of the residual C4/Gaussian-integer bank from a source;
2. Moore-local formation of the shared bank and two coprime address rings;
3. externally heralded preparations with exactly one click assigned to each
   source emission rather than event-defined renewal trials;
4. finite-window discrepancy and phase-of-entry robustness before a complete
   renewal cycle;
5. macroscopic amplification and surviving apparatus records;
6. sequential context changes and detector/source backreaction;
7. multipartite spacelike composition with operational no-signalling; and
8. derivation of equations (6)--(8) from the common action rather than
   selection of the finite permutation.

Accordingly this theorem advances the physical Born pushforward but does not
establish stochastic fundamental probability or the general quantum
measurement postulates.

---

## 9. Consequence for the one-action program

The one-action target no longer needs an $O(L^2)$ prepared detector tape. It
must generate only:

\[
 \text{source histories}
 \to \text{residual routed C4 bank}
 \to \text{two local address circulations}
 \to \text{one ternary renewal detector/Gauss resource}.     \tag{18}
\]

The next locked Born gate is therefore narrower: generate the bank and ring
ownership from the same phase-complete transaction action, couple each source
emission to one renewal event without postselection, and prove finite-window
and multipartite no-signalling behavior without changing equations
(6)--(17).

The subsequent
[heralded fixed-window Poincare pushforward](THEOREM_C4_HERALDED_FIXED_WINDOW_BORN_POINCARE_PUSHFORWARD_v1.md)
closes the externally heralded one-event assignment and incomplete-window
bound for a selected prepared, isolated-source model. Bright addresses form
one reversible cyclic section; each source herald advances to the next bright
address, latches that route, and releases exactly one Gauss event at a common
$T$-tick endpoint. Every $B$-trial block has the exact counts
$M_o=|Z_o|^2$, while an arbitrary $N$-trial window has total-variation error
strictly below $B/N$. Native bank/ring/herald-counter formation, overlapping
traffic, action realization, amplification, and multipartite no-signalling
remain open.
