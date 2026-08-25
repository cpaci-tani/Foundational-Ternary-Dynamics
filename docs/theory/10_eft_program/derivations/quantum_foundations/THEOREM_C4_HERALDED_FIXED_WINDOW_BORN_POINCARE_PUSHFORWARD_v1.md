# C4 heralded fixed-window Born Poincare pushforward v1

**Date:** 2026-08-24

**Status:** **[THEOREM — BRIGHT-SECTION CYCLIC RETURN MAP AND EXACT COMPLETE-CYCLE COUNTS]** +
**[THEOREM, CONDITIONAL — ONE PHYSICAL GAUSS EVENT PER ISOLATED HERALD]** +
**[THEOREM, CONDITIONAL — FIXED-WINDOW TIMING AND FINITE-WINDOW BOUND]** +
**[SELECTION — PREPARED BANK/RINGS, HERALD LATCH, AND FINITE COUNTER]** +
**[OPEN — NATIVE PREPARATION, OVERLAPPING TRAFFIC, ACTION, AND MULTIPARTITE NO-SIGNALLING]**

**Physical Born status:** the prepared finite C4 construction now supports one
exclusive field-bearing event for each isolated external source herald, with
fixed external trial duration and an exact deterministic finite-window error
bound. The autonomous general Born rule remains open.

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_c4_heralded_fixed_window_born_poincare_pushforward.py](../../../../../scripts/proofs/proof_c4_heralded_fixed_window_born_poincare_pushforward.py)
performs 280,936 exact checks. It exhausts every one-outcome C4 count vector in
$\{0,1,2,3\}^4$, four multi-outcome preparations, every entry phase of every
nonempty bright section, trial windows through three complete bright cycles,
both manifestation orientations, and the canonical cotangent Gauss event.

---

## 1. The trial-assignment problem

The
[autonomous renewal detector](THEOREM_C4_AUTONOMOUS_REVERSIBLE_BORN_RENEWAL_DETECTOR_v1.md)
registers every bright pointer encountered by a continuously running prepared
address clock. It therefore proves a steady-stream event frequency, but it
does not yet associate one externally identifiable source emission with
exactly one outcome. It also allows the waiting time to the next bright
pointer to depend on the pointer entry phase.

This theorem closes those two debts for isolated heralds by using the bright
addresses themselves as a Poincare section and padding every trial to one
common address period. The source herald, bank, counter, and isolation
condition are declared selections, not outputs of the common action.

---

## 2. Prepared section

Let a payload-complete residual C4 bank have finite capacity $L$ and let the
two coprime address heads define

\[
 n\in\mathbb Z_T,\qquad T=L(L+1).                         \tag{1}
\]

As before, $\chi(n)=1$ precisely when the addressed ordered history pair is
phase-compatible and routed to a physical outcome $o(n)$. Define the bright
section

\[
 \Sigma=\{n\in\mathbb Z_T:\chi(n)=1\},\qquad
 B=|\Sigma|=\sum_o B_o,                                   \tag{2}
\]

where the coprime-ring count theorem gives

\[
 \boxed{B_o=|Z_o|^2}.                                     \tag{3}
\]

The nontrivial case is $B>0$. A preparation with $B=0$ has no admissible
manifestation event and is outside the conditioned trial ensemble.

---

## 3. Bright successor as a reversible return map

For $n\in\Sigma$, let $d(n)\in\{1,\ldots,T\}$ be the least positive distance
to the next bright address and define

\[
 R(n)=n+d(n)\pmod T.                                      \tag{4}
\]

### Theorem 1

$R$ is a single cyclic permutation of $\Sigma$. Its inverse is the scan to
the preceding bright address, and

\[
 \sum_{n\in\Sigma}d(n)=T.                                 \tag{5}
\]

### Proof

The cyclic order inherited from $\mathbb Z_T$ gives every bright address one
unique successor and one unique predecessor. Successive applications of $R$
visit all bright addresses before returning, because no bright address lies
between $n$ and $R(n)$. The disjoint forward gaps partition the address ring,
so their sum is $T$. $\square$

No amplitude, norm, probability, or requested outcome appears in the update.
The micro-path is an ordinary forward scan of the prepared address ring.

---

## 4. One fixed-duration heralded event

Retain the previously used bright pointer $n_k\in\Sigma$. When one isolated
source herald arrives:

1. release the address scan immediately after $n_k$;
2. scan $d(n_k)$ local address steps to $n_{k+1}=R(n_k)$;
3. latch the unique route $o(n_{k+1})$ while retaining the herald token;
4. hold the latched route for $T-d(n_k)$ counter ticks; and
5. at the common $T$-tick endpoint, transfer the retained detector token and
   eight-record cotangent packet into one physical Gauss event.

Thus every admitted herald satisfies

\[
 d(n_k)+[T-d(n_k)]=T                                      \tag{6}
\]

and produces exactly one outcome at the same external completion time. The
route is selected by the next bright section point, not by an amplitude read.

The reverse description starts from the retained endpoint, route latch,
counter phase, detector/source payload, and herald record, unholds the route,
and scans to the preceding bright address. No signal record, canceled dark
record, source herald, route latch, detector token, or field packet is erased.
This is why the construction is compatible with finite reversible dynamics.

The physical release is the already certified cotangent event. For route
$d_o$ and polarity $\epsilon$ it has

\[
 E_{\rm raw}=8\epsilon d_o,\qquad B_{\rm raw}=0,
 \qquad \Delta\operatorname{div}E=\Delta\rho,              \tag{7}
\]

with zero net charge on the finite source pair.

Equation (6) removes an outcome-dependent completion-time side channel in
this isolated prepared model. It is not a proof of relativistic
no-signalling, because neither spacelike detector composition nor overlapping
traffic has been supplied.

---

## 5. Exact frequency and finite-window theorem

Let the starting point $n_0\in\Sigma$ be arbitrary. The outcomes of successive
heralds are

\[
 o(R(n_0)),\ o(R^2(n_0)),\ldots.                            \tag{8}
\]

### Theorem 2

Every block of $B$ successive heralded trials visits every bright address
once, independently of $n_0$. Hence

\[
 \boxed{M_o(B)=B_o=|Z_o|^2},                               \tag{9}
\]

and, conditional on an event,

\[
 \boxed{f_o={|Z_o|^2\over\sum_r|Z_r|^2}}.                 \tag{10}
\]

For an arbitrary $N\ge1$, write $N=qB+r$ with $0\le r<B$ and let $\widehat
p_N$ be the empirical outcome distribution. Then

\[
 \boxed{
 d_{\rm TV}\!\left(\widehat p_N,{(B_o)_o\over B}\right)
 \le {r\over N} < {B\over N}.}                            \tag{11}
\]

### Proof

The return map is one $B$-cycle, so each complete block contributes exactly
$B_o$ events to route $o$. Only the final $r$ trials differ from an integral
number of complete cycles. The total variation between two nonnegative
measures of common total mass $r$ is at most $r$; division by $N$ proves
equation (11). $\square$

This is a deterministic discrepancy theorem. It does not introduce an ontic
random variable. Apparent probability is the pushforward of incomplete
knowledge of the entry point and/or a finite observation window through the
physical return map.

---

## 6. What is closed and what remains open

### Closed for the selected prepared, isolated-herald model

- one physical field-bearing manifestation per admitted source herald;
- exact C4/Gaussian-integer Born multiplicities over every complete section
  cycle;
- entry-phase independence of complete-cycle counts;
- an exact $O(B/N)$ finite-window discrepancy bound;
- identical $T$-tick external completion time for every outcome;
- one reusable detector/source payload; and
- reversible retention of the records needed by the inverse.

### Still open

1. native formation of the residual history bank and coprime address rings;
2. derivation of the herald latch and $T$-state counter from the common action;
3. physical generation and isolation of one herald per prepared source event;
4. concurrent or overlapping source traffic;
5. macroscopic amplification and durable apparatus records;
6. sequential changes of measurement context and apparatus backreaction;
7. multipartite spacelike composition and operational no-signalling; and
8. a measure theorem for generic dynamically formed history ensembles.

The result therefore does not refute the mathematical Born rule or derive the
full quantum measurement formalism. It supplies a finite reversible ontology
whose selected prepared event counts have the Born form.

---

## 7. One-action consequence

The measurement branch of the common-action program is now localized to

\[
 \text{source transaction}
 \longrightarrow \text{phase-complete residual bank/rings}
 \longrightarrow \text{isolated herald}
 \longrightarrow \text{one fixed-window Gauss event}.      \tag{12}
\]

The locked gate is no longer the combinatorics of squared amplitudes, event
exclusivity, reset, or finite-window timing. It is the autonomous causal
production of the bank, clocked trial apparatus, and multipartite composition
by the same action that supplies matter, fields, recoil, and clocks.

### Subsequent physical-record vertex (2026-08-24)

The selected
[reciprocal packet/clock/recoil absorption generator](../common_action_mechanics_reciprocity/THEOREM_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_AND_GRAVITY_SOURCE_BOUNDARY_v1.md)
provides one exact downstream readout for equation (12): the released Gauss
packet can become a retained packet history plus a finite apparatus-clock and
recoil record while preserving energy and reversibility. This closes neither
native bank preparation nor a general Born pushforward. It only proves that a
prepared routed event can terminate in a physical, non-erasing local record
without adding a separate irreversible measurement rule.
