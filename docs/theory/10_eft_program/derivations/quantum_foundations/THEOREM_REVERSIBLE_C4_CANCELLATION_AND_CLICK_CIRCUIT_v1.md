# Reversible C4 cancellation and click circuit v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT LOCAL C4 FUSION/CANCELLATION INVOLUTION]** +
**[THEOREM — EXACT NONDESTRUCTIVE CLICK COMPARATOR]** +
**[THEOREM, CONDITIONAL — CANONICAL RESIDUAL CIRCUIT PLUS COPRIME ORBIT GIVES BORN COUNTS]** +
**[SELECTION CANDIDATE — DETECTOR MEMORY MECHANICS]** +
**[OPEN — ACTION-GENERATED ROUTING, WORK, MANIFESTATION, RESET, GENERAL AMPLITUDES, MULTIPARTY CAUSALITY]**  
**Physical Born-rule status:** finite equal-weight C4 cancellation and
enumeration circuit constructed; general physical derivation remains open  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_reversible_c4_cancellation_click_circuit.py](../../../../../scripts/proofs/proof_reversible_c4_cancellation_click_circuit.py)
checks gate inversion, record conservation, C4 covariance, canonical
opposite-phase residuals, dark-record completeness, nondestructive click
reversal, and exact normalized multi-outcome counts. It uses exact finite
states and rational arithmetic, with no numerical fit or target constant.

---

## 1. Purpose

The
[paired-history identity](THEOREM_C4_PAIRED_HISTORY_BORN_COUNT_AND_PHYSICAL_BOUNDARY_v1.md)
proved the square, and the
[coprime-address theorem](THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md)
gave a target-blind deterministic orbit through every ordered address pair.
Two physical questions remained:

1. Can opposite phases leave the active channel without ontic deletion?
2. Can a compatible address pair register reversibly, including a record
   paired with its own address?

The finite gates below answer both questions exactly. They do not yet derive
their hardware or work exchange from the common action.

---

## 2. Phase records and the one relation test

An active detector-memory record is

\[
 h=(o,k,\iota),\qquad k\in\mathbb Z_4,               \tag{1}
\]

where \(o\) is a physical apparatus port, \(k\) is its C4 phase exponent,
and \(\iota\) is a history identity. Define the rail

\[
 r(k)=
 \begin{cases}
 R,&k\in\{0,2\},\\
 I,&k\in\{1,3\}.
 \end{cases}                                         \tag{2}
\]

For two records at the same port and on the same rail, their relative phase

\[
 \delta=k_2-k_1\pmod4                                \tag{3}
\]

has only two possibilities:

\[
 \delta=0\quad\text{bright/constructive},\qquad
 \delta=2\quad\text{dark/opposite}.                  \tag{4}
\]

Different ports or different rails are incompatible at this gate. A common
C4 rotation \(k_j\mapsto k_j+q\) leaves \(\delta\), the rail relation, and the
channel unchanged.

---

## 3. Reversible fusion/cancellation involution

Let \(D\) be one local bound-record slot. Its nonblank state stores the
channel and the complete ordered payload:

\[
 D=(\gamma,h_1,h_2),\qquad
 \gamma\in\{\mathrm{bright},\mathrm{dark}\}.          \tag{5}
\]

Define the local exchange

\[
 \mathfrak F:
 (h_1,h_2;D=0)
 \longleftrightarrow
 (0,0;D=(\gamma,h_1,h_2)),                            \tag{6}
\]

when equation (4) defines \(\gamma\). All incompatible or collision-blocked
states remain fixed.

### Theorem 1 — exact reversible record transfer

On its declared finite state space, \(\mathfrak F\):

1. is an involution;
2. preserves both record identities and their order;
3. commutes with a common C4 phase rotation;
4. preserves the complete phase-record inventory; and
5. preserves the positive token energy

\[
 E_{\rm rec}=E_*
 \left(N_{\rm active}+2N_{\rm bound}\right),\qquad E_*>0. \tag{7}
\]

### Proof

The forward branch writes every datum needed by the inverse into \(D\). The
reverse branch restores those two records and clears \(D\); a second
application therefore restores the input. Equation (3) is invariant under a
common C4 rotation. Both sides of equation (6) contain the same two payload
records, and equation (7) assigns them the same total token energy. \(\square\)

For \(\gamma=\mathrm{dark}\), equation (6) is physical cancellation without
information destruction. The pair is inactive for interference but remains
in a finite environment/memory record with an exact inverse.

---

## 4. Canonical finite cancellation

At each physical port, route the real phases \(0,2\) to opposite queue heads
and likewise route the imaginary phases \(1,3\). Apply the dark branch of
\(\mathfrak F\) until either queue is empty. For phase counts
\((N_{o,0},N_{o,1},N_{o,2},N_{o,3})\), the active residuals are exactly

\[
 r_{o,R}=|N_{o,0}-N_{o,2}|,\qquad
 r_{o,I}=|N_{o,1}-N_{o,3}|.                          \tag{8}
\]

The order of opposite-pair encounters cannot change equation (8). Every
canceled pair contributes zero to the active coherent response and survives
in a dark record, so

\[
 Z_o^{\rm residual}=Z_o^{\rm input}.                 \tag{9}
\]

The exact certificate constructs a deterministic queue pairing and verifies
equations (8)--(9) exhaustively over the declared bounded census. The
demultiplexing/router hardware that brings every eligible pair together by
Moore-local motion remains an action-level construction debt.

---

## 5. Why the click gate must be nondestructive

The exact square includes self-address pairs \((h,h)\). A destructive
two-record fusion cannot consume one physical record twice. The detector
comparison must therefore read the shared bank nondestructively.

Let \(c\in\mathbb Z_2\) be a local event bit. Define

\[
 \mathfrak C(h_a,h_b,c)
 =
 \left(h_a,h_b,\,
 c\mathbin{\mathrm{xor}}\chi(h_a,h_b)\right),         \tag{10}
\]

where

\[
 \chi(h_a,h_b)=
 \begin{cases}
 1,&o_a=o_b,\ k_a=k_b,\\
 0,&\text{otherwise}.
 \end{cases}                                         \tag{11}
\]

After equation (8), same-port/same-rail records necessarily have the same
phase, so equation (11) is exactly the bright compatibility test.

### Theorem 2 — exact reversible comparator

\(\mathfrak C\) is an involution, leaves both records unchanged, is globally
C4 covariant, and toggles the event bit exactly on a compatible ordered
address pair. Self-address readout is valid because the record is a catalyst,
not a consumed input.

The event bit is a reversible microscopic flag, not yet an irreversible
macroscopic detector click. Its downstream work and amplification remain
open.

---

## 6. Conditional circuit pushforward

Place the residual records from equation (8) in the shared bank of capacity
\(L\). Advance the two address heads with periods \(L,L+1\) as in the
coprime-address theorem, and apply equation (10) at each joint address state.
One complete orbit visits every ordered bank-address pair once. Therefore

\[
 C_o=r_{o,R}^2+r_{o,I}^2=|Z_o|^2,                   \tag{12}
\]

and, conditioning the complete event stream on \(\chi=1\),

\[
 f_o={C_o\over\sum_rC_r}
 ={ |Z_o|^2\over\sum_r|Z_r|^2}.                     \tag{13}
\]

This finite pushforward contains no random-number generator and no table of
desired outcome weights. The phase relation, physical port route, local
address motion, and complete-period orbit determine the counts.

---

## 7. Common-action embedding and its boundary

The unadopted
[phase-complete bond action](../../scopes_and_specs/SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md)
already admits local constraint multipliers. A detector-memory extension can
place the three exact permutations in that same constraint family:

\[
 S_{\rm det}=S_C+
 \sum_n\left[
 \langle\Lambda_n,\mathcal C_{\rm shift}\rangle+
 \langle\Xi_n,\mathcal C_{\rm fusion}\rangle+
 \langle\Pi_n,\mathcal C_{\rm click}\rangle
 \right].                                           \tag{14}
\]

Here:

- \(\mathcal C_{\rm shift}\) is the one-hop cyclic address transport;
- \(\mathcal C_{\rm fusion}\) is equation (6); and
- \(\mathcal C_{\rm click}\) is equation (10).

Equation (14) is a **selection candidate**, not yet the requested
non-tautological native action. Writing exact permutations as constraints
proves their mutual consistency and reversibility; it does not explain why
the substrate selects this router, forms this apparatus, or supplies its work.

The event bit must ultimately control the same neutral manifestation
transaction used elsewhere,

\[
 (0,0;0;z)\longleftrightarrow
 (\epsilon,-\epsilon;z;0),                           \tag{15}
\]

with exact Hamiltonian work, capacity debit, refractory behavior, and inverse.
Until equation (15) is generated from the common energy/action rather than
being wired to \(c=1\), physical actualization is incomplete.

---

## 8. Epistemic result

The equal-weight C4 branch now has an exact finite chain:

1. phase-labelled physical records;
2. reversible opposite-phase transfer to dark memory;
3. exact residual coherent response;
4. a shared bank with two coprime address heads;
5. a nondestructive reversible bright comparator; and
6. exact normalized \(|Z_o|^2\) event counts.

This closes the **finite cancellation/enumerator algebra**. It does not close
the general physical Born rule.

Still required:

1. action-generated record production and phase routing;
2. finite Moore-local queue formation and dark-memory ownership;
3. detector energy, manifestation work, amplification, and reciprocal reset;
4. a one-outcome-per-preparation competition law preserving equation (13);
5. finite-window robustness away from a complete address period;
6. rational blocking and a controlled general-complex-amplitude limit;
7. sequential/context composition; and
8. multipartite operational no-signalling.

---

## 9. Falsifiers and next gate

Close or demote the route if:

- local routing cannot reach the canonical residual without target labels;
- dark memory overflows for a finite declared apparatus capacity;
- the event latch or reset biases the complete-period count;
- manifestation work changes the relative frequencies;
- self-address terms cannot be represented physically as a nondestructive
  autocorrelation;
- the general-amplitude limit requires fitted multiplicities; or
- multipartite context composition permits signalling.

The next target is no longer another counting identity. It is the
**work-complete actualization transaction**: add a finite positive detector
reserve to equation (14), derive equation (15) as an energy-conserving
reversible response to the event bit, and show that reset exports consequences
without deleting the complete history.

That target is now passed on the prepared finite equal-weight domain by the
[physical Born actualization tape](THEOREM_C4_PHYSICAL_BORN_ACTUALIZATION_TAPE_v1.md).
Every pointer state owns a separate complete detector token. The unchanged
bright signal pair controls its reserve-to-manifested ownership transfer, so
self-address terms need no record copy; a second complete orbit restores the
tape. General preparation, router/tape formation, amplitude completion,
single-trial competition, and multipartite no-signalling remain open.
